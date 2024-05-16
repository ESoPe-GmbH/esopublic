/**
 * 	@file 		crc8.h
 * 	@copyright Urheberrecht 2018-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		Is used for calculating 8-bit crc.
 *  			Standard CRC polynoms:
 *  				CRC-CCITT: 				0x07 (x^8 + x^2 + x^1 + 1)
 *
 *  @version	1.00 (22.02.2024)
 *  	- Intial release
 *
 ******************************************************************************/

#ifndef CRC8_H_
#define CRC8_H_

#include "module_public.h"
#if MODULE_ENABLE_CRC
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the crc module
#define CRC8_STR_VERSION "1.00"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * 	@struct crc8_t
 *
 * The structure is used for calculating the crc. It contains the initial value, a polynom used for calculation and
 * a final value that is used XOR the calculated crc.
 */
typedef struct
{
	/// Generator polynom used in the calculation. For CRC-CCITT use 0x07.
	uint8_t polynom;		
	/// Initial value for the crc word before adding the first byte.
	uint8_t initial;		
	/// Final value that is calculated xor the calculated crc.
	uint8_t final_xor;		
	/// Reverses the crc calculation
	uint8_t reverse;		
}crc8_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * 	@brief		Initializes the crc_t by adding the parameters to it.
 *
 * 				For x^8 + x^2 + x^1 + 1 the polynom is 0x07.
 *
 * 	@param c			Pointer to the @c crc8_t structure that needs to be intialized.
 * 	@param polynom		Generator polynom used in the calculation. For CRC-CCITT use 0x1021.
 * 	@param initial		Initial value for the crc word before adding the first byte.
 * 	@param final_xor	Final value that is calculated xor the calculated crc.
 *  @param reversed		Can be set to calculate the reversed CRC.
 */
void crc8_init(crc8_t *c, uint8_t polynom, uint8_t initial, uint8_t final_xor, bool reversed);

/**
 *	@brief		Calculates a crc over the data using the crc_t.
 *
 * @pre	The @c crc8_t must be initialized with @c crc_init (Should happen externally by the user).
 *
 * 	@param c			Pointer to the @c crc8_t structure that is used for the calculation.
 * 	@param data			Pointer to the data array that is used for the calculation.
 * 	@param data_len		Number of bytes that should be calculated.
 * 	@return				Calculated crc word for the data array.
 */
uint8_t crc8_calc(crc8_t *c, const uint8_t *data, size_t data_len);

/**
 * @brief	Calculates one byte for the crc and returns the new byte.
 *
 * If not using crc_calc you need to set crc to c->initial manually before! Also you need to XOR manually the final crc with c->final_xor.
 *
 * Calculates the new crc byte by using the old crc byte, the new byte and the crc polynom.
 *
 * @pre	The @c crc8_t must be initialized with @c crc_init (Should happen externally by the user).
 *
 * @param c		Pointer to the @c crc8_t structure.
 * @param crc	CRC word before adding the new byte b.
 * @param b		Byte that is added to the crc.
 * @return		New crc word after adding the byte b.
 */
uint8_t crc8_calc_byte(crc8_t *c, uint8_t crc, uint8_t b);

#endif

#endif /* CRC8_H_ */
