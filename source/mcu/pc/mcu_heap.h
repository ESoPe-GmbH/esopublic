// Urheberrecht 2018-2021 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file mcu_heap.h
 * 	@copyright Urheberrecht 2018-2021 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			
 *  @version	1.00 (09.12.2021)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef __MCU_HEAP_H__
#define __MCU_HEAP_H__

#include "mcu/mcu.h"

#if MCU_TYPE == PC_EMU

// #define MCU_HEAP_DEBUG          
#include <stdlib.h>

#if defined(MCU_HEAP_DEBUG)
#include "module/comm/dbg.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Get the size of available heap. Note that the returned value may be larger than the maximum contiguous block which can be allocated.
 */
#define mcu_heap_get_free_size()            0xFFFFFF

#if defined(MCU_HEAP_DEBUG)

void* mcu_heap_calloc_debug(char* filename, char* line_str, size_t num, size_t size);

#define mcu_heap_calloc(num, size)          mcu_heap_calloc_debug(DBG_STRING, num, size)

void* mcu_heap_malloc_debug(char* filename, char* line_str, size_t size);

#define mcu_heap_malloc(size)               mcu_heap_malloc_debug(DBG_STRING, size)

void mcu_heap_free_debug(char* filename, char* line_str, void* ptr);

/**
 * @brief Frees a pointer from the heap. 
 */
#define mcu_heap_free(ptr)                  mcu_heap_free_debug(DBG_STRING, ptr)

#else

#define mcu_heap_calloc(num, size)          calloc(num, size) 
#define mcu_heap_malloc(size)               malloc(size) 
/**
 * @brief Frees a pointer from the heap. 
 */
#define mcu_heap_free(ptr)                  free(ptr)

#endif // defined(MCU_HEAP_DEBUG)

#endif // MCU_TYPE == MCU_ESP32

#endif /* __MCU_HEAP_H__ */
