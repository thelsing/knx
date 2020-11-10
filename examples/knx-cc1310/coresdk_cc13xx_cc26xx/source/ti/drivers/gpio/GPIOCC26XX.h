/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
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
 *  @file       GPIOCC26XX.h
 *
 *  @brief      GPIO driver implementation for CC26xx/CC13xx devices
 *
 *  The GPIO header file should be included in an application as follows:
 *  @code
 *  #include <ti/drivers/GPIO.h>
 *  #include <ti/drivers/gpio/GPIOCC26XX.h>
 *  @endcode
 *
 *  Refer to @ref GPIO.h for a complete description of the GPIO
 *  driver APIs provided and examples of their use.
 *
 *  ### CC26XX GPIO Driver Configuration #
 *
 *  In order to use the GPIO APIs, the application is required
 *  to provide 3 structures in the Board.c file:
 *
 *  1.  An array of @ref GPIO_PinConfig elements that defines the
 *  initial configuration of each pin used by the application. A
 *  pin is referenced in the application by its corresponding index in this
 *  array. The pin type (that is, INPUT/OUTPUT), its initial state (that is
 *  OUTPUT_HIGH or LOW), interrupt behavior (RISING/FALLING edge, etc.)
 *  (see @ref GPIO_PinConfigSettings), and
 *  device specific pin identification (see @ref GPIOCC26XX_PinConfigIds)
 *  are configured in each element of this array.
 *  Below is an CC26XX device specific example of the GPIO_PinConfig array:
 *  @code
 *  //
 *  // Array of Pin configurations
 *  // NOTE: The order of the pin configurations must coincide with what was
 *  //       defined in CC2650_LAUNCH.h
 *  // NOTE: Pins not used for interrupts should be placed at the end of the
 *  //       array.  Callback entries can be omitted from callbacks array to
 *  //       reduce memory usage.
 *  //
 *  GPIO_PinConfig gpioPinConfigs[] = {
 *      // Input pins
 *      GPIOCC26XX_DIO_13 | GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_RISING,  // Button 0
 *      GPIOCC26XX_DIO_14 | GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_RISING,  // Button 1
 *
 *      // Output pins
 *      GPIOCC26XX_DIO_07 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,     //  Green LED
 *      GPIOCC26XX_DIO_06 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,     //  Red LED
 *  };
 *  @endcode
 *
 *  2.  An array of @ref GPIO_CallbackFxn elements that is used to store
 *  callback function pointers for GPIO pins configured with interrupts.
 *  The indexes for these array elements correspond to the pins defined
 *  in the @ref GPIO_PinConfig array. These function pointers can be defined
 *  statically by referencing the callback function name in the array
 *  element, or dynamically, by setting the array element to NULL and using
 *  GPIO_setCallback() at runtime to plug the callback entry.
 *  Pins not used for interrupts can be omitted from the callback array to
 *  reduce memory usage (if they are placed at the end of the @ref
 *  GPIO_PinConfig array). The callback function syntax should match the
 *  following:
 *  @code
 *  void (*GPIO_CallbackFxn)(unsigned int index);
 *  @endcode
 *  The index parameter is the same index that was passed to
 *  GPIO_setCallback(). This allows the same callback function to be used
 *  for multiple GPIO interrupts, by using the index to identify the GPIO
 *  that caused the interrupt.
 *  Below is an CC26XX device specific example of the @ref GPIO_CallbackFxn
 *  array:
 *  @code
 *  //
 *  // Array of callback function pointers
 *  // NOTE: The order of the pin configurations must coincide with what was
 *  //       defined in CC2650_LAUNCH.h
 *  // NOTE: Pins not used for interrupts can be omitted from callbacks array to
 *  //       reduce memory usage (if placed at end of gpioPinConfigs array).
 *  //
 *  GPIO_CallbackFxn gpioCallbackFunctions[] = {
 *      NULL,  //  Button 0
 *      NULL,  //  Button 1
 *  };
 *  @endcode
 *
 *  3.  The device specific GPIOCC26XX_Config structure that tells the GPIO
 *  driver where the two aforementioned arrays are and the number of elements
 *  in each. The interrupt priority of all pins configured to generate
 *  interrupts is also specified here. Values for the interrupt priority are
 *  device-specific. You should be well-acquainted with the interrupt
 *  controller used in your device before setting this parameter to a
 *  non-default value. The sentinel value of (~0) (the default value) is
 *  used to indicate that the lowest possible priority should be used.
 *  Below is an example of an initialized GPIOCC26XX_Config
 *  structure:
 *  @code
 *  const GPIOCC26XX_Config GPIOCC26XX_config = {
 *      .pinConfigs = (GPIO_PinConfig *)gpioPinConfigs,
 *      .callbacks = (GPIO_CallbackFxn *)gpioCallbackFunctions,
 *      .numberOfPinConfigs = sizeof(gpioPinConfigs)/sizeof(GPIO_PinConfig),
 *      .numberOfCallbacks = sizeof(gpioCallbackFunctions)/sizeof(GPIO_CallbackFxn),
 *      .intPriority = (~0)
 *  };
 *  @endcode
 *
 *  ============================================================================
 */

#ifndef ti_drivers_GPIOCC26XX__include
#define ti_drivers_GPIOCC26XX__include

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <ti/drivers/GPIO.h>
#include <ti/devices/DeviceFamily.h>

#include DeviceFamily_constructPath(driverlib/ioc.h)



/*!
 *  @brief  GPIO device specific driver configuration structure
 *
 *  The device specific GPIOCC26XX_Config structure that tells the GPIO
 *  driver where the two aforementioned arrays are and the number of elements
 *  in each. The interrupt priority of all pins configured to generate
 *  interrupts is also specified here. Values for the interrupt priority are
 *  device-specific. You should be well-acquainted with the interrupt
 *  controller used in your device before setting this parameter to a
 *  non-default value. The sentinel value of (~0) (the default value) is
 *  used to indicate that the lowest possible priority should be used.
 *
 *  Below is an example of an initialized GPIOCC26XX_Config
 *  structure:
 *  @code
 *  const GPIOCC26XX_Config GPIOCC26XX_config = {
 *      .pinConfigs = (GPIO_PinConfig *)gpioPinConfigs,
 *      .callbacks = (GPIO_CallbackFxn *)gpioCallbackFunctions,
 *      .numberOfPinConfigs = sizeof(gpioPinConfigs)/sizeof(GPIO_PinConfig),
 *      .numberOfCallbacks = sizeof(gpioCallbackFunctions)/sizeof(GPIO_CallbackFxn),
 *      .intPriority = (~0)
 *  };
 *  @endcode
 */
typedef struct GPIOCC26XX_Config {
    /*! Pointer to the board's GPIO_PinConfig array */
    GPIO_PinConfig  *pinConfigs;

    /*! Pointer to the board's GPIO_CallbackFxn array */
    GPIO_CallbackFxn  *callbacks;

    /*! Number of GPIO_PinConfigs defined */
    uint32_t numberOfPinConfigs;

    /*! Number of GPIO_Callbacks defined */
    uint32_t numberOfCallbacks;

    /*!
     *  Interrupt priority used for call back interrupts.
     *
     *  intPriority is the interrupt priority, as defined by the
     *  underlying OS.  It is passed unmodified to the underlying OS's
     *  interrupt handler creation code, so you need to refer to the OS
     *  documentation for usage.  If the driver uses the ti.dpl
     *  interface instead of making OS calls directly, then the HwiP port
     *  handles the interrupt priority in an OS specific way.  In the case
     *  of the SYS/BIOS port, intPriority is passed unmodified to Hwi_create().
     *
     *  Setting ~0 will configure the lowest possible priority
     */
    uint32_t intPriority;
} GPIOCC26XX_Config;

/*!
 *  \defgroup GPIOCC26XX_PinConfigIds GPIO pin identification macros used to configure GPIO pins
 *  @{
 */
/**
 *  @name Device specific GPIO port/pin identifiers to be used within the board's GPIO_PinConfig table.
 *  @{
*/
#define GPIOCC26XX_EMPTY_PIN  0xffff   /*!< @hideinitializer */

#define GPIOCC26XX_DIO_00    IOID_0    /*!< @hideinitializer */
#define GPIOCC26XX_DIO_01    IOID_1    /*!< @hideinitializer */
#define GPIOCC26XX_DIO_02    IOID_2    /*!< @hideinitializer */
#define GPIOCC26XX_DIO_03    IOID_3    /*!< @hideinitializer */
#define GPIOCC26XX_DIO_04    IOID_4    /*!< @hideinitializer */
#define GPIOCC26XX_DIO_05    IOID_5    /*!< @hideinitializer */
#define GPIOCC26XX_DIO_06    IOID_6    /*!< @hideinitializer */
#define GPIOCC26XX_DIO_07    IOID_7    /*!< @hideinitializer */

#define GPIOCC26XX_DIO_08    IOID_8    /*!< @hideinitializer */
#define GPIOCC26XX_DIO_09    IOID_9    /*!< @hideinitializer */
#define GPIOCC26XX_DIO_10    IOID_10   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_11    IOID_11   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_12    IOID_12   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_13    IOID_13   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_14    IOID_14   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_15    IOID_15   /*!< @hideinitializer */

#define GPIOCC26XX_DIO_16    IOID_16   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_17    IOID_17   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_18    IOID_18   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_19    IOID_19   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_20    IOID_20   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_21    IOID_21   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_22    IOID_22   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_23    IOID_23   /*!< @hideinitializer */

#define GPIOCC26XX_DIO_24    IOID_24   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_25    IOID_25   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_26    IOID_26   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_27    IOID_27   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_28    IOID_28   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_29    IOID_29   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_30    IOID_30   /*!< @hideinitializer */
#define GPIOCC26XX_DIO_31    IOID_31   /*!< @hideinitializer */

/** @} */
/** @} end of GPIOCC26XX_PinConfigIds group */

/*!
 *  @brief     Un-oonfigure a GPIO pin
 *
 *  Disables pin interrupt, clears callback, restores pin to default setting,
 *  removes pin from PIN object
 *
 *  @param      index    GPIO index
 */
extern void GPIOCC26xx_release(int index);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_GPIOCC26XX__include */
