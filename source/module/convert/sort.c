// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file sort.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */
 
#include "module_public.h"
#if MODULE_ENABLE_CONVERT_SORT

#include "sort.h"

void sort_uint32_array(uint32_t *arr, uint16_t number_of_elements)
{
	uint16_t i, j;
	uint32_t tmp;
	
	for(i = 0; i < number_of_elements; i++)
	{
		for(j = i; j < number_of_elements; j++)
		{
			if(arr[i] > arr[j])
			{
				tmp = arr[i];
				arr[i] = arr[j];
				arr[j] = tmp;
			}
		}
	}
}

#endif
