/*
 * Copyright (c) 2017-2018, Texas Instruments Incorporated
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
#include <ti/drivers/dpl/DebugP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/ECDH.h>
#include <ti/drivers/ecdh/ECDHCC26X2.h>
#include <ti/drivers/cryptoutils/sharedresources/PKAResourceCC26XX.h>

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
static void ECDHCC26X2_hwiFxn (uintptr_t arg0);
static void ECDHCC26X2_internalCallbackFxn (ECDH_Handle handle,
                                               int_fast16_t returnStatus,
                                               ECDH_Operation operation,
                                               ECDH_OperationType operationType);
static int_fast16_t ECDHCC26X2_waitForAccess(ECDH_Handle handle);
static int_fast16_t ECDHCC26X2_waitForResult(ECDH_Handle handle);
static int_fast16_t ECDHCC26X2_runFSM(ECDH_Handle handle);
static int_fast16_t ECDHCC26X2_convertReturnValue(uint32_t pkaResult);

/* Extern globals */
extern const ECDH_Config ECDH_config[];
extern const uint_least8_t ECDH_count;

/* Static globals */
static bool isInitialized = false;
static uint32_t resultAddress;

/*
 *  ======== ECDHCC26X2_internalCallbackFxn ========
 */
static void ECDHCC26X2_internalCallbackFxn (ECDH_Handle handle,
                                             int_fast16_t returnStatus,
                                             ECDH_Operation operation,
                                             ECDH_OperationType operationType) {
    ECDHCC26X2_Object *object = handle->object;

    /* This function is only ever registered when in ECDH_RETURN_BEHAVIOR_BLOCKING
     * or ECDH_RETURN_BEHAVIOR_POLLING.
     */
    if (object->returnBehavior == ECDH_RETURN_BEHAVIOR_BLOCKING) {
        SemaphoreP_post(&PKAResourceCC26XX_operationSemaphore);
    }
    else {
        PKAResourceCC26XX_pollingFlag = 1;
    }
}

/*
 *  ======== ECDHCC26X2_hwiFxn ========
 */
static void ECDHCC26X2_hwiFxn (uintptr_t arg0) {
    ECDHCC26X2_Object *object = ((ECDH_Handle)arg0)->object;
    int_fast16_t operationStatus;
    ECDH_Operation operation;
    ECDH_OperationType operationType;
    uint32_t key;

    /* Disable interrupt again. It may be reenabled in the FSM function. */
    IntDisable(INT_PKA_IRQ);

    /* Execute next states */
    do {
        object->operationStatus = ECDHCC26X2_runFSM((ECDH_Handle)arg0);
        object->fsmState++;
    } while (object->operationStatus == ECDHCC26X2_STATUS_FSM_RUN_FSM);

    /* We need a critical section here in case the operation is canceled
     * asynchronously.
     */
    key = HwiP_disable();

    if(object->operationCanceled) {
        /* Set function register to 0. This should stop the current operation */
        HWREG(PKA_BASE + PKA_O_FUNCTION) = 0;

        object->operationStatus = ECDH_STATUS_CANCELED;
    }

    switch (object->operationStatus) {
        case ECDHCC26X2_STATUS_FSM_RUN_PKA_OP:

            HwiP_restore(key);

            /* Do nothing. The PKA or TRNG hardware
             * will execute in the background and post
             * this SWI when it is done.
             */
            break;
        case ECDH_STATUS_SUCCESS:
            /* Intentional fall through */
        case ECDH_STATUS_ERROR:
            /* Intentional fall through */
        case ECDH_STATUS_CANCELED:
            /* Intentional fall through */
        default:

            /* Mark this operation as complete */
            object->operationInProgress = false;

            /* Clear any pending interrupt in case a transaction kicked off
             * above already finished
             */
            IntDisable(INT_PKA_IRQ);
            IntPendClear(INT_PKA_IRQ);

            /* We can end the critical section since the operation may no
             * longer be canceled
             */
            HwiP_restore(key);

            /* Make sure there is no keying material remaining in PKA RAM */
            PKAClearPkaRam();

            /* Save all inputs to the callbackFxn on the stack
             * in case a higher priority hwi comes in and
             * starts a new operation after we have released the
             * access semaphore.
             */
            operationStatus     = object->operationStatus;
            operation           = object->operation;
            operationType       = object->operationType;

            Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

            /*  Grant access for other threads to use the crypto module.
             *  The semaphore must be posted before the callbackFxn to allow the chaining
             *  of operations. This does have the drawback that another hwi
             *  can come in and start an operation before the original
             *  on finished completely. This should be prevented by
             *  customers only starting operations with the same
             *  handle from a single context and waiting for
             *  the callback of the original operation to
             *  be executed in callback return mode.
             */
            SemaphoreP_post(&PKAResourceCC26XX_accessSemaphore);


            object->callbackFxn((ECDH_Handle)arg0,
                                operationStatus,
                                operation,
                                operationType);
    }
}

int_fast16_t ECDHCC26X2_runFSM(ECDH_Handle handle) {
    ECDHCC26X2_Object *object = handle->object;
    uint32_t pkaResult;

    switch (object->fsmState) {
        case ECDHCC26X2_FSM_GEN_PUB_KEY_VALIDATE_PRIVATE_KEY:
            /* We need to verify that private key in [1, n] for arbitrary short Weierstrass curves. */
            if (PKAArrayAllZeros(object->operation.generatePublicKey->myPrivateKey->u.plaintext.keyMaterial,
                                 object->operation.generatePublicKey->curve->length)) {
                return ECDH_STATUS_PRIVATE_KEY_ZERO;
            }

            PKABigNumCmpStart(object->operation.generatePublicKey->myPrivateKey->u.plaintext.keyMaterial,
                              object->operation.generatePublicKey->curve->order,
                              object->operation.generatePublicKey->curve->length);

            while(PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY);

            pkaResult = PKABigNumCmpGetResult();

            return ECDHCC26X2_convertReturnValue(pkaResult);

        case ECDHCC26X2_FSM_GEN_PUB_KEY_MULT_PRIVATE_KEY_BY_GENERATOR:

            /* Perform an elliptic curve multiplication on a short Weierstrass curve */
            PKAEccMultiplyStart(object->operation.generatePublicKey->myPrivateKey->u.plaintext.keyMaterial,
                                object->operation.generatePublicKey->curve->generatorX,
                                object->operation.generatePublicKey->curve->generatorY,
                                object->operation.generatePublicKey->curve->prime,
                                object->operation.generatePublicKey->curve->a,
                                object->operation.generatePublicKey->curve->b,
                                object->operation.generatePublicKey->curve->length,
                                &resultAddress);

            break;

        case ECDHCC26X2_FSM_GEN_PUB_KEY_MULT_PRIVATE_KEY_BY_GENERATOR_RESULT:

            /* Get X and Y coordinates for short Weierstrass curves */
            pkaResult = PKAEccMultiplyGetResult(object->operation.generatePublicKey->myPublicKey->u.plaintext.keyMaterial,
                                                object->operation.generatePublicKey->myPublicKey->u.plaintext.keyMaterial
                                                    + object->operation.generatePublicKey->curve->length,
                                                resultAddress,
                                                object->operation.generatePublicKey->curve->length);

            return ECDHCC26X2_convertReturnValue(pkaResult);

        case ECDHCC26X2_FSM_GEN_PUB_KEY_MULT_PRIVATE_KEY_BY_GENERATOR_MONTGOMERY:

            /* Perform an elliptic curve multiplication on a Montgomery curve. Likely Curve25519. */
            PKAEccMontgomeryMultiplyStart(object->operation.generatePublicKey->myPrivateKey->u.plaintext.keyMaterial,
                                          object->operation.generatePublicKey->curve->generatorX,
                                          object->operation.generatePublicKey->curve->prime,
                                          object->operation.generatePublicKey->curve->a,
                                          object->operation.generatePublicKey->curve->length,
                                          &resultAddress);


            break;

        case ECDHCC26X2_FSM_GEN_PUB_KEY_VALIDATE_PRIVATE_KEY_MONTGOMERY:
            /* Curve25519 private keys must be formatted according to cr.yp.to/ecdh.html.
             * Since the keying material may not be altered (because the array is in flash e.g.),
             * we need to reject any non-conforming private keys.
             */
            if (object->operation.generatePublicKey->myPrivateKey->u.plaintext.keyMaterial[0] & 0x07 ||
                object->operation.generatePublicKey->myPrivateKey->u.plaintext.keyMaterial[31] & 0x80 ||
                !(object->operation.generatePublicKey->myPrivateKey->u.plaintext.keyMaterial[31] & 0x40)) {
                /* If the bottom three bits or the top bit are set or the second to last bit is not set,
                 * throw an error.
                 */
                return ECDH_STATUS_ERROR;
            }
            else {
                return ECDHCC26X2_STATUS_FSM_RUN_FSM;
            }

        case ECDHCC26X2_FSM_GEN_PUB_KEY_MULT_PRIVATE_KEY_BY_GENERATOR_RESULT_MONTGOMERY:

            /* The PKA hw only returns the X coordinate for Montgomery multiplications. This is fine for Curve25519 */
            pkaResult = PKAEccMultiplyGetResult(object->operation.generatePublicKey->myPublicKey->u.plaintext.keyMaterial,
                                                NULL,
                                                resultAddress,
                                                object->operation.generatePublicKey->curve->length);

            /* Zero-out the Y coordinate */
            memset(object->operation.generatePublicKey->myPublicKey->u.plaintext.keyMaterial
                    + object->operation.generatePublicKey->curve->length,
                   0x00,
                   object->operation.generatePublicKey->curve->length);

            return ECDHCC26X2_convertReturnValue(pkaResult);

        case ECDHCC26X2_FSM_COMPUTE_SHARED_SECRET_VALIDATE_PUB_KEY:

            /* If we are using a short Weierstrass curve, we need to validate the public key */
            pkaResult = PKAEccVerifyPublicKeyWeierstrassStart(object->operation.computeSharedSecret->theirPublicKey->u.plaintext.keyMaterial,
                                                              object->operation.computeSharedSecret->theirPublicKey->u.plaintext.keyMaterial
                                                                + object->operation.computeSharedSecret->curve->length,
                                                              object->operation.computeSharedSecret->curve->prime,
                                                              object->operation.computeSharedSecret->curve->a,
                                                              object->operation.computeSharedSecret->curve->b,
                                                              object->operation.computeSharedSecret->curve->order,
                                                              object->operation.computeSharedSecret->curve->length);

            /* Break out early since no PKA operation was started by the verify fxn */
            return ECDHCC26X2_convertReturnValue(pkaResult);


        case ECDHCC26X2_FSM_COMPUTE_SHARED_SECRET_MULT_PRIVATE_KEY_BY_PUB_KEY:

                /* Perform an elliptic curve multiplication on a short Weierstrass curve */
                PKAEccMultiplyStart(object->operation.computeSharedSecret->myPrivateKey->u.plaintext.keyMaterial,
                                    object->operation.computeSharedSecret->theirPublicKey->u.plaintext.keyMaterial,
                                    object->operation.computeSharedSecret->theirPublicKey->u.plaintext.keyMaterial
                                        + object->operation.computeSharedSecret->curve->length,
                                    object->operation.computeSharedSecret->curve->prime,
                                    object->operation.computeSharedSecret->curve->a,
                                    object->operation.computeSharedSecret->curve->b,
                                    object->operation.computeSharedSecret->curve->length,
                                    &resultAddress);


            break;

        case ECDHCC26X2_FSM_COMPUTE_SHARED_SECRET_MULT_PRIVATE_KEY_BY_PUB_KEY_RESULT:

            /* Get X and Y coordinates for short Weierstrass curves */
            pkaResult = PKAEccMultiplyGetResult(object->operation.computeSharedSecret->sharedSecret->u.plaintext.keyMaterial,
                                                object->operation.computeSharedSecret->sharedSecret->u.plaintext.keyMaterial
                                                    + object->operation.computeSharedSecret->curve->length,
                                                resultAddress,
                                                object->operation.computeSharedSecret->curve->length);


            return ECDHCC26X2_convertReturnValue(pkaResult);

        case ECDHCC26X2_FSM_COMPUTE_SHARED_SECRET_MULT_PRIVATE_KEY_BY_PUB_KEY_MONTGOMERY:

            /* Perform an elliptic curve multiplication on a Montgomery curve. Likely Curve25519. */
            PKAEccMontgomeryMultiplyStart(object->operation.computeSharedSecret->myPrivateKey->u.plaintext.keyMaterial,
                                          object->operation.computeSharedSecret->theirPublicKey->u.plaintext.keyMaterial,
                                          object->operation.computeSharedSecret->curve->prime,
                                          object->operation.computeSharedSecret->curve->a,
                                          object->operation.computeSharedSecret->curve->length,
                                          &resultAddress);

            break;

        case ECDHCC26X2_FSM_COMPUTE_SHARED_SECRET_MULT_PRIVATE_KEY_BY_PUB_KEY_RESULT_MONTGOMERY:

            /* The PKA hw only returns the X coordinate for Montgomery multiplications. This is fine for Curve25519 */
            pkaResult = PKAEccMultiplyGetResult(object->operation.computeSharedSecret->sharedSecret->u.plaintext.keyMaterial,
                                                NULL,
                                                resultAddress,
                                                object->operation.computeSharedSecret->curve->length);

            /* Zero-out the Y coordinate */
            memset(object->operation.computeSharedSecret->sharedSecret->u.plaintext.keyMaterial
                    + object->operation.computeSharedSecret->curve->length,
                   0x00,
                   object->operation.computeSharedSecret->curve->length);


            return ECDHCC26X2_convertReturnValue(pkaResult);

        case ECDHCC26X2_FSM_GEN_PUB_KEY_RETURN:
        case ECDHCC26X2_FSM_GEN_PUB_KEY_RETURN_MONTGOMERY:
        case ECDHCC26X2_FSM_COMPUTE_SHARED_SECRET_RETURN:
        case ECDHCC26X2_FSM_COMPUTE_SHARED_SECRET_RETURN_MONTGOMERY:
            return ECDH_STATUS_SUCCESS;
        default:
            return ECDH_STATUS_ERROR;
    }

    // If we get to this point, we want to perform another PKA operation
    IntPendClear(INT_PKA_IRQ);
    IntEnable(INT_PKA_IRQ);

    return ECDHCC26X2_STATUS_FSM_RUN_PKA_OP;
}

/*
 *  ======== ECDHCC26X2_convertReturnValue ========
 */
static int_fast16_t ECDHCC26X2_convertReturnValue(uint32_t pkaResult) {
    switch (pkaResult) {
        case PKA_STATUS_SUCCESS:
        case PKA_STATUS_A_LESS_THAN_B:
            /* A less than B only comes up when checking private
             * key values. It indicates a key within the correct range.
             */
            return ECDHCC26X2_STATUS_FSM_RUN_FSM;

        case PKA_STATUS_A_GREATER_THAN_B:
        case PKA_STATUS_EQUAL:
            /* This indicates a private key >= n which is not permitted. */
            return ECDH_STATUS_PRIVATE_KEY_LARGER_EQUAL_ORDER;

        case PKA_STATUS_X_ZERO:
        case PKA_STATUS_Y_ZERO:
        case PKA_STATUS_RESULT_0:
            /* Theoretically, PKA_STATUS_RESULT_0 might be caused by other
             * operations failing but the only one that really should yield
             * 0 is ECC multiplication with invalid inputs that yield the
             * point at infinity.
             */
            return ECDH_STATUS_POINT_AT_INFINITY;

        case PKA_STATUS_X_LARGER_THAN_PRIME:
        case PKA_STATUS_Y_LARGER_THAN_PRIME:
            return ECDH_STATUS_PUBLIC_KEY_LARGER_THAN_PRIME;

        case PKA_STATUS_POINT_NOT_ON_CURVE:
            return ECDH_STATUS_PUBLIC_KEY_NOT_ON_CURVE;

        default:
            return ECDH_STATUS_ERROR;
    }
}

/*
 *  ======== ECDH_init ========
 */
void ECDH_init(void) {
    PKAResourceCC26XX_constructRTOSObjects();

    isInitialized = true;
}

/*
 *  ======== ECDH_Params_init ========
 */
void ECDH_Params_init(ECDH_Params *params){
    *params = ECDH_defaultParams;
}

/*
 *  ======== ECDH_open ========
 */
ECDH_Handle ECDH_open(uint_least8_t index, ECDH_Params *params) {
    ECDH_Handle                  handle;
    ECDHCC26X2_Object           *object;
    uint_fast8_t                key;

    handle = (ECDH_Handle)&(ECDH_config[index]);
    object = handle->object;

    DebugP_assert(index < ECDH_count);

    key = HwiP_disable();

    if (!isInitialized ||  object->isOpen) {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    // If params are NULL, use defaults
    if (params == NULL) {
        params = (ECDH_Params *)&ECDH_defaultParams;
    }

    DebugP_assert((params->returnBehavior == ECDH_RETURN_BEHAVIOR_CALLBACK) ? params->callbackFxn : true);

    object->returnBehavior = params->returnBehavior;
    object->callbackFxn = params->returnBehavior == ECDH_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : ECDHCC26X2_internalCallbackFxn;
    object->semaphoreTimeout = params->timeout;

    // Set power dependency - i.e. power up and enable clock for PKA (PKAResourceCC26XX) module.
    Power_setDependency(PowerCC26X2_PERIPH_PKA);

    return handle;
}

/*
 *  ======== ECDH_close ========
 */
void ECDH_close(ECDH_Handle handle) {
    ECDHCC26X2_Object         *object;

    DebugP_assert(handle);

    // Get the pointer to the object
    object = handle->object;

    // Mark the module as available
    object->isOpen = false;

    // Release power dependency on PKA Module.
    Power_releaseDependency(PowerCC26X2_PERIPH_PKA);
}


/*
 *  ======== ECDHCC26X2_waitForAccess ========
 */
static int_fast16_t ECDHCC26X2_waitForAccess(ECDH_Handle handle) {
    ECDHCC26X2_Object *object = handle->object;
    uint32_t timeout;

    // Set to SemaphoreP_NO_WAIT to start operations from SWI or HWI context
    timeout = object->returnBehavior == ECDH_RETURN_BEHAVIOR_BLOCKING ? object->semaphoreTimeout : SemaphoreP_NO_WAIT;

    return SemaphoreP_pend(&PKAResourceCC26XX_accessSemaphore, timeout);
}

/*
 *  ======== ECDHCC26X2_waitForResult ========
 */
static int_fast16_t ECDHCC26X2_waitForResult(ECDH_Handle handle){
    ECDHCC26X2_Object *object = handle->object;

    object->operationInProgress = true;

    switch (object->returnBehavior) {
        case ECDH_RETURN_BEHAVIOR_POLLING:
            while(!PKAResourceCC26XX_pollingFlag);
            return object->operationStatus;
        case ECDH_RETURN_BEHAVIOR_BLOCKING:
            SemaphoreP_pend(&PKAResourceCC26XX_operationSemaphore, SemaphoreP_WAIT_FOREVER);
            return object->operationStatus;
        case ECDH_RETURN_BEHAVIOR_CALLBACK:
            return ECDH_STATUS_SUCCESS;
        default:
            return ECDH_STATUS_ERROR;
    }
}

/*
 *  ======== ECDH_generatePublicKey ========
 */
int_fast16_t ECDH_generatePublicKey(ECDH_Handle handle, ECDH_OperationGeneratePublicKey *operation) {
    ECDHCC26X2_Object *object              = handle->object;
    ECDHCC26X2_HWAttrs const *hwAttrs      = handle->hwAttrs;

    if (ECDHCC26X2_waitForAccess(handle) != SemaphoreP_OK) {
        return ECDH_STATUS_RESOURCE_UNAVAILABLE;
    }

    /* Copy over all parameters we will need access to in the FSM.
     * The FSM runs in SWI context and thus needs to keep track of
     * all of them somehow.
     */
    object->operationStatus                 = ECDHCC26X2_STATUS_FSM_RUN_FSM;
    object->operation.generatePublicKey     = operation;
    object->operationType                   = ECDH_OPERATION_TYPE_GENERATE_PUBLIC_KEY;
    object->operationCanceled               = false;

    /* Use the correct state chain for the curve type */
    if (operation->curve->curveType == ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS) {
        object->fsmState = ECDHCC26X2_FSM_GEN_PUB_KEY_VALIDATE_PRIVATE_KEY;
    }
    else {
        object->fsmState = ECDHCC26X2_FSM_GEN_PUB_KEY_VALIDATE_PRIVATE_KEY_MONTGOMERY;
    }


    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&PKAResourceCC26XX_hwi, ECDHCC26X2_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_PKA_IRQ, hwAttrs->intPriority);

    PKAResourceCC26XX_pollingFlag = 0;

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* Start running FSM to generate public key. The PKA interrupt is level triggered and
     * will run imediately once enabled
     */
    IntEnable(INT_PKA_IRQ);

    return ECDHCC26X2_waitForResult(handle);
}

/*
 *  ======== ECDH_computeSharedSecret ========
 */
int_fast16_t ECDH_computeSharedSecret(ECDH_Handle handle, ECDH_OperationComputeSharedSecret *operation) {
    ECDHCC26X2_Object *object              = handle->object;
    ECDHCC26X2_HWAttrs const *hwAttrs      = handle->hwAttrs;

    if (ECDHCC26X2_waitForAccess(handle) != SemaphoreP_OK) {
        return ECDH_STATUS_RESOURCE_UNAVAILABLE;
    }

    /* Copy over all parameters we will need access to in the FSM.
     * The FSM runs in SWI context and thus needs to keep track of
     * all of them somehow.
     */
    object->operationStatus                 = ECDHCC26X2_STATUS_FSM_RUN_FSM;
    object->operation.computeSharedSecret   = operation;
    object->operationType                   = ECDH_OPERATION_TYPE_COMPUTE_SHARED_SECRET;
    object->operationCanceled               = false;


    /* Use the correct state chain for the curve type */
    if (operation->curve->curveType == ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS) {
        object->fsmState = ECDHCC26X2_FSM_COMPUTE_SHARED_SECRET_VALIDATE_PUB_KEY;
    }
    else {
        object->fsmState = ECDHCC26X2_FSM_COMPUTE_SHARED_SECRET_MULT_PRIVATE_KEY_BY_PUB_KEY_MONTGOMERY;
    }


    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&PKAResourceCC26XX_hwi, ECDHCC26X2_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_PKA_IRQ, hwAttrs->intPriority);

    PKAResourceCC26XX_pollingFlag = 0;

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* Start running FSM to generate PMSN. The PKA interrupt is level triggered and
     * will run imediately once enabled
     */
    IntEnable(INT_PKA_IRQ);

    return ECDHCC26X2_waitForResult(handle);
}

/*
 *  ======== ECDH_cancelOperation ========
 */
int_fast16_t ECDH_cancelOperation(ECDH_Handle handle) {
    ECDHCC26X2_Object *object = handle->object;

    if(!object->operationInProgress){
        return ECDH_STATUS_ERROR;
    }

    object->operationCanceled = true;

    /* Post hwi as if operation finished for cleanup */
    IntEnable(INT_PKA_IRQ);
    HwiP_post(INT_PKA_IRQ);


    return ECDH_STATUS_SUCCESS;
}
