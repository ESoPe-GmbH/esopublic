/**
 * 	@file module_public_include.h
 * 	@copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Felix Herrmann
 *
 *  @brief		Includes all modules that are enabled in module_public.h
 *
 *  @version	1.00 (09.04.2024)
 *  	- Initial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_MODULE_PUBLIC_INCLUDE_H_
#define MODULE_MODULE_PUBLIC_INCLUDE_H_

#include "module_public.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Include of modules based on activated modules
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
#if MODULE_ENABLE_COMM
	#include "module/comm/comm.h"
	#include "module/comm/dbg.h"
#endif

#if MODULE_ENABLE_CONSOLE
	#include "module/console/console.h"
#endif

#if MODULE_ENABLE_CONVERT_BASE64
	#include "module/convert/base64.h"
#endif

#if MODULE_ENABLE_CONVERT_MATH
	#include "module/convert/math.h"
#endif

#if MODULE_ENABLE_CONVERT_SORT
	#include "module/convert/sort.h"
#endif

#if MODULE_ENABLE_CONVERT_STRING
	#include "module/convert/string.h"
#endif

#if MODULE_ENABLE_CONVERT_SWAP
	#include "module/convert/swap.h"
#endif

#if MODULE_ENABLE_CRC
	#include "module/crc/crc.h"
#endif

#if MODULE_ENABLE_ENUM
	#include "module/enum/function_return.h"
#endif

#if MODULE_ENABLE_FIFO
	#include "module/fifo/fifo.h"
#endif

#if MODULE_ENABLE_LED
	#include "module/led/led.h"
#endif

#endif /* MODULE_MODULE_PUBLIC_H_ */
