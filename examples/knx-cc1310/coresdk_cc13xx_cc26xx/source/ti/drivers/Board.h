/*
 * Copyright (c) 2015-2018, Texas Instruments Incorporated
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
 *  @file       Board.h
 *  @brief      Portable board-specific symbols
 *
 *  The Board header file should be included in an application as follows:
 *  @code
 *  #include "Board.h"
 *  @endcode
 *
 *  This header serves as device-independent interface for applications using
 *  peripherals connected to the device via standard digital interfaces; e.g,
 *  GPIO, SPI, I2C, UART, etc. Its purpose is to enable application code that
 *  references a peripheral to be portable to any device and board that
 *  supports the peripheral.
 *
 *  ## Usage ##
 *
 *  @anchor ti_drivers_Board_Synopsis
 *  ### Synopsis #
 *  @anchor ti_drivers_Board_Synopsis_Code
 *  @code
 *  #include "Board.h"
 *
 *  void main(void)
 *  {
 *      Board_init();
 *       :
 *  }
 *  @endcode
 *
 *  ## Initializing the hardware ##
 *
 *  \p Board_init() must be called before any other driver API. This function
 *  calls the device specific initialization code that is required to as soon
 *  as possible after a device reset; e.g., to initialize clocks and power
 *  management functionality.
 *
 *  ## Portable peripheral usage
 *
 *  Each driver module declares symbols in \p Board.h that, if used, will
 *  improve code portability between both different devices and boards.
 *
 *  @anchor ti_drivers_I2C_Example_portable
 *  For example, the I2C driver adds \p Board.h symbol definitions of the form
 *  * <I>bus_name</I> - the I2C bus instance ID,
 *  * <I>bus_name</I>_MAXBITRATE - the maximum supported BITRATE for the bus
 *    <I>bus_name</I>, and
 *  * Board_I2C_<I>comp_name</I>_ADDR - the slave address for the named I2C
 *    component
 *  where <I>comp_name</I> is the name given to an I2C peripheral by the
 *  board manufacturer, and <I>bus_name</I> is the user defined name of the
 *  I2C bus instance.  These symbols enable applications to portably acquire
 *  an I2C bus handle and control an I2C slave on that bus.
 *  @code
 *  #include <ti/drivers/I2C.h>
 *  #include "Board.h"
 *
 *  // portably open an I2C bus instance
 *  I2C_Params i2cParams;
 *  I2C_Params_init(&i2cParams);
 *  i2cParams.bitRate = Board_I2C0_MAXBITRATE;  // bus name == Board_I2C0
 *  i2cHandle = I2C_open(Board_I2C0, &i2cParams);
 *
 *  // portably read from an I2C slave
 *  I2C_Transaction trans;
 *  trans.slaveAddress = Board_I2C_TMP006_ADDR; // component name = TMP006
 *  trans.readBuf = ...;
 *  trans.readCount = ...;
 *  trans.writeCount = 0;
 *  I2C_transfer(i2cHandle, &trans);
 *  @endcode
 ******************************************************************************
 */

#ifndef ti_boards_Board__include
#define ti_boards_Board__include

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief  Performs "early" board-level initialization required by TI-DRIVERS
 *
 *  Board_init() must be called before any other TI-DRIVER API. This function
 *  calls all device and board specific initialization functions needed by
 *  TI-DRIVERS; e.g., to initialize clocks and power management functionality.
 *
 *  This function should only be called once and as early in the application's
 *  startup as possible.  In most applications, a call to Board_init() is the
 *  first statement in \p main().
 *
 *  @pre    \p Board_init must be called after every CPU reset and _prior_ to
 *          enabling any interrupts.
 */
extern void Board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_boards_Board_include */
