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

/*
 *  ======== main_nortos.c ========
 */
#include <stdint.h>
#include <stddef.h>

#include "SEGGER_RTT.h"

#include <NoRTOS.h>

/* Example/Board Header files */
#include <ti/drivers/Board.h>
#include <ti/drivers/NVS.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/pin/PINCC26XX.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/driverlib_release.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/vims.h)

#include "knx_wrapper.h"

#define USE_32KHZ_XTAL_AS_LF_CLOCK false

extern void *mainThread(void *arg0);

void __cxa_pure_virtual() 
{ 
    SEGGER_RTT_WriteString(0, "Pure virtual method called! System halted.\r\n");
    while (1); 
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    setup();

    /* Loop forever echoing */
    while (1) {
        loop();
    }
}

/*
 *  ======== main ========
 */
int main(void)
{
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);

    SEGGER_RTT_WriteString(0, "System startup.\r\n");

    DRIVERLIB_ASSERT_CURR_RELEASE();

    // Enable flash cache
    VIMSModeSet(VIMS_BASE, VIMS_MODE_ENABLED);
    // Configure round robin arbitration and prefetching
    VIMSConfigure(VIMS_BASE, true, true);

    // set LF clock source needed for low power deepSleep() function further below
    //OSCClockSourceSet(OSC_SRC_CLK_LF, USE_32KHZ_XTAL_AS_LF_CLOCK ? OSC_XOSC_LF : OSC_RCOSC_LF);

    // Call driver init functions
    Board_init();

    // TI Drivers init before starting NoRTOS/RTOS
    UART_init();
    NVS_init();
    //I2C_init();
    //SPI_init();
    //TRNG_init();

    // Start NoRTOS
    NoRTOS_start();

    // Call mainThread function
    mainThread(NULL);

    // Shall not be reached
    while (1) {}
}
