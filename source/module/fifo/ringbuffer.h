/**
 * @file ringbuffer.h
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * @author Tim Koczwara
 * 
 * @brief Ringbuffer module that automatically overrides the oldest data.
 * 
 * A ringbuffer that can be used to store data, like sensor data with a specified amount of elements. The oldest data will be overwritten automatically.
 * It can be used to read it out for graphical representation. You can get the data by either using @c ringbuffer_get and the index you want or by iterating
 * through the buffer by using @c ringbuffer_get_first and @c ringbuffer_get_next.
 * 
 * Creating the ringbuffer and adding data. In this example a float ringbuffer is used for 1000 elements and is filled with 1000 random values.
 * @code {.c}
 * ringbuffer_t* buffer = ringbuffer_create(sizeof(float), 1000);
 * 
 * for(int i = 0; i < 1000; i++)
 * {
 *      ringbuffer_put(buffer, (float)rand());
 * }
 * @endcode
 * 
 * In this example the ringbuffer is read until the end by using the index.
 * @code {.c}
 * size_t i = 0;
 * FUNCTION_RETURN_T ret;
 * do
 * {
 *  float value;
 *  ret = ringbuffer_get(buffer, &value, index);
 *  if(ret != FUNCTION_RETURN_OK)
 *      break;
 *  // Do something with your value
 *  printf("%.2f\n", value);
 *  i++;
 * }while(true);
 * @endcode
 * 
 * In this example the ringbuffer is read until the end by using iteration.
 * @code {.c}
 * uintptr_t pos;
 * FUNCTION_RETURN_T ret;
 * float value;
 * ret = ringbuffer_get_first(buffer, &value, &pos);
 * while(ret == FUNCTION_RETURN_OK)
 * {
 *  // Do something with your value
 *  printf("%.2f\n", value);
 * 
 *  ret = ringbuffer_get_next(buffer, &value, &pos);
 * }
 * @endcode
 * 
 * @version 1.00 (23.02.2024)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_RINGBUFFER_H_
#define __MODULE_RINGBUFFER_H_

#include "module_public.h"
#if MODULE_ENABLE_FIFO
#include "module/enum/function_return.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Structure for a ringbuffer that automatically overrides the data if full.
typedef struct ringbuffer_s
{
    /// @brief Buffer to store the data.
    void* buffer;
    /// @brief Position of the data that is written next.
    uintptr_t w_pos;
    /// @brief Total size of the buffer in byte.
    size_t buffer_size;
    /// @brief Maximum number of elements that fit in the buffer.
    size_t max_elements;
    /// @brief Number of elements that were written into the buffer.
    size_t elements;
    /// @brief Size of a single element inside the buffer.
    size_t element_size;
}ringbuffer_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Create a ringbuffer structure and a buffer by the defined size of the elements and the number of elements.
**/
/**
 * @brief   Create a ringbuffer structure and a buffer by the defined size of the elements and the number of elements.
 * 
 * @param element_size      Size of a single element inside the buffer.
 * @param element_num       Number of elements the buffer has to store.
 * 
 * @return                  Pointer to the buffer structure that was created or NULL if it could not be created.
*/
ringbuffer_t* ringbuffer_create(size_t element_size, size_t element_num);
/**
 * @brief   Frees the ringbuffer and the internal buffer.
 * 
 * @param buffer            Pointer to the buffer that was created using @c ringbuffer_create.
 */
void ringbuffer_free(ringbuffer_t* buffer);
/**
 * @brief   Adds a single element into the buffer.
 * 
 * @param buffer            Pointer to the buffer that was created using @c ringbuffer_create.
 * @param element           Pointer to the element that should be stored.
 * @retval FUNCTION_RETURN_PARAM_ERROR  @c buffer or @c element were NULL.
 * @retval FUNCTION_RETURN_OK           Value was added to the buffer.
 */
FUNCTION_RETURN_T ringbuffer_put(ringbuffer_t* buffer, const void* element);
/**
 * @brief   Get one element from the buffer based on it's index.
 * 
 * @param buffer            Pointer to the buffer that was created using @c ringbuffer_create.
 * @param element           Pointer to the element that should be read from the buffer.
 * @param index             Index of the element inside the buffer.
 * @retval FUNCTION_RETURN_PARAM_ERROR  @c buffer or @c element were NULL.
 * @retval FUNCTION_RETURN_NOT_FOUND    No element was found on this index. Might be an indicator that you reached the end.
 * @retval FUNCTION_RETURN_OK           Value was read from the buffer.
 */
FUNCTION_RETURN_T ringbuffer_get(ringbuffer_t* buffer, void* element, size_t index);
/**
 * @brief 
 * 
 * @param buffer            Pointer to the buffer that was created using @c ringbuffer_create.
 * @param element           Pointer to the element that should be read from the buffer.
 * @param pos               Pointer to a position index that is filled from the ringbuffer function. This is needed for @c ringbuffer_get_next to determine where the ringbuffer starts searching.
 * @retval FUNCTION_RETURN_PARAM_ERROR  @c buffer, @c element or @c pos were NULL.
 * @retval FUNCTION_RETURN_NOT_FOUND    No element was found in the buffer. The buffer is empty
 * @retval FUNCTION_RETURN_OK           Value was read from the buffer.
 */
FUNCTION_RETURN_T ringbuffer_get_first(ringbuffer_t* buffer, void* element, uintptr_t* pos);
/**
 * @brief 
 * 
 * @param buffer            Pointer to the buffer that was created using @c ringbuffer_create.
 * @param element           Pointer to the element that should be read from the buffer.
 * @param pos               Pointer to a position index that is filled from the ringbuffer function. Make sure you called @c ringbuffer_get_first before to get the correct first value.
 * @retval FUNCTION_RETURN_PARAM_ERROR  @c buffer, @c element or @c pos were NULL.
 * @retval FUNCTION_RETURN_NOT_FOUND    No next element was found in the buffer. Might be an indicator that you reached the end.
 * @retval FUNCTION_RETURN_OK           Value was read from the buffer.
 */
FUNCTION_RETURN_T ringbuffer_get_next(ringbuffer_t* buffer, void* element, uintptr_t* pos);

#endif // MODULE_ENABLE_FIFO

#endif /* __MODULE_RINGBUFFER_H_ */