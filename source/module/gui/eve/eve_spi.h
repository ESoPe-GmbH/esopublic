/**
 * 	@file eve_spi.h
 *  @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief	Offers the spi write and read functions for the eve chip to write registers and host commands.
 *  		The handle function of this source file checks the interrupt handler and calls the corresponding eve_copro
 *  		functions.
 *
 *  		The functions are internally handled in eve and gui module! Do not call them from outside of the eve or gui module!
 *
 *  @version	1.00 (07.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef EVE_SPI_H_
#define EVE_SPI_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "eve.h"
#include "eve_register.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief	Writes SPI Host commands to the eve chip. See EVE_HOST_CMD for details.
 *
 * @param cmd		SPI Host command. See EVE_HOST_CMD for details.
 */
void eve_spi_send_host_command(eve_t* obj, EVE_HOST_CMD cmd);

/**
 * @brief	Writes a bunch of data to the address of the eve chip. Can be used to write strings to the
 * 			eve chip. This function is also used by the other write functions, except eve_spi_send_host_command.
 * 			Same functionality as eve_spi_write_data, except bytes > 0x7F in data are skipped and a 0 is appended
 * 			at the end.
 *
 * @param address	Address of the eve chip, where the data is written to.
 * @param data		Pointer to the 0-terminated string, where the data to be written is stored.
 * @param add_padding   If true, the data will be pad with 0 to be a multiple of 4, otherwise spi stops after transmitting terminating 0.
 *
 * @return 			Number of bytes of the string written, excluding the 0xFF
 */
uint16_t eve_spi_write_str_data(eve_t* obj, uint32_t address, char *data, bool add_padding);

/**
 * @brief	Writes a data buffer to the address of the eve chip. Can be used to write images or binary data to the
 * 			eve chip. This function is also used by the other write functions, except eve_spi_send_host_command.
 *
 * @param address	Address of the eve chip, where the data is written to.
 * @param data		Pointer to the buffer, where the data to be written is stored.
 * @param len		Number of bytes, stored in data, that should be written.
 * @param add_padding   If true, the data will be pad with 0 to be a multiple of 4, otherwise spi stops after transmitting @c len bytes.
 */
void eve_spi_write_data(eve_t* obj, uint32_t address, const uint8_t *data, uint32_t len, bool add_padding);

/**
 * @brief	Writes multiple sets of data to the address of the eve chip. Can be used to write images or binary data to the
 * 			eve chip. This function is also used by the other write functions, except eve_spi_send_host_command.
 *
 * @param address	    Address of the eve chip, where the data is written to.
 * @param data		    Array of length @c num_dataset containing the pointer to the buffer, where the data to be written is stored for each dataset.
 * @param len		    Array of length @c num_dataset containing the number of bytes, stored in data, that should be written for each dataset.
 * @param num_dataset   Number of datasets to write.
 */
void eve_spi_write_multi_data(eve_t* obj, uint32_t address, const uint8_t **data, uint32_t *len, uint32_t num_dataset);

/**
 * @brief	Writes an 8-bit value to the address of the eve chip.
 *
 * @param address	Address of the eve chip, where the data is written to.
 * @param value		8-bit value to write to the chip.
 */
void eve_spi_write_8(eve_t* obj, uint32_t address, uint8_t value);

/**
 * @brief	Writes an 16-bit value to the address of the eve chip.
 *
 * @param address	Address of the eve chip, where the data is written to.
 * @param value		16-bit value to write to the chip.
 */
void eve_spi_write_16(eve_t* obj, uint32_t address, uint16_t value);

/**
 * @brief	Writes an 32-bit value to the address of the eve chip.
 *
 * @param address	Address of the eve chip, where the data is written to.
 * @param value		32-bit value to write to the chip.
 */
void eve_spi_write_32(eve_t* obj, uint32_t address, uint32_t value);

/**
 * @brief	Reads a bunch of data from the address of the eve chip. This function is used by the other read functions.
 *
 * @param address	Address of the eve chip, where the data is read from.
 * @param data		Pointer to the buffer, where the read data is stored.
 * @param len		Number of bytes that should be read.
 */
void eve_spi_read_data(eve_t* obj, uint32_t address, uint8_t *data, uint16_t len);

/**
 * @brief	Reads an 8-bit value from the address of the eve chip.
 *
 * @param address	Address of the eve chip, where the data is read from.
 * @return			8-bit value read from the chip.
 */
uint8_t eve_spi_read_8(eve_t* obj, uint32_t address);

/**
 * @brief	Reads an 16-bit value from the address of the eve chip.
 *
 * @param address	Address of the eve chip, where the data is read from.
 * @return			16-bit value read from the chip.
 */
uint16_t eve_spi_read_16(eve_t* obj, uint32_t address);

/**
 * @brief	Reads an 16-bit value from the address of the eve chip twice to make sure that both
 * 			readings have the same value.
 *
 * @param address	Address of the eve chip, where the data is read from.
 * @return			16-bit value read from the chip.
 */
uint16_t eve_spi_read_16_twice(eve_t* obj, uint32_t address);

/**
 * @brief	Reads an 32-bit value from the address of the eve chip.
 *
 * @param address	Address of the eve chip, where the data is read from.
 * @return			32-bit value read from the chip.
 */
uint32_t eve_spi_read_32(eve_t* obj, uint32_t address);

#endif

#endif /* EVE_SPI_H_ */
