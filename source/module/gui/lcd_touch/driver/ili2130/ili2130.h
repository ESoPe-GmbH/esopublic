/**
 * @file ili2130.device
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

#ifndef __MODULE_ILI2130_H_
#define __MODULE_ILI2130_H_

#include "module_public.h"
#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ILI2130 && MODULE_ENABLE_LCD_TOUCH
#include "module/enum/function_return.h"
#include "module/gui/lcd_touch/lcd_touch.h"
#include "module/comm/i2c/i2c.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct ili2130_hw_config_s
{
    /// Pointer to the i2c handler used for communication
    i2c_t* i2c;
    /// I/O pin for reset. Set to PIN_NONE if /RESET is not connected
    MCU_IO_PIN io_reset;
    /// I/O interrupt handler. Can be NULL if INT pin is not connected
    mcu_io_int_t io_int;
    /// Set to true if touch is handled in protothread. This can be false for slint, since ESP Slint is handling touch on its own.
    bool use_protothread;

}ili2130_hw_config_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

extern const struct lcd_touch_interface_s ili2130_lcd_touch_interface;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Create the device handler for ILI2130 tocuh driver.
**/
lcd_touch_device_handle_t ili2130_create(const ili2130_hw_config_t* hw);

/**
 * @brief Delete Touch
 *
 * @param device: Touch device handle
 *
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T ili2130_free(lcd_touch_device_handle_t device);

/**
 * @brief Set touch controller into sleep mode
 *
 * @note This function is usually blocking.
 *
 * @param device: Touch device handle
 *
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T ili2130_enter_sleep(lcd_touch_device_handle_t device);

/**
 * @brief set touch controller into normal mode
 *
 * @note This function is usually blocking.
 *
 * @param device: Touch device handle
 *
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T ili2130_exit_sleep(lcd_touch_device_handle_t device);

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
FUNCTION_RETURN_T ili2130_read_data(lcd_touch_device_handle_t device);

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
bool ili2130_get_xy(lcd_touch_device_handle_t device, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);

/**
 * @brief Change flags for mirror and swapping.
 * 
 * @param device Touch device handle
 * @param flags Flags to set for the touch driver
 * 
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T ili2130_set_flags(lcd_touch_device_handle_t device, struct lcd_touch_flags_s flags);

/**
 * @brief Get flags for mirror and swapping.
 * 
 * @param device Touch device handle
 * @param flags Get current flags that are set in the touch driver.
 * 
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T ili2130_get_flags(lcd_touch_device_handle_t device, struct lcd_touch_flags_s* flags);

#endif // MODULE_ENABLE_ILI2130

#endif /* __MODULE_ILI2130_H_ */