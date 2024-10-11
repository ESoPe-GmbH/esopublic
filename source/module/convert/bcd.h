/**
 * 	@file 	swap.h
 * 	@copyright Urheberrecht 2018-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Contains functions for encoding and decoding bcd integer.
 *
 *	@version	1.00 (09.09.2024)
 *		- Initial release
 *
 *	@par 	References
 *
 ******************************************************************************/
#ifndef BCD_H_
#define BCD_H_

#include "module_public.h"
#if MODULE_ENABLE_CONVERT_BCD

#include <stdint.h>

/**
 * @brief Converts a decimal value into a BCD value.
 * 
 * @param value 		Value in decimal
 * @return uint8_t 		Value in BCD
 */
static uint8_t bcd_encode_uint8(uint8_t value);
/**
 * @brief Converts a BCD value into a decimal value.
 * 
 * @param value 		Value in BCD
 * @return uint8_t 		Value in decimal
 */
static uint8_t bcd_decode_uint8(uint8_t value);

#endif

#endif /* CONVERT_H_ */
