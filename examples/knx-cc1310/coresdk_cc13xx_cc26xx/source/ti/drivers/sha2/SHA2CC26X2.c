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

#include <ti/drivers/cryptoutils/sharedresources/CryptoResourceCC26XX.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/sha2/SHA2CC26X2.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_crypto.h)
#include DeviceFamily_constructPath(driverlib/sha2.h)
#include DeviceFamily_constructPath(driverlib/cpu.h)
#include DeviceFamily_constructPath(driverlib/interrupt.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/smph.h)

/* Defines and enumerations */
#define SHA2_UNUSED(value)    ((void)(value))

typedef enum {
    SHA2_OperationType_SingleStep,
    SHA2_OperationType_MultiStep,
    SHA2_OperationType_Finalize,
} SHA2_OperationType;

/* Forward declarations */
static uint32_t floorUint32(uint32_t value, uint32_t divider);
static void SHA2_hwiFxn (uintptr_t arg0);
static int_fast16_t SHA2_waitForAccess(SHA2_Handle handle);
static int_fast16_t SHA2_waitForResult(SHA2_Handle handle);

/* Static globals */
static const uint32_t hashModeTable[] = {
    SHA2_MODE_SELECT_SHA224,
    SHA2_MODE_SELECT_SHA256,
    SHA2_MODE_SELECT_SHA384,
    SHA2_MODE_SELECT_SHA512
};

static const uint8_t blockSizeTable[] = {
    SHA2_BLOCK_SIZE_BYTES_224,
    SHA2_BLOCK_SIZE_BYTES_256,
    SHA2_BLOCK_SIZE_BYTES_384,
    SHA2_BLOCK_SIZE_BYTES_512
};

static const uint8_t digestSizeTable[] = {
    SHA2_DIGEST_LENGTH_BYTES_224,
    SHA2_DIGEST_LENGTH_BYTES_256,
    SHA2_DIGEST_LENGTH_BYTES_384,
    SHA2_DIGEST_LENGTH_BYTES_512
};

static const uint8_t *SHA2_data;

static uint32_t SHA2_dataBytesRemaining;

static SHA2_OperationType SHA2_operationType;

static bool isInitialized = false;

/*
 *  ======== floorUint32 helper ========
 */
uint32_t floorUint32(uint32_t value, uint32_t divider) {
    return (value / divider) * divider;
}

/*
 *  ======== SHA2_hwiFxn ========
 */
static void SHA2_hwiFxn (uintptr_t arg0) {
    SHA2CC26X2_Object *object = ((SHA2_Handle)arg0)->object;
    uint32_t blockSize = blockSizeTable[object->hashType];;
    uint32_t irqStatus;
    uint32_t key;

    irqStatus = SHA2IntStatusRaw();
    SHA2IntClear(SHA2_RESULT_RDY | SHA2_DMA_IN_DONE | SHA2_DMA_BUS_ERR);

    /*
     * Prevent the following section from being interrupted by SHA2_cancelOperation().
     */
    key = HwiP_disable();

    if (object->operationCanceled) {
        /*
         * If the operation has been canceled we can end here.
         * Cleanup is done by SHA2_cancelOperation()
         */
        HwiP_restore(key);
        return;

    } else if (irqStatus & SHA2_DMA_BUS_ERR) {
        /*
         * In the unlikely event of an error we can stop here.
         */
        object->returnStatus = SHA2_STATUS_ERROR;

    } else if (SHA2_dataBytesRemaining == 0) {
        /*
         * Last transaction has finished. Nothing to do.
         */

    } else if (SHA2_dataBytesRemaining >= blockSize) {
        /*
         * Start another transaction
         */
        uint32_t transactionLength = floorUint32(SHA2_dataBytesRemaining, blockSize);

        SHA2ComputeIntermediateHash(SHA2_data,
                               object->digest,
                               hashModeTable[object->hashType],
                               transactionLength);

        SHA2_dataBytesRemaining -= transactionLength;
        SHA2_data += transactionLength;
        object->bytesProcessed += transactionLength;

        HwiP_restore(key);
        return;

    } else if (SHA2_dataBytesRemaining > 0) {
        /*
         * Copy remaining data into buffer
         */
        memcpy(object->buffer, SHA2_data, SHA2_dataBytesRemaining);
        object->bytesInBuffer += SHA2_dataBytesRemaining;
        SHA2_dataBytesRemaining = 0;
    }

    /*
     * Since we got here, every transaction has been finished
     */
    object->operationInProgress = false;

    /*
     * Reset byte counter if a hash has been finalized
     */
    if (SHA2_operationType != SHA2_OperationType_MultiStep) {
        object->bytesProcessed = 0;
        object->bytesInBuffer = 0;
    }

    HwiP_restore(key);

    /*  Grant access for other threads to use the crypto module.
     *  The semaphore must be posted before the callbackFxn to allow the chaining
     *  of operations.
     */
    SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);

    Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    if (object->returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING) {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoResourceCC26XX_operationSemaphore);
    }
    else if (object->returnBehavior == SHA2_RETURN_BEHAVIOR_CALLBACK)
    {
        if (object->callbackFxn) {
            object->callbackFxn((SHA2_Handle)arg0, object->returnStatus);
        }
    }
}


/*
 *  ======== SHA2_waitForAccess ========
 */
static int_fast16_t SHA2_waitForAccess(SHA2_Handle handle) {
    SHA2CC26X2_Object *object = handle->object;

    return SemaphoreP_pend(&CryptoResourceCC26XX_accessSemaphore, object->accessTimeout);
}

/*
 *  ======== SHA2_waitForResult ========
 */
static int_fast16_t SHA2_waitForResult(SHA2_Handle handle){
    SHA2CC26X2_Object *object = handle->object;

    if (object->returnBehavior == SHA2_RETURN_BEHAVIOR_POLLING) {
        do {
            SHA2WaitForIRQFlags(SHA2_RESULT_RDY | SHA2_DMA_BUS_ERR);
            SHA2_hwiFxn((uintptr_t)handle);
        } while (object->operationInProgress);

        return object->returnStatus;
    }
    else if (object->returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING) {
        SemaphoreP_pend(&CryptoResourceCC26XX_operationSemaphore, (uint32_t)SemaphoreP_WAIT_FOREVER);

        return object->returnStatus;
    }
    else {
        return SHA2_STATUS_SUCCESS;
    }

}

/*
 *  ======== SHA2_init ========
 */
void SHA2_init(void) {
    CryptoResourceCC26XX_constructRTOSObjects();

    isInitialized = true;
}

/*
 *  ======== SHA2_open ========
 */
SHA2_Handle SHA2_open(uint_least8_t index, const SHA2_Params *params) {
    DebugP_assert(index < SHA2_count);

    SHA2_Config *config = (SHA2_Config*)&SHA2_config[index];
    return SHA2CC26X2_construct(config, params);
}

/*
 *  ======== SHA2_construct ========
 */
SHA2_Handle SHA2CC26X2_construct(SHA2_Config *config, const SHA2_Params *params) {
    SHA2_Handle                 handle;
    SHA2CC26X2_Object           *object;
    uint_fast8_t                key;

    handle = (SHA2_Config*)config;
    object = handle->object;

    key = HwiP_disable();

    if (object->isOpen || !isInitialized) {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;
    object->operationInProgress = false;
    object->operationCanceled = false;

    HwiP_restore(key);

    if (params == NULL) {
        params = &SHA2_defaultParams;
    }

    DebugP_assert(params->returnBehavior == SHA2_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    object->bytesInBuffer   = 0;
    object->bytesProcessed  = 0;
    object->returnBehavior  = params->returnBehavior;
    object->callbackFxn     = params->callbackFxn;
    object->hashType        = params->hashType;

    if (params->returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING) {
        object->accessTimeout = params->timeout;
    } else {
        object->accessTimeout = SemaphoreP_NO_WAIT;
    }

    /* Set power dependency - i.e. power up and enable clock for Crypto (CryptoResourceCC26XX) module. */
    Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);

    return handle;
}

/*
 *  ======== SHA2_close ========
 */
void SHA2_close(SHA2_Handle handle) {
    SHA2CC26X2_Object         *object;
    uintptr_t key;

    DebugP_assert(handle);

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;

    /* If there is still an operation ongoing, abort it now. */
    key = HwiP_disable();
    if (object->operationInProgress) {
        SHA2_cancelOperation(handle);
    }
    object->isOpen = false;
    HwiP_restore(key);

    /* Release power dependency on Crypto Module. */
    Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== SHA2_startHash ========
 */
int_fast16_t SHA2_addData(SHA2_Handle handle, const void* data, size_t length) {
    SHA2CC26X2_Object *object = handle->object;
    SHA2CC26X2_HWAttrs const *hwAttrs = handle->hwAttrs;
    uint32_t blockSize = blockSizeTable[object->hashType];
    uintptr_t key;

    /* Try and obtain access to the crypto module */
    if (SHA2_waitForAccess(handle) != SemaphoreP_OK) {
        return SHA2_STATUS_RESOURCE_UNAVAILABLE;
    }

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* If we are in SHA2_RETURN_BEHAVIOR_POLLING, we do not want an interrupt to trigger.
     * We need to disable it before kicking off the operation.
     */
    if (object->returnBehavior == SHA2_RETURN_BEHAVIOR_POLLING)  {
        IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }
    else {
        /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
         * drivers and they all need to coexist. Whenever a driver starts an operation, it
         * registers its HWI callback with the OS.
         */
        HwiP_setFunc(&CryptoResourceCC26XX_hwi, SHA2_hwiFxn, (uintptr_t)handle);
        HwiP_setPriority(INT_CRYPTO_RESULT_AVAIL_IRQ, hwAttrs->intPriority);

        IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);
        IntEnable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }

    object->returnStatus = SHA2_STATUS_SUCCESS;
    object->operationCanceled = false;
    SHA2_operationType = SHA2_OperationType_MultiStep;

    if ((object->bytesInBuffer + length) >= blockSize) {
        /* We have accumulated enough data to start a transaction. Now the question
         * remains whether we have to merge bytes from the data stream into the
         * buffer first. If so, we do that now, then start a transaction.
         * If the buffer is empty, we can start a transaction on the data stream.
         * Once the transaction is finished, we will decide how to follow up,
         * i.e. copy remaining data into the buffer.
         */
        uint32_t transactionLength;
        const uint8_t* transactionStartAddress;

        if (object->bytesInBuffer > 0) {
            uint8_t *bufferTail = &object->buffer[object->bytesInBuffer];
            uint32_t bytesToCopyToBuffer = blockSize - object->bytesInBuffer;
            memcpy(bufferTail, data, bytesToCopyToBuffer);

            /* We reset the value already. That saves a comparison
             * in the ISR handler
             */
            object->bytesInBuffer = 0;

            transactionStartAddress = object->buffer;
            transactionLength       = blockSize;

            SHA2_data = (const uint8_t*)data + bytesToCopyToBuffer;
            SHA2_dataBytesRemaining = length - bytesToCopyToBuffer;
        } else {
            transactionStartAddress = data;
            transactionLength = floorUint32(length, blockSize);

            SHA2_data = (const uint8_t*)data + transactionLength;
            SHA2_dataBytesRemaining = length - transactionLength;
        }

        /*
         * Starting the accelerator and setting the operationInProgress
         * flag must be atomic.
         */
        key = HwiP_disable();

        /*
         * Finally we need to decide whether this is the first hash
         * operation or a follow-up from a previous one.
         */
        if (object->bytesProcessed > 0) {
            SHA2ComputeIntermediateHash(transactionStartAddress,
                                   object->digest,
                                   hashModeTable[object->hashType],
                                   transactionLength);
        } else {
            SHA2ComputeInitialHash(transactionStartAddress,
                                   object->digest,
                                   hashModeTable[object->hashType],
                                   transactionLength);
        }

        object->bytesProcessed += transactionLength;
        object->operationInProgress = true;
        HwiP_restore(key);

    } else {
        /* There is no action required by the hardware. But we kick the
         * interrupt in order to follow the same code path as the other
         * operations.
         */
        uint8_t *bufferTail = &object->buffer[object->bytesInBuffer];
        memcpy(bufferTail, data, length);
        object->bytesInBuffer += length;
        SHA2_dataBytesRemaining = 0;

        /*
         * Asserting the IRQ and setting the operationInProgress
         * flag must be atomic.
         */
        key = HwiP_disable();
        object->operationInProgress = true;
        SHA2IntEnable(SHA2_RESULT_RDY);
        HWREG(CRYPTO_BASE + CRYPTO_O_IRQSET) = SHA2_RESULT_RDY;
        HwiP_restore(key);
    }

    return SHA2_waitForResult(handle);
}

/*
 *  ======== SHA2_finalize ========
 */
int_fast16_t SHA2_finalize(SHA2_Handle handle, void *digest) {
    SHA2CC26X2_Object *object = handle->object;
    SHA2CC26X2_HWAttrs const *hwAttrs = handle->hwAttrs;
    uintptr_t key;

    /* Try and obtain access to the crypto module */
    if (SHA2_waitForAccess(handle) != SemaphoreP_OK) {
        return SHA2_STATUS_RESOURCE_UNAVAILABLE;
    }

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* If we are in SHA2_RETURN_BEHAVIOR_POLLING, we do not want an interrupt to trigger.
     * We need to disable it before kicking off the operation.
     */
    if (object->returnBehavior == SHA2_RETURN_BEHAVIOR_POLLING)  {
        IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }
    else {
        /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
         * drivers and they all need to coexist. Whenever a driver starts an operation, it
         * registers its HWI callback with the OS.
         */
        HwiP_setFunc(&CryptoResourceCC26XX_hwi, SHA2_hwiFxn, (uintptr_t)handle);
        HwiP_setPriority(INT_CRYPTO_RESULT_AVAIL_IRQ, hwAttrs->intPriority);

        IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);
        IntEnable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }

    object->returnStatus = SHA2_STATUS_SUCCESS;
    object->operationCanceled = false;
    SHA2_operationType = SHA2_OperationType_Finalize;

    /*
     * Starting the accelerator and setting the operationInProgress
     * flag must be atomic.
     */
    key = HwiP_disable();
    object->operationInProgress = true;

    if (object->bytesProcessed == 0) {
        /*
         * Since no hash operation has been performed yet and no intermediate
         * digest is available, we have to perform a full hash operation
         */
        SHA2ComputeHash(object->buffer,
                        digest,
                        object->bytesInBuffer,
                        hashModeTable[object->hashType]);
    }
    else if (object->bytesInBuffer > 0) {
        uint32_t totalLength = object->bytesProcessed + object->bytesInBuffer;
        uint32_t chunkLength = object->bytesInBuffer;

        SHA2ComputeFinalHash(object->buffer,
                             digest,
                             object->digest,
                             totalLength,
                             chunkLength,
                             hashModeTable[object->hashType]);
    } else {
        /*
         * The hardware is incapable of finalizing an empty partial message,
         * but we can trick it by pretending this to be an intermediate block.
         *
         * Calculate the length in bits and put it at the end of the dummy
         * finalization block in big endian order
         */
        uint64_t lengthInBits = object->bytesProcessed * 8;
        uint32_t blockSize    = blockSizeTable[object->hashType];
        uint8_t *lengthBytes  = (uint8_t*)&lengthInBits;

        /*
         * Use the existing buffer as scratch pad
         */
        memset(object->buffer, 0, blockSize);

        /*
         * Final block starts with '10000000'.
         */
        object->buffer[0] = 0x80;

        /*
         * The length is written into the end of the finalization block
         * in big endian order. We always write only the last 8 bytes.
         */
        uint32_t i = 0;
        for (i = 0; i < 4; i++) {
            object->buffer[blockSize - 8 + i] = lengthBytes[7 - i];
            object->buffer[blockSize - 4 + i] = lengthBytes[3 - i];
        }

        /*
         * SHA2ComputeIntermediateHash uses the same digest location for
         * both input and output. Instead of copying the final digest result
         * we use the final location as input and output.
         */
        memcpy(digest, object->digest, digestSizeTable[object->hashType]);

        SHA2ComputeIntermediateHash(object->buffer,
                               digest,
                               hashModeTable[object->hashType],
                               blockSize);
    }

    HwiP_restore(key);

    return SHA2_waitForResult(handle);
}

/*
 *  ======== SHA2_hashData ========
 */
int_fast16_t SHA2_hashData(SHA2_Handle handle, const void *data, size_t length, void *digest) {
    SHA2CC26X2_Object *object = handle->object;
    SHA2CC26X2_HWAttrs const *hwAttrs = handle->hwAttrs;
    uintptr_t key;

    /* Try and obtain access to the crypto module */
    if (SHA2_waitForAccess(handle) != SemaphoreP_OK) {
        return SHA2_STATUS_RESOURCE_UNAVAILABLE;
    }

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* If we are in SHA2_RETURN_BEHAVIOR_POLLING, we do not want an interrupt to trigger.
     * We need to disable it before kicking off the operation.
     */
    if (object->returnBehavior == SHA2_RETURN_BEHAVIOR_POLLING)  {
        IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }
    else {
        /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
         * drivers and they all need to coexist. Whenever a driver starts an operation, it
         * registers its HWI callback with the OS.
         */
        HwiP_setFunc(&CryptoResourceCC26XX_hwi, SHA2_hwiFxn, (uintptr_t)handle);
        HwiP_setPriority(INT_CRYPTO_RESULT_AVAIL_IRQ, hwAttrs->intPriority);

        IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);
        IntEnable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }

    SHA2_operationType = SHA2_OperationType_SingleStep;
    SHA2_dataBytesRemaining = 0;

    object->returnStatus = SHA2_STATUS_SUCCESS;
    object->operationCanceled = false;
    object->bytesInBuffer = 0;
    object->bytesProcessed = 0;

    /*
     * Starting the accelerator and setting the operationInProgress
     * flag must be atomic.
     */
    key = HwiP_disable();
    SHA2ComputeHash(data,
                    digest,
                    length,
                    hashModeTable[object->hashType]);

    object->operationInProgress = true;
    HwiP_restore(key);

    return SHA2_waitForResult(handle);
}

/*
 *  ======== SHA2_reset ========
 */
void SHA2_reset(SHA2_Handle handle)
{
    SHA2CC26X2_Object *object = (SHA2CC26X2_Object*)handle->object;

    uint32_t key = HwiP_disable();

    if (object->operationInProgress == true)
    {
        SHA2_cancelOperation(handle);
    }

    object->bytesInBuffer  = 0;
    object->bytesProcessed = 0;

    HwiP_restore(key);
}

/*
 *  ======== SHA2_cancelOperation ========
 */
int_fast16_t SHA2_cancelOperation(SHA2_Handle handle) {
    SHA2CC26X2_Object *object         = handle->object;
    uint32_t key;

    key = HwiP_disable();

    if (!object->operationInProgress) {
        HwiP_restore(key);
        return SHA2_STATUS_ERROR;
    }

    /* Reset the accelerator. Immediately stops ongoing operations. */
    HWREG(CRYPTO_BASE + CRYPTO_O_SWRESET) = CRYPTO_SWRESET_SW_RESET;

    /* Consume any outstanding interrupts we may have accrued
     * since disabling interrupts.
     */
    IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);

    Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    object->bytesInBuffer = 0;
    object->bytesProcessed = 0;
    object->operationCanceled = true;
    object->returnStatus = SHA2_STATUS_CANCELED;

    HwiP_restore(key);

    /*  Grant access for other threads to use the crypto module.
     *  The semaphore must be posted before the callbackFxn to allow the chaining
     *  of operations.
     */
    SemaphoreP_post(&CryptoResourceCC26XX_accessSemaphore);


    if (object->returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING) {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoResourceCC26XX_operationSemaphore);
    }
    else {
        /* Call the callback function provided by the application. */
        object->callbackFxn(handle, SHA2_STATUS_CANCELED);
    }

    return SHA2_STATUS_SUCCESS;
}

int_fast16_t SHA2_setHashType(SHA2_Handle handle, SHA2_HashType type) {

    SHA2CC26X2_Object *object = (SHA2CC26X2_Object*)handle->object;

    if (object->operationInProgress) {
        return SHA2_STATUS_ERROR;
    }

    object->hashType = type;

    return SHA2_STATUS_SUCCESS;
}
