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
 *  @file       AESCBC.h
 *
 *  @brief      AESCBC driver header
 *
 *  @warning     This is a beta API. It may change in future releases.
 *
 *  @anchor ti_drivers_AESCBC_Overview
 *  # Overview #
 *  The Cipher Block Chaining (CBC) mode of operation is a generic
 *  block cipher mode of operation. It can be used with any block cipher
 *  including AES.
 *
 *  CBC mode encrypts messages of any practical length that have a length
 *  evenly divisibly by the block size. Unlike ECB, it guarantees
 *  confidentiality of the entire message when the message is larger than
 *  one block.
 *
 *  ## Operation #
 *  In CBC encryption, the initialization vector (IV) is XOR'd with a block of
 *  plaintext and then encrypted. The output ciphertext block is then XOR'd with
 *  the next plaintext block and the result is encryped. This process is repeated
 *  until the final block of plaintext has been encrypted.
 *
 *  To decrypt the message, decrypt the first block of ciphertext and XOR the result
 *  with the IV. The result is the first plaintext block. For subsequent ciphertext
 *  blocks, decrypt each block and XOR the previous block of the encrypted message
 *  into the result.
 *
 *  ## Padding #
 *  CBC operates on entire blocks of ciphertext and plaintext at a time. This
 *  means that message lengths must be a multiple of the block cipher block size.
 *  AES has a block size of 16 bytes no matter the key size. Since messages do
 *  not necessarily always have a length that is a multiple of 16 bytes, it may
 *  be necessary to pad the message to a 16-byte boundary. Padding requires
 *  the sender and receiver to implicitly agree on the padding convention.
 *  Improperly designed or implemented padding schemes may leak information
 *  to an attacker through a padding oracle attack for example.
 *
 *  ## Initialization Vectors #
 *  The IV is generated by the party performing the encryption operation.
 *  Within the scope of any encryption key, the IV value must be unique.
 *  The IV does not need to be kept secret and is usually transmitted together
 *  with the ciphertext to the decryting party.
 *  In CBC mode, the IVs must not be predictable. Two recommended ways to
 *  generate IVs is to either:
 *
 *  - Apply the block cipher (AESECB), using the same key used with CBC,
 *    to a nonce. This nonce must be unique for each key-message pair.
 *    A counter will usually suffice. If the same symmetric key is used
 *    by both parties to encrypt messages, they should agree to use a
 *    nonce scheme that avoids generating the same nonce and thus IV twice.
 *    Incrementing the counter by two and making one party use even numbers
 *    and the other odd numbers is a common method to avoid such collisions.
 *  - Use a TRNG (True Random Number Generator) or PRNG
 *    (Pseudo-Random Number Generator) to generate a random number for use
 *    as IV.
 *
 *  ## Drawbacks #
 *  CBC mode has several drawbacks. Unless interfacing with legacy devices,
 *  it is recommended to use an AEAD (Authenticated Encryption with Associated Data)
 *  mode such as CCM or GCM. Below is a non-exhaustive list of reasons to use
 *  a different block cipher mode of operation.
 *
 *  - CBC mode does not offer authentication or integrity guarantees. In practice,
 *    this means that attackers can intercept the encrypted message and manipulate
 *    the ciphertext before sending the message on to the receiver. While this
 *    does not break confidentiality and reveal the plaintext, it has enabled several
 *    attacks in the past. This is especially problematic given that changing the
 *    ciphertext of a block will only corrupt the block itself and the subsequent
 *    block of resultant plaintext. This property may be used to manipulate only
 *    certain parts of the message.
 *
 *  - CBC mode requires message lengths to be evenly divisible by the block size.
 *    This necessitates a padding scheme. Improperly implemented padding schemes
 *    may lead to vulnerabilities that can be exploited by attackers. It often
 *    makes more sense to use a dedicated stream cipher such as CTR (Counter) that
 *    does not have this restriction. CCM and GCM both use CTR for encryption.
 *
 *  @anchor ti_drivers_AESCBC_Usage
 *  # Usage #
 *  ## Before starting a CBC operation #
 *
 *  Before starting a CBC operation, the application must do the following:
 *      - Call #AESCBC_init() to initialize the driver
 *      - Call #AESCBC_Params_init() to initialize the #AESCBC_Params to default values.
 *      - Modify the #AESCBC_Params as desired
 *      - Call #AESCBC_open() to open an instance of the driver
 *      - Initialize a CryptoKey. These opaque data structures are representations
 *        of keying material and its storage. Depending on how the keying material
 *        is stored (RAM or flash, key store, key blob), the CryptoKey must be
 *        initialized differently. The AESCBC API can handle all types of CryptoKey.
 *        However, not all device-specific implementions support all types of CryptoKey.
 *        Devices without a key store will not support CryptoKeys with keying material
 *        stored in a key store for example.
 *        All devices support plaintext CryptoKeys.
 *      - Initialise the #AESCBC_Operation using #AESCBC_Operation_init() and set all
 *        length, key, and buffer fields.
 *
 *  ## Starting a CBC operation #
 *
 *  The #AESCBC_oneStepEncrypt and #AESCBC_oneStepDecrypt functions perform a CBC operation
 *  in a single call. They will always be the most highly optimized routines with the
 *  least overhead and the fastest runtime. However, they require all plaintext
 *  or ciphertext to be available to the function at the start of the call.
 *  All devices support single call operations.
 *
 *  ## After the CBC operation completes #
 *
 *  After the CBC operation completes, the application should either start
 *  another operation or close the driver by calling #AESCBC_close().
 *
 *  @anchor ti_drivers_AESCBC_Synopsis
 *  ## Synopsis
 *  @anchor ti_drivers_AESCBC_Synopsis_Code
 *  @code
 *  // Import AESCBC Driver definitions
 *  #include <ti/drivers/AESCBC.h>
 *
 *  // Define name for AESCBC channel index
 *  #define AESCBC_INSTANCE 0
 *
 *  AESCBC_init();
 *
 *  handle = AESCBC_open(AESCBC_INSTANCE, NULL);
 *
 *  // Initialize symmetric key
 *  CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
 *
 *  // Set up AESCBC_Operation
 *  AESCBC_Operation_init(&operation);
 *  operation.key               = &cryptoKey;
 *  operation.input             = plaintext;
 *  operation.output            = ciphertext;
 *  operation.inputLength       = sizeof(plaintext);
 *  operation.iv                = iv;
 *
 *  encryptionResult = AESCBC_oneStepEncrypt(handle, &operation);
 *
 *  AESCBC_close(handle);
 *  @endcode
 *
 *  @anchor ti_drivers_AESCBC_Examples
 *  ## Examples
 *
 *  ### Single call CBC encryption with plaintext CryptoKey in blocking return mode #
 *  @code
 *
 *  #include <ti/drivers/AESCBC.h>
 *  #include <ti/drivers/types/cryptoKey/CryptoKey_Plaintext.h>
 *
 *  ...
 *
 *  AESCBC_Handle handle;
 *  CryptoKey cryptoKey;
 *  int_fast16_t encryptionResult;
 *
 *  // For example purposes only. Generate IVs in a non-static way in practice.
 *  // Test vector 0 from NIST CAPV set CBCMMT128
 *  uint8_t iv[16] =                {0x2f, 0xe2, 0xb3, 0x33, 0xce, 0xda, 0x8f, 0x98,
 *                                   0xf4, 0xa9, 0x9b, 0x40, 0xd2, 0xcd, 0x34, 0xa8};
 *  uint8_t plaintext[16] =         {0x45, 0xcf, 0x12, 0x96, 0x4f, 0xc8, 0x24, 0xab,
 *                                   0x76, 0x61, 0x6a, 0xe2, 0xf4, 0xbf, 0x08, 0x22};
 *  uint8_t ciphertext[sizeof(plaintext)];
 *  uint8_t keyingMaterial[16] =    {0x1f, 0x8e, 0x49, 0x73, 0x95, 0x3f, 0x3f, 0xb0,
 *                                   0xbd, 0x6b, 0x16, 0x66, 0x2e, 0x9a, 0x3c, 0x17};
 *
 *  // The ciphertext should be the following after the encryption operation:
 *  //  0x0f, 0x61, 0xc4, 0xd4, 0x4c, 0x51, 0x47, 0xc0
 *  //  0x3c, 0x19, 0x5a, 0xd7, 0xe2, 0xcc, 0x12, 0xb2
 *
 *
 *  handle = AESCBC_open(0, NULL);
 *
 *  if (handle == NULL) {
 *      // handle error
 *  }
 *
 *  CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
 *
 *  AESCBC_Operation operation;
 *  AESCBC_Operation_init(&operation);
 *
 *  operation.key               = &cryptoKey;
 *  operation.input             = plaintext;
 *  operation.output            = ciphertext;
 *  operation.inputLength       = sizeof(plaintext);
 *  operation.iv                = iv;
 *
 *  encryptionResult = AESCBC_oneStepEncrypt(handle, &operation);
 *
 *  if (encryptionResult != AESCBC_STATUS_SUCCESS) {
 *      // handle error
 *  }
 *
 *  AESCBC_close(handle);
 *
 *  @endcode
 *
 *  ### Single call CBC decryption with plaintext CryptoKey in callback return mode #
 *  @code
 *
 *  #include <ti/drivers/AESCBC.h>
 *  #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
 *
 *  ...
 *
 *  // Test vector 0 from NIST CAPV set CBCMMT256
 *
 *  uint8_t iv[16] =                {0xdd, 0xbb, 0xb0, 0x17, 0x3f, 0x1e, 0x2d, 0xeb,
 *                                   0x23, 0x94, 0xa6, 0x2a, 0xa2, 0xa0, 0x24, 0x0e};
 *  uint8_t ciphertext[16] =        {0xd5, 0x1d, 0x19, 0xde, 0xd5, 0xca, 0x4a, 0xe1,
 *                                   0x4b, 0x2b, 0x20, 0xb0, 0x27, 0xff, 0xb0, 0x20};
 *  uint8_t keyingMaterial[] =      {0x43, 0xe9, 0x53, 0xb2, 0xae, 0xa0, 0x8a, 0x3a,
 *                                   0xd5, 0x2d, 0x18, 0x2f, 0x58, 0xc7, 0x2b, 0x9c,
 *                                   0x60, 0xfb, 0xe4, 0xa9, 0xca, 0x46, 0xa3, 0xcb,
 *                                   0x89, 0xe3, 0x86, 0x38, 0x45, 0xe2, 0x2c, 0x9e};
 *  uint8_t plaintext[sizeof(ciphertext)];
 *
 *  // The plaintext should be the following after the decryption operation:
 *  //  0x07, 0x27, 0x0d, 0x0e, 0x63, 0xaa, 0x36, 0xda
 *  //  0xed, 0x8c, 0x6a, 0xde, 0x13, 0xac, 0x1a, 0xf1
 *
 *
 *  void cbcCallback(AESCBC_Handle handle,
 *                   int_fast16_t returnValue,
 *                   AESCBC_Operation *operation,
 *                   AESCBC_OperationType operationType) {
 *
 *      if (returnValue != AESCBC_STATUS_SUCCESS) {
 *          // handle error
 *      }
 *  }
 *
 *  AESCBC_Operation operation;
 *
 *  void cbcStartFunction(void) {
 *      AESCBC_Handle handle;
 *      AESCBC_Params params;
 *      CryptoKey cryptoKey;
 *      int_fast16_t decryptionResult;
 *
 *      AESCBC_Params_init(&params);
 *      params.returnBehavior = AESCBC_RETURN_BEHAVIOR_CALLBACK;
 *      params.callbackFxn = cbcCallback;
 *
 *      handle = AESCBC_open(0, &params);
 *
 *      if (handle == NULL) {
 *          // handle error
 *      }
 *
 *      CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
 *
 *      AESCBC_Operation_init(&operation);
 *
 *      operation.key               = &cryptoKey;
 *      operation.input             = plaintext;
 *      operation.output            = ciphertext;
 *      operation.inputLength       = sizeof(plaintext);
 *      operation.iv                = iv;
 *
 *      decryptionResult = AESCBC_oneStepDecrypt(handle, &operation);
 *
 *      if (decryptionResult != AESCBC_STATUS_SUCCESS) {
 *          // handle error
 *      }
 *
 *      // do other things while CBC operation completes in the background
 *  }
 *
 *  @endcode
 */

#ifndef ti_drivers_AESCBC__include
#define ti_drivers_AESCBC__include

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

/*!
 * Common AESCBC status code reservation offset.
 * AESCBC driver implementations should offset status codes with
 * #AESCBC_STATUS_RESERVED growing negatively.
 *
 * Example implementation specific status codes:
 * @code
 * #define AESCBCXYZ_STATUS_ERROR0    AESCBC_STATUS_RESERVED - 0
 * #define AESCBCXYZ_STATUS_ERROR1    AESCBC_STATUS_RESERVED - 1
 * #define AESCBCXYZ_STATUS_ERROR2    AESCBC_STATUS_RESERVED - 2
 * @endcode
 */
#define AESCBC_STATUS_RESERVED        (-32)

/*!
 * @brief   Successful status code.
 *
 * Functions return #AESCBC_STATUS_SUCCESS if the function was executed
 * successfully.
 */
#define AESCBC_STATUS_SUCCESS         (0)

/*!
 * @brief   Generic error status code.
 *
 * Functions return #AESCBC_STATUS_ERROR if the function was not executed
 * successfully and no more pertinent error code could be returned.
 */
#define AESCBC_STATUS_ERROR           (-1)

/*!
 * @brief   An error status code returned if the hardware or software resource
 * is currently unavailable.
 *
 * AESCBC driver implementations may have hardware or software limitations on how
 * many clients can simultaneously perform operations. This status code is returned
 * if the mutual exclusion mechanism signals that an operation cannot currently be performed.
 */
#define AESCBC_STATUS_RESOURCE_UNAVAILABLE (-2)

/*!
 *  @brief  The ongoing operation was canceled.
 */
#define AESCBC_STATUS_CANCELED (-3)


/*!
 *  @brief  A handle that is returned from an #AESCBC_open() call.
 */
typedef struct AESCBC_Config*    AESCBC_Handle;

/*!
 * @brief   The way in which CBC function calls return after performing an
 * encryption or decryption operation.
 *
 * Not all CBC operations exhibit the specified return behavor. Functions that do not
 * require significant computation and cannot offload that computation to a background thread
 * behave like regular functions. Which functions exhibit the specfied return behavior is not
 * implementation dependent. Specifically, a software-backed implementation run on the same
 * CPU as the application will emulate the return behavior while not actually offloading
 * the computation to the background thread.
 *
 * AESCBC functions exhibiting the specified return behavior have restrictions on the
 * context from which they may be called.
 *
 * |                                | Task  | Hwi   | Swi   |
 * |--------------------------------|-------|-------|-------|
 * |AESCBC_RETURN_BEHAVIOR_CALLBACK | X     | X     | X     |
 * |AESCBC_RETURN_BEHAVIOR_BLOCKING | X     |       |       |
 * |AESCBC_RETURN_BEHAVIOR_POLLING  | X     | X     | X     |
 *
 */
typedef enum
{
    AESCBC_RETURN_BEHAVIOR_CALLBACK = 1,    /*!< The function call will return immediately while the
                                             *   CBC operation goes on in the background. The registered
                                             *   callback function is called after the operation completes.
                                             *   The context the callback function is called (task, HWI, SWI)
                                             *   is implementation-dependent.
                                             */
    AESCBC_RETURN_BEHAVIOR_BLOCKING = 2,    /*!< The function call will block while the CBC operation goes
                                             *   on in the background. CBC operation results are available
                                             *   after the function returns.
                                             */
    AESCBC_RETURN_BEHAVIOR_POLLING  = 4,    /*!< The function call will continuously poll a flag while CBC
                                             *   operation goes on in the background. CBC operation results
                                             *   are available after the function returns.
                                             */
} AESCBC_ReturnBehavior;

/*!
 *  @brief  Enum for the direction of the CBC operation.
 */
typedef enum
{
    AESCBC_MODE_ENCRYPT = 1,
    AESCBC_MODE_DECRYPT = 2,
} AESCBC_Mode;

/*!
 *  @brief  Struct containing the parameters required for encrypting/decrypting
 *          a message.
 */
typedef struct
{
    CryptoKey*                key;                       /*!< A previously initialized CryptoKey. */
    const uint8_t*            input;                     /*!<
                                                         *   - Encryption: The plaintext buffer to be
                                                         *     encrypted in the CBC operation.
                                                         *   - Decryption: The ciphertext to be decrypted.
                                                         */
    uint8_t*                  output;                    /*!<
                                                         *   - Encryption: The output ciphertext buffer that
                                                         *     the encrypted plaintext is copied to.
                                                         *   - Decryption: The plaintext derived from the
                                                         *     decrypted ciphertext is copied here.
                                                         */
    uint8_t*                  iv;                        /*!< A buffer containing an IV. IVs must be unique to
                                                         *   each CBC operation and may not be reused. If
                                                         *   ivInternallyGenerated is set, the iv will be
                                                         *   generated by this function call and copied to
                                                         *   this buffer.
                                                         */
    size_t                   inputLength;                /*!< Length of the input and output in bytes. */
    bool                     ivInternallyGenerated;      /*!< When true, the iv buffer passed into #AESCBC_oneStepEncrypt() functions
                                                         *   will be overwritten with a randomly generated iv.
                                                         *   Not supported by all implementations.
                                                         */
} AESCBC_Operation;

/*!
 *  @brief  Enum for the operation types supported by the driver.
 */
typedef enum
{
    AESCBC_OPERATION_TYPE_ENCRYPT = 1,
    AESCBC_OPERATION_TYPE_DECRYPT = 2,
} AESCBC_OperationType;

/*!
 *  @brief AESCBC Global configuration
 *
 *  The #AESCBC_Config structure contains a set of pointers used to characterize
 *  the AESCBC driver implementation.
 *
 *  This structure needs to be defined before calling #AESCBC_init() and it must
 *  not be changed thereafter.
 *
 *  @sa     #AESCBC_init()
 */
typedef struct AESCBC_Config
{
    /*! Pointer to a driver specific data object */
    void*               object;

    /*! Pointer to a driver specific hardware attributes structure */
    void         const* hwAttrs;
} AESCBC_Config;

/*!
 *  @brief  The definition of a callback function used by the AESCBC driver
 *          when used in ::AESCBC_RETURN_BEHAVIOR_CALLBACK
 *
 *  @param  handle Handle of the client that started the CBC operation.
 *
 *  @param  returnValue  The result of the CBC operation. May contain an error code.
 *                       Informs the application of why the callback function was
 *                       called.
 *
 *  @param  operation A pointer to an operation struct.
 *
 *  @param  operationType This parameter determines which operation the
 *          callback refers to.
 */
typedef void (*AESCBC_CallbackFxn) (AESCBC_Handle handle,
                                    int_fast16_t returnValue,
                                    AESCBC_Operation* operation,
                                    AESCBC_OperationType operationType);

/*!
 *  @brief  CBC Parameters
 *
 *  CBC Parameters are used to with the #AESCBC_open() call. Default values for
 *  these parameters are set using #AESCBC_Params_init().
 *
 *  @sa     #AESCBC_Params_init()
 */
typedef struct
{
    AESCBC_ReturnBehavior   returnBehavior;             /*!< Blocking, callback, or polling return behavior */
    AESCBC_CallbackFxn      callbackFxn;                /*!< Callback function pointer */
    uint32_t                timeout;                    /*!< Timeout before the driver returns an error in
                                                         *   ::AESCBC_RETURN_BEHAVIOR_BLOCKING
                                                         */
    void*                   custom;                     /*!< Custom argument used by driver
                                                         *   implementation
                                                         */
} AESCBC_Params;

/*!
 *  @brief Default #AESCBC_Params structure
 *
 *  @sa     #AESCBC_Params_init()
 */
extern const AESCBC_Params AESCBC_defaultParams;

/*!
 *  @brief  This function initializes the CBC module.
 *
 *  @pre    The AESCBC_config structure must exist and be persistent before this
 *          function can be called. This function must also be called before
 *          any other CBC driver APIs. This function call does not modify any
 *          peripheral registers.
 */
void AESCBC_init(void);

/*!
 *  @brief  Function to initialize the #AESCBC_Params struct to its defaults
 *
 *  @param  params      An pointer to #AESCBC_Params structure for
 *                      initialization
 *
 *  Defaults values are:
 *      returnBehavior              = AESCBC_RETURN_BEHAVIOR_BLOCKING
 *      callbackFxn                 = NULL
 *      timeout                     = SemaphoreP_WAIT_FOREVER
 *      custom                      = NULL
 */
void AESCBC_Params_init(AESCBC_Params* params);

/*!
 *  @brief  This function opens a given CBC peripheral.
 *
 *  @pre    CBC controller has been initialized using #AESCBC_init()
 *
 *  @param  index         Logical peripheral number for the CBC indexed into
 *                        the AESCBC_config table
 *
 *  @param  params        Pointer to an parameter block, if NULL it will use
 *                        default values.
 *
 *  @return An #AESCBC_Handle on success or a NULL on an error or if it has
 *          been opened already.
 *
 *  @sa     #AESCBC_init()
 *  @sa     #AESCBC_close()
 */
AESCBC_Handle AESCBC_open(uint_least8_t index, AESCBC_Params* params);

/*!
 *  @brief  Function to close a CBC peripheral specified by the CBC handle
 *
 *  @pre    #AESCBC_open() has to be called first.
 *
 *  @param  handle A CBC handle returned from #AESCBC_open()
 *
 *  @sa     #AESCBC_open()
 */
void AESCBC_close(AESCBC_Handle handle);

/*!
 *  @brief  Function to initialize an #AESCBC_Operation struct to its defaults
 *
 *  @param  operationStruct     A pointer to an #AESCBC_Operation structure for
 *                              initialization
 *
 *  Defaults values are all zeros.
 */
void AESCBC_Operation_init(AESCBC_Operation* operationStruct);

/*!
 *  @brief  Function to perform an AESCBC encryption operation in one call.
 *
 *  @note   None of the buffers provided as arguments may be altered by the application during an ongoing operation.
 *          Doing so can yield corrupted ciphertext.
 *
 *  @pre    #AESCBC_open() and #AESCBC_Operation_init() must be called first.
 *
 *  @param  [in] handle                 A CBC handle returned from #AESCBC_open()
 *
 *  @param  [in] operationStruct        A pointer to a struct containing the parameters required to perform the operation.
 *
 *  @retval #AESCBC_STATUS_SUCCESS               The operation succeeded.
 *  @retval #AESCBC_STATUS_ERROR                 The operation failed.
 *  @retval #AESCBC_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was not available. Try again later.
 *  @retval #AESCBC_STATUS_CANCELED              The operation was canceled.
 *
 *  @sa     #AESCBC_oneStepDecrypt()
 */
int_fast16_t AESCBC_oneStepEncrypt(AESCBC_Handle handle, AESCBC_Operation* operationStruct);

/*!
 *  @brief  Function to perform an AESCBC decryption operation in one call.
 *
 *  @note   None of the buffers provided as arguments may be altered by the application during an ongoing operation.
 *          Doing so can yield corrupted plaintext.
 *
 *  @pre    AESCBC_open() and AESCBC_Operation_init() must be called first.
 *
 *  @param  [in] handle                 A CBC handle returned from AESCBC_open()
 *
 *  @param  [in] operationStruct        A pointer to a struct containing the parameters required to perform the operation.
 *
 *  @retval #AESCBC_STATUS_SUCCESS               The operation succeeded.
 *  @retval #AESCBC_STATUS_ERROR                 The operation failed.
 *  @retval #AESCBC_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was not available. Try again later.
 *  @retval #AESCBC_STATUS_CANCELED              The operation was canceled.
 *
 *  @sa     AESCBC_oneStepEncrypt()
 */
int_fast16_t AESCBC_oneStepDecrypt(AESCBC_Handle handle, AESCBC_Operation* operationStruct);

/*!
 *  @brief Cancels an ongoing AESCBC operation.
 *
 *  Asynchronously cancels an AESCBC operation. Only available when using
 *  AESCBC_RETURN_BEHAVIOR_CALLBACK or AESCBC_RETURN_BEHAVIOR_BLOCKING.
 *  The operation will terminate as though an error occured. The
 *  return status code of the operation will be AESCBC_STATUS_CANCELED.
 *
 *  @param  [in] handle Handle of the operation to cancel
 *
 *  @retval #AESCBC_STATUS_SUCCESS               The operation was canceled.
 *  @retval #AESCBC_STATUS_ERROR                 The operation was not canceled. There may be no operation to cancel.
 */
int_fast16_t AESCBC_cancelOperation(AESCBC_Handle handle);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_AESCBC__include */
