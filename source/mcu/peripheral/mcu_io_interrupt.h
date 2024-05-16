/**
 * 	@file 	mcu_io_interrupt.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_io_interrupt functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 * 	@subsection mcu_htu_ioint IO Interrupt Usage
 *		With some Input pins it is possible to set an IO Interrupt. This interrupt is triggered if the input level
 *		of the pin changes.
 *
 *		First thing to do using the IO interrupt is creating the handler. This action usually takes place in the board
 *		module, because this is the only function which needs to know which input pin the IO Interrupt needs.
 *
@code
	mcu_io_int_t my_io_int = mcu_io_interrupt_init(5, P1_7);
	if(my_io_int==NULL)	// Handle the error
		...
@endcode
 *
 * 		After creating the handler in the board you can use it in your module. The module needs to configure the
 * 		IO interrupt for its needs with @link mcu_io_interrupt_set_param mcu_io_interrupt_set_param@endlink.
 * 		First parameter is the handler object. Second parameter is a pointer to an object that is used as a parameter
 * 		inside the callback function. Third is a pointer to the function called after a I/O interrupt is triggered.
 * 		The Fourth parameter is the Interrupt level (None/Low/Medium/High) and the last
 * 		parameter is the trigger for the interrupt. The trigger might be a falling edge (input changes from high
 * 		to low), a rising edge (input changes from low to high) or on both edges.
 *
@code 
	void mcu_example_io_interrupt_trigger(void*);	// Prototype for interrupt function
	...
	if(MCU_OK == mcu_io_interrupt_set_param(my_io_int, NULL, mcu_example_io_interrupt_trigger,
														MCU_INT_LVL_MED, MCU_IO_INT_EDGE_BOTH))
		... // Settings OK and interrupt can be used
	else
		...	// Settings are invalid and interrupt can not be used
@endcode
 *
 *		After this the interrupt can be used. In some applications you will need to disable and enable this interrupts
 *		only in some special cases. For this you can use the @link mcu_io_interrupt_enable
 *		mcu_io_interrupt_enable@endlink and @link mcu_io_interrupt_disable mcu_io_interrupt_disable@endlink
 *		functions. Both of them only need the handler as parameter. If you call the enable function the interrupt level
 *		will be set to the level configured with the @link mcu_io_interrupt_set_param mcu_io_interrupt_set_param@endlink
 *		function.
 *
 *	@attention	If a pull-up is needed you should activate it in the board manually!
 *
 *
 *  @version    2.00 (20.08.2022)
 *               - Extracted mcu_io_interrupt functions in this file. History from now on will only contain infos about mcu_io_interrupt.
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

#ifndef __MCU_IO_INTERRUPT_HEADER_FIRST_INCLUDE__
#define __MCU_IO_INTERRUPT_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT>0
#ifdef _RENESAS_SYNERGY_
/**
 * Initializes the I/O Interrupt as set by the Synergy framework. The I/O Interrupt must be created inside configuration.xml.
 * @param h     Pointer to the I/O Interrupt instance as created with configuration.xml
 * @param pin   Pin of the I/O Interrupt, as the synergy framework is not storing it internally.
 */
MCU_RESULT mcu_io_interrupt_init(mcu_io_int_t h, MCU_IO_PIN pin);
#else
/**
 * @brief	Initializes a mcu_io_int_t pointer for the given io interrupt pin. If it is not possible to set the i/o interrupt
 * 			the function returns NULL and an error will be set into mcu_last_error.
 *
 *			The following errors might occur:
 * 			 - MCU_ERROR_IO_INT_PIN_INVALID: 	There is no i/o interrupt with the pin and number.\n
 *			 - MCU_ERROR_IO_INT_NOT_AVAILABLE: 	All i/o interrupts are in use. Adjust MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT in mcu_config.h
 *
 * @pre		Check MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT in the mcu_config.h before.
 *
 * @param num				Number of the I/O Interrupt.
 * @param pin				I/O Pin of the i/o interrupt.
 * @return					NULL: The i/o interrupt could not be initialized.
 *							Other then NULL: The i/o interrupt handler.
 */
mcu_io_int_t mcu_io_interrupt_init(uint8_t num, MCU_IO_PIN pin);
#endif

/**
 * @brief	Sets the parameters for an initializes mcu_io_int_t. Activates the interrupt if a MCU_INT_LVL is set.
 *
 *			The following errors might occur:
 *			 - MCU_ERROR_IO_INT_INVALID: The io interrupt handler is NULL.
 * 			 - MCU_ERROR_IO_INT_EDGE_INVALID: The edge does not exist.
 * 			 - MCU_ERROR_IO_INT_LVL_INVALID: The interrupt level does not exist.
 *
 * @pre		handle must be created with mcu_io_interrupt_init before.
 *
 * @param handle			I/o interrupt handler.
 * @param obj				Pointer to an object that will be used as a parameter in the callback function.\n
 * 							Can be NULL if no object pointer is needed.
 * @param f					Callback function for the io interrupt.\n
 * 							The callback function has one parameter which is always the same as the obj parameter in this function.
 * @param lvl				Interrupt priority level high, medium, low or none.
 * @param edge				Interrupt edge trigger: rising, falling or both edges.
 * @return					MCU_OK: No error occured.\n
 *			 				MCU_ERROR_IO_INT_INVALID: The io interrupt handler is NULL.\n
 * 			 				MCU_ERROR_IO_INT_EDGE_INVALID: The edge does not exist.\n
 * 			 				MCU_ERROR_IO_INT_LVL_INVALID: The interrupt level does not exist.
 */
MCU_RESULT mcu_io_interrupt_set_param(mcu_io_int_t handle, void* obj, void (*f)(void*), MCU_INT_LVL lvl, MCU_IO_INT_EDGE edge);

/**
 * @brief	Disables the i/o interrupt.
 *
 * @pre		handle must be created with mcu_io_interrupt_init.
 * @pre		handle must be configured with mcu_io_interrupt_set_param.
 *
 * @param handle			I/o interrupt handler.
 */
void mcu_io_interrupt_disable(mcu_io_int_t handle);

/**
 * @brief	Enables the i/o interrupt. Must only be called if MCU_INT_LVL_OFF was set in mcu_io_interrupt_set_param or mcu_io_interrupt_disable
 * 			was called before.
 *
 * @pre		handle must be created with mcu_io_interrupt_init.
 * @pre		handle must be configured with mcu_io_interrupt_set_param.
 *
 * @param handle			I/o interrupt handler.
 */
void mcu_io_interrupt_enable(mcu_io_int_t handle);

/**
 * @brief	Returns the MCU_IO_PIN that was set for this i/o interrupt.
 *
 * @pre		handle must be created with mcu_io_interrupt_init before.
 *
 * @param handle			I/o interrupt handler.
 * @return					MCU_IO_PIN of the i/o interrupt or PIN_NONE if the handle is invalid.
 */
MCU_IO_PIN mcu_io_interrupt_get_pin(mcu_io_int_t handle);

#endif

#endif // __MCU_IO_INTERRUPT_HEADER_FIRST_INCLUDE__
