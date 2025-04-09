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

typedef enum eve_status_e
{
	/// The eve module is not initialized. This is the default value of the eve module.
	/// This is used to check if the eve module is initialized or not.
	EVE_STATUS_NOT_INITIALIZED = 0,

	/// The eve module is initialized and can be used.
	EVE_STATUS_OK,

	/// During initialization, the chip version is checked. If it is wrong, the initialization failed.
	EVE_STATUS_INVALID_CHIP_VERSION,

	/// If a coprocessor error occured, the chip is reinitialized. The display may not work properly then.
	EVE_STATUS_INVALID_COPROCESSOR_ERROR,

	/// The displaytype parameter is invalid.
	EVE_STATUS_INVALID_DISPLAYTYPE,

	/// After EVE_STATUS_INVALID_COPROCESSOR_ERROR the module tries to reinitialize the eve chip. If this failed, the eve cannot be used anymore.
	EVE_STATUS_REINITIALIZATION_FAILED,

	/// Failed to read the EDID data from the display. This is only used for SLD displays.
	EVE_STATUS_READING_EDID_FAILED,

}EVE_STATUS_T;

#endif

#endif /* EVE_ERRORCODES_H_ */
