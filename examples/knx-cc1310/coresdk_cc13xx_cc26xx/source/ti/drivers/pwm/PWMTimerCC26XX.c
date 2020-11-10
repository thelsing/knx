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
/*!*****************************************************************************
 *  @file       PWMTimerCC26XX.c
 *  @brief      CC26XX/CC13XX implementation of ti/drivers/PWM.h
 *
 *  # Overview #
 *  CC26XX/CC13XX PWM driver using the built-in GPTimer.
 *
 *  # Note #
 *  The driver requires the GPTimer bit GPT.TnMR.TnPLO to be set.
 *  Using this, the PWM output will be always low when load=match and always
 *  high when match>load. Setting match > load is used for 100% duty cycle
 *
 *******************************************************************************
 */

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/ClockP.h>

#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>

#include "ti/drivers/PWM.h"
#include "ti/drivers/pwm/PWMTimerCC26XX.h"
#include "ti/drivers/timer/GPTimerCC26XX.h"

/* PWMTimerCC26XX defines */
#define PWM_COUNT_MAX    0xFFFFFE /* GPTimer has maximum 24 bits incl prescaler.
                                     Max count is set to (2^24 - 2) to allow for
                                     a glitch free 100% duty cycle at max period count.*/

/*!
 *  @brief If the PWM period is lower than this value, setDutyAndPeriod
 *  will briefly disable the PWM channel to set the new values.
 *
 *  This is to prevent the case where the period, but not the duty, is
 *  applied before the timeout and the next cycle is in an undetermined state.
 */
#define PWM_PERIOD_FOR_GLITCH_PROTECTION 0xF

/* PWMTimerCC26XX functions */
void         PWMTimerCC26XX_close(PWM_Handle handle);
int_fast16_t PWMTimerCC26XX_control(PWM_Handle handle, uint_fast16_t cmd,
        void * arg);
void         PWMTimerCC26XX_init(PWM_Handle handle);
PWM_Handle   PWMTimerCC26XX_open(PWM_Handle handle, PWM_Params *params);
int_fast16_t PWMTimerCC26XX_setDuty(PWM_Handle handle, uint32_t dutyValue);
int_fast16_t PWMTimerCC26XX_setPeriod(PWM_Handle handle, uint32_t periodValue);
int_fast16_t PWMTimerCC26XX_setDutyAndPeriod(PWM_Handle handle, uint32_t dutyValue, uint32_t periodValue);
void         PWMTimerCC26XX_start(PWM_Handle handle);
void         PWMTimerCC26XX_stop(PWM_Handle handle);

/* PWMTimerCC26XX internal functions */
static uint32_t         PWMTimerCC26XX_getperiodCounts(PWM_Period_Units periodUnit, uint32_t periodValue);
static int32_t         PWMTimerCC26XX_getdutyCounts(uint32_t periodCounts, PWM_Duty_Units dutyUnit, uint32_t dutyValue);

/* PWM function table for PWMTimerCC26XX implementation */
const PWM_FxnTable PWMTimerCC26XX_fxnTable =
{
    PWMTimerCC26XX_close,
    PWMTimerCC26XX_control,
    PWMTimerCC26XX_init,
    PWMTimerCC26XX_open,
    PWMTimerCC26XX_setDuty,
    PWMTimerCC26XX_setPeriod,
    PWMTimerCC26XX_setDutyAndPeriod,
    PWMTimerCC26XX_start,
    PWMTimerCC26XX_stop,
};


/* PIN configuration used for PWMTimerCC26XX implementation. Handle is shared
   across all PWM peripherals
 */
static PIN_Handle hPins = NULL;
static PIN_State  pinState;

/*!
 *  @brief PWM CC26XX initialization
 *
 *  This is a dummy function since driver implementation assumes
 *  the handle->object->isOpen flag is set to 0 at boot
 *
 *  @pre    Calling context: Hwi, Swi, Task, Main
 *
 *  @param handle  A SPI_Handle
 *
 */
void PWMTimerCC26XX_init(PWM_Handle handle)
{
}


/* Open the specific PWM peripheral with the settings given in params.
   Will return a PWM handle if successfull, NULL if failed.
   PWM will output configured idle level when opened.
   Function sets a dependency on the underlying timer and adds the PWM pin to
   its internal PIN handle.
 */
PWM_Handle PWMTimerCC26XX_open(PWM_Handle handle, PWM_Params *params)
{
    PWMTimerCC26XX_HwAttrs const *hwAttrs = handle->hwAttrs;
    PWMTimerCC26XX_Object        *object  = handle->object;

    /* Check if PWM already open  */
    uint32_t key = HwiP_disable();
    if (object->isOpen)
    {
        HwiP_restore(key);
        DebugP_log1("PWM_open(%x):  Unit already in use.", (uintptr_t) handle);
        return NULL;
    }
    object->isOpen = 1;
    HwiP_restore(key);

    /* Open timer resource */
    GPTimerCC26XX_Params timerParams;
    GPTimerCC26XX_Params_init(&timerParams);
    timerParams.width           = GPT_CONFIG_16BIT;
    timerParams.mode            = GPT_MODE_PWM;
    timerParams.debugStallMode  = GPTimerCC26XX_DEBUG_STALL_OFF;
    timerParams.matchTiming     = GPTimerCC26XX_SET_MATCH_ON_TIMEOUT;
    GPTimerCC26XX_Handle hTimer = GPTimerCC26XX_open(hwAttrs->gpTimerUnit, &timerParams);

    /* Fail if cannot open timer */
    if (hTimer == NULL)
    {
        DebugP_log2("PWM_open(%x): Timer unit (%d) already in use.", (uintptr_t) handle, hwAttrs->gpTimerUnit);
        object->isOpen = false;
        return NULL;
    }

    /* Open pin resource */
    PIN_Config pinConfig;

    /* Initial open of pin handle with no pins in it. Should never fail. */
    if (hPins == NULL)
    {
        pinConfig = PIN_TERMINATE;
        hPins     = PIN_open(&pinState, &pinConfig);
    }

    uint32_t idleLevel = PIN_GPIO_HIGH;
    if (params->idleLevel == PWM_IDLE_LOW)
    {
        idleLevel = PIN_GPIO_LOW;
    }

    /* Generate pin config for PWM pin. */
    pinConfig = hwAttrs->pwmPin | PIN_INPUT_DIS | PIN_GPIO_OUTPUT_EN | idleLevel |
                PIN_PUSHPULL | PIN_DRVSTR_MAX;

    /* Fail if cannot add pin */
    if (PIN_add(hPins, pinConfig) != PIN_SUCCESS)
    {
        DebugP_log2("PWM_open(%x): PIN (%d) already in use.", (uintptr_t) handle, hwAttrs->pwmPin);
        GPTimerCC26XX_close(hTimer);
        object->isOpen = false;
        return NULL;
    }

    /* Store configuration to object */
    object->periodUnit  = params->periodUnits;
    object->periodValue = params->periodValue;
    object->dutyUnit    = params->dutyUnits;
    object->dutyValue   = params->dutyValue;
    object->idleLevel   = params->idleLevel;
    object->hTimer      = hTimer;

    /* Configure PWM period*/
    uint32_t period = object->periodValue;

    /* This will also set the duty cycle */
    if (PWMTimerCC26XX_setPeriod(handle, period) != PWM_STATUS_SUCCESS)
    {
        DebugP_log1("PWM_open(%x): Failed setting period", (uintptr_t) handle);
        PIN_remove(hPins, hwAttrs->pwmPin);
        GPTimerCC26XX_close(hTimer);
        object->isOpen = false;
        return NULL;
    }

    DebugP_log1("PWM_open(%x): Opened with great success!", (uintptr_t) handle);
    return handle;
}


/* PWMTimerCC26XX_setPeriod -
   Sets / update PWM period. Unit must already be defined in object.
   Also updates duty cycle.
 */
int_fast16_t PWMTimerCC26XX_setPeriod(PWM_Handle handle, uint32_t periodValue)
{
    PWMTimerCC26XX_Object *object = handle->object;
    /* Copy current duty value and store new period */
    uint32_t dutyValue       = object->dutyValue;
    uint32_t newperiodCounts = PWMTimerCC26XX_getperiodCounts(object->periodUnit, periodValue);
    int32_t  newdutyCounts   = PWMTimerCC26XX_getdutyCounts(newperiodCounts, object->dutyUnit, dutyValue);

    /* Fail if period is out of range */
    if ((newperiodCounts > PWM_COUNT_MAX) ||
        (newperiodCounts == 0))
    {
        DebugP_log2("PWM(%x): Period (%d) is out of range", (uintptr_t) handle, periodValue);
        return PWM_STATUS_INVALID_PERIOD;
    }

    /* Compare to new period and fail if invalid */
    if (newperiodCounts < (newdutyCounts - 1) || (newdutyCounts < 0))
    {
        DebugP_log2("PWM(%x): Period is shorter than duty (%d)", (uintptr_t) handle, periodValue);
        return PWM_STATUS_INVALID_PERIOD;
    }

    /* Store new period and update timer */
    object->periodValue  = periodValue;
    object->periodCounts = newperiodCounts;
    GPTimerCC26XX_setLoadValue(object->hTimer, newperiodCounts);

    /* Store new duty cycle and update timer */
    object->dutyValue  = dutyValue;
    object->dutyCounts = newdutyCounts;

    GPTimerCC26XX_setMatchValue(object->hTimer, newdutyCounts);

    DebugP_log1("PWM_setPeriod(%x): Period set with great success!", (uintptr_t) handle);
    return PWM_STATUS_SUCCESS;
}

/* PWMTimerCC26XX_setDuty -
   Sets / update PWM duty. Unit must already be defined in object.
   Period must already be configured in object before calling this API.
 */
int_fast16_t PWMTimerCC26XX_setDuty(PWM_Handle handle, uint32_t dutyValue)
{
    PWMTimerCC26XX_Object *object = handle->object;
    /* Copy current duty unit and store new period */
    PWM_Duty_Units dutyUnit = object->dutyUnit;
    int32_t newdutyCounts   = PWMTimerCC26XX_getdutyCounts(object->periodCounts, dutyUnit, dutyValue);

    /* Fail if duty cycle count is out of range. */
    if (newdutyCounts > PWM_COUNT_MAX)
    {
        DebugP_log2("PWM(%x): Duty (%d) is out of range", (uintptr_t) handle, dutyValue);
        return PWM_STATUS_INVALID_DUTY;
    }

    /* Error checking:
     *  Unit PWM_DUTY_FRACTION will always be within range
     *  Unit PWM_DUTY_US with value 0 will always be correct(set by getdutyCounts)
     *  Unit PWM_DUTY_US value != 0 needs error checking
     *  Unit PWM_DUTY_COUNTS needs error checking
     */
    if (((newdutyCounts > (object->periodCounts + 1)) &&
        ((dutyUnit == PWM_DUTY_US) || (dutyUnit == PWM_DUTY_COUNTS))) ||
        (newdutyCounts < 0))
    {
        DebugP_log2("PWM(%x): Duty (%d) is larger than period", (uintptr_t) handle, dutyValue);
        return PWM_STATUS_INVALID_DUTY;
    }

    /* Store new duty cycle and update timer */
    object->dutyValue  = dutyValue;
    object->dutyCounts = newdutyCounts;

    GPTimerCC26XX_setMatchValue(object->hTimer, newdutyCounts);

    DebugP_log1("PWM_setDuty(%x): Duty set with great success!", (uintptr_t) handle);
    return PWM_STATUS_SUCCESS;
}

/* ======== PWMTimerCC26XX_setDutyAndPeriod ========
   Sets / update PWM duty and period. Unit must already be defined in object.
 */
int_fast16_t PWMTimerCC26XX_setDutyAndPeriod (PWM_Handle handle, uint32_t dutyValue, uint32_t periodValue)
{
    uint32_t key;
    bool stopped = false;
    PWMTimerCC26XX_Object *object = handle->object;

    uint32_t oldPeriod = object->periodValue;
    uint32_t newperiodCounts = PWMTimerCC26XX_getperiodCounts(object->periodUnit, periodValue);
    int32_t  newdutyCounts   = PWMTimerCC26XX_getdutyCounts(newperiodCounts, object->dutyUnit, dutyValue);

    /* Fail if period is out of range or incompatible with new duty */
    if ((newperiodCounts > PWM_COUNT_MAX) || (newperiodCounts == 0)
        || (newperiodCounts < (newdutyCounts - 1))) {
        return PWM_STATUS_INVALID_PERIOD;
    }

    /* Fail if duty cycle count is out of range. */
    if ((newdutyCounts > PWM_COUNT_MAX) || (newdutyCounts < 0)) {
        return PWM_STATUS_INVALID_DUTY;
    }

    /* Store new period */
    object->periodValue  = periodValue;
    object->periodCounts = newperiodCounts;

    /* Store new duty cycle */
    object->dutyValue  = dutyValue;
    object->dutyCounts = newdutyCounts;

    // Disable interrupts for register update
    key = HwiP_disable();

    if (object->isRunning && (oldPeriod <= PWM_PERIOD_FOR_GLITCH_PROTECTION || GPTimerCC26XX_getValue(object->hTimer) <= PWM_PERIOD_FOR_GLITCH_PROTECTION)) {
        stopped = true;
        GPTimerCC26XX_stop(object->hTimer);
    }

    /* Update timer */
    GPTimerCC26XX_setLoadValue(object->hTimer, newperiodCounts);
    GPTimerCC26XX_setMatchValue(object->hTimer, newdutyCounts);

    if (stopped) {
        GPTimerCC26XX_start(object->hTimer);
    }

    // Restore interrupts
    HwiP_restore(key);
    return PWM_STATUS_SUCCESS;
}

/* Return period in timer counts */
static uint32_t PWMTimerCC26XX_getperiodCounts(PWM_Period_Units periodUnit, uint32_t periodValue)
{
    ClockP_FreqHz freq;
    ClockP_getCpuFreq(&freq);

    uint32_t periodCounts;

    switch (periodUnit)
    {
    case PWM_PERIOD_US:
        periodCounts = ((uint64_t)freq.lo * (uint64_t)periodValue / 1000000) - 1;
        break;
    case PWM_PERIOD_HZ:
        periodCounts = (freq.lo / periodValue) - 1;
        break;
    case PWM_PERIOD_COUNTS:
    /* Fall through */
    default:
        periodCounts = periodValue;
        break;
    }
    return periodCounts;
}

/* Return duty cycle in timer counts */
static int32_t PWMTimerCC26XX_getdutyCounts(uint32_t periodCounts, PWM_Duty_Units dutyUnit, uint32_t dutyValue)
{
    ClockP_FreqHz freq;
    ClockP_getCpuFreq(&freq);

    uint32_t dutyCounts;

    /* Corner case, 0% duty cycle. Set timer count to period count */
    if (dutyValue == 0)
    {
        dutyCounts = periodCounts;
    }
    else
    {
        /* Invert the duty cycle count to get the expected PWM signal output. */
        switch (dutyUnit)
        {
        case PWM_DUTY_US:
            dutyCounts = periodCounts - (((uint64_t)freq.lo * (uint64_t)dutyValue / 1000000) - 1);
            break;
        case PWM_DUTY_FRACTION:
            dutyCounts = periodCounts - ((uint64_t)dutyValue * (uint64_t)periodCounts /
                         PWM_DUTY_FRACTION_MAX);
            break;
        case PWM_DUTY_COUNTS:
        /* Fall through */
        default:
            dutyCounts = periodCounts - dutyValue;
            break;
        }

        /* Corner case: If 100% duty cycle, the resulting dutyCount will be 0, set new dutyCounts to periodCounts + 1 to create a glitch free signal. */
        if (dutyCounts == 0)
        {
            dutyCounts = periodCounts + 1;
        }
    }
    return dutyCounts;
}

/* Stop PWM output for given PWM peripheral. PWM pin will be routed
   to the GPIO module to provide Idle level and timer is stopped
 */
void PWMTimerCC26XX_stop(PWM_Handle handle)
{
    PWMTimerCC26XX_HwAttrs const *hwAttrs = handle->hwAttrs;
    PWMTimerCC26XX_Object        *object  = handle->object;

    uint32_t key = HwiP_disable();
    object->isRunning = 0;
    HwiP_restore(key);

    GPTimerCC26XX_stop(object->hTimer);
    /* Route PWM pin to GPIO module */
    PINCC26XX_setMux(hPins, hwAttrs->pwmPin, IOC_PORT_GPIO);
}

/* Start PWM output for given PWM peripheral.
   PWM pin will be routed to the Timer module and timer is started
 */
void PWMTimerCC26XX_start(PWM_Handle handle)
{
    PWMTimerCC26XX_HwAttrs const *hwAttrs = handle->hwAttrs;
    PWMTimerCC26XX_Object        *object  = handle->object;

    uint32_t key = HwiP_disable();
    object->isRunning = 1;
    HwiP_restore(key);

    /* Route PWM pin to timer output */
    GPTimerCC26XX_PinMux pinMux = GPTimerCC26XX_getPinMux(object->hTimer);
    PINCC26XX_setMux(hPins, hwAttrs->pwmPin, pinMux);
    GPTimerCC26XX_start(object->hTimer);
}

/* Close the specific PWM peripheral. A running PWM must be stopped first.
   PWM output will revert to output value given in PIN_init if any is defined.
 */
void PWMTimerCC26XX_close(PWM_Handle handle)
{
    PWMTimerCC26XX_HwAttrs const *hwAttrs = handle->hwAttrs;
    PWMTimerCC26XX_Object        *object  = handle->object;

    /* Restore PWM pin to GPIO module with default configuration from PIN_init */
    PIN_remove(hPins, hwAttrs->pwmPin);
    /* Close and delete timer handle */
    GPTimerCC26XX_close(object->hTimer);
    object->hTimer = NULL;

    /* Clear isOpen flag */
    uint32_t key = HwiP_disable();
    object->isOpen = 0;
    HwiP_restore(key);
}

/* Driver specific control options. PWM peripheral must be opened before
   using this API */
int_fast16_t PWMTimerCC26XX_control(PWM_Handle handle, uint_fast16_t cmd,
        void * arg)
{
    PWMTimerCC26XX_Object *object = handle->object;
    int stat = PWM_STATUS_SUCCESS;

    switch (cmd)
    {
    case PWMTimerCC26XX_CMD_DEBUG_STALL:
        GPTimerCC26XX_configureDebugStall(object->hTimer, *(GPTimerCC26XX_DebugMode *)arg);
        break;
    default:
        stat = PWM_STATUS_UNDEFINEDCMD;
        break;
    }
    return stat;
}
