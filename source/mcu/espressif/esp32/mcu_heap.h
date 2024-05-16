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

#if MCU_TYPE == MCU_ESP32

#include "esp_system.h"
#include "esp_heap_caps.h"
#include <stdlib.h>

// #define MCU_HEAP_DEBUG          

#if defined(MCU_HEAP_DEBUG)
#include "module/comm/dbg.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Get the size of available heap. Note that the returned value may be larger than the maximum contiguous block which can be allocated.
 */
#define mcu_heap_get_free_size()            esp_get_free_heap_size()

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

#if CONFIG_SPIRAM
#define mcu_heap_calloc(num, size)          heap_caps_calloc(num, size, MALLOC_CAP_SPIRAM) 
#define mcu_heap_malloc(size)               heap_caps_malloc(size, MALLOC_CAP_SPIRAM) 
#else
#define mcu_heap_calloc(num, size)          calloc(num, size) 
#define mcu_heap_malloc(size)               malloc(size) 
#endif
/**
 * @brief Frees a pointer from the heap. 
 */
#define mcu_heap_free(ptr)                  free(ptr)

#endif // defined(MCU_HEAP_DEBUG)

#endif // MCU_TYPE == MCU_ESP32

#endif /* __MCU_HEAP_H__ */
