/*
 * Copyright (c) 2018, Texas Instruments Incorporated
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
 *  ======== AESGCM.c ========
 *
 *  This file contains default values for the AESGCM_Params struct.
 *
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <ti/drivers/AESGCM.h>
#include <ti/drivers/dpl/SemaphoreP.h>

const AESGCM_Params AESGCM_defaultParams = {
    .returnBehavior = AESGCM_RETURN_BEHAVIOR_BLOCKING,
    .callbackFxn = NULL,
    .timeout = SemaphoreP_WAIT_FOREVER,
    .custom = NULL,
};

/*
 *  ======== AESGCM_Params_init ========
 */
void AESGCM_Params_init(AESGCM_Params *params){
    *params = AESGCM_defaultParams;
}

/*
 *  ======== AESGCM_Operation_init ========
 */
void AESGCM_Operation_init(AESGCM_Operation *operationStruct) {
    memset(operationStruct, 0x00, sizeof(AESGCM_Operation));

    /* The only supported ivLength is 12 for now */
    operationStruct->ivLength = 12;
}
