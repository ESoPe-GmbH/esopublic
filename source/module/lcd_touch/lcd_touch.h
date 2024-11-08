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
#include "module/list/list.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Touch device handle
typedef struct lcd_touch_device_s* lcd_touch_device_handle_t;

/// @brief Touch handle
typedef struct lcd_touch_s* lcd_touch_handle_t;

/// @brief Configuration structure for registering an observer.
typedef struct lcd_touch_observer_config_s lcd_touch_observer_config_t;

/// @brief Event structure that is used when an observer is triggered.
typedef struct lcd_touch_observer_event_s lcd_touch_observer_event_t;

/// @brief Callback for a touch event
/// @param h        Touch handle
/// @param event    Pointer to the event that triggered the observer
typedef void (*lcd_touch_observer_cb_t)(lcd_touch_handle_t h, lcd_touch_observer_event_t* event);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Configuration structure for registering an observer.
struct lcd_touch_observer_config_s
{
    /// @brief Callback function that needs to be called when event occurs
    lcd_touch_observer_cb_t f_cb;
    /// @brief Custom user pointer to register with the observer
    void* user_ctx;
    /// @brief If set to true, an event will be triggered for every internal touch event. If clear only finger touch and release will be triggered.
    bool track_finger;
};

/// @brief Event structure that is used when an observer is triggered.
struct lcd_touch_observer_event_s
{
    /// @brief Custom user pointer that was registered with the observer
    void* user_ctx;
    /// @brief Points to a list of x-coordinates where the display is touched.
    uint16_t *x;
    /// @brief Points to a list of y-coordinates where the display is touched.
    uint16_t *y; 
    /// @brief Points to a list of strength with which the display is pressed.
    uint16_t *strength;
    /// @brief Number of fingers that touch the display. If 0, the finger were removed.
    uint8_t point_num;
};

/// @brief Flags for configuring the touch
struct lcd_touch_flags_s
{
    /// @brief Swap the x and y coordinates. Is needed for landscape/portrait mode
    uint8_t swap_xy : 1;
    /// @brief Mirror the x coordinates, needed for rotation.
    uint8_t mirror_x : 1;
    /// @brief Mirror the y coordinates, needed for rotation.
    uint8_t mirror_y : 1;
};

/// @brief Configured the touch screen attached to the display
struct lcd_touch_config_s
{
    /// @brief Maximum x coordinate
    uint16_t x_max;
    /// @brief Maximum y coordinate
    uint16_t y_max;
    /// @brief Optional function that is used to process touch coordinates. This is for example needed for a user calibration of the touch or for specific displays.
    bool (*process_xy)(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);
    /// @brief Flags for configuring the touch
    struct lcd_touch_flags_s flags;
};

/// @brief Specify the touch interface for the device.
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

/**
 * @brief Frees the touch handle and clears the reference to NULL.
 * 
 * @param handle    Pointer to a handle that is freed and cleared to NULL.
 * @return FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T lcd_touch_free(lcd_touch_handle_t* handle);

/**
 * @brief Add an observer to the touch module.
 * 
 * @param h         Touch handle
 * @param config    Content is copied into internal observer list.
 * 
 * @retval FUNCTION_RETURN_OK on success
 * @retval FUNCTION_RETURN_INSUFFICIENT_MEMORY when not enough memory is available to copy config
 * @retval FUNCTION_RETURN_PARAM_ERROR when @c h or @c config are @c NULL.
 */
FUNCTION_RETURN_T lcd_touch_add_observer(lcd_touch_handle_t h, const lcd_touch_observer_config_t* config);

/**
 * @brief Remove an observer from the touch module.
 * 
 * @param h         Touch handle
 * @param config    Specify @c user_ctx and/or @c f_cb that should be removed from the observer list.
 * 
 * @retval FUNCTION_RETURN_OK on success
 * @retval FUNCTION_RETURN_PARAM_ERROR when @c h or @c config are @c NULL.
 * @retval FUNCTION_RETURN_NOT_FOUND when nothing was found to remove.
 */
FUNCTION_RETURN_T lcd_touch_remove_observer(lcd_touch_handle_t h, const lcd_touch_observer_config_t* config);

/**
 * @brief Set touch controller into sleep mode
 *
 * @note This function is usually blocking.
 *
 * @param h: Touch handle
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
 * @param h: Touch handle
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
 * @param h: Touch handle
 *
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T lcd_touch_read_data(lcd_touch_handle_t h);

/**
 * @brief Get coordinates from touch controller (mandatory)
 *
 * @param h: Touch handle
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
 * @param h Touch handle
 * @param flags Flags to set for the touch driver
 * 
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T lcd_touch_set_flags(lcd_touch_handle_t h, struct lcd_touch_flags_s flags);

/**
 * @brief Get flags for mirror and swapping.
 * 
 * @param h Touch handle
 * @param flags Get current flags that are set in the touch driver.
 * 
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T lcd_touch_get_flags(lcd_touch_handle_t h, struct lcd_touch_flags_s* flags);

/**
 * @brief Delete Touch
 *
 * @param h: Touch handle
 *
 * @return
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T lcd_touch_del(lcd_touch_handle_t h);

/**
 * @brief Get the configured dimensions for the touch panel.
 * 
 * @param h     Touch handle
 * @param x_max Maximum X-Coordinate
 * @param y_max Maximum Y-Coordinate
 * @return 
 *      - FUNCTION_RETURN_OK on success, otherwise returns FUNCTION_RETURN_xxx
 */
FUNCTION_RETURN_T lcd_touch_get_dimensions(lcd_touch_handle_t h, uint16_t* x_max, uint16_t* y_max);

#endif // MODULE_ENABLE_LCD_TOUCH

#endif /* __MODULE_TOUCH_H_ */