/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_ctimer.h"

#include "pin_mux.h"
#include <stdbool.h>
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CTIMER          CTIMER2         /* Timer 2 */
#define CTIMER_MAT0_OUT kCTIMER_Match_1 /* Match output 1 */
#define CTIMER_MAT1_OUT kCTIMER_Match_2 /* Match output 2 */
#define CTIMER_CLK_FREQ CLOCK_GetCTimerClkFreq(2U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ctimer_match0_callback(uint32_t flags);
void ctimer_match1_callback(uint32_t flags);

/* Array of function pointers for callback for each channel */
ctimer_callback_t ctimer_callback_table[] = {
    NULL, ctimer_match0_callback, ctimer_match1_callback, NULL, NULL, NULL, NULL, NULL};

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Match Configuration for Channel 0 */
static ctimer_match_config_t matchConfig0;
/* Match Configuration for Channel 1 */
static ctimer_match_config_t matchConfig1;

/*******************************************************************************
 * Code
 ******************************************************************************/

void ctimer_match1_callback(uint32_t flags)
{
    static uint32_t count            = 0;
    static uint32_t matchUpdateCount = 8;

    if (++count > matchUpdateCount)
    {
        count = 0;
        matchConfig1.matchValue >>= 1;
        matchUpdateCount <<= 1;
        if (matchUpdateCount == (1 << 8))
        {
            matchUpdateCount        = 8;
            matchConfig1.matchValue = CTIMER_CLK_FREQ / 2;
        }
        CTIMER_SetupMatch(CTIMER, CTIMER_MAT1_OUT, &matchConfig1);
    }
}

void ctimer_match0_callback(uint32_t flags)
{
    static uint32_t count            = 0;
    static uint32_t matchUpdateCount = 8;

    if (++count > matchUpdateCount)
    {
        count = 0;
        matchConfig0.matchValue >>= 1;
        matchUpdateCount <<= 1;
        if (matchUpdateCount == (1 << 8))
        {
            matchUpdateCount        = 8;
            matchConfig0.matchValue = CTIMER_CLK_FREQ / 2;
        }
        CTIMER_SetupMatch(CTIMER, CTIMER_MAT0_OUT, &matchConfig0);
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    ctimer_config_t config;

    /* Init hardware*/
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Use FRO_HF clock as input clock source. */
    CLOCK_AttachClk(kFRO_HF_to_CTIMER2);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("CTimer match example to toggle the output. \r\n");
    PRINTF("This example uses interrupt to change the match period. \r\n");

    CTIMER_GetDefaultConfig(&config);

    CTIMER_Init(CTIMER, &config);

    /* Configuration 0 */
    matchConfig0.enableCounterReset = true;
    matchConfig0.enableCounterStop  = false;
    matchConfig0.matchValue         = CTIMER_CLK_FREQ / 2;
    matchConfig0.outControl         = kCTIMER_Output_Toggle;
    matchConfig0.outPinInitState    = false;
    matchConfig0.enableInterrupt    = true;

    /* Configuration 1 */
    matchConfig1.enableCounterReset = true;
    matchConfig1.enableCounterStop  = false;
    matchConfig1.matchValue         = CTIMER_CLK_FREQ / 2;
    matchConfig1.outControl         = kCTIMER_Output_Toggle;
    matchConfig1.outPinInitState    = true;
    matchConfig1.enableInterrupt    = true;

    CTIMER_RegisterCallBack(CTIMER, &ctimer_callback_table[0], kCTIMER_MultipleCallback);
    CTIMER_SetupMatch(CTIMER, CTIMER_MAT0_OUT, &matchConfig0);
    CTIMER_SetupMatch(CTIMER, CTIMER_MAT1_OUT, &matchConfig1);
    CTIMER_StartTimer(CTIMER);

    while (1)
    {
    }
}
