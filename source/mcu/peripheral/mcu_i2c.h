/**
 * 	@file 	mcu_i2c.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_i2c functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *
 *  @version    2.00 (20.08.2022)
 *               - Extracted mcu_i2c functions in this file. History from now on will only contain infos about mcu_i2c
 *	@version	1.12 (10.10.2016)
 *				 - Extracted types into new mcu_types.h
 *				 - Added I2C
 *
 ******************************************************************************/

#ifndef __MCU_I2C_HEADER_FIRST_INCLUDE__
#define __MCU_I2C_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_DEVICE_COUNT_I2C > 0
/**
 * Initializes an i2c interface on the specified pins. If the interface is not available, NULL is returned.
 *
 * @param num				Number of the i2c interface
 * @param sda				Data pin of the i2c interface
 * @param scl				Clock pin of the i2c interface
 *
 * @return					I2C handler that can be used with other mcu_i2c functions or NULL if the interface is not available.
 */
mcu_i2c_t mcu_i2c_init(uint8_t num, MCU_IO_PIN sda, MCU_IO_PIN scl);
/**
 * @brief Frees an i2c interface.
 * 
 * @param h					I2C handler, that was created with mcu_i2c_init.
 */
void mcu_i2c_free(mcu_i2c_t h);
/**
 * @brief	Sets the frequency for the i2c interface.
 *
 * @pre		The I2C handler must be created with mcu_i2c_init.
 *
 * @param h					I2C handler, that was created with mcu_i2c_init.
 * @param frequency			The frequency that should be used.
 */
void mcu_i2c_set_frq(mcu_i2c_t h, uint32_t frequency);
/**
 * @brief 	Returns the exact frequency, the i2c interface uses.
 *
 * @pre		The I2C handler must be created with mcu_i2c_init.
 *
 * @param h					I2C handler, that was created with mcu_i2c_init.
 * @return					The exact frequency.
 */
uint32_t mcu_i2c_get_frq(mcu_i2c_t h);
/**
 * @brief Sets the address of the device that is used when writing/reading data.
 *
 * @pre		The I2C handler must be created with mcu_i2c_init.
 *
 * @param h					I2C handler, that was created with mcu_i2c_init.
 * @param address			Address of the device.
 */
void mcu_i2c_set_address(mcu_i2c_t h, uint8_t address);
/**
 * @brief Writes and reads data on i2c. wlen and rlen describe how many bytes should be written/read.
 * If data is only read, set wlen to 0.
 * If data is only written, set rlen to 0.
 *
 * @pre		The I2C handler must be created with mcu_i2c_init.
 * @pre		The address must be set with mcu_i2c_set_address.
 *
 * @param h					I2C handler, that was created with mcu_i2c_init.
 * @param wbuf				Pointer to the buffer where the data to write is stored. Can be NULL if wlen is 0.
 * @param wlen				Number of bytes to write from wbuf.
 * @param rbuf				Pointer to the buffer where the read data should be written into. Can be NULL if rlen is 0.
 * @param rlen				Number of bytes to read from i2c device.
 */
bool mcu_i2c_wr(mcu_i2c_t h, uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen);
/**
 * @brief Writes and reads data on i2c. wlen and rlen describe how many bytes should be written/read.
 * If data is only read, set wlen to 0.
 * If data is only written, set rlen to 0.
 *
 * @pre		The I2C handler must be created with mcu_i2c_init.
 * @pre		The address must be set with mcu_i2c_set_address.
 *
 * @param h					I2C handler, that was created with mcu_i2c_init.
 * @param wbuf				Pointer to the buffer where the data to write is stored. Can be NULL if wlen is 0.
 * @param wlen				Number of bytes to write from wbuf.
 * @param w2buf		        Pointer to the buffer where the data is stored that should be written. This is the second write buffer that will be concatenated behind the first buffer.
 * @param w2len		        Number of bytes to write except for the address of the slave device. This is the length for the second write buffer.
 * @param rbuf				Pointer to the buffer where the read data should be written into. Can be NULL if rlen is 0.
 * @param rlen				Number of bytes to read from i2c device.
 */
bool mcu_i2c_wwr(mcu_i2c_t h, uint8_t* wbuf, size_t wlen, uint8_t* w2buf, size_t w2len, uint8_t* rbuf, size_t rlen);

#endif

#endif // __MCU_I2C_HEADER_FIRST_INCLUDE__
