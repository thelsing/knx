/******************************************************************************
 *  Filename:       rf_patch_mce_genfsk.h
 *  Revised:        $Date: 2019-01-31 15:04:25 +0100 (to, 31 jan 2019) $
 *  Revision:       $Revision: 18842 $
 *
 *  Description: RF core patch for CC13x0 Generic FSK
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

#ifndef _RF_PATCH_MCE_GENFSK_H
#define _RF_PATCH_MCE_GENFSK_H

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

MCE_PATCH_TYPE patchGenfskMce[460] =
    {
        0x2fcf602b,
        0x030c3f9d,
        0x070c680a,
        0xfff0003f,
        0xff0000ff,
        0x00030006,
        0x3d1f0007,
        0x00000000,
        0x000f0400,
        0x03870000,
        0x40f4000b,
        0x80000043,
        0x06708082,
        0x091e0000,
        0x00540510,
        0x02000005,
        0x00613e10,
        0x002f0000,
        0x027f3030,
        0x00000000,
        0xaa000000,
        0x72200000,
        0xa32d7248,
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
        0x39521020,
        0x00200670,
        0x11011630,
        0x6c011401,
        0x60696068,
        0x635360e5,
        0x60686068,
        0x60686068,
        0x60696068,
        0x635360e5,
        0x60686068,
        0x60686068,
        0x72201210,
        0x7310730f,
        0x81817311,
        0x91800010,
        0x6040b070,
        0xc030605e,
        0xc0b16792,
        0xc470c282,
        0x6f131820,
        0x16116e23,
        0x686f1612,
        0x9ab07830,
        0x9ac07840,
        0x9ad07850,
        0xc5a0c482,
        0x40821820,
        0x6e231203,
        0x687f1612,
        0x97e078a0,
        0x7276605e,
        0x94908160,
        0x39808140,
        0x10012a70,
        0x84321611,
        0xc0f38444,
        0xc200c0f5,
        0x40aa1c01,
        0x1c10c100,
        0x4ca240a0,
        0x18031013,
        0x1a131830,
        0x39121a10,
        0x60aa689d,
        0x60aa13f3,
        0x101513f3,
        0x1850c100,
        0x1a101a15,
        0x68a83914,
        0x7100b0d8,
        0xa0d8b108,
        0xb760b200,
        0x978087e0,
        0xb0c1b0f1,
        0xb0027100,
        0xb0f1b012,
        0x7276a0c1,
        0xb003b480,
        0x7229b013,
        0x7100b0d0,
        0x8140b100,
        0x71009290,
        0x8140b100,
        0x44c322f0,
        0x1c0313f0,
        0x929340cf,
        0x71009492,
        0x9295b100,
        0x71009494,
        0xb0d0b100,
        0x7000a480,
        0xc030a0d1,
        0xc0409760,
        0xb0f19780,
        0x7100b0c1,
        0xa0c1b0f1,
        0xa0037276,
        0xa200a002,
        0x730f7000,
        0xc0407310,
        0xc1006792,
        0x648591c0,
        0xb0f3b483,
        0x7100b0c3,
        0x64d6a0c3,
        0xb006605e,
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
        0x16101030,
        0x31211001,
        0x22103930,
        0x12204110,
        0x10033150,
        0x00103180,
        0x93501630,
        0x12041202,
        0x41232273,
        0x97c08430,
        0x1a8287d2,
        0x97c08450,
        0x1a8487d4,
        0x22636125,
        0x84404130,
        0x87d097c0,
        0x14021a80,
        0x97c08460,
        0x1a8087d0,
        0x613c1404,
        0x78918440,
        0x97c00410,
        0x1a4287d2,
        0x78918460,
        0x97c00410,
        0x1a4487d4,
        0x31543152,
        0x06333963,
        0x38321613,
        0x31823834,
        0x31843982,
        0x95720042,
        0x90307810,
        0x78209050,
        0x90609040,
        0x8ae2b205,
        0x93028303,
        0xc00bc00c,
        0x31808140,
        0x39403980,
        0xc0f38141,
        0xc0140431,
        0xc0021441,
        0x695e1412,
        0x847d3122,
        0x140dc010,
        0x142d312d,
        0x318e8ace,
        0x397e311e,
        0x31498ac9,
        0x39493979,
        0x109a3129,
        0x720d720c,
        0xb101720e,
        0x7100b0d1,
        0xa0d1b072,
        0xb06ea04e,
        0xb06cb011,
        0x978ab089,
        0xb7647276,
        0xc662a764,
        0xc04f9762,
        0x8ab166d4,
        0x458c22f1,
        0x22f18ad1,
        0x6232458c,
        0xb0737100,
        0x80b7b760,
        0x45c32207,
        0x8ab1a760,
        0x419d22f1,
        0x419d2237,
        0x80b0b113,
        0x45982230,
        0x22e161ab,
        0x809041b0,
        0x41b02250,
        0x8210b0f5,
        0x418c2220,
        0xb7649789,
        0xb0f6a764,
        0x978d618c,
        0xa764b764,
        0x618cb0f6,
        0x22f08ad0,
        0x223741bc,
        0xb07541bc,
        0x80b0b113,
        0x45b62230,
        0x618cb087,
        0x431722d1,
        0x22208090,
        0x669a4317,
        0x978f618c,
        0x8410c7f3,
        0x39803180,
        0x00303183,
        0xb0879410,
        0xb0f2a0e3,
        0xb0f5a0c2,
        0xb0f1a0c5,
        0xa0c6b0c1,
        0xb113b110,
        0x220080b0,
        0x223045d4,
        0x710045d4,
        0x97801260,
        0xb88fb0f1,
        0x85708961,
        0x95511801,
        0x8a718a60,
        0xa487a488,
        0x1801c022,
        0x4df41c21,
        0x49f21412,
        0x1c0161f5,
        0x4df441f5,
        0x61f5b487,
        0xb041b488,
        0x8ad0b061,
        0x41fd22e0,
        0x22208210,
        0x71004570,
        0xb06eb04e,
        0x220180b1,
        0x2231468c,
        0x7276468c,
        0x8471b0f6,
        0xc2603121,
        0x97801410,
        0x9760c7e0,
        0x9760c6f0,
        0xb0c6b0f6,
        0xb7b0a0c1,
        0x8a748a63,
        0x8a948a83,
        0x80b17100,
        0x468c2201,
        0x468c2231,
        0x22c08ab0,
        0x89914624,
        0x41702201,
        0xc00081c1,
        0x847091c0,
        0x6a2881a2,
        0xc30091c1,
        0xb2019070,
        0xa0e3a0e0,
        0x7000a044,
        0xb0737100,
        0x80b7b760,
        0x46512207,
        0x466f2237,
        0x8ab1a760,
        0x424a22e1,
        0x22508090,
        0xb0f5424a,
        0x22208210,
        0x978d4232,
        0xa764b764,
        0x6232b0f6,
        0x431722d1,
        0x22208090,
        0x669a4317,
        0x978f6232,
        0xa0c2b0f2,
        0xa0c5b0f5,
        0xb0c1b0f1,
        0xb110a0c6,
        0x80b0b113,
        0x46592200,
        0x46592230,
        0x12607100,
        0xb0f19780,
        0x8961b88f,
        0x31808570,
        0x18013d80,
        0x8a609551,
        0xa1828a71,
        0x978f61e6,
        0xa0c2b0f2,
        0xa0c5b0f5,
        0xb0c1b0f1,
        0xb110a0c6,
        0x80b0b113,
        0x46772200,
        0x46772230,
        0x12607100,
        0xb0f19780,
        0x8961b88f,
        0x3d808570,
        0x95511801,
        0x8a918a80,
        0x61e6b182,
        0xa760b073,
        0xa7b0b760,
        0xa04eb072,
        0xb011b06e,
        0x22f08ab0,
        0x220145c3,
        0x626f4651,
        0x22b08ab0,
        0x1e3b46a0,
        0x62a246d2,
        0x46d21e7b,
        0xb889c00b,
        0x31808940,
        0x16103d80,
        0x140c3d30,
        0x220080b0,
        0x700042ae,
        0x39838ab3,
        0x8ab106f3,
        0x0401cff0,
        0x1c1c3031,
        0x12004eca,
        0x1c0c1810,
        0x80b04acc,
        0x42bf2200,
        0x10c27000,
        0x3c321612,
        0x83208ae1,
        0x42ce2210,
        0x930162d0,
        0x7000b0f2,
        0x62bb101c,
        0x62bb100c,
        0x62c71821,
        0x62c71421,
        0x62c8161b,
        0xb0f1b0f6,
        0xb113b110,
        0xb0f2b0f5,
        0x720d720c,
        0xb0e0720e,
        0x8ab2b0e3,
        0x42e522f2,
        0xb763b0c6,
        0x8ad062e8,
        0x430822f0,
        0xa404b405,
        0xa429b428,
        0x3180caa0,
        0x0001caa1,
        0x94619451,
        0x31838ad3,
        0x84103983,
        0x39803180,
        0x00303183,
        0x84009410,
        0x39503150,
        0x39838ad3,
        0xc1f406f3,
        0x31841834,
        0x00403134,
        0xb0899400,
        0x431222e2,
        0x394a8aca,
        0x312a398a,
        0xb0c5978a,
        0xb763b0c6,
        0x22d28ab2,
        0xb0c24316,
        0xb20f7000,
        0xa0e3a0e0,
        0xb764978e,
        0xb0f6a764,
        0xb113b110,
        0x22f08210,
        0xb0f54320,
        0xa0048002,
        0xa001a006,
        0x72047203,
        0x6792c050,
        0xb7647100,
        0xb0c5b0f6,
        0x7100a20f,
        0xa0c5b0f5,
        0x90307810,
        0x78209002,
        0x90609040,
        0xa20fb072,
        0x978a66d4,
        0xb0f6a764,
        0xb88c6185,
        0x89a48180,
        0x31843924,
        0x91840004,
        0x6792c060,
        0x72767376,
        0x72067248,
        0x72047202,
        0x73067305,
        0x1300605e,
        0xb32d91b0,
        0x6792c070,
        0x64f3b0f8,
        0x1a101200,
        0xc3809780,
        0xc2809760,
        0xa0c19760,
        0x8090b0c6,
        0x44402200,
        0x1e048154,
        0x97844363,
        0x8552b0f6,
        0x9862d080,
        0x89916792,
        0x43792211,
        0x8a938a82,
        0x9862e090,
        0x67929873,
        0x8a62637f,
        0xe0a08a73,
        0x98739862,
        0x87906792,
        0x1c018781,
        0x18014b8f,
        0x4b8d1ef1,
        0x1af18781,
        0x71009781,
        0x16f1b0f6,
        0xa2059781,
        0xb0f67100,
        0x6341a0c6,
        0x88409850,
        0x47932200,
        0x7000b830};

PATCH_FUN_SPEC void rf_patch_mce_genfsk(void)
{
#ifdef __PATCH_NO_UNROLLING
    uint32_t i;

    for (i = 0; i < 460; i++)
    {
        HWREG(RFC_MCERAM_BASE + 4 * i) = patchGenfskMce[i];
    }

#else
    const uint32_t* pS = patchGenfskMce;
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
    *pD++ = t1;
    *pD++ = t2;
    *pD++ = t3;
    *pD++ = t4;
#endif
}

#endif
