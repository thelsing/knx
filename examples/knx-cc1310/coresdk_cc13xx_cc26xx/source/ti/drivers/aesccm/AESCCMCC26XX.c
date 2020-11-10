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
#include <ti/drivers/AESCCM.h>
#include <ti/drivers/aesccm/AESCCMCC26XX.h>
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
static void AESCCM_hwiFxn (uintptr_t arg0);
static int_fast16_t AESCCM_startOperation(AESCCM_Handle handle,
                                          AESCCM_Operation *operation,
                                          AESCCM_OperationType operationType);
static int_fast16_t AESCCM_waitForResult(AESCCM_Handle handle);
static void AESCCM_cleanup(AESCCM_Handle handle);

/* Extern globals */
extern const AESCCM_Config AESCCM_config[];
extern const uint_least8_t AESCCM_count;

/* Static globals */
static bool isInitialized = false;

/*
 *  ======== AESCCM_hwiFxn ========
 */
static void AESCCM_hwiFxn (uintptr_t arg0) {
    AESCCMCC26XX_Object *object = ((AESCCM_Handle)arg0)->object;
    uint32_t key;

    key = HwiP_disable();
    if (!object->operationCanceled) {

        /* Mark that we are done with the operation so that AESCCM_cancelOperation
         * knows not to try canceling.
         */
        object->operationInProgress = false;

        HwiP_restore(key);
    }
    else {
        HwiP_restore(key);
        return;
    }

    /* Propagate the DMA error from driverlib to the application */
    if (AESIntStatusRaw() & AES_DMA_BUS_ERR) {
        object->returnStatus = AESCCM_STATUS_ERROR;
    }

    AESIntClear(AES_RESULT_RDY | AES_DMA_IN_DONE | AES_DMA_BUS_ERR);

    /* Handle cleaning up of the operation. Read out the tag
     * or verify it against the provided one, invalidate the key,
     * release the Power constraints, and post the access semaphore.
     */
    AESCCM_cleanup((AESCCM_Handle)arg0);

    if (object->returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING) {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoResourceCC26XX_operationSemaphore);
    }
    else {
        /* Call the callback function provided by the application.
         */
        object->callbackFxn((AESCCM_Handle)arg0,
                            object->returnStatus,
                            object->operation,
                            object->operationType);
    }
}

static void AESCCM_cleanup(AESCCM_Handle handle) {
    AESCCMCC26XX_Object *object = handle->object;

    /* We need to copy / verify the MAC now so that it is not clobbered when we
     * release the CryptoResourceCC26XX_accessSemaphore semaphore.
     */
    if (object->operationType == AESCCM_OPERATION_TYPE_ENCRYPT) {
        /* If we are encrypting and authenticating a message, we only want to
         * copy the MAC to the target buffer
         */
        AESReadTag(object->operation->mac, object->operation->macLength);
    }
    else {
        /* If we are decrypting and verifying a message, we must now verify that the provided
         * MAC matches the one calculated in the decryption operation.
         */
        uint32_t verifyResult = AESVerifyTag(object->operation->mac, object->operation->macLength);

        object->returnStatus = (verifyResult == AES_SUCCESS) ? object->returnStatus : AESCCM_STATUS_MAC_INVALID;
    }

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

    Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /*  Grant access for other threads to use the crypto module.
     *  The semaphore must be posted before the callbackFxn to allow the chaining
     *  of operations.
     */
    SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);
}

/*
 *  ======== AESCCM_init ========
 */
void AESCCM_init(void) {
    CryptoResourceCC26XX_constructRTOSObjects();

    isInitialized = true;
}

/*
 *  ======== AESCCM_open ========
 */
AESCCM_Handle AESCCM_open(uint_least8_t index, AESCCM_Params *params) {
    AESCCM_Handle               handle;
    AESCCMCC26XX_Object        *object;
    uint_fast8_t                key;

    handle = (AESCCM_Handle)&(AESCCM_config[index]);
    object = handle->object;

    DebugP_assert(index < AESCCM_count);

    key = HwiP_disable();

    if (!isInitialized ||  object->isOpen) {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    /* If params are NULL, use defaults */
    if (params == NULL) {
        params = (AESCCM_Params *)&AESCCM_defaultParams;
    }

    /* This is currently not supported. Eventually it will make the TRNG generate the nonce */
    DebugP_assert(!params->nonceInternallyGenerated);
    DebugP_assert(params->returnBehavior == AESCCM_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    object->returnBehavior = params->returnBehavior;
    object->callbackFxn = params->callbackFxn;
    object->semaphoreTimeout = params->returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING ? params->timeout : SemaphoreP_NO_WAIT;

    /* Set power dependency - i.e. power up and enable clock for Crypto (CryptoResourceCC26XX) module. */
    Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);

    return handle;
}

/*
 *  ======== AESCCM_close ========
 */
void AESCCM_close(AESCCM_Handle handle) {
    AESCCMCC26XX_Object         *object;

    DebugP_assert(handle);

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;

    /* Mark the module as available */
    object->isOpen = false;

    /* Release power dependency on Crypto Module. */
    Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== AESCCM_startOperation ========
 */
static int_fast16_t AESCCM_startOperation(AESCCM_Handle handle,
                                          AESCCM_Operation *operation,
                                          AESCCM_OperationType operationType) {
    AESCCMCC26XX_Object *object = handle->object;
    AESCCMCC26XX_HWAttrs const *hwAttrs = handle->hwAttrs;
    SemaphoreP_Status resourceAcquired;

    /* Only plaintext CryptoKeys are supported for now */
    uint16_t keyLength = operation->key->u.plaintext.keyLength;
    uint8_t *keyingMaterial = operation->key->u.plaintext.keyMaterial;

    DebugP_assert(handle);
    DebugP_assert(key);
    DebugP_assert(nonce && (nonceLength >= 7 && nonceLength <= 13));
    DebugP_assert((aad && aadLength) || (input && inputLength));
    DebugP_assert(mac && (macLength <= 16));
    DebugP_assert(key->encoding == CryptoKey_PLAINTEXT);

    /* Try and obtain access to the crypto module */
    resourceAcquired = SemaphoreP_pend(&CryptoResourceCC26XX_accessSemaphore,
                                       object->semaphoreTimeout);

    if (resourceAcquired != SemaphoreP_OK) {
        return AESCCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    object->operationType = operationType;
    object->operation = operation;
    /* We will only change the returnStatus if there is an error */
    object->returnStatus = AESCCM_STATUS_SUCCESS;
    object->operationCanceled = false;

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&CryptoResourceCC26XX_hwi, AESCCM_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_CRYPTO_RESULT_AVAIL_IRQ, hwAttrs->intPriority);

    /* Load the key from RAM or flash into the key store at a hardcoded and reserved location */
    if (AESWriteToKeyStore(keyingMaterial, keyLength, AES_KEY_AREA_6) != AES_SUCCESS) {
        /* Release the CRYPTO mutex */
        SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);

        return AESCCM_STATUS_ERROR;
    }

    /* If we are in AESCCM_RETURN_BEHAVIOR_POLLING, we do not want an interrupt to trigger.
     * AESWriteToKeyStore() disables and then re-enables the CRYPTO IRQ in the NVIC so we
     * need to disable it before kicking off the operation.
     */
    if (object->returnBehavior == AESCCM_RETURN_BEHAVIOR_POLLING) {
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

        return AESCCM_STATUS_ERROR;
    }

    /* Disallow standby. We are about to configure and start the accelerator.
     * Setting the constraint should happen after all opportunities to fail out of the
     * function. This way, we do not need to undo it each time we exit with a failure.
     */
    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    AESWriteCCMInitializationVector(operation->nonce, operation->nonceLength);

    AESConfigureCCMCtrl(operation->nonceLength, operation->macLength, operationType == AESCCM_OPERATION_TYPE_ENCRYPT);

    AESSetDataLength(operation->inputLength);
    AESSetAuthLength(operation->aadLength);

    if (operation->aadLength) {
        /* If aadLength were 0, AESWaitForIRQFlags() would never return as the AES_DMA_IN_DONE flag
         * would never trigger.
         */
        AESStartDMAOperation(operation->aad, operation->aadLength,  NULL, 0);
        AESWaitForIRQFlags(AES_DMA_IN_DONE | AES_DMA_BUS_ERR);
    }

    AESStartDMAOperation(operation->input, operation->inputLength, operation->output, operation->inputLength);


    return AESCCM_waitForResult(handle);
}

/*
 *  ======== AESCCM_waitForResult ========
 */
static int_fast16_t AESCCM_waitForResult(AESCCM_Handle handle) {
    AESCCMCC26XX_Object *object = handle->object;

    object->operationInProgress = true;

    if (object->returnBehavior == AESCCM_RETURN_BEHAVIOR_POLLING) {
        /* Wait until the operation is complete and check for DMA errors. */
        if(AESWaitForIRQFlags(AES_RESULT_RDY | AES_DMA_BUS_ERR) & AES_DMA_BUS_ERR){
            object->returnStatus = AESCCM_STATUS_ERROR;
        }

        /* Mark that we are done with the operation */
        object->operationInProgress = false;

        /* Make sure to also clear DMA_IN_DONE as it is not cleared above
         * but will be set none-the-less.
         */
        AESIntClear(AES_RESULT_RDY | AES_DMA_IN_DONE | AES_DMA_BUS_ERR);

        /* Instead of posting the swi to handle cleanup, we will execute
         * the core of the function here */
        AESCCM_cleanup(handle);

        return object->returnStatus;
    }
    else if (object->returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING) {
        SemaphoreP_pend(&CryptoResourceCC26XX_operationSemaphore, SemaphoreP_WAIT_FOREVER);

        return object->returnStatus;
    }
    else {
        return AESCCM_STATUS_SUCCESS;
    }
}

/*
 *  ======== AESCCM_oneStepEncrypt ========
 */
int_fast16_t AESCCM_oneStepEncrypt(AESCCM_Handle handle, AESCCM_Operation *operationStruct) {

    return AESCCM_startOperation(handle, operationStruct, AESCCM_OPERATION_TYPE_ENCRYPT);
}

/*
 *  ======== AESCCM_oneStepDecrypt ========
 */
int_fast16_t AESCCM_oneStepDecrypt(AESCCM_Handle handle, AESCCM_Operation *operationStruct) {

    return AESCCM_startOperation(handle, operationStruct, AESCCM_OPERATION_TYPE_DECRYPT);
}

/*
 *  ======== AESCCM_cancelOperation ========
 */
int_fast16_t AESCCM_cancelOperation(AESCCM_Handle handle) {
    AESCCMCC26XX_Object *object         = handle->object;
    uint32_t key;

    key = HwiP_disable();

    if (!object->operationInProgress) {
        HwiP_restore(key);
        return AESCCM_STATUS_ERROR;
    }

    /* Reset the accelerator. Immediately stops ongoing operations. */
    AESReset();

    /* Consume any outstanding interrupts we may have accrued
     * since disabling interrupts.
     */
    IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);

    Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    object->operationCanceled = true;
    object->returnStatus = AESCCM_STATUS_CANCELED;

    HwiP_restore(key);

    /*  Grant access for other threads to use the crypto module.
     *  The semaphore must be posted before the callbackFxn to allow the chaining
     *  of operations.
     */
    SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);


    if (object->returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING) {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoResourceCC26XX_operationSemaphore);
    }
    else {
        /* Call the callback function provided by the application. */
        object->callbackFxn(handle,
                            AESCCM_STATUS_CANCELED,
                            object->operation,
                            object->operationType);
    }

    return AESCCM_STATUS_SUCCESS;
}
