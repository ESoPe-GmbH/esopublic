/**
 * @file module_public.h
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * @author Felix Herrmann
 * 
 * @brief Configuration to enable / disable modules for this project.
 * 
 * @version 1.00 (04.04.2024)
 * 	- Initial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_MODULE_PUBLIC_H_
#define __MODULE_MODULE_PUBLIC_H_

#ifdef ESP_PLATFORM
#include "sdkconfig.h"
#endif

#if CONFIG_ESOPUBLIC_ENABLE
// Use mcu config from KConfig

#include "module_public_kconfig.h"

#else

#include "module_enable.h"

// Enforce enables that are needed

#include "module_config.h"

#endif

/// True: FT81x register are used, false: FT800 register are used
#define EVE_USE_FT81X								(EVE_GENERATION > 1)

#if DEBUG_CONSOLE_ENABLE_ESP && MCU_TYPE != MCU_ESP32
#warning esp console can only be used on esp32 mcus!
#undef DEBUG_CONSOLE_ENABLE_ESP
#define DEBUG_CONSOLE_ENABLE_ESP                    false
#endif

#if MODULE_PREFER_HEAP

/// Declares an array based on heap settings as heap or fixed array. If you need a static variable, write static before the macro call.
/// Use this for the variable declaration, but make sure to also call MODULE_ARRAY_CREATE so the array will be initialized correctly before using it.
/// @param t            Type of the array
/// @param n            Name of the array
/// @param size         Number of elements in the array.
#define MODULE_ARRAY_DECLARE(t, n, size)            t* n
/// Initializes an array that was declared using MODULE_ARRAY_DECLARE.
/// @param n            Name of the array
/// @param size         Number of elements in the array.
#define MODULE_ARRAY_CREATE(n, size)                n = mcu_heap_calloc(sizeof(__typeof__(n)), size)
/// Frees an array. To use it again, call MODULE_ARRAY_CREATE.
/// @param n            Name of the array
#define MODULE_ARRAY_DESTROY(n)                     mcu_heap_free(n)

#else

/// Declares an array based on heap settings as heap or fixed array. If you need a static variable, write static before the macro call.
/// Use this for the variable declaration, but make sure to also call MODULE_ARRAY_CREATE so the array will be initialized correctly before using it.
/// @param t            Type of the array
/// @param n            Name of the array
/// @param size         Number of elements in the array.
#define MODULE_ARRAY_DECLARE(t, n, size)            t n[size]
/// Initializes an array that was declared using MODULE_ARRAY_DECLARE.
/// @param n            Name of the array
/// @param size         Number of elements in the array.
#define MODULE_ARRAY_CREATE(n, size)                memset(n, 0, sizeof(__typeof__(n)) * size)
/// Frees an array. To use it again, call MODULE_ARRAY_CREATE.
/// @param n            Name of the array
#define MODULE_ARRAY_DESTROY(n)                     do{}while(0)

#endif

#include "mcu/mcu.h"
#include "mcu/sys.h"

#endif /* __MODULE_MODULE_PUBLIC_H_ */