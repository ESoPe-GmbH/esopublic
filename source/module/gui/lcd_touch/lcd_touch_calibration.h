/**
 * @file lcd_touch_calibration.h
 * @author Tim Koczwara
 * 
 * @brief 
 * 
 * @version 1.00 (10.07.2024)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_LCD_TOUCH_CALIBRATION_H_
#define __MODULE_LCD_TOUCH_CALIBRATION_H_

#include "module_public.h"
#if MODULE_ENABLE_LCD_TOUCH
#include "module/enum/function_return.h"
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

bool lcd_touch_calibration_process_etml035023udra(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);

bool lcd_touch_calibration_process_etml043023udra(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);

bool lcd_touch_calibration_process_etml050023udra(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);

bool lcd_touch_calibration_process_etml070023udra(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);

#endif // MODULE_ENABLE_LCD_TOUCH_CALIBRATION

#endif /* __MODULE_LCD_TOUCH_CALIBRATION_H_ */