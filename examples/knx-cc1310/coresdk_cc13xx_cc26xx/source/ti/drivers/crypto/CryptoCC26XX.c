/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
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

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/crypto/CryptoCC26XX.h>
#include <ti/drivers/cryptoutils/sharedresources/CryptoResourceCC26XX.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_crypto.h)
#include DeviceFamily_constructPath(driverlib/crypto.h)
#include DeviceFamily_constructPath(driverlib/cpu.h)
#include DeviceFamily_constructPath(driverlib/interrupt.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/smph.h)

/* Externs */
/*
 *  CryptoCC26XX configuration - initialized in the board file.
 *
 *  @code
 *  // Include drivers
 *  #include <ti/drivers/crypto/CryptoCC26XX.h>
 *
 *  // Crypto objects
 *  CryptoCC26XX_Object cryptoCC26XXObjects[CC2650_CRYPTOCOUNT];
 *
 *  // Crypto configuration structure, describing which pins are to be used
 *  const CryptoCC26XX_HWAttrs cryptoCC26XXHWAttrs[CC2650_CRYPTOCOUNT] = {
 *      {
 *          .baseAddr = CRYPTO_BASE,
 *          .powerMngrId = PERIPH_CRYPTO,
 *          .intNum = INT_CRYPTO,
 *          .intPriority = ~0
 *      }
 *  };
 *
 *  // Crypto configuration structure
 *  const CryptoCC26XX_Config CryptoCC26XX_config[] = {
 *      {&cryptoCC26XXObjects[0], &cryptoCC26XXHWAttrs[0]},
 *      {NULL, NULL}
 *  };
 *  @endcode
 */
extern const CryptoCC26XX_Config CryptoCC26XX_config[];

/* Forward declarations */
static int cryptoPostNotify(unsigned int eventType, uintptr_t eventArg, uintptr_t clientArg);
static bool cryptoTransactionPend(CryptoCC26XX_Handle handle);
static bool cryptoTransactionPoll(void);
static int cryptoTransactionExecute(CryptoCC26XX_Handle handle, CryptoCC26XX_Transaction *transaction, bool polling);

/* Flag to signal interrupt has happened */
static volatile bool hwiIntFlag;

static bool isInitialized = false;

/*
 *  ======== CryptoCC26XX_hwiIntFxn ========
 *  Hwi function that processes CryptoCC26XX interrupts.
 *
 *  @param(arg)         The CryptoCC26XX_Handle for this Hwi.
 */
void CryptoCC26XX_hwiIntFxn(uintptr_t arg)
{
    CryptoCC26XX_Object          *object;

    /* Get the pointer to the object */
    object = ((CryptoCC26XX_Handle)arg)->object;

    /* Clear interrupts */
    CRYPTOIntClear(CRYPTO_DMA_IN_DONE | CRYPTO_RESULT_RDY);

    /* Set hwi flag */
    hwiIntFlag = true;
    if(object->currentTransact->mode == CRYPTOCC26XX_MODE_BLOCKING) {
        SemaphoreP_post(&CryptoResourceCC26XX_operationSemaphore);
    }
}

/*
 *  ======== CryptoCC26XX_init ========
 */
void CryptoCC26XX_init(void)
{
    CryptoResourceCC26XX_constructRTOSObjects();

    isInitialized = true;
}

/*
 *  ======== CryptoCC26XX_Params_init ========
 */
void CryptoCC26XX_Params_init(CryptoCC26XX_Params *params)
{
    DebugP_assert(params != NULL);

    params->timeout = SemaphoreP_WAIT_FOREVER;
}

/*
 *  ======== CryptoCC26XX_AESCCM_Transac_init ========
 */
void CryptoCC26XX_Transac_init(CryptoCC26XX_Transaction *trans, CryptoCC26XX_Operation opType)
{
    DebugP_assert(trans != NULL);

    trans->opType = opType;
}

/*
 *  ======== CryptoCC26XX_open ========
 */
CryptoCC26XX_Handle CryptoCC26XX_open(unsigned int index, bool exclusiveAccess, CryptoCC26XX_Params *params)
{
    unsigned int                    key;
    CryptoCC26XX_Handle             handle;
    CryptoCC26XX_Object            *object;
    CryptoCC26XX_HWAttrs const     *hwAttrs;
    CryptoCC26XX_Params             cryptoParams;

    /* Ensure that Crypto driver has been successfully initialized */
    DebugP_assert(isInitialized == true);

    // Ensure that only one client at a time can call CryptoCC26XX_open()
    SemaphoreP_pend(&CryptoResourceCC26XX_accessSemaphore, SemaphoreP_WAIT_FOREVER);

    /* Get handle for this driver instance */
    handle = (CryptoCC26XX_Handle)&(CryptoCC26XX_config[index]);
    /* Get the pointer to the object and hwAttrs */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* Disable preemption while checking if the CryptoCC26XX is open. */
    key = HwiP_disable();

    if (isInitialized == false) {
        HwiP_restore(key);
        /* Release semaphore */
        SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);
        return (NULL);
    }

    /* Check if the CryptoCC26XX is open already with exclusive access or
     * if Crypto is already open and new client want exclusive access.
     */
    if (object->openCnt < 0) {
        HwiP_restore(key);
        DebugP_log1("CryptoCC26XX:(%p) in use with exclusive access.", hwAttrs->baseAddr);
        /* Release semaphore */
        SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);
        return (NULL);
    }

    if (object->openCnt > 0 && exclusiveAccess == true) {
        HwiP_restore(key);
        DebugP_log1("CryptoCC26XX:(%p) already in use, exclusive access is not possible.", hwAttrs->baseAddr);
        /* Release semaphore */
        SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);
        return (NULL);
    }

    /* Update openCnt */
    if (exclusiveAccess) {
        /* If we have come this far and exclusiveAccess is set, set openCnt to negative value */
        object->openCnt = -1;
    } else {
        /* If not exclusive, increment openCnt */
        object->openCnt += 1;
    }

    /* Re-enable the hwis */
    HwiP_restore(key);

    /* Check if the CryptoCC26XX is open already with the base addr. */
    if (object->openCnt > 1) {
        /* Release semaphore */
        SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);
        /* Crypto is already configured, return handle */
        return (handle);
    }

    /* If params are NULL use defaults. */
    if (params == NULL) {
        CryptoCC26XX_Params_init(&cryptoParams);
        params = &cryptoParams;
    }
    object->timeout = params->timeout;

    /* Reserve key slots 6 and 7 for use by other drivers */
    object->keyStore = 1 << 6 | 1 << 7;

    /* Set power dependency - i.e. power up and enable clock for Crypto (CryptoCC26XX) module. */
    Power_setDependency(hwAttrs->powerMngrId);

    /* Register notification function */
    Power_registerNotify(&object->cryptoNotiObj, PowerCC26XX_AWAKE_STANDBY, (Power_NotifyFxn)cryptoPostNotify, (uint32_t)handle);

    DebugP_log1("CryptoCC26XX:(%p) opened", hwAttrs->baseAddr);

    /* Release semaphore */
    SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);

    /* Return the handle */
    return (handle);
}

/*
 *  ======== CryptoCC26XX_close ========
 */
int CryptoCC26XX_close(CryptoCC26XX_Handle handle)
{
    unsigned int                        key;
    CryptoCC26XX_Object                *object;
    CryptoCC26XX_HWAttrs const         *hwAttrs;

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* If openCnt is zero and someone tries to close, return error*/
    if (object->openCnt == 0) {
        return CRYPTOCC26XX_STATUS_ERROR;
    }

    /* Disable preemption while updating opened count. */
    key = HwiP_disable();

    if (object->openCnt < 0) {
        /* If the openCnt is less than zero(-1), it is exclusiveAccess */
        object->openCnt = 0;
    } else {
        object->openCnt -= 1;
    }

    /* Re-enable the hwis */
    HwiP_restore(key);

    if(object->openCnt == 0) {
        /* Release power dependency - i.e. potentially power down peripheral domain. */
        Power_releaseDependency(hwAttrs->powerMngrId);
        /* Unregister power notification object */
        Power_unregisterNotify(&object->cryptoNotiObj);
    }

    DebugP_log1("CryptoCC26XX:(%p) closed", hwAttrs->baseAddr);
    return CRYPTOCC26XX_STATUS_SUCCESS;
}

/*
 *  ======== CryptoCC26XX_releaseKey ========
 */
int CryptoCC26XX_releaseKey(CryptoCC26XX_Handle handle, int *keyIndex)
{
    unsigned int                        hwikey;
    CryptoCC26XX_Object                *object;

    /* Get the pointer to the object */
    object = handle->object;

    /* Disable preemption while updating opened count. */
    hwikey = HwiP_disable();

    /* Clear key index in key store */
    object->keyStore &= ~(1<<(*keyIndex));
    *keyIndex = -1;

    /* Re-enable the hwis */
    HwiP_restore(hwikey);

    return CRYPTOCC26XX_STATUS_SUCCESS;
}

/*
 *  ======== CryptoCC26XX_allocateKey ========
 */
int CryptoCC26XX_allocateKey(CryptoCC26XX_Handle handle, CryptoCC26XX_KeyLocation keyLocation, const uint32_t *keySrc)
{
    unsigned long              res;
    int                        keyIndex;
    CryptoCC26XX_Object       *object;
    int                        i;

    object = handle->object;
    keyIndex = CRYPTOCC26XX_STATUS_ERROR;

    /* Wait for the HW module to become available */
    SemaphoreP_pend(&CryptoResourceCC26XX_accessSemaphore, SemaphoreP_WAIT_FOREVER);

    /* KEY_ANY means first available kay starting from highest index will be used */
    if (keyLocation == CRYPTOCC26XX_KEY_ANY) {
        for (i = CRYPTOCC26XX_KEY_5; i >= 0 ; i--) {
            /* Search for first available key in store */
            if (!(object->keyStore & (1<<i))) {
                object->keyStore |= (1<<i);
                keyIndex = i;
                break;
            }
        }
    } else { /* If keyLocation is available, grab it. */
        if (!(object->keyStore & (1<<keyLocation))) {
            object->keyStore |= (1<<keyLocation);
            keyIndex = keyLocation;
        }
    }

    if ((keyIndex != CRYPTOCC26XX_STATUS_ERROR) && (keySrc != NULL)) {
        /* We are not allowed to switch to XOSC_HF while other bus transactions are ongoing */
        Power_setConstraint(PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING);
        /* Write key to crypto RAM */
        res = CRYPTOAesLoadKey((uint32_t*) keySrc, keyIndex);

        Power_releaseConstraint(PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING);

        /* If load key call failed (res != 0), reset keyIndex. */
        if(res) {
            object->keyStore &= ~(1<<keyIndex);
            keyIndex = CRYPTOCC26XX_STATUS_ERROR;
        }
    }

    /* Release semaphore */
    SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);

    return (keyIndex);
}

/*
 *  ======== CryptoCC26XX_loadKey ========
 */
int CryptoCC26XX_loadKey(CryptoCC26XX_Handle handle, int keyIndex, const uint32_t *keySrc){
    CryptoCC26XX_Object         *object;
    int                         loadKeyStatus = CRYPTOCC26XX_STATUS_ERROR;
    bool                        blockingAllowed;
    SemaphoreP_Status           semaphoreAcquired;

    object = handle->object;

    /* Blocking is allowed if we are in task context.
     * If blocking is allowed, we should block based on the timeout
     * and otherwise use NO_WAIT instead if in Hwi or Swi .
     */
    blockingAllowed = !(SwiP_inISR() || HwiP_inISR());

    /* Try to acquire the semaphore */
    semaphoreAcquired = SemaphoreP_pend(&CryptoResourceCC26XX_accessSemaphore, blockingAllowed ? object->timeout : SemaphoreP_NO_WAIT);

    /* Check if we acquired the semaphore */
    if(semaphoreAcquired == SemaphoreP_OK) {
        if(keyIndex != CRYPTOCC26XX_STATUS_ERROR){
            /* We are not allowed to switch to XOSC_HF while other bus transactions are ongoing */
            Power_setConstraint(PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING);
            /* Write key to crypto RAM */
            uint32_t tmpReturnVal = CRYPTOAesLoadKey((uint32_t*) keySrc, keyIndex);

            Power_releaseConstraint(PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING);

            if(tmpReturnVal == AES_SUCCESS){
                loadKeyStatus = CRYPTOCC26XX_STATUS_SUCCESS;
            }
        }
        /* Release semaphore */
        SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);
    }

    return (loadKeyStatus);
}


/*
 *  ======== CryptoCC26XX_transact ========
 */
int CryptoCC26XX_transact(CryptoCC26XX_Handle handle, CryptoCC26XX_Transaction *transaction)
{
    return cryptoTransactionExecute(handle, transaction, false);
}

/*
*  ======== CryptoCC26XX_transactPolling ========
*/
int CryptoCC26XX_transactPolling(CryptoCC26XX_Handle handle, CryptoCC26XX_Transaction *transaction)
{
    return cryptoTransactionExecute(handle, transaction, true);
}

/*
*  ======== CryptoCC26XX_transactBlocking ========
*/
int CryptoCC26XX_transactCallback(CryptoCC26XX_Handle handle, CryptoCC26XX_Transaction *transaction)
{
    return (CRYPTOCC26XX_STATUS_UNDEFINEDCMD);
}

/*
 *  ======== cryptoPostNotify ========
 *  This functions is called to notify the CRYPTO driver of an ongoing transition
 *  out of standby mode.
 *
 *  @pre    Function assumes that the CRYPTO handle (clientArg) is pointing to a
 *          hardware module which has already been opened.
 */
static int cryptoPostNotify(unsigned int eventType, uintptr_t eventArg, uintptr_t clientArg)
{
    CryptoCC26XX_Object   *object;
    /* Currently only subscribing to AWAKE_STANDBY notification, so if notified        */
    /* we are returning from standby. Reset the keyStore since the RAM content is lost */
    object = ((CryptoCC26XX_Handle) clientArg)->object;
    object->keyStore = 1 << 6 | 1 << 7;

    return Power_NOTIFYDONE;
}

/*
 *  ======== cryptoTransactionPend ========
 *  This function pends on a semaphore posted by the crypto hwi after the crypto operation is completed.
 */
static bool cryptoTransactionPend(CryptoCC26XX_Handle handle){
    CryptoCC26XX_Object   *object;
    object = handle->object;
    bool transactionCompleted = false;

    /* Pend on blocking mode semaphore and wait for Hwi to finish. */
    if (SemaphoreP_OK != SemaphoreP_pend(&CryptoResourceCC26XX_operationSemaphore, object->timeout))
    {
        /* Semaphore timed out */
        DebugP_log1("CryptoCC26XX:(%p) AES transaction timed out",
                   (((CryptoCC26XX_HWAttrs *)handle->hwAttrs)->baseAddr));
        /* Release constraint since transaction is done */
        Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        Power_releaseConstraint(PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING);
        /* Release semaphore */
        SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);
        transactionCompleted = false;
    }
    else{
        transactionCompleted = true;
    }
    return transactionCompleted;
}

/*
 *  ======== cryptoTransactionPoll ========
 *  This function polls a global variable that is set in the crypto hwi. This allows for a crypto transaction to be called from a different hwi.
 */
static bool cryptoTransactionPoll(void){
    /* Polling mode, wait for intterupt */
    do {
        CPUdelay(1);
    } while(!hwiIntFlag);
    return true;
}

/*
 *  ======== cryptoTransactionPoll ========
 *  This function handles all supported crypto modes and interfaces with driverlib to configure the hardware correctly.
 */
static int cryptoTransactionExecute(CryptoCC26XX_Handle handle, CryptoCC26XX_Transaction *transaction, bool polling){
    CryptoCC26XX_Object         *object;
    const CryptoCC26XX_HWAttrs  *hwAttrs;
    unsigned int                key;
    int                         res;
    uint8_t                     transactionCompleted;
    union {
        CryptoCC26XX_AESCCM_Transaction *aesccm;
        CryptoCC26XX_AESECB_Transaction *aesecb;
        CryptoCC26XX_AESCBC_Transaction *aescbc;
    } transUnion;

    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* Check if the crypto is active already (grab semaphore) */
    if (SemaphoreP_OK != SemaphoreP_pend(&CryptoResourceCC26XX_accessSemaphore,
                                         polling ? SemaphoreP_NO_WAIT : SemaphoreP_WAIT_FOREVER)) {
        DebugP_log0("CryptoCC26XX: CryptoCC26XX_transactPolling() was called when crypto module is already busy.");
        return AES_DMA_BSY;
    }

    /* Set interrupt flag to unhandled */
    hwiIntFlag = false;

    /* Set current transaction as head and tail */
    object->currentTransact = transaction;

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&CryptoResourceCC26XX_hwi, CryptoCC26XX_hwiIntFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_CRYPTO_RESULT_AVAIL_IRQ, hwAttrs->intPriority);

    if (polling) {
        /* Set mode of the transaction */
        transaction->mode = CRYPTOCC26XX_MODE_POLLING;
    }
    else {
        /* Set mode of the transaction */
        transaction->mode = CRYPTOCC26XX_MODE_BLOCKING;
        /* Set constraints to guarantee transaction */
        Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
        Power_setConstraint(PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING);
    }

    /* Check type field to decide which transact/operation to perform */
    /* TODO: To queue or not to queue? */
    switch (transaction->opType) {
#ifndef CRYPTOCC26XX_EXCLUDE_AES_CCM_ENCRYPT
        case CRYPTOCC26XX_OP_AES_CCM_ENCRYPT :
        case CRYPTOCC26XX_OP_AES_CCM_ENCRYPT_AAD_ONLY :
            /* Do some typecasting to get the right fields */
            transUnion.aesccm = (CryptoCC26XX_AESCCM_Transaction *) transaction;

            /*
             * Disable HWIs
             * Ensure no preemption issue with driverlib implementation
             * although it's already protected with semaphore.
             */
            key = HwiP_disable();

            /* Do the transaction/operation */
            res = CRYPTOCcmAuthEncrypt((transaction->opType) == CRYPTOCC26XX_OP_AES_CCM_ENCRYPT,
                                       transUnion.aesccm->authLength,
                                       (uint32_t*) transUnion.aesccm->nonce,
                                       (uint32_t*) transUnion.aesccm->msgIn,
                                       transUnion.aesccm->msgInLength,
                                       (uint32_t*) transUnion.aesccm->header,
                                       transUnion.aesccm->headerLength,
                                       transUnion.aesccm->keyIndex,
                                       transUnion.aesccm->fieldLength,
                                       true);

            /* Restore HWIs */
            HwiP_restore(key);

            /* If operation setup failed, break out and return error */
            if (res != AES_SUCCESS) {
                break;
            }

            transactionCompleted = polling ? cryptoTransactionPoll() : cryptoTransactionPend(handle);
            if(!transactionCompleted){
                return CRYPTOCC26XX_TIMEOUT;
            }

            /* Get CCM status */
            res = CRYPTOCcmAuthEncryptStatus();
            if(res == AES_SUCCESS) {
                res = CRYPTOCcmAuthEncryptResultGet(transUnion.aesccm->authLength,
                                                    (uint32_t*) transUnion.aesccm->msgOut);
            }
            /* CCM finished */
            break;
#endif
#ifndef CRYPTOCC26XX_EXCLUDE_AES_CCM_DECRYPT
        case CRYPTOCC26XX_OP_AES_CCM_DECRYPT :
        case CRYPTOCC26XX_OP_AES_CCM_DECRYPT_AAD_ONLY :
            /* Do some typecasting to get the right fields */
            transUnion.aesccm = (CryptoCC26XX_AESCCM_Transaction *) transaction;

            /*
             * Disable HWIs
             * Ensure no preemption issue with driverlib implementation
             * although it's already protected with semaphore.
             */
            key = HwiP_disable();

            /* Do the transaction/operation */
            res = CRYPTOCcmInvAuthDecrypt((transaction->opType) == CRYPTOCC26XX_OP_AES_CCM_DECRYPT,
                                          transUnion.aesccm->authLength,
                                          (uint32_t*) transUnion.aesccm->nonce,
                                          (uint32_t*) transUnion.aesccm->msgIn,
                                          transUnion.aesccm->msgInLength,
                                          (uint32_t*) transUnion.aesccm->header,
                                          transUnion.aesccm->headerLength,
                                          transUnion.aesccm->keyIndex,
                                          transUnion.aesccm->fieldLength,
                                          true);

            /* Restore HWIs */
            HwiP_restore(key);

            /* If operation setup failed, break out and return error */
            if (res != AES_SUCCESS){
                break;
            }

            transactionCompleted = polling ? cryptoTransactionPoll() : cryptoTransactionPend(handle);
            if(!transactionCompleted){
                return CRYPTOCC26XX_TIMEOUT;
            }

            /* Get CCMINV status */
            res = CRYPTOCcmInvAuthDecryptStatus();
            if(res == AES_SUCCESS)
            {
                res = CRYPTOCcmInvAuthDecryptResultGet(transUnion.aesccm->authLength,
                                                       (uint32_t*) transUnion.aesccm->msgIn,
                                                       transUnion.aesccm->msgInLength,
                                                       (uint32_t*) transUnion.aesccm->msgOut);
            }
            /* CCMINV finished */
            break;
#endif
#ifndef CRYPTOCC26XX_EXCLUDE_AES_ECB
        case CRYPTOCC26XX_OP_AES_ECB_ENCRYPT :
        case CRYPTOCC26XX_OP_AES_ECB_DECRYPT :
            /* Do some typecasting to get the right fields */
            transUnion.aesecb = (CryptoCC26XX_AESECB_Transaction *) transaction;

            /*
             * Disable HWIs
             * Ensure no preemption issue with driverlib implementation
             * although it's already protected with semaphore.
             */
            key = HwiP_disable();

            /* Do the transaction/operation */
            res = CRYPTOAesEcb(transUnion.aesecb->msgIn,
                               transUnion.aesecb->msgOut,
                               transUnion.aesecb->keyIndex,
                               (transaction->opType) == CRYPTOCC26XX_OP_AES_ECB_ENCRYPT,
                               true);

            /* Restore HWIs */
            HwiP_restore(key);

            /* If operation setup failed, break out and return error */
            if (res != AES_SUCCESS){
                break;
            }

            transactionCompleted = polling ? cryptoTransactionPoll() : cryptoTransactionPend(handle);
            if(!transactionCompleted){
                return CRYPTOCC26XX_TIMEOUT;
            }

            /* Get ECB status */
            res = CRYPTOAesEcbStatus();
            CRYPTOAesEcbFinish();
            /* ECB finished */
            break;
#endif
#ifndef CRYPTOCC26XX_EXCLUDE_AES_CBC
        case CRYPTOCC26XX_OP_AES_CBC_ENCRYPT:
        case CRYPTOCC26XX_OP_AES_CBC_DECRYPT:
            /* Do some typecasting to get the right fields */
            transUnion.aescbc = (CryptoCC26XX_AESCBC_Transaction *) transaction;

            /*
             * Disable HWIs
             * Ensure no preemption issue with driverlib implementation
             * although it's already protected with semaphore.
             */
            key = HwiP_disable();

            /* Do the transaction/operation */
            res = CRYPTOAesCbc(transUnion.aescbc->msgIn,
                               transUnion.aescbc->msgOut,
                               transUnion.aescbc->msgInLength,
                               transUnion.aescbc->nonce,
                               transUnion.aescbc->keyIndex,
                               (transaction->opType) == CRYPTOCC26XX_OP_AES_CBC_ENCRYPT,
                               true);

            /* Restore HWIs */
            HwiP_restore(key);

            /* If operation setup failed, break out and return error */
            if (res != AES_SUCCESS){
                break;
            }

            transactionCompleted = polling ? cryptoTransactionPoll() : cryptoTransactionPend(handle);
            if(!transactionCompleted){
                return CRYPTOCC26XX_TIMEOUT;
            }

            /* Get CBC status */
            res = CRYPTOAesCbcStatus();
            CRYPTOAesCbcFinish();
            /* CBC finished */
            break;
#endif
        default :
            DebugP_log1("CryptoCC26XX: Could not recognize transaction (%p).",
                         (transaction->opType));
            res = CRYPTOCC26XX_STATUS_ERROR;
    }

    /* Release constraint since transaction is done */
    if(!polling){
        Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        Power_releaseConstraint(PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING);
    }
    /* Release semaphore */
    SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);
    return (res);
}
