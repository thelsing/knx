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
/*!****************************************************************************
 * @file       AESCTR.h
 *
 * @brief      AESCTR driver header
 *
 * @warning    This is a beta API. It may change in future releases.
 *
 * @anchor ti_drivers_AESCTR_Overview
 * <h3> Overview </h3>
 * The Counter (CTR) mode of operation is a generic block cipher mode of operation
 * that can be used with any block cipher including AES.
 *
 * CTR mode encrypts and decrypts messages. It is not required for the message
 * length to be evenly divisible by the cipher block size. This also means
 * that padding the message is not required.
 *
 * <h3> Operation </h3>
 * CTR encryption and decryption perform the following steps:
 *     -# Set the counter value to the initial counter value
 *     -# Encrypt the counter value under the symmetric key
 *     -# XOR the encrypted counter value with the input block (plaintext or ciphertext)
 *     -# Increment the counter value. Interpret the byte array as a big-endian number.
 *     -# Repeat steps 2 to 4 until the input is completely processed. If the
 *        input is not evenly divisible by the block size, XOR the last
 *        (u = input length % block size) input bytes with the most significant
 *        u bytes of the last encrypted counter value.
 *
 * CTR performs the same steps regardless of whether it is used to
 * encrypt or decrypt a message. The input merely changes.
 *
 * <h3> Choosing Initial Counter Values </h3>
 * CTR requires that each counter value used to encrypt a block of a message
 * is unique for each key used. If this requirement is not kept, the
 * confidentiality of that message block may be compromised.
 *
 * There are two general strategies when choosing the initial counter value
 * of a CTR operation to ensure this requirement holds.
 *
 * The first is to choose an initial counter value for the first message
 * and increment the initial counter value for a subsequent message by
 * by message length % block length (16-bytes for AES). This effectively
 * turns a sequence of messages into one long message. If 0 is chosen
 * as the initial counter value, up to 2^128 - 1 blocks may be encrypted before
 * key rotation is mandatory.
 *
 * The second is to split the initial counter value into a nonce and
 * counter section. The nonce of length n bits must be unique per message.
 * This allows for up to 2^n - 1 messages to be encrypted before
 * key rotation is required. The counter section of length c is incremented
 * as usual. This limits messages to a length of at most 2^c - 1 blocks.
 * n and c must be chosen such that n + c = block length in bits
 * (128 bits for AES) holds.
 *
 * @anchor ti_drivers_AESCTR_Usage
 * <h3> Usage </h3>
 * <h4> Before starting a CTR operation </h4>
 *
 * Before starting a CTR operation, the application must do the following:
 *     - Call #AESCTR_init() to initialize the driver
 *     - Call #AESCTR_Params_init() to initialize the #AESCTR_Params to default values.
 *     - Modify the #AESCTR_Params as desired
 *     - Call #AESCTR_open() to open an instance of the driver
 *     - Initialize a CryptoKey. These opaque data structures are representations
 *       of keying material and its storage. Depending on how the keying material
 *       is stored (RAM or flash, key store, key blob), the CryptoKey must be
 *       initialized differently. The AESCTR API can handle all types of CryptoKey.
 *       However, not all device-specific implementions support all types of CryptoKey.
 *       Devices without a key store will not support CryptoKeys with keying material
 *       stored in a key store for example.
 *       All devices support plaintext CryptoKeys.
 *     - Initialize the #AESCTR_Operation using #AESCTR_Operation_init() and set all
 *       length, key, and buffer fields.
 *
 * <h4> Starting a CTR operation </h4>
 *
 * The AESCTR_oneStepEncrypt() and AESCTR_oneStepDecrypt() functions perform a CTR operation
 * in a single call.
 *
 * <h4> After the CTR operation completes </h4>
 *
 * After the CTR operation completes, the application should either start
 * another operation or close the driver by calling #AESCTR_close().
 *
 * @anchor ti_drivers_AESCTR_Synopsis
 * ## Synopsis
 *
 * @anchor ti_drivers_AESCTR_Synopsis_Code
 * @code
 *
 * // Import AESCTR Driver definitions
 * #include <ti/drivers/AESCTR.h>
 *
 * // Define name for AESCTR channel index
 * #define AESCTR_INSTANCE 0
 *
 * AESCTR_init();
 *
 * handle = AESCTR_open(AESCTR_INSTANCE, NULL);
 *
 * // Initialize symmetric key
 * CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
 *
 * // Set up AESCTR_Operation
 * AESCTR_Operation_init(&operation);
 * operation.key               = &cryptoKey;
 * operation.input             = plaintext;
 * operation.output            = ciphertext;
 * operation.inputLength       = sizeof(plaintext);
 * operation.iv                = iv;
 *
 * encryptionResult = AESCTR_oneStepEncrypt(handle, &operation);
 *
 * AESCTR_close(handle);
 * @endcode
 *
 * @anchor ti_drivers_AESCTR_Examples
 * <h4> Examples </h4>
 *
 * <h5> Single call CTR encryption with plaintext CryptoKey in blocking return mode </h5>
 * @code
 *
 * #include <ti/drivers/AESCTR.h>
 * #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
 *
 * ...
 *
 *     AESCTR_Handle handle;
 *     CryptoKey cryptoKey;
 *     int_fast16_t encryptionResult;
 *
 *     // For example purposes only. Generate IVs in a non-static way in practice.
 *     // Test vector from NIST SP 800-38A
 *     uint8_t initialCounter[16] =    {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
 *                                      0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
 *     uint8_t plaintext[64] =         {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
 *                                      0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
 *                                      0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
 *                                      0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
 *                                      0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
 *                                      0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
 *                                      0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
 *                                      0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10};
 *     ruint8_t ciphertext[sizeof(plaintext)];
 *     uint8_t keyingMaterial[16] =    {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
 *                                      0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
 *
 *     handle = AESCTR_open(0, NULL);
 *
 *     if (handle == NULL) {
 *         // handle error
 *         while(1);
 *     }
 *
 *     CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
 *
 *     AESCTR_Operation operation;
 *     AESCTR_Operation_init(&operation);
 *
 *     operation.key               = &cryptoKey;
 *     operation.input             = plaintext;
 *     operation.output            = ciphertext;
 *     operation.inputLength       = sizeof(plaintext);
 *     operation.initialCounter    = initialCounter;
 *
 *     encryptionResult = AESCTR_oneStepEncrypt(handle, &operation);
 *
 *     if (encryptionResult != AESCTR_STATUS_SUCCESS) {
 *         // handle error
 *         while(1);
 *     }
 *
 *     // The ciphertext should be the following after the encryption operation:
 *     // 0x87, 0x4d, 0x61, 0x91, 0xb6, 0x20, 0xe3, 0x26,
 *     // 0x1b, 0xef, 0x68, 0x64, 0x99, 0x0d, 0xb6, 0xce,
 *     // 0x98, 0x06, 0xf6, 0x6b, 0x79, 0x70, 0xfd, 0xff,
 *     // 0x86, 0x17, 0x18, 0x7b, 0xb9, 0xff, 0xfd, 0xff,
 *     // 0x5a, 0xe4, 0xdf, 0x3e, 0xdb, 0xd5, 0xd3, 0x5e,
 *     // 0x5b, 0x4f, 0x09, 0x02, 0x0d, 0xb0, 0x3e, 0xab,
 *     // 0x1e, 0x03, 0x1d, 0xda, 0x2f, 0xbe, 0x03, 0xd1,
 *     // 0x79, 0x21, 0x70, 0xa0, 0xf3, 0x00, 0x9c, 0xee
 *
 *     AESCTR_close(handle);
 *
 * @endcode
 *
 * <h5> Single call CTR decryption with plaintext CryptoKey in callback return mode </h5>
 * @code
 *
 * #include <ti/drivers/AESCTR.h>
 * #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
 *
 * ...
 *
 *
 * void ctrCallback(AESCTR_Handle handle,
 *                  int_fast16_t returnValue,
 *                  AESCTR_Operation *operation,
 *                  AESCTR_OperationType operationType) {
 *
 *     if (returnValue != AESCTR_STATUS_SUCCESS) {
 *         // handle error
 *         while(1);
 *     }
 * }
 * AESCTR_Operation operation;
 *
 * void ctrStartFunction(void) {
 *     uint8_t initialCounter[16] =    {0x00, 0xE0, 0x01, 0x7B, 0x27, 0x77, 0x7F, 0x3F,
 *                                      0x4A, 0x17, 0x86, 0xF0, 0x00, 0x00, 0x00, 0x01};
 *     uint8_t ciphertext[] =          {0xC1, 0xCF, 0x48, 0xA8, 0x9F, 0x2F, 0xFD, 0xD9,
 *                                      0xCF, 0x46, 0x52, 0xE9, 0xEF, 0xDB, 0x72, 0xD7,
 *                                      0x45, 0x40, 0xA4, 0x2B, 0xDE, 0x6D, 0x78, 0x36,
 *                                      0xD5, 0x9A, 0x5C, 0xEA, 0xAE, 0xF3, 0x10, 0x53,
 *                                      0x25, 0xB2, 0x07, 0x2F};
 *     uint8_t keyingMaterial[] =      {0x76, 0x91, 0xBE, 0x03, 0x5E, 0x50, 0x20, 0xA8,
 *                                      0xAC, 0x6E, 0x61, 0x85, 0x29, 0xF9, 0xA0, 0xDC};
 *     uint8_t plaintext[sizeof(ciphertext)];
 *
 *     AESCTR_Handle handle;
 *     AESCTR_Params params;
 *     CryptoKey cryptoKey;
 *     int_fast16_t decryptionResult;
 *
 *     AESCTR_Operation operation;
 *
 *     AESCTR_Params_init(&params);
 *     params.returnBehavior = AESCTR_RETURN_BEHAVIOR_CALLBACK;
 *     params.callbackFxn = ctrCallback;
 *
 *     handle = AESCTR_open(0, &params);
 *
 *     if (handle == NULL) {
 *         // handle error
 *         while(1);
 *     }
 *
 *     CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
 *
 *     AESCTR_Operation_init(&operation);
 *
 *     operation.key               = &cryptoKey;
 *     operation.input             = ciphertext;
 *     operation.output            = plaintext;
 *     operation.inputLength       = sizeof(ciphertext);
 *     operation.initialCounter    = initialCounter;
 *
 *     decryptionResult = AESCTR_oneStepDecrypt(handle, &operation);
 *
 *     if (decryptionResult != AESCTR_STATUS_SUCCESS) {
 *         // handle error
 *         while(1);
 *     }
 *
 *     // do other things while CTR operation completes in the background
 *
 *     // After the operation completes and the callback is invoked, the resultant
 *     // plaintext should be
 *     // 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
 *     // 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 *     // 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
 *     // 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
 *     // 0x20, 0x21, 0x22, 0x23
 * }
 *
 * @endcode
 */

#ifndef ti_drivers_AESCTR__include
#define ti_drivers_AESCTR__include

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Common AESCTR status code reservation offset.
 * AESCTR driver implementations should offset status codes with
 * #AESCTR_STATUS_RESERVED growing negatively.
 *
 * Example implementation specific status codes:
 * @code
 * #define AESCTRXYZ_STATUS_ERROR0    AESCTR_STATUS_RESERVED - 0
 * #define AESCTRXYZ_STATUS_ERROR1    AESCTR_STATUS_RESERVED - 1
 * #define AESCTRXYZ_STATUS_ERROR2    AESCTR_STATUS_RESERVED - 2
 * @endcode
 */
#define AESCTR_STATUS_RESERVED (-32)

/*!
 * @brief   Successful status code.
 *
 * Functions return #AESCTR_STATUS_SUCCESS if the function was executed
 * successfully.
 */
#define AESCTR_STATUS_SUCCESS (0)

/*!
 * @brief   Generic error status code.
 *
 * Functions return #AESCTR_STATUS_ERROR if the function was not executed
 * successfully and no more pertinent error code could be returned.
 */
#define AESCTR_STATUS_ERROR (-1)

/*!
 * @brief   An error status code returned if the hardware or software resource
 * is currently unavailable.
 *
 * AESCTR driver implementations may have hardware or software limitations on how
 * many clients can simultaneously perform operations. This status code is returned
 * if the mutual exclusion mechanism signals that an operation cannot currently be performed.
 */
#define AESCTR_STATUS_RESOURCE_UNAVAILABLE (-2)

/*!
 *  @brief  The ongoing operation was canceled.
 */
#define AESCTR_STATUS_CANCELED (-3)

/*!
 * @brief   The way in which CTR function calls return after performing an
 * encryption or decryption operation.
 *
 * Not all CTR operations exhibit the specified return behavor. Functions that do not
 * require significant computation and cannot offload that computation to a background thread
 * behave like regular functions. Which functions exhibit the specfied return behavior is not
 * implementation dependent. Specifically, a software-backed implementation run on the same
 * CPU as the application will emulate the return behavior while not actually offloading
 * the computation to the background thread.
 *
 * AESCTR functions exhibiting the specified return behavior have restrictions on the
 * context from which they may be called.
 *
 * |                                | Task  | Hwi   | Swi   |
 * |--------------------------------|-------|-------|-------|
 * |AESCTR_RETURN_BEHAVIOR_CALLBACK | X     | X     | X     |
 * |AESCTR_RETURN_BEHAVIOR_BLOCKING | X     |       |       |
 * |AESCTR_RETURN_BEHAVIOR_POLLING  | X     | X     | X     |
 *
 */
typedef enum
{
    AESCTR_RETURN_BEHAVIOR_CALLBACK = 1, /*!< The function call will return immediately while the
                                          *   CTR operation goes on in the background. The registered
                                          *   callback function is called after the operation completes.
                                          *   The context the callback function is called (task, HWI, SWI)
                                          *   is implementation-dependent.
                                          */
    AESCTR_RETURN_BEHAVIOR_BLOCKING = 2, /*!< The function call will block while the CTR operation goes
                                          *   on in the background. CTR operation results are available
                                          *   after the function returns.
                                          */
    AESCTR_RETURN_BEHAVIOR_POLLING = 4,  /*!< The function call will continuously poll a flag while CTR
                                          *   operation goes on in the background. CTR operation results
                                          *   are available after the function returns.
                                          */
} AESCTR_ReturnBehavior;

/*!
 *  @brief  Enum for the direction of the CTR operation.
 */
typedef enum
{
    AESCTR_MODE_ENCRYPT = 1,
    AESCTR_MODE_DECRYPT = 2,
} AESCTR_Mode;

/*!
 *  @brief  Struct containing the parameters required for encrypting/decrypting
 *          a message.
 *
 *  The driver may access it at any point during the operation. It must remain
 *  in scope for the entire duration of the operation.
 */
typedef struct
{
        const CryptoKey* key;          /*!< A previously initialized CryptoKey. */
        const uint8_t* input;          /*!<
                                        *   - Encryption: The plaintext buffer to be
                                        *     encrypted in the CTR operation.
                                        *   - Decryption: The ciphertext to be decrypted.
                                        */
        uint8_t* output;               /*!<
                                        *   - Encryption: The output ciphertext buffer that
                                        *     the encrypted plaintext is copied to.
                                        *   - Decryption: The plaintext derived from the
                                        *     decrypted ciphertext is copied here.
                                        */
        const uint8_t* initialCounter; /*!< A buffer containing an initial counter. Under
                                        *   the same key, each counter value may only be
                                        *   used to encrypt or decrypt a single input
                                        *   block.
                                        */
        size_t inputLength;            /*!< Length of the input and output in bytes. */
} AESCTR_Operation;

/*!
 *  @brief  Enum for the operation types supported by the driver.
 */
typedef enum
{
    AESCTR_OPERATION_TYPE_ENCRYPT = 1,
    AESCTR_OPERATION_TYPE_DECRYPT = 2,
} AESCTR_OperationType;

/*!
 *  @brief AESCTR Global configuration
 *
 *  The #AESCTR_Config structure contains a set of pointers used to characterize
 *  the AESCTR driver implementation.
 *
 *  This structure needs to be defined before calling #AESCTR_init() and it must
 *  not be changed thereafter.
 *
 *  @sa     #AESCTR_init()
 */
typedef struct AESCTR_Config
{
        /*! Pointer to a driver specific data object */
        void* object;

        /*! Pointer to a driver specific hardware attributes structure */
        void const* hwAttrs;
} AESCTR_Config;

/*!
 *  @brief  A handle that is returned from an #AESCTR_open() call.
 */
typedef AESCTR_Config* AESCTR_Handle;

/*!
 *  @brief  The definition of a callback function used by the AESCTR driver
 *          when used in ::AESCTR_RETURN_BEHAVIOR_CALLBACK
 *
 *  @param  handle Handle of the client that started the CTR operation.
 *
 *  @param  returnValue  The result of the CTR operation. May contain an error code.
 *                       Informs the application of why the callback function was
 *                       called.
 *
 *  @param  operation A pointer to an operation struct.
 *
 *  @param  operationType This parameter determines which operation the
 *          callback refers to.
 */
typedef void (*AESCTR_CallbackFxn)(AESCTR_Handle handle,
                                   int_fast16_t returnValue,
                                   AESCTR_Operation* operation,
                                   AESCTR_OperationType operationType);

/*!
 *  @brief  CTR Parameters
 *
 *  CTR Parameters are used to with the #AESCTR_open() call. Default values for
 *  these parameters are set using #AESCTR_Params_init().
 *
 *  @sa     #AESCTR_Params_init()
 */
typedef struct
{
        AESCTR_ReturnBehavior returnBehavior; /*!< Blocking, callback, or polling return behavior */
        AESCTR_CallbackFxn callbackFxn;       /*!< Callback function pointer */
        uint32_t timeout;                     /*!< Timeout before the driver returns an error in
                                               *   ::AESCTR_RETURN_BEHAVIOR_BLOCKING
                                               */
        void* custom;                         /*!< Custom argument used by driver
                                               *   implementation
                                               */
} AESCTR_Params;

/*!
 *  @brief Default #AESCTR_Params structure
 *
 *  @sa     #AESCTR_Params_init()
 */
extern const AESCTR_Params AESCTR_defaultParams;

/*!
 *  @brief  This function initializes the CTR module.
 *
 *  @pre    The AESCTR_config structure must exist and be persistent before this
 *          function can be called. This function must also be called before
 *          any other CTR driver APIs. This function call does not modify any
 *          peripheral registers.
 */
void AESCTR_init(void);

/*!
 *  @brief  Function to initialize the #AESCTR_Params struct to its defaults
 *
 *  @param  params      An pointer to #AESCTR_Params structure for
 *                      initialization
 *
 *  Defaults values are:
 *      returnBehavior              = AESCTR_RETURN_BEHAVIOR_BLOCKING
 *      callbackFxn                 = NULL
 *      timeout                     = SemaphoreP_WAIT_FOREVER
 *      custom                      = NULL
 */
void AESCTR_Params_init(AESCTR_Params* params);

/*!
 *  @brief  This function opens a given AESCTR peripheral.
 *
 *  @pre    AESCTR driver has been initialized using #AESCTR_init()
 *
 *  @param  index         Logical peripheral number for the CTR indexed into
 *                        the AESCTR_config table
 *
 *  @param  params        Pointer to a parameter block, if NULL it will use
 *                        default values.
 *
 *  @return A #AESCTR_Handle on success or a NULL on an error or if it has
 *          been opened already.
 *
 *  @sa     #AESCTR_init()
 *  @sa     #AESCTR_close()
 */
AESCTR_Handle AESCTR_open(uint_least8_t index, const AESCTR_Params* params);

/*!
 *  @brief  Function to close a CTR peripheral specified by the CTR handle
 *
 *  @pre    #AESCTR_open() has to be called first.
 *
 *  @param  handle A CTR handle returned from #AESCTR_open()
 *
 *  @sa     #AESCTR_open()
 */
void AESCTR_close(AESCTR_Handle handle);

/*!
 *  @brief  Function to initialize an #AESCTR_Operation struct to its defaults
 *
 *  @param  operationStruct     A pointer to an #AESCTR_Operation structure for
 *                              initialization
 *
 *  Defaults values are all zeros.
 */
void AESCTR_Operation_init(AESCTR_Operation* operationStruct);

/*!
 *  @brief  Function to perform an AESCTR encryption operation in one call.
 *
 *  @note   None of the buffers provided as arguments may be altered by the application during an ongoing operation.
 *          Doing so can yield corrupted ciphertext.
 *
 *  @pre    #AESCTR_open() and #AESCTR_Operation_init() must be called first.
 *
 *  @param  [in] handle                 A CTR handle returned from #AESCTR_open()
 *
 *  @param  [in] operationStruct        A pointer to a struct containing the parameters required to perform the operation.
 *
 *  @retval #AESCTR_STATUS_SUCCESS               The operation succeeded.
 *  @retval #AESCTR_STATUS_ERROR                 The operation failed.
 *  @retval #AESCTR_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was not available. Try again later.
 *  @retval #AESCTR_STATUS_CANCELED              The operation was canceled.
 *
 *  @sa     #AESCTR_oneStepDecrypt()
 */
int_fast16_t AESCTR_oneStepEncrypt(AESCTR_Handle handle, AESCTR_Operation* operationStruct);

/*!
 *  @brief  Function to perform an AESCTR decryption operation in one call.
 *
 *  @note   None of the buffers provided as arguments may be altered by the application during an ongoing operation.
 *          Doing so can yield corrupted plaintext.
 *
 *  @pre    AESCTR_open() and AESCTR_Operation_init() must be called first.
 *
 *  @param  [in] handle                 A CTR handle returned from AESCTR_open()
 *
 *  @param  [in] operationStruct        A pointer to a struct containing the parameters required to perform the operation.
 *
 *  @retval #AESCTR_STATUS_SUCCESS               The operation succeeded.
 *  @retval #AESCTR_STATUS_ERROR                 The operation failed.
 *  @retval #AESCTR_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was not available. Try again later.
 *  @retval #AESCTR_STATUS_CANCELED
 *
 *  @sa     AESCTR_oneStepEncrypt()
 */
int_fast16_t AESCTR_oneStepDecrypt(AESCTR_Handle handle, AESCTR_Operation* operationStruct);

/*!
 *  @brief Cancels an ongoing AESCTR operation.
 *
 *  Asynchronously cancels an AESCTR operation. Only available when using
 *  AESCTR_RETURN_BEHAVIOR_CALLBACK or AESCTR_RETURN_BEHAVIOR_BLOCKING.
 *  The operation will terminate as though an error occured. The
 *  return status code of the operation will be AESCTR_STATUS_CANCELED.
 *
 *  @param  handle Handle of the operation to cancel
 *
 *  @retval #AESCTR_STATUS_SUCCESS               The operation was canceled.
 *  @retval #AESCTR_STATUS_ERROR                 The operation was not canceled. There may be no operation to cancel.
 */
int_fast16_t AESCTR_cancelOperation(AESCTR_Handle handle);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_AESCTR__include */
