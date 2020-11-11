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
/** ============================================================================
 *  @file       ECCParams.h
 *
 *  This file contains a common definition for eliptic curve structures used
 *  throughout the ECC based drivers.
 */

#ifndef ti_drivers_cryptoutils_ecc_ECCParams__include
#define ti_drivers_cryptoutils_ecc_ECCParams__include

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

/* Error status codes for the utility functions */

/*!
 * @brief   Successful status code.
 *
 * Function return ECCParams_STATUS_SUCCESS if the control code was executed
 * successfully.
 */
#define ECCParams_STATUS_SUCCESS         (0)

/*!
 * @brief   Generic error status code.
 *
 * Functions return ECCParams_STATUS_ERROR if the control code was not executed
 * successfully.
 */
#define ECCParams_STATUS_ERROR           (-1)

/*!
 *  @brief Enumeration of curve equations supported.
 *
 *  Elliptic curves can be expressed using multiple equations of polynomials over
 *  finite fields.
 *  All forms can be converted to one another using parameter substitution.
 *  Each curve has a default curve equations it was designed to use.
 *
 *  Some curve implementations have restrictions on which algorithms and schemes
 *  they work with. For example, Curve25519 was explicitely designed with ECDH in mind.
 *  It only uses and yields the X coordinate of a point on the elliptic curve in common
 *  implementations. Some implementations do provide X and Y affine coordinates but most
 *  do not.
 *  Therefore, ECDSA and ECJPAKE do not have compatible implementations
 *  for Curve25519 on some devices as the Y coordinate is required by them.
 *
 *  Check the header files of each device-specific implementation for information
 *  regarding curve-support for specific schemes on a device.
 *
 *  | Name              | Equation                      |
 *  |-------------------|-------------------------------|
 *  | Short Weierstrass | y^3 = x^2 + a*x + b mod p     |
 *  | Montgomery        | By^2 = x^3 + Ax^2 + x mod p   |
 *  | Edwards           | x^2 + y^2 = 1 + dx^2y^2 mod p |
 *
 */
typedef enum ECCParams_CurveType_ {
    ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS = 0,
    ECCParams_CURVE_TYPE_MONTGOMERY,
    ECCParams_CURVE_TYPE_EDWARDS,
} ECCParams_CurveType;

/*!
 *
 *  @brief A structure containing the parameters of an elliptic curve in short Weierstrass form.
 *
 *  Elliptical Curve Cryptography (ECC) prime curve.
 *
 *  The equation used to define the curve is expressed in the short Weierstrass
 *  form y^3 = x^2 + a*x + b
 *
 */
typedef struct ECCParams_CurveParams_ {
    const ECCParams_CurveType   curveType;
    const size_t                length;         //!< Length of the curve in bytes. All other buffers have this length.
    const uint8_t               *prime;         //!< The prime that defines the field of the curve.
    const uint8_t               *order;         //!< Order of the curve.
    const uint8_t               *a;             //!< Coefficient a of the equation.
    const uint8_t               *b;             //!< Coefficient b of the equation.
    const uint8_t               *generatorX;    //!< X coordinate of the generator point of the curve.
    const uint8_t               *generatorY;    //!< Y coordinate of the generator point of the curve.
}
ECCParams_CurveParams;



/* Short Weierstrass curves */

/*!
 *
 *  @brief The NISTP224 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_NISTP224;

/*!
 *
 *  @brief The NISTP256 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_NISTP256;

/*!
 *
 *  @brief The NISTP384 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_NISTP384;

/*!
 *
 *  @brief The NISTP521 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_NISTP521;

/*!
 *
 *  @brief The BrainpoolP256R1 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_BrainpoolP256R1;

/*!
 *
 *  @brief The BrainpoolP384R1 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_BrainpoolP384R1;

/*!
 *
 *  @brief The BrainpoolP512R1 curve in short Weierstrass form.
 *
 */
extern const ECCParams_CurveParams ECCParams_BrainpoolP512R1;



/* Montgomery curves */

/*!
 *
 *  @brief The Curve25519 curve in Montgomery form.
 *
 */
extern const ECCParams_CurveParams ECCParams_Curve25519;



/* Edwards curves */

/* Utility functions */

/*!
 *  @brief Formats a CryptoKey to conform to Curve25519 private key requirements.
 *
 *  Curve25519 has specific private key requirements specified by the curve definition.
 *  Specifically, the bottom three and the top bit may not be set and the second to
 *  last bit must be set.
 *
 *  @param  myPrivateKey    An initialized CryptoKey describing the entropy for a
 *                          Curve25519 private key. Platform-specific restrictions
 *                          for the location of the keying material apply. Some
 *                          implementations do not support modifying keying material
 *                          in flash for example.
 *
 *  @pre Initialize the CryptoKey with a 32-byte buffer in a compliant location.
 */
int_fast16_t ECCParams_FormatCurve25519PrivateKey(CryptoKey *myPrivateKey);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_cryptoutils_ecc_ECCParams__include */
