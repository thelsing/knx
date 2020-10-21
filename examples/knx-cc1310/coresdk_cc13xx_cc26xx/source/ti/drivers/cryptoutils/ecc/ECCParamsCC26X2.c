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
/*
 *  ======== ECCParamsCC26X2.c ========
 *
 *  This file contains structure definitions for various ECC curves for use
 *  on CC26X2 devices.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <ti/drivers/cryptoutils/ecc/ECCParams.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/pka.h)

const ECCParams_CurveParams ECCParams_NISTP224 = {
    .curveType      = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS,
    .length         = NISTP224_PARAM_SIZE_BYTES,
    .prime          = NISTP224_prime.byte,
    .order          = NISTP224_order.byte,
    .a              = NISTP224_a.byte,
    .b              = NISTP224_b.byte,
    .generatorX     = NISTP224_generator.x.byte,
    .generatorY     = NISTP224_generator.y.byte
};

const ECCParams_CurveParams ECCParams_NISTP256 = {
    .curveType      = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS,
    .length         = NISTP256_PARAM_SIZE_BYTES,
    .prime          = NISTP256_prime.byte,
    .order          = NISTP256_order.byte,
    .a              = NISTP256_a.byte,
    .b              = NISTP256_b.byte,
    .generatorX     = NISTP256_generator.x.byte,
    .generatorY     = NISTP256_generator.y.byte
};

const ECCParams_CurveParams ECCParams_NISTP384 = {
    .curveType      = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS,
    .length         = NISTP384_PARAM_SIZE_BYTES,
    .prime          = NISTP384_prime.byte,
    .order          = NISTP384_order.byte,
    .a              = NISTP384_a.byte,
    .b              = NISTP384_b.byte,
    .generatorX     = NISTP384_generator.x.byte,
    .generatorY     = NISTP384_generator.y.byte
};

const ECCParams_CurveParams ECCParams_NISTP521 = {
    .curveType      = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS,
    .length         = NISTP521_PARAM_SIZE_BYTES,
    .prime          = NISTP521_prime.byte,
    .order          = NISTP521_order.byte,
    .a              = NISTP521_a.byte,
    .b              = NISTP521_b.byte,
    .generatorX     = NISTP521_generator.x.byte,
    .generatorY     = NISTP521_generator.y.byte
};

const ECCParams_CurveParams ECCParams_BrainpoolP256R1 = {
    .curveType      = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS,
    .length         = BrainpoolP256R1_PARAM_SIZE_BYTES,
    .prime          = BrainpoolP256R1_prime.byte,
    .order          = BrainpoolP256R1_order.byte,
    .a              = BrainpoolP256R1_a.byte,
    .b              = BrainpoolP256R1_b.byte,
    .generatorX     = BrainpoolP256R1_generator.x.byte,
    .generatorY     = BrainpoolP256R1_generator.y.byte
};

const ECCParams_CurveParams ECCParams_BrainpoolP384R1 = {
    .curveType      = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS,
    .length         = BrainpoolP384R1_PARAM_SIZE_BYTES,
    .prime          = BrainpoolP384R1_prime.byte,
    .order          = BrainpoolP384R1_order.byte,
    .a              = BrainpoolP384R1_a.byte,
    .b              = BrainpoolP384R1_b.byte,
    .generatorX     = BrainpoolP384R1_generator.x.byte,
    .generatorY     = BrainpoolP384R1_generator.y.byte
};

const ECCParams_CurveParams ECCParams_BrainpoolP512R1 = {
    .curveType      = ECCParams_CURVE_TYPE_SHORT_WEIERSTRASS,
    .length         = BrainpoolP512R1_PARAM_SIZE_BYTES,
    .prime          = BrainpoolP512R1_prime.byte,
    .order          = BrainpoolP512R1_order.byte,
    .a              = BrainpoolP512R1_a.byte,
    .b              = BrainpoolP512R1_b.byte,
    .generatorX     = BrainpoolP512R1_generator.x.byte,
    .generatorY     = BrainpoolP512R1_generator.y.byte
};

const ECCParams_CurveParams ECCParams_Curve25519 = {
    .curveType      = ECCParams_CURVE_TYPE_MONTGOMERY,
    .length         = 32,
    .prime          = Curve25519_prime.byte,
    .order          = Curve25519_order.byte,
    .a              = Curve25519_a.byte,
    .b              = Curve25519_b.byte,
    .generatorX     = Curve25519_generator.x.byte,
    .generatorY     = Curve25519_generator.y.byte
};

/*
 *  ======== ECCParams_FormatCurve25519PrivateKey ========
 */
int_fast16_t ECCParams_FormatCurve25519PrivateKey(CryptoKey *myPrivateKey){
    myPrivateKey->u.plaintext.keyMaterial[0] &= 0xF8;
    myPrivateKey->u.plaintext.keyMaterial[31] &= 0x7F;
    myPrivateKey->u.plaintext.keyMaterial[31] |= 0x40;

    return ECCParams_STATUS_SUCCESS;
}
