/**
 * 	@file 	mcu_flash.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_flash functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *	@subsection	mcu_htu_flash_init Flash functions
 *
 *	The flash module can be used to store data inside the data and program flash. There are no functions for reading because you can do this
 *	simply with pointers. Before using the write / erase functions you need to initialize the flash module (important for some controllers).
@code
	mcu_flash_init();	// Thats all
@endcode
 *
 *	After initialization you can use the erase and write functions. It is important that you erase a flash block before writing it because
 *	it does not work to rewrite bytes without erasing the whole block. Block definitions for your microcontroller can be found inside the
 *	mcu_controller.h. Important is that the block defines for some controllers point to the end of the block while the same defines for
 *	another controller point to the beginning of the block. The reason is that these block defines can be used for the erase function.
 *	For writing you should calculate the block start address!
 *
 *	You also need to know the MCU_CONTROLLER_FLASH_MIN_STEPPING inside mcu_controller.h because it defines how many bytes you need to
 *	write at once. For example the RX62N has a minimum stepping of 256 bytes. This means if you write 128 byte of data at once inside
 *	BLOCK_0 you can not write another 128 Byte to address (BLOCK_0 + 128). The next data you can write is at address (BLOCK_0 + 256).
 *
 *	Controllers like the R32C have a minimum stepping of 8 byte but there are also controllers which have a stepping of 1 byte.
 *
 *	Another important notice is that on some controllers the mcu_flash_erase and mcu_flash_write functions are stored inside the ram and
 *	you need to declare sections for them inside the compiler settings.
 *
 *	The example below shows how to Erase and Write into the flash. The example writes to the program flash but it can also write to the data
 *	flash.
@code
	uint8_t some_buffer_with_your_data[256];
	//...
	mcu_flash_erase(BLOCK_0); // Erases a block in programm flash
	mcu_flash_write(BLOCK_0, some_buffer_with_your_data, sizeof(some_buffer_with_your_data));
	// Writes data to block 0 (if BLOCK_0 is the start address!)
@endcode
 *
 *  @version    2.00 (20.08.2022)
 *               - Extracted mcu_flash functions in this file. History from now on will only contain infos about mcu_flash
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

#ifndef __MCU_FLASH_HEADER_FIRST_INCLUDE__
#define __MCU_FLASH_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_ENABLE_FLASH
/**
 * @brief 	Initializes the mcu flash routine. Needed on some mcus to copy the flash functions into ram.
 *
 */
void mcu_flash_init(void);

/**
 * @brief 	Erases a block in flash memory or data flash memory. Check mcu_controller.h or the manual of the mcu to see which blocks are available.
 *
 *  @param flash_addr			Address of the block that will be erased.
 */
bool mcu_flash_erase(ERASE_PTR_TYPE  flash_addr);

/**
 * @brief 	Writes data to the address inside the flash. Make sure that the area is erased before writing inside it.
 * 			You should also check MCU_CONTROLLER_FLASH_MIN_STEPPING to see how many bytes should be written at once.
 *
 * @attention	If MCU_CONTROLLER_FLASH_MIN_STEPPING is 8 for example you can only write to addresses which can be divided by 8.
 *
 *  @param flash_addr			Address inside flash memory or data flash memory where the data should be written.
 *  @param buffer_addr			Pointer to the data that should be written.
 *  @param bytes				Number of bytes that should be written.
 */
bool mcu_flash_write(FLASH_PTR_TYPE flash_addr, BUF_PTR_TYPE buffer_addr, uint32_t bytes);

/**
 * @brief 	Reads data from the address inside the flash and writes it into the buffer address.
 *
 *  @param flash_addr			Address inside flash memory or data flash memory where the data should be read.
 *  @param buffer_addr			Pointer to the buffer where the read data will be stored.
 *  @param bytes				Number of bytes that should be read.
 */
bool mcu_flash_read(FLASH_PTR_TYPE flash_addr, BUF_PTR_TYPE buffer_addr, uint32_t bytes);

#endif

#endif // __MCU_FLASH_HEADER_FIRST_INCLUDE__
