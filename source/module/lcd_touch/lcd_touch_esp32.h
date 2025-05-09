/**
 * @file lcd_touch_esp32.h
 * @author Tim Koczwara
 * 
 * @brief 
 * 
 * @version 1.00 (15.07.2024)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_LCD_TOUCH_ESP32_H_
#define __MODULE_LCD_TOUCH_ESP32_H_

#include "module_public.h"
#if MODULE_ENABLE_LCD_TOUCH && MCU_TYPE == MCU_ESP32 && defined(KERNEL_USES_SLINT)
#include "module/enum/function_return.h"
#include "esp_lcd_touch.h"
#include "lcd_touch.h"

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


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Intializes the module
**/
FUNCTION_RETURN_T lcd_touch_esp32_create(lcd_touch_handle_t touch_handle, esp_lcd_touch_handle_t* esp_touch_handle);

#endif // MODULE_ENABLE_LCD_TOUCH_ESP32

#endif /* __MODULE_LCD_TOUCH_ESP32_H_ */