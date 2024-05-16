/**
 * 	@file 	mcu_ad.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_ad functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *	@subsection	mcu_htu_ad_init AD converter
 *
 *	The ad converter needs to be initialized with the corresponding i/o pin.
@code
	mcu_ad_t my_ad = mcu_ad_init(P0_0);
@endcode
 *
 *	The AD converter offers 3 different modi:
 *		- Manual read
 *		- Interrupt mode
 *		- Freerun mode
 *
 *	In manual read mode you need to start the reading manually. You can then poll mcu_ad_ready to see when the reading is finished and read
 *	it out.
@code
	// Manual read (Interrupt function is NULL and auto_read is false):
	mcu_ad_set_param(my_ad, NULL, MCU_INT_LVL_OFF, MCU_AD_UNSIGNED, 8, false);
	
	mcu_ad_start(my_ad);	// Start reading
	while(!mcu_ad_ready(my_ad));	// Wait for the reading to finish (Can be non-blocking with if).
	uint8_t data = mcu_ad_read(my_ad);	// Read the value (can be 16-bit value if the resolution is > 8-Bit)
@endcode
 *
 *	If you use the interrupt mode you do not need to poll mcu_ad_ready. If the reading is finished your interrupt function is called
 *	automatically. You just need to call mcu_ad_start manually like in manual read.
@code
	void my_ad_read(int value);	// Prototype of your ad read function
	...
	// Interrupt mode (Interrupt function is my_ad_read and auto_read is false):
	mcu_ad_set_param(my_ad, my_ad_read, MCU_INT_LVL_MED, MCU_AD_UNSIGNED, 8, false);
	mcu_ad_start(my_ad);	// Start reading
	// Now your interrupt will be called once the reading finishes.
@endcode
 *
 *	The freerun mode works like manual read in case of reading with the difference that you only need to start the reading once. After this
 *	the ad module will permanently read out ad values.
@code
	// Freerun Mode (Interrupt function is NULL and auto_read is true):
	mcu_ad_set_param(my_ad, NULL, MCU_INT_LVL_OFF, MCU_AD_UNSIGNED, 8, true);
	mcu_ad_start(my_ad);	// Start the reading once
	int data = mcu_ad_read(my_ad);	// Value can be read anytime (For first reading check mcu_ad_ready...)
@endcode
 *
 *  @version    2.00 (20.08.2022)
 *               - Extracted mcu_ad functions in this file. History from now on will only contain infos about mcu_ad.
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

#ifndef __MCU_AD_HEADER_FIRST_INCLUDE__
#define __MCU_AD_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_DEVICE_COUNT_AD>0
/**
 * @brief 	Initializes an AD handler for the given pin.
 *
 *			Following errors might occur:
 * 							MCU_ERROR_AD_NOT_AVAILABLE: 	All AD converters are in use.
 *															The MCU_PERIPHERY_DEVICE_COUNT_AD define in mcu_config.h might be too low. Check it out.
 * 							MCU_ERROR_AD_IO_PIN_INVALID: 	The i/o pin cannot be used for the ad converter.
 *
 * @pre		Check if MCU_PERIPHERY_DEVICE_COUNT_AD in mcu_config.h is high enough.
 *
 * @param pin				I/O Pin of the ad channel.
 * @return					NULL: The AD handler could not be created.
 *							Else: Pointer to the AD Handler.
 */
mcu_ad_t mcu_ad_init(MCU_IO_PIN pin);

/**
 * @brief 	This function is used to set the parameters for the ad converter.
 * 			Be careful, because on some mcus this function sets the parameters not only for one, but globally for all channels.
 * 			The function f is called if an ad converter interrupt occurs after calling mcu_ad_start. f might be NULL if lvl is MCU_INT_LVL_OFF.
 *
 * @pre		The AD handler must be created with mcu_ad_init.
 *
 * @param h					AD handler, that was created with mcu_ad_init.
 * @param f					Callback function for the ad converter interrupt. Has an int parameter for the converted ad value.
 * @param lvl				Level of the interrupt.
 * @param sign				Defines if the converted ad values is signed or unsigned. Check @link MCU_AD_SIGNEDNESS MCU_AD_SIGNEDNESS@endlink. Some mcus only support unsigned.
 * @param bit_resolution	The bit resolution of the ad converter. Typical values are 8, 10, 12 or 16.
 * @param auto_read			true: After calling mcu_ad_start once, the mcu converts the ad value periodically.\n
 * 							false: After calling mcu_ad_start the mcu converts the ad value once.
 * @return 					MCU_OK: No error occured.\n
 * 							MCU_ERROR_AD_INVALID: The AD Handler is NULL\n
 *							MCU_ERROR_AD_INTERRUPT_NOT_SUPPORTED: The AD Interrupt function is not supported.\n
 * 							MCU_ERROR_AD_SIGNEDNESS_INVALID: The signedness value is not supported by the mcu..\n
 *							MCU_ERROR_AD_RESOLUTION_INVALID: The bit resolution is not supported by the mcu.
 */
MCU_RESULT mcu_ad_set_param(mcu_ad_t h, void (*f)(int), MCU_INT_LVL lvl, MCU_AD_SIGNEDNESS sign, uint8_t bit_resolution, bool auto_read);

/**
 * @brief 	Since most mcus can only have one ad channel active at a time you need to use this function to switch between the different channels.
 * 			If you use different parameters for each channel you need to call mcu_ad_set_param everytime after switching the channel. If the parameters for the different channel are the same
 * 			you only need to call this function to change between the channels.
 *
 * @pre		The AD handler must be created with mcu_ad_init.
 *
 * @param h					AD handler, that was created with mcu_ad_init.
 * @return 					MCU_OK: No error occured.\n
 * 							MCU_ERROR_AD_INVALID: The AD Handler is NULL\n
 *							MCU_ERROR_AD_IO_PIN_INVALID: The i/o pin cannot be used for the ad converter. Should not occur, because it is already checked in mcu_ad_init.
 */
MCU_RESULT mcu_ad_set_channel_active(mcu_ad_t h);

/**
 * @brief 	Starts the ad conversion.
 *
 * @pre		The AD handler must be created with mcu_ad_init.
 * @pre		The channel must be set to active by calling mcu_ad_set_channel_active.
 * @pre		The parameters for this channel must be set by calling mcu_ad_set_param. If using different parameters for each channel you need to call mcu_ad_set_param after changing a channel.
 *
 * @param h					AD handler, that was created with mcu_ad_init.
 * @return 					MCU_OK: No error occured.\n
 * 							MCU_ERROR_AD_INVALID: The AD Handler is NULL.
 */
MCU_RESULT mcu_ad_start(mcu_ad_t h);

/**
 * @brief 	Indicates if the converted ad value can be read. This functions can only be used when the ad converter is not set to auto read and no interrupt is used.
 *
 * @pre		The AD handler must be created with mcu_ad_init.
 * @pre		The function mcu_ad_start should be used before to start an ad conversion.
 *
 * @param h					AD handler, that was created with mcu_ad_init.
 * @return 					true: Conversion finished.\n
 * 							false: Conversion in progress.
 */
bool mcu_ad_ready(mcu_ad_t h);

/**
 * @brief 	Reads the ad value.
 *
 * @pre		The AD handler must be created with mcu_ad_init.
 * @pre		The function mcu_ad_start should be used before to start an ad conversion.
 * @pre		Check the function mcu_ad_ready before calling this function to see if the conversion is finished. Not needed in auto read mode.
 *
 * @param h					AD handler, that was created with mcu_ad_init.
 * @return 					Converted ad value or 0 in case of an error.
 */
int mcu_ad_read(mcu_ad_t h);
#endif

#endif // __MCU_AD_HEADER_FIRST_INCLUDE__
