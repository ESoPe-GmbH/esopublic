// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file debug_buffer.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *
 *
 *	@version	1.01 (19.01.2022)
 * 	    - Modified to be used in esopekernel
 *  @version	1.00 (02.10.2015)
 *  	- Initial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef DEBUG_BUFFER_H_
#define DEBUG_BUFFER_H_

#include "module_public.h"

// DECLARE_DEBUG_BUFFER shall only be defines inside debug_console.c before including debug_buffer.h
// This ensures that the variables inside this header do not have the "extern" prefix in gui_control.
// All other files that include this header will have the "extern" prefix for the variables to make sure they
// do not declare the variables.
#ifdef DECLARE_DEBUG_BUFFER
	#define DEBUG_BUFFER_EXTERN
#else
	#define DEBUG_BUFFER_EXTERN	extern
#endif

/// Version of the debug console module
#define DEBUG_BUFFER_STR_VERSION "1.01"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Variables for a global temporary buffer that can be used by the debug console commands for temporary operations
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Temporary buffer that can be used by all screens.
DEBUG_BUFFER_EXTERN char debug_buffer[DEBUG_BUFFER_SIZE];

#endif /* DEBUG_BUFFER_H_ */
