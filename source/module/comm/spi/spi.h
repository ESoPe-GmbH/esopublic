/**
 * 	@file spi.h
 *  @copyright Urheberrecht 2021-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Felix Herrmann
 *
 *  @brief Bitbanging SPI
 *			
 *
 *  @version	1.00 (09.12.2021)
 *  	- Intial release
 *
 *	@par 	References
 *  	@see	[1] http://de.wikipedia.org/wiki/Serial_Peripheral_Interface from 06.08.2011
 ******************************************************************************/

#ifndef MODULE_COMM_SPI_SPI_H_
#define MODULE_COMM_SPI_SPI_H_

#include "module_public.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Hardware structure for SPI.
 * 
 * Can be used to either abstract from the mcu spi or to perform a bitbanging SPI.
 * To use the mcu spi, set the device member to the result from `mcu_spi_init` or `mcu_spi_create`.
 * To use bitbanging, set the device member to NULL and set the mosi, miso, clk, cs pins manually.
 *  
 */
typedef struct
{
	/// Device handler for SPI.
	mcu_spi_t device;
	/// MOSI pin used for bitbanging
	MCU_IO_PIN mosi;
	/// MISO pin used for bitbanging
	MCU_IO_PIN miso;
	/// CLK pin used for bitbanging
	MCU_IO_PIN clk;
	/// CS pin used for bitbanging -> IDLE High
	MCU_IO_PIN cs;

	// If set to true the CS is managed externally and will not be set by the driver
	bool chip_select_external;

	/// SPI mode to use. Use `spi_set_param` to manipulate it after initialization.
	MCU_SPI_MODE_T mode;
	/// Frequency for the SPI clock in Hz. Use `spi_set_param` or `spi_set_clock` to manipulate it after initialization.
	uint32_t frq;
}spi_hw_t;
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initialize the SPI to use. Set the members of the spi_hw_t before calling the function.
 * 
 * If device should be used for mcu spi, you need to call mcu_spi_init before calling spi_init.
 * 
 * @param hw 			Pointer to the SPI data structure.
 */
void spi_init(spi_hw_t* hw);
/**
 * @brief 	Sets the parameters for the SPI handler.
 * 
 * Since the SPI clock cannot be set freely, because it depends on the mcu clock, the frq value is the maximum value that could be set.
 * If the frq value cannot be set exactly, the next possible lower value for frq is set for the clock.
 * 
 * @param hw 			Pointer to the SPI data structure.
 * @param mode 			SPI Mode 0-3. [1]
 * @param frq 			SPI frequency in Hertz.
 * @return FUNCTION_RETURN 
 */
FUNCTION_RETURN spi_set_param(spi_hw_t* hw, MCU_SPI_MODE_T mode, uint32_t frq);
/**
 * @brief 	This function can be used to set the SPI clock.
 * 
 * Since the SPI clock cannot be set freely, because it depends on the mcu clock, the frq value is the maximum value that could be set.
 * If the frq value cannot be set exactly, the next possible lower value for frq is set for the clock.
 * 
 * @param hw 			Pointer to the SPI data structure.
 * @param frq 			SPI frequency in Hertz.
 * @return FUNCTION_RETURN 
 */
FUNCTION_RETURN spi_set_clock(spi_hw_t* hw, uint32_t frq);
/**
 * @brief Sets the spi chip select pin to 1 or 0.
 * Set to 0 to start a SPI transmission or 1 to set SPI back to idle.
 * 
 * @param hw 			Pointer to the SPI data structure.
 * @param state 		Set to 0 to start a SPI transmission or 1 to set SPI back to idle.
 */
void spi_set_chip_select(spi_hw_t* hw, uint8_t state);
/**
 * @brief 	Writes data over spi and writes the received data into the receive buffer.
 * 			rx_buf can be tx_buf, if only a single buffer is used for sending and receiving.
 * 
 * @param hw 			Pointer to the SPI data structure.
 * @param tx_buf 		Pointer to the buffer of the data that should be written.
 * @param rx_buf 		Pointer to the buffer of the data where the response is stored.
 * @param len 			Number of bytes in the buffer for writing and reading.
 */
void spi_tx_rx(spi_hw_t* hw, const uint8_t* tx_buf, uint8_t* rx_buf, uint16_t len);

#endif /* MODULE_COMM_SPI_SPI_H_ */
