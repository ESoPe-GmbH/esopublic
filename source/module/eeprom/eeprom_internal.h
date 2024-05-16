// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file eeprom_internal.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief	FuInternal definitions and structures for eeproms.
 *			
 *  @version	1.00 (26.1.2022)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_EEPROM_EEPROM_INTERNAL_H_
#define MODULE_EEPROM_EEPROM_INTERNAL_H_

#include "module_public.h"

#if MODULE_ENABLE_EEPROM
#include "module/enum/function_return.h"
#include "eeprom.h"
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

/**
 * Interface for a eeprom device.
 */
struct eeprom_device_interface_s
{
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
	FUNCTION_RETURN (*f_read)(eeprom_device_t device, uint16_t address, void* buffer, uint16_t length);
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
	FUNCTION_RETURN (*f_write)(eeprom_device_t device, uint16_t address, const void* buffer, uint16_t length);
};

/// Interface for a eeprom device
struct eeprom_device_s
{
	/// Interface for erasing/writing/reading eeprom data.
	struct eeprom_device_interface_s interface;
	/// Pointer to the eeprom devices configuration.
	const void* config;
	/// Pointer to the context of the eeprom device. This is used by the eeprom devices for their own calculations and stuff.
	void* context;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


#endif

#endif /* MODULE_EEPROM_EEPROM_INTERNAL_H_ */
