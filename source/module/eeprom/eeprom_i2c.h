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

#ifndef MODULE_EEPROM_EEPROM_I2C_H_
#define MODULE_EEPROM_EEPROM_I2C_H_

#include "module_public.h"

#if MODULE_ENABLE_EEPROM
#include "module/comm/i2c/i2c.h"
#include "module/enum/function_return.h"
#include "eeprom.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the eeprom module
#define EEPROM_STR_VERSION "1.00"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Initialization value for eeprom_i2c_config_t when using M24C01 EEPROM.
/// @param i2c_interface  Pointer to i2c structure to use.
/// @param e0   Level of E0 pin
/// @param e1   Level of E1 pin
/// @param e2   Level of E2 pin
#define EEPROM_I2C_M24C01(i2c_interface, e0, e1, e2)   { .i2c = i2c_interface, .a0 = e0, .a1 = e1, .a2 = e2, .max_size = 1024, .page_size = 16}

/// Initialization value for eeprom_i2c_config_t when using M24C02 EEPROM.
/// @param i2c_interface  Pointer to i2c structure to use.
/// @param e0   Level of E0 pin
/// @param e1   Level of E1 pin
/// @param e2   Level of E2 pin
#define EEPROM_I2C_M24C02(i2c_interface, e0, e1, e2)   { .i2c = i2c_interface, .a0 = e0, .a1 = e1, .a2 = e2, .max_size = 2048, .page_size = 16}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Type for the eeprom i2c config structure
typedef struct eeprom_i2c_config_s eeprom_i2c_config_t;

/// @brief Configuration for the eeprom device.
struct eeprom_i2c_config_s
{
    /// @brief Pointer to i2c structure to use.
    i2c_t* i2c;
    /// @brief Setting of address pin 0 (Only available for eeproms with 1K or 2K)
    uint8_t a0;
    /// @brief Setting of address pin 1 (Only available for eeproms with 1K, 2K or 4K)
    uint8_t a1;
    /// @brief Setting of address pin 2 (Only available for eeproms with 1K, 2K, 4K or 8K)
    uint8_t a2;
    /// @brief Maximum size of the eeprom in bits. Valid values are 1K, 2K, 4K, 8K and 16K.
    uint16_t max_size;
    /// @brief Size of a single page inside the eeprom.
    uint16_t page_size;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates an eeprom device handle for an i2c eeprom.
 * 
 * @param config            Configuration for the I2C EEPROM device.
 * @return eeprom_device_t  NULL on invalid configuration or
 */
eeprom_device_t eeprom_i2c_init(const eeprom_i2c_config_t* config);
/**
 * @brief Free the device handle for the i2c eeprom.
 * 
 * @param device            Device handle that was created using `eeprom_i2c_init`.
 */
void eeprom_i2c_free(eeprom_device_t device);
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
FUNCTION_RETURN eeprom_i2c_read(eeprom_device_t device, uint16_t address, void* buffer, uint16_t length);
/**
 * Writes data to the eeprom. When writing
 * @param address			Address of the eeprom area to write into.
 * 							The address starts at 0 for an eeprom device.
 * @param buffer			Pointer to the buffer where the data to write is stored.
 * @param length			Number of bytes to write.
 * @retval FUNCTION_RETURN_OK				All bytes were written.
 * @retval FUNCTION_RETURN_PARAM_ERROR		One or more parameter are invalid (Null-Pointer or invalid address).
 * @retval FUNCTION_RETURN_NOT_SUPPORTED	Is triggered when the write is not possible to the EEPROM.
 */
FUNCTION_RETURN eeprom_i2c_write(eeprom_device_t device, uint16_t address, const void* buffer, uint16_t length);

#endif

#endif /* MODULE_EEPROM_EEPROM_I2C_H_ */
