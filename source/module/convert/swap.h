// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 	swap.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Contains functions for swapping endian Data from Big to Little Endian and back.
 *
 *	@version	2.02 (07.06.2018)
 *		- Added module.h support
 *  @version	2.01 (17.02.2013)
 *  	- Added comments for swap16 and swap32
 *  @version	2.00 (04.01.2011)
 *  	- build the new convert module on the base of the old 1.63
 *
 *	@par 	References
 *
 ******************************************************************************/
#ifndef SWAP_H_
#define SWAP_H_

#include "module_public.h"
#if MODULE_ENABLE_CONVERT_SWAP

#include <stdint.h>

/**
 * @brief 	Swaps a 16-Bit unsigned integer value between big and little endian.
 *
 * @param x		16-Bit unsigned integer value.
 * @return		Swapped integer value.
 */
uint16_t swap16(uint16_t x);

/**
 * @brief 	Swaps a 32-Bit unsigned integer value between big and little endian.
 *
 * @param x		32-Bit unsigned integer value.
 * @return		Swapped integer value.
 */
uint32_t swap32(uint32_t x);
/**
 * @brief 	Swaps a 64-Bit unsigned integer value between big and little endian.
 *
 * @param x		64-Bit unsigned integer value.
 * @return		Swapped integer value.
 */
uint64_t swap64(uint64_t x);
/**
 * @brief   Swaps the endianness of a float.
 * 
 * @param x     Float value that needs to be swapped
 * @return      Swapped float value
 */
float swapf(float x);
/**
 * @brief   Swaps the endianness of a float directly by using a pointer.
 * 
 * @param ptr   Pointer to a float value to swap endianness.
 */
void swapf_ptr(float* ptr);

#endif

#endif /* CONVERT_H_ */
