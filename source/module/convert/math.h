// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 	math.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Contains mathematical functions.
 *			Extracted from the old ESoPe convert.c module.
 *
 *	@version	1.06 (19.01.2022)
 * 	    - Modified to be used in esopekernel
 *  @version    1.05 (24.11.2021)
 *      - Added MATH_CONSTRAIN
 *	@version	1.04 (06.08.2018)
 *		- MODULE_ENABLE_CONVERT_MATH fixed
 *	@version	1.03 (07.06.2018)
 *		- Added module.h support
 *	@version	1.02 (24.05.2018)
 *		- Added MATH_MAX, MATH_MIN,MATH_ABS
 *  @version	1.01 (15.01.2015)
 *  	- Added MATH_ENABLE_64BIT_OPERATIONS and math_pow_mod64
 *  @version	1.00 (28.09.2012)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/
 
#ifndef _CONVERT_MATH_HEADER_FIRST__INCL__
#define _CONVERT_MATH_HEADER_FIRST__INCL__

#include "module_public.h"
#if MODULE_ENABLE_CONVERT_MATH

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
/// Version of the math module
#define MATH_STR_VERSION "1.06"

/// Uses the maximum of x or y
#define MATH_MAX(x, y)  ((x) > (y) ? (x) : (y))

/// Uses the minimum of x or y
#define MATH_MIN(x, y)  ((x) < (y) ? (x) : (y))

/// Uses the absolute value of x and y
#define MATH_ABS(x, y)  ((x) > (y) ? (x) - (y) : (y) - (x))

/// Constrains a value to minimum and maximum values.
#define MATH_CONSTRAIN(v, min, max)     ( (v) < (min) ? (min) : (v) > (max) ? (max) : (v))

#if MATH_ENABLE_64BIT_OPERATIONS
/**
 * @brief	Calculates a 64-bit unsigned integer of the basis to the power of the exponent.
 * 			This calculation uses a modulo during calculation. It is the same as the calculation
 * 			without the modulo and using it at the end. But using the modulo during calculation
 * 			keeps the values smaller.
 *
 * @param basis			Basis of the calculation
 * @param exponent		Exponent of the calculation
 * @param modulo		Modulo that is used during calculation
 *
 * @return				Returns (basis to the power of exponent) % modulo
 */
uint64_t math_pow_mod64(uint64_t basis, uint64_t exponent, uint64_t modulo);
#endif

/**
 *	Returns the number of numeric characters for the given decimal value.
 *
 * @param value					Number for which the number of numeric charactes will be calculated. Hast to be unsigned with a maximum range of 32-bit.
 * @return						Number of numeric characters for decimal value.
 **/
uint8_t math_declen(uint32_t value);

/**
 *	Returns the number of numeric characters for the given hexadecimal value.
 *
 * @param value					Number for which the number of numeric charactes will be calculated. Hast to be unsigned with a maximum range of 32-bit.
 * @return						Number of numeric characters for hexadecimal value (always a multiple of 2).
 **/
uint8_t math_hexlen(uint32_t value);

/**
 * @brief Calculates the log2 for an integer number. Can be used to find the highest bit index in a bitmask.
 * 
 * @param value                 Integer value to check.
 * @return int                  Index of the highest bit = log2(value).
 */
int math_log2_int(int value);

#endif

#endif
