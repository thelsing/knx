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

#include <stdint.h>
#include <stdbool.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/drivers/PIN.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOCC26XX.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(driverlib/interrupt.h)
#include DeviceFamily_constructPath(driverlib/prcm.h)
#include DeviceFamily_constructPath(driverlib/gpio.h)
#include DeviceFamily_constructPath(driverlib/ioc.h)

#if defined(__IAR_SYSTEMS_ICC__)
#include <intrinsics.h>
#define PIN2IOID(pin)   (31 - __CLZ(pin))
#define IOID2PIN(ioid)  (1 << ioid)
#endif

#if defined(__TI_COMPILER_VERSION__)
#define PIN2IOID(pin)   (31 - __clz(pin))
#define IOID2PIN(ioid)  (1 << ioid)
#endif

#if defined(__GNUC__) && !defined(__TI_COMPILER_VERSION__)
#define PIN2IOID(pin)   (31 - __builtin_clz(pin))
#define IOID2PIN(ioid)  (1 << ioid)
#endif

/*
 * By default disable both asserts and log for this module.
 * This must be done before DebugP.h is included.
 */
#ifndef DebugP_ASSERT_ENABLED
#define DebugP_ASSERT_ENABLED 0
#endif
#ifndef DebugP_LOG_ENABLED
#define DebugP_LOG_ENABLED 0
#endif

static PIN_State gpioPinState;
static PIN_Handle gpioPinHandle;
static PIN_Config gpioPinTable[] = {
    PIN_TERMINATE
};

/*
 * Map GPIO_INT types to corresponding PIN interrupt options
 */
static const uint32_t interruptType[] = {
    0,                  /* Undefined interrupt type */
    PIN_IRQ_NEGEDGE,    /* Interrupt on falling edge */
    PIN_IRQ_POSEDGE,    /* Interrupt on rising edge */
    PIN_IRQ_BOTHEDGES,  /* Interrupt on both edges */
    0,                  /* Interrupt on low level, not supported */
    0                   /* Interrupt on high level, not supported */
};

/*
 * EDGE_IRQ_EN_MASK is used to strip the EDGE_IRQ_EN bit (bit 18)
 * of an updateMask argument to prevent auto-enabling of interrupt
 * by PIN_setConfig().
 */
#define EDGE_IRQ_EN_MASK        (0xFFFBFFFF)

/* Table of GPIO input types */
const uint32_t inPinTypes [] = {
    PIN_INPUT_EN | PIN_NOPULL,      /* GPIO_CFG_IN_NOPULL */
    PIN_INPUT_EN | PIN_PULLUP,      /* GPIO_CFG_IN_PU */
    PIN_INPUT_EN | PIN_PULLDOWN     /* GPIO_CFG_IN_PD */
};

/* Table of GPIO output types */
const uint32_t outPinTypes [] = {
    PIN_GPIO_OUTPUT_EN | PIN_PUSHPULL,                   /* GPIO_CFG_OUT_STD */
    PIN_GPIO_OUTPUT_EN | PIN_OPENDRAIN | PIN_NOPULL,     /* GPIO_CFG_OUT_OD_NOPULL */
    PIN_GPIO_OUTPUT_EN | PIN_OPENDRAIN | PIN_PULLUP,     /* GPIO_CFG_OUT_OD_PU */
    PIN_GPIO_OUTPUT_EN | PIN_OPENDRAIN | PIN_PULLDOWN    /* GPIO_CFG_OUT_OD_PD */
};

/* Table of GPIO drive strengths */
const uint32_t outPinStrengths [] = {
    PIN_DRVSTR_MIN,    /* GPIO_CFG_OUT_STR_LOW */
    PIN_DRVSTR_MED,    /* GPIO_CFG_OUT_STR_MED */
    PIN_DRVSTR_MAX     /* GPIO_CFG_OUT_STR_HIGH */
};

#define NUM_PORTS           1
#define NUM_PINS_PER_PORT   32

/*
 * Extracts the GPIO interrupt type from the pinConfig.  Value to index into the
 * interruptType table.
 */
#define getIntTypeNumber(pinConfig) \
    ((pinConfig & GPIO_CFG_INT_MASK) >> GPIO_CFG_INT_LSB)

/* Uninitialized callbackInfo pinIndex */
#define CALLBACK_INDEX_NOT_CONFIGURED 0xFF

/*
 * Device specific interpretation of the GPIO_PinConfig content
 */
typedef struct PinConfig {
    uint8_t ioid;
    uint8_t added;  /* 0 = pin has not been added to gpioPinState */
    uint16_t config;
} PinConfig;

/*
 * User defined pin indexes assigned to a port's pins.
 * Used by pin callback function to locate callback assigned
 * to a pin.
 */
typedef struct PortCallbackInfo {
    /*
     * the port's corresponding
     * user defined pinId indices
     */
    uint8_t pinIndex[NUM_PINS_PER_PORT];
} PortCallbackInfo;

/*
 * Only one PortCallbackInfo object is needed for CC26xx since the 32 pins
 * are all on one port.
 */
static PortCallbackInfo gpioCallbackInfo;

/*
 *  Bit mask used to keep track of which pins (IOIDs) of the GPIO objects
 *  in the config structure have interrupts enabled.
 */
static uint32_t configIntsEnabledMask = 0;

/*
 * Internal boolean to confirm that GPIO_init() has been called.
 */
static bool initCalled = false;

extern const GPIOCC26XX_Config GPIOCC26XX_config;

/*
 *  ======== getInPinTypesIndex ========
 */
static inline uint32_t getInPinTypesIndex(uint32_t pinConfig)
{
    uint32_t index;

    index = (pinConfig & GPIO_CFG_IN_TYPE_MASK) >> GPIO_CFG_IN_TYPE_LSB;

    /*
     * If index is out-of-range, default to 0. This should never
     * happen, but it's needed to keep Klocwork checker happy.
     */
    if (index >= sizeof(inPinTypes) / sizeof(inPinTypes[0])) {
        index = 0;
    }

    return (index);
}

/*
 *  ======== getInterruptTypeIndex ========
 */
static inline uint32_t getInterruptTypeIndex(uint32_t pinConfig)
{
    uint32_t index;

    index = (pinConfig & GPIO_CFG_INT_MASK) >> GPIO_CFG_INT_LSB;

    /*
     * If index is out-of-range, default to 0. This should never
     * happen, but it's needed to keep Klocwork checker happy.
     */
    if (index >= sizeof(interruptType) / sizeof(interruptType[0])) {
        index = 0;
    }

    return (index);
};

/*
 *  ======== getOutPinTypesIndex ========
 */
static inline uint32_t getOutPinTypesIndex(uint32_t pinConfig)
{
    uint32_t index;

    index = (pinConfig & GPIO_CFG_OUT_TYPE_MASK) >> GPIO_CFG_OUT_TYPE_LSB;

    /*
     * If index is out-of-range, default to 0. This should never
     * happen, but it's needed to keep Klocwork checker happy.
     */
    if (index >= sizeof(outPinTypes) / sizeof(outPinTypes[0])) {
        index = 0;
    }

    return (index);
}

/*
 *  ======== getOutPinStrengthsIndex ========
 */
static inline uint32_t getOutPinStrengthsIndex(uint32_t pinConfig)
{
    uint32_t index;

    index = (pinConfig & GPIO_CFG_OUT_STRENGTH_MASK) >>
        GPIO_CFG_OUT_STRENGTH_LSB;

    /*
     * If index is out-of-range, default to 0. This should never
     * happen, but it's needed to keep Klocwork checker happy.
     */
    if (index >= sizeof(outPinStrengths) / sizeof(outPinStrengths[0])) {
        index = 0;
    }

    return (index);
}

/*
 *  ======== getPinNumber ========
 *
 *  Internal function to efficiently find the index of the right most set bit.
 */
static inline uint32_t getPinNumber(uint32_t x) {
    return(x); /* ioid is the same as the pinNumber */
}

/*
 *  ======== GPIO_clearInt ========
 */
void GPIO_clearInt(uint_least8_t index)
{
    PinConfig *config = (PinConfig *) &GPIOCC26XX_config.pinConfigs[index];

    /* Clear interrupt flag */
    PIN_clrPendInterrupt(gpioPinHandle, config->ioid);
}

/*
 *  ======== GPIO_disableInt ========
 */
void GPIO_disableInt(uint_least8_t index)
{
    unsigned int key;
    PinConfig *config = (PinConfig *) &GPIOCC26XX_config.pinConfigs[index];

    /* Make atomic update */
    key = HwiP_disable();

    /* Disable interrupt. */
    PIN_setInterrupt(gpioPinHandle, config->ioid | PIN_IRQ_DIS);

    configIntsEnabledMask &= ~(1 << config->ioid);

    HwiP_restore(key);
}

/*
 *  ======== GPIO_enableInt ========
 */
void GPIO_enableInt(uint_least8_t index)
{
    unsigned int key;
    PinConfig *config = (PinConfig *) &GPIOCC26XX_config.pinConfigs[index];
    uint32_t intTypeNum;

    /* Make atomic update */
    key = HwiP_disable();

    /* Get the index into the interruptType array */
    intTypeNum = getIntTypeNumber((config->config << 16));

    /* Enable interrupt. */
    PIN_setInterrupt(gpioPinHandle, (config->ioid | interruptType[intTypeNum]));

    configIntsEnabledMask |= (1 << config->ioid);

    HwiP_restore(key);
}

/*
 *  ======== GPIO_getConfig ========
 */
void GPIO_getConfig(uint_least8_t index, GPIO_PinConfig *pinConfig)
{
    *pinConfig = GPIOCC26XX_config.pinConfigs[index];
}

/*
 *  ======== GPIO_hwiIntFxn ========
 *  Hwi function that processes GPIO interrupts.
 */
void GPIO_hwiIntFxn(PIN_Handle pinHandle, PIN_Id pinId)
{
    unsigned int      pinIndex;

    pinIndex = gpioCallbackInfo.pinIndex[pinId];

    /* only call plugged callbacks */
    if (pinIndex != CALLBACK_INDEX_NOT_CONFIGURED) {
        /* PIN_swi() will call callback even if interrupt is disabled */
        if ((1 << pinId) & configIntsEnabledMask) {
            GPIOCC26XX_config.callbacks[pinIndex](pinIndex);
        }
    }
}

/*
 *  ======== GPIO_init ========
 */
void GPIO_init()
{
    unsigned int i, hwiKey;
    SemaphoreP_Handle sem;
    static SemaphoreP_Handle initSem;

    /* speculatively create a binary semaphore */
    sem = SemaphoreP_createBinary(1);

    /* There is no way to inform user of this fatal error. */
    if (sem == NULL) return;

    hwiKey = HwiP_disable();

    if (initSem == NULL) {
        initSem = sem;
        HwiP_restore(hwiKey);
    }
    else {
        /* init already called */
        HwiP_restore(hwiKey);
        /* delete unused Semaphore */
        if (sem) SemaphoreP_delete(sem);
    }

    /* now use the semaphore to protect init code */
    SemaphoreP_pend(initSem, SemaphoreP_WAIT_FOREVER);

    /* Only perform init once */
    if (initCalled) {
        SemaphoreP_post(initSem);
        return;
    }

    gpioPinHandle = PIN_open(&gpioPinState, gpioPinTable);

    /* install our Hwi callback function */
    PIN_registerIntCb(gpioPinHandle, GPIO_hwiIntFxn);

    for (i = 0; i < NUM_PINS_PER_PORT; i++) {
        gpioCallbackInfo.pinIndex[i] = CALLBACK_INDEX_NOT_CONFIGURED;
    }

    /*
     * Configure pins and create Hwis per static array content
     */
    for (i = 0; i < GPIOCC26XX_config.numberOfPinConfigs; i++) {
        if (!(GPIOCC26XX_config.pinConfigs[i] & GPIO_DO_NOT_CONFIG)) {
            GPIO_setConfig(i, GPIOCC26XX_config.pinConfigs[i]);
        }
        if (i < GPIOCC26XX_config.numberOfCallbacks) {
            if (GPIOCC26XX_config.callbacks[i] != NULL) {
                /* create Hwi as necessary */
                GPIO_setCallback(i, GPIOCC26XX_config.callbacks[i]);
            }
        }
    }

    initCalled = true;

    SemaphoreP_post(initSem);
}

/*
 *  ======== GPIO_read ========
 */
uint_fast8_t GPIO_read(uint_least8_t index)
{
    unsigned int value;

    PinConfig *config = (PinConfig *) &GPIOCC26XX_config.pinConfigs[index];

    value = GPIO_readMultiDio(IOID2PIN(config->ioid));

    value = value & (IOID2PIN(config->ioid)) ? 1 : 0;

    return (value);
}

/*
 *  ======== GPIO_setCallback ========
 */
void GPIO_setCallback(uint_least8_t index, GPIO_CallbackFxn callback)
{
    uint32_t   pinNum;
    PinConfig *config = (PinConfig *) &GPIOCC26XX_config.pinConfigs[index];

    /*
     * Ignore bogus callback indexes.
     * Required to prevent out-of-range callback accesses if
     * there are configured pins without callbacks
     */
    if (index >= GPIOCC26XX_config.numberOfCallbacks) {
        return;
    }

    /*
     * plug the pin index into the corresponding
     * port's callbackInfo pinIndex entry
     */
    pinNum = getPinNumber(config->ioid);

    if (callback == NULL) {
        gpioCallbackInfo.pinIndex[pinNum] =
            CALLBACK_INDEX_NOT_CONFIGURED;
    }
    else {
        gpioCallbackInfo.pinIndex[pinNum] = index;
    }

    /*
     * Only update callBackFunctions entry if different.
     * This allows the callBackFunctions array to be in flash for static systems.
     */
    if (GPIOCC26XX_config.callbacks[index] != callback) {
        GPIOCC26XX_config.callbacks[index] = callback;
    }
}

/*
 *  ======== GPIO_setConfig ========
 */
int_fast16_t GPIO_setConfig(uint_least8_t index, GPIO_PinConfig pinConfig)
{
    unsigned int key;
    uint16_t direction;
    GPIO_PinConfig gpioPinConfig;
    PIN_Config pinPinConfig = 0; /* PIN driver PIN_config ! */
    PinConfig *config = (PinConfig *) &GPIOCC26XX_config.pinConfigs[index];

    if (pinPinConfig & GPIO_DO_NOT_CONFIG) {
        return (GPIO_STATUS_SUCCESS);
    }

    if ((pinConfig & GPIO_CFG_IN_INT_ONLY) == 0) {
        if (pinConfig & GPIO_CFG_INPUT) {
            /* configure input */
            direction = GPIO_OUTPUT_DISABLE;
            pinPinConfig = inPinTypes[getInPinTypesIndex(pinConfig)];
        }
        else {
            /* configure output */
            direction = GPIO_OUTPUT_ENABLE;
            pinPinConfig = outPinTypes[getOutPinTypesIndex(pinConfig)];
            pinPinConfig |=
                outPinStrengths[getOutPinStrengthsIndex(pinConfig)];
        }

        key = HwiP_disable();

        /* Set output value */
        if (direction == GPIO_OUTPUT_ENABLE) {
            pinPinConfig |= ((pinConfig & GPIO_CFG_OUT_HIGH) ? PIN_GPIO_HIGH : PIN_GPIO_LOW);
        }

        /*
         *  Update pinConfig with the latest GPIO configuration and
         *  clear the GPIO_DO_NOT_CONFIG bit if it was set.
         */
        gpioPinConfig = GPIOCC26XX_config.pinConfigs[index];
        gpioPinConfig &= ~(GPIO_CFG_IO_MASK | GPIO_DO_NOT_CONFIG);
        gpioPinConfig |= (pinConfig & GPIO_CFG_IO_MASK);
        GPIOCC26XX_config.pinConfigs[index] = gpioPinConfig;

        HwiP_restore(key);
    }

    /* Set type of interrupt and then clear it */
    if (pinConfig & GPIO_CFG_INT_MASK) {
        key = HwiP_disable();

        /*
         *  Update pinConfig with the latest interrupt configuration and
         *  clear the GPIO_DO_NOT_CONFIG bit if it was set.
         */
        gpioPinConfig = GPIOCC26XX_config.pinConfigs[index];
        gpioPinConfig &= ~(GPIO_CFG_INT_MASK | GPIO_DO_NOT_CONFIG);
        gpioPinConfig |= (pinConfig & GPIO_CFG_INT_MASK);
        GPIOCC26XX_config.pinConfigs[index] = gpioPinConfig;

        pinPinConfig |= interruptType[getInterruptTypeIndex(pinConfig)];
        HwiP_restore(key);
    }

    /* or in the pin ID */
    pinPinConfig |= (uint32_t)config->ioid;

    if (config->added == 0) {
        if (PIN_add(gpioPinHandle, pinPinConfig) != PIN_SUCCESS) {
            return (GPIO_STATUS_ERROR);
        }
        config->added = 1;
    }
    else {
        uint32_t bmMask;
        if (pinConfig & GPIO_CFG_IN_INT_ONLY) {
            bmMask = PIN_BM_IRQ;
        }
        else {
            bmMask = PIN_BM_ALL;
        }
        if (PIN_setConfig(gpioPinHandle, bmMask & EDGE_IRQ_EN_MASK,
                          pinPinConfig) != PIN_SUCCESS) {
            return (GPIO_STATUS_ERROR);
        }
    }

    return (GPIO_STATUS_SUCCESS);
}

/*
 *  ======== GPIO_toggle ========
 */
void GPIO_toggle(uint_least8_t index)
{
    unsigned int key;
    PinConfig *config = (PinConfig *) &GPIOCC26XX_config.pinConfigs[index];

    /* Make atomic update */
    key = HwiP_disable();

    GPIO_toggleDio(config->ioid);

    /* Update config table entry with value written */
    GPIOCC26XX_config.pinConfigs[index] ^= GPIO_CFG_OUT_HIGH;

    HwiP_restore(key);
}

/*
 *  ======== GPIO_write ========
 */
void GPIO_write(uint_least8_t index, unsigned int value)
{
    unsigned int key;
    PinConfig *config = (PinConfig *) &GPIOCC26XX_config.pinConfigs[index];

    key = HwiP_disable();

    if (value) {
        /* Set the pinConfig output bit to high */
        GPIOCC26XX_config.pinConfigs[index] |= GPIO_CFG_OUT_HIGH;
    }
    else {
        /* Clear output from pinConfig */
        GPIOCC26XX_config.pinConfigs[index] &= ~GPIO_CFG_OUT_HIGH;
    }

    value = value ? IOID2PIN(config->ioid) : 0;

    GPIO_writeMultiDio(IOID2PIN(config->ioid), value);

    HwiP_restore(key);
}

/*
 *  ======== GPIOCC26xx_release ========
 */
void GPIOCC26xx_release(int index)
{
    PinConfig *config = (PinConfig *) &GPIOCC26XX_config.pinConfigs[index];
    unsigned int key;

    key = HwiP_disable();

    if (config->added) {
        /* disable the pin's interrupt */
        GPIO_disableInt(index);

        /* remove its callback */
        GPIO_setCallback(index, NULL);

        config->added = 0;

        PIN_remove(gpioPinHandle, config->ioid);
    }

    HwiP_restore(key);
}
