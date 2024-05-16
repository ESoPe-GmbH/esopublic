/**
 * @file common.h
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * @author Tim Koczwara
 * 
 * @brief Common type for eve ui
 * 
 * @version 1.00 (04.04.2024)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_COMMON_H_
#define __MODULE_COMMON_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Type for a screen
typedef struct screen_s screen_t;

/// Type for a screen device
typedef struct screen_device_s screen_device_t;

#endif // MODULE_ENABLE_COMMON

#endif /* __MODULE_COMMON_H_ */