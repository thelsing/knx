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
#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/AESGCM.h>
#include <ti/drivers/aesgcm/AESGCMCC26XX.h>
#include <ti/drivers/cryptoutils/sharedresources/CryptoResourceCC26XX.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_crypto.h)
#include DeviceFamily_constructPath(driverlib/aes.h)


/* Forward declarations */
static void AESGCM_hwiFxn (uintptr_t arg0);
static int_fast16_t AESGCM_startOperation(AESGCM_Handle handle,
                                          AESGCM_Operation *operation,
                                          AESGCM_OperationType operationType);
static int_fast16_t AESGCM_waitForResult(AESGCM_Handle handle);
static void AESGCM_cleanup(AESGCM_Handle handle);

/* Extern globals */
extern const AESGCM_Config AESGCM_config[];
extern const uint_least8_t AESGCM_count;

/* Static globals */
static bool isInitialized = false;


/*
 *  ======== AESGCM_hwiFxn ========
 */
static void AESGCM_hwiFxn (uintptr_t arg0) {
    AESGCMCC26XX_Object *object = ((AESGCM_Handle)arg0)->object;
    uint32_t key;

    key = HwiP_disable();
    if (!object->operationCanceled) {

        /* Mark that we are done with the operation so that AESGCM_cancelOperation
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
        object->returnStatus = AESGCM_STATUS_ERROR;
    }

    AESIntClear(AES_RESULT_RDY | AES_DMA_IN_DONE | AES_DMA_BUS_ERR);

    /* Handle cleaning up of the operation. Read out the tag
     * or verify it against the provided one, invalidate the key,
     * release the Power constraints, and post the access semaphore.
     */
    AESGCM_cleanup((AESGCM_Handle)arg0);

    if (object->returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING) {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoResourceCC26XX_operationSemaphore);
    }
    else {
        /* Call the callback function provided by the application.
         */
        object->callbackFxn((AESGCM_Handle)arg0,
                            object->returnStatus,
                            object->operation,
                            object->operationType);
    }
}

static void AESGCM_cleanup(AESGCM_Handle handle) {
    AESGCMCC26XX_Object *object = handle->object;

    /* We need to copy / verify the MAC now so that it is not clobbered when we
     * release the CryptoResourceCC26XX_accessSemaphore semaphore.
     */
    if (object->operationType == AESGCM_OPERATION_TYPE_ENCRYPT) {
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

        object->returnStatus = (verifyResult == AES_SUCCESS) ? object->returnStatus : AESGCM_STATUS_MAC_INVALID;
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
 *  ======== AESGCM_init ========
 */
void AESGCM_init(void) {
    CryptoResourceCC26XX_constructRTOSObjects();

    isInitialized = true;
}

/*
 *  ======== AESGCM_open ========
 */
AESGCM_Handle AESGCM_open(uint_least8_t index, AESGCM_Params *params) {
    AESGCM_Handle               handle;
    AESGCMCC26XX_Object        *object;
    uint_fast8_t                key;

    handle = (AESGCM_Handle)&(AESGCM_config[index]);
    object = handle->object;

    DebugP_assert(index < AESGCM_count);

    key = HwiP_disable();

    if (!isInitialized ||  object->isOpen) {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    /* If params are NULL, use defaults */
    if (params == NULL) {
        params = (AESGCM_Params *)&AESGCM_defaultParams;
    }

    /* This is currently not supported. Eventually it will make the TRNG generate the iv */
    DebugP_assert(!params->ivInternallyGenerated);
    DebugP_assert(params->returnBehavior == AESGCM_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    object->returnBehavior = params->returnBehavior;
    object->callbackFxn = params->callbackFxn;
    object->semaphoreTimeout = params->returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING ? params->timeout : SemaphoreP_NO_WAIT;

    /* Set power dependency - i.e. power up and enable clock for Crypto (CryptoResourceCC26XX) module. */
    Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);

    return handle;
}

/*
 *  ======== AESGCM_close ========
 */
void AESGCM_close(AESGCM_Handle handle) {
    AESGCMCC26XX_Object         *object;

    DebugP_assert(handle);

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;

    /* Mark the module as available */
    object->isOpen = false;

    /* Release power dependency on Crypto Module. */
    Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== AESGCM_startOperation ========
 */
static int_fast16_t AESGCM_startOperation(AESGCM_Handle handle,
                                          AESGCM_Operation *operation,
                                          AESGCM_OperationType operationType) {
    AESGCMCC26XX_Object *object = handle->object;
    AESGCMCC26XX_HWAttrs const *hwAttrs = handle->hwAttrs;
    SemaphoreP_Status resourceAcquired;
    uint32_t aesCtrl;

    /* Only plaintext CryptoKeys are supported for now */
    uint16_t keyLength = operation->key->u.plaintext.keyLength;
    uint8_t *keyingMaterial = operation->key->u.plaintext.keyMaterial;

    DebugP_assert(handle);
    DebugP_assert(key);
    DebugP_assert(iv && ivLength == 12);
    DebugP_assert((aad && aadLength) || (input && inputLength));
    DebugP_assert(mac && (macLength <= 16));
    DebugP_assert(key->encoding == CryptoKey_PLAINTEXT);

    /* Try and obtain access to the crypto module */
    resourceAcquired = SemaphoreP_pend(&CryptoResourceCC26XX_accessSemaphore,
                                       object->semaphoreTimeout);

    if (resourceAcquired != SemaphoreP_OK) {
        return AESGCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    object->operationType = operationType;
    object->operation = operation;
    /* We will only change the returnStatus if there is an error */
    object->returnStatus = AESGCM_STATUS_SUCCESS;
    object->operationCanceled = false;

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&CryptoResourceCC26XX_hwi, AESGCM_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_CRYPTO_RESULT_AVAIL_IRQ, hwAttrs->intPriority);

    /* Load the key from RAM or flash into the key store at a hardcoded and reserved location */
    if (AESWriteToKeyStore(keyingMaterial, keyLength, AES_KEY_AREA_6) != AES_SUCCESS) {
        /* Release the CRYPTO mutex */
        SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);

        return AESGCM_STATUS_ERROR;
    }

    /* If we are in AESGCM_RETURN_BEHAVIOR_POLLING, we do not want an interrupt to trigger.
     * AESWriteToKeyStore() disables and then re-enables the CRYPTO IRQ in the NVIC so we
     * need to disable it before kicking off the operation.
     */
    if (object->returnBehavior == AESGCM_RETURN_BEHAVIOR_POLLING) {
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

        return AESGCM_STATUS_ERROR;
    }

    /* Disallow standby. We are about to configure and start the accelerator.
     * Setting the constraint should happen after all opportunities to fail out of the
     * function. This way, we do not need to undo it each time we exit with a failure.
     */
    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV0) = ((uint32_t *)operation->iv)[0];
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV1) = ((uint32_t *)operation->iv)[1];
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV2) = ((uint32_t *)operation->iv)[2];
    /* Set initial counter value to 1. Counter is interpreted as big-endian number of last
     * four bytes of IV
     */
    HWREG(CRYPTO_BASE + CRYPTO_O_AESIV3) = 0x01000000;

    /* We need to split building aesCtrl into multiple calls.
     * Trying to combine all of the below leads to the compiler removing the GCM flag
     * for some reason.
     * Unlike CCM, GCM CTR only increments the 32-bit counter at the end of the IV not
     * the entire 16-byte IV itself.
     */
    aesCtrl = CRYPTO_AESCTL_GCM_M |
              CRYPTO_AESCTL_CTR |
              CRYPTO_AESCTL_SAVE_CONTEXT |
              CRYPTO_AESCTL_CTR_WIDTH_32_BIT;
    aesCtrl |= operationType == AESGCM_OPERATION_TYPE_ENCRYPT ? CRYPTO_AESCTL_DIR : 0;
    AESSetCtrl(aesCtrl);

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


    return AESGCM_waitForResult(handle);
}

/*
 *  ======== AESGCM_waitForResult ========
 */
static int_fast16_t AESGCM_waitForResult(AESGCM_Handle handle) {
    AESGCMCC26XX_Object *object = handle->object;

    object->operationInProgress = true;

    if (object->returnBehavior == AESGCM_RETURN_BEHAVIOR_POLLING) {
        /* Wait until the operation is complete and check for DMA errors. */
        if(AESWaitForIRQFlags(AES_RESULT_RDY | AES_DMA_BUS_ERR) & AES_DMA_BUS_ERR){
            object->returnStatus = AESGCM_STATUS_ERROR;
        }

        /* Mark that we are done with the operation */
        object->operationInProgress = false;

        /* Make sure to also clear DMA_IN_DONE as it is not cleared above
         * but will be set none-the-less.
         */
        AESIntClear(AES_RESULT_RDY | AES_DMA_IN_DONE | AES_DMA_BUS_ERR);

        /* Instead of posting the swi to handle cleanup, we will execute
         * the core of the function here */
        AESGCM_cleanup(handle);

        return object->returnStatus;
    }
    else if (object->returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING) {
        SemaphoreP_pend(&CryptoResourceCC26XX_operationSemaphore, SemaphoreP_WAIT_FOREVER);

        return object->returnStatus;
    }
    else {
        return AESGCM_STATUS_SUCCESS;
    }
}

/*
 *  ======== AESGCM_oneStepEncrypt ========
 */
int_fast16_t AESGCM_oneStepEncrypt(AESGCM_Handle handle, AESGCM_Operation *operationStruct) {

    return AESGCM_startOperation(handle, operationStruct, AESGCM_OPERATION_TYPE_ENCRYPT);
}

/*
 *  ======== AESGCM_oneStepDecrypt ========
 */
int_fast16_t AESGCM_oneStepDecrypt(AESGCM_Handle handle, AESGCM_Operation *operationStruct) {

    return AESGCM_startOperation(handle, operationStruct, AESGCM_OPERATION_TYPE_DECRYPT);
}

/*
 *  ======== AESGCM_cancelOperation ========
 */
int_fast16_t AESGCM_cancelOperation(AESGCM_Handle handle) {
    AESGCMCC26XX_Object *object         = handle->object;
    uint32_t key;

    key = HwiP_disable();

    if (!object->operationInProgress) {
        HwiP_restore(key);
        return AESGCM_STATUS_ERROR;
    }

    /* Reset the accelerator. Immediately stops ongoing operations. */
    AESReset();

    /* Consume any outstanding interrupts we may have accrued
     * since disabling interrupts.
     */
    IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);

    Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    object->operationCanceled = true;
    object->returnStatus = AESGCM_STATUS_CANCELED;

    HwiP_restore(key);

    /*  Grant access for other threads to use the crypto module.
     *  The semaphore must be posted before the callbackFxn to allow the chaining
     *  of operations.
     */
    SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);


    if (object->returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING) {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoResourceCC26XX_operationSemaphore);
    }
    else {
        /* Call the callback function provided by the application. */
        object->callbackFxn(handle,
                            AESGCM_STATUS_CANCELED,
                            object->operation,
                            object->operationType);
    }

    return AESGCM_STATUS_SUCCESS;
}
