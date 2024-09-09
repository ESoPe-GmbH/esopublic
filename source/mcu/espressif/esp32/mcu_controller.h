// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_controller.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *  @date		05.09.2011
 *
 *  @brief
 *
 *  @version	1.00
 *  			 - Erste Version
 *
 ******************************************************************************/

#ifndef MCU_ESP32_MCU_CONTROLLER_HEADER_FIRST_INCLUDE_GUARD
#define MCU_ESP32_MCU_CONTROLLER_HEADER_FIRST_INCLUDE_GUARD

#define MCU_TYPE 					MCU_ESP32

#include "mcu_heap.h"

#define MCU_RTC_ALLOWS_FRACTIONS		1

// CPU IO Pinne
typedef enum
{
	GPIO0  = 0x0000,
	GPIO1  = 0x0001,
	GPIO2  = 0x0002,
	GPIO3  = 0x0003,
	GPIO4  = 0x0004,
	GPIO5  = 0x0005,
	GPIO6  = 0x0006,
	GPIO7  = 0x0007,
	GPIO8  = 0x0008,
	GPIO9  = 0x0009,
	GPIO10 = 0x000A,
	GPIO11 = 0x000B,
	GPIO12 = 0x000C,
	GPIO13 = 0x000D,
	GPIO14 = 0x000E,
	GPIO15 = 0x000F,
	GPIO16 = 0x0010,
	GPIO17 = 0x0011,
	GPIO18 = 0x0012,
	GPIO19 = 0x0013,
#if CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32P4
	GPIO20 = 0x0014,
#endif
	GPIO21 = 0x0015,
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32P4
	GPIO22 = 0x0016,
	GPIO23 = 0x0017,
	GPIO24 = 0x0018,
	GPIO25 = 0x0019,
	GPIO26 = 0x001A,
	GPIO27 = 0x001B,
#if CONFIG_IDF_TARGET_ESP32P4
	GPIO28 = 28,
	GPIO29 = 29,
	GPIO30 = 30,
	GPIO31 = 31,
#endif
	GPIO32 = 0x0020,
	GPIO33 = 0x0021,
	GPIO34 = 0x0022,
#endif
	GPIO35 = 0x0023,
	GPIO36 = 0x0024,
	GPIO37 = 0x0025,
	GPIO38 = 0x0026,
	GPIO39 = 0x0027,

#if CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32P4
	GPIO40 = 40,
	GPIO41 = 41,
	GPIO42 = 42,
	GPIO43 = 43,
	GPIO44 = 44,
	GPIO45 = 45,
	GPIO46 = 46,
	GPIO47 = 47,
	GPIO48 = 48,
#endif

#if CONFIG_IDF_TARGET_ESP32P4
	GPIO49 = 49,
	GPIO50 = 50,
	GPIO51 = 51,
	GPIO52 = 52,
	GPIO53 = 53,
	GPIO54 = 54,
	GPIO55 = 55,
	GPIO56 = 56,
#endif

	GPIO_EXTERNAL = 100,

	PIN_NONE = 0xFFFF
	
} MCU_IO_PIN;


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// UART Interfaces
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if CONFIG_IDF_TARGET_ESP32S3
	#define MCU_UART_INIT_DEBUG_PARAM		0, GPIO43, GPIO44
#elif CONFIG_IDF_TARGET_ESP32P4
	#define MCU_UART_INIT_DEBUG_PARAM		0, GPIO37, GPIO38
#elif CONFIG_IDF_TARGET_ESP32
	#define MCU_UART_INIT_DEBUG_PARAM		0, GPIO1, GPIO3
#else
	#error "Set the Debug UART Parameter in MCU_UART_INIT_DEBUG_PARAM"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// SPI Interfaces
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Flash Type defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define FLASH_PTR_TYPE	uint32_t
#define BUF_PTR_TYPE   	uint32_t
#define ERASE_PTR_TYPE	uint32_t

#define MCU_CONTROLLER_FLASH_MIN_STEPPING	128

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Flash Block Addresses
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/* Code Flash Block Area: 256 2kB Blocks */
#define BLOCK(n)	(0x08000000 + ((n) * 0x800))

/* No special dataflash on controller: Use normal Code Flash */
#define BLOCK_DB(n)	BLOCK(n)

#endif
