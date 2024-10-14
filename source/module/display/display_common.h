/**
 * 	@file display_common.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten
 *  @author 	Tim Koczwara
 *
 *  @brief	Defines an enumeration that can be used as generic return values for functions.
 *			
 *  @version	1.00 (19.08.2022)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef __DISPLAY__DISPLAY_COMMON_H__
#define __DISPLAY__DISPLAY_COMMON_H__

#include "module.h"

#if MODULE_ENABLE_DISPLAY

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the function return module
#define DISPLAY_STR_VERSION "1.00"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Enumeration for the different supported interface types
typedef enum display_interface_e
{
    /// Invalid default value to ensure this will be configured explicitly.
    DISPLAY_INTERFACE_INVALID = 0,
    /// RGB interface
    DISPLAY_INTERFACE_RGB,
    /// 8080 interface
    DISPLAY_INTERFACE_8080,
    /// Max value to limit the enum.
    DISPLAY_INTERFACE_MAX
}DISPLAY_INTERFACE_T;

/// Enumeration for the different display driver to call in @see display_common_init.
typedef enum display_device_e
{
    /// Invalid default value to ensure this will be configured explicitly.
    DISPLAY_DEVICE_INVALID = 0,
#if DISPLAY_ENABLE_SLD
    /// Smart Line Display will be used.
    DISPLAY_DEVICE_SLD,
#endif
#if DISPLAY_ENABLE_KD021WVFPD003
    /// KD021WVFPD003 display will be used.
    DISPLAY_DEVICE_KD021WVFPD003,
#endif
    /// Max value to limit the enum.
    DISPLAY_DEVICE_MAX
}DISPLAY_DEVICE_T;

/// Enumeration for the different display driver to call in @see display_common_init.
typedef enum display_event_e
{
    // TBD

    /// Max value to limit the enum.
    DISPLAY_EVENT_MAX
}DISPLAY_EVENT_T;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Handle for the display data. Struct is used internally by the display.
typedef struct display_data_s* display_handle_t;

/**
 * @brief Type of RGB LCD panel event data
 */
typedef struct 
{
    /// Type of event that occured.
    DISPLAY_EVENT_T event; 
    // No event data for now.
} display_event_data_t;

/**
 * @brief Declare the prototype of the function that will be invoked when panel IO finishes transferring color data
 *
 * @param[in] panel LCD panel handle, returned from the `display_common_init` function. 
 * @param[in] edata Display event data, fed by driver
 * @param[in] user_ctx User data, passed from `display_lvgl_hardware_t`
 * @return Whether a high priority task has been waken up by this function
 */
typedef bool (*display_frame_trans_done_cb_t)(display_handle_t panel, display_event_data_t *edata, void *user_ctx);

/**
 * @brief Hardware interface structure for connecting a display.
 * 
 * Fill this structure in board.c according to the used hardware. It can then be used by the concrete display initializer to initialize the display.
 * The display will then configure internally the display driver and call mcu functions applying to the display.
 */
typedef struct display_common_hardware_s
{
    /// Select the display device to use. Make sure to enable it first in module_config.h
    DISPLAY_DEVICE_T display;
    /// Select the interface that will be used for the display. Use the corresponding structures of below union according to the interface value.
    DISPLAY_INTERFACE_T interface;
    /// User data which would be passed to on_frame_trans_done's user_ctx. Leave NULL if you do not need it.
    void *user_ctx;
    /// Callback invoked when one frame buffer has transferred done.
    display_frame_trans_done_cb_t on_frame_trans_done;
    union
    {
        /// For displays using the intel 8080 interface, fill this.
        struct
        {
            /// GPIO used for D/C line
            MCU_IO_PIN dc;
            /// GPIO used for WR line
            MCU_IO_PIN wr;
            /// Size of the bus in bits. Also indicates the number of used data lines.
            size_t bus_width; 
            /// GPIOs used for data lines
            MCU_IO_PIN data[24]; 
        }intel8080;
        /// For displays using the RGB interface, fill this.
        struct
        {
            /// Common values are 16 (RGB565), 18 (RGB666) and 24 (RGB888)
            uint8_t data_width;
            /// Corresponds to r0 to r7 pins of display. Pins are used according to the set data_width.
            MCU_IO_PIN r[8];
            /// Corresponds to g0 to g7 pins of display. Pins are used according to the set data_width.
            MCU_IO_PIN g[8];
            /// Corresponds to b0 to b7 pins of display. Pins are used according to the set data_width.
            MCU_IO_PIN b[8];
            /// DISP_EN pin of the display.
            MCU_IO_PIN disp_en;
            /// PCLK pin of the display.
            MCU_IO_PIN pclk;
            /// VSYNC pin of the display.
            MCU_IO_PIN vsync;
            /// HSYNC pin of the display.
            MCU_IO_PIN hsync;
            /// DE pin of the display.
            MCU_IO_PIN de;

            // For RGB + SPI displays, the SPI interface must also be selected!

            /// SPI Unit number (as used in mcu_spi_init)
            uint8_t spi_unit;
            /// SPI MOSI pin for RGB+SPI
            MCU_IO_PIN mosi;
            /// SPI MISO pin for RGB+SPI
            MCU_IO_PIN miso;
            /// SPI CLK pin for RGB+SPI
            MCU_IO_PIN clk;
            /// SPI CS pin for RGB+SPI
            MCU_IO_PIN cs;
        } rgb;
    };
}display_common_hardware_t;

/**
 * @brief Initailizes a display driver based on the setting of display in the config.
 * 
 * @param config    Pointer to the configuration that should be used by the display.
 * @return display_handle_t     Handle for the display functions. Is NULL on error.
 */
display_handle_t display_common_init(const display_common_hardware_t* config);

/**
 * @brief Reset LCD panel
 *
 * @note Panel reset must be called before attempting to initialize the panel using `display_mcu_init()`.
 *
 * @param[in] display Display handle, which is created by `display_common_init()`
 * @return  FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_device_reset(display_handle_t display);
/**
 * @brief Initialize LCD panel
 *
 * @note Before calling this function, make sure the LCD panel has finished the `reset` stage by `display_mcu_reset()`.
 *
 * @param[in] display Display handle, which is created by `display_common_init()`
 * @return  FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_device_init(display_handle_t display);
/**
 * @brief Deinitialize the LCD panel
 *
 * @param[in] display Display handle, which is created by `display_common_init()`
 * @return  FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_device_del(display_handle_t display);
/**
 * @brief Draw bitmap on display
 *
 * @param[in] display Display handle, which is created by `display_common_init()`
 * @param[in] x_start Start index on x-axis (x_start included)
 * @param[in] y_start Start index on y-axis (y_start included)
 * @param[in] x_end End index on x-axis (x_end not included)
 * @param[in] y_end End index on y-axis (y_end not included)
 * @param[in] color_data RGB color data that will be dumped to the specific window range
 * @return  FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_device_draw_bitmap(display_handle_t display, int x_start, int y_start, int x_end, int y_end, const void *color_data);
/**
 * @brief Mirror the LCD panel on specific axis
 *
 * @note Combined with `display_device_swap_xy()`, one can realize screen rotation
 *
 * @param[in] display Display handle, which is created by `display_common_init()`
 * @param[in] mirror_x Whether the panel will be mirrored about the x axis
 * @param[in] mirror_y Whether the panel will be mirrored about the y axis
 * @return
 *          - FUNCTION_RETURN_OK on success
 *          - FUNCTION_RETURN_UNSUPPORTED if this function is not supported by the panel
 */
FUNCTION_RETURN_T display_device_mirror(display_handle_t display, bool mirror_x, bool mirror_y);
/**
 * @brief Swap/Exchange x and y axis
 *
 * @note Combined with `display_device_mirror()`, one can realize screen rotation
 *
 * @param[in] display Display handle, which is created by `display_common_init()`
 * @param[in] swap_axes Whether to swap the x and y axis
 * @return
 *          - FUNCTION_RETURN_OK on success
 *          - FUNCTION_RETURN_UNSUPPORTED if this function is not supported by the panel
 */
FUNCTION_RETURN_T display_device_swap_xy(display_handle_t display, bool swap_axes);
/**
 * @brief Set extra gap in x and y axis
 *
 * The gap is the space (in pixels) between the left/top sides of the LCD panel and the first row/column respectively of the actual contents displayed.
 *
 * @note Setting a gap is useful when positioning or centering a frame that is smaller than the LCD.
 *
 * @param[in] display Display handle, which is created by `display_common_init()`
 * @param[in] x_gap Extra gap on x axis, in pixels
 * @param[in] y_gap Extra gap on y axis, in pixels
 * @return
 *          - FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_device_set_gap(display_handle_t display, int x_gap, int y_gap);
/**
 * @brief Invert the color (bit-wise invert the color data line)
 *
 * @param[in] display Display handle, which is created by `display_common_init()`
 * @param[in] invert_color_data Whether to invert the color data
 * @return
 *          - FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_device_invert_color(display_handle_t display, bool invert_color_data);
/**
 * @brief Turn off the display
 *
 * @param[in] display Display handle, which is created by `display_common_init()`
 * @param[in] off Whether to turn off the screen
 * @return
 *          - FUNCTION_RETURN_OK on success
 *          - FUNCTION_RETURN_UNSUPPORTED if this function is not supported by the panel
 */
FUNCTION_RETURN_T display_device_disp_off(display_handle_t display, bool off);
/**
 * @brief Manually trigger once transmission of the frame buffer to the LCD panel.
 *  
 * @param[in] display Display handle, which is created by @c display_common_init()
 * @return
 *          - FUNCTION_RETURN_OK on success
 *          - FUNCTION_RETURN_UNSUPPORTED if this function is not supported by the panel
 */
FUNCTION_RETURN_T display_device_refresh(display_handle_t display);

#endif // MODULE_ENABLE_DISPLAY

#endif /* __DISPLAY__DISPLAY_COMMON_H__ */
