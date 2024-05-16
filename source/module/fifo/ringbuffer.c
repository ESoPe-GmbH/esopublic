/**
 * @file ringbuffer.c
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "ringbuffer.h"
#include "module/util/assert.h"
#include <string.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

ringbuffer_t* ringbuffer_create(size_t element_size, size_t element_num)
{
    ringbuffer_t* buffer = mcu_heap_calloc(1, sizeof(ringbuffer_t));
    ASSERT_RET_NOT_NULL(buffer, NO_ACTION, NULL);
    
    buffer->buffer = mcu_heap_calloc(element_size, element_num);    
    ASSERT_RET_NOT_NULL(buffer->buffer, mcu_heap_free(buffer), NULL);

    buffer->buffer_size = element_size * element_num;
    buffer->element_size = element_size;
    buffer->max_elements = element_num;
    
    return buffer;
}

void ringbuffer_free(ringbuffer_t* buffer)
{
    ASSERT_RET_NOT_NULL(buffer, NO_ACTION, NO_RETURN);
    if(buffer->buffer)
    {
        mcu_heap_free(buffer->buffer);
    }
    mcu_heap_free(buffer);
}

FUNCTION_RETURN_T ringbuffer_put(ringbuffer_t* buffer, const void* element)
{
    ASSERT_RET_NOT_NULL(buffer, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(element, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    // Add element into the buffer at current write position
    memcpy(buffer->buffer + buffer->w_pos, element, buffer->element_size);
    // Increment the write position
    buffer->w_pos = (buffer->w_pos + buffer->element_size) % buffer->buffer_size;
    // If not full, increment element counter
    if(buffer->elements < buffer->max_elements)
        buffer->elements++;
    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T ringbuffer_get_first(ringbuffer_t* buffer, void* element, uintptr_t* pos)
{
    ASSERT_RET_NOT_NULL(buffer, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(element, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(pos, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    if(buffer->elements == buffer->max_elements)
    {
        // Buffer is full, take element that would be overwritten next
        memcpy(element, buffer->buffer + buffer->w_pos, buffer->element_size);
        *pos = buffer->w_pos;
    }
    else
    {
        // Buffer not full, first element is at the beginning of the buffer
        memcpy(element, buffer->buffer, buffer->element_size);
        *pos = 0;
    }

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T ringbuffer_get_next(ringbuffer_t* buffer, void* element, uintptr_t* pos)
{
    ASSERT_RET_NOT_NULL(buffer, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(element, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(pos, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    *pos = ((*pos) + buffer->element_size) % buffer->buffer_size;

    if(*pos == buffer->w_pos)
    {
        memset(element, 0, buffer->element_size);
        return FUNCTION_RETURN_NOT_FOUND;
    }
    
    memcpy(element, buffer->buffer + *pos, buffer->element_size);

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T ringbuffer_get(ringbuffer_t* buffer, void* element, size_t index)
{
    ASSERT_RET_NOT_NULL(buffer, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(element, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    if(index >= buffer->elements)
    {
        // Have a silent assert
        return FUNCTION_RETURN_NOT_FOUND;
    }

    if(buffer->elements == buffer->max_elements)
    {
        // Buffer is full, take element that would be overwritten next as starting point
        uintptr_t pos = (buffer->w_pos + (uintptr_t)(index * buffer->element_size)) % buffer->buffer_size;
        memcpy(element, buffer->buffer + pos, buffer->element_size);
    }
    else
    {
        // Buffer not full, first element is at the beginning of the buffer, index is starting from there
        memcpy(element, buffer->buffer + (index * buffer->element_size), buffer->element_size);
    }

    return FUNCTION_RETURN_OK;

}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

