/*
 * Copyright 2017-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
 * !!GlobalInfo
 * product: Pins v6.0
 * processor: LPC55S69
 * package_id: LPC55S69JBD100
 * mcu_data: ksdk2_0
 * processor_version: 0.0.0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_iocon.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
	BOARD_InitPins();
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
 * BOARD_InitPins:
 * - options: {callFromInitBoot: 'true', coreID: cm33_core0, enableClock: 'true'}
 * - pin_list:
 * - {pin_num: '21', peripheral: SWD, signal: SWO, pin_signal: PIO0_10/FC6_SCK/CT_INP10/CTIMER2_MAT0/FC1_TXD_SCL_MISO_WS/SCT0_OUT2/SWO/SECURE_GPIO0_10/ADC0_1, mode: inactive,
 *  slew_rate: standard, invert: disabled, open_drain: disabled, asw: disabled}
 * - {pin_num: '10', peripheral: GPIO, signal: 'PIO1, 9', pin_signal: PIO1_9/FC1_SCK/CT_INP4/SCT0_OUT2/FC4_CTS_SDA_SSEL0/ADC0_12, mode: inactive, slew_rate: standard,
 *  invert: disabled, open_drain: disabled, asw: enabled}
 * - {pin_num: '88', peripheral: GPIO, signal: 'PIO0, 5', pin_signal: PIO0_5/FC4_RXD_SDA_MOSI_DATA/CTIMER3_MAT0/SCT_GPI5/FC3_RTS_SCL_SSEL1/MCLK/SECURE_GPIO0_5, mode: pullUp,
 *  slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '64', peripheral: GPIO, signal: 'PIO1, 18', pin_signal: PIO1_18/SD1_POW_EN/SCT0_OUT5/PLU_OUT0, mode: inactive, slew_rate: standard, invert: disabled,
 *  open_drain: disabled}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the Cortex-M33 (Core #0) */
void BOARD_InitPins(void)
{
	/* Enables the clock for the I/O controller.: Enable Clock. */
	CLOCK_EnableClock(kCLOCK_Iocon);

	const uint32_t port0_pin10_config = (/* Pin is configured as SWO */
		IOCON_PIO_FUNC6 |
		/* No addition pin function */
		IOCON_PIO_MODE_INACT |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI |
		/* Analog switch is open (disabled) */
		IOCON_PIO_ASW_DI);
	/* PORT0 PIN10 (coords: 21) is configured as SWO */
	IOCON_PinMuxSet(IOCON, 0U, 10U, port0_pin10_config);

	const uint32_t port0_pin5_config = (/* Pin is configured as PIO0_5 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-up function */
		IOCON_PIO_MODE_PULLUP |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT0 PIN5 (coords: 88) is configured as PIO0_5 */
	IOCON_PinMuxSet(IOCON, 0U, 5U, port0_pin5_config);

	const uint32_t port1_pin18_config = (/* Pin is configured as PIO1_18 */
		IOCON_PIO_FUNC0 |
		/* No addition pin function */
		IOCON_PIO_MODE_INACT |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT1 PIN18 (coords: 64) is configured as PIO1_18 */
	IOCON_PinMuxSet(IOCON, 1U, 18U, port1_pin18_config);

	if (Chip_GetVersion() == 1) {
		const uint32_t port1_pin9_config = (/* Pin is configured as PIO1_9 */
			IOCON_PIO_FUNC0 |
			/* No addition pin function */
			IOCON_PIO_MODE_INACT |
			/* Standard mode, output slew rate control is enabled */
			IOCON_PIO_SLEW_STANDARD |
			/* Input function is not inverted */
			IOCON_PIO_INV_DI |
			/* Enables digital function */
			IOCON_PIO_DIGITAL_EN |
			/* Open drain is disabled */
			IOCON_PIO_OPENDRAIN_DI |
			/* Analog switch is closed (enabled) */
			IOCON_PIO_ASW_EN);
		/* PORT1 PIN9 (coords: 10) is configured as PIO1_9 */
		IOCON_PinMuxSet(IOCON, 1U, 9U, port1_pin9_config);
	} else {
		const uint32_t port1_pin9_config = (/* Pin is configured as PIO1_9 */
			IOCON_PIO_FUNC0 |
			/* No addition pin function */
			IOCON_PIO_MODE_INACT |
			/* Standard mode, output slew rate control is enabled */
			IOCON_PIO_SLEW_STANDARD |
			/* Input function is not inverted */
			IOCON_PIO_INV_DI |
			/* Enables digital function */
			IOCON_PIO_DIGITAL_EN |
			/* Open drain is disabled */
			IOCON_PIO_OPENDRAIN_DI |
			/* Analog switch is closed (enabled), only for A0 version */
			IOCON_PIO_ASW_DIS_EN);
		/* PORT1 PIN9 (coords: 10) is configured as PIO1_9 */
		IOCON_PinMuxSet(IOCON, 1U, 9U, port1_pin9_config);
	}
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
 * manage_evk_io_optimization:
 * - options: {callFromInitBoot: 'false', coreID: cm33_core0, enableClock: 'true'}
 * - pin_list:
 * - {pin_num: '1', peripheral: GPIO, signal: 'PIO1, 4', pin_signal: PIO1_4/FC0_SCK/SD0_D0/CTIMER2_MAT1/SCT0_OUT0/FREQME_GPIO_CLK_A, mode: pullUp, slew_rate: standard,
 *  invert: disabled, open_drain: disabled}
 * - {pin_num: '5', peripheral: GPIO, signal: 'PIO1, 6', pin_signal: PIO1_6/FC0_TXD_SCL_MISO_WS/SD0_D3/CTIMER2_MAT1/SCT_GPI3, mode: pullUp, slew_rate: standard, invert: disabled,
 *  open_drain: disabled}
 * - {pin_num: '9', peripheral: GPIO, signal: 'PIO1, 7', pin_signal: PIO1_7/FC0_RTS_SCL_SSEL1/SD0_D1/CTIMER2_MAT2/SCT_GPI4, mode: pullUp, slew_rate: standard, invert: disabled,
 *  open_drain: disabled}
 * - {pin_num: '89', peripheral: GPIO, signal: 'PIO0, 6', pin_signal: PIO0_6/FC3_SCK/CT_INP13/CTIMER4_MAT0/SCT_GPI6/SECURE_GPIO0_6, mode: pullDown, slew_rate: standard,
 *  invert: disabled, open_drain: disabled}
 * - {pin_num: '92', peripheral: FLEXCOMM0, signal: RXD_SDA_MOSI_DATA, pin_signal: PIO0_29/FC0_RXD_SDA_MOSI_DATA/SD1_D2/CTIMER2_MAT3/SCT0_OUT8/CMP0_OUT/PLU_OUT2/SECURE_GPIO0_29,
 *  mode: pullDown, slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '94', peripheral: FLEXCOMM0, signal: TXD_SCL_MISO_WS, pin_signal: PIO0_30/FC0_TXD_SCL_MISO_WS/SD1_D3/CTIMER0_MAT0/SCT0_OUT9/SECURE_GPIO0_30, mode: pullDown,
 *  slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '90', peripheral: GPIO, signal: 'PIO0, 19', pin_signal: PIO0_19/FC4_RTS_SCL_SSEL1/UTICK_CAP0/CTIMER0_MAT2/SCT0_OUT2/FC7_TXD_SCL_MISO_WS/PLU_IN4/SECURE_GPIO0_19,
 *  mode: pullDown, slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '74', peripheral: GPIO, signal: 'PIO0, 20', pin_signal: PIO0_20/FC3_CTS_SDA_SSEL0/CTIMER1_MAT1/CT_INP15/SCT_GPI2/FC7_RXD_SDA_MOSI_DATA/HS_SPI_SSEL0/PLU_IN5/SECURE_GPIO0_20/FC4_TXD_SCL_MISO_WS,
 *  mode: pullDown, slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '76', peripheral: GPIO, signal: 'PIO0, 21', pin_signal: PIO0_21/FC3_RTS_SCL_SSEL1/UTICK_CAP3/CTIMER3_MAT3/SCT_GPI3/FC7_SCK/PLU_CLKIN/SECURE_GPIO0_21,
 *  mode: pullDown, slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '2', peripheral: GPIO, signal: 'PIO1, 13', pin_signal: PIO1_13/FC6_RXD_SDA_MOSI_DATA/CT_INP6/USB0_OVERCURRENTN/USB0_FRAME/SD0_CARD_DET_N, mode: pullDown,
 *  slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '91', peripheral: GPIO, signal: 'PIO1, 31', pin_signal: PIO1_31/MCLK/SD1_CLK/CTIMER0_MAT2/SCT0_OUT6/PLU_IN0, mode: pullDown, slew_rate: standard, invert: disabled,
 *  open_drain: disabled}
 * - {pin_num: '66', peripheral: GPIO, signal: 'PIO0, 28', pin_signal: PIO0_28/FC0_SCK/SD1_CMD/CT_INP11/SCT0_OUT7/USB0_OVERCURRENTN/PLU_OUT1/SECURE_GPIO0_28, mode: pullDown,
 *  slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '67', peripheral: GPIO, signal: 'PIO1, 12', pin_signal: PIO1_12/FC6_SCK/CTIMER1_MAT1/USB0_PORTPWRN/HS_SPI_SSEL2, mode: pullDown, slew_rate: standard,
 *  invert: disabled, open_drain: disabled}
 * - {pin_num: '80', peripheral: GPIO, signal: 'PIO1, 29', pin_signal: PIO1_29/FC7_RXD_SDA_MOSI_DATA/SD0_D6/SCT_GPI6/USB1_PORTPWRN/USB1_FRAME/PLU_IN2, mode: pullDown,
 *  slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '65', peripheral: GPIO, signal: 'PIO1, 30', pin_signal: PIO1_30/FC7_TXD_SCL_MISO_WS/SD0_D7/SCT_GPI7/USB1_OVERCURRENTN/USB1_LEDN/PLU_IN1, mode: pullDown,
 *  slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '41', peripheral: GPIO, signal: 'PIO1, 22', pin_signal: PIO1_22/SD0_CMD/CTIMER2_MAT3/SCT_GPI5/FC4_SSEL3/PLU_OUT4, mode: pullDown, slew_rate: standard,
 *  invert: disabled, open_drain: disabled}
 * - {pin_num: '73', peripheral: GPIO, signal: 'PIO1, 28', pin_signal: PIO1_28/FC7_SCK/SD0_D5/CT_INP2/PLU_IN3, mode: pullDown, slew_rate: standard, invert: disabled,
 *  open_drain: disabled}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : manage_evk_io_optimization
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the Cortex-M33 (Core #0) */
void manage_evk_io_optimization(void)
{
	/* Enables the clock for the I/O controller.: Enable Clock. */
	CLOCK_EnableClock(kCLOCK_Iocon);

	const uint32_t port0_pin19_config = (/* Pin is configured as PIO0_19 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT0 PIN19 (coords: 90) is configured as PIO0_19 */
	IOCON_PinMuxSet(IOCON, 0U, 19U, port0_pin19_config);

	const uint32_t port0_pin20_config = (/* Pin is configured as PIO0_20 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT0 PIN20 (coords: 74) is configured as PIO0_20 */
	IOCON_PinMuxSet(IOCON, 0U, 20U, port0_pin20_config);

	const uint32_t port0_pin21_config = (/* Pin is configured as PIO0_21 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT0 PIN21 (coords: 76) is configured as PIO0_21 */
	IOCON_PinMuxSet(IOCON, 0U, 21U, port0_pin21_config);

	const uint32_t port0_pin28_config = (/* Pin is configured as PIO0_28 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT0 PIN28 (coords: 66) is configured as PIO0_28 */
	IOCON_PinMuxSet(IOCON, 0U, 28U, port0_pin28_config);

	const uint32_t port0_pin29_config = (/* Pin is configured as FC0_RXD_SDA_MOSI_DATA */
		IOCON_PIO_FUNC1 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT0 PIN29 (coords: 92) is configured as FC0_RXD_SDA_MOSI_DATA */
	IOCON_PinMuxSet(IOCON, 0U, 29U, port0_pin29_config);

	const uint32_t port0_pin30_config = (/* Pin is configured as FC0_TXD_SCL_MISO_WS */
		IOCON_PIO_FUNC1 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT0 PIN30 (coords: 94) is configured as FC0_TXD_SCL_MISO_WS */
	IOCON_PinMuxSet(IOCON, 0U, 30U, port0_pin30_config);

	const uint32_t port0_pin6_config = (/* Pin is configured as PIO0_6 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT0 PIN6 (coords: 89) is configured as PIO0_6 */
	IOCON_PinMuxSet(IOCON, 0U, 6U, port0_pin6_config);

	const uint32_t port1_pin12_config = (/* Pin is configured as PIO1_12 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT1 PIN12 (coords: 67) is configured as PIO1_12 */
	IOCON_PinMuxSet(IOCON, 1U, 12U, port1_pin12_config);

	const uint32_t port1_pin13_config = (/* Pin is configured as PIO1_13 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT1 PIN13 (coords: 2) is configured as PIO1_13 */
	IOCON_PinMuxSet(IOCON, 1U, 13U, port1_pin13_config);

	const uint32_t port1_pin22_config = (/* Pin is configured as PIO1_22 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT1 PIN22 (coords: 41) is configured as PIO1_22 */
	IOCON_PinMuxSet(IOCON, 1U, 22U, port1_pin22_config);

	const uint32_t port1_pin28_config = (/* Pin is configured as PIO1_28 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT1 PIN28 (coords: 73) is configured as PIO1_28 */
	IOCON_PinMuxSet(IOCON, 1U, 28U, port1_pin28_config);

	const uint32_t port1_pin29_config = (/* Pin is configured as PIO1_29 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT1 PIN29 (coords: 80) is configured as PIO1_29 */
	IOCON_PinMuxSet(IOCON, 1U, 29U, port1_pin29_config);

	const uint32_t port1_pin30_config = (/* Pin is configured as PIO1_30 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT1 PIN30 (coords: 65) is configured as PIO1_30 */
	IOCON_PinMuxSet(IOCON, 1U, 30U, port1_pin30_config);

	const uint32_t port1_pin31_config = (/* Pin is configured as PIO1_31 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-down function */
		IOCON_PIO_MODE_PULLDOWN |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT1 PIN31 (coords: 91) is configured as PIO1_31 */
	IOCON_PinMuxSet(IOCON, 1U, 31U, port1_pin31_config);

	const uint32_t port1_pin4_config = (/* Pin is configured as PIO1_4 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-up function */
		IOCON_PIO_MODE_PULLUP |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT1 PIN4 (coords: 1) is configured as PIO1_4 */
	IOCON_PinMuxSet(IOCON, 1U, 4U, port1_pin4_config);

	const uint32_t port1_pin6_config = (/* Pin is configured as PIO1_6 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-up function */
		IOCON_PIO_MODE_PULLUP |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT1 PIN6 (coords: 5) is configured as PIO1_6 */
	IOCON_PinMuxSet(IOCON, 1U, 6U, port1_pin6_config);

	const uint32_t port1_pin7_config = (/* Pin is configured as PIO1_7 */
		IOCON_PIO_FUNC0 |
		/* Selects pull-up function */
		IOCON_PIO_MODE_PULLUP |
		/* Standard mode, output slew rate control is enabled */
		IOCON_PIO_SLEW_STANDARD |
		/* Input function is not inverted */
		IOCON_PIO_INV_DI |
		/* Enables digital function */
		IOCON_PIO_DIGITAL_EN |
		/* Open drain is disabled */
		IOCON_PIO_OPENDRAIN_DI);
	/* PORT1 PIN7 (coords: 9) is configured as PIO1_7 */
	IOCON_PinMuxSet(IOCON, 1U, 7U, port1_pin7_config);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
