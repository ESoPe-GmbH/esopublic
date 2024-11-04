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
#include "module/lcd_touch/lcd_touch.h"
#include "module/comm/i2c/i2c.h"

#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
#include "module/lcd_touch/driver/st1633i/st1633i.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Initializer for @c display_sld_hardware_t
/// @param i2c_touch    Pointer to the i2c interface to use
#define DISPLAY_SLD_HARDWARE_INIT_ESP_S3(i2c_touch)    \
{    \
    .display =     \
    {    \
        .display = DISPLAY_DEVICE_SLD,    \
        .interface = DISPLAY_INTERFACE_RGB,    \
        .rgb =     \
        {    \
            .r = {GPIO14, GPIO20, GPIO13, GPIO19, GPIO12, PIN_NONE, PIN_NONE, PIN_NONE},    \
            .g = {GPIO8, GPIO21, GPIO3, GPIO11, GPIO18, GPIO10, PIN_NONE, PIN_NONE},    \
            .b = {GPIO9, GPIO17, GPIO46, GPIO16, GPIO7, PIN_NONE, PIN_NONE, PIN_NONE},    \
            .pclk = GPIO4,    \
            .de = GPIO5,    \
            .hsync = GPIO15,    \
            .vsync = GPIO6,    \
            .data_width = 16,    \
            .disp_en = GPIO42    \
        },    \
        .on_frame_trans_done = NULL    \
    },    \
    .backlight =     \
    {    \
        .timer_unit = 0,    \
        .timer_channel = 0,    \
        .output_pin = GPIO39    \
    },    \
    .touch =     \
    {            \
        .i2c = i2c_touch,    \
        .io_reset = GPIO2,    \
    }    \
}

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
    /// Screen diagonal of the display in inch as read from EEPROM e.g. "2.4"
    char screen_diagonal[6];
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