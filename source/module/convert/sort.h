// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 	sort.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Contains sorting algorithms.
 *			Extracted from the old ESoPe convert.c module.
 *
 *	@version	1.01 (07.06.2018)
 *		- Added module.h support
 *  @version	1.00 (28.09.2012)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/
 
#ifndef _CONVERT_SORT_HEADER_FIRST__INCL__
#define _CONVERT_SORT_HEADER_FIRST__INCL__

#include "module_public.h"
#if MODULE_ENABLE_CONVERT_SORT

#include <stdint.h>

/**
 *	Sorts an array with unsigned 32-bit numbers by ordering it ascending.
 *
 * @param arr					Pointer to the start address of the array.
 * @param number_of_elements	Number of 32-bit values inside the array.
 **/
void sort_uint32_array(uint32_t *arr, uint16_t number_of_elements);

#endif

#endif
