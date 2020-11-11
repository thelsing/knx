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
 * EXEMPLARY, OR CONSEQueueNTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ======== PowerCC26XX_calibrateRCOSC.c ========
 */

#include <stdbool.h>

#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/HwiP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_aux_evctl.h)
#include DeviceFamily_constructPath(inc/hw_aux_smph.h)
#include DeviceFamily_constructPath(inc/hw_aux_wuc.h)
#include DeviceFamily_constructPath(inc/hw_aux_tdc.h)
#include DeviceFamily_constructPath(inc/hw_ddi_0_osc.h)
#include DeviceFamily_constructPath(inc/hw_ddi.h)
#include DeviceFamily_constructPath(inc/hw_ccfg.h)
#include DeviceFamily_constructPath(driverlib/aon_batmon.h)
#include DeviceFamily_constructPath(driverlib/ddi.h)
#include DeviceFamily_constructPath(driverlib/ioc.h)
#include DeviceFamily_constructPath(driverlib/osc.h)
#include DeviceFamily_constructPath(driverlib/gpio.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/aux_wuc.h)

#define AUX_TDC_SEMAPHORE_NUMBER                     1     /* semaphore 1 protects TDC */
#define NUM_RCOSC_LF_PERIODS_TO_MEASURE              32    /* x RCOSC_LF periods vs XOSC_HF */
#define NUM_RCOSC_HF_PERIODS_TO_MEASURE              1     /* x RCOSC_HF periods vs XOSC_HF */
#define ACLK_REF_SRC_RCOSC_HF                        0     /* Use RCOSC_HF for ACLK REF */
#define ACLK_REF_SRC_RCOSC_LF                        2     /* Use RCOSC_LF for ACLK REF */
#define SCLK_LF_OPTION_RCOSC_LF                      3     /* defined in cc26_ccfg.xls */
#define RCOSC_HF_LOW_THRESHOLD_TDC_VALUE             1535  /* If TDC value is within threshold range, no need for another TDC measurement */
#define RCOSC_HF_PERFECT_TDC_VALUE                   1536  /* RCOSC_HF runs at perfect 48 MHz when ending up with this TDC value */
#define RCOSC_HF_HIGH_THRESHOLD_TDC_VALUE            1537  /* If TDC value is within threshold range, no need for another TDC measurement */

#define DDI_0_OSC_O_CTL1_LOCAL                       0x00000004             /* offset */
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_M     0x007C0000             /* mask */
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_S     18                     /* shift */
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN_LOCAL_M  0x00020000             /* mask */
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN_LOCAL_S  17                     /* shift */
#define DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_M 0x00000C00    /* offset */
#define DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_S 10            /* shift */

/* AUX ISR states */
#define WAIT_SMPH       0   /* just took SMPH, start RCOSC_LF */
#define CAL_RCOSC_LF    1   /* just finished RCOSC_LF, start first RCOSC_HF */
#define CAL_RCOSC_HF1   2   /* just finished 1st RCOSC_HF, start 2nd */
#define CAL_RCOSC_HF2   3   /* just finished 2nd RCOSC_HF, decide best */

/* calibration steps */
#define STEP_TDC_INIT_1    1
#define STEP_TDC_INIT_2    2
#define STEP_CAL_LF_1      3
#define STEP_CAL_LF_2      4
#define STEP_CAL_LF_3      5
#define STEP_CAL_HF1_1     6
#define STEP_CAL_HF1_2     7
#define STEP_CAL_HF1_3     8
#define STEP_CAL_HF2_1     9
#define STEP_CAL_HF2_2     10
#define STEP_CLEANUP_1     11
#define STEP_CLEANUP_2     12

/* macros */
#define Min(a,b)        (((a)<(b))?(a):(b))
#define Max(a,b)        (((a)>(b))?(a):(b))
#define Abs(x)          ((x) < 0 ? -(x) : (x))
#define Scale_rndInf(x)  ((3 * (x) + (((x) < 0) ? -2 : 2)) / 4)

#define INSTRUMENT 0

#if INSTRUMENT
volatile unsigned int gotSEM = 0;
volatile unsigned int calLFi = 0;
volatile unsigned int calHF1i = 0;
volatile unsigned int calHF2i = 0;
volatile bool doneCal = false;
unsigned int tdcResult_LF = 0;
unsigned int tdcResult_HF1 = 0;
unsigned int tdcResult_HF2 = 0;
unsigned int numISRs = 0;
unsigned int calClocks = 0;
#endif

/* Forward declarations */
static bool getTdcSemaphore();
static void updateSubSecInc(uint32_t tdcResult);
static void calibrateRcoscHf1(int32_t tdcResult);
static void calibrateRcoscHf2(int32_t tdcResult);
void PowerCC26XX_doCalibrate(void);

/* Externs */
extern PowerCC26XX_ModuleState PowerCC26XX_module;
extern const PowerCC26XX_Config PowerCC26XX_config;

/*
 *  ======== PowerCC26XX_initiateCalibration ========
 *  Initiate calibration of RCOSC_LF and RCOSCHF
 */
bool PowerCC26XX_initiateCalibration()
{
    unsigned int hwiKey;
    bool busy = false;
    bool status;
    bool gotSem;

    if ((PowerCC26XX_module.calLF == false) &&
        (PowerCC26XX_config.calibrateRCOSC_HF == false)) {
        return (false);
    }

    /* make sure calibration is not already in progress */
    hwiKey = HwiP_disable();

    if (PowerCC26XX_module.busyCal == false) {
        PowerCC26XX_module.busyCal = true;
    }
    else {
        busy = true;
    }

    HwiP_restore(hwiKey);

    if (busy == true) {
        return (false);
    }

#if INSTRUMENT
    gotSEM = 0;
    calLFi = 0;
    calHF1i = 0;
    calHF2i = 0;
    doneCal = false;
#endif

    /* set contraint to prohibit standby during calibration sequence */
    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* set dependency to keep XOSC_HF active during calibration sequence */
    Power_setDependency(PowerCC26XX_XOSC_HF);

    /* initiate acquisition of semaphore protecting TDC */
    gotSem = getTdcSemaphore();

    /* if didn't acquire semaphore, must wait for autotake ISR */
    if (gotSem == false) {
        PowerCC26XX_module.hwiState = WAIT_SMPH;
        status = false;  /* false: don't do anything else until acquire SMPH */
    }

    /* else, semaphore acquired, OK to proceed with first measurement */
    else {
#if INSTRUMENT
        gotSEM = 1;
#endif
        status = true;   /* true: OK to start first measurement */
    }

    return (status);
}

/*
 *  ======== PowerCC26XX_auxISR ========
 *  ISR for the AUX combo interrupt event.  Implements Hwi state machine to
 *  step through the RCOSC calibration steps.
 */
void PowerCC26XX_auxISR(uintptr_t arg)
{
    uint32_t tdcResult;

#if INSTRUMENT
    numISRs++;
#endif

    /*
    * disable all events that are part of AUX_COMBINED_INTERRUPT.
    * This interrupt is reserved for use during RCOSC calibration.
    * Other AUX perihperals that want to generate interrupts to CM3
    * must use dedicated interrupt lines or go through AON combined.
    */
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_COMBEVTOMCUMASK) = 0;

    /* ****** state = WAIT_SMPH: arrive here if just took the SMPH ****** */
    if (PowerCC26XX_module.hwiState == WAIT_SMPH) {
#if INSTRUMENT
        gotSEM = 1;
#endif
    }

    /* **** state = CAL_RCOSC_LF: here when just finished LF counting **** */
    else if (PowerCC26XX_module.hwiState == CAL_RCOSC_LF) {

        tdcResult = HWREG(AUX_TDC_BASE + AUX_TDC_O_RESULT);

#if INSTRUMENT
        tdcResult_LF = tdcResult;
#endif
        /* update the RTC SUBSECINC register based on LF measurement result */
        updateSubSecInc(tdcResult);
#if INSTRUMENT
        calLFi = 1;
#endif
        /* if doing HF calibration initiate it now */
        if (PowerCC26XX_config.calibrateRCOSC_HF) {
            PowerCC26XX_module.calStep = STEP_CAL_LF_3;  /* next: trigger LF */
        }

        /* else, start cleanup */
        else {
            PowerCC26XX_module.calStep = STEP_CLEANUP_1; /* next: cleanup */
        }
    }

    /* ****** state = CAL_RCOSC_HF1: here when just finished 1st RCOSC_HF */
    else if (PowerCC26XX_module.hwiState == CAL_RCOSC_HF1) {

        tdcResult = HWREG(AUX_TDC_BASE + AUX_TDC_O_RESULT);

#if INSTRUMENT
        tdcResult_HF1 = tdcResult;
        calHF1i = 1;
#endif

        /* use first HF measurement to setup new trim values */
        calibrateRcoscHf1(tdcResult);

        /* if HF setting perfect, nothing more to do, calibration is done */
        if ((tdcResult >= RCOSC_HF_LOW_THRESHOLD_TDC_VALUE) &&
            (tdcResult <= RCOSC_HF_HIGH_THRESHOLD_TDC_VALUE)) {
            PowerCC26XX_module.calStep = STEP_CLEANUP_1;  /* next: cleanup */
        }

        /* else, tweak trims, initiate another HF measurement */
        else {

            PowerCC26XX_module.calStep = STEP_CAL_HF1_3;  /* next: HF meas. #2 */
        }
    }

    /* ****** state = just finished second RCOSC_HF measurement ****** */
    else if (PowerCC26XX_module.hwiState == CAL_RCOSC_HF2) {

        tdcResult = HWREG(AUX_TDC_BASE + AUX_TDC_O_RESULT);

#if INSTRUMENT
        tdcResult_HF2 = tdcResult;
#endif
        /* look for improvement on #2, else revert to previous trim values */
        calibrateRcoscHf2(tdcResult);

        PowerCC26XX_module.calStep = STEP_CLEANUP_1;    /* next: cleanup */
    }

    /* do the next calibration step... */
    PowerCC26XX_doCalibrate();
}

/*
 *  ======== PowerCC26XX_doCalibrate ========
 */
void PowerCC26XX_doCalibrate(void)
{
    switch (PowerCC26XX_module.calStep) {

        case STEP_TDC_INIT_1:

            /* turn on clock to TDC module */
            AUXWUCClockEnable(AUX_WUC_TDCIF_CLOCK);

            /* set saturation config to 2^24 */
            HWREG(AUX_TDC_BASE + AUX_TDC_O_SATCFG) =
                AUX_TDC_SATCFG_LIMIT_R24;

            /* set start and stop trigger sources and polarity */
            HWREG(AUX_TDC_BASE + AUX_TDC_O_TRIGSRC) =
                (AUX_TDC_TRIGSRC_STOP_SRC_ACLK_REF |
                 AUX_TDC_TRIGSRC_STOP_POL_HIGH) |
                (AUX_TDC_TRIGSRC_START_SRC_ACLK_REF |
                 AUX_TDC_TRIGSRC_START_POL_HIGH);

            /* set TDC_SRC clock to be XOSC_HF/2 = 24 MHz */
            DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL0,
                DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_M,
                DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_S, 2);

            /* read back to ensure no race condition between OSC_DIG and AUX_WUC */
            DDI16BitfieldRead(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL0,
                  DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_M, DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_S);

            /* set AUX_WUC:TDCCLKCTL.REQ... */
            HWREG(AUX_WUC_BASE + AUX_WUC_O_TDCCLKCTL) = AUX_WUC_TDCCLKCTL_REQ;

            /* set next state */
            PowerCC26XX_module.calStep = STEP_TDC_INIT_2;

            /* start Clock object to delay while wait for ACK */
            ClockP_start(ClockP_handle(&PowerCC26XX_module.calClockStruct));

            break;

        case STEP_TDC_INIT_2:

            /* Enable trig count */
            HWREG(AUX_TDC_BASE + AUX_TDC_O_TRIGCNTCFG) =
                AUX_TDC_TRIGCNTCFG_EN;

            /* if LF calibration enabled start LF measurement */
            if (PowerCC26XX_module.calLF) {

               /* clear UPD_REQ, new sub-second increment is NOT available */
                HWREG(AUX_WUC_BASE + AUX_WUC_O_RTCSUBSECINCCTL) = 0;

                /* set next Swi state */
                PowerCC26XX_module.calStep = STEP_CAL_LF_1;
            }

            /* else, start first HF measurement */
            else {
                /* set next Swi state */
                PowerCC26XX_module.calStep = STEP_CAL_HF1_1;
            }

            /* abort TDC */
            HWREG(AUX_TDC_BASE + AUX_TDC_O_CTL) = AUX_TDC_CTL_CMD_ABORT;

            /* clear AUX_WUC:REFCLKCTL.REQ... */
            HWREG(AUX_WUC_BASE + AUX_WUC_O_REFCLKCTL) = 0;

            /* if not ready, start Clock object to delay while wait for ACK */
            if (HWREG(AUX_WUC_BASE + AUX_WUC_O_REFCLKCTL) &
                AUX_WUC_REFCLKCTL_ACK) {

                /* start Clock object to delay while wait for ACK */
                ClockP_start(ClockP_handle(&PowerCC26XX_module.calClockStruct));

                break;
            }

            /* else, if ready now, fall thru to next step ... */


        case STEP_CAL_LF_1:
        case STEP_CAL_HF1_1:
        case STEP_CAL_HF2_1:

            if (PowerCC26XX_module.calStep == STEP_CAL_LF_1) {

                /* set the ACLK reference clock */
                DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL0,
                           DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_M,
                           DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_S,
                           ACLK_REF_SRC_RCOSC_LF);

                /* set next Swi state */
                PowerCC26XX_module.calStep = STEP_CAL_LF_2;
            }
            else {

                /* set the ACLK reference clock */
                DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL0,
                       DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_M,
                       DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_S,
                       ACLK_REF_SRC_RCOSC_HF);

                /* set next Swi state */
                if (PowerCC26XX_module.calStep == STEP_CAL_HF1_1) {
                    PowerCC26XX_module.calStep = STEP_CAL_HF1_2;
                }
                else {
                    PowerCC26XX_module.calStep = STEP_CAL_HF2_2;
                }
            }

            /* set AUX_WUC:REFCLKCTL.REQ */
            HWREG(AUX_WUC_BASE + AUX_WUC_O_REFCLKCTL) = AUX_WUC_REFCLKCTL_REQ;

            /* start Clock object to delay while wait for ACK */
            ClockP_start(ClockP_handle(&PowerCC26XX_module.calClockStruct));

            break;

        case STEP_CAL_LF_2:
        case STEP_CAL_HF1_2:
        case STEP_CAL_HF2_2:

            if (PowerCC26XX_module.calStep == STEP_CAL_LF_2) {

                /* Set number of periods of ACLK to count */
                HWREG(AUX_TDC_BASE + AUX_TDC_O_TRIGCNTLOAD) =
                    NUM_RCOSC_LF_PERIODS_TO_MEASURE;

                /* set next Hwi state before triggering TDC */
                PowerCC26XX_module.hwiState = CAL_RCOSC_LF;
            }
            else {

                /* Set number of periods of ACLK to count */
                HWREG(AUX_TDC_BASE + AUX_TDC_O_TRIGCNTLOAD) =
                    NUM_RCOSC_HF_PERIODS_TO_MEASURE;

                /* set next Hwi state before triggering TDC */
                if (PowerCC26XX_module.calStep == STEP_CAL_HF2_2) {
                    PowerCC26XX_module.hwiState = CAL_RCOSC_HF2;
                }
                else {
                    PowerCC26XX_module.hwiState = CAL_RCOSC_HF1;
                }
            }

            /* Reset/clear result of TDC */
            HWREG(AUX_TDC_BASE + AUX_TDC_O_CTL) = AUX_TDC_CTL_CMD_CLR_RESULT;

            /* Clear possible pending interrupt source */
            HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOMCUFLAGSCLR) =
                AUX_EVCTL_EVTOMCUFLAGSCLR_TDC_DONE;

            /* Enable TDC done interrupt as part of AUX_COMBINED interrupt */
            HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_COMBEVTOMCUMASK) =
                AUX_EVCTL_COMBEVTOMCUMASK_TDC_DONE;

            /* Run TDC (start synchronously) */
            HWREG(AUX_TDC_BASE + AUX_TDC_O_CTL) =
                AUX_TDC_CTL_CMD_RUN_SYNC_START;

            break;

        case STEP_CAL_LF_3:
        case STEP_CAL_HF1_3:

            /* set next Swi state */
            if (PowerCC26XX_module.calStep == STEP_CAL_LF_3) {
                PowerCC26XX_module.calStep = STEP_CAL_HF1_1;
            }
            else {
                PowerCC26XX_module.calStep = STEP_CAL_HF2_1;
            }

            /* clear AUX_WUC:REFCLKCTL.REQ... */
            HWREG(AUX_WUC_BASE + AUX_WUC_O_REFCLKCTL) = 0;

            /* start Clock object to delay while wait for ACK */
            ClockP_start(ClockP_handle(&PowerCC26XX_module.calClockStruct));

            break;

        case STEP_CLEANUP_1:

            /* release the TDC clock request */
            HWREG(AUX_WUC_BASE + AUX_WUC_O_TDCCLKCTL) = 0;

            /* release the TDC reference clock request */
            HWREG(AUX_WUC_BASE + AUX_WUC_O_REFCLKCTL) = 0;

            /* set next state */
            PowerCC26XX_module.calStep = STEP_CLEANUP_2;

            /* start Clock object to delay while wait for ACK */
            ClockP_start(ClockP_handle(&PowerCC26XX_module.calClockStruct));

            break;

        case STEP_CLEANUP_2:

            /*
            * Disable all interrupts as part of AUX_COMBINED interrupt
            * Once we release semaphore, the sensor controller is allowed
            * to use the TDC. When it does, we must ensure that this
            * does not cause any unexpected interrupts to the CM3.
            */
            HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_COMBEVTOMCUMASK) = 0;

            /* release AUX semaphore */
            HWREG(AUX_SMPH_BASE + AUX_SMPH_O_SMPH1) = 1;

            /* release the power down constraints and XOSC_HF dependency */
            Power_releaseDependency(PowerCC26XX_XOSC_HF);
            Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

            /* set next state */
            PowerCC26XX_module.calStep = STEP_TDC_INIT_1;

#if INSTRUMENT
            doneCal = true;
            calHF2i = 1;
#endif
            PowerCC26XX_module.busyCal = false;
            break;

        default:
            for (;;) {
            }
    }
}

/*
 *  ======== PowerCC26XX_RCOSC_clockFunc ========
 */
void PowerCC26XX_RCOSC_clockFunc(uintptr_t arg)
{
#if INSTRUMENT
    calClocks++;
#endif

    switch (PowerCC26XX_module.calStep) {

        case STEP_TDC_INIT_2:
            /* finish wait for AUX_WUC:TDCCLKCTL.ACK to be set ... */
            while(!(HWREG(AUX_WUC_BASE + AUX_WUC_O_TDCCLKCTL) &
                AUX_WUC_TDCCLKCTL_ACK));
            break;

        case STEP_CAL_LF_1:
        case STEP_CAL_HF1_1:
        case STEP_CAL_HF2_1:
            /* finish wait for AUX_WUC:REFCLKCTL.ACK to be cleared ... */
            while(HWREG(AUX_WUC_BASE + AUX_WUC_O_REFCLKCTL) &
                AUX_WUC_REFCLKCTL_ACK);
            break;

        case STEP_CAL_LF_2:
        case STEP_CAL_HF1_2:
        case STEP_CAL_HF2_2:
            /* finish wait for AUX_WUC:REFCLKCTL.ACK to be set ... */
            while(!(HWREG(AUX_WUC_BASE + AUX_WUC_O_REFCLKCTL) &
                AUX_WUC_REFCLKCTL_ACK));
            break;

        case STEP_CLEANUP_2:
            /* finish wait for AUX_WUC:TDCCLKCTL.ACK to be cleared ... */
            while ((HWREG(AUX_WUC_BASE + AUX_WUC_O_TDCCLKCTL) &
                AUX_WUC_TDCCLKCTL_ACK));
            /* finish wait for AUX_WUC:REFCLKCTL.ACK to be cleared ... */
            while(HWREG(AUX_WUC_BASE + AUX_WUC_O_REFCLKCTL) &
                AUX_WUC_REFCLKCTL_ACK);
            break;

        default:
            for (;;) {
            }
    }

    PowerCC26XX_doCalibrate();
}

/*
 *  ======== getTdcSemaphore ========
 *  Get TDC semaphore (number 1)
 */
static bool getTdcSemaphore()
{
    unsigned int own;

    /* try to acquire SMPH */
    own = HWREG(AUX_SMPH_BASE + AUX_SMPH_O_SMPH1);

    /* if acquired SMPH: done */
    if (own != 0) {
        return (true);
    }

    /* clear the interrupt source, can only be cleared when we don't have semaphore */
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOMCUFLAGSCLR) =
        AUX_EVCTL_EVTOMCUFLAGSCLR_SMPH_AUTOTAKE_DONE;

    /*
     * else, did not acquire the semaphore, enable SMPH_AUTOTAKE_DONE event
     * (don't OR, write entire register, no other interrupts can be enabled!)
     */
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_COMBEVTOMCUMASK) =
        AUX_EVCTL_COMBEVTOMCUMASK_SMPH_AUTOTAKE_DONE;

    /* start AUTOTAKE of semaphore for TDC access */
    HWREG(AUX_SMPH_BASE + AUX_SMPH_O_AUTOTAKE) = AUX_TDC_SEMAPHORE_NUMBER;

    return (false);
}

/*
 *  ======== updateSubSecInc ========
 *  Update the SUBSECINC register based on measured RCOSC_LF frequency
 */
static void updateSubSecInc(uint32_t tdcResult)
{
    int32_t newSubSecInc;
    uint32_t oldSubSecInc;
    uint32_t subSecInc;
    uint32_t ccfgModeConfReg;
    int32_t hposcOffset;
    int32_t hposcOffsetInv;

    /*
     * Calculate the new SUBSECINC
     * Here's the formula: AON_RTC:SUBSECINC = (45813 * NR) / 256
     * Based on measuring 32 LF clock periods
     */
    newSubSecInc = (45813 * tdcResult) / 256;

    /* TODO: Replace with ccfgread driverlib call */
    ccfgModeConfReg = HWREG( CCFG_BASE + CCFG_O_MODE_CONF );
    /* Compensate HPOSC drift if HPOSC is in use */
    if(((ccfgModeConfReg & CCFG_MODE_CONF_XOSC_FREQ_M ) >> CCFG_MODE_CONF_XOSC_FREQ_S) == 1) {
        /* Get the HPOSC relative offset at this temperature */
        hposcOffset = OSC_HPOSCRelativeFrequencyOffsetGet(AONBatMonTemperatureGetDegC());
        /* Convert to RF core format */
        hposcOffsetInv = OSC_HPOSCRelativeFrequencyOffsetToRFCoreFormatConvert(hposcOffset);
        /* Adjust SUBSECINC */
        newSubSecInc += (((newSubSecInc >> 4) * (hposcOffsetInv >> 3)) >> 15);
    }

    /* Apply filter, but not for first calibration */
    if (PowerCC26XX_module.firstLF) {
        /* Don't apply filter first time, to converge faster */
        subSecInc = newSubSecInc;
        /* No longer first measurement */
        PowerCC26XX_module.firstLF = false;
    }
    else {
        /* Read old SUBSECINC value */
        oldSubSecInc = HWREG(AON_RTC_BASE + AON_RTC_O_SUBSECINC) & 0x00FFFFFF;
        /* Apply filter, 0.5 times old value, 0.5 times new value */
        subSecInc = (oldSubSecInc * 1 + newSubSecInc * 1) / 2;
    }

    /* Update SUBSECINC values */
    HWREG(AUX_WUC_BASE + AUX_WUC_O_RTCSUBSECINC0) = subSecInc;
    HWREG(AUX_WUC_BASE + AUX_WUC_O_RTCSUBSECINC1) = subSecInc >> 16;

    /* update to use new values */
    HWREG(AUX_WUC_BASE + AUX_WUC_O_RTCSUBSECINCCTL) =
        AUX_WUC_RTCSUBSECINCCTL_UPD_REQ;
}

/*
 *  ======== PowerCC26XX_calibrateRcoscHf1 ========
 *  Calibrate RCOSC_HF agains XOSC_HF: compute and setup new trims
 */
static void calibrateRcoscHf1(int32_t tdcResult)
{
    /* *** STEP 1: Find RCOSC_HF-XOSC_HF frequency offset with current trim settings */
    /* Read in current trim settings */
    PowerCC26XX_module.nCtrimCurr =
        (DDI32RegRead(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_RCOSCHFCTL) &
        DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_M) >>
        DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_S;

    PowerCC26XX_module.nCtrimFractCurr =
        (DDI32RegRead(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL1_LOCAL)
        & DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_M) >>
        DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_S;

    PowerCC26XX_module.nRtrimCurr =
        (DDI32RegRead(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_ATESTCTL)
        & DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_M) >>
        DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_S;


    /*
     * Find RCOSC_HF-XOSC_HF frequency offset with current trim settings
     *   Positive value => RCOSC_HF runs slow, CTRIM(FRACT) should be increased
     *   Negative value => RCOSC_HF runs fast, CTRIM(FRACT) should be decreased
     * Resolution: 31.25 kHz; CTRIMFRACT resolution ~30 kHz
     */
    PowerCC26XX_module.nDeltaFreqCurr = (int32_t) tdcResult - RCOSC_HF_PERFECT_TDC_VALUE;

    /* *** STEP 2: Attempt to calculate more optimal settings */
    if (PowerCC26XX_module.nDeltaFreqCurr == 0) {
        /* If perfect, don't perform second measurement and keep current settings */
        PowerCC26XX_module.bRefine = false;
        return;
    }
    if (PowerCC26XX_module.bRefine) {
        /*
         * Trying to find better match across CTRIM/RTRIM. Due to mismatches the
         * first try might not have been more optimal than the current setting.
         * Continue refining, starting from stored values
         */
    } else {
        /* Start from current values */
        PowerCC26XX_module.nCtrimFractNew = PowerCC26XX_module.nCtrimFractCurr;
        PowerCC26XX_module.nCtrimNew      = PowerCC26XX_module.nCtrimCurr;
        PowerCC26XX_module.nRtrimNew      = PowerCC26XX_module.nRtrimCurr;
        PowerCC26XX_module.nDeltaFreqNew  = PowerCC26XX_module.nDeltaFreqCurr;
    }

    /*
     * Calculate change to CTRIMFRACT with safe assumptions of gain,
     * apply delta to current CTRIMFRACT and convert to valid CTRIM/CTRIMFRACT
     */
    PowerCC26XX_module.nCtrimFractNew = PowerCC26XX_module.nCtrimFractNew +
                                        Scale_rndInf(PowerCC26XX_module.nDeltaFreqNew);
    PowerCC26XX_module.nCtrimNew = PowerCC26XX_module.nCtrimCurr;

    /* One step of CTRIM is about 500 kHz, so limit to one CTRIM step */
    if (PowerCC26XX_module.nCtrimFractNew < 1) {
        if (PowerCC26XX_module.nRtrimNew == 3) {
            /* We try the slow RTRIM in this CTRIM first */
            PowerCC26XX_module.nCtrimFractNew = Max(1, PowerCC26XX_module.nCtrimFractNew + 21);
            PowerCC26XX_module.nRtrimNew = 0;
        }
        else {
            /* Step down one CTRIM and use fast RTRIM */
            PowerCC26XX_module.nCtrimFractNew = Max(1, PowerCC26XX_module.nCtrimFractNew + 32 - 21);
            PowerCC26XX_module.nCtrimNew = Max(0, PowerCC26XX_module.nCtrimNew - 1);
            PowerCC26XX_module.nRtrimNew = 3;
        }
    }
    else if (PowerCC26XX_module.nCtrimFractNew > 30) {
        if (PowerCC26XX_module.nRtrimNew == 0) {
            /* We try the slow RTRIM in this CTRIM first */
            PowerCC26XX_module.nCtrimFractNew = Min(30, PowerCC26XX_module.nCtrimFractNew - 21);
            PowerCC26XX_module.nRtrimNew = 3;
        }
        else {
            /* Step down one CTRIM and use fast RTRIM */
            PowerCC26XX_module.nCtrimFractNew = Min(30, PowerCC26XX_module.nCtrimFractNew - 32 + 21);
            PowerCC26XX_module.nCtrimNew = Min(0x3F, PowerCC26XX_module.nCtrimNew + 1);
            PowerCC26XX_module.nRtrimNew = 0;
        }
    }
    else
    {
        /* We're within sweet spot of current CTRIM => no change */
    }

    /* Find RCOSC_HF vs XOSC_HF frequency offset with new trim settings */
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_RCOSCHFCTL,
                           DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_M,
                           DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_S,
                           PowerCC26XX_module.nCtrimNew);

    /* Enable RCOSCHFCTRIMFRACT_EN */
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL1_LOCAL,
                           DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN_LOCAL_M,
                           DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN_LOCAL_S,
                           1);

    /* Modify CTRIM_FRACT */
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL1_LOCAL,
                           DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_M,
                           DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_S,
                           PowerCC26XX_module.nCtrimFractNew);

    /* Modify RTRIM */
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_ATESTCTL,
                           DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_M,
                           DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_S,
                           PowerCC26XX_module.nRtrimNew);
}

/*
 *  ======== Power_calibrateRcoscHf2 ========
 *  Calibrate RCOSC_HF agains XOSC_HF: determine better result, set new trims
 */
static void calibrateRcoscHf2(int32_t tdcResult)
{

    PowerCC26XX_module.nDeltaFreqNew = (int32_t) tdcResult - RCOSC_HF_PERFECT_TDC_VALUE;
    /* Calculate new delta freq */

    /* *** STEP 4: Determine whether the new settings are better or worse */
    if (Abs(PowerCC26XX_module.nDeltaFreqNew) <= Abs(PowerCC26XX_module.nDeltaFreqCurr)) {
        /* New settings are better or same -> make current by keeping in registers */
        PowerCC26XX_module.bRefine = false;
    }
    else {
        /* First measurement was better than second, restore current settings */
        DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_RCOSCHFCTL,
                           DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_M,
                           DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_S,
                           PowerCC26XX_module.nCtrimCurr);

        DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL1_LOCAL,
                           DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_M,
                           DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_S,
                           PowerCC26XX_module.nCtrimFractCurr);

        DDI16BitfieldWrite(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_ATESTCTL,
                           DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_M,
                           DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_S,
                           PowerCC26XX_module.nRtrimCurr);

        /* Enter a refinement mode where we keep searching for better matches */
        PowerCC26XX_module.bRefine = true;
    }

}
