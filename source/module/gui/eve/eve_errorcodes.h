/**
 * 	@file eve_errorcodes.h
 *  @copyright Urheberrecht 2015-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Contains definitions of possible errorcodes for the eve.
 *
 *  @version	1.00 (13.10.2015)
 *  	- Intial release
 *
 ******************************************************************************/

#ifndef EVE_ERRORCODES_H_
#define EVE_ERRORCODES_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI

typedef enum
{
	/// If the initialization at the beginning fails, this code is thrown at the end (the error responsible for failing is thrown before this error).
	EVE_ERROR_INITIALIZATION_FAILED = 0x000,

	/// During initialization, the chip version is checked. If it is wrong, the initialization failed.
	EVE_ERROR_INVALID_CHIP_VERSION = 0x0001,

	/// If a coprocessor error occured, the chip is reinitialized. The display may not work properly then.
	EVE_ERROR_INVALID_COPROCESSOR_ERROR = 0x0002,

	/// The displaytype parameter is invalid.
	EVE_ERROR_INVALID_DISPLAYTYPE = 0x0003,

	/// After EVE_ERROR_INVALID_COPROCESSOR_ERROR the module tries to reinitialize the eve chip. If this failed, the eve cannot be used anymore.
	EVE_ERROR_REINITIALIZATION_FAILED = 0x0004,

	/// Failed to read the EDID data from the display. This is only used for SLD displays.
	EVE_ERROR_READING_EDID_FAILED = 0x0005,

}EVE_ERROR;

#endif

#endif /* EVE_ERRORCODES_H_ */
