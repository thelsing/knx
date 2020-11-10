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
/** ============================================================================
 *  @file       CryptoCC26XX.h
 *
 *  @brief      Crypto driver implementation for a CC26XX Crypto controller
 *
 *  # Driver Include #
 *  The Crypto header file should be included in an application as follows:
 *  @code
 *  #include <ti/drivers/crypto/CryptoCC26XX.h>
 *  @endcode
 *
 *  @anchor ti_drivers_Crypto_Overview
 *  # Overview #
 *
 *  The CryptoCC26XX driver simplifies reading and writing to the CryptoCC26XX
 *  peripheral on the board with multiple modes of operation and performance.
 *  These include blocking and polling. A timeout can be configured in blocking mode.
 *  The driver supports encryption and decryption for both AES-ECB and AES-CCM.
 *
 *  @anchor ti_drivers_Crypto_Usage
 *  ## Usage #
 *
 *  @warning     The application should not attempt to encrypt a value stored in flash or use a
 *               key stored in flash if the application might switch to the XOSC_HF,
 *               the high frequency external oscillator, during the operation.
 *
 *  ### Opening the driver #
 *  - The application initializes the CryptoCC26XX driver by
 *    calling CryptoCC26XX_init() and is then ready to open a CryptoCC26XX by calling
 *    CryptoCC26XX_open().
 *  - If one client (the first) calls the CryptoCC26XX_open() with the exclusiveAccess
 *    flag set, all subsequent calls to open will fail (i.e. return a NULL pointer).
 *    The first client can use the Crypto module exclusively.
 *  - Only one client can call CryptoCC26XX_open() at the same time, a second client
 *    will pend until the first client's call will release a semaphore.
 *  - The number of clients are counted with openCnt in ::CryptoCC26XX_Object.
 *  .
 *  @code
 *  // Declaration (typically done in a task)
 *  CryptoCC26XX_Handle      handle;
 *  CryptoCC26XX_Params      params;
 *  bool                     exclusiveAccess = false;
 *
 *  // Initialize Crypto driver
 *  CryptoCC26XX_init();
 *
 *  // Configure CryptoCC26XX parameters.
 *  CryptoCC26XX_Params_init(&params);
 *
 *  // Attempt to open CryptoCC26XX.
 *  handle = CryptoCC26XX_open(Board_CRYPTO0, exclusiveAccess, &params);
 *  if (!handle) {
 *      System_printf("CryptoCC26XX did not open");
 *  }
 *  @endcode
 *
 *  ### Before starting a crypto operation #
 *  - Before any encryption/decryption operation starts, the key store must have at
 *    least one key loaded.
 *  - To get access to a location in the key store, the caller must allocated it by
 *    calling CryptoCC26XX_allocateKey().
 *  - If the requested key location is available, the client will get a key index
 *    (uint8_t) returned.
 *  - If the requested key location is already occupied, the CryptoCC26XX_allocateKey()
 *    will fail and return ::CRYPTOCC26XX_STATUS_ERROR.
 *  - The key locations available are defined in ::CryptoCC26XX_KeyLocation.
 *  - To select any available key location, call the allocate function with key
 *    location set to CRYPTOCC26XX_KEY_ANY.
 *  .
 *  ### Performing a crypto operation #
 *  - The supported crypto operations are defined in ::CryptoCC26XX_Operation.
 *  - The operation is initiated by calling a transaction function.
 *  - The type of operation is decided by the transaction pointer passed to the
 *    transaction function.
 *  - The mode of the transaction (i.e. blocking/polling) is decided by selecting the
 *    corresponding transact function (see Supported transaction modes below).
 *  - The key index is passed to the encryption/decryption transaction functions as
 *    part of the transaction object.
 *  - When a transaction starts, a semaphore is used to ensure that only one transaction
 *    is performed at a given time. The polling transaction is using 0 wait time, and
 *    will return immediately with ::CRYPTOCC26XX_STATUS_ERROR if the semaphore is not granted.
 *  - In blocking mode a constraint is set to ensure that the device only enters idle mode
 *    if the CPU becomes inactive.
 *  - When the transaction ends, the device might enter standby.
 *  - A key location can be deallocated by calling CryptoCC26XX_releaseKey().
 *  .
 *  ### Supported transaction modes #
 *  - ::CRYPTOCC26XX_MODE_BLOCKING, typically used from task context. Selected by calling the
 *    CryptoCC26XX_transact() function.
 *  - ::CRYPTOCC26XX_MODE_POLLING, typically used from hwi/swi context. Selected by calling the
 *    CryptoCC26XX_transactPolling() function. This mode should be replaced by a
 *    callback mode in a future release.
 *  .
 *  ### Closing the driver #
 *  The crypto driver is closed by calling CryptoCC26XX_close() function. When the openCnt
 *  is decremented to zero, the crypto related bios modules are destructed.
 *  @code
 *  CryptoCC26XX_close(handle);
 *  @endcode
 *
 *  ## Error handling ##
 *  If an error occur during encryption/decryption, the operation will return the
 *  error code and the device might enter standby.
 *
 *  ## Power Management ##
 *  During a transaction in blocking mode, the Power_DISALLOW_STANDBY
 *  constraint is set to block the system from entering standby mode. During
 *  this period the system will enter idle mode if no tasks are running.
 *
 *  A system dependency on crypto will be active as long as there are
 *  clients with an open handle to the crypto.
 *
 *  @note When coming out of standby the key store RAM has lost it's content, so the
 *  keyStore in ::CryptoCC26XX_Object will be cleared and the clients will have to
 *  allocate the key over again.
 *
 *  ## Unsupported functionality:
 *  Functionality that currently not supported:
 *  - Callback mode
 *  - Queued transactions for use in callback mode
 *
 *  ## Supported Operations ##
 *  | Operation Type                                | Description                                                               |
 *  |-----------------------------------------------|---------------------------------------------------------------------------|
 *  | ::CRYPTOCC26XX_OP_AES_CCM_ENCRYPT             | AES-CCM encryption of plaintext and authentication of AAD and plaintext   |
 *  | ::CRYPTOCC26XX_OP_AES_CCM_ENCRYPT_AAD_ONLY    | AES-CCM authentication of AAD only. No payload.                           |
 *  | ::CRYPTOCC26XX_OP_AES_CCM_DECRYPT             | AES-CCM decryption of plaintext and verification of AAD and plaintext     |
 *  | ::CRYPTOCC26XX_OP_AES_CCM_DECRYPT_AAD_ONLY    | AES-CCM verification of AAD only. No payload.                             |
 *  | ::CRYPTOCC26XX_OP_AES_ECB_ENCRYPT             | AES-ECB encryption                                                        |
 *  | ::CRYPTOCC26XX_OP_AES_ECB_DECRYPT             | AES-ECB decryption                                                        |
 *  | ::CRYPTOCC26XX_OP_AES_CBC_ENCRYPT             | AES-CBC encryption                                                        |
 *  | ::CRYPTOCC26XX_OP_AES_CBC_DECRYPT             | AES-CBC decryption                                                        |
 *
 *  @anchor ti_drivers_Crypto_Synopsis
 *  ## Synopsis
 *  @anchor ti_drivers_Crypto_Synopsis_Code
 *  @code
 *  // Import Crypto Driver definitions
 *  #include <ti/drivers/Crypto.h>
 *
 *  // Define name for Crypto channel index
 *  #define Crypto_INSTANCE 0
 *
 *  // Initialize Crypto driver
 *  CryptoCC26XX_init();
 *
 *
 *  // Attempt to open CryptoCC26XX.
 *  handle = CryptoCC26XX_open(Board_CRYPTO0, false, NULL);
 *
 *  keyIndex = CryptoCC26XX_allocateKey(handle, ecbExample.keyLocation,
 *                                     (const uint32_t *) ecbExample.key);
 *
 *  // Initialize transaction
 *  CryptoCC26XX_Transac_init((CryptoCC26XX_Transaction *) &trans, CRYPTOCC26XX_OP_AES_ECB_ENCRYPT);
 *
 *  // Setup transaction
 *  trans.keyIndex         = keyIndex;
 *  trans.msgIn            = (uint32_t *) ecbExample.clearText;
 *  trans.msgOut           = (uint32_t *) ecbExample.msgOut;
 *
 *
 *  // Encrypt the plaintext with AES ECB
 *  status = CryptoCC26XX_transact(handle, (CryptoCC26XX_Transaction *) &trans);
 *
 *  // Initialize transaction
 *  CryptoCC26XX_Transac_init((CryptoCC26XX_Transaction *) &trans, CRYPTOCC26XX_OP_AES_ECB_DECRYPT);
 *
 *  // Setup transaction
 *  trans.keyIndex         = keyIndex;
 *  trans.msgIn            = (uint32_t *) ecbExample.msgOut;
 *  trans.msgOut           = (uint32_t *) ecbExample.clearText;
 *
 *  // Decrypt the plaintext with AES ECB
 *  status = CryptoCC26XX_transact(handle, (CryptoCC26XX_Transaction *) &trans);
 *
 *  CryptoCC26XX_releaseKey(handle, &keyIndex);
 *
 *  Crypto_close(handle);
 *
 *  @endcode
 *
 *  @anchor ti_drivers_Crypto_Examples
 *  ## Examples ##
 *  ### AES ECB operation #
 *  Perform a crypto operation with AES-ECB in ::CRYPTOCC26XX_MODE_BLOCKING.

 *  @code
 *  // AES-ECB example struct
 *  typedef struct
 *  {
 *    uint8_t key[16];                      // Stores the Aes Key
 *    CryptoCC26XX_KeyLocation keyLocation; // Location in Key RAM
 *    uint8_t clearText[AES_ECB_LENGTH];    // Input message - cleartext
 *    uint8_t msgOut[AES_ECB_LENGTH];       // Output message
 *  } AESECBExample;
 *
 *  // AES ECB example data
 *  AESECBExample ecbExample =
 *  {
 *    { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
 *    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C },
 *    CRYPTOCC26XX_KEY_0,
 *    {'t','h','i','s','i','s','a','p','l','a','i','n','t','e','x','t'},
 *    { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 }
 *  };
 *
 *  // Declaration (typically done in a task)
 *  CryptoCC26XX_Handle             handle;
 *  int32_t                         keyIndex;
 *  int32_t                         status;
 *  CryptoCC26XX_AESECB_Transaction trans;
 *
 *  // Initialize Crypto driver
 *  CryptoCC26XX_init();
 *
 *
 *  // Attempt to open CryptoCC26XX.
 *  handle = CryptoCC26XX_open(Board_CRYPTO0, false, NULL);
 *  if (!handle) {
 *    System_abort("Crypto module could not be opened.");
 *  }
 *
 *  keyIndex = CryptoCC26XX_allocateKey(handle, ecbExample.keyLocation,
 *                                     (const uint32_t *) ecbExample.key);
 *
 *  if (keyIndex == CRYPTOCC26XX_STATUS_ERROR) {
 *      System_abort("Key Location was not allocated.");
 *  }
 *
 *  // Initialize transaction
 *  CryptoCC26XX_Transac_init((CryptoCC26XX_Transaction *) &trans, CRYPTOCC26XX_OP_AES_ECB_ENCRYPT);
 *
 *  // Setup transaction
 *  trans.keyIndex         = keyIndex;
 *  trans.msgIn            = (uint32_t *) ecbExample.clearText;
 *  trans.msgOut           = (uint32_t *) ecbExample.msgOut;
 *
 *
 *  // Encrypt the plaintext with AES ECB
 *  status = CryptoCC26XX_transact(handle, (CryptoCC26XX_Transaction *) &trans);
 *  if(status != CRYPTOCC26XX_STATUS_SUCCESS){
 *      System_abort("Encryption failed.");
 *  }
 *
 *  // Initialize transaction
 *  CryptoCC26XX_Transac_init((CryptoCC26XX_Transaction *) &trans, CRYPTOCC26XX_OP_AES_ECB_DECRYPT);
 *
 *  // Setup transaction
 *  trans.keyIndex         = keyIndex;
 *  trans.msgIn            = (uint32_t *) ecbExample.msgOut;
 *  trans.msgOut           = (uint32_t *) ecbExample.clearText;
 *
 *  // Zero original clear text before decrypting the cypher text into the ecbExample.clearText array
 *  memset(ecbExample.clearText, 0x0, AES_ECB_LENGTH);
 *
 *  // Decrypt the plaintext with AES ECB
 *  status = CryptoCC26XX_transact(handle, (CryptoCC26XX_Transaction *) &trans);
 *  if(status != CRYPTOCC26XX_STATUS_SUCCESS){
 *      System_abort("Encryption failed.");
 *  }
 *
 *  CryptoCC26XX_releaseKey(handle, &keyIndex);
 *  @endcode
 *
 *
 *  ### AES CCM operation #
 *  Perform a crypto and authentication operation with AES-CCM in ::CRYPTOCC26XX_MODE_BLOCKING.
 *
 *  @code
 *  #define macLength           (4)
 *  #define clearTextLength     (16)
 *  #define cipherTextLength    (macLength + clearTextLength)
 *  #define nonceLength         (12)
 *  #define aadLength           (14)
 *
 *  // Holds the AES-CCM setup for this example
 *  typedef struct
 *  {
 *      uint8_t key[16];                                // A 128 Bit AES key
 *      CryptoCC26XX_KeyLocation keyLocation;           // One of 8 key locations in the hardware
 *      uint8_t clearAndCipherText[cipherTextLength];   // Holds the cleartext before, and the ciphertext
 *                                                      // after the encryption operation.
 *                                                      // Ciphertext = encrypted text + message authentication code (MAC).
 *      uint8_t nonce[nonceLength];                     // A value that is used only once (cryptographic term 'nonce')
 *      uint8_t header[aadLength];                      // A header that is not encrypted but is authenticated in the operation (AAD).
 *      uint8_t verificationMAC[macLength];             // Location that the recalculated and encrypted MAC is stored during decryption.
 *  } AesCcmExample;
 *
 *  AesCcmExample ccmSetup =
 *  {
 *      .key = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
 *               0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C },
 *      .keyLocation = CRYPTOCC26XX_KEY_0,
 *      .clearAndCipherText = { 't','h','i','s','i','s','a','p','l','a','i','n','t','e','x','t','0','0','0','0' },
 *      .nonce  = { 't','h','i','s','i','s','a','n','o','n','c','e' },
 *      .header = { 't','h','i','s','i','s','a','h','e','a','d','e','r','1' }
 *  };
 *
 *  CryptoCC26XX_Handle             handle;
 *  int32_t                         keyIndex;
 *  CryptoCC26XX_AESCCM_Transaction trans;
 *  int32_t                         status;
 *
 *  // Initialize Crypto driver structures
 *  CryptoCC26XX_init();
 *
 *  // Open the crypto hardware with non-exclusive access and default parameters.
 *  handle = CryptoCC26XX_open(Board_CRYPTO0, false, NULL);
 *  if (handle == NULL) {
 *      System_abort("CryptoCC26XX did not open");
 *  }
 *
 *  // Allocate a key storage location in the hardware
 *  keyIndex = CryptoCC26XX_allocateKey(handle, ccmSetup.keyLocation, (const uint32_t *) ccmSetup.key);
 *  if (keyIndex == CRYPTOCC26XX_STATUS_ERROR) {
 *      System_abort("Key Location was not allocated.");
 *  }
 *
 *  // Encrypt and authenticate the message
 *  CryptoCC26XX_Transac_init((CryptoCC26XX_Transaction *) &trans, CRYPTOCC26XX_OP_AES_CCM);
 *  trans.keyIndex   = keyIndex;
 *  trans.authLength = macLength;
 *  trans.nonce  = (char *) ccmSetup.nonce;
 *  trans.header = (char *) ccmSetup.header;
 *  trans.fieldLength  = 3;
 *  trans.msgInLength  = clearTextLength;
 *  trans.headerLength = aadLength;
 *  trans.msgIn  = (char *) &(ccmSetup.clearAndCipherText[0]);                // Message is encrypted in place
 *  trans.msgOut = (char *) &(ccmSetup.clearAndCipherText[clearTextLength]);  // MAC will be written to this position
 *  status = CryptoCC26XX_transact(handle, (CryptoCC26XX_Transaction *) &trans);
 *  if (status != CRYPTOCC26XX_STATUS_SUCCESS) {
 *      System_abort("Encryption and signing failed.");
 *  }
 *
 *  // Decrypt and authenticate message
 *  CryptoCC26XX_Transac_init((CryptoCC26XX_Transaction *) &trans, CRYPTOCC26XX_OP_AES_CCMINV);
 *  trans.keyIndex   = keyIndex;
 *  trans.authLength = macLength;
 *  trans.nonce  = (char *) ccmSetup.nonce;
 *  trans.header = (char *) ccmSetup.header;
 *  trans.fieldLength  = 3;
 *  trans.msgInLength  = cipherTextLength;
 *  trans.headerLength = aadLength;
 *  trans.msgIn  = (char *) &(ccmSetup.clearAndCipherText[0]);                // Message is decrypted in place
 *  trans.msgOut = (char *) ccmSetup.verificationMAC;
 *
 *  // Do AES-CCM decryption and authentication
 *  status = CryptoCC26XX_transact(handle, (CryptoCC26XX_Transaction *) &trans);
 *  if(status != CRYPTOCC26XX_STATUS_SUCCESS){
 *      System_abort("Decryption and authentication failed.");
 *  }
 *
 *  // Release the key location
 *  status = CryptoCC26XX_releaseKey(handle, &keyIndex);
 *  if (status != CRYPTOCC26XX_STATUS_SUCCESS) {
 *      System_abort("Key release was not successful.");
 *  }
 *
 *  @endcode
 *
 *  # Instrumentation #
 *  The CryptoCC26XX driver interface produces log statements if instrumentation is
 *  enabled.
 *
 *  Diagnostics Mask | Log details                                |
 *  ---------------- | ------------------------------------------ |
 *  Diags_USER1      | Basic CryptoCC26XX operations performed    |
 *  Diags_USER2      | Detailed CryptoCC26XX operations performed |
 *
 *  ============================================================================
 */

#ifndef ti_drivers_crypto_CryptoCC26XX__include
#define ti_drivers_crypto_CryptoCC26XX__include

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include <ti/drivers/Power.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/crypto.h)

#if DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X2_CC26X2
    #warning "This driver is deprecated for the CC26x2 and CC13x2 families.\
              It is superceded by AESECB and AESCCM."
#endif

/**
 *  @addtogroup Crypto_STATUS
 *  CryptoCC26XX_STATUS_* macros are command codes only defined in the
 *  CryptoCC26XX.h driver implementation and need to:
 *  @code
 *  #include <ti/drivers/crypto/CryptoCC26XX.h>
 *  @endcode
 *  @{
 */

/* Add CryptoCC26XX_STATUS_* macros here */

/** @}*/

/**
 *  @addtogroup Crypto_CMD
 *  CryptoCC26XX_CMD_* macros are command codes only defined in the
 *  CryptoCC26XX.h driver implementation and need to:
 *  @code
 *  #include <ti/drivers/crypto/CryptoCC26XX.h>
 *  @endcode
 *  @{
 */

/* Add CryptoCC26XX_CMD_* macros here */

/** @}*/

#define CRYPTOCC26XX_TIMEOUT 20 /*!< Timeout Return Code */

#define CRYPTOCC26XX_STATUS_SUCCESS        0  /*!< Success Return Code           */
#define CRYPTOCC26XX_STATUS_ERROR         -1  /*!< Error Return Code             */
#define CRYPTOCC26XX_STATUS_UNDEFINEDCMD  -2  /*!< Command Undefined Return Code */

#define CRYPTOCC26XX_OP_AES_CCM_ENCRYPT             0   /*!< AES-CCM encryption of both AAD and plain text */
#define CRYPTOCC26XX_OP_AES_CCM_ENCRYPT_AAD_ONLY    1   /*!< AES-CCM authentication of ADD only */
#define CRYPTOCC26XX_OP_AES_CCM_DECRYPT             2   /*!< AES-CCM decryption of both AAD and plain text and verification of both */
#define CRYPTOCC26XX_OP_AES_CCM_DECRYPT_AAD_ONLY    3   /*!< AES-CCM verification of ADD only */
#define CRYPTOCC26XX_OP_AES_ECB_ENCRYPT             4   /*!< AES-ECB encryption */
#define CRYPTOCC26XX_OP_AES_ECB_DECRYPT             5   /*!< AES-ECB decryption */
#define CRYPTOCC26XX_OP_AES_CBC_ENCRYPT             6   /*!< AES-CBC encryption */
#define CRYPTOCC26XX_OP_AES_CBC_DECRYPT             7   /*!< AES-CBC decryption */

/* Deprecated operation mode names */
#define CRYPTOCC26XX_OP_AES_CCM                 CRYPTOCC26XX_OP_AES_CCM_ENCRYPT
#define CRYPTOCC26XX_OP_AES_CCM_NOCRYPT         CRYPTOCC26XX_OP_AES_CCM_ENCRYPT_AAD_ONLY
#define CRYPTOCC26XX_OP_AES_CCMINV              CRYPTOCC26XX_OP_AES_CCM_DECRYPT
#define CRYPTOCC26XX_OP_AES_CCMINV_NOCRYPT      CRYPTOCC26XX_OP_AES_CCM_DECRYPT_AAD_ONLY
#define CRYPTOCC26XX_OP_AES_ECB                 CRYPTOCC26XX_OP_AES_ECB_ENCRYPT
#define CRYPTOCC26XX_OP_AES_ECB_NOCRYPT         CRYPTOCC26XX_OP_AES_ECB_DECRYPT


#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

/*!
 *  @brief      A handle that is returned from a CryptoCC26XX_open() call.
 */
typedef struct CryptoCC26XX_Config      *CryptoCC26XX_Handle;

/*!
 *  @brief      CryptoCC26XX Mode Settings
 *
 *  This enum defines the read and write modes for the
 *  configured CryptoCC26XX.
 */
typedef enum CryptoCC26XX_Mode {
    /*!
      *  Uses a semaphore to block while data is being sent. Context of the call
      *  must be a Task.
      */
    CRYPTOCC26XX_MODE_BLOCKING,

    /*!
      *  Will return when the operation has finished. Call can be made from
      *  hwi and swi context.
      */
    CRYPTOCC26XX_MODE_POLLING
} CryptoCC26XX_Mode;

/*!
 *  @brief    CryptoCC26XX Operation Type
 *
 *  This type holds the CryptoCC26XX operation.
 *
 *  Currently supported types are
 *
 *  | Encryption                                    | Decryption                                    |
 *  |-----------------------------------------------|-----------------------------------------------|
 *  | ::CRYPTOCC26XX_OP_AES_CCM_ENCRYPT             | ::CRYPTOCC26XX_OP_AES_CCM_DECRYPT             |
 *  | ::CRYPTOCC26XX_OP_AES_CCM_ENCRYPT_AAD_ONLY    | ::CRYPTOCC26XX_OP_AES_CCM_DECRYPT_AAD_ONLY    |
 *  | ::CRYPTOCC26XX_OP_AES_ECB_ENCRYPT             | ::CRYPTOCC26XX_OP_AES_ECB_DECRYPT             |
 *  | ::CRYPTOCC26XX_OP_AES_CBC_ENCRYPT             | ::CRYPTOCC26XX_OP_AES_CBC_DECRYPT             |
 */
typedef uint8_t CryptoCC26XX_Operation;

/*!
 *  @brief    CryptoCC26XX Key Store Location
 *
 *  This enumeration defines the possible key locations in CryptoCC26XX.
 *
 */
typedef enum CryptoCC26XX_KeyLocation {
    CRYPTOCC26XX_KEY_0 = 0,
    CRYPTOCC26XX_KEY_1,
    CRYPTOCC26XX_KEY_2,
    CRYPTOCC26XX_KEY_3,
    CRYPTOCC26XX_KEY_4,
    CRYPTOCC26XX_KEY_5,
    CRYPTOCC26XX_KEY_COUNT,
    CRYPTOCC26XX_KEY_ANY,
} CryptoCC26XX_KeyLocation;

/*!
 *  @brief    CryptoCC26XX Parameters
 */
typedef struct CryptoCC26XX_Params {
    uint32_t        timeout;  /*!< Timeout for read semaphore */
} CryptoCC26XX_Params;

/*!
 *  @brief  CryptoCC26XX Key
 *
 *  This holds allocation information for the keys in the key store.
 */
typedef uint8_t CryptoCC26XX_KeyStore;

/*!
 *  @brief  CryptoCC26XX Transaction
 *
 *  This structure defines the nature of a general crypto transaction. An operation
 *  specific object, e.g. CryptoCC26XX_AESCCM_Transaction, must be used by the clients.
 *  All transaction must be typecasted to this common type when calling the transact
 *  functions. The first data of all transactions must hold a type field indicating
 *  which type of transaction to be performed.
 */
typedef struct CryptoCC26XX_Transaction {
    CryptoCC26XX_Operation    opType;          /*!< The type of the crypto operation */
    CryptoCC26XX_Mode         mode;            /*!< The mode of current transaction */
    uint8_t                   data[];          /*!< A void pointer to rest of transaction (transac. specific) */
} CryptoCC26XX_Transaction;

/*!
 *  @brief  CryptoCC26XX AES-CCM Transaction
 *
 *  The Counter with CBC-MAC (CCM) mode of operation is a generic
 *  authenticated encryption block cipher mode.  It can be used with
 *  any 128-bit block cipher.
 *  AES-CCM combines CBC-MAC with an AES block cipher.
 *
 *  AES-CCM encryption has the following inputs and outputs:
 *
 * <table>
 * <caption id="CryptoCC26XX_multi_row">AES-CCM input and output parameters</caption>
 * <tr><th>Encryption</th><th>Decryption</th></tr>
 * <tr><th colspan=2>Input</th></tr>
 * <tr><td>Shared AES key</td><td> Shared AES key</td></tr>
 * <tr><td>Nonce</td><td>Nonce</td></tr>
 * <tr><td>Cleartext</td><td>Ciphertext (encrypted cleartext + MAC)</td></tr>
 * <tr><td>AAD (optional)</td><td>AAD (optional)</td></tr>
 * <tr><th colspan=2>Output</th></tr>
 * <tr><td>Ciphertext (encrypted cleartext + MAC)</td><td>Cleartext</td></tr>
 * </table>
 *
 *  The AES key is a shared secret between the two parties and has a length
 *  of 128 Bit.  The key is stored in the dedicated RAM of the AES hardware
 *  unit before the crypto operation.
 *
 *  The nonce is generated by the party performing the authenticated
 *  encryption operation.  Within the scope of any authenticated
 *  encryption key, the nonce value must be unique.  That is, the set of
 *  nonce values used with any given key must not contain any duplicate
 *  values.  Using the same nonce for two different messages encrypted
 *  with the same key destroys the security properties.
 *
 *  The optional AAD is authenticated, but not encrypted.  Thus, the AAD
 *  is not included in the AES-CCM output.  It can be used to authenticate
 *  packet headers for transport layer security.
 *
 *  After the encryption operation, the ciphertext contains the encrypted
 *  data and the message authentication code (MAC). The MAC can be seen as an
 *  encrypted fingerprint of the message header and content.
 *
 *  AES-CCM works in both ways: encryption and decryption. When a message is
 *  decrypted, then ciphertext, AAD and nonce are used as inputs while
 *  the output comprises the cleartext only.  The decryption operation is
 *  successful, when the received ciphertext, the nonce and the AAD
 *  can reproduce the containing MAC.
 *
 *  The CryptoCC26XX_AESCCM_Transaction structure defines all necessary
 *  parameters for a AES-CCM transaction.
 */
typedef struct CryptoCC26XX_AESCCM_Transaction {
    CryptoCC26XX_Operation  opType;         /*!< The type of the crypto operation */
    CryptoCC26XX_Mode       mode;           /*!< The mode of current transaction. Set by transact function. */
    uint8_t                 keyIndex;       /*!< The key store index to be used */
    uint8_t                 authLength;     /*!< Is the the length of the authentication field */
                                            /*!< 0, 2, 4, 6, 8, 10, 12, 14 or 16 octets. */
    char                   *nonce;          /*!< A pointer to a nonce. It must satisfy the equation 15 = q + n,
                                             *   where q is the fieldLength and n is the length of the nonce.
                                             *
                                             *   The minimum size of the array containing the nonce is 12 bytes.
                                             *   When using nonces of length < 12 bytes, the nonce must be zero-padded
                                             *   to 12 bytes. The driverlib implementation in ROM was written
                                             *   with 12 and 13-byte nonces in mind. It constructs the IV's
                                             *   internally and hence copies either 12 or 13 bytes into another buffer.
                                             *   Providing a nonce buffer with less than 12 bytes would result in
                                             *   whatever is after the nonce in memory being incorrectly copied
                                             *   into the IV's.
                                             *
                                             *   As long as the correct fieldLength is set for the < 12-bytes nonce,
                                             *   the correct nonce-length will be used.
                                             *
                                             *   Valid nonce lengths are {7, 8, 9, 10, 11, 12, 13}.
                                             */
    char                   *msgIn;          /*!<
                                             *   - Encryption: A pointer to the octet string input message and after the transaction,
                                             *     the location of the encrypted cleartext. The cleatext is encrypted in place.
                                             *   - Decryption:  A pointer to the encrypted ciphertext composed of the encrypted cleartext
                                             *     concatenated with the encrypted message authentication code.
                                             */
    char                   *header;         /*!< The Additional Authentication Data (AAD). This header is authenticated but not encrypted. */
    void                   *msgOut;         /*!< A pointer to where the encrypted CBC-MAC shall be written to.
                                             *   - Encryption: It is recommended to set this to msgIn + msgInLength. The cyphertext sent out
                                             *     must be the concatenation of the encrypted message and encrypted MAC anyway.
                                             *   - Decyption: Do NOT set msgOut to the same location as the received MAC in the
                                             *     cyphertext within msgIn! Doing this effectively disables verification.
                                             */
    uint8_t                 fieldLength;    /*!< This parameter specifies the size in bytes of the message length field.
                                             *   (Not the length of the message itself!)
                                             *
                                             *   It sets the maximum length of the message
                                             *   according to p < 2^(8*q) where p is the message length and q is the fieldLength.
                                             *
                                             *   It must satisfy the equation 15 = q + n where q is the fieldLength and n is the
                                             *   length of the nonce.
                                             *
                                             *   Valid values are {2, 3, 4, 5, 6, 7, 8}.
                                             */
    uint16_t                msgInLength;    /*!< - Encryption:  The length of the cleartext.
                                                 - Decryption: The length of the ciphertext. */
    uint16_t                headerLength;   /*!< The length of the header in octets */
} CryptoCC26XX_AESCCM_Transaction;

/*!
 *  @brief  CryptoCC26XX AES-CBC Transaction
 *
 *  This structure defines the nature of the AES-CBC transaction. An object of this structure must
 *  be initialized by calling CryptoCC26XX_Transac_init().
 */
typedef struct CryptoCC26XX_AESCBC_Transaction {
    CryptoCC26XX_Operation  opType;         /*!< The type of the crypto operation */
    CryptoCC26XX_Mode       mode;           /*!< The mode of current transaction. Set by transact function. */
    uint8_t                 keyIndex;       /*!< The key store index to be used */
    void                    *nonce;         /*!< A pointer to 16 byte Nonce. */
    void                    *msgIn;         /*!< A pointer to the octet string input message */
    void                    *msgOut;        /*!< A pointer to the output message location */
    uint16_t                msgInLength;    /*!< The length of the message */
} CryptoCC26XX_AESCBC_Transaction;

/*!
 *  @brief  CryptoCC26XX AES-ECB Transaction
 *
 *  This structure defines the nature of the AES-ECB transaction. An object of this structure must
 *  be initialized by calling CryptoCC26XX_Transac_init().
 */
typedef struct CryptoCC26XX_AESECB_Transaction {
    CryptoCC26XX_Operation    opType;    /*!< The type of the crypto operation */
    CryptoCC26XX_Mode         mode;      /*!< The mode of current transaction. Set by transact function. */
    uint8_t                   keyIndex;  /*!< The key store index to be used. */
    void                     *msgIn;     /*!< A poiner to the octet string input message */
    void                     *msgOut;    /*!< A pointer to the output message location */
} CryptoCC26XX_AESECB_Transaction;

/*!
 *  @brief      CryptoCC26XX Hardware Attributes
 *
 *  These fields, with the exception of intPriority,
 *  are used by driverlib APIs and therefore must be populated by
 *  driverlib macro definitions. For CC26XXWare these definitions are found in:
 *      - inc/hw_memmap.h
 *      - inc/hw_ints.h
 *
 *  intPriority is the Crypto peripheral's interrupt priority, as defined by the
 *  underlying OS.  It is passed unmodified to the underlying OS's interrupt
 *  handler creation code, so you need to refer to the OS documentation
 *  for usage.  If the driver uses the ti.dpl interface
 *  instead of making OS calls directly, then the HwiP port handles the
 *  interrupt priority in an OS specific way.  In the case of the SYS/BIOS
 *  port, intPriority is passed unmodified to Hwi_create().
 *
 *  A sample structure is shown below:
 *  @code
 *  const CryptoCC26XX_HWAttrs cryptoCC26XXHWAttrs[] = {
 *      {
 *          .baseAddr = CRYPTO_BASE,
 *          .powerMngrId = PERIPH_CRYPTO,
 *          .intNum = INT_CRYPTO,
 *          .intPriority = (~0)
 *      }
 *  };
 *  @endcode
 */
typedef struct CryptoCC26XX_HWAttrs {
    /*! Crypto Peripheral's base address */
    uint32_t   baseAddr;
    /*! Crypto Peripheral's power manager ID */
    int        powerMngrId;
    /*! Crypto Peripheral's interrupt vector */
    int        intNum;
    /*! @brief Crypto Peripheral's interrupt priority.

        The CC26xx uses three of the priority bits, meaning ~0 has the same effect as (7 << 5).

        (7 << 5) will apply the lowest priority.

        (1 << 5) will apply the highest priority.

        Setting the priority to 0 is not supported by this driver.

        HWI's with priority 0 ignore the HWI dispatcher to support zero-latency interrupts, thus invalidating the critical sections in this driver.
    */
    uint8_t    intPriority;
} CryptoCC26XX_HWAttrs;

/*!
 *  @brief      CryptoCC26XX Object
 *
 *  The application must not access any member variables of this structure!
 */
typedef struct CryptoCC26XX_Object {
    /* CryptoCC26XX control variables */
    int                       openCnt;          /*!< Counting number of clients */
    uint32_t                  timeout;          /*!< Timeout for encrypt/decrypt operation */
    CryptoCC26XX_KeyStore     keyStore;         /*!< Key store for Crypto */
    CryptoCC26XX_Transaction *currentTransact;  /*!< Pointer to ongoing transaction */

    /*! Crypto notification object */
    Power_NotifyObj           cryptoNotiObj;

    /* CryptoCC26XX SYS/BIOS objects */
    HwiP_Struct hwi;      /*!< Hwi object */
} CryptoCC26XX_Object;

/*! @brief CryptoCC26XX Global Configuration */
typedef struct CryptoCC26XX_Config {
    /*! Pointer to a driver specific data object */
    void                   *object;

    /*! Pointer to a driver specific hardware attributes structure */
    void          const    *hwAttrs;
} CryptoCC26XX_Config;

/*!
 *  @brief  Function to closes a given CryptoCC26XX peripheral specified by the CryptoCC26XX
 *  handle.
 *
 *  @pre    CryptoCC26XX_open() had to be called first.
 *          Calling context: Task
 *
 *  @param  handle  A CryptoCC26XX_Handle returned from CryptoCC26XX_open().
 *
 *  @return Returns CRYPTOCC26XX_STATUS_SUCCESS if successful, otherwise will return
 *          CRYPTOCC26XX_STATUS_ERROR.
 *
 *  @sa     CryptoCC26XX_open
 */
int CryptoCC26XX_close(CryptoCC26XX_Handle handle);

/*!
 *  @brief  Function to initialize CryptoCC26XX driver.
 *          Users of this module must call init().
 *           Multiple users/libraries may call init(), though subsequent calls may be benign.
 *
 *  @pre    This function must be called before any other CryptoCC26XX driver APIs.
 *          Calling context: Task and Main.
 *
 */
void CryptoCC26XX_init(void);

/*!
 *  @brief  Function to initialize a given CryptoCC26XX peripheral specified by the
 *          particular index value. The parameter specifies which mode the CryptoCC26XX
 *          will operate.
 *
 *  @pre    The CryptoCC26XX_Config structure must exist and be persistent before this
 *          function can be called. CryptoCC26XX has been initialized with CryptoCC26XX_init().
 *          Calling context: Task.
 *
 *  @param  index         Logical peripheral number indexed into the HWAttrs
 *                        table. This is usually provided in the Board support
 *                        file.
 *
 *  @param  exclusiveAccess Boolean flag to get exclusive access, if true all
 *                          subsequent calls to CryptoCC26XX_open() will fail.
 *
 *  @param  params        Pointer to an parameter block, if NULL it will use
 *                        default values.
 *
 *  @return A CryptoCC26XX_Handle on success or a NULL on an error or if it has been
 *          already opened with exclusiveAccess set.
 *
 *  @sa     CryptoCC26XX_close(), CryptoCC26XX_init()
 */
CryptoCC26XX_Handle CryptoCC26XX_open(unsigned int index, bool exclusiveAccess, CryptoCC26XX_Params *params);

/*!
 *  @brief  Function to initialize the CryptoCC26XX_Params struct to its defaults.
 *
 *  @pre    Calling context: Hwi, Swi and Task.
 *
 *  Defaults values are:
 *  timeout = BIOS_WAIT_FOREVER;
 *
 *  @param  params  Parameter structure to initialize.
 */
void CryptoCC26XX_Params_init(CryptoCC26XX_Params *params);

/*!
 *  @brief  Function to initialize the CryptoCC26XX_Transaction struct to its defaults.
 *
 *  @pre    Calling context: Hwi, Swi and Task.
 *
 *  @param  trans   Transaction structure to initialize.
 *
 *  @param  opType  Cryto Operation type to perform in the transaction. See
 *                  ::CryptoCC26XX_Operation for currently supported types.
 */
void CryptoCC26XX_Transac_init(CryptoCC26XX_Transaction *trans, CryptoCC26XX_Operation opType);

/*!
 *  @brief  Function that allocates key, writes key into key store RAM and returns
 *          a handle to CryptoCC26XX Key.
 *
 *  This function tries to allocate the wanted key location, initiates an operation to
 *  write a key into one of the keystore RAM entries and returns a key index integer
 *  to the calling client. The allocated key index shall be used when building a
 *  transaction data object, e.g. ::CryptoCC26XX_AESCCM_Transaction.keyIndex.
 *  The function blocks the task calling it until the crypto hardware is available.
 *
 *  @pre    CryptoCC26XX_open() has to be called first.
 *          Calling context: Hwi, Swi and Task.
 *
 *  @param  handle          A CryptoCC26XX_Handle returned by CryptoCC26XX_open().
 *
 *  @param  keyLocation     The key location in key store to allocate. If set to
 *                          CRYPTOCC26XX_KEY_ANY, the first available key, starting
 *                          from highest index, will be allocated.
 *
 *  @param  keySrc          A pointer to buffer containing key to be written. If this pointer is NULL,
 *                          the key store will be reserved for the handle, but no key will be written to the key store.
 *
 *  @return An integer representing the index allocated in key store or a
 *  CRYPTOCC26XX_STATUS_ERROR on an error.
 *
 *  @sa     CryptoCC26XX_releaseKey()
 *  @sa     CryptoCC26XX_loadKey()
 */
int CryptoCC26XX_allocateKey(CryptoCC26XX_Handle handle, CryptoCC26XX_KeyLocation keyLocation, const uint32_t *keySrc);


/*!
 *  @brief  Function that writes a given key into a key store
 *
 *  This function loads a key into a keystore without needing to release the key store and
 *  re-allocate it using CryptoCC26XX_releaseKey() and  CryptoCC26XX_allocateKey().
 *  If called in task context, the function blocks for as long as specified in the timeout.
 *  If called in Swi or Hwi context, the function returns an error immediately if it cannot
 *  acquire the semaphore.
 *
 *  @pre    CryptoCC26XX_open() and CryptoCC26XX_allocateKey() have to be called first.
 *          Calling context: Hwi, Swi and Task.
 *
 *  @param  handle      A CryptoCC26XX_Handle returned by CryptoCC26XX_open().
 *
 *  @param  keyIndex    A key index returned by a previous call to CryptoCC26XX_allocateKey().
 *
 *  @param  keySrc      A pointer to buffer containing a key.
 *
 *  @return An integer representing success (::CRYPTOCC26XX_STATUS_SUCCESS) or an error (::CRYPTOCC26XX_STATUS_ERROR).
 *
 *  @sa     CryptoCC26XX_releaseKey()
 *  @sa     CryptoCC26XX_loadKey()
 */
int CryptoCC26XX_loadKey(CryptoCC26XX_Handle handle, int keyIndex, const uint32_t *keySrc);

/*!
 *  @brief  Function that releases the specified CryptoCC26XX Key.
 *
 *  This function releases the crypto key, so it can be used by other clients.
 *
 *  @pre    Driver must have been opened and the key must have been allocated first.
 *          Calling context: Hwi, Swi and Task.
 *
 *  @param  handle       A CryptoCC26XX_Handle returned by CryptoCC26XX_open().
 *
 *  @param  keyIndex     A pointer to the keyIndex to be released.
 *
 *  @return Returns CRYPTOCC26XX_STATUS_SUCCESS if successful, otherwise will return
 *          CRYPTOCC26XX_STATUS_ERROR.
 *
 *  @sa     CryptoCC26XX_allocateKey()
 */
int CryptoCC26XX_releaseKey(CryptoCC26XX_Handle handle, int *keyIndex);

/*!
 *  @brief  Function to do a Crypto operation (encryption or decryption) in blocking mode.
 *
 *  This function initiates a blocking crypto operation.
 *
 *  CryptoCC26XX_transact() will block task execution until all
 *  the data has been encrypted or decrypted, a task switch could be done
 *  when pending on a semaphore.
 *
 *  @pre    Driver must have been opened and a key must have been allocated first.
 *          Calling context: Task.
 *
 *  @param  handle          A CryptoCC26XX_Handle returned by CryptoCC26XX_open().
 *
 *  @param  transaction     Pointer to a transaction descriptor.
 *
 *  @return Returns CRYPTOCC26XX_STATUS_SUCCESS if successful, error code if not.
 *
 *  @sa     CryptoCC26XX_open(), CryptoCC26XX_allocateKey(), CryptoCC26XX_transactPolling()
 */
int CryptoCC26XX_transact(CryptoCC26XX_Handle handle, CryptoCC26XX_Transaction *transaction);

/*!
 *  @brief  Function to do a Crypto transaction operation (encryption or decryption) in polling mode.
 *
 *  This function initiates a polling crypto operation.
 *
 *  CryptoCC26XX_transactPolling() blocks task execution and does not pend
 *  on a semaphore, consequently no task switch will be done as a result of pending.
 *
 *  @pre    Driver must have been opened and a key must have been allocated first.
 *          Calling context: Hwi, Swi and Task.
 *
 *  @param  handle          An CryptoCC26XX_Handle returned by CryptoCC26XX_open().
 *
 *  @param  transaction     Pointer to a transaction descriptor.
 *
 *  @return Returns CRYPTOCC26XX_STATUS_SUCCESS if successful, error code if not.
 *
 *  @sa     CryptoCC26XX_open(), CryptoCC26XX_allocateKey(), CryptoCC26XX_transact()
 */
int CryptoCC26XX_transactPolling(CryptoCC26XX_Handle handle, CryptoCC26XX_Transaction *transaction);

/*!
 *  @brief  Function to do a Crypto transaction operation (encryption or decryption) in
 *          callback mode. Currently not supported.
 *
 *  This function initiates a callback crypto operation.
 *
 *  CryptoCC26XX_transactCallback() will continue task execution (i.e. does not pend
 *  on a semaphore). A callback function must be defined, and will be called when the
 *  crypto operation has finished.
 *
 *  @pre    Driver must have been opened and a key must have been allocated first.
 *          Calling context: Hwi, Swi and Task.
 *
 *  @param  handle          An CryptoCC26XX_Handle returned by CryptoCC26XX_open().
 *
 *  @param  transaction     Pointer to a transaction descriptor.
 *
 *  @return Returns CRYPTOCC26XX_STATUS_SUCCESS if successful, error code if not.
 *
 *  @note Currently not supported. Will replace CryptoCC26XX_transactPolling in the future.
 *
 *  @sa     CryptoCC26XX_open(), CryptoCC26XX_allocateKey(), CryptoCC26XX_transact()
 */
int CryptoCC26XX_transactCallback(CryptoCC26XX_Handle handle, CryptoCC26XX_Transaction *transaction);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_crypto_CryptoCC26XX__include */
