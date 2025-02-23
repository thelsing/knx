/******************************************************************************
 *  Filename:       rf_patch_mce_genook.h
 *  Revised:        $Date: 2019-01-31 15:04:25 +0100 (to, 31 jan 2019) $
 *  Revision:       $Revision: 18842 $
 *
 *  Description: RF core patch for CC13x0 Generic OOK
 *
 *  Copyright (c) 2015-2019, Texas Instruments Incorporated
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1) Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2) Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3) Neither the name of the ORGANIZATION nor the names of its contributors may
 *     be used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#ifndef _RF_PATCH_MCE_GENOOK_H
#define _RF_PATCH_MCE_GENOOK_H

#include "../inc/hw_types.h"
#include <stdint.h>

#ifndef MCE_PATCH_TYPE
#define MCE_PATCH_TYPE static const uint32_t
#endif

#ifndef PATCH_FUN_SPEC
#define PATCH_FUN_SPEC static inline
#endif

#ifndef RFC_MCERAM_BASE
#define RFC_MCERAM_BASE 0x21008000
#endif

#ifndef MCE_PATCH_MODE
#define MCE_PATCH_MODE 0
#endif

MCE_PATCH_TYPE patchGenookMce[463] =
    {
        0x00006030,
        0x01952fcf,
        0x7fff0001,
        0x030c003f,
        0x070c680a,
        0x00010000,
        0xaaaa000f,
        0x00fc00aa,
        0x00170003,
        0x0000001f,
        0x04000000,
        0x0000000f,
        0x00020387,
        0x00434074,
        0x20028000,
        0x000006f0,
        0x0500091e,
        0x00000054,
        0x50140000,
        0x00000050,
        0x7f30000f,
        0x0000007f,
        0x00000000,
        0x00000000,
        0x72487220,
        0x73057303,
        0x73047203,
        0x72047306,
        0x72767376,
        0x8001c7c0,
        0x90010001,
        0x08019010,
        0x720c9001,
        0x720e720d,
        0x7100b0c0,
        0xa0c0b0f0,
        0x81327218,
        0x9862d030,
        0x10206798,
        0x1e000670,
        0x1e104074,
        0x1e204075,
        0x3982405f,
        0x163206f2,
        0x14211101,
        0x61826c01,
        0x63186182,
        0x3982632e,
        0x16323942,
        0x14211101,
        0x60e36c01,
        0x610d60e3,
        0x606b1220,
        0x72201210,
        0x7310730f,
        0x81817311,
        0x91800010,
        0x6044b070,
        0xc101606a,
        0xc470c282,
        0x6f131820,
        0x16116e23,
        0x68791612,
        0x9ab07870,
        0x9ac07880,
        0x9ad07890,
        0x981078b0,
        0xc5a0c482,
        0x408e1820,
        0x6e231203,
        0x688b1612,
        0x9ae078a0,
        0x8160606a,
        0x81409490,
        0x2a703980,
        0x16111001,
        0x84448432,
        0xc0f5c0f3,
        0x1c01c200,
        0xc10040b5,
        0x40ab1c10,
        0x10134cad,
        0x18301803,
        0x1a101a13,
        0x68a83912,
        0x13f360b5,
        0x13f360b5,
        0xc1001015,
        0x1a151850,
        0x39141a10,
        0xb0d868b3,
        0xb1087100,
        0xb200a0d8,
        0xb012b002,
        0x22168216,
        0x814640bc,
        0x06f63d46,
        0x81408165,
        0x105106f0,
        0x65570611,
        0x68c53d15,
        0x22f08140,
        0x1a1644bf,
        0x8ae14cc2,
        0x9861d040,
        0x13f06798,
        0x40dc1c03,
        0x1021c0f0,
        0x65570611,
        0x68d73d12,
        0x1041c0f0,
        0x65570611,
        0x68dd3d14,
        0x72207000,
        0x7310730f,
        0x91c0c000,
        0xb0c1b0f1,
        0x9760c050,
        0x9780c010,
        0x6491c008,
        0x39838ad3,
        0x06133953,
        0x221081e0,
        0x81a14104,
        0x10170831,
        0x81306557,
        0x39403980,
        0x45031e10,
        0x0a111071,
        0x60f46557,
        0x65571201,
        0xa0c1b204,
        0xa0c3b0f1,
        0x6798c050,
        0x7220606a,
        0x7310730f,
        0x91c0c000,
        0xb0c1b0f1,
        0x9760c050,
        0x9780c010,
        0x8216b200,
        0x41192216,
        0xb012b002,
        0xc030c008,
        0x10a178ca,
        0x65570611,
        0x6921391a,
        0x78dac0f0,
        0x061110a1,
        0x391a6557,
        0xc0706928,
        0x10a178ea,
        0x65570611,
        0x692f391a,
        0x78fac090,
        0x061110a1,
        0x391a6557,
        0x8ad36936,
        0x39533983,
        0x81e00613,
        0x414c2210,
        0x0831c011,
        0x81a16557,
        0x65576793,
        0x0831c001,
        0x613f6557,
        0x6557c011,
        0x6557c001,
        0xa0c1b204,
        0xa0c3b0f1,
        0x6798c060,
        0xc029606a,
        0x455d2208,
        0x41732201,
        0x2201616c,
        0x8aef4573,
        0x416c22ff,
        0x31116578,
        0x39119201,
        0x80fe1018,
        0x456bc019,
        0x6173c029,
        0x7100b0f1,
        0x92013111,
        0x10183911,
        0xb0f1c019,
        0x1a197100,
        0x70004573,
        0x785f10f9,
        0x100004f9,
        0x10001000,
        0x1a191000,
        0x7000457b,
        0xc0706750,
        0x847d6798,
        0x140dc010,
        0x142d312d,
        0x318e8ace,
        0x397e311e,
        0x31498ac9,
        0x39493979,
        0x10903129,
        0x72769780,
        0xa764b764,
        0x9762c662,
        0xb012b002,
        0x986be080,
        0x6798987f,
        0x6699b485,
        0x8ab1a182,
        0x45aa22f1,
        0x22f18ad1,
        0x61df45aa,
        0x80b77100,
        0x45fd2207,
        0x22b08090,
        0x105441b6,
        0x662d858c,
        0x61aa668e,
        0x22f18ab1,
        0x223741c0,
        0xb11341c0,
        0x223080b0,
        0x61ce45bb,
        0x41d322e1,
        0x22508090,
        0xb0f541d3,
        0x22108210,
        0x978941aa,
        0xa764b764,
        0x61aab0f6,
        0xb764978d,
        0xb0f6a764,
        0x8ad061aa,
        0x42da22f0,
        0x42da2237,
        0xb113b075,
        0x223080b0,
        0xb08745d9,
        0x710061aa,
        0x220780b7,
        0x223745fd,
        0x809045fc,
        0x41ee22b0,
        0x858c1054,
        0x668e662d,
        0x8ab161df,
        0x41df22e1,
        0x22508090,
        0xb0f541df,
        0x22108210,
        0x978d41df,
        0xa764b764,
        0x61dfb0f6,
        0xb110b182,
        0xb113a0e0,
        0xb074a0e3,
        0xa044b201,
        0x986ad090,
        0x10806798,
        0x1c0a1610,
        0x1cfa4a0e,
        0x66704e0e,
        0xc00ec00f,
        0x80907100,
        0x44442200,
        0x1054858c,
        0x668e662d,
        0x39808130,
        0x1e1006f0,
        0x667a461f,
        0x66646220,
        0x1e008150,
        0x1a104210,
        0x4e101cf0,
        0x62106228,
        0xb0f6a0c6,
        0xb0fba0cb,
        0xb8846306,
        0x881188c2,
        0x1e010631,
        0x1e21424a,
        0x1e31423c,
        0x10564243,
        0x39161426,
        0x624b1065,
        0x31261056,
        0x14261856,
        0x10653926,
        0x1056624b,
        0x18563136,
        0x39361426,
        0x624b1065,
        0x82121026,
        0x1c263922,
        0x18624e59,
        0x1c12c101,
        0x12014e57,
        0x31211821,
        0xcc016261,
        0x18266261,
        0x1c16c101,
        0x10614e60,
        0x62613121,
        0x9581c401,
        0x7000b0fb,
        0x466f1c8a,
        0x39208210,
        0x4e6c1c04,
        0x626dc001,
        0x9191c011,
        0x7000161f,
        0x39208210,
        0x4e761c04,
        0x6277c001,
        0x9191c011,
        0x7000c01f,
        0x468d1c8a,
        0x31808580,
        0x10013d80,
        0x10171870,
        0x468c1e1e,
        0x39703980,
        0x39818ad1,
        0x08103951,
        0x161f9190,
        0x70000a1e,
        0x10c08581,
        0x22700810,
        0x120a4295,
        0x1cba6298,
        0x161a4293,
        0xb0fb7000,
        0xb0f1b0f6,
        0xb113b110,
        0xb0f2b0f5,
        0x720d720c,
        0xb0cb720e,
        0xb0e3b0e0,
        0x22f28ab2,
        0xb0c642ac,
        0x62afb763,
        0x22f08ad0,
        0xb40542cf,
        0xa428a404,
        0xcaa0a429,
        0xcaa13180,
        0x94510001,
        0x8ad39461,
        0x39833183,
        0x31808410,
        0x31833980,
        0x94100030,
        0x31508400,
        0x8ad33950,
        0x06f33983,
        0x1834c1f4,
        0x31343184,
        0x94000040,
        0x22e2b089,
        0x8aca42d9,
        0x398a394a,
        0x978a312a,
        0xb0c6b0c5,
        0x7000b763,
        0xa0e0b20f,
        0xa0cba0e3,
        0xb764978e,
        0xb0f6a764,
        0xb113b110,
        0x8210b0fb,
        0x42e52200,
        0x8002b0f5,
        0xa006a004,
        0x7203a001,
        0xc0a07204,
        0x71006798,
        0xb0f6b764,
        0xa20fb0c5,
        0xb0f57100,
        0x7820a0c5,
        0x90029030,
        0x90407830,
        0xb0729060,
        0x6699a20f,
        0xa764978a,
        0x61a3b0f6,
        0x8180b88c,
        0x392489a4,
        0x00043184,
        0xc0b09184,
        0x73766798,
        0x72487276,
        0x72027206,
        0x73057204,
        0x606a7306,
        0xc0c06750,
        0xb0f86798,
        0xb0fbb0cb,
        0xb228b005,
        0xb0fb7100,
        0x22e08ad0,
        0x82104328,
        0x43202210,
        0x8580662d,
        0x0a103970,
        0x63206789,
        0xc0d06750,
        0xb0cb6798,
        0x120cb074,
        0x398e881e,
        0x433e1e0e,
        0x30e01210,
        0x71001a20,
        0x6b3b662d,
        0x8ad07100,
        0x434522e0,
        0x22108210,
        0x662d4336,
        0x0a113971,
        0x81549191,
        0x43361e04,
        0x1cc4161c,
        0x63364306,
        0x91b01200,
        0xb006b0f8,
        0xb004b016,
        0xb002b014,
        0x8400b012,
        0x04207862,
        0x39838143,
        0x94732a73,
        0x1832c1f2,
        0x10213162,
        0x00123151,
        0x94000020,
        0x90307820,
        0x78309050,
        0x90609040,
        0x8330c04b,
        0x06303930,
        0x43751e00,
        0x10b8300b,
        0x39181a1b,
        0x108fc00a,
        0xa203a204,
        0x22408330,
        0x165f4382,
        0x6386b204,
        0x43862230,
        0xb203163f,
        0xb072b205,
        0x22007000,
        0xb005478d,
        0x80006392,
        0x43922250,
        0xa005b240,
        0x82a27000,
        0x06123972,
        0x70000821,
        0x88409850,
        0x47992200,
        0x7000b830};

PATCH_FUN_SPEC void rf_patch_mce_genook(void)
{
#ifdef __PATCH_NO_UNROLLING
    uint32_t i;

    for (i = 0; i < 463; i++)
    {
        HWREG(RFC_MCERAM_BASE + 4 * i) = patchGenookMce[i];
    }

#else
    const uint32_t* pS = patchGenookMce;
    volatile unsigned long* pD = &HWREG(RFC_MCERAM_BASE);
    uint32_t t1, t2, t3, t4, t5, t6, t7, t8;
    uint32_t nIterations = 57;

    do
    {
        t1 = *pS++;
        t2 = *pS++;
        t3 = *pS++;
        t4 = *pS++;
        t5 = *pS++;
        t6 = *pS++;
        t7 = *pS++;
        t8 = *pS++;
        *pD++ = t1;
        *pD++ = t2;
        *pD++ = t3;
        *pD++ = t4;
        *pD++ = t5;
        *pD++ = t6;
        *pD++ = t7;
        *pD++ = t8;
    } while (--nIterations);

    t1 = *pS++;
    t2 = *pS++;
    t3 = *pS++;
    t4 = *pS++;
    t5 = *pS++;
    t6 = *pS++;
    t7 = *pS++;
    *pD++ = t1;
    *pD++ = t2;
    *pD++ = t3;
    *pD++ = t4;
    *pD++ = t5;
    *pD++ = t6;
    *pD++ = t7;
#endif
}

#endif
