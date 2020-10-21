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
 * EXEMPLARY, OR CONSEQueueNTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** ============================================================================
 *  @file       PowerCC26X2.h
 *
 *  @brief      Power manager interface for CC26X2
 *
 *  The Power header file should be included in an application as follows:
 *  @code
 *  #include <ti/drivers/Power.h>
 *  #include <ti/drivers/power/PowerCC26X2.h>
 *  @endcode
 *
 *  Refer to @ref Power.h for a complete description of APIs.
 *
 *  ## Implementation #
 *  This header file defines the power resources, constraints, events, sleep
 *  states and transition latencies for CC26X2.
 *
 *  ============================================================================
 */

#ifndef ti_drivers_power_PowerCC26X2_
#define ti_drivers_power_PowerCC26X2_

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

/*! The latency to reserve for resume from STANDBY (usec). */
#define PowerCC26X2_RESUMETIMESTANDBY  750

/*! The total latency to reserve for entry to and exit from STANDBY (usec). */
#define PowerCC26X2_TOTALTIMESTANDBY   1000

/*! The initial delay when waking from STANDBY (usec). */
#define PowerCC26X2_WAKEDELAYSTANDBY    240

/*! The initial wait time (usec) before checking if RCOSC_LF is stable. */
#define PowerCC26X2_INITIALWAITRCOSC_LF 1000

/*! The retry wait time (usec) when checking to see if RCOSC_LF is stable. */
#define PowerCC26X2_RETRYWAITRCOSC_LF   1000

/*! The initial wait time (usec) before checking if XOSC_HF is stable. */
#define PowerCC26X2_INITIALWAITXOSC_HF  50

/*! The retry wait time (usec) when checking to see if XOSC_HF is stable. */
#define PowerCC26X2_RETRYWAITXOSC_HF    50

/*! The initial wait time (usec) before checking if XOSC_LF is stable. */
#define PowerCC26X2_INITIALWAITXOSC_LF  10000

/*! The retry wait time (usec) when checking to see if XOSC_LF is stable. */
#define PowerCC26X2_RETRYWAITXOSC_LF    5000

#define PowerCC26X2_PERIPH_PKA          PowerCC26XX_NUMRESOURCES /*!< Resource ID: PKA Module */

#define PowerCC26X2_PERIPH_UART1        PowerCC26XX_NUMRESOURCES + 1 /*!< Resource ID: UART1 */

/* \cond */
#define PowerCC26X2_NUMRESOURCES   (PowerCC26XX_NUMRESOURCES + 2) /* Number of resources in database */
/* \endcond */

/* \cond */
#define PowerCC26X2_NUMCONSTRAINTS (PowerCC26XX_NUMCONSTRAINTS + 0) /* Number of constraints supported */
/* \endcond */

/* \cond */
/*
 *  Calibration stages
 */
#define PowerCC26X2_SETUP_CALIBRATE     1
#define PowerCC26X2_INITIATE_CALIBRATE  2
#define PowerCC26X2_DO_CALIBRATE        3
/* \endcond */


/*! @brief Global configuration structure */
typedef struct PowerCC26X2_Config {
    /*!
     *  @brief The Power Policy's initialization function
     *
     *  If the policy does not have an initialization function, 'NULL'
     *  should be specified.
     */
    Power_PolicyInitFxn policyInitFxn;
    /*!
     *  @brief The Power Policy function
     *
     *  When enabled, this function is invoked in the idle loop, to
     *  opportunistically select and activate sleep states.
     *
     *  Two reference policies are provided:
     *
     *    PowerCC26X2_doWFI() - a simple policy that invokes CPU wait for
     *    interrupt (WFI)
     *
     *    PowerCC26X2_standbyPolicy() - an agressive policy that considers
     *    constraints, time until next scheduled work, and sleep state
     *    latencies, and optionally puts the device into the STANDBY state,
     *    the IDLE state, or as a minimum, WFI.
     *
     *  Custom policies can be written, and specified via this function pointer.
     *
     *  In addition to this static selection, the Power Policy can be
     *  dynamically changed at runtime, via the Power_setPolicy() API.
     */
    Power_PolicyFxn policyFxn;
    /*!
     *  @brief The function to be used for activating RC Oscillator (RCOSC)
     *  calibration
     *
     *  Calibration is normally enabled, via specification of the function
     *  PowerCC26X2_calibrate().  This enables high accuracy operation, and
     *  faster high frequency crystal oscillator (XOSC_HF) startups.
     *
     *  To disable RCOSC calibration, the function PowerCC26X2_noCalibrate()
     *  should be specified.
     */
    bool (*calibrateFxn)(unsigned int);
    /*!
     *  @brief Boolean specifying if the Power Policy function is enabled
     *
     *  If 'true', the policy function will be invoked once for each pass
     *  of the idle loop.
     *
     *  If 'false', the policy will not be invoked.
     *
     *  In addition to this static setting, the power policy can be dynamically
     *  enabled and disabled at runtime, via the Power_enablePolicy() and
     *  Power_disablePolicy() functions, respectively.
     */
    bool enablePolicy;
    /*!
     *  @brief Boolean specifying whether the low frequency RC oscillator
     * (RCOSC_LF) should be calibrated.
     *
     *  If RCOSC calibration is enabled (above, via specification of
     *  an appropriate calibrateFxn), this Boolean specifies whether
     *  RCOSC_LF should be calibrated.
     */
    bool calibrateRCOSC_LF;
    /*!
     *  @brief Boolean specifying whether the high frequency RC oscillator
     * (RCOSC_HF) should be calibrated.
     *
     *  If RCOSC calibration is enabled (above, via specification of
     *  an appropriate calibrateFxn), this Boolean specifies whether
     *  RCOSC_HF should be calibrated.
     */
    bool calibrateRCOSC_HF;
} PowerCC26X2_Config;

/*!
 *  @brief  PowerCC26X2_ModuleState
 *
 *  Power manager state structure. The application must not access any members
 *  of this structure!
 */
typedef struct PowerCC26X2_ModuleState {
    List_List notifyList;           /*!< Event notification list */
    uint32_t constraintMask;        /*!< Aggregate constraints mask */
    ClockP_Struct clockObj;         /*!< Clock object for scheduling wakeups */
    ClockP_Struct calibrationClock; /*!< Clock object for scheduling wakeups */
    HwiP_Struct oscHwi;             /*!< Hwi object for oscillator stabilisation */
    HwiP_Struct tdcHwi;             /*!< Hwi object for RCOSC calibration */
    int32_t nDeltaFreqCurr;         /*!< RCOSC calibration variable */
    int32_t nCtrimCurr;             /*!< RCOSC calibration variable */
    int32_t nCtrimFractCurr;        /*!< RCOSC calibration variable */
    int32_t nCtrimNew;              /*!< RCOSC calibration variable */
    int32_t nCtrimFractNew;         /*!< RCOSC calibration variable */
    int32_t nRtrimNew;              /*!< RCOSC calibration variable */
    int32_t nRtrimCurr;             /*!< RCOSC calibration variable */
    int32_t nDeltaFreqNew;          /*!< RCOSC calibration variable */
    bool bRefine;                   /*!< RCOSC calibration variable */
    uint32_t state;                 /*!< Current transition state */
    bool xoscPending;               /*!< Is XOSC_HF activation in progress? */
    bool calLF;                     /*!< Calibrate RCOSC_LF? */
    uint8_t auxHwiState;            /*!< The AUX ISR calibration state */
    bool busyCal;                   /*!< Already busy calibrating? */
    uint32_t calStep;               /*!< The current calibration step */
    bool firstLF;                   /*!< Is this the first LF calibration? */
    bool enablePolicy;              /*!< Is the Power policy enabled? */
    bool initialized;               /*!< Has Power_init() been called? */
    uint8_t constraintCounts[PowerCC26X2_NUMCONSTRAINTS];
    /*!< Array to maintain constraint reference counts */
    uint8_t resourceCounts[PowerCC26X2_NUMRESOURCES];
    /*!< Array to maintain resource dependency reference counts */
    unsigned int (*resourceHandlers[3])(unsigned int);
    /*!< Array of special dependency handler functions */
    Power_PolicyFxn policyFxn;   /*!< The Power policy function */
} PowerCC26X2_ModuleState;


#ifdef __cplusplus
}
#endif

#endif /* POWER_CC26X2_ */
