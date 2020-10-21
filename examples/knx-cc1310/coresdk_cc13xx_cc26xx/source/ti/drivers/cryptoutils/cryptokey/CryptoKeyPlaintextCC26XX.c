/*
 * Copyright (c) 2017, Texas Instruments Incorporated
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


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

/*!
 *  \brief Initializes a CryptoKey type
 *
 *
 *  @param[in]     keyHandle   Pointer to a CryptoKey which will be initialized to type CryptoKey_PLAINTEXT
 *                             and ready for use
 *  @param[in]     key         Pointer to key value
 *  @param[in]     keyLength   Length of key, in bytes
 *
 */
int_fast16_t CryptoKeyPlaintext_initKey(CryptoKey *keyHandle, uint8_t *key, size_t keyLength){
    keyHandle->encoding = CryptoKey_PLAINTEXT;
    keyHandle->u.plaintext.keyMaterial = key;
    keyHandle->u.plaintext.keyLength = keyLength;

    return CryptoKey_STATUS_SUCCESS;
}


/*!
 *  \brief Initializes an empty plaintext CryptoKey type
 *
 *
 *  @param[in]     keyHandle    Pointer to a CryptoKey which will be initialized to type
 *                              CryptoKey_BLANK_PLAINTEXT
 *  @param[in]     keyLocation  Pointer to location where plaintext key can be stored
 *  @param[in]     keyLength    Length of array allocated at key, in bytes
 *
 */
int_fast16_t CryptoKeyPlaintext_initBlankKey(CryptoKey *keyHandle, uint8_t *keyLocation, size_t keyLength){
    return CryptoKeyPlaintext_initKey(keyHandle, keyLocation, keyLength);
}



/*!
 * \brief Sets the CryptoKey.keyLocation pointer
 *
 *  Updates the key location for a plaintext CryptoKey.
 *  Does not modify data at the pointer location.
 *
 *  @param[in]      keyHandle   Pointer to a plaintext CryptoKey who's key data pointer will be modified
 *  @param[in]      location    Pointer to key data location
 */
int_fast16_t CryptoKeyPlaintext_setKeyLocation(CryptoKey *keyHandle, uint8_t *location){
    keyHandle->u.plaintext.keyMaterial = location;

    return CryptoKey_STATUS_SUCCESS;
}


/*!
 *  \brief Gets the length of a plaintext key
 *
 *  @param[in]      keyHandle   Pointer to a plaintext CryptoKey
 *  @param[in]      length      Length value will be updated to CryptoKey length, in bytes
 */
int_fast16_t CryptoKeyPlaintext_getKeyLength(CryptoKey *keyHandle, size_t *length){
    *length = keyHandle->u.plaintext.keyLength;

    return CryptoKey_STATUS_SUCCESS;
}
