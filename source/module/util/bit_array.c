/***
 * @file bit_array.c
 * @copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/
#include "bit_array.h"
#include "module/util/assert.h"
#include <string.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

bit_array_handle_t bit_array_create(size_t num_bits)
{
    size_t num_bytes = num_bits >> 3;
    if(num_bits & 7)
        num_bytes++;

    bit_array_handle_t ba = mcu_heap_calloc(1, sizeof(struct bit_array_s));

    if(ba)
    {
        ba->array = mcu_heap_calloc(1, num_bytes);
        if(ba->array)
        {
            ba->size = num_bytes;
            return ba;
        }
        mcu_heap_free(ba);
    }

    return NULL;
}

bit_array_handle_t bit_array_create_from_bytes(uint8_t* bytes, size_t num_bytes)
{
    bit_array_handle_t ba = mcu_heap_malloc(sizeof(struct bit_array_s));

    if(ba)
    {
        ba->array = mcu_heap_malloc(num_bytes);
        if(ba->array)
        {
            memcpy(ba->array, bytes, num_bytes);
            ba->size = num_bytes;
            return ba;
        }
        mcu_heap_free(ba);
    }

    return NULL;
}

void bit_array_free(bit_array_handle_t ba)
{
    mcu_heap_free(ba->array);
    ba->array = NULL;
    mcu_heap_free(ba);
}

bit_array_handle_t bit_array_clone(bit_array_handle_t ba)
{
    bit_array_handle_t ba2 = mcu_heap_malloc(sizeof(struct bit_array_s));

    if(ba2)
    {
        ba2->array = mcu_heap_malloc(ba->size);
        if(ba2->array)
        {
            memcpy(ba2->array, ba->array, ba->size);
            ba2->size = ba->size;
            return ba2;
        }
        mcu_heap_free(ba2);
    }

    return NULL;
}

FUNCTION_RETURN_T bit_array_set(bit_array_handle_t ba, size_t index)
{
    ASSERT_RET(ba, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid bit array handle\n");
    ASSERT_RET((index >> 3) < ba->size, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid bit array index\n");

    ba->array[index >> 3] |= (1 << (index & 7));

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T bit_array_clear(bit_array_handle_t ba, size_t index)
{
    ASSERT_RET(ba, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid bit array handle\n");
    ASSERT_RET((index >> 3) < ba->size, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid bit array index\n");

    ba->array[index >> 3] &= ~(1 << (index & 7));

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T bit_array_clear_all(bit_array_handle_t ba)
{
    ASSERT_RET(ba, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid bit array handle\n");

    memset(ba->array, 0, ba->size);

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T bit_array_set_value(bit_array_handle_t ba, size_t index, bool value)
{
    ASSERT_RET(ba, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid bit array handle\n");
    ASSERT_RET((index >> 3) < ba->size, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid bit array index\n");

    if(value)
        ba->array[index >> 3] |= (1 << (index & 7));
    else
        ba->array[index >> 3] &= ~(1 << (index & 7));

    return FUNCTION_RETURN_OK;
}

bool bit_array_is_set(bit_array_handle_t ba, size_t index)
{
    ASSERT_RET(ba, NO_ACTION, false, "Invalid bit array handle\n");
    ASSERT_RET((index >> 3) < ba->size, NO_ACTION, false, "Invalid bit array index\n");

    return (ba->array[index >> 3] & (1 << (index & 7))) != 0;
}

bool bit_array_has_any_set(bit_array_handle_t ba)
{
    ASSERT_RET(ba, NO_ACTION, false, "Invalid bit array handle\n");

    for(int i = 0; i < ba->size; i++)
    {
        if(ba->array[i])
            return true;
    }
    return false;
}

int bit_array_compare(bit_array_handle_t ba1, bit_array_handle_t ba2)
{
	if(ba1 == 0)
		return ba2 == 0;
	if(ba2 == 0)
		return ba1 == 0;

	if(ba1->size != ba2->size)
		return ba1->size - ba2->size;

	return memcmp(ba1->array, ba2->array, ba1->size);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

