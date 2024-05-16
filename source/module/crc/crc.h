// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		crc.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		Is used for calculating 16-bit crc.
 *  			Standard CRC polynoms:
 *  				CRC-CCITT: 				0x1021 (x^16 + x^12 + x^5 + 1)
 *  				CRC-CCITT reversed: 	0x8408 (x^16 + x^12 + x^5 + 1)
 *  				CRC16:					0x8005 (x^16 + x^15 + x^2 + 1)
 *
 *	@version	1.07 (19.12.2022)
 * 	    - data parameter in crc_calc is now const, because it is not modified inside the function.
 *	@version	1.06 (19.01.2022)
 * 	    - Modified to be used in esopekernel
 *	@version 	1.05 (07.06.2018)
 *		- Added module.h support
 *  @version 	1.04 (13.01.2015)
 *  	- Added constant lookup table for x^16 + x^15 + x^2 + 1 in not reversed state.
 *  @version 	1.03 (21.05.2014)
 *  	- Added crc_init_handler_reversed for calculation in reversed byte order.
 *  	- Added constant lookup tables for x^16 + x^12 + x^5 + 1 in reversed and not reversed state.
 *  @version 	1.02 (10.05.2013)
 *  	- Made crc_calc_byte public.
 *  @version 	1.01 (17.02.2013)
 *  	- Added comments
 *  @version	1.00 (15.05.2012)
 *  	- Intial release
 *
 ******************************************************************************/

#ifndef CRC_H_
#define CRC_H_

#include "module_public.h"
#if MODULE_ENABLE_CRC
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the crc module
#define CRC_STR_VERSION "1.07"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * 	@struct crc_t
 *
 * The structure is used for calculating the crc. It contains the initial value, a polynom used for calculation and
 * a final value that is used XOR the calculated crc.
 */
typedef struct{
	uint16_t polynom;		///< Generator polynom used in the calculation. For CRC-CCITT use 0x1021.
	uint16_t initial;		///< Initial value for the crc word before adding the first byte.
	uint16_t final_xor;		///< Final value that is calculated xor the calculated crc.
	bool reverse;			///< Reverses the crc calculation
}crc_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * 	@brief		Initializes the crc_t by adding the parameters to it.
 *
 * 				For x^16 + x^12 + x^5 + 1 the polynom is 0x1021.
 *
 * 	@param c			Pointer to the crc_t structure that needs to be intialized.
 * 	@param polynom		Generator polynom used in the calculation. For CRC-CCITT use 0x1021.
 * 	@param initial		Initial value for the crc word before adding the first byte.
 * 	@param final_xor	Final value that is calculated xor the calculated crc.
 */
void crc_init_handler(crc_t *c, uint16_t polynom, uint16_t initial, uint16_t final_xor);

/**
 * 	@brief		Initializes the crc_t by adding the parameters to it by setting the calculation in reversed data bytes
 * 				and with a reversed crc result before final XOR.
 *
 * 				By using the inverted crc, the polynom bust me also reversed. For x^16 + x^12 + x^5 + 1 the polynom is 0x8408.
 *
 *
 * 	@param c			Pointer to the crc_t structure that needs to be intialized.
 * 	@param polynom		Generator polynom used in the calculation. For CRC-CCITT use 0x8408.
 * 	@param initial		Initial value for the crc word before adding the first byte.
 * 	@param final_xor	Final value that is calculated xor the calculated crc.
 */
void crc_init_handler_reversed(crc_t *c, uint16_t polynom, uint16_t initial, uint16_t final_xor);

/**
 *	@brief		Calculates a crc over the data using the crc_t.
 *
 * 	@param c			Pointer to the crc_t structure that is used for the calculation.
 * 	@param data			Pointer to the data array that is used for the calculation.
 * 	@param data_len		Number of bytes that should be calculated.
 * 	@return				Calculated crc word for the data array.
 */
uint16_t crc_calc(crc_t *c, const uint8_t *data, uint16_t data_len);

/**
 * @brief	Calculates one word for the crc and returns the new word.
 *
 * If not using crc_calc you need to set crc to c->initial manually before! Also you need to XOR manually the final crc with c->final_xor.
 *
 * Calculates the new crc word by using the old crc word, the new byte and the crc polynom.
 *
 * @pre	The crc_t must be initialized with crc_init_handler (Should happen externally by the user).
 *
 * @param c		Pointer to the crc_t structure.
 * @param crc	CRC word before adding the new byte b.
 * @param b		Byte that is added to the crc.
 * @return		New crc word after adding the byte b.
 */
uint16_t crc_calc_byte(crc_t *c, uint16_t crc, uint8_t b);

#endif

#endif /* CRC_H_ */
