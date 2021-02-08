/*
 * Copyright (c) 2018-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/DebugP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/TRNG.h>
#include <ti/drivers/trng/TRNGCC26XX.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(driverlib/cpu.h)
#include DeviceFamily_constructPath(driverlib/interrupt.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/trng.h)

/* Macros */
#define MAX(x,y)   (((x) > (y)) ?  (x) : (y))
#define MIN(x,y)   (((x) < (y)) ?  (x) : (y))

/* Forward declarations */
static void TRNGCC26XX_basicHwiFxn (uintptr_t arg0);
static int_fast16_t TRNGCC26XX_waitForAccess(TRNG_Handle handle);
static int_fast16_t TRNGCC26XX_waitForResult(TRNG_Handle handle);
static void TRNGCC26XX_copyEntropy(uint32_t interruptStatus, TRNGCC26XX_Object *object);
static void TRNG_restartFRO(uint32_t interruptStatus);

/* Extern globals */
extern const TRNG_Config TRNG_config[];
extern const uint_least8_t TRNG_count;

/* TRNG driver semaphore used to synchronize accesses to the TRNG module */
static SemaphoreP_Struct TRNGCC26XX_accessSemaphore;
static SemaphoreP_Struct TRNGCC26XX_operationSemaphore;

static HwiP_Struct TRNGCC26XX_hwi;

static bool isInitialized = false;

static void errorSpin(uintptr_t arg) {
    while(1);
}

static void TRNGCC26XX_copyEntropy(uint32_t interruptStatus, TRNGCC26XX_Object *object) {
    uint8_t tmpEntropyBuf[TRNGCC26XX_MIN_BYTES_PER_ITERATION];
    size_t bytesToCopy = 0;

    if (interruptStatus & TRNG_IRQFLAGSTAT_RDY_M) {
        ((uint32_t *)tmpEntropyBuf)[0] = TRNGNumberGet(TRNG_LOW_WORD);
        ((uint32_t *)tmpEntropyBuf)[1] = TRNGNumberGet(TRNG_HI_WORD);

        bytesToCopy =  MIN(object->entropyRequested - object->entropyGenerated, sizeof(tmpEntropyBuf));

        memcpy(object->entropyBuffer + object->entropyGenerated,
               tmpEntropyBuf,
               bytesToCopy);

        object->entropyGenerated += bytesToCopy;
    }
}

static void TRNG_restartFRO(uint32_t interruptStatus) {
     if (interruptStatus & TRNG_IRQFLAGSTAT_SHUTDOWN_OVF_M) {
        uint32_t froAlarmMask;

        froAlarmMask = HWREG(TRNG_BASE + TRNG_O_ALARMSTOP);

        /* Clear alarms for FROs that exhibited repeating pattern */
        HWREG(TRNG_BASE + TRNG_O_ALARMMASK) = 0;

        /* Clear alarms for FROs that stopped */
        HWREG(TRNG_BASE + TRNG_O_ALARMSTOP) = 0;

        /* De-tune the FROs that had an alarm to attempt to */
        /* break their lock-in on SCLK_HF */
        HWREG(TRNG_BASE + TRNG_O_FRODETUNE) = froAlarmMask;

        /* Re-enable the FROs */
        HWREG(TRNG_BASE + TRNG_O_FROEN) |= froAlarmMask;
    }
}

/*
 *  ======== TRNGCC26XX_basicHwiFxn ========
 */
static void TRNGCC26XX_basicHwiFxn (uintptr_t arg0) {
    TRNGCC26XX_Object *object = ((TRNG_Handle)arg0)->object;
    uint32_t interruptStatus;

    interruptStatus = TRNGStatusGet();
    TRNGIntClear(TRNG_NUMBER_READY | TRNG_FRO_SHUTDOWN);

    TRNGCC26XX_copyEntropy(interruptStatus, object);

    TRNG_restartFRO(interruptStatus);

    if (object->entropyGenerated >= object->entropyRequested) {

        TRNGDisable();

        object->returnStatus = TRNG_STATUS_SUCCESS;

        /*  Grant access for other threads to use the crypto module.
         *  The semaphore must be posted before the callbackFxn to allow the chaining
         *  of operations.
         */
        SemaphoreP_post(&TRNGCC26XX_accessSemaphore);

        Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

        /* This function is only ever registered when in TRNG_RETURN_BEHAVIOR_BLOCKING
         * or TRNG_RETURN_BEHAVIOR_POLLING.
         */
        if (object->returnBehavior == TRNG_RETURN_BEHAVIOR_BLOCKING) {
            SemaphoreP_post(&TRNGCC26XX_operationSemaphore);
        }
        else if (object->returnBehavior == TRNG_RETURN_BEHAVIOR_CALLBACK) {
            object->callbackFxn((TRNG_Handle)arg0,
                                object->returnStatus,
                                object->entropyKey);
        }
    }
}

/*
 *  ======== TRNG_init ========
 */
void TRNG_init(void) {
    uint_fast8_t key;

    key = HwiP_disable();

    if (!isInitialized) {
        /* Construct the common Hwi with a dummy ISR function. This should not matter as the function is set
         * whenever we start an operation after pending on TRNGCC26XX_accessSemaphore
         */
        HwiP_construct(&(TRNGCC26XX_hwi), INT_TRNG_IRQ, errorSpin, NULL);

        SemaphoreP_constructBinary(&TRNGCC26XX_accessSemaphore, 1);
        SemaphoreP_constructBinary(&TRNGCC26XX_operationSemaphore, 0);

        isInitialized = true;
    }

    HwiP_restore(key);
}

/*
 *  ======== TRNG_open ========
 */
TRNG_Handle TRNG_open(uint_least8_t index, TRNG_Params *params) {
    DebugP_assert(index <= TRNG_count);

    TRNG_Config *config = (TRNG_Config*)&TRNG_config[index];

    return TRNGCC26XX_construct(config, params);
}

/*
 *  ======== TRNGCC26XX_construct ========
 */
TRNG_Handle TRNGCC26XX_construct(TRNG_Config *config, const TRNG_Params *params) {
    TRNG_Handle                 handle;
    TRNGCC26XX_Object           *object;
    TRNGCC26XX_HWAttrs const    *hwAttrs;
    uintptr_t                   key;

    handle = config;
    object = handle->object;
    hwAttrs = handle->hwAttrs;


    key = HwiP_disable();

    if (object->isOpen || !isInitialized) {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    /* If params are NULL, use defaults */
    if (params == NULL) {
        params = (TRNG_Params *)&TRNG_defaultParams;
    }

    object->returnBehavior      = params->returnBehavior;
    object->semaphoreTimeout    = params->timeout;
    object->callbackFxn         = params->callbackFxn;

    if (hwAttrs->samplesPerCycle >= TRNGCC26XX_SAMPLES_PER_CYCLE_MIN &&
        hwAttrs->samplesPerCycle <= TRNGCC26XX_SAMPLES_PER_CYCLE_MAX) {

        object->samplesPerCycle = hwAttrs->samplesPerCycle;

    } else {
        object->samplesPerCycle = TRNGCC26XX_SAMPLES_PER_CYCLE_DEFAULT;
    }

    /* Set power dependency - i.e. power up and enable clock for TRNG (TRNGCC26XX) module. */
    Power_setDependency(PowerCC26XX_PERIPH_TRNG);

    return handle;
}

/*
 *  ======== TRNG_close ========
 */
void TRNG_close(TRNG_Handle handle) {
    TRNGCC26XX_Object         *object;

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;

    /* Mark the module as available */
    object->isOpen = false;

    /* Release power dependency on TRNG Module. */
    Power_releaseDependency(PowerCC26XX_PERIPH_TRNG);


}

/*
 *  ======== TRNGCC26XX_waitForAccess ========
 */
static int_fast16_t TRNGCC26XX_waitForAccess(TRNG_Handle handle) {
    TRNGCC26XX_Object *object = handle->object;
    uint32_t timeout;

    /* Set to SemaphoreP_NO_WAIT to start operations from SWI or HWI context */
    timeout = object->returnBehavior == TRNG_RETURN_BEHAVIOR_BLOCKING ? object->semaphoreTimeout : SemaphoreP_NO_WAIT;

    return SemaphoreP_pend(&TRNGCC26XX_accessSemaphore, timeout);
}

/*
 *  ======== TRNGCC26XX_waitForResult ========
 */
static int_fast16_t TRNGCC26XX_waitForResult(TRNG_Handle handle){
    TRNGCC26XX_Object *object = handle->object;

    if (object->returnBehavior == TRNG_RETURN_BEHAVIOR_POLLING) {

        /* Repeat until we have generated enough entropy. */
        while(object->entropyGenerated < object->entropyRequested) {
            /* Wait until the TRNG has generated 64 bits of entropy */
            do {
                CPUdelay(1);
            }
            while(!(TRNGStatusGet() & (TRNG_NUMBER_READY | TRNG_FRO_SHUTDOWN)));

            TRNGCC26XX_basicHwiFxn((uintptr_t)handle);
        }

        return object->returnStatus;
    }
    else if (object->returnBehavior == TRNG_RETURN_BEHAVIOR_BLOCKING) {

        SemaphoreP_pend(&TRNGCC26XX_operationSemaphore, SemaphoreP_WAIT_FOREVER);

        return object->returnStatus;
    }
    else {
        return TRNG_STATUS_SUCCESS;
    }
}

/*
 *  ======== TRNGCC26XX_setSamplesPerCycle ========
 * samplesPerCycle must be between 2^8 and 2^24 (256 and 16777216)
 */
int_fast16_t TRNGCC26XX_setSamplesPerCycle(TRNG_Handle handle, uint32_t samplesPerCycle) {
    TRNGCC26XX_Object *object = handle->object;

    object->samplesPerCycle = samplesPerCycle;
    return TRNG_STATUS_SUCCESS;
}

/*
 *  ======== TRNG_generateEntropy ========
 */
int_fast16_t TRNG_generateEntropy(TRNG_Handle handle, CryptoKey *entropy) {
    TRNGCC26XX_Object *object = handle->object;
    TRNGCC26XX_HWAttrs const *hwAttrs = handle->hwAttrs;

    /* Try and obtain access to the crypto module */
    if (TRNGCC26XX_waitForAccess(handle) != SemaphoreP_OK) {
        return TRNG_STATUS_RESOURCE_UNAVAILABLE;
    }

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    object->entropyGenerated    = 0;
    object->entropyKey          = entropy;
    object->entropyBuffer       = entropy->u.plaintext.keyMaterial;
    object->entropyRequested    = entropy->u.plaintext.keyLength;

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&TRNGCC26XX_hwi, TRNGCC26XX_basicHwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_TRNG_IRQ, hwAttrs->intPriority);

    if (object->returnBehavior == TRNG_RETURN_BEHAVIOR_POLLING) {
        TRNGIntDisable(TRNG_NUMBER_READY | TRNG_FRO_SHUTDOWN);
    }
    else {
        TRNGIntEnable(TRNG_NUMBER_READY | TRNG_FRO_SHUTDOWN);
    }

    /* The first argument copies arg2 when set to zero - this instructs
     * the TRNG to sample exactly samplesPerCycle times. The final argument
     * causes the samples to happen each clock cycle.
     */
    TRNGConfigure(0, object->samplesPerCycle, 0);
    TRNGEnable();

    return TRNGCC26XX_waitForResult(handle);
}
