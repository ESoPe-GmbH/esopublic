/**
 * 	@file 	mcu_rtc.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_rtc functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *
 *  @version    2.00 (20.08.2022)
 *               - Extracted mcu_rtc functions in this file. History from now on will only contain infos about mcu_rtc.
 *	@version	1.12 (10.10.2016)
 *				 - Extracted types into new mcu_types.h
 *				 - Added I2C
 *	@version	1.11 (11.01.2015)
 *				 - Added mcu_flash_read -> Is needed for RL78
 *	@version	1.10 (13.11.2014)
 *				 - Changes mcu_io_interrupt_set_param -> There is now an object pointer as a parameter
 *				   inside the function and as a pointer parameter inside the callback function.
 *	@version	1.09 (25.05.2014)
 *				 - Added mcu_uart_set_direct_transmit_interrupt
 *				 - Added mcu_uart_transmit_ready
 *				 - Added mcu_io_interrupt_get_pin function
 *				 - Added rtc functions (init, get and set)
 *	@version	1.08 (28.02.2013)
 *				 - Translated the comments in english
 *				 - Added DA converter and flash to the How-to-use
 *				 - Added parameter to mcu_uart_init, mcu_spi_init and mcu_io_interrupt_init
 *	@version	1.07 (04.06.2012)
 *				 - Added DA converter
 *	@version	1.06 (02.01.2012)
 *				 - Added Flash Init function
 *				 - Changed defines for mcu_config.h 1.04
 *	@version	1.05 (24.09.2011)
 *				 - Added CAN
 *				 - Added Flash
 *	@version	1.04 (22.09.2011)
 *				 - MCU_PERIPHERY_ENABLE_ check removed for functions using MCU_PERIPHERY_DEVICE_COUNT_
 *				 - How-to-use rewritten for handler objects
 *	@version	1.03 (21.09.2011)
 *				 - Added mcu_io_handler_t with the macros MCU_IO_HANDLER_INIT, MCU_IO_SET_HANDLE, MCU_IO_SET_HANDLE_PTR,
 *				 	MCU_IO_GET_HANDLER, MCU_IO_GET_HANDLE_PTR, MCU_IO_TOGGLE_HANDLE and MCU_IO_TOGGLE_HANDLE_PTR
 *	@version	1.02
 *				 - Added debug functions, which needs to be implemented for each controller in the mcu.c
 *	@version	1.01
 *				 - Global Interrupt Levels for all MCU
 *  @version	1.00
 *  			 - Initial Release
 *
 ******************************************************************************/

#ifndef __MCU_RTC_HEADER_FIRST_INCLUDE__
#define __MCU_RTC_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_ENABLE_RTC

#ifdef _RENESAS_SYNERGY_
/**
 * @brief   Initializes the internal rtc of the mcu.
 * @param rtc       The RTC instance as created via configuration.xml
 */
void mcu_rtc_init(rtc_instance_t* rtc);
#else
/**
 * @brief	Initializes the internal rtc of the mcu.
 * @param use_ext_subclock		true: External 32kHz subclock is used for the rtc.
 * 								false: External main clock is used for the rtc.
 */
void mcu_rtc_init(bool use_ext_subclock);
#endif
/**
 * @brief	Sets the time of the time struct into the internal rtc.
 *
 * @param time				Pointer to the time structure that contains the current time.
 */
void mcu_rtc_set_time(rtc_time_t *time);

/**
 * @brief	Reads the current time from the rtc and writes it into the time struct.
 *
 * @param time				Pointer to the time structure where the current time will be written into.
 */
void mcu_rtc_get_time(rtc_time_t *time);

#endif

#endif // __MCU_RTC_HEADER_FIRST_INCLUDE__
