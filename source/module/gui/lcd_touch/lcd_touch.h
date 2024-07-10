/**
 * @file lcd_touch.h
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

#ifndef __MODULE_LCD_TOUCH_H_
#define __MODULE_LCD_TOUCH_H_

#include "module_public.h"
#if MODULE_ENABLE_LCD_TOUCH
#include "module/enum/function_return.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct lcd_touch_device_s* lcd_touch_device_handle_t;

typedef struct lcd_touch_s* lcd_touch_handle_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct lcd_touch_flags_s
{
    uint8_t swap_xy : 1;
    uint8_t mirror_x : 1;
    uint8_t mirror_y : 1;
};

struct lcd_touch_config_s
{
    uint16_t x_max;

    uint16_t y_max;

    bool (*process_xy)(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);
 
    struct lcd_touch_flags_s flags;
};

struct lcd_touch_interface_s
{
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
    FUNCTION_RETURN_T (*enter_sleep)(lcd_touch_device_handle_t device);

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
    FUNCTION_RETURN_T (*exit_sleep)(lcd_touch_device_handle_t device);

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
    FUNCTION_RETURN_T (*read_data)(lcd_touch_device_handle_t device);

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
    bool (*get_xy)(lcd_touch_device_handle_t device, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);

    /**
     * @brief Change flags for mirror and swapping.
     * 
     * @param device Touch device handle
     * @param flags Flags to set for the touch driver
     * 
     * @return
     *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
     */
    FUNCTION_RETURN_T (*set_flags)(lcd_touch_device_handle_t device, struct lcd_touch_flags_s flags);

    /**
     * @brief Get flags for mirror and swapping.
     * 
     * @param device Touch device handle
     * @param flags Get current flags that are set in the touch driver.
     * 
     * @return
     *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
     */
    FUNCTION_RETURN_T (*get_flags)(lcd_touch_device_handle_t device, struct lcd_touch_flags_s* flags);

    /**
     * @brief Delete Touch
     *
     * @param device: Touch device handle
     *
     * @return
     *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
     */
    FUNCTION_RETURN_T (*del)(lcd_touch_device_handle_t device);
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Intializes the module
**/
FUNCTION_RETURN_T lcd_touch_create(lcd_touch_device_handle_t device, const struct lcd_touch_interface_s* interface, const struct lcd_touch_config_s* config, lcd_touch_handle_t* handle);


FUNCTION_RETURN_T lcd_touch_free(lcd_touch_handle_t* handle);

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
FUNCTION_RETURN_T lcd_touch_enter_sleep(lcd_touch_handle_t h);

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
FUNCTION_RETURN_T lcd_touch_exit_sleep(lcd_touch_handle_t h);

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
FUNCTION_RETURN_T lcd_touch_read_data(lcd_touch_handle_t h);

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
bool lcd_touch_get_xy(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);

/**
 * @brief Change flags for mirror and swapping.
 * 
 * @param device Touch device handle
 * @param flags Flags to set for the touch driver
 * 
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T lcd_touch_set_flags(lcd_touch_handle_t h, struct lcd_touch_flags_s flags);

/**
 * @brief Get flags for mirror and swapping.
 * 
 * @param device Touch device handle
 * @param flags Get current flags that are set in the touch driver.
 * 
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T lcd_touch_get_flags(lcd_touch_handle_t h, struct lcd_touch_flags_s* flags);

/**
 * @brief Delete Touch
 *
 * @param device: Touch device handle
 *
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T lcd_touch_del(lcd_touch_handle_t h);

#endif // MODULE_ENABLE_LCD_TOUCH

#endif /* __MODULE_TOUCH_H_ */