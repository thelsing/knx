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
#include <ti/drivers/dpl/SwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/DebugP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/ECDSA.h>
#include <ti/drivers/ecdsa/ECDSACC26X2.h>
#include <ti/drivers/cryptoutils/sharedresources/PKAResourceCC26XX.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_pka.h)
#include DeviceFamily_constructPath(inc/hw_pka_ram.h)
#include DeviceFamily_constructPath(driverlib/pka.h)
#include DeviceFamily_constructPath(driverlib/cpu.h)
#include DeviceFamily_constructPath(driverlib/interrupt.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)

#define ECDSACC26X2_SCRATCH_BUFFER_SIZE 256

/* Since we only support <= 384-bit curves, we can allocate two temporary buffers
 * at the midway address in PKA_RAM. This leaves us with 1024 bytes of RAM
 * to work with. That should be sufficient for our needs. Multiplications will
 * take up to 3x the curve param length of 384 bits or 48 bytes. ECC addition
 * requires 8x the curve param length in RAM. 1024 bytes would even suffice
 * for 384-bit curves.
 */
#define ECDSACC26X2_SCRATCH_BUFFER ((uint8_t *)(PKA_RAM_BASE + PKA_RAM_TOT_BYTE_SIZE / 2))
#define ECDSACC26X2_SCRATCH_BUFFER_2 ((uint8_t *)(PKA_RAM_BASE + PKA_RAM_TOT_BYTE_SIZE / 2 + ECDSACC26X2_SCRATCH_BUFFER_SIZE))

/* Forward declarations */
static void ECDSACC26X2_hwiFxn (uintptr_t arg0);
static void ECDSACC26X2_internalCallbackFxn (ECDSA_Handle handle,
                                             int_fast16_t returnStatus,
                                             ECDSA_Operation operation,
                                             ECDSA_OperationType operationType);
static int_fast16_t ECDSACC26X2_waitForAccess(ECDSA_Handle handle);
static int_fast16_t ECDSACC26X2_waitForResult(ECDSA_Handle handle);
static int_fast16_t ECDSACC26X2_runSignFSM(ECDSA_Handle handle);
static int_fast16_t ECDSACC26X2_runVerifyFSM(ECDSA_Handle handle);
static int_fast16_t ECDSACC26X2_convertReturnValue(uint32_t pkaResult);

/* Extern globals */
extern const ECDSA_Config ECDSA_config[];
extern const uint_least8_t ECDSA_count;
extern const ECDSA_Params ECDSA_defaultParams;

/* Static globals */
static bool isInitialized = false;
static uint32_t resultAddress;

static uint32_t scratchBufferSize = ECDSACC26X2_SCRATCH_BUFFER_SIZE;
static uint32_t scratchBuffer2Size = ECDSACC26X2_SCRATCH_BUFFER_SIZE;


/*
 *  ======== ECDSACC26X2_internalCallbackFxn ========
 */
static void ECDSACC26X2_internalCallbackFxn (ECDSA_Handle handle,
                                             int_fast16_t returnStatus,
                                             ECDSA_Operation operation,
                                             ECDSA_OperationType operationType) {
    ECDSACC26X2_Object *object = handle->object;

    /* This function is only ever registered when in ECDSA_RETURN_BEHAVIOR_BLOCKING
     * or ECDSA_RETURN_BEHAVIOR_POLLING.
     */
    if (object->returnBehavior == ECDSA_RETURN_BEHAVIOR_BLOCKING) {
        SemaphoreP_post(&PKAResourceCC26XX_operationSemaphore);
    }
    else {
        PKAResourceCC26XX_pollingFlag = 1;
    }
}

/*
 *  ======== ECDSACC26X2_hwiFxn ========
 */
static void ECDSACC26X2_hwiFxn (uintptr_t arg0) {
    ECDSACC26X2_Object *object = ((ECDSA_Handle)arg0)->object;
    uint32_t key;

    /* Disable interrupt again */
    IntDisable(INT_PKA_IRQ);

    /* Execute next states */
    do {
        object->operationStatus = object->fsmFxn((ECDSA_Handle)arg0);
        object->fsmState++;
    } while (object->operationStatus == ECDSACC26X2_STATUS_FSM_RUN_FSM);

    /* We need a critical section here in case the operation is canceled
     * asynchronously.
     */
    key = HwiP_disable();

    if(object->operationCanceled) {
        /* Set function register to 0. This should stop the current operation */
        HWREG(PKA_BASE + PKA_O_FUNCTION) = 0;

        object->operationStatus = ECDSA_STATUS_CANCELED;
    }

    switch (object->operationStatus) {
        case ECDSACC26X2_STATUS_FSM_RUN_PKA_OP:

            HwiP_restore(key);

            /* Do nothing. The PKA hardware
             * will execute in the background and post
             * this SWI when it is done.
             */
            break;
        case ECDSA_STATUS_SUCCESS:
            /* Intentional fall through */
        case ECDSA_STATUS_ERROR:
            /* Intentional fall through */
        case ECDSA_STATUS_CANCELED:
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

            /*  Grant access for other threads to use the crypto module.
             *  The semaphore must be posted before the callbackFxn to allow the chaining
             *  of operations.
             */
            SemaphoreP_post(&PKAResourceCC26XX_accessSemaphore);

            Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

            object->callbackFxn((ECDSA_Handle)arg0,
                                object->operationStatus,
                                object->operation,
                                object->operationType);
    }
}

/*
 *  ======== ECDSACC26X2_runSignFSM ========
 */
static int_fast16_t ECDSACC26X2_runSignFSM(ECDSA_Handle handle) {
    ECDSACC26X2_Object *object = handle->object;
    uint32_t pkaResult;

    switch (object->fsmState) {
        case ECDSACC26X2_FSM_SIGN_VALIDATE_PMSN:
            if (PKAArrayAllZeros(object->operation.sign->pmsn->u.plaintext.keyMaterial,
                                 object->operation.sign->curve->length)) {
                return ECDSA_STATUS_INVALID_PMSN;
            }

            PKABigNumCmpStart(object->operation.sign->pmsn->u.plaintext.keyMaterial,
                              object->operation.sign->curve->order,
                              object->operation.sign->curve->length);

            while(PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY);

            pkaResult = PKABigNumCmpGetResult();

            if (pkaResult != PKA_STATUS_A_LESS_THAN_B) {
                return ECDSA_STATUS_INVALID_PMSN;
            }
            else {
                return ECDSACC26X2_STATUS_FSM_RUN_FSM;
            }

        case ECDSACC26X2_FSM_SIGN_COMPUTE_R:

            PKAEccMultiplyStart(object->operation.sign->pmsn->u.plaintext.keyMaterial,
                                object->operation.sign->curve->generatorX,
                                object->operation.sign->curve->generatorY,
                                object->operation.sign->curve->prime,
                                object->operation.sign->curve->a,
                                object->operation.sign->curve->b,
                                object->operation.sign->curve->length,
                                &resultAddress);

            break;

        case ECDSACC26X2_FSM_SIGN_COMPUTE_R_RESULT:

            pkaResult = PKAEccMultiplyGetResult(object->operation.sign->r,
                                                NULL,
                                                resultAddress,
                                                object->operation.sign->curve->length);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_SIGN_R_MOD_N:

            PKABigNumModStart(object->operation.sign->r,
                              object->operation.sign->curve->length,
                              object->operation.sign->curve->order,
                              object->operation.sign->curve->length,
                              &resultAddress);

            break;

        case ECDSACC26X2_FSM_SIGN_R_MOD_N_RESULT:

            pkaResult = PKABigNumModGetResult(object->operation.sign->r,
                                              object->operation.sign->curve->length,
                                              resultAddress);


            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_SIGN_COMPUTE_PMSN_INVERSE:

            PKABigNumInvModStart(object->operation.sign->pmsn->u.plaintext.keyMaterial,
                                 object->operation.sign->curve->length,
                                 object->operation.sign->curve->order,
                                 object->operation.sign->curve->length,
                                 &resultAddress);

            break;

        case ECDSACC26X2_FSM_SIGN_COMPUTE_PMSN_INVERSE_RESULT:

            pkaResult = PKABigNumInvModGetResult(object->operation.sign->s,
                                                 object->operation.sign->curve->length,
                                                 resultAddress);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_SIGN_COMPUTE_PRIVATE_KEY_X_R:

            PKABigNumMultiplyStart(object->operation.sign->myPrivateKey->u.plaintext.keyMaterial,
                                   object->operation.sign->curve->length,
                                   object->operation.sign->r,
                                   object->operation.sign->curve->length,
                                   &resultAddress);

            break;

        case ECDSACC26X2_FSM_SIGN_COMPUTE_PRIVATE_KEY_X_R_RESULT:

            scratchBufferSize = ECDSACC26X2_SCRATCH_BUFFER_SIZE;

            pkaResult = PKABigNumMultGetResult(ECDSACC26X2_SCRATCH_BUFFER,
                                               &scratchBufferSize,
                                               resultAddress);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_SIGN_ADD_HASH:

            PKABigNumAddStart(ECDSACC26X2_SCRATCH_BUFFER,
                              scratchBufferSize,
                              object->operation.sign->hash,
                              object->operation.sign->curve->length,
                              &resultAddress);

            break;

        case ECDSACC26X2_FSM_SIGN_ADD_HASH_RESULT:

            scratchBufferSize = ECDSACC26X2_SCRATCH_BUFFER_SIZE;

            pkaResult = PKABigNumAddGetResult(ECDSACC26X2_SCRATCH_BUFFER,
                                              &scratchBufferSize,
                                              resultAddress);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_SIGN_MULT_BY_PMSN_INVERSE:

            PKABigNumMultiplyStart(ECDSACC26X2_SCRATCH_BUFFER,
                                   scratchBufferSize,
                                   object->operation.sign->s,
                                   object->operation.sign->curve->length,
                                   &resultAddress);

            break;


        case ECDSACC26X2_FSM_SIGN_MULT_BY_PMSN_INVERSE_RESULT:

            scratchBufferSize = ECDSACC26X2_SCRATCH_BUFFER_SIZE;

            pkaResult = PKABigNumMultGetResult(ECDSACC26X2_SCRATCH_BUFFER,
                                               &scratchBufferSize,
                                               resultAddress);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_SIGN_MOD_N:

            PKABigNumModStart(ECDSACC26X2_SCRATCH_BUFFER,
                              scratchBufferSize,
                              object->operation.sign->curve->order,
                              object->operation.sign->curve->length,
                              &resultAddress);

            break;

        case ECDSACC26X2_FSM_SIGN_MOD_N_RESULT:

            pkaResult = PKABigNumModGetResult(object->operation.sign->s,
                                              object->operation.sign->curve->length,
                                              resultAddress);

            if (pkaResult == PKA_STATUS_SUCCESS) {
                return ECDSA_STATUS_SUCCESS;
            }
            else {
                return ECDSA_STATUS_ERROR;
            }

        default:
            return ECDSA_STATUS_ERROR;
    }

    // If we get to this point, we want to perform another PKA operation
    IntPendClear(INT_PKA_IRQ);
    IntEnable(INT_PKA_IRQ);

    return ECDSACC26X2_STATUS_FSM_RUN_PKA_OP;
}

/*
 *  ======== ECDSACC26X2_runVerifyFSM ========
 */
static int_fast16_t ECDSACC26X2_runVerifyFSM(ECDSA_Handle handle) {
    ECDSACC26X2_Object *object = handle->object;
    uint32_t pkaResult;

    switch (object->fsmState) {
        case ECDSACC26X2_FSM_VERIFY_R_S_IN_RANGE:

            PKABigNumCmpStart(object->operation.verify->r,
                              object->operation.verify->curve->order,
                              object->operation.verify->curve->length);

            while(PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY);

            pkaResult = PKABigNumCmpGetResult();

            if (pkaResult != PKA_STATUS_A_LESS_THAN_B) {
                return ECDSA_STATUS_R_LARGER_THAN_ORDER;
            }

            PKABigNumCmpStart(object->operation.verify->s,
                              object->operation.verify->curve->order,
                              object->operation.verify->curve->length);

            while(PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY);

            pkaResult = PKABigNumCmpGetResult();

            if (pkaResult == PKA_STATUS_A_LESS_THAN_B) {
                return ECDSACC26X2_STATUS_FSM_RUN_FSM;
            }
            else {
                return ECDSA_STATUS_S_LARGER_THAN_ORDER;
            }

        case ECDSACC26X2_FSM_VERIFY_VALIDATE_PUBLIC_KEY:

            pkaResult = PKAEccVerifyPublicKeyWeierstrassStart(object->operation.verify->theirPublicKey->u.plaintext.keyMaterial,
                                                              object->operation.verify->theirPublicKey->u.plaintext.keyMaterial
                                                                + object->operation.verify->curve->length,
                                                              object->operation.verify->curve->prime,
                                                              object->operation.verify->curve->a,
                                                              object->operation.verify->curve->b,
                                                              object->operation.verify->curve->order,
                                                              object->operation.verify->curve->length);

            // Break out early since no PKA operation was started by the verify fxn
            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_VERIFY_COMPUTE_S_INV:

            PKABigNumInvModStart(object->operation.verify->s,
                                 object->operation.verify->curve->length,
                                 object->operation.verify->curve->order,
                                 object->operation.verify->curve->length,
                                 &resultAddress);

            break;

        case ECDSACC26X2_FSM_VERIFY_COMPUTE_S_INV_RESULT:

            pkaResult = PKABigNumInvModGetResult(ECDSACC26X2_SCRATCH_BUFFER_2,
                                                 object->operation.verify->curve->length,
                                                 resultAddress);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_VERIFY_MULT_S_INV_HASH:

            PKABigNumMultiplyStart(ECDSACC26X2_SCRATCH_BUFFER_2,
                                   object->operation.verify->curve->length,
                                   object->operation.verify->hash,
                                   object->operation.verify->curve->length,
                                   &resultAddress);

            break;

        case ECDSACC26X2_FSM_VERIFY_MULT_S_INV_HASH_RESULT:

            scratchBufferSize = ECDSACC26X2_SCRATCH_BUFFER_SIZE;

            pkaResult = PKABigNumMultGetResult(ECDSACC26X2_SCRATCH_BUFFER,
                                               &scratchBufferSize,
                                               resultAddress);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_VERIFY_S_INV_MULT_HASH_MOD_N:

            PKABigNumModStart(ECDSACC26X2_SCRATCH_BUFFER,
                              scratchBufferSize,
                              object->operation.verify->curve->order,
                              object->operation.verify->curve->length,
                              &resultAddress);

            break;

        case ECDSACC26X2_FSM_VERIFY_S_INV_MULT_HASH_MOD_N_RESULT:
            // Check previous result
            pkaResult = PKABigNumModGetResult(ECDSACC26X2_SCRATCH_BUFFER,
                                              object->operation.verify->curve->length,
                                              resultAddress);

            scratchBufferSize = object->operation.verify->curve->length;

            return ECDSACC26X2_convertReturnValue(pkaResult);


        case ECDSACC26X2_FSM_VERIFY_MULT_G:

            PKAEccMultiplyStart(ECDSACC26X2_SCRATCH_BUFFER,
                                object->operation.verify->curve->generatorX,
                                object->operation.verify->curve->generatorY,
                                object->operation.verify->curve->prime,
                                object->operation.verify->curve->a,
                                object->operation.verify->curve->b,
                                object->operation.verify->curve->length,
                                &resultAddress);

            break;

        case ECDSACC26X2_FSM_VERIFY_MULT_G_RESULT:

            pkaResult = PKAEccMultiplyGetResult(ECDSACC26X2_SCRATCH_BUFFER,
                                                ECDSACC26X2_SCRATCH_BUFFER + object->operation.verify->curve->length,
                                                resultAddress,
                                                object->operation.verify->curve->length);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_VERIFY_MULT_S_INV_R:

            PKABigNumMultiplyStart(ECDSACC26X2_SCRATCH_BUFFER_2,
                                   object->operation.verify->curve->length,
                                   object->operation.verify->r,
                                   object->operation.verify->curve->length,
                                   &resultAddress);

            break;

        case ECDSACC26X2_FSM_VERIFY_MULT_S_INV_R_RESULT:

            scratchBuffer2Size = ECDSACC26X2_SCRATCH_BUFFER_SIZE;

            pkaResult = PKABigNumMultGetResult(ECDSACC26X2_SCRATCH_BUFFER_2,
                                               &scratchBuffer2Size,
                                               resultAddress);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_VERIFY_MULT_S_INV_R_MOD_N:

            PKABigNumModStart(ECDSACC26X2_SCRATCH_BUFFER_2,
                              scratchBuffer2Size,
                              object->operation.verify->curve->order,
                              object->operation.verify->curve->length,
                              &resultAddress);

            break;

        case ECDSACC26X2_FSM_VERIFY_MULT_S_INV_R_MOD_N_RESULT:

            pkaResult = PKABigNumModGetResult(ECDSACC26X2_SCRATCH_BUFFER_2,
                                              object->operation.verify->curve->length,
                                              resultAddress);

            scratchBuffer2Size = object->operation.verify->curve->length;

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_VERIFY_MULT_PUB_KEY:

            PKAEccMultiplyStart(ECDSACC26X2_SCRATCH_BUFFER_2,
                                object->operation.verify->theirPublicKey->u.plaintext.keyMaterial,
                                object->operation.verify->theirPublicKey->u.plaintext.keyMaterial + object->operation.verify->curve->length,
                                object->operation.verify->curve->prime,
                                object->operation.verify->curve->a,
                                object->operation.verify->curve->b,
                                object->operation.verify->curve->length,
                                &resultAddress);

            break;

        case ECDSACC26X2_FSM_VERIFY_MULT_PUB_KEY_RESULT:

            pkaResult = PKAEccMultiplyGetResult(ECDSACC26X2_SCRATCH_BUFFER_2,
                                                ECDSACC26X2_SCRATCH_BUFFER_2 + object->operation.verify->curve->length,
                                                resultAddress,
                                                object->operation.verify->curve->length);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_VERIFY_ADD_MULT_RESULTS:

            PKAEccAddStart(ECDSACC26X2_SCRATCH_BUFFER,
                           ECDSACC26X2_SCRATCH_BUFFER + object->operation.verify->curve->length,
                           ECDSACC26X2_SCRATCH_BUFFER_2,
                           ECDSACC26X2_SCRATCH_BUFFER_2 + object->operation.verify->curve->length,
                           object->operation.verify->curve->prime,
                           object->operation.verify->curve->a,
                           object->operation.verify->curve->length,
                           &resultAddress);

            break;

        case ECDSACC26X2_FSM_VERIFY_ADD_MULT_RESULTS_RESULT:

            pkaResult = PKAEccAddGetResult(ECDSACC26X2_SCRATCH_BUFFER,
                                           NULL,
                                           resultAddress,
                                           object->operation.verify->curve->length);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_VERIFY_POINTX_MOD_N:

            PKABigNumModStart(ECDSACC26X2_SCRATCH_BUFFER,
                              object->operation.verify->curve->length,
                              object->operation.verify->curve->order,
                              object->operation.verify->curve->length,
                              &resultAddress);

            break;

        case ECDSACC26X2_FSM_VERIFY_POINTX_MOD_N_RESULT:

            pkaResult = PKABigNumModGetResult(ECDSACC26X2_SCRATCH_BUFFER,
                                              object->operation.verify->curve->length,
                                              resultAddress);

            return ECDSACC26X2_convertReturnValue(pkaResult);

        case ECDSACC26X2_FSM_VERIFY_COMPARE_RESULT_R:

            PKABigNumCmpStart(object->operation.verify->r,
                              ECDSACC26X2_SCRATCH_BUFFER,
                              object->operation.verify->curve->length);

            while(PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY);

            pkaResult = PKABigNumCmpGetResult();

            if (pkaResult == PKA_STATUS_EQUAL) {
                return ECDSA_STATUS_SUCCESS;
            }
            else {
                return ECDSA_STATUS_ERROR;
            }


        default:
            return ECDSA_STATUS_ERROR;
    }

    // If we get to this point, we want to perform another PKA operation
    IntPendClear(INT_PKA_IRQ);
    IntEnable(INT_PKA_IRQ);

    return ECDSACC26X2_STATUS_FSM_RUN_PKA_OP;
}

/*
 *  ======== ECDSACC26X2_convertReturnValue ========
 */
static int_fast16_t ECDSACC26X2_convertReturnValue(uint32_t pkaResult) {
    switch (pkaResult) {
        case PKA_STATUS_SUCCESS:

            return ECDSACC26X2_STATUS_FSM_RUN_FSM;

        case PKA_STATUS_X_ZERO:
        case PKA_STATUS_Y_ZERO:
        case PKA_STATUS_RESULT_0:
            /* Theoretically, PKA_STATUS_RESULT_0 might be caused by other
             * operations failing but the only one that really should yield
             * 0 is ECC multiplication with invalid inputs that yield the
             * point at infinity.
             */
            return ECDSA_STATUS_POINT_AT_INFINITY;

        case PKA_STATUS_X_LARGER_THAN_PRIME:
        case PKA_STATUS_Y_LARGER_THAN_PRIME:

            return ECDSA_STATUS_PUBLIC_KEY_LARGER_THAN_PRIME;

        case PKA_STATUS_POINT_NOT_ON_CURVE:

            return ECDSA_STATUS_PUBLIC_KEY_NOT_ON_CURVE;

        default:
            return ECDSA_STATUS_ERROR;
    }
}

/*
 *  ======== ECDSACC26X2_waitForAccess ========
 */
static int_fast16_t ECDSACC26X2_waitForAccess(ECDSA_Handle handle) {
    ECDSACC26X2_Object *object = handle->object;
    uint32_t timeout;

    /* Set to SemaphoreP_NO_WAIT to start operations from SWI or HWI context */
    timeout = object->returnBehavior == ECDSA_RETURN_BEHAVIOR_BLOCKING ? object->semaphoreTimeout : SemaphoreP_NO_WAIT;

    return SemaphoreP_pend(&PKAResourceCC26XX_accessSemaphore, timeout);
}

/*
 *  ======== ECDSACC26X2_waitForResult ========
 */
static int_fast16_t ECDSACC26X2_waitForResult(ECDSA_Handle handle){
    ECDSACC26X2_Object *object = handle->object;

    object->operationInProgress = true;

    switch (object->returnBehavior) {
        case ECDSA_RETURN_BEHAVIOR_POLLING:
            while(!PKAResourceCC26XX_pollingFlag);
            return object->operationStatus;
        case ECDSA_RETURN_BEHAVIOR_BLOCKING:
            SemaphoreP_pend(&PKAResourceCC26XX_operationSemaphore, SemaphoreP_WAIT_FOREVER);
            return object->operationStatus;
        case ECDSA_RETURN_BEHAVIOR_CALLBACK:
            return ECDSA_STATUS_SUCCESS;
        default:
            return ECDSA_STATUS_ERROR;
    }
}


/*
 *  ======== ECDSA_init ========
 */
void ECDSA_init(void) {
    PKAResourceCC26XX_constructRTOSObjects();

    isInitialized = true;
}


/*
 *  ======== ECDSA_close ========
 */
void ECDSA_close(ECDSA_Handle handle) {
    ECDSACC26X2_Object         *object;

    DebugP_assert(handle);

    /* Get the pointer to the object */
    object = handle->object;

    /* Mark the module as available */
    object->isOpen = false;

    /* Release power dependency on PKA Module. */
    Power_releaseDependency(PowerCC26X2_PERIPH_PKA);
}


/*
 *  ======== ECDSA_open ========
 */
ECDSA_Handle ECDSA_open(uint_least8_t index, ECDSA_Params *params) {
    ECDSA_Handle                  handle;
    ECDSACC26X2_Object           *object;
    uint_fast8_t                key;

    handle = (ECDSA_Handle)&(ECDSA_config[index]);
    object = handle->object;

    DebugP_assert(index < ECDSA_count);

    /* If params are NULL, use defaults */
    if (params == NULL) {
        params = (ECDSA_Params *)&ECDSA_defaultParams;
    }

    key = HwiP_disable();

    if (!isInitialized || object->isOpen) {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    DebugP_assert((params->returnBehavior == ECDSA_RETURN_BEHAVIOR_CALLBACK) ? params->callbackFxn : true);

    object->returnBehavior = params->returnBehavior;
    object->callbackFxn = params->returnBehavior == ECDSA_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : ECDSACC26X2_internalCallbackFxn;
    object->semaphoreTimeout = params->timeout;

    /* Set power dependency - i.e. power up and enable clock for PKA (PKAResourceCC26XX) module. */
    Power_setDependency(PowerCC26X2_PERIPH_PKA);

    return handle;
}

/*
 *  ======== ECDSA_sign ========
 */
int_fast16_t ECDSA_sign(ECDSA_Handle handle, ECDSA_OperationSign *operation) {
    ECDSACC26X2_Object *object              = handle->object;
    ECDSACC26X2_HWAttrs const *hwAttrs      = handle->hwAttrs;

    DebugP_assert(handle);
    DebugP_assert(operation);
    DebugP_assert(operation->eccParams);
    DebugP_assert(operation->eccParams->length == myPrivateKey->u.plaintext.keyLength);
    DebugP_assert(operation->myPrivateKey->encoding == CryptoKey_PLAINTEXT);
    DebugP_assert(operation->hash);
    DebugP_assert(operation->r);
    DebugP_assert(operation->s);
    DebugP_assert(operation->pmsn);

    if (ECDSACC26X2_waitForAccess(handle) != SemaphoreP_OK) {
        return ECDSA_STATUS_RESOURCE_UNAVAILABLE;
    }

    object->operation.sign      = operation;
    object->operationType       = ECDSA_OPERATION_TYPE_SIGN;
    object->fsmState            = ECDSACC26X2_FSM_SIGN_VALIDATE_PMSN;
    object->fsmFxn              = ECDSACC26X2_runSignFSM;
    object->operationStatus     = ECDSA_STATUS_ERROR;
    object->operationCanceled   = false;
    scratchBufferSize           = ECDSACC26X2_SCRATCH_BUFFER_SIZE;
    scratchBuffer2Size          = ECDSACC26X2_SCRATCH_BUFFER_SIZE;


    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&PKAResourceCC26XX_hwi, ECDSACC26X2_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_PKA_IRQ, hwAttrs->intPriority);

    PKAResourceCC26XX_pollingFlag = 0;

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* Run the FSM by triggering the interrupt. It is level triggered
     * and the complement of the RUN bit.
     */
    IntEnable(INT_PKA_IRQ);

    return ECDSACC26X2_waitForResult(handle);
}

/*
 *  ======== ECDSA_verify ========
 */
int_fast16_t ECDSA_verify(ECDSA_Handle handle, ECDSA_OperationVerify *operation) {
    ECDSACC26X2_Object *object              = handle->object;
    ECDSACC26X2_HWAttrs const *hwAttrs      = handle->hwAttrs;

    DebugP_assert(handle);
    DebugP_assert(operation);
    DebugP_assert(operation->eccParams);
    DebugP_assert(operation->eccParams->length == theirPublicKey->u.plaintext.keyLength / 2);
    DebugP_assert(operation->theirPublicKey->encoding == CryptoKey_PLAINTEXT);
    DebugP_assert(operation->hash);
    DebugP_assert(operation->r);
    DebugP_assert(operation->s);


    if (ECDSACC26X2_waitForAccess(handle) != SemaphoreP_OK) {
        return ECDSA_STATUS_RESOURCE_UNAVAILABLE;
    }


    object->operation.verify    = operation;
    object->operationType       = ECDSA_OPERATION_TYPE_VERIFY;
    object->fsmState            = ECDSACC26X2_FSM_VERIFY_R_S_IN_RANGE;
    object->fsmFxn              = ECDSACC26X2_runVerifyFSM;
    object->operationStatus     = ECDSA_STATUS_ERROR;
    object->operationCanceled   = false;
    scratchBufferSize           = ECDSACC26X2_SCRATCH_BUFFER_SIZE;
    scratchBuffer2Size          = ECDSACC26X2_SCRATCH_BUFFER_SIZE;



    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&PKAResourceCC26XX_hwi, ECDSACC26X2_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_PKA_IRQ, hwAttrs->intPriority);

    PKAResourceCC26XX_pollingFlag = 0;

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* Run the FSM by triggering the interrupt. It is level triggered
     * and the complement of the RUN bit.
     */
    IntEnable(INT_PKA_IRQ);

    return ECDSACC26X2_waitForResult(handle);
}

/*
 *  ======== ECDSA_cancelOperation ========
 */
int_fast16_t ECDSA_cancelOperation(ECDSA_Handle handle) {
    ECDSACC26X2_Object *object = handle->object;

    if(!object->operationInProgress){
        return ECDSA_STATUS_ERROR;
    }

    object->operationCanceled = true;

    /* Post hwi as if operation finished for cleanup */
    IntEnable(INT_PKA_IRQ);
    HwiP_post(INT_PKA_IRQ);


    return ECDSA_STATUS_SUCCESS;
}
