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
 *  @file       TRNG.h
 *
 *  @brief      TRNG driver header
 *
 *  @warning    This is a beta API. It may change in future releases.
 *
 *  @anchor ti_drivers_TRNG_Overview
 *  # Overview #
 *  The True Random Number Generator (TRNG) module generates numbers of variable
 *  lengths from a source of entropy. The output is suitable for applications
 *  requiring cryptographically random numbers such as keying material for
 *  private or symmetric keys.
 *
 *  @anchor ti_drivers_TRNG_Usage
 *  # Usage #
 *
 *  ## Before starting a TRNG operation #
 *
 *  Before starting a TRNG operation, the application must do the following:
 *      - Call TRNG_init() to initialize the driver.
 *      - Call TRNG_Params_init() to initialize the TRNG_Params to default values.
 *      - Modify the TRNG_Params as desired.
 *      - Call TRNG_open() to open an instance of the driver.
 *      - Initialize a blank CryptoKey. These opaque datastructures are representations
 *        of keying material and its storage. Depending on how the keying material
 *        is stored (RAM or flash, key store, key blob), the CryptoKey must be
 *        initialized differently. The TRNG API can handle all types of CryptoKey.
 *        However, not all device-specific implementions support all types of CryptoKey.
 *        Devices without a key store will not support CryptoKeys with keying material
 *        stored in a key store for example.
 *        All devices support plaintext CryptoKeys.
 *
 *  ## TRNG operations #
 *
 *  TRNG_generateEntropy() provides the most basic functionality. Use it to
 *  generate random numbers of a specified width without further restrictions.
 *  An example use-case would be generating a symmetric key for AES encryption
 *  and / or authentication.
 *
 *  To generate an ECC private key, you should use rejection sampling to ensure
 *  that the keying material is in the interval [1, n - 1]. The ECDH public key
 *  genreation APIs will reject private keys that are outside of this interval.
 *  This information may be used to generate keying material until a suitable
 *  key is generated. For most curves, it is improbable to generate a random number
 *  outside of this interval because n is a large number close to the maximum
 *  number that would fit in the k-byte keying material array. An example
 *  of how to do this is given below.
 *
 *  ## After the TRNG operation completes #
 *
 *  After the TRNG operation completes, the application should either start another operation
 *  or close the driver by calling TRNG_close().
 *
 *  @anchor ti_drivers_TRNG_Synopsis
 *  ## Synopsis
 *  @anchor ti_drivers_TRNG_Synopsis_Code
 *  @code
 *  // Import TRNG Driver definitions
 *  #include <ti/drivers/TRNG.h>
 *  #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
 *
 *  // Define name for TRNG channel index
 *  #define TRNG_INSTANCE 0
 *
 *  #define KEY_LENGTH_BYTES 16
 *
 *  TRNG_init();
 *
 *  handle = TRNG_open(TRNG_INSTANCE, NULL);
 *
 *  CryptoKeyPlaintext_initBlankKey(&entropyKey, entropyBuffer, KEY_LENGTH_BYTES);
 *
 *  result = TRNG_generateEntropy(handle, &entropyKey);
 *
 *  TRNG_close(handle);
 *
 *  @endcode
 *
 *  @anchor ti_drivers_TRNG_Examples
 *  ## Examples
 *
 *  ### Generate symmetric encryption key #
 *  @code
 *
 *  #include <ti/drivers/TRNG.h>
 *  #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
 *
 *  #define KEY_LENGTH_BYTES 16
 *
 *  TRNG_Handle handle;
 *  int_fast16_t result;
 *
 *  CryptoKey entropyKey;
 *  uint8_t entropyBuffer[KEY_LENGTH_BYTES];
 *
 *  handle = TRNG_open(0, NULL);
 *
 *  if (!handle) {
 *      // Handle error
 *      while(1);
 *  }
 *
 *  CryptoKeyPlaintext_initBlankKey(&entropyKey, entropyBuffer, KEY_LENGTH_BYTES);
 *
 *  result = TRNG_generateEntropy(handle, &entropyKey);
 *
 *  if (result != TRNG_STATUS_SUCCESS) {
 *      // Handle error
 *      while(1);
 *  }
 *
 *  TRNG_close(handle);
 *
 *  @endcode
 *
 *  ### Generate ECC private and public key using rejection sampling #
 *  @code
 *
 *  #include <ti/drivers/TRNG.h>
 *  #include <ti/drivers/ECDH.h>
 *  #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
 *  #include <ti/drivers/cryptoutils/ecc/ECCParams.h>
 *
 *  TRNG_Handle trngHandle;
 *  ECDH_Handle ecdhHandle;
 *
 *  CryptoKey privateKey;
 *  CryptoKey publicKey;
 *
 *  int_fast16_t trngResult;
 *  int_fast16_t ecdhResult;
 *
 *  uint8_t privateKeyingMaterial[32];
 *  uint8_t publicKeyingMaterial[64];
 *
 *  ECDH_OperationGeneratePublicKey genPubKeyOperation;
 *
 *  trngHandle = TRNG_open(0, NULL);
 *  if (!trngHandle) {
 *      while(1);
 *  }
 *
 *  ecdhHandle = ECDH_open(0, NULL);
 *  if (!ecdhHandle) {
 *      while(1);
 *  }
 *
 *  // Repeatedly generate random numbers until they are in the range [1, n - 1].
 *  // Since the NIST-P256 order is so close to 2^256, the probability of needing
 *  // to generate more than one random number is incredibly low but not non-zero.
 *  do {
 *
 *      CryptoKeyPlaintext_initBlankKey(&privateKey, privateKeyingMaterial, ECCParams_NISTP256.length);
 *      CryptoKeyPlaintext_initBlankKey(&publicKey, publicKeyingMaterial, 2 * ECCParams_NISTP256.length);
 *
 *      trngResult = TRNG_generateEntropy(trngHandle, &privateKey);
 *
 *      if (trngResult != TRNG_STATUS_SUCCESS) {
 *          while(1);
 *      }
 *
 *      ECDH_OperationGeneratePublicKey_init(&genPubKeyOperation);
 *      genPubKeyOperation.curve = &ECCParams_NISTP256;
 *      genPubKeyOperation.myPrivateKey = &privateKey;
 *      genPubKeyOperation.myPublicKey = &publicKey;
 *
 *      ecdhResult = ECDH_generatePublicKey(ecdhHandle, &genPubKeyOperation);
 *
 *  } while(ecdhResult == ECDH_STATUS_PRIVATE_KEY_LARGER_EQUAL_ORDER || ecdhResult == ECDH_STATUS_PRIVATE_KEY_ZERO);
 *
 *  TRNG_close(trngHandle);
 *  ECDH_close(ecdhHandle);
 *
 *  @endcode
 */

#ifndef ti_drivers_TRNG__include
#define ti_drivers_TRNG__include

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

/*!
 * Common TRNG status code reservation offset.
 * TRNG driver implementations should offset status codes with
 * TRNG_STATUS_RESERVED growing negatively.
 *
 * Example implementation specific status codes:
 * @code
 * #define TRNGXYZ_STATUS_ERROR0    TRNG_STATUS_RESERVED - 0
 * #define TRNGXYZ_STATUS_ERROR1    TRNG_STATUS_RESERVED - 1
 * #define TRNGXYZ_STATUS_ERROR2    TRNG_STATUS_RESERVED - 2
 * @endcode
 */
#define TRNG_STATUS_RESERVED        (-32)

/*!
 * @brief   Successful status code.
 *
 * Functions return TRNG_STATUS_SUCCESS if the function was executed
 * successfully.
 */
#define TRNG_STATUS_SUCCESS         (0)

/*!
 * @brief   Generic error status code.
 *
 * Functions return TRNG_STATUS_ERROR if the function was not executed
 * successfully.
 */
#define TRNG_STATUS_ERROR           (-1)

/*!
 * @brief   An error status code returned if the hardware or software resource
 * is currently unavailable.
 *
 * TRNG driver implementations may have hardware or software limitations on how
 * many clients can simultaneously perform operations. This status code is returned
 * if the mutual exclusion mechanism signals that an operation cannot currently be performed.
 */
#define TRNG_STATUS_RESOURCE_UNAVAILABLE (-2)

/*!
 *  @brief  A handle that is returned from a TRNG_open() call.
 */
typedef struct TRNG_Config  *TRNG_Handle;

/*!
 * @brief   The way in which TRNG function calls return after generating
 *          the requested entropy.
 *
 * Not all TRNG operations exhibit the specified return behavor. Functions that do not
 * require significant computation and cannot offload that computation to a background thread
 * behave like regular functions. Which functions exhibit the specfied return behavior is not
 * implementation dependent. Specifically, a software-backed implementation run on the same
 * CPU as the application will emulate the return behavior while not actually offloading
 * the computation to the background thread.
 *
 * TRNG functions exhibiting the specified return behavior have restrictions on the
 * context from which they may be called.
 *
 * |                              | Task  | Hwi   | Swi   |
 * |------------------------------|-------|-------|-------|
 * |TRNG_RETURN_BEHAVIOR_CALLBACK | X     | X     | X     |
 * |TRNG_RETURN_BEHAVIOR_BLOCKING | X     |       |       |
 * |TRNG_RETURN_BEHAVIOR_POLLING  | X     | X     | X     |
 *
 */
typedef enum {
    TRNG_RETURN_BEHAVIOR_CALLBACK = 1,    /*!< The function call will return immediately while the
                                             *   TRNG operation goes on in the background. The registered
                                             *   callback function is called after the operation completes.
                                             *   The context the callback function is called (task, HWI, SWI)
                                             *   is implementation-dependent.
                                             */
    TRNG_RETURN_BEHAVIOR_BLOCKING = 2,    /*!< The function call will block while TRNG operation goes
                                             *   on in the background. TRNG operation results are available
                                             *   after the function returns.
                                             */
    TRNG_RETURN_BEHAVIOR_POLLING  = 4,    /*!< The function call will continuously poll a flag while TRNG
                                             *   operation goes on in the background. TRNG operation results
                                             *   are available after the function returns.
                                             */
} TRNG_ReturnBehavior;

/*!
 *  @brief TRNG Global configuration
 *
 *  The TRNG_Config structure contains a set of pointers used to characterize
 *  the TRNG driver implementation.
 *
 *  This structure needs to be defined before calling TRNG_init() and it must
 *  not be changed thereafter.
 *
 *  @sa     TRNG_init()
 */
typedef struct TRNG_Config {
    /*! Pointer to a driver specific data object */
    void               *object;

    /*! Pointer to a driver specific hardware attributes structure */
    void         const *hwAttrs;
} TRNG_Config;

/*!
 *  @brief  The definition of a callback function used by the TRNG driver
 *          when used in ::TRNG_RETURN_BEHAVIOR_CALLBACK
 *
 *  @param  handle  Handle of the client that started the TRNG operation.
 *
 *  @param  returnValue Return status code describing the outcome of the operation.
 *
 *  @param  entropy     The CryptoKey that describes the location the generated
 *                      entropy will be copied to.
 */
typedef void (*TRNG_CallbackFxn) (TRNG_Handle handle,
                                  int_fast16_t returnValue,
                                  CryptoKey *entropy);

/*!
 *  @brief  TRNG Parameters
 *
 *  TRNG Parameters are used to with the TRNG_open() call. Default values for
 *  these parameters are set using TRNG_Params_init().
 *
 *  @sa     TRNG_Params_init()
 */
typedef struct {
    TRNG_ReturnBehavior     returnBehavior;             /*!< Blocking, callback, or polling return behavior */
    TRNG_CallbackFxn        callbackFxn;                /*!< Callback function pointer */
    uint32_t                timeout;                    /*!< Timeout before the driver returns an error in
                                                         *   ::TRNG_RETURN_BEHAVIOR_BLOCKING
                                                         */
    void                   *custom;                     /*!< Custom argument used by driver
                                                         *   implementation
                                                         */
} TRNG_Params;

/*!
 *  @brief Default TRNG_Params structure
 *
 *  @sa     TRNG_Params_init()
 */
extern const TRNG_Params TRNG_defaultParams;

/*!
 *  @brief  This function initializes the TRNG module.
 *
 *  @pre    The TRNG_config structure must exist and be persistent before this
 *          function can be called. This function must also be called before
 *          any other TRNG driver APIs. This function call does not modify any
 *          peripheral registers.
 */
void TRNG_init(void);

/*!
 *  @brief  Function to initialize the TRNG_Params struct to its defaults
 *
 *  @param  params      An pointer to TRNG_Params structure for
 *                      initialization
 *
 *  Defaults values are:
 *      returnBehavior              = TRNG_RETURN_BEHAVIOR_BLOCKING
 *      callbackFxn                 = NULL
 *      timeout                     = SemaphoreP_WAIT_FOREVER
 *      custom                      = NULL
 */
void TRNG_Params_init(TRNG_Params *params);

/*!
 *  @brief  This function opens a given TRNG peripheral.
 *
 *  @pre    TRNG controller has been initialized using TRNG_init()
 *
 *  @param  index         Logical peripheral number for the TRNG indexed into
 *                        the TRNG_config table
 *
 *  @param  params        Pointer to an parameter block, if NULL it will use
 *                        default values.
 *
 *  @return A TRNG_Handle on success or a NULL on an error or if it has been
 *          opened already.
 *
 *  @sa     TRNG_init()
 *  @sa     TRNG_close()
 */
TRNG_Handle TRNG_open(uint_least8_t index, TRNG_Params *params);

/*!
 *  @brief  Function to close a TRNG peripheral specified by the TRNG handle
 *
 *  @pre    TRNG_open() has to be called first.
 *
 *  @param  handle A TRNG handle returned from TRNG_open()
 *
 *  @sa     TRNG_open()
 */
void TRNG_close(TRNG_Handle handle);

/*!
 *  @brief  Generate a random number
 *
 *  Generates a random bitstream of the size defined in the \c entropy
 *  CryptoKey in the range 0 <= \c entropy buffer < 2 ^ (entropy length * 8).
 *  The entropy will be generated and stored according to the storage requirements
 *  defined in the CryptoKey.
 *
 *  @pre    TRNG_open() has to be called first.
 *
 *  @param  handle A TRNG handle returned from TRNG_open().
 *
 *  @param  entropy A blank, initialized CryptoKey describing the target location
 *                  the entropy shall be stored in.
 *
 *  @retval #TRNG_STATUS_SUCCESS               The operation succeeded.
 *  @retval #TRNG_STATUS_ERROR                 The operation failed.
 *  @retval #TRNG_STATUS_RESOURCE_UNAVAILABLE  The required hardware resource was not available. Try again later.
 */
int_fast16_t TRNG_generateEntropy(TRNG_Handle handle, CryptoKey *entropy);




#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_TRNG__include */
