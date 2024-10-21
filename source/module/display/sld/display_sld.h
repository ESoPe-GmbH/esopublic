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
#include "module/gui/lcd_touch/lcd_touch.h"
#include "module/comm/i2c/i2c.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct display_sld_s
{
    /// @brief Handle used for display functions.
    display_handle_t display;
    /// @brief Handle used for touch functions.
    lcd_touch_handle_t touch;
}display_sld_t;

/// @brief Handle for the SmartLineDisplay
typedef struct display_sld_s* display_sld_handle_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MODULE_ENABLE_EEPROM
/**
 * @brief Initialize the display driver for sld_init.
 * Reads the i2c
 * 
 * @param config        Hardware configuration for the connection to the display.
 * @param i2c           Pointer to i2c that is used to communicate with touch and eeprom of the display.
 * @return display_sld_handle_t     Handle for display and touch 
 */
display_sld_handle_t display_sld_init_hardware(const display_common_hardware_t* config, i2c_t* i2c);
#endif

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