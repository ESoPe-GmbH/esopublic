/**
 * 	@file display_sld.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten
 *  @author 	Tim Koczwara
 *
 *  @brief	Display driver implementation for SLD.
 *			
 *  @version	1.00 (19.08.2022)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef __DISPLAY__SLD__DISPLAY_SLD_H__
#define __DISPLAY__SLD__DISPLAY_SLD_H__

#include "module.h"

#if MODULE_ENABLE_DISPLAY && DISPLAY_ENABLE_SLD

#include "../display_common.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initialize the display driver for sld_init.
 * 
 * @param config        Hardware configuration for the connection to the display.
 * @param eeid          Pointer to eeid where data will be extracted from.
 * @param eeid_length   Length of the eeid data-
 * @return display_handle_t     Handle used for display functions.
 */
display_handle_t display_sld_init(const display_common_hardware_t* config, const uint8_t *eeid, uint8_t eeid_length);

#endif // MODULE_ENABLE_DISPLAY

#endif // __DISPLAY__SLD__DISPLAY_SLD_H__