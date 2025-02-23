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
/*!****************************************************************************
 *  @file       AESCCM.h
 *
 *  @brief      AESCCM driver header
 *
 * @warning     This is a beta API. It may change in future releases.
 *
 *  @anchor ti_drivers_AESCCM_Overview
 *  # Overview #
 *  The Counter with CBC-MAC (CCM) mode of operation is a generic
 *  authenticated encryption block cipher mode.  It can be used with
 *  any block cipher.
 *  AESCCM combines CBC-MAC with an AES block cipher in CTR mode of operation.
 *
 *  This combination of block cipher modes enables CCM to encrypt messages of any
 *  length and not only multiples of the block cipher block size.
 *
 *  CTR provides confidentiality. The defined application of CBC-MAC provides
 *  message integrity and authentication.
 *
 *  AESCCM has the following inputs and outputs:
 *
 * <table>
 * <caption id="AESCCM_multi_row">AES-CCM input and output parameters</caption>
 * <tr><th>Encryption</th><th>Decryption</th></tr>
 * <tr><th colspan=2>Input</th></tr>
 * <tr><td>Shared AES key</td><td> Shared AES key</td></tr>
 * <tr><td>Nonce</td><td>Nonce</td></tr>
 * <tr><td>Cleartext</td><td>Ciphertext</td></tr>
 * <tr><td></td><td>MAC</td></tr>
 * <tr><td>AAD (optional)</td><td>AAD (optional)</td></tr>
 * <tr><th colspan=2>Output</th></tr>
 * <tr><td>Ciphertext</td><td>Cleartext</td></tr>
 * <tr><td>MAC</td><td></td></tr>
 * </table>
 *
 *  The AES key is a shared secret between the two parties and has a length
 *  between 128, 192, or 256 bits.
 *
 *  The nonce is generated by the party performing the authenticated
 *  encryption operation.  Within the scope of any authenticated
 *  encryption key, the nonce value must be unique.  That is, the set of
 *  nonce values used with any given key must not contain any duplicate
 *  values.  Using the same nonce for two different messages encrypted
 *  with the same key destroys the security properties.
 *
 *  The length of the nonce determines the maximum number of messages that may
 *  be encrypted and authenticated before you must regenerate the key.
 *  Reasonable session key rotation schemes will regenerate the key before reaching
 *  this limit.
 *  There is a trade-off between the nonce-length and the maximum length of
 *  the plaintext to encrypt and authenticate per nonce. This is because
 *  CTR concatenates the nonce and an internal counter into one 16-byte
 *  IV. The counter is incremented after generating an AES-block-sized
 *  pseudo-random bitstream. This bitstream is XOR'd with the plaintext.
 *  The counter would eventually roll over for a sufficiently long message.
 *  This is must not happen. Hence, the longer the nonce and the more messages
 *  you can send before needing to rotate the key, the shorter the
 *  lengths of invidual messages sent may be. The minimum and maximum
 *  nonce length defined by the CCM standard provide for both a reasonable
 *  number of messages before key rotation and a reasonable maximum message length.
 *  Check NIST SP 800-38C for details.
 *
 *  The optional additional authentication data (AAD) is authenticated
 *  but not encrypted. Thus, the AAD is not included in the AES-CCM output.
 *  It can be used to authenticate packet headers.
 *
 *  After the encryption operation, the ciphertext contains the encrypted
 *  data. The message authentication code (MAC) is also provided.
 *
 *  # CCM Variations #
 *  The AESCCM driver supports both classic CCM as defined by NIST SP 800-38C and
 *  the CCM* variant used in IEEE 802.15.4.
 *  CCM* allows for unauthenticated encryption using CCM by permitting a MAC length
 *  of 0. It also imposes the requirement that the MAC length be embedded in
 *  the nonce used for each message if the MAC length varies within the protocol
 *  using CCM*.
 *
 *  @anchor ti_drivers_AESCCM_Usage
 *  # Usage #
 *
 *  ## Before starting a CCM operation #
 *
 *  Before starting a CCM operation, the application must do the following:
 *      - Call AESCCM_init() to initialize the driver
 *      - Call AESCCM_Params_init() to initialize the AESCCM_Params to default values.
 *      - Modify the AESCCM_Params as desired
 *      - Call AESCCM_open() to open an instance of the driver
 *      - Initialize a CryptoKey. These opaque datastructures are representations
 *        of keying material and its storage. Depending on how the keying material
 *        is stored (RAM or flash, key store, key blob), the CryptoKey must be
 *        initialized differently. The AESCCM API can handle all types of CryptoKey.
 *        However, not all device-specific implementions support all types of CryptoKey.
 *        Devices without a key store will not support CryptoKeys with keying material
 *        stored in a key store for example.
 *        All devices support plaintext CryptoKeys.
 *      - Initialise the AESCCM_Operation using AESCCM_Operation_init() and set all
 *        length, key, and buffer fields.
 *
 *  ## Starting a CCM operation #
 *
 *  The AESCCM_oneStepEncrypt and AESCCM_oneStepDecrypt functions do a CCM operation in a single call.
 *  They will always be the most highly optimized routines with the least overhead and the fastest
 *  runtime. However, they require all AAD and plaintext or ciphertext data to be
 *  available to the function at the start of the call.
 *  All devices support single call operations.
 *
 *  When performing a decryption operation with AESCCM_oneStepDecrypt(), the MAC is
 *  automatically verified.
 *
 *  ## After the CCM operation completes #
 *
 *  After the CCM operation completes, the application should either start another operation
 *  or close the driver by calling AESCCM_close()
 *
 *  @anchor ti_drivers_AESCCM_Synopsis
 *  ## Synopsis
 *
 *  @anchor ti_drivers_AESCCM_Synopsis_Code
 *  @code
 *
 *  // Import AESCCM Driver definitions
 *  #include <ti/drivers/AESCCM.h>
 *
 *  // Define name for AESCCM channel index
 *  #define AESCCM_INSTANCE 0
 *
 *  AESCCM_init();
 *
 *  handle = AESCCM_open(AESCCM_INSTANCE, NULL);
 *
 *  // Initialize symmetric key
 *  CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
 *
 *  // Set up AESCCM_Operation
 *  AESCCM_Operation_init(&operation);
 *  operation.key               = &cryptoKey;
 *  operation.aad               = aad;
 *  operation.aadLength         = sizeof(aad);
 *  operation.input             = plaintext;
 *  operation.output            = ciphertext;
 *  operation.inputLength       = sizeof(plaintext);
 *  operation.nonce             = nonce;
 *  operation.nonceLength       = sizeof(nonce);
 *  operation.mac               = mac;
 *  operation.macLength         = sizeof(mac);
 *
 *  encryptionResult = AESCCM_oneStepEncrypt(handle, &operation);
 *
 *  AESCCM_close(handle);
 *  @endcode
 *
 *  @anchor ti_drivers_AESCCM_Examples
 *  ## Examples
 *
 *  ### Single call CCM encryption + authentication with plaintext CryptoKey in blocking return mode #
 *  @code
 *
 *  #include <ti/drivers/AESCCM.h>
 *  #include <ti/drivers/types/cryptoKey/CryptoKey_Plaintext.h>
 *
 *  ...
 *
 *  AESCCM_Handle handle;
 *  CryptoKey cryptoKey;
 *  int_fast16_t encryptionResult;
 *  uint8_t nonce[] = "Thisisanonce";
 *  uint8_t aad[] = "This string will be authenticated but not encrypted.";
 *  uint8_t plaintext[] = "This string will be encrypted and authenticated.";
 *  uint8_t mac[16];
 *  uint8_t ciphertext[sizeof(plaintext)];
 *  uint8_t keyingMaterial[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
 *                                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 *                                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
 *                                0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F}
 *
 *  handle = AESCCM_open(0, NULL);
 *
 *  if (handle == NULL) {
 *      // handle error
 *  }
 *
 *  CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
 *
 *      AESCCM_Operation operation;
 *      AESCCM_Operation_init(&operation);
 *
 *      operation.key               = &cryptoKey;
 *      operation.aad               = aad;
 *      operation.aadLength         = sizeof(aad);
 *      operation.input             = plaintext;
 *      operation.output            = ciphertext;
 *      operation.inputLength       = sizeof(plaintext);
 *      operation.nonce             = nonce;
 *      operation.nonceLength       = sizeof(nonce);
 *      operation.mac               = mac;
 *      operation.macLength         = sizeof(mac);
 *
 *  encryptionResult = AESCCM_oneStepEncrypt(handle, &operation);
 *
 *  if (encryptionResult != AESCCM_STATUS_SUCCESS) {
 *      // handle error
 *  }
 *
 *  AESCCM_close(handle);
 *
 *  @endcode
 *
 *  ### Single call CCM decryption + verification with plaintext CryptoKey in callback return mode #
 *  @code
 *
 *  #include <ti/drivers/AESCCM.h>
 *  #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
 *
 *  ...
 *
 *  // The following test vector is Packet Vector 1 from RFC 3610 of the IETF.
 *
 *  uint8_t nonce[]                         = {0x00, 0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0xA0,
 *                                             0xA1, 0xA2, 0xA3, 0xA4, 0xA5};
 *  uint8_t aad[]                           = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
 *  uint8_t mac[]                           = {0x17, 0xE8, 0xD1, 0x2C, 0xFD, 0xF9, 0x26, 0xE0};
 *  uint8_t ciphertext[]                    = {0x58, 0x8C, 0x97, 0x9A, 0x61, 0xC6, 0x63, 0xD2,
 *                                             0xF0, 0x66, 0xD0, 0xC2, 0xC0, 0xF9, 0x89, 0x80,
 *                                             0x6D, 0x5F, 0x6B, 0x61, 0xDA, 0xC3, 0x84};
 *  uint8_t keyingMaterial[]                = {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
 *                                             0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF};
 *  uint8_t plaintext[sizeof(ciphertext)];
 *
 *  // The plaintext should be the following after the decryption operation:
 *  //  {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 *  //  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
 *  //  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E}
 *
 *
 *  void ccmCallback(AESCCM_Handle handle,
 *                   int_fast16_t returnValue,
 *                   AESCCM_Operation *operation,
 *                   AESCCM_OperationType operationType) {
 *
 *      if (returnValue != AESCCM_STATUS_SUCCESS) {
 *          // handle error
 *      }
 *  }
 *
 *  AESCCM_Operation operation;
 *
 *  void ccmStartFunction(void) {
 *      AESCCM_Handle handle;
 *      AESCCM_Params params;
 *      CryptoKey cryptoKey;
 *      int_fast16_t decryptionResult;
 *
 *      AESCCM_Params_init(&params);
 *      params.returnBehavior = AESCCM_RETURN_BEHAVIOR_CALLBACK;
 *      params.callbackFxn = ccmCallback;
 *
 *      handle = AESCCM_open(0, &params);
 *
 *      if (handle == NULL) {
 *          // handle error
 *      }
 *
 *      CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
 *
 *      AESCCM_Operation_init(&operation);
 *
 *      operation.key               = &cryptoKey;
 *      operation.aad               = aad;
 *      operation.aadLength         = sizeof(aad);
 *      operation.input             = plaintext;
 *      operation.output            = ciphertext;
 *      operation.inputLength       = sizeof(plaintext);
 *      operation.nonce             = nonce;
 *      operation.nonceLength       = sizeof(nonce);
 *      operation.mac               = mac;
 *      operation.macLength         = sizeof(mac);
 *
 *      decryptionResult = AESCCM_oneStepDecrypt(handle, &operation);
 *
 *      if (decryptionResult != AESCCM_STATUS_SUCCESS) {
 *          // handle error
 *      }
 *
 *      // do other things while CCM operation completes in the background
 *
 *  }
 *
 *
 *  @endcode
 */

#ifndef ti_drivers_AESCCM__include
#define ti_drivers_AESCCM__include

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

/*!
 * Common AESCCM status code reservation offset.
 * AESCCM driver implementations should offset status codes with
 * AESCCM_STATUS_RESERVED growing negatively.
 *
 * Example implementation specific status codes:
 * @code
 * #define AESCCMXYZ_STATUS_ERROR0    AESCCM_STATUS_RESERVED - 0
 * #define AESCCMXYZ_STATUS_ERROR1    AESCCM_STATUS_RESERVED - 1
 * #define AESCCMXYZ_STATUS_ERROR2    AESCCM_STATUS_RESERVED - 2
 * @endcode
 */
#define AESCCM_STATUS_RESERVED (-32)

/*!
 * @brief   Successful status code.
 *
 * Functions return AESCCM_STATUS_SUCCESS if the function was executed
 * successfully.
 */
#define AESCCM_STATUS_SUCCESS (0)

/*!
 * @brief   Generic error status code.
 *
 * Functions return AESCCM_STATUS_ERROR if the function was not executed
 * successfully and no more pertinent error code could be returned.
 */
#define AESCCM_STATUS_ERROR (-1)

/*!
 * @brief   An error status code returned if the hardware or software resource
 * is currently unavailable.
 *
 * AESCCM driver implementations may have hardware or software limitations on how
 * many clients can simultaneously perform operations. This status code is returned
 * if the mutual exclusion mechanism signals that an operation cannot currently be performed.
 */
#define AESCCM_STATUS_RESOURCE_UNAVAILABLE (-2)

/*!
 * @brief   An error status code returned if the MAC provided by the application for
 *  a decryption operation does not match the one calculated during the operation.
 *
 * This code is returned by AESCCM_oneStepDecrypt() if the verification of the
 * MAC fails.
 */
#define AESCCM_STATUS_MAC_INVALID (-3)

/*!
 *  @brief  The ongoing operation was canceled.
 */
#define AESCCM_STATUS_CANCELED (-4)

/*!
 *  @brief  A handle that is returned from an AESCCM_open() call.
 */
typedef struct AESCCM_Config* AESCCM_Handle;

/*!
 * @brief   The way in which CCM function calls return after performing an
 * encryption + authentication or decryption + verification operation.
 *
 * Not all CCM operations exhibit the specified return behavor. Functions that do not
 * require significant computation and cannot offload that computation to a background thread
 * behave like regular functions. Which functions exhibit the specfied return behavior is not
 * implementation dependent. Specifically, a software-backed implementation run on the same
 * CPU as the application will emulate the return behavior while not actually offloading
 * the computation to the background thread.
 *
 * AESCCM functions exhibiting the specified return behavior have restrictions on the
 * context from which they may be called.
 *
 * |                                | Task  | Hwi   | Swi   |
 * |--------------------------------|-------|-------|-------|
 * |AESCCM_RETURN_BEHAVIOR_CALLBACK | X     | X     | X     |
 * |AESCCM_RETURN_BEHAVIOR_BLOCKING | X     |       |       |
 * |AESCCM_RETURN_BEHAVIOR_POLLING  | X     | X     | X     |
 *
 */
typedef enum
{
    AESCCM_RETURN_BEHAVIOR_CALLBACK = 1, /*!< The function call will return immediately while the
                                          *   CCM operation goes on in the background. The registered
                                          *   callback function is called after the operation completes.
                                          *   The context the callback function is called (task, HWI, SWI)
                                          *   is implementation-dependent.
                                          */
    AESCCM_RETURN_BEHAVIOR_BLOCKING = 2, /*!< The function call will block while the CCM operation goes
                                          *   on in the background. CCM operation results are available
                                          *   after the function returns.
                                          */
    AESCCM_RETURN_BEHAVIOR_POLLING = 4,  /*!< The function call will continuously poll a flag while CCM
                                          *   operation goes on in the background. CCM operation results
                                          *   are available after the function returns.
                                          */
} AESCCM_ReturnBehavior;

/*!
 *  @brief  Enum for the direction of the CCM operation.
 */
typedef enum
{
    AESCCM_MODE_ENCRYPT = 1,
    AESCCM_MODE_DECRYPT = 2,
} AESCCM_Mode;

/*!
 *  @brief  Struct containing the parameters required for encrypting/decrypting
 *          and authenticating/verifying a message.
 */
typedef struct
{
        CryptoKey* key;                /*!< A previously initialized CryptoKey */
        uint8_t* aad;                  /*!< A buffer of length \c aadLength containing additional
                                        *   authentication data to be authenticated/verified but not
                                        *   encrypted/decrypted.
                                        */
        uint8_t* input;                /*!<
                                        *   - Encryption: The plaintext buffer to be encrypted and authenticated
                                        *   in the CCM operation.
                                        *   - Decryption: The ciphertext to be decrypted and verified.
                                        */
        uint8_t* output;               /*!<
                                        *   - Encryption: The output ciphertext buffer that the encrypted plaintext
                                        *   is copied to.
                                        *   - Decryption: The plaintext derived from the decrypted and verified
                                        *   ciphertext is copied here.
                                        */
        uint8_t* nonce;                /*!< A buffer containing a nonce. Nonces must be unique to
                                        *   each CCM operation and may not be reused. If
                                        *   nonceInternallyGenerated is set the nonce will be
                                        *   generated by this function call and copied to
                                        *   this buffer.
                                        */
        uint8_t* mac;                  /*!<
                                        *   - Encryption: The buffer where the message authentication
                                        *   code is copied.
                                        *   - Decryption: The buffer containing the received message
                                        *   authentication code.
                                        */
        size_t aadLength;              /*!< Length of \c aad in bytes. Either \c aadLength or
                                        *   \c plaintextLength must benon-zero.
                                        *   encrypted.
                                        */
        size_t inputLength;            /*!< Length of the input and output in bytes. Either \c aadLength or
                                        *   \c inputLength must be
                                        *   non-zero.
                                        */
        uint8_t nonceLength;           /*!< Length of \c nonce in bytes.
                                        *   Valid nonce lengths are [7, 8, ... 13].
                                        */
        uint8_t macLength;             /*!< Length of \c mac in bytes.
                                        *   Valid MAC lengths are [0, 4, 6, 8, 10, 12, 14, 16].
                                        *   A length of 0 disables authentication and verification. This is
                                        *   only permitted when using CCM*.
                                        */
        bool nonceInternallyGenerated; /*!< When true, the nonce buffer passed into the AESCCM_setupEncrypt()
                                        *   and AESCCM_oneStepEncrypt() functions will be overwritten with a
                                        *   randomly generated nonce. Not supported by all implementations.
                                        */
} AESCCM_Operation;

/*!
 *  @brief  Enum for the operation types supported by the driver.
 */
typedef enum
{
    AESCCM_OPERATION_TYPE_ENCRYPT = 1,
    AESCCM_OPERATION_TYPE_DECRYPT = 2,
} AESCCM_OperationType;

/*!
 *  @brief AESCCM Global configuration
 *
 *  The AESCCM_Config structure contains a set of pointers used to characterize
 *  the AESCCM driver implementation.
 *
 *  This structure needs to be defined before calling AESCCM_init() and it must
 *  not be changed thereafter.
 *
 *  @sa     AESCCM_init()
 */
typedef struct AESCCM_Config
{
        /*! Pointer to a driver specific data object */
        void* object;

        /*! Pointer to a driver specific hardware attributes structure */
        void const* hwAttrs;
} AESCCM_Config;

/*!
 *  @brief  The definition of a callback function used by the AESCCM driver
 *          when used in ::AESCCM_RETURN_BEHAVIOR_CALLBACK
 *
 *  @param  handle Handle of the client that started the CCM operation.
 *
 *  @param  returnValue  The result of the CCM operation. May contain an error code.
 *                       Informs the application of why the callback function was
 *                       called.
 *
 *  @param  operation A pointer to an operation struct.
 *
 *  @param  operationType This parameter determines which operation the
 *          callback refers to.
 */
typedef void (*AESCCM_CallbackFxn)(AESCCM_Handle handle,
                                   int_fast16_t returnValue,
                                   AESCCM_Operation* operation,
                                   AESCCM_OperationType operationType);

/*!
 *  @brief  CCM Parameters
 *
 *  CCM Parameters are used to with the AESCCM_open() call. Default values for
 *  these parameters are set using AESCCM_Params_init().
 *
 *  @sa     AESCCM_Params_init()
 */
typedef struct
{
        AESCCM_ReturnBehavior returnBehavior; /*!< Blocking, callback, or polling return behavior */
        AESCCM_CallbackFxn callbackFxn;       /*!< Callback function pointer */
        uint32_t timeout;                     /*!< Timeout before the driver returns an error in
                                               *   ::AESCCM_RETURN_BEHAVIOR_BLOCKING
                                               */
        void* custom;                         /*!< Custom argument used by driver
                                               *   implementation
                                               */
} AESCCM_Params;

/*!
 *  @brief Default AESCCM_Params structure
 *
 *  @sa     AESCCM_Params_init()
 */
extern const AESCCM_Params AESCCM_defaultParams;

/*!
 *  @brief  This function initializes the CCM module.
 *
 *  @pre    The AESCCM_config structure must exist and be persistent before this
 *          function can be called. This function must also be called before
 *          any other CCM driver APIs. This function call does not modify any
 *          peripheral registers.
 */
void AESCCM_init(void);

/*!
 *  @brief  Function to initialize the AESCCM_Params struct to its defaults
 *
 *  @param  params      An pointer to AESCCM_Params structure for
 *                      initialization
 *
 *  Defaults values are:
 *      returnBehavior              = AESCCM_RETURN_BEHAVIOR_BLOCKING
 *      callbackFxn                 = NULL
 *      timeout                     = SemaphoreP_WAIT_FOREVER
 *      custom                      = NULL
 */
void AESCCM_Params_init(AESCCM_Params* params);

/*!
 *  @brief  This function opens a given CCM peripheral.
 *
 *  @pre    CCM controller has been initialized using AESCCM_init()
 *
 *  @param  index         Logical peripheral number for the CCM indexed into
 *                        the AESCCM_config table
 *
 *  @param  params        Pointer to an parameter block, if NULL it will use
 *                        default values.
 *
 *  @return An AESCCM_Handle on success or a NULL on an error or if it has been
 *          opened already.
 *
 *  @sa     AESCCM_init()
 *  @sa     AESCCM_close()
 */
AESCCM_Handle AESCCM_open(uint_least8_t index, AESCCM_Params* params);

/*!
 *  @brief  Function to close a CCM peripheral specified by the CCM handle
 *
 *  @pre    AESCCM_open() has to be called first.
 *
 *  @param  handle A CCM handle returned from AESCCM_open()
 *
 *  @sa     AESCCM_open()
 */
void AESCCM_close(AESCCM_Handle handle);

/*!
 *  @brief  Function to initialize an AESCCM_Operation struct to its defaults
 *
 *  @param  operationStruct     A pointer to an AESCCM_Operation structure for
 *                              initialization
 *
 *  Defaults values are all zeros.
 */
void AESCCM_Operation_init(AESCCM_Operation* operationStruct);

/*!
 *  @brief  Function to perform an AESCCM encryption + authentication operation in one call.
 *
 *  @note   None of the buffers provided as arguments may be altered by the application during an ongoing operation.
 *          Doing so can yield corrupted ciphertext or incorrect authentication.
 *
 *  @pre    AESCCM_open() and AESCCM_Operation_init() have to be called first.
 *
 *  @param  [in] handle                 A CCM handle returned from AESCCM_open()
 *
 *  @param  [in] operationStruct        A pointer to a struct containing the parameters required to perform the operation.
 *
 *  @retval #AESCCM_STATUS_SUCCESS               The operation succeeded.
 *  @retval #AESCCM_STATUS_ERROR                 The operation failed.
 *  @retval #AESCCM_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was not available. Try again later.
 *  @retval #AESCCM_STATUS_CANCELED              The operation was canceled.
 *
 *  @sa     AESCCM_oneStepDecrypt()
 */
int_fast16_t AESCCM_oneStepEncrypt(AESCCM_Handle handle, AESCCM_Operation* operationStruct);

/*!
 *  @brief  Function to perform an AESCCM decryption + verification operation in one call.
 *
 *  @note   None of the buffers provided as arguments may be altered by the application during an ongoing operation.
 *          Doing so can yield corrupted plaintext or incorrectly failed verification.
 *
 *  @pre    AESCCM_open() and AESCCM_Operation_init() have to be called first.
 *
 *  @param  [in] handle                 A CCM handle returned from AESCCM_open()
 *
 *  @param  [in] operationStruct        A pointer to a struct containing the parameters required to perform the operation.
 *
 *  @retval #AESCCM_STATUS_SUCCESS               The operation succeeded.
 *  @retval #AESCCM_STATUS_ERROR                 The operation failed.
 *  @retval #AESCCM_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was not available. Try again later.
 *  @retval #AESCCM_STATUS_CANCELED              The operation was canceled.
 *  @retval #AESCCM_STATUS_MAC_INVALID           The provided MAC did no match the recomputed one.
 *
 *  @sa     AESCCM_oneStepEncrypt()
 */
int_fast16_t AESCCM_oneStepDecrypt(AESCCM_Handle handle, AESCCM_Operation* operationStruct);

/*!
 *  @brief Cancels an ongoing AESCCM operation.
 *
 *  Asynchronously cancels an AESCCM operation. Only available when using
 *  AESCCM_RETURN_BEHAVIOR_CALLBACK or AESCCM_RETURN_BEHAVIOR_BLOCKING.
 *  The operation will terminate as though an error occured. The
 *  return status code of the operation will be AESCCM_STATUS_CANCELED.
 *
 *  @param  [in] handle Handle of the operation to cancel
 *
 *  @retval #AESCBC_STATUS_SUCCESS               The operation was canceled.
 *  @retval #AESCBC_STATUS_ERROR                 The operation was not canceled.
 */
int_fast16_t AESCCM_cancelOperation(AESCCM_Handle handle);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_AESCCM__include */
