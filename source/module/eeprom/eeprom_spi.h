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

#ifndef MODULE_EEPROM_EEPROM_SPI_H_
#define MODULE_EEPROM_EEPROM_SPI_H_

#include "module_public.h"

#if MODULE_ENABLE_EEPROM
#include "module/comm/spi/spi.h"
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

/// Initialization value for eeprom_spi_config_t when using CAT25010 EEPROM.
/// @param spi_interface  Pointer to spi structure to use.
#define EEPROM_SPI_CAT25010(spi_interface)   { .spi = spi_interface, .max_size = 128, .page_size = 16, .frequency = 10000000}

/// Initialization value for eeprom_spi_config_t when using CAT25020 EEPROM.
/// @param spi_interface  Pointer to spi structure to use.
#define EEPROM_SPI_CAT25020(spi_interface)   { .spi = spi_interface, .max_size = 256, .page_size = 16, .frequency = 10000000}

/// Initialization value for eeprom_spi_config_t when using CAT25040 EEPROM.
/// @param spi_interface  Pointer to spi structure to use.
#define EEPROM_SPI_CAT25040(spi_interface)   { .spi = spi_interface, .max_size = 512, .page_size = 16, .frequency = 10000000}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Type for the eeprom spi config structure
typedef struct eeprom_spi_config_s eeprom_spi_config_t;

/// @brief Configuration for the eeprom device.
struct eeprom_spi_config_s
{
    /// @brief Pointer to spi structure to use.
    mcu_spi_t spi;
    /// @brief Clock frequency of SPI to use.
    uint32_t frequency;
    /// @brief Maximum size of the eeprom in bits. Valid values are 1K, 2K, 4K, 8K and 16K.
    uint16_t max_size;
    /// @brief Size of a single page inside the eeprom.
    uint16_t page_size;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates an eeprom device handle for an spi eeprom.
 * 
 * @param config            Configuration for the SPI EEPROM device.
 * @return eeprom_device_t  NULL on invalid configuration or
 */
eeprom_device_t eeprom_spi_init(const eeprom_spi_config_t* config);
/**
 * @brief Free the device handle for the spi eeprom.
 * 
 * @param device            Device handle that was created using `eeprom_spi_init`.
 */
void eeprom_spi_free(eeprom_device_t device);
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
FUNCTION_RETURN eeprom_spi_read(eeprom_device_t device, uint16_t address, void* buffer, uint16_t length);
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
FUNCTION_RETURN eeprom_spi_write(eeprom_device_t device, uint16_t address, const void* buffer, uint16_t length);

#endif

#endif /* MODULE_EEPROM_EEPROM_SPI_H_ */
