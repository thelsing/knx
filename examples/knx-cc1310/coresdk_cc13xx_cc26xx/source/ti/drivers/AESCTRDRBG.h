/*
 * Copyright (c) 2019, Texas Instruments Incorporated
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
 * @file       AESCTRDRBG.h
 *
 * @brief      AESCTRDRBG driver header
 *
 * @warning    This is a beta API. It may change in future releases.
 *
 * @anchor ti_drivers_AESCTRDRBG_Overview
 * <h3> Overview </h3>
 * AES_CTR_DRBG is a cryptographically secure deterministic random bit generator
 * that is used to efficiently generate random numbers for use in keying material
 * or other security related purposes. It is based on the AES block cipher
 * operating in Counter (CTR) mode and is defined by NIST SP 800-90A.
 *
 * AES_CTR_DRBG derives a sequence of pseudo-random numbers based on an initial
 * secret seed and additional, non-secret personalization data provided during
 * instantiation. A sequence of random bits generated by AES_CTR_DRBG will have
 * an equivalent entropy content of MIN(sequenceLength, security strength).
 * The security strength is based on the seed length and the AES key length used
 * in the AES_CTR_DRBG instance.
 *
 * |                                       | AES-128 | AES-192 | AES-256 |
 * |---------------------------------------|---------|---------|---------|
 * | Security Strength (bits)              | 128     | 192     | 256     |
 * | Seed Length (bits)                    | 192     | 320     | 384     |
 * | Personalization String Length (bits)  | <= 192  | <= 320  | <= 384  |
 * | Max Requests Between Reseeds          | 2^48    | 2^48    | 2^48    |
 * | Max Request Length (bits)             | 2^19    | 2^19    | 2^19    |
 *
 * <h3> Security Strength </h3>
 * The seed must be sourced from a cryptographically secure source such as
 * a true random number generator and contain seed length bits of entropy.
 * Since the seed length is always larger than the security strength for
 * any one AES key length, the output of one AES_CTR_DRBG instance may not
 * be used to seed another instance of the same or higher security strength.
 *
 * <h3> Reseeding </h3>
 * Because of the way AES CTR operates, there are a limited number of output
 * bitstreams that may be generated before the AES_CTR_DRBG instance must be
 * reseeded. The time between reseeding is set by the number of random bit
 * sequences generated not by their individual or combined lengths. Each time
 * random bits are requested of the AES_CTR_DRBG instance by the application,
 * the reseed counter is incremented by one regardless of how many bits at a
 * time are requested. When this counter reaches the configured reseed limit,
 * the AES_CTR_DRBG instance will return #AESCTRDRBG_STATUS_RESEED_REQUIRED
 * until it is reseeded.
 *
 * The maximum permitted number of requests between reseeds is 2^48.
 * The default counter is only 2^32 long for ease of implementation.
 * A more conservative reseed limit may be configured by the application
 * for increased security.
 *
 * A previously used seed may never be reused to reseed an AESCTRDRBG instance.
 * The seed used to instantiate or reseed an instance must be generated by
 * an approved entropy source and never be reused.
 *
 * <h3> Derivation Function </h3>
 * NIST specifies the the use of an optional derivation function to reduced
 * enctropy and personalizationg string lengths longer than the seed
 * length down to the seed length. This feature is not presently supported.
 *
 * @anchor ti_drivers_AESCTRDRBG_Usage
 * <h3> Usage </h3>
 *
 * This documentation provides a basic @ref ti_drivers_AESCTRDRBG_Synopsis
 * "usage summary" and a set of @ref ti_drivers_AESCTRDRBG_Examples "examples"
 * in the form of commented code fragments. Detailed descriptions of the
 * APIs are provided in subsequent sections.
 *
 * @anchor ti_drivers_AESCTRDRBG_Synopsis
 * <h3> Synopsis </h3>
 * @anchor ti_drivers_AESCTRDRBG_Synopsis_Code
 * @code
 *     #include <ti/drivers/AESCTRDRBG.h>
 *
 *     AESCTRDRBG_init();
 *
 *     // Instantiate the AESCTRDRBG instance
 *     AESCTRDRBG_Params_init(&params);
 *     params.keyLength = AESCTRDRBG_AES_KEY_LENGTH_128;
 *     params.reseedInterval = 0xFFFFFFFF;
 *     params.seed = seedBuffer;
 *
 *     handle = AESCTRDRBG_open(0, &params);
 *
 *     result = AESCTRDRBG_getBytes(handle, &resultKey);
 *
 *     reseedResult = AESCTRDRBG_reseed(handle, reseedBuffer, NULL, 0);
 *
 *     AESCTRDRBG_close(handle);
 * @endcode
 *
 * @anchor ti_drivers_AESCTRDRBG_Examples
 * <h3> Examples </h3>
 *
 * <h4> Instantiating an AESCTRDRBG Instance with TRNG </h4>
 * @code
 *
 * #include <ti/drivers/AESCTRDRBG.h>
 * #include <ti/drivers/TRNG.h>
 * #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
 *
 * ...
 *
 *     AESCTRDRBG_Handle handle;
 *     AESCTRDRBG_Params params;
 *     TRNG_Handle trngHandle;
 *     CryptoKey seedKey;
 *     int_fast16_t result;
 *
 *     uint8_t seedBuffer[AESCTRDRBG_SEED_LENGTH_AES_128];
 *
 *     // Generate the seed
 *     trngHandle = TRNG_open(0, NULL);
 *
 *     if (trngHandle == NULL) {
 *         // Handle error
 *         while(1);
 *     }
 *
 *     CryptoKeyPlaintext_initBlankKey(&seedKey, seedBuffer, AESCTRDRBG_SEED_LENGTH_AES_128);
 *
 *     result = TRNG_generateEntropy(trngHandle, &seedKey);
 *     if (result != TRNG_STATUS_SUCCESS) {
 *         // Handle error
 *         while(1);
 *     }
 *
 *     TRNG_close(trngHandle);
 *
 *     // Instantiate the AESCTRDRBG instance
 *     AESCTRDRBG_Params_init(&params);
 *     params.keyLength = AESCTRDRBG_AES_KEY_LENGTH_128;
 *     params.reseedInterval = 0xFFFFFFFF;
 *     params.seed = seedBuffer;
 *
 *     handle = AESCTRDRBG_open(0, &params);
 *     if (handle == NULL) {
 *         // Handle error
 *         while(1);
 *     }
 * @endcode
 *
 * <h4> Generating Random Data with Reseeding </h4>
 *
 * @code
 *
 * #include <ti/drivers/AESCTRDRBG.h>
 * #include <ti/drivers/TRNG.h>
 * #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
 *
 * ...
 *
 *     #define ENTROPY_REQUEST_LENGTH 256
 *
 *     AESCTRDRBG_Handle handle;
 *     TRNG_Handle trngHandle;
 *     CryptoKey entropyKey;
 *     int_fast16_t result;
 *
 *     uint8_t entropyBuffer[ENTROPY_REQUEST_LENGTH];
 *
 *     // Initialise the AESCTRDRBG instance here
 *     ...
 *
 *     // Start generating random numbers
 *     CryptoKeyPlaintext_initBlankKey(&entropyKey, entropyBuffer, ENTROPY_REQUEST_LENGTH);
 *
 *     result = AESCTRDRBG_getBytes(handle, &resultKey);
 *
 *     // Check return value and reseed if needed. This should happen only after many invocations
 *     // of AESCTRDRBG_getBytes().
 *     if (result == AESCTRDRBG_STATUS_RESEED_REQUIRED) {
 *         TRNG_Handle trngHandle;
 *         CryptoKey seedKey;
 *         int_fast16_t reseedResult;
 *         uint8_t reseedBuffer[AESCTRDRBG_SEED_LENGTH_AES_128];
 *
 *         CryptoKeyPlaintext_initBlankKey(&seedKey, reseedBuffer, AESCTRDRBG_SEED_LENGTH_AES_128);
 *
 *         reseedResult = TRNG_generateEntropy(trngHandle, &seedKey);
 *         if (reseedResult != TRNG_STATUS_SUCCESS) {
 *             // Handle error
 *             while(1);
 *         }
 *
 *         TRNG_close(trngHandle);
 *
 *         reseedResult = AESCTRDRBG_reseed(handle, reseedBuffer, NULL, 0);
 *         if (reseedResult != AESCTRDRBG_STATUS_SUCCESS) {
 *             // Handle error
 *             while(1);
 *         }
 *     }
 *     else if (result != AESCTRDRBG_STATUS_SUCCESS) {
 *         // Handle error
 *         while(1);
 *     }
 *
 * @endcode
 *
 */

#ifndef ti_drivers_AESCTRDRBG__include
#define ti_drivers_AESCTRDRBG__include

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/AESCTR.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * Common AESCTRDRBG status code reservation offset.
 * AESCTRDRBG driver implementations should offset status codes with
 * #AESCTRDRBG_STATUS_RESERVED growing negatively.
 *
 * Example implementation specific status codes:
 * @code
 * #define AESCTRDRBGXYZ_STATUS_ERROR0    AESCTRDRBG_STATUS_RESERVED - 0
 * #define AESCTRDRBGXYZ_STATUS_ERROR1    AESCTRDRBG_STATUS_RESERVED - 1
 * #define AESCTRDRBGXYZ_STATUS_ERROR2    AESCTRDRBG_STATUS_RESERVED - 2
 * @endcode
 */
#define AESCTRDRBG_STATUS_RESERVED        (-32)

/*!
 * @brief   Successful status code.
 *
 * Functions return #AESCTRDRBG_STATUS_SUCCESS if the function was executed
 * successfully.
 */
#define AESCTRDRBG_STATUS_SUCCESS         (0)

/*!
 * @brief   Generic error status code.
 *
 * Functions return #AESCTRDRBG_STATUS_ERROR if the function was not executed
 * successfully and no more pertinent error code could be returned.
 */
#define AESCTRDRBG_STATUS_ERROR           (-1)

/*!
 * @brief   An error status code returned if the hardware or software resource
 * is currently unavailable.
 *
 * AESCTRDRBG driver implementations may have hardware or software limitations on how
 * many clients can simultaneously perform operations. This status code is returned
 * if the mutual exclusion mechanism signals that an operation cannot currently be performed.
 */
#define AESCTRDRBG_STATUS_RESOURCE_UNAVAILABLE (-2)

/*!
 * @brief   The AESCTRDRBG instance must be reseeded.
 *
 * An AESCTRDRBG instance may only service a limited number of bit
 * generation requests before reseeding with more entropy is required.
 */
#define AESCTRDRBG_STATUS_RESEED_REQUIRED (-3)

/*!
 * @brief   The AES block size in bytes.
 */
#define AESCTRDRBG_AES_BLOCK_SIZE_BYTES 16

/*!
 * @brief   Length in bytes of the internal AES key used by an instance
 */
typedef enum
{
    AESCTRDRBG_AES_KEY_LENGTH_128 = 16,
    AESCTRDRBG_AES_KEY_LENGTH_256 = 32,
} AESCTRDRBG_AES_KEY_LENGTH;

/*!
 * @brief   Length in bytes of seed used to instantiate or reseed instance
 */
typedef enum
{
    AESCTRDRBG_SEED_LENGTH_AES_128 = AESCTRDRBG_AES_KEY_LENGTH_128 + AESCTRDRBG_AES_BLOCK_SIZE_BYTES,
    AESCTRDRBG_SEED_LENGTH_AES_256 = AESCTRDRBG_AES_KEY_LENGTH_256 + AESCTRDRBG_AES_BLOCK_SIZE_BYTES,
} AESCTRDRBG_SEED_LENGTH;

/*!
 * @brief   The way in which AESCTRDRBG function calls return after generating
 *          the requested entropy.
 *
 * Not all AESCTRDRBG operations exhibit the specified return behavor. Functions that do not
 * require significant computation and cannot offload that computation to a background thread
 * behave like regular functions. Which functions exhibit the specfied return behavior is not
 * implementation dependent. Specifically, a software-backed implementation run on the same
 * CPU as the application will emulate the return behavior while not actually offloading
 * the computation to the background thread.
 *
 * AESCTRDRBG functions exhibiting the specified return behavior have restrictions on the
 * context from which they may be called.
 *
 * |                                     | Task  | Hwi   | Swi   |
 * |-------------------------------------|-------|-------|-------|
 * |#AESCTRDRBG_RETURN_BEHAVIOR_BLOCKING | X     |       |       |
 * |#AESCTRDRBG_RETURN_BEHAVIOR_POLLING  | X     | X     | X     |
 *
 */
typedef enum AESCTRDRBG_ReturnBehavior_
{
    /*!< The function call will block while AESCTRDRBG operation goes
     *   on in the background. AESCTRDRBG operation results are available
     *   after the function returns.
     */
    AESCTRDRBG_RETURN_BEHAVIOR_BLOCKING = AESCTR_RETURN_BEHAVIOR_BLOCKING,
    /*!< The function call will continuously poll a flag while AESCTRDRBG
     *   operation goes on in the background. AESCTRDRBG operation results
     *   are available after the function returns.
     */
    AESCTRDRBG_RETURN_BEHAVIOR_POLLING  = AESCTR_RETURN_BEHAVIOR_POLLING,
} AESCTRDRBG_ReturnBehavior;

/*!
 *  @brief AESCTRDRBG Global configuration
 *
 *  The #AESCTRDRBG_Config structure contains a set of pointers used to characterize
 *  the AESCTRDRBG driver implementation.
 *
 *  This structure needs to be defined before calling #AESCTRDRBG_init() and it must
 *  not be changed thereafter.
 *
 *  @sa     #AESCTRDRBG_init()
 */
typedef struct
{
    /*! Pointer to a driver specific data object */
    void*               object;

    /*! Pointer to a driver specific hardware attributes structure */
    void         const* hwAttrs;
} AESCTRDRBG_Config;

/*!
 *  @brief  A handle that is returned from an #AESCTRDRBG_open() call.
 */
typedef AESCTRDRBG_Config* AESCTRDRBG_Handle;

/*!
 *  @brief  AESCTRDRBG Parameters
 *
 *  AESCTRDRBG Parameters are used to with the #AESCTRDRBG_open() call. Default values for
 *  these parameters are set using #AESCTRDRBG_Params_init().
 *
 *  @sa     #AESCTRDRBG_Params_init()
 */
typedef struct
{
    AESCTRDRBG_AES_KEY_LENGTH   keyLength;                      /*!< Length of the internal AES key
                                                                 *   of the driver instance.
                                                                 */
    uint32_t                    reseedInterval;                 /*!< Number of random number generation
                                                                 *   requests before the application is
                                                                 *   required to reseed the driver.
                                                                 */
    const void*                  seed;                          /*!< Entropy used to seed the internal
                                                                 *   state of the driver. Must be one of
                                                                 *   #AESCTRDRBG_SEED_LENGTH long depending
                                                                 *   on \c keyLength.
                                                                 */
    const void*                  personalizationData;           /*!< Optional non-secret personalization
                                                                 *   data to mix into the driver's internal
                                                                 *   state.
                                                                 */
    size_t                      personalizationDataLength;      /*!< Length of the optional
                                                                 *   \c personalizationData. Must satisfy
                                                                 *   0 <= \c personalizationDataLength <= seed length.
                                                                 */
    AESCTRDRBG_ReturnBehavior   returnBehavior;                 /*!< Return behavior of the driver instance.
                                                                 *   #AESCTRDRBG_RETURN_BEHAVIOR_POLLING is
                                                                 *   strongly recommended unless requests
                                                                 *   for > 500 bytes with AES-256 or
                                                                 *   1250 bytes for AES-128 will be common
                                                                 *   usecases for this driver instance.
                                                                 */
    void*                        custom;                        /*!< Custom argument used by driver
                                                                 *   implementation
                                                                 */
} AESCTRDRBG_Params;

/*!
 *  @brief Default #AESCTRDRBG_Params structure
 *
 *  @sa     #AESCTRDRBG_Params_init()
 */
extern const AESCTRDRBG_Params AESCTRDRBG_defaultParams;

/*!
 *  @brief  This function initializes the AESCTRDRBG driver.
 *
 *  @pre    The #AESCTRDRBG_Config structure must exist and be persistent before this
 *          function can be called. This function must also be called before
 *          any other AESCTRDRBG driver APIs. This function call does not modify any
 *          peripheral registers.
 */
void AESCTRDRBG_init(void);

/*!
 *  @brief  Function to initialize the #AESCTRDRBG_Params struct to its defaults
 *
 *  @param  [out]    params  Pointer to #AESCTRDRBG_Params structure for
 *                           initialization
 */
void AESCTRDRBG_Params_init(AESCTRDRBG_Params* params);

/*!
 *  @brief  This function opens a given AESCTRDRBG instance.
 *
 *  @pre    AESCTRDRBG controller has been initialized using #AESCTRDRBG_init()
 *
 *  @param  [in]    index   Logical peripheral number for the AESCTRDRBG indexed into
 *                          the #AESCTRDRBG_Config table
 *
 *  @param  [in]    params  Pointer to an parameter block, if NULL it will use
 *                          default values.
 *
 *  @return An #AESCTRDRBG_Handle on success or a NULL on an error or if it has
 *          been opened already.
 *
 *  @sa     #AESCTRDRBG_init()
 *  @sa     #AESCTRDRBG_close()
 */
AESCTRDRBG_Handle AESCTRDRBG_open(uint_least8_t index, const AESCTRDRBG_Params* params);

/*!
 *  @brief  Function to close an AESCTRDRBG peripheral specified by the #AESCTRDRBG_Handle
 *
 *  @pre    #AESCTRDRBG_open() has to be called first.
 *
 *  @param  [in]    handle  An #AESCTRDRBG_Handle returned from #AESCTRDRBG_open()
 *
 *  @sa     #AESCTRDRBG_open()
 */
void AESCTRDRBG_close(AESCTRDRBG_Handle handle);

/*!
 *  @brief  Generate a requested number of random bytes
 *
 *  @param  [in]        handle      An #AESCTRDRBG_Handle returned from #AESCTRDRBG_open()
 *
 *  @param  [in,out]    randomBytes #CryptoKey describing how many random bytes are requested and
 *                                  where to put them.
 *
 *  @retval #AESCTRDRBG_STATUS_SUCCESS              Random bytes generated.
 *  @retval #AESCTRDRBG_STATUS_ERROR                Random bytes not generated.
 *  @retval #AESCTRDRBG_STATUS_RESOURCE_UNAVAILABLE The requires hardware was unavailable.
 *  @retval #AESCTRDRBG_STATUS_RESEED_REQUIRED      Reseed counter >= reseed limit. Reseed required.
 */
int_fast16_t AESCTRDRBG_getBytes(AESCTRDRBG_Handle handle, CryptoKey* randomBytes);

/*!
 *  @brief  Reseed an AESCTRDRBG instance.
 *
 *  @param  [in]    handle                  An #AESCTRDRBG_Handle returned from #AESCTRDRBG_open()
 *
 *  @param  [in]    seed                    Entropy to mix into the AESCTRDRBG instance state
 *
 *  @param  [in]    additionalData          Optional non-secret additional data to mix into the
 *                                          instance state.
 *
 *  @param  [in]    additionalDataLength    Length of the optional additional data.
 *                                          0 <= \c additionalDataLength <= seed length of the
 *                                          instance.
 *
 *  @retval #AESCTRDRBG_STATUS_SUCCESS                  Reseed successful. Reseed counter reset.
 *  @retval #AESCTRDRBG_STATUS_ERROR                    Reseed not successful. Reseed counter not reset.
 *  @retval #AESCTRDRBG_STATUS_RESOURCE_UNAVAILABLE     The requires hardware was unavailable.
 */
int_fast16_t AESCTRDRBG_reseed(AESCTRDRBG_Handle handle,
                               const void* seed,
                               const void* additionalData,
                               size_t additionalDataLength);



#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_AESCTRDRBG__include */
