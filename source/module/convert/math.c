// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file math.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */
 

#include "module_public.h"
#if MODULE_ENABLE_CONVERT_MATH

#include "math.h"

#if MATH_ENABLE_64BIT_OPERATIONS
uint64_t math_pow_mod64(uint64_t basis, uint64_t exponent, uint64_t modulo)
{
    uint64_t result = basis;

	if(exponent == 0)
		return 1;

	for(exponent = exponent - 1; exponent > 0; exponent--)
		result = (result * basis) % modulo;

	return result;
}
#endif

uint8_t math_declen(uint32_t value)
{
	uint8_t i = 0;
	
	if(value == 0) 
		return 1;
		
	for(i = 0; i < 10; i++)
	{
		if(pow(10, i) > value)
			break;
	}
	
	return i;
}

uint8_t math_hexlen(uint32_t value)
{
	uint8_t i = 0;
	
	if(value == 0) 
		return 2;
	
	for(i = 0; i < 9; i++)
	{
		if(pow(16, i) > value)
			break;
	}
	
	return i + (i%2);	// if i is a multiple of 2 the modulo is zero. Else it is increased by one.
}

#endif
