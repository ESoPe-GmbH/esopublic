/**
 * 	@file 	mcu_io.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_io functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *
 *  @version    2.01 (04.09.2024)
 *               - Added mcu_io_reset
 *  @version    2.00 (20.08.2022)
 *               - Extracted mcu_io functions in this file. History from now on will only contain infos about mcu_io
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
 *  @see	[1] http://de.wikipedia.org/wiki/Serial_Peripheral_Interface am 06.08.2011
 *
 ******************************************************************************/

#ifndef __MCU_IO_HEADER_FIRST_INCLUDE__
#define __MCU_IO_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

//------------------------------------------------------------------------------------------------------------
// I/O Macros
//------------------------------------------------------------------------------------------------------------
/// Macro can be used to toggle the output of an io pin. The parameter s needs to be a MCU_IO_PIN type.
#define MCU_IO_TOGGLE(s)     					mcu_io_toggle(s)
//mcu_io_set(s, mcu_io_get(s)^1)

/**
 * Can be used to initialize a mcu_io_handler_t with a pin and the inverted value.
 * @param pin		MCU_IO_PIN used inside the handler.
 * @param inverted	true if the pin is inverted, false if not.
 **/ 
#define MCU_IO_HANDLER_INIT(pin, inverted)		{pin, inverted}		
																									
/** 
 *	Used to set the output value of an mcu_io_handler_t pin.
 *	@param handler		The mcu_io_handler_t used for which the output value should be set.
 *	@param value		1 or 0 for the output value. Is inverted according to the inverted byte of the structure.
 **/
#define MCU_IO_SET_HANDLER(handler, value)		mcu_io_set((handler).pin, value^(handler).is_inverted)	

/** 
 *	Same macro as MCU_IO_SET_HANDLER with the difference that handler is a pointer to a mcu_io_handler_t.
 *	@param handler		Pointer to the mcu_io_handler_t used for which the output value should be set.
 *	@param value		1 or 0 for the output value. Is inverted according to the inverted byte of the structure.
 **/
#define MCU_IO_SET_HANDLER_PTR(handler, value)	MCU_IO_SET_HANDLER(*handler, value)

/** 
 * 	Used to read the input value of an mcu_io_handler_t pin.
 * 	@param handler		The mcu_io_handler_t used for which the input value should be read.
 * 	@return				Returns the input level which is inverted according to the inverted byte inside the structure.
 **/
#define MCU_IO_GET_HANDLER(handler)				(mcu_io_get((handler).pin)^(handler).is_inverted)	

/** 
 *	Same macro as MCU_IO_GET_HANDLER with the difference that handler is a pointer to a mcu_io_handler_t.
 * 	@param handler		Pointer to the mcu_io_handler_t used for which the input value should be read.
 * 	@return				Returns the input level which is inverted according to the inverted byte inside the structure.
 **/
#define MCU_IO_GET_HANDLER_PTR(handler)		MCU_IO_GET_HANDLER(*handler)

/** 
 *	Same as MCU_IO_TOGGLE(handler.pin) with the difference that mcu_io_handler_t is used instead of a MCU_IO_PIN.
 *	@param handler		The mcu_io_handler_t used for which the output value should be toggled.
 **/
#define MCU_IO_TOGGLE_HANDLER(handler)		MCU_IO_TOGGLE((handler).pin)	

/** 
 *	Same macro as MCU_IO_TOGGLE with the difference that handler is a pointer to a mcu_io_handler_t.
 *	@param handler		Pointer to the mcu_io_handler_t used for which the output value should be toggled.
 **/
#define MCU_IO_TOGGLE_HANDLER_PTR(handler)		MCU_IO_TOGGLE_HANDLER(*handler)

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

/**
 * @brief   Reset all GPIO functionality like alternate functions to use it as a GPIO.
 * 
 * @param p					I/O Pin
 */
void mcu_io_reset(MCU_IO_PIN p);
/**
 * @brief	Sets the direction of a port (8 Pins) at once.
 * @param p					I/O Port (For example P2 to set the direction for the pins P2_0 - P2_7)
 * @param d 				Bitwise direction for each pin. E.g. 0xFF to set all pins to output direction.
 */
void mcu_io_set_port_dir(MCU_IO_PIN p, uint8_t d);		
/**
 * @brief	Sets the direction of a pin.
 * @param p					I/O Pin
 * @param d 				Direction of the pin (input or output).
 */
void mcu_io_set_dir(MCU_IO_PIN p, MCU_IO_DIRECTION d);	
/**
 * @brief	Enables/Disables Pull-Ups of mcu pins (not for ports). Not every pin has a pull-up. On some mcus some pins share the same
 * 			pull-up register so with activating one the others are also active.
 * @param p					I/O Pin
 * @param pullup_active 	true: Enable Pull Up.\n
 * 							false: Disable Pull Up.
 */
void mcu_io_set_pullup(MCU_IO_PIN p, bool pullup_active);
/**
* @brief	Sets the output value for a single pin or a whole port (8 Pins).
* @param p					I/O Pin or I/O Port
* @param d 					If p is a pin: 1 or 0\n
* 							If p is a port: Bitwise output value for each pin of the port.
*/
void mcu_io_set(MCU_IO_PIN p, uint8_t d);
/*
 * Toggles the output of a single pin or whole port.
 * @param p					I/O Pin or I/O Port
 */
void mcu_io_toggle(MCU_IO_PIN p);
/**
 * @brief	Returns the input value for a single pin or port (8 Pins).
* @param p					I/O Pin or I/O Port
* @return					If p is a pin: 1 or 0\n
* 							If p is a port: Bitwise input value for each pin of the port.
 */
uint8_t mcu_io_get(MCU_IO_PIN p);	

//------------------------------------------------------------------------------------------------------------
// I/O expander functions
//------------------------------------------------------------------------------------------------------------

/**
 * @brief 	Register an I/O Expander to the mcu to make it available for mcu_io_ functions.
 * 			Before registering you need to initialize the I/O expander and the mcu_io_expander_t with the object of the I/O expander as well 
 * 			as it's pin_base, pin_count and functions. Based on these the mcu will call the I/O expander if a Pin is used on the mcu_io_ functions
 * 			that is not a pin of the mcu.
 * 
 * @param io_expander 		Pointer to the I/O expander.
 */
void mcu_register_io_expander(mcu_io_expander_t* io_expander);
/**
 * @brief 	Removes a previously registered I/O expander from the mcu.
 * 
 * @param io_expander 		Pointer to the I/O expander.
 */
void mcu_unregister_io_expander(mcu_io_expander_t* io_expander);

#endif // __MCU_IO_HEADER_FIRST_INCLUDE__
