// Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file mcu_heap.h
 * 	@copyright Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			
 *  @version	1.00 (14.05.2025)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef __MCU_HEAP_H__
#define __MCU_HEAP_H__

#include "mcu/mcu.h"

#if MCU_TYPE == MCU_NEW_NAME

#include <stdlib.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Get the size of available heap. Note that the returned value may be larger than the maximum contiguous block which can be allocated.
 */
#define mcu_heap_get_free_size()            0 // TODO: Implement for mcu
/**
 * @brief Allocate memory in heap that is cleared to 0.
 * 
 * @param num   Number of elements to allocate
 * @param size  Size in bytes for a single element
 * @return      Pointer to the allocated space with (@c num * @c size) Bytes or NULL if allocation failed.
 */
#define mcu_heap_calloc(num, size)          calloc(num, size) 
/**
 * @brief Allocate memory in heap.
 * 
 * @param size  Number of bytes to allocate
 * 
 * @return      Pointer to the allocated space with @c size Bytes or NULL if allocation failed.
 */
#define mcu_heap_malloc(size)               malloc(size) 
/**
 * @brief Frees a pointer from the heap. 
 * 
 * @param ptr   Pointer to the allocated space that needs to be freed.
 */
#define mcu_heap_free(ptr)                  free(ptr)

#endif // MCU_TYPE == MCU_NEW_NAME

#endif /* __MCU_HEAP_H__ */
