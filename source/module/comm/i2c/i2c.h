/**
 * 	@file i2c.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		The i2c module is used to generify the access to the I2C interface of either the mcu peripheral or an manual implementation
 *  			for i2c.
 *
 *  @version	1.00 (11.06.2016)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_COMM_I2C_I2C_H_
#define MODULE_COMM_I2C_I2C_H_

#include "module_public.h"

#if MODULE_ENABLE_COMM_I2C

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
 * @struct i2c_t
 * Contains variables needed to manage the I2C handling.
 */
typedef struct
{
	/// Handle for the mcu peripheral of the i2c. Is NULL if the manual implementation is used.
	mcu_i2c_t i2c;
	/// Data pin of the I2C interface.
	MCU_IO_PIN sda;
	/// Clock pin of the I2C interface.
	MCU_IO_PIN scl;
	/// Real frequency that is applied on the interface.
	uint32_t frequency;
	/// Address of the slave peripheral.
	uint8_t addr;
}i2c_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes the I2C interface which is identified by the parameters.
 * @details	Tries to initializes the mcu peripheral for the I2C. If this fails, a timer for a manual I2C implementation is started.
 *
 * @param h			Pointer to the I2C interface context structure.
 * @param num		Index of the mcu peripheral might be needed if no manual implementation is used.
 * @param sda		Data pin for the I2C interface.
 * @param scl		Clock pin of the I2C interface.
 **/
void i2c_init(i2c_t* h, uint8_t num, MCU_IO_PIN sda, MCU_IO_PIN scl);
/**
 * @brief 	Frees the I2C interface. Does not de-allocate memory of the handle.
 * 
 * @param h			Pointer to the I2C interface context structure.
 */
void i2c_free(i2c_t* h);
/**
 * @brief	Sets the frequency for the I2C communication.
 *
 * @param h			Pointer to the I2C interface context structure.
 * @param frequency	Frequency that should be used for the I2C interface communication.
 */
void i2c_set_frq(i2c_t* h, uint32_t frequency);
/**
 * @brief	Returns the real frequency that was set for the I2C communication.
 *
 * @param h			Pointer to the I2C interface context structure.
 * @return			Real frequency that was set for the I2C communication.
 */
uint32_t i2c_get_frq(i2c_t* h);
/**
 * @brief 	Sets the address of the slave device.
 *
 * @param h			Pointer to the I2C interface context structure.
 * @param address	Address of the slave device.
 */
void i2c_set_address(i2c_t* h, uint8_t address);
/**
 * @brief 	Writes/Reads bytes on the I2C interface. If wbuf is NULL or wlen is 0, only the address is sent and bytes are read afterwards.
 * 			If rbuf is NULL or rlen is 0, data is only written and nothing is read.
 *
 * @param h			Pointer to the I2C interface context structure.
 * @param wbuf		Pointer to the buffer where the data is stored that should be written.
 * @param wlen		Number of bytes to write except for the address of the slave device.
 * @param rbuf		Pointer to the buffer where the data is stored after reading it.
 * @param rlen		Number of bytes to read from the slave device.
 * @return			true: Data was written / read successfully.\n
 * 					false: An NAK was received and data writing / reading failed.
 */
bool i2c_wr(i2c_t* h, uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen);
/**
 * @brief 	Writes/Reads bytes on the I2C interface. If wbuf is NULL or wlen is 0, only the address is sent and bytes are read afterwards.
 * 			If rbuf is NULL or rlen is 0, data is only written and nothing is read.
 *
 * @param h			Pointer to the I2C interface context structure.
 * @param wbuf		Pointer to the buffer where the data is stored that should be written. This is the first write buffer.
 * @param wlen		Number of bytes to write except for the address of the slave device. This is the length for the first write buffer.
 * @param w2buf		Pointer to the buffer where the data is stored that should be written. This is the second write buffer that will be concatenated behind the first buffer.
 * @param w2len		Number of bytes to write except for the address of the slave device. This is the length for the second write buffer.
 * @param rbuf		Pointer to the buffer where the data is stored after reading it.
 * @param rlen		Number of bytes to read from the slave device.
 * @return			true: Data was written / read successfully.\n
 * 					false: An NAK was received and data writing / reading failed.
 */
bool i2c_wwr(i2c_t* h, uint8_t* wbuf, size_t wlen, uint8_t* w2buf, size_t w2len, uint8_t* rbuf, size_t rlen);

#endif // MODULE_ENABLE_COMM_I2C

#endif /* MODULE_COMM_I2C_I2C_H_ */
