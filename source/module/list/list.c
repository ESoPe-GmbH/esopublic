/**
 * 	@file 	list.c
 * 	@copyright Urheberrecht 2020-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "list.h"
#include "module/comm/dbg.h"
#include <string.h>

//------------------------------------------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// Internal variables
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// External functions
//------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN list_init(list_t* list, void* s, void* s_next)
{
	if(list == NULL)
		return FUNCTION_RETURN_PARAM_ERROR;

	list->first = NULL;
	list->offset_next = (uintptr_t )(s_next - s);

	return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN list_add_element(list_t* list, void* s)
{
	if(list == NULL || s == NULL)
		return FUNCTION_RETURN_PARAM_ERROR;

	if(list->first == NULL)
	{
		list->first = s;
	}
	else
	{
		// Set object pointer to first element in list.
		void* ptr = list->first;

		// While pointer to next element is not NULL
		while(*(void**)(ptr + list->offset_next) != NULL && ptr != s)
		{
			// Set Object Pointer to the next element
			memcpy(&ptr, ptr + list->offset_next, sizeof(void*));
		}

		if(ptr == s)
			return FUNCTION_RETURN_NOT_READY;

		// Copy pointer to element to the next element's position of the last element.
		memcpy(ptr + list->offset_next, &s, sizeof(void*));
		// Set pointer to the next element of the current element to NULL.
		*(uintptr_t *)(s + list->offset_next) = (uintptr_t )NULL;
	}
	return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN list_remove_element(list_t* list, void* s)
{
	if(list == NULL || s == NULL)
		return FUNCTION_RETURN_PARAM_ERROR;

	if(s == list->first)
	{
		list->first = (void*)*(uintptr_t *)(s + list->offset_next);
		return FUNCTION_RETURN_OK;
	}
	else
	{
		// Set object pointer to first element in list.
		void* ptr = list->first;

		do
		{
			// If next element is the element to remove
			if((void*)*(uintptr_t *)(ptr + list->offset_next) == s)
			{
				// Change the next pointer of the current element to the next pointer of the element to remove.
				memcpy(ptr + list->offset_next, s + list->offset_next, sizeof(void*));
				return FUNCTION_RETURN_OK;
			}
		}
		// While pointer to next element is not NULL -> ptr is also set to the next element.
		while(*(void**)memcpy(&ptr, ptr + list->offset_next, sizeof(void*)) != NULL);

		return FUNCTION_RETURN_NOT_FOUND;
	}
}

void* list_first_element(list_t* list)
{
	if(list == NULL)
		return NULL;

	return list->first;
}

void* list_next_element(list_t* list, void* s)
{
	if(list == NULL || s == NULL)
		return NULL;

	return (void*)*(uintptr_t *)(s + list->offset_next);
}


//------------------------------------------------------------------------------------------------------------
// Internal functions
//------------------------------------------------------------------------------------------------------------

