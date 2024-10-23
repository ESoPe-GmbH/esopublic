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

#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
#include "module/gui/lcd_touch/driver/st1633i/st1633i.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct display_sld_hardware_s
{
    /// Hardware configuration for the connection to the display.
    display_common_hardware_t display;
    /// Hardware configuration for a PWM used for the backlight of the display.
    mcu_pwm_config_hw_t backlight;
#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
    /// Hardware configuration for the touch interface I2C is used for touch and eeprom.
    st1633i_hw_config_t touch;
#else
    struct
    {
        /// Pointer to the i2c handler used for communication with the eeprom
        i2c_t* i2c;
    }touch;
#endif
}display_sld_hardware_t;

typedef struct display_sld_s
{
    /// @brief Handle used for display functions.
    display_handle_t display;
    /// @brief Handle used for controlling the backlight.
    mcu_pwm_t backlight;
#if MODULE_ENABLE_LCD_TOUCH
    /// @brief Handle used for touch functions.
    lcd_touch_handle_t touch;
#endif
    /// Horizontal resolution of the display
    int32_t width;
    /// Vertical resolution of the display
    int32_t height;
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
 * @param config        Hardware configuration for the connection to the display and touch.
 * @return display_sld_handle_t     Handle for display and touch 
 */
display_sld_handle_t display_sld_init_hardware(const display_sld_hardware_t* config);
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

/**
 * @brief Sets the backlight brightness in percent.
 * 
 * @param device        Handle of the display.
 * @param pwm           Brightness in percent
 */
void display_sld_set_backlight(display_sld_handle_t device, float pwm);

#endif // MODULE_ENABLE_DISPLAY

#endif // __DISPLAY__SLD__DISPLAY_SLD_H__