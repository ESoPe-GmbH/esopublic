// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file eeprom.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief	Functions for handling an external SPI-Flash.
 *			
 *  @version	1.00 (26.1.2022)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_EEPROM_EEPROM_H_
#define MODULE_EEPROM_EEPROM_H_

#include "module_public.h"

#if MODULE_ENABLE_EEPROM
#include "module/enum/function_return.h"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the eeprom module
#define EEPROM_STR_VERSION "1.00"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief  Pointer to the eeprom device config. The structure to use for it can be found inside the respective eeprom_<interface>.h file.
typedef void* eeprom_device_config_t;

/// Type for the handle of an eeprom device
typedef struct eeprom_device_s* eeprom_device_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Read data from the eeprom.
 * @param address			Address of the eeprom area to read from.
 * 							The address starts at 0 for an eeprom device.
 * @param buffer			Pointer to the buffer where the read data should be put into.
 * @param length			Number of bytes to read.
 * @retval FUNCTION_RETURN_OK				All bytes were read and stored into buf.
 * @retval FUNCTION_RETURN_PARAM_ERROR		One or more parameter are invalid (Null-Pointer or invalid address).
 * @retval FUNCTION_RETURN_NOT_READY		The eeprom could not be initialized during initialization and is therefore not accessed anymore.
 */
FUNCTION_RETURN eeprom_read(eeprom_device_t device, uint16_t address, void* buffer, uint16_t length);
/**
 * Writes data to the eeprom.
 * @param address			Address of the eeprom area to write into.
 * 							The address starts at 0 for an eeprom device.
 * @param buffer			Pointer to the buffer where the data to write is stored.
 * @param length			Number of bytes to write.
 * @retval FUNCTION_RETURN_OK				All bytes were written.
 * @retval FUNCTION_RETURN_PARAM_ERROR		One or more parameter are invalid (Null-Pointer or invalid address).
 * @retval FUNCTION_RETURN_NOT_SUPPORTED	Is triggered when the write is not possible to the EEPROM.
 */
FUNCTION_RETURN eeprom_write(eeprom_device_t device, uint16_t address, const void* buffer, uint16_t length);

#endif

#endif /* MODULE_EEPROM_EEPROM_H_ */
