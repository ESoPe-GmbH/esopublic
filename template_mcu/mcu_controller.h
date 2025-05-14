// Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_controller.h
 * 	@copyright Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *  @date		13.05.2025
 *
 *  @brief		Specify MCU_TYPE of the mcu and other MCU specific defines and enumerations.
 *
 *  @version	1.00
 *  			 - Initial release
 *
 ******************************************************************************/

#ifndef MCU_NEW_NAME_MCU_CONTROLLER_HEADER_FIRST_INCLUDE_GUARD
#define MCU_NEW_NAME_MCU_CONTROLLER_HEADER_FIRST_INCLUDE_GUARD

#define MCU_TYPE 					MCU_NEW_NAME

#include "mcu_heap.h"

/// @brief I/Os to use on the mcu
typedef enum
{
	// TODO: Add I/Os

	/// @brief First external pin when using a port expander that is registered to the mcu
	GPIO_EXTERNAL = 1000,
	/// @brief Value to set when no Pin is specified
	PIN_NONE = 0xFFFF
	
} MCU_IO_PIN;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// UART Interfaces
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

// TODO: Specify the parameters of mcu_uart_init for initializing the debug interface
#define MCU_UART_INIT_DEBUG_PARAM		0, PIN_NONE, PIN_NONE

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Flash Type defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Type for flash addresses
#define FLASH_PTR_TYPE	uint32_t
/// Type for buffer addresses
#define BUF_PTR_TYPE   	uint32_t
/// Type for erase addresses
#define ERASE_PTR_TYPE	uint32_t

/// Page size of the flash
#define MCU_CONTROLLER_FLASH_MIN_STEPPING	128

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Flash Block Addresses
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

// TODO: Implement based on memory mapping of the flash

/* Code Flash Block Area: 256 2kB Blocks */
// #define BLOCK(n)	(0x08000000 + ((n) * 0x800))

/* No special dataflash on controller: Use normal Code Flash */
// #define BLOCK_DB(n)	BLOCK(n)

#endif
