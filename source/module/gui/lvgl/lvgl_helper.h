/**
 * @file lvgl_helper.h
 * @author Tim Koczwara
 * 
 * @brief 
 * 
 * @version 1.00 (23.10.2024)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_LVGL_HELPER_H_
#define __MODULE_LVGL_HELPER_H_

#include "module.h"
#if defined(KERNEL_USES_LVGL)
#include "module/enum/function_return.h"
#include "module/display/display_common.h"
#include "module/lcd_touch/lcd_touch.h"
#include "lvgl.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Callback that is called from the lvgl task to create the ui.
 * 
 * @param disp  Pointer to the display that is used.
 */
typedef void (*lvgl_helper_ui_init_t)(lv_display_t *disp);

typedef struct lvgl_helper_config_s
{
    /// Handler for the display
    display_handle_t display;
    /// Handler for the touch
    lcd_touch_handle_t touch;
    /// @brief Unit of the timer to use for tick count
    uint8_t timer_unit;
    /// @brief Function to call to create the ui using lvgl functions.
    lvgl_helper_ui_init_t f_ui_init;
}lvgl_helper_config_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Intializes the module
**/
FUNCTION_RETURN lvgl_helper_init(const lvgl_helper_config_t* config);

#endif // MODULE_ENABLE_LVGL_HELPER

#endif /* __MODULE_LVGL_HELPER_H_ */