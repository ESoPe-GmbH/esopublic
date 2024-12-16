/**
 * 	@file 		flash_info.h
 * 	@copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten
 *  @author 	Tim Koczwara
 *
 *  @brief		Is used for storing hardware id, testing date and custom data.
 *
 *	@version	1.09 (23.01.2023)
 *		- Added flash_info_pc.c
 *	@version	1.08 (02.12.2022)
 *		- R5F514 added
 *	@version	1.07 (19.01.2022)
 * 	    - Modified to be used in esopekernel
 *	@version	1.06 (30.11.2018)
 *		- R5F231 added
 *		- Verification added in flash_info_save
 *		- Used new debug print functions
 *	@version	1.05 (07.06.2018)
 *		- Added module.h support
 *	@version	1.04 (23.08.2017)
 *		- R5F571 added -> Uses multiple blocks
 *		- Added MAC address
 *	@version	1.03 (18.09.2014)
 * 		- FLASH_USE_CUSTOM_DATA and FLASH_CUSTOM_DATA_READ_ONLY added. Functions changed according to these defines.
 * 		- R5F562N8 changed to R5F562N
 * 		- R5F563N added
 *	@version	1.02 (08.05.2013)
 *		- Added Flash info blocks in dependency of MCU_TYPE
 *		- Added FLASH_INFO_DEBUG
 *		- Removed a bug inside flash_info.c where the crc of custom data check was always wrong
 *  @version 	1.01 (20.02.2013)
 *  	- Added comments
 *  @version	1.00 (15.05.2012)
 *  	- Intial release
 *
 ******************************************************************************/

#ifndef FLASH_H_
#define FLASH_H_

#include "module_public.h"

#if MODULE_ENABLE_FLASH_INFO && (MCU_PERIPHERY_ENABLE_DATA_FLASH || MCU_TYPE == MCU_ESP32 || MCU_TYPE == PC_EMU)
//------------------------------------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------------------------------------
/// Version of the flash_info module
#define FLASH_INFO_STR_VERSION "1.09"

#if !defined(FLASH_INFO_BLOCK) && !defined(FLASH_INFO_BLOCK_START_ADDRESS)
#if MCU_TYPE == R5F562
	#define FLASH_INFO_BLOCK						BLOCK_DB0					///< Block definition for mcu_flash_erase
	#define FLASH_INFO_BLOCK_START_ADDRESS			(BLOCK_DB0 & 0xFFFFF800)	///< Block start adress for mcu_flash_write
#elif MCU_TYPE == R5F6411 || MCU_TYPE == R5F6411 || MCU_TYPE == R5F6418
	#define FLASH_INFO_BLOCK						BLOCK_B						///< Block definition for mcu_flash_erase
	#define FLASH_INFO_BLOCK_START_ADDRESS			(BLOCK_B & 0xFFFFF000)		///< Block start adress for mcu_flash_write
#elif MCU_TYPE == R5F100 || MCU_TYPE == R5F521 || MCU_TYPE == R5F571 || MCU_TYPE ==R5F523 || MCU_TYPE ==R5F514 || MCU_TYPE == R5F563 || MCU_TYPE == RSYNERGY
	#define FLASH_INFO_BLOCK						BLOCK_DB(0)					///< Block definition for mcu_flash_erase
	#define FLASH_INFO_BLOCK_START_ADDRESS			BLOCK_DB(0)					///< Block start adress for mcu_flash_write
#elif MCU_TYPE == MCU_STM32F7 || MCU_TYPE == MCU_STM32F4 || MCU_TYPE == MCU_STM32F3 || MCU_TYPE == MCU_STM32F0
	#define FLASH_INFO_BLOCK						BLOCK(1)					///< Block definition for mcu_flash_erase
	#define FLASH_INFO_BLOCK_START_ADDRESS			BLOCK(1)					///< Block start adress for mcu_flash_write
#endif			
#endif

/// Version of this protocol. Stored in flash. Can be used to detect compability problems.
#define FLASH_INFO_VERSION						2	

//------------------------------------------------------------------------------------------------------------
// External functions
//------------------------------------------------------------------------------------------------------------

/**
 * @brief	Initializes variables and tries to load the stored information from the flash.
 *
 * If there is no information stored in the rom hardware id, test date and custom data will be 0.
 */
void flash_info_init(void);

/**
 * @brief	Saves Hardware id, testing date and custom data into the flash.
 *
 * Deletes the flash block FLASH_INFO_BLOCK and writes the new data into it.
 *
 * @return		true when flash info was saved successfully
 * 				false when flash info could not be saved.
 */
bool flash_info_save(void);

///	@return		Returns the flash_info version stored in the flash.
uint8_t flash_info_get_flash_info_version(void);

///	@return		Returns the hardware id stored in the flash.
uint32_t flash_info_get_hardware_id(void);

///	@return	Returns the year of the testing date stored in the flash.
uint16_t flash_info_get_tested_year(void);

///	@return	Returns the month of the testing date stored in the flash.
uint8_t flash_info_get_tested_month(void);

///	@return	Returns the day of the testing date stored in the flash.
uint8_t flash_info_get_tested_day(void);

///	@return	Returns the hour of the testing date stored in the flash.
uint8_t flash_info_get_tested_hour(void);

///	@return	Returns the minute of the testing date stored in the flash.
uint8_t flash_info_get_tested_minute(void);

#if FLASH_USE_CUSTOM_DATA

/**
 * 	@brief	This function returns a pointer to the custom data buffer that is stored and loaded from the ram.
 * 			Use the pointer to read and write your own data. By calling flash_info_save this data is stored into the
 * 			flash.
 *
 * 			Reserves 4 Byte in the custom data.
 *
 * 	@return	Returns a pointer to 4 bytes of custom data.
 */
uint32_t* flash_info_get_4_byte_ptr(void);


/**
 * 	@brief	This function returns a pointer to the custom data buffer that is stored and loaded from the ram.
 * 			Use the pointer to read and write your own data. By calling flash_info_save this data is stored into the
 * 			flash.
 *
 * 			Reserves 2 Byte in the custom data.
 *
 * 	@return				Returns a pointer to 2 bytes of custom data.
 */
uint16_t* flash_info_get_2_byte_ptr(void);


/**
 * 	@brief	This function returns a pointer to the custom data buffer that is stored and loaded from the ram.
 * 			Use the pointer to read and write your own data. By calling flash_info_save this data is stored into the
 * 			flash.
 *
 * 			Reserves a number of Bytes, according to size, in the custom data.
 *
 * 	@param size			Number of bytes that are reserved in the custom data.
 * 	@return				Returns a pointer to a number of size bytes of custom data.
 */
uint8_t* flash_info_get_byte_array(uint16_t size);

#endif

/**
 *	@brief	Sets the hardware id into the flash info data array.
 *			To save it into the flash you must call flash_info_save after this.
 *
 *	@param id			4-Byte hardware id of this device.
 */
void flash_info_set_hardware_id(uint32_t id);


/**
 *	@brief	Sets the testing date into the flash info data array.
 *			To save it into the flash you must call flash_info_save after this.
 *
 *	@param year			Year of the testing date.
 *	@param month		Month of the testing date.
 *	@param day			Day of the testing date.
 *	@param hour			Hour of the testing date.
 *	@param minute		Minute of the testing date.
 */
void flash_info_set_tested_date(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute);

/**
 *	@brief	Sets the mac address into the flash info data array.
 *			To save it into the flash you must call flash_info_save after this.
 *
 *	@param mac	address to set
 */
void flash_info_set_mac_address(uint8_t* mac);

///	brief	Copies the mac address stored in the flash.
void flash_info_get_mac_address(uint8_t* mac);
#endif // #if MCU_PERIPHERY_ENABLE_FLASH

#endif /* FLASH_H_ */
