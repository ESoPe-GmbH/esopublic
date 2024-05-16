/**
 * 	@file 	mcu_da.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_da functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *	@subsection	mcu_htu_da_init DA converter
 *
 *	The da converter has only 3 functions. Initialization, setting parameters and setting an output value.
 *	First initialize it like the ad converter with the pin used for da conversion (example below).
@code
	mcu_da_t da_handle;
	...
	da_handle = mcu_da_init(P9_3);
@endcode
 *
 * 	Now that the da_handle is initializes you can set the parameters (only the resolution at the moment).
@code
	if(MCU_OK == mcu_da_set_param(da_handle, 8))
		// ...
@endcode
 *
 * 	Last step is setting the da converter output value. The output voltage must be calculated with the help of the reference voltage.
@code
	mcu_da_set_value(da_handle, 0xd2);
		// ...
@endcode
 *
 *  @version    2.00 (20.08.2022)
 *               - Extracted mcu_da functions in this file. History from now on will only contain infos about mcu_da.
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

#ifndef __MCU_DA_HEADER_FIRST_INCLUDE__
#define __MCU_DA_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_DEVICE_COUNT_DA>0
/**
 * @brief 	Initializes a DA handler for the given pin.
 *
 *			Following errors might occur:
 * 							MCU_ERROR_DA_NOT_AVAILABLE: 	All DA converters are in use.
 *															The MCU_PERIPHERY_DEVICE_COUNT_DA define in mcu_config.h might be too low. Check it out.
 * 							MCU_ERROR_DA_IO_PIN_INVALID: 	The i/o pin cannot be used for the da converter.
 *
 * @pre		Check if MCU_PERIPHERY_DEVICE_COUNT_DA in mcu_config.h is high enough.
 *
 * @param pin				IO Pin of the da channel.
 * @return					NULL: The da handler could not be created.
 *							Else: Pointer to the DA Handler.
 */
mcu_da_t mcu_da_init(MCU_IO_PIN pin);

/**
 * @brief 	Sets the bit resolution for the da converter.
 *
 * @pre		The DA handler must be created with mcu_da_init.
 *
 * @param h					DA handler, that was created with mcu_da_init.
 * @param bit_resolution	Bit resolution of the da module. Typical values are 8 or 10.
 * @return 					MCU_OK: No error occurred.\n
 * 							MCU_ERROR_DA_INVALID: The DA Handler is NULL\n
 *							MCU_ERROR_DA_RESOLUTION_INVALID: The bit resolution is not supported.
 */
MCU_RESULT mcu_da_set_param(mcu_da_t h, uint8_t bit_resolution);

/**
 * @brief 	Sets the converted da value.
 *
 * @pre		The DA handler must be created with mcu_da_init.
 * @pre		mcu_da_set_param must be used beforehand to set the bit resolution.
 *
 * @param h					DA handler, that was created with mcu_da_init.
 * @param val				The value that is converted and then outputted on the da pin.
 * @return 					MCU_OK: No error occurred.\n
 * 							MCU_ERROR_DA_INVALID: The DA Handler is NULL.
 */
MCU_RESULT mcu_da_set_value(mcu_da_t h, uint16_t val);

#endif

#endif // __MCU_DA_HEADER_FIRST_INCLUDE__
