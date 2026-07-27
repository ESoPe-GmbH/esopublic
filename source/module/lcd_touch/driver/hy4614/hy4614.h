/**
 * @file hy4614.device
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

#ifndef __MODULE_HY4614_H_
#define __MODULE_HY4614_H_

#include "module_public.h"
#if MODULE_ENABLE_LCD_TOUCH_DRIVER_HY4614 && MODULE_ENABLE_LCD_TOUCH
#include "module/enum/function_return.h"
#include "module/lcd_touch/lcd_touch.h"
#include "module/comm/i2c/i2c.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct hy4614_hw_config_s
{
    /// Pointer to the i2c handler used for communication
    i2c_t* i2c;
    /// I/O pin for reset. Set to PIN_NONE if /RESET is not connected
    MCU_IO_PIN io_reset;
    /// I/O interrupt handler. Can be NULL if INT pin is not connected
    mcu_io_int_t io_int;
    /// Set to true if touch is handled in protothread. This can be false for slint, since ESP Slint is handling touch on its own.
    bool use_protothread;

}hy4614_hw_config_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

extern const struct lcd_touch_interface_s hy4614_lcd_touch_interface;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Create the device handler for HY4614 tocuh driver.
**/
lcd_touch_device_handle_t hy4614_create(const hy4614_hw_config_t* hw);

/**
 * @brief Delete Touch
 *
 * @param device: Touch device handle
 *
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T hy4614_free(lcd_touch_device_handle_t device);

/**
 * @brief Read data from touch controller (mandatory)
 *
 * @note This function is usually blocking.
 *
 * @param device: Touch device handle
 *
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T hy4614_read_data(lcd_touch_device_handle_t device);

/**
 * @brief Get coordinates from touch controller (mandatory)
 *
 * @param device: Touch device handle
 * @param x: Array of X coordinates
 * @param y: Array of Y coordinates
 * @param strength: Array of strengths
 * @param point_num: Count of points touched (equals with count of items in x and y array)
 * @param max_point_num: Maximum count of touched points to return (equals with max size of x and y array)
 *
 * @return
 *      - Returns true, when touched and coordinates readed. Otherwise returns false.
 */
bool hy4614_get_xy(lcd_touch_device_handle_t device, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);

#endif // MODULE_ENABLE_HY4614

#endif /* __MODULE_HY4614_H_ */