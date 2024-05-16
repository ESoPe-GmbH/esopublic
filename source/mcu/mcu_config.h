// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 	mcu_config.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  	Contains Defines for the configuration of the MCU module. The file is divided in two parts. In the first part you can
 *  	configure which features of the MCU you are going to use. In the second path you need to set which MCU you are using.
 *
 *  	Part 1: Features
 *
 *  	The MCU_PERIPHERY_ENABLE_ defines can enable or disable a peripheral unit like the flash or a watchdog. These
 *  	units only have one instance unlike UARTs or SPI interfaces which might contain more than one instance.
 *
 *  	MCU_PERIPHERY_DEVICE_COUNT_ defines disable a peripheral unit when set to 0. If the value is greater than zero
 *  	it declares how many instances of this unit will be statically allocated in the memory.
 *
 *  	If using the comm module the MCU offers functions in the UART and the CAN Module for creating interfaces so you
 *  	only need to call the comm_create method to initialize your comm handler to your UART handler. You can enable these
 *  	functions with the MCU_PERIPHERY_ENABLE_COMM_MODE_ define.
 *
 *  	Part 2: MCU configuration
 *
 *  	In this part of the file you have to configure the MCU type you are using. First you need to define the pin count of
 *  	the MCU with the MCU_CONTROLLER_PIN_COUNT define. The next step is the ROM size of the MCU, which is needed for flash
 *  	operations and such. You can define the ROM size with the MCU_CONTROLLER_ROM_SIZE_KBYTE define.
 *
 *  	In the next part are two lists for all processors which are supports by the MCU module. You only need them if you
 *  	want to add a new MCU type.
 *
 *  	The last configuration is at the bottom of the file. You need to include the mcu_controller.h of the MCU you are
 *  	using. The path is the same as the paths in the mcu_internal.h list with the exception that you need to include the
 *  	mcu_controller.h and not the mcu_internal.h. The latter is included in the mcu.c for the implementation of debugging
 *  	features.
 *
 *  @par 	Processors:
 *  			- Renesas:
 *  				- R32C 118 	(100-Pin)
 *  				- R32C 111 	(64-Pin and 100-Pin)
 *  				- RL78/G13	(64-Pin)
 *  				- RX62 		(100-Pin)
 *  				- RX63		(100-Pin and 176-Pin)
 *  				- RX210		(64-Pin)
 *  				- RX111		(64-Pin)
 *  				- RX71M		(100-Pin)
 *                  - RX231     (48-Pin)
 *                  - Synergy   (100-Pin, 144-Pin, 176-Pin)
*               - Espressif
*               	- ESP32
*              	- ST
*              		- STM32F3
*              		- STM32F4
*              		- STM32F7
*              	- Silicon Labs
*              		- EFM32GG11
*              		- EFM32PG12
 *
 *	@version	1.14 (02.12.2022)
 *	@version	1.13 (19.01.2022)
 *				 - Made mcu_config compatible to ESoPeKernel and include mcu_selection for project specific configuration.
 *	@version	1.12 (27.12.2019)
 *				 - Added PC
 *				 - Added ESP32 (Espressif)
 *				 - Added STM32Fx (ST)
 *				 - Added EFM32 (Silicon Labs)
 *  @version    1.11 (16.03.2019)
 *               - Added Synergy
 *	@version	1.10 (27.11.2018)
 *				 - Added RX231
 *	@version	1.09 (16.08.2017)
 *				 - Added RX71M
 *	@version	1.08 (10.10.2016)
 *				 - Added I2C
 *				 - Added RX111
 *				 - Changed comments to before the define
 *  @version	1.07 (23.01.2015)
 *  			 - Shortened folder names and defines of mcus. -> Old defines contained flash functionality or pin size.
 *  			 - Added RX210
 *  @version	1.06 (13.09.2014)
 *				 - Added MCU_PERIPHERY_ENABLE_RTC
 *  @version	1.05 (19.08.2013)
 *				 - Added RL78/G13
 *				 - Added RX63N
 *  @version	1.04 (31.12.2011)
 *				 - MCU_CONTROLLER_ROM_SIZE_KBYTE added.
 *				 - Translated the comments to English because of compiler problems with umlauts.
 *  @version	1.03 (24.09.2011)
 *				 - MCU_PERIPHERY_ENABLE_ added for the Flash.
 *				 - MCU_PERIPHERY_DEVICE_COUNT_ added for CAN.
 *				 - MCU_CONTROLLER_PIN_COUNT added.
 *  @version	1.02 (22.09.2011)
 *  			 - deleted MCU_PERIPHERY_ENABLE_ defines for UART and other peripherals which only need the
 *  				MCU_PERIPHERY_DEVICE_COUNT_ define.
 *				 - added the true and false definitions because they are needed for the definitions below
 *  @version	1.01 (20.09.2011)
 *				 - Paths changed to mcu_internal.h
 *				 - Debug enable added.
 *  @version	1.00 (12.09.2011)
 *  			 - configuration separated from the mcu.h
 *
 **/
#ifndef MCU_CONFIG_HEADER_FIRST_INCLUDE_GUARD
#define MCU_CONFIG_HEADER_FIRST_INCLUDE_GUARD

#ifndef true
	/// Define true if it does not exist
	#define true 1		
#endif

#ifndef false
	/// Define false if it does not exist
	#define false 0		
#endif

// Supported processors are set into the MCU_TYPE define in the mcu_controller.h
/// MCU Type: Renesas R32C/111
#define R5F6411						1									
/// MCU Type: Renesas R32C/118
#define R5F6418						2									
/// MCU Type: Renesas R8C/23
#define R5F2123						3									
/// MCU Type: Renesas R8C/1B
#define R5F211B						4									
/// MCU Type: Renesas RX621 / RX62N
#define R5F562						5									
/// MCU Type: Renesas RX630 / RX631 / RX63N
#define R5F563						6									
/// MCU Type: Renesas RL78/G13
#define R5F100						7									
/// MCU Type: Renesas RX210 / RX21A
#define R5F521						8									
/// MCU Type: Renesas RX111
#define R5F5111						9									
/// MCU Type: Renesas RX71M
#define R5F571						10
/// MCU Type: Renesas RX231
#define R5F523						11
/// MCU Type: Renesas Synergy Series
#define RSYNERGY                    12
/// MCU Type: Espressif ESP32
#define MCU_ESP32					13
/// MCU Type: ST STM32F3
#define MCU_STM32F3					14
/// MCU Type: ST STM32F4
#define MCU_STM32F4					15
/// MCU Type: ST STM32F7
#define MCU_STM32F7					16
/// MCU Type: Silicon Labs Giant Gecko
#define MCU_EFM32GG11				17
/// MCU Type: Silicon Labs Jade Gecko
#define MCU_EFM32PG12				18
/// MCU Type: ST STM32F0
#define MCU_STM32F0					19
/// MCU Type: ST STM32F0
#define MCU_STM32G0					20
/// MCU Type: Renesas RX140
#define R5F514						21
/// MCU Type: PC (needed for testing applications on a pc)
#define PC_EMU    					100

// Paths to the mcu_internal files for the internal debugging.
/// MCU Path: Renesas R32C/111
#define PATH_R5F6411		"renesas/r5f6411/mcu_internal.h"			
/// MCU Path: Renesas R32C/118
#define PATH_R5F6418		"renesas/r5f6418/mcu_internal.h"			
/// MCU Path: Renesas R8C/23
#define PATH_R5F2123		"renesas/r5f2123/mcu_internal.h"			
/// MCU Path: Renesas R8C/1B
#define PATH_R5F211B		"renesas/r5f211b/mcu_internal.h"			
/// MCU Path: Renesas RX621 / RX62N
#define PATH_R5F562			"renesas/r5f562/mcu_internal.h"				
/// MCU Path: Renesas RX630 / RX631 / RX63N
#define PATH_R5F563			"renesas/r5f563/mcu_internal.h"				
/// MCU Path: Renesas RL78/G13
#define PATH_R5F100			"renesas/r5f100/mcu_internal.h"				
/// MCU Path: Renesas RX210 / RX21A
#define PATH_R5F521			"renesas/r5f521/mcu_internal.h"				
/// MCU Path: Renesas RX210 / RX21A
#define PATH_R5F5111		"renesas/r5f5111/mcu_internal.h"
/// MCU Path: Renesas RX71M
#define PATH_R5F571			"renesas/r5f571/mcu_internal.h"
/// MCU Path: Renesas RX231
#define PATH_R5F523			"renesas/r5f523/mcu_internal.h"
/// MCU Path: Renesas Synergy
#define PATH_RSYNERGY       "renesas/synergy/mcu_internal.h"
/// MCU Path: Espressif ESP32
#define PATH_ESP32			"espressif/esp32/mcu_internal.h"
/// MCU Path: ST STM32F3
#define PATH_STM32F3		"st/stm32f3/mcu_internal.h"
/// MCU Path: ST STM32F4
#define PATH_STM32F4		"st/stm32f4/mcu_internal.h"
/// MCU Path: ST STM32F7
#define PATH_STM32F7		"st/stm32f7/mcu_internal.h"
/// MCU Path: Silicon Labs Giant Gecko
#define PATH_EFM32GG11		"sl/efm32gg11/mcu_internal.h"
/// MCU Path: Silicon Labs Jade Gecko
#define PATH_EFM32PG12		"sl/efm32pg12/mcu_internal.h"
/// MCU Path: ST STM32F0
#define PATH_STM32F0		"st/stm32f0/mcu_internal.h"
/// MCU Path: ST STM32G0
#define PATH_STM32G0		"st/stm32g0/mcu_internal.h"
/// MCU Path: Renesas RX140
#define PATH_R5F514			"renesas/r5f514/mcu_internal.h"
/// MCU Path: PC
#define PATH_PC     		"pc/mcu_internal.h"

// SELECT THE MCU USING THIS INCLUDE!
#include "mcu_selection.h"

/// Enables flash functionality in general if code flash, data flash or both flash functions are needed.
#define MCU_PERIPHERY_ENABLE_FLASH					(MCU_PERIPHERY_ENABLE_CODE_FLASH || MCU_PERIPHERY_ENABLE_DATA_FLASH)

#endif
