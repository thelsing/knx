/*
 * Copyright (c) 2017-2019, Texas Instruments Incorporated
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
#include <ti/drivers/AESECB.h>
#include <ti/drivers/aesecb/AESECBCC26XX.h>
#include <ti/drivers/cryptoutils/sharedresources/CryptoResourceCC26XX.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_crypto.h)
#include DeviceFamily_constructPath(driverlib/aes.h)
#include DeviceFamily_constructPath(driverlib/cpu.h)
#include DeviceFamily_constructPath(driverlib/interrupt.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/smph.h)


/* Forward declarations */
static void AESECB_hwiFxn (uintptr_t arg0);
static int_fast16_t AESECB_waitForResult(AESECB_Handle handle);
static void AESECB_cleanup(AESECB_Handle handle);
static int_fast16_t AESECB_startOperation(AESECB_Handle handle,
                                          AESECB_Operation *operation,
                                          AESECB_OperationType operationType);

/* Extern globals */
extern const AESECB_Config AESECB_config[];
extern const uint_least8_t AESECB_count;

/* Static globals */
static bool isInitialized = false;

/*
 *  ======== AESECB_hwiFxn ========
 */
static void AESECB_hwiFxn (uintptr_t arg0) {
    AESECBCC26XX_Object *object = ((AESECB_Handle)arg0)->object;
    uint32_t key;

    key = HwiP_disable();
    if (!object->operationCanceled) {

        /* Mark that we are done with the operation so that AESECB_cancelOperation
         * knows not to try canceling.
         */
        object->operationInProgress = false;

        HwiP_restore(key);
    }
    else {
        HwiP_restore(key);
        return;
    }

    if (AESIntStatusRaw() & AES_DMA_BUS_ERR) {
        object->returnStatus = AESECB_STATUS_ERROR;
    }

    AESIntClear(AES_RESULT_RDY | AES_DMA_IN_DONE | AES_DMA_BUS_ERR);

    AESECB_cleanup((AESECB_Handle)arg0);

    if (object->returnBehavior == AESECB_RETURN_BEHAVIOR_BLOCKING) {
        SemaphoreP_post(&CryptoResourceCC26XX_operationSemaphore);
    }
    else {
        object->callbackFxn((AESECB_Handle)arg0,
                            object->returnStatus,
                            object->operation,
                            object->operationType);
    }
}

/*
 *  ======== AESECB_cleanup ========
 */
static void AESECB_cleanup(AESECB_Handle handle) {

    /* Since plaintext keys use two reserved (by convention) slots in the keystore,
     * the slots must be invalidated to prevent its re-use without reloading
     * the key material again.
     */
    AESInvalidateKey(AES_KEY_AREA_6);
    AESInvalidateKey(AES_KEY_AREA_7);

    /*  This powers down all sub-modules of the crypto module until needed.
     *  It does not power down the crypto module at PRCM level and provides small
     *  power savings.
     */
    AESSelectAlgorithm(0x00);

    /*  Grant access for other threads to use the crypto module.
     *  The semaphore must be posted before the callbackFxn to allow the chaining
     *  of operations.
     */
    SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);

    Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
}

/*
 *  ======== AESECB_init ========
 */
void AESECB_init(void) {
    CryptoResourceCC26XX_constructRTOSObjects();

    isInitialized = true;
}

/*
 *  ======== AESECB_open ========
 */
AESECB_Handle AESECB_open(uint_least8_t index, AESECB_Params *params) {
    AESECB_Handle               handle;
    AESECBCC26XX_Object        *object;
    uint_fast8_t                key;

    handle = (AESECB_Handle)&(AESECB_config[index]);
    object = handle->object;

    DebugP_assert(index < AESECB_count);

    key = HwiP_disable();

    if (!isInitialized ||  object->isOpen) {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    /* If params are NULL, use defaults */
    if (params == NULL) {
        params = (AESECB_Params *)&AESECB_defaultParams;
    }

    /* This is currently not supported. Eventually it will make the TRNG generate the nonce */
    DebugP_assert(!params->nonceInternallyGenerated);
    DebugP_assert(params->returnBehavior == AESECB_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    object->returnBehavior = params->returnBehavior;
    object->callbackFxn = params->callbackFxn;
    object->semaphoreTimeout = params->returnBehavior == AESECB_RETURN_BEHAVIOR_BLOCKING ? params->timeout : SemaphoreP_NO_WAIT;

    /* Set power dependency - i.e. power up and enable clock for Crypto (CryptoResourceCC26XX) module. */
    Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);

    return handle;
}

/*
 *  ======== AESECB_close ========
 */
void AESECB_close(AESECB_Handle handle) {
    AESECBCC26XX_Object         *object;

    DebugP_assert(handle);

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;

    /* Mark the module as available */
    object->isOpen = false;

    /* Release power dependency on Crypto Module. */
    Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== AESECB_waitForResult ========
 */
int_fast16_t AESECB_waitForResult(AESECB_Handle handle){
    AESECBCC26XX_Object *object = handle->object;

    object->operationInProgress = true;

    if (object->returnBehavior == AESECB_RETURN_BEHAVIOR_POLLING) {
        /* Wait until the operation is complete and check for DMA errors. */
        if(AESWaitForIRQFlags(AES_RESULT_RDY | AES_DMA_BUS_ERR) & AES_DMA_BUS_ERR){
            object->returnStatus = AESECB_STATUS_ERROR;
        }

        /* Mark that we are done with the operation */
        object->operationInProgress = false;

        /* Make sure to also clear DMA_IN_DONE as it is not cleared above
         * but will be set none-the-less.
         */
        AESIntClear(AES_RESULT_RDY | AES_DMA_IN_DONE | AES_DMA_BUS_ERR);

        /* Instead of posting the swi to handle cleanup, we will execute
         * the core of the function here */
        AESECB_cleanup(handle);

        return object->returnStatus;
    }
    else if (object->returnBehavior == AESECB_RETURN_BEHAVIOR_BLOCKING) {
        SemaphoreP_pend(&CryptoResourceCC26XX_operationSemaphore, SemaphoreP_WAIT_FOREVER);

        return object->returnStatus;
    }
    else {
        return AESECB_STATUS_SUCCESS;
    }
}

/*
 *  ======== AESECB_startOperation ========
 */
static int_fast16_t AESECB_startOperation(AESECB_Handle handle,
                                          AESECB_Operation *operation,
                                          AESECB_OperationType operationType) {
    AESECBCC26XX_Object *object = handle->object;
    AESECBCC26XX_HWAttrs const *hwAttrs = handle->hwAttrs;
    SemaphoreP_Status resourceAcquired;

    /* Only plaintext CryptoKeys are supported for now */
    uint16_t keyLength = operation->key->u.plaintext.keyLength;
    uint8_t *keyingMaterial = operation->key->u.plaintext.keyMaterial;

    DebugP_assert(handle);
    DebugP_assert(operation);
    DebugP_assert(operationType == AESECB_OPERATION_TYPE_DECRYPT ||
                  operationType == AESECB_OPERATION_TYPE_ENCRYPT);
    DebugP_assert(keyingMaterial);
    DebugP_assert(keyLength == 16 ||
                  keyLength == 32);

    /* Try and obtain access to the crypto module */
    resourceAcquired = SemaphoreP_pend(&CryptoResourceCC26XX_accessSemaphore,
                                       object->semaphoreTimeout);

    if (resourceAcquired != SemaphoreP_OK) {
        return AESECB_STATUS_RESOURCE_UNAVAILABLE;
    }

    object->operationType = operationType;
    object->operation = operation;
    /* We will only change the returnStatus if there is an error */
    object->returnStatus = AESECB_STATUS_SUCCESS;
    object->operationCanceled = false;

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&CryptoResourceCC26XX_hwi, AESECB_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_CRYPTO_RESULT_AVAIL_IRQ, hwAttrs->intPriority);

    /* Load the key from RAM or flash into the key store at a hardcoded and reserved location */
    if (AESWriteToKeyStore(keyingMaterial, keyLength, AES_KEY_AREA_6) != AES_SUCCESS) {
        /* Release the CRYPTO mutex */
        SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);

        return AESECB_STATUS_ERROR;
    }

    /* If we are in AESECB_RETURN_BEHAVIOR_POLLING, we do not want an interrupt to trigger.
     * AESWriteToKeyStore() disables and then re-enables the CRYPTO IRQ in the NVIC so we
     * need to disable it before kicking off the operation.
     */
    if (object->returnBehavior == AESECB_RETURN_BEHAVIOR_POLLING)  {
        IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }

    /* Power the AES sub-module of the crypto module */
    AESSelectAlgorithm(AES_ALGSEL_AES);

    /* Load the key from the key store into the internal register banks of the AES sub-module */
    if (AESReadFromKeyStore(AES_KEY_AREA_6) != AES_SUCCESS) {
        /* Since plaintext keys use two reserved (by convention) slots in the keystore,
         * the slots must be invalidated to prevent its re-use without reloading
         * the key material again.
         */
        AESInvalidateKey(AES_KEY_AREA_6);
        AESInvalidateKey(AES_KEY_AREA_7);

        /* Release the CRYPTO mutex */
        SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);

        return AESECB_STATUS_ERROR;
    }

    /* Disallow standby. We are about to configure and start the accelerator.
     * Setting the constraint should happen after all opportunities to fail out of the
     * function. This way, we do not need to undo it each time we exit with a failure.
     */
    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* Set direction of operation. */
    AESSetCtrl(operationType == AESECB_OPERATION_TYPE_DECRYPT ? 0 : CRYPTO_AESCTL_DIR);

    AESSetDataLength(operation->inputLength);

    AESStartDMAOperation(operation->input, operation->inputLength, operation->output, operation->inputLength);

    return AESECB_waitForResult(handle);
}

/*
 *  ======== AESECB_oneStepEncrypt ========
 */
int_fast16_t AESECB_oneStepEncrypt(AESECB_Handle handle, AESECB_Operation *operationStruct) {

    return AESECB_startOperation(handle, operationStruct, AESECB_OPERATION_TYPE_ENCRYPT);
}

/*
 *  ======== AESECB_oneStepDecrypt ========
 */
int_fast16_t AESECB_oneStepDecrypt(AESECB_Handle handle, AESECB_Operation *operationStruct) {

    return AESECB_startOperation(handle, operationStruct, AESECB_OPERATION_TYPE_DECRYPT);
}

/*
 *  ======== AESECB_cancelOperation ========
 */
int_fast16_t AESECB_cancelOperation(AESECB_Handle handle) {
    AESECBCC26XX_Object *object         = handle->object;
    uint32_t key;

    key = HwiP_disable();

    if (!object->operationInProgress) {
        HwiP_restore(key);
        return AESECB_STATUS_ERROR;
    }

    /* Reset the accelerator. Immediately stops ongoing operations. */
    AESReset();

    /* Consume any outstanding interrupts we may have accrued
     * since disabling interrupts.
     */
    IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);

    Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    object->operationCanceled = true;
    object->returnStatus = AESECB_STATUS_CANCELED;

    HwiP_restore(key);

    /*  Grant access for other threads to use the crypto module.
     *  The semaphore must be posted before the callbackFxn to allow the chaining
     *  of operations.
     */
    SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);

    if (object->returnBehavior == AESECB_RETURN_BEHAVIOR_BLOCKING) {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoResourceCC26XX_operationSemaphore);
    }
    else {
        /* Call the callback function provided by the application. */
        object->callbackFxn(handle,
                            AESECB_STATUS_CANCELED,
                            object->operation,
                            object->operationType);
    }

    return AESECB_STATUS_SUCCESS;
}
