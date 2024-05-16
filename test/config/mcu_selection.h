/**
 * @file mcu_selection.h
 * @author Tim Koczwara (tim.koczwara@esope.de)
 * @brief Configuration for the selected mcu.
 * @version 1.0
 * @date 19.01.2022
 * 
 * @copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * 
 */
#ifndef __MCU_SELECTION_H_GUARD__
#define __MCU_SELECTION_H_GUARD__

/// Enable/disable debug functions in the mcu.h .
#define MCU_DEBUG_ENABLE							false				

/***************************************************************************************************************************
 *  Part 1: Features
 ***************************************************************************************************************************/

// Peripherie aktivieren
/// Enables/disables the timer used for mcu_wait functions
#define MCU_PERIPHERY_ENABLE_WAIT_TIMER				true
/// Enable/disable rtc functions.
#define MCU_PERIPHERY_ENABLE_RTC					false
/// Set this to enable the external quartz for RTC.
#define MCU_PERIPHERY_ENABLE_EXT_QUARTZ				false
/// Enable/disable functions to write into the code flash.
#define MCU_PERIPHERY_ENABLE_CODE_FLASH				false
/// Enable/disable functions to write into the data flash.
#define MCU_PERIPHERY_ENABLE_DATA_FLASH				false
/// Enable/disable Watchdog functions.
#define MCU_PERIPHERY_ENABLE_WATCHDOG				true
/// Enable/disable ethernet. If enable, lwip is needed in the project
#define MCU_PERIPHERY_ENABLE_ETHERNET 				true
/// Enable/disable wifi of mcu.
#define MCU_PERIPHERY_ENABLE_WIFI 					false
/// Number of allocated IO Interrupt instances.
#define MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT		0
/// Number of allocated Timer instances.
#define MCU_PERIPHERY_DEVICE_COUNT_TIMER			0
/// Number of allocated UART instances.
#define MCU_PERIPHERY_DEVICE_COUNT_UART				0
/// Number of allocated SPI instances.
#define MCU_PERIPHERY_DEVICE_COUNT_SPI				0
/// Number of allocated I2C instances.
#define MCU_PERIPHERY_DEVICE_COUNT_I2C				0					
/// Number of allocated CAN instances.
#define MCU_PERIPHERY_DEVICE_COUNT_CAN				0
/// Number of allocated AD instances.
#define MCU_PERIPHERY_DEVICE_COUNT_AD				0					
/// Number of allocated AD instances.
#define MCU_PERIPHERY_DEVICE_COUNT_DA				0
/// Number of allocatable PWM instances.
#define MCU_PERIPHERY_DEVICE_COUNT_PWM              0

/// Enable/disable comm support for bridging serial data over the can. More details in the CAN
/// section of the mcu.h How-to-use
#define MCU_PERIPHERY_ENABLE_COMM_MODE_CAN			false				

/// Enable/disable comm interface create function for the UART.
#define MCU_PERIPHERY_ENABLE_COMM_MODE_UART			true

/// Enable/disable freertos
#define MCU_ENABLE_FREERTOS							false
																		
/***************************************************************************************************************************
 *  Part 2: MCU configuration
 ***************************************************************************************************************************/

/// Number of Pins the MCU has. */
#define MCU_CONTROLLER_PIN_COUNT					100					
/// Size of the internal Flash Memory in KBytes. */
#define MCU_CONTROLLER_ROM_SIZE_KBYTE				256

#include "mcu/pc/mcu_controller.h"	// Includes the controller specific definitions and enumerations

#endif
