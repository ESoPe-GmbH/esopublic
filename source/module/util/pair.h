// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file pair.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Defines pair structure.
 *
 *  @version	1.00 (30.10.2019)
 *  	- Intial release
 *
 *	@par 	References
 *
 **/
#ifndef MODULE_UTIL_PAIR_H_
#define MODULE_UTIL_PAIR_H_

#include <stdint.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Defines a structure for a string pair
 */
typedef struct pair_str_s
{
	/// Pointer to the key
	char* key;
	/// Pointer to the value
	char* value;
}pair_str_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Macro for initializing a pair_str_t by using key and value as arguments
#define PAIR_STR_INIT(k, v)			{.key = k, .value = v}
/// Number of entries when an array of pair_str_t is used
#define PAIR_STR_SIZE(p)			(sizeof(p) / sizeof(pair_str_t))

#endif /* MODULE_UTIL_PAIR_H_ */
