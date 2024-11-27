/**
 * 	@file display_internal.h
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

#ifndef __DISPLAY__INTERNAL__DISPLAY_INTERNAL_H__
#define __DISPLAY__INTERNAL__DISPLAY_INTERNAL_H__

#include "module_public.h"

#if MODULE_ENABLE_DISPLAY

#include "../display_common.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Configures device specific settings.
 * 
 * Is set by the display driver internally, when initializing it, since these settings depend on the physical connected display.
 */
typedef union display_device_config_s
{
    union
    {
        /// For displays using the intel 8080 interface
        struct
        {
            uint8_t dummy_need_to_be_replaced_by_actual_data;
            // TBD
        }intel8080;
        /**
         * @brief For displays using the RGB interface.
         * @verbatim
         *                                                 Total Width
         *                             <--------------------------------------------------->
         *                       HSYNC width HBP             Active Width                HFP
         *                             <---><--><--------------------------------------><--->
         *                         ____    ____|_______________________________________|____|
         *                             |___|   |                                       |    |
         *                                     |                                       |    |
         *                         __|         |                                       |    |
         *            /|\    /|\  |            |                                       |    |
         *             | VSYNC|   |            |                                       |    |
         *             |Width\|/  |__          |                                       |    |
         *             |     /|\     |         |                                       |    |
         *             |  VBP |      |         |                                       |    |
         *             |     \|/_____|_________|_______________________________________|    |
         *             |     /|\     |         | / / / / / / / / / / / / / / / / / / / |    |
         *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
         *    Total    |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
         *    Height   |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
         *             |Active|      |         |/ / / / / / / / / / / / / / / / / / / /|    |
         *             |Heigh |      |         |/ / / / / / Active Display Area / / / /|    |
         *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
         *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
         *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
         *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
         *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
         *             |     \|/_____|_________|_______________________________________|    |
         *             |     /|\     |                                                      |
         *             |  VFP |      |                                                      |
         *            \|/    \|/_____|______________________________________________________|
         * @endverbatim
         */
        struct
        {
            /// Frequency of pixel clock
            unsigned int pclk_hz;       
            /// Horizontal resolution, i.e. the number of pixels in a line
            unsigned int h_res;             
            /// Vertical resolution, i.e. the number of lines in the frame
            unsigned int v_res;
            /// Horizontal sync width, unit: PCLK period
            unsigned int hsync_pulse_width;
            /// Horizontal back porch, number of PCLK between hsync and start of line active data
            unsigned int hsync_back_porch;
            /// Horizontal front porch, number of PCLK between the end of active data and the next hsync
            unsigned int hsync_front_porch;
            /// Vertical sync width, unit: number of lines
            unsigned int vsync_pulse_width;
            /// Vertical back porch, number of invalid lines between vsync and start of frame
            unsigned int vsync_back_porch;
            /// Vertical front porch, number of invalid lines between the end of frame and the next vsync
            unsigned int vsync_front_porch;
            struct 
            {
                /// The hsync signal is low in IDLE state
                unsigned int hsync_idle_low: 1;  
                /// The vsync signal is low in IDLE state
                unsigned int vsync_idle_low: 1;  
                /// The de signal is high in IDLE state
                unsigned int de_idle_high: 1;    
                /// Whether the display data is clocked out at the falling edge of PCLK
                unsigned int pclk_active_neg: 1;
                /// The PCLK stays at high level in IDLE phase
                unsigned int pclk_idle_high: 1; 
            } flags;
        } rgb;
    };
}display_device_config_t;

/**
 * @brief Configures mcu specific settings for rgb interface.
 * 
 * Is set by the display driver internally. U
 */
typedef union display_mcu_rgb_config_s
{
    union
    {
        /// For displays using the intel 8080 interface
        struct
        {
            struct
            {
                /// Maximum transfer size, this determines the length of internal DMA link
                size_t max_transfer_bytes;
                /// Alignment for framebuffer that allocated in SRAM
                size_t sram_trans_align;
                /// Alignment for framebuffer that allocated in PSRAM 
                size_t psram_trans_align;
            }esp32s3;
        }intel8080;
        /// For displays using the RGB interface.
        struct
        {
            struct
            {
                struct 
                {
                    /// If this flag is enabled, the host won't refresh the LCD if nothing changed in host's frame buffer (this is usefull for LCD with built-in GRAM)
                    unsigned int relax_on_idle: 1; 
                    /// If this flag is enabled, the frame buffer will be allocated from PSRAM preferentially
                    unsigned int fb_in_psram: 1;  
                    /// If this flag is enabled, a low level of display control signal can turn the screen on; vice versa
                    unsigned int disp_active_low: 1;      
                } flags;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
                /// DMA burst size, in bytes
                size_t dma_burst_size;
#else
                /// Alignment for framebuffer that allocated in SRAM
                size_t sram_trans_align;
                /// Alignment for framebuffer that allocated in PSRAM 
                size_t psram_trans_align;
#endif
                /// When using psram you can create a bounce buffer that is used to transmit data to the display from the sram.
                /// If used, the esp copies the data from psram into sram before the data is sent to the display.
                /// Set the value in percent based on the resolution of the display.
                float bounce_buffer_size_percent;
            }esp32; // ESP32S3 and P4
        }rgb;
    };
}display_mcu_config_t;

/// Handle for mcu data structure.
typedef struct display_mcu_data_s* display_mcu_handle_t;

/**
 * @brief Structure used by the displays to initialize the display driver.
 */
struct display_data_s
{
    /// Handle for mcu functions
    display_mcu_handle_t mcu;
    /// MCU configuration is filled by specific display
    display_mcu_config_t mcu_config;
    /// Device configuration is filled by specific display
    display_device_config_t device_config;
    /// @brief Setting for swap xy
    bool swap_xy;
    /// @brief Setting to mirror x
    bool mirror_x;
    /// @brief Setting to mirror y
    bool mirror_y;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initialize the mcu specific settings based on the configuration of the hardware and display configurations.
 * 
 * @param config            Hardware configuration done by the user in board.c and given in display_common_init.
 * @param display           Display handle containing MCU configuration created by the display itself during its initialization. Contains also device specific configuration created by the display driver during its initialization.
 * @return display_mcu_handle_t     Handle for the mcu specific functions.
 */
display_mcu_handle_t display_mcu_init(const display_common_hardware_t* config, display_handle_t display);
/**
 * @brief Set a callback for display events.
 * 
 * @param mcu               Handle gained in @see display_mcu_init.
 * @param f                 Function that is triggered when a display event occurs. On ESP32, you should mark it with IRAM_ATTR if you activate IRAM safety for rgb
 * @param ctx               Pointer to user context that is given when callback is called. Can be NULL.
 */
void display_mcu_set_event_callback(display_mcu_handle_t mcu, display_event_cb_t f, void* ctx);
/**
 * @brief Function for RGB+SPI to write a configuration byte
 * 
 * @param mcu               Handle gained in @see display_mcu_init.
 * @param command           Command to write via SPI.
 */
void display_mcu_rgb_spi_write_command(display_mcu_handle_t mcu, unsigned char command);
/**
 * @brief Function for RGB+SPI to write a data byte
 * 
 * @param mcu               Handle gained in @see display_mcu_init.
 * @param data              Data to write via SPI.
 */
void display_mcu_rgb_spi_write_data(display_mcu_handle_t mcu, unsigned char data);
/**
 * @brief Reset LCD panel
 *
 * @note Panel reset must be called before attempting to initialize the panel using `display_mcu_init()`.
 *
 * @param[in] display Display handle, which is created by `display_mcu_init()`
 * @return  FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_mcu_reset(display_mcu_handle_t display);
/**
 * @brief Initialize LCD panel
 *
 * @note Before calling this function, make sure the LCD panel has finished the `reset` stage by `display_mcu_reset()`.
 *
 * @param[in] display Display handle, which is created by `display_mcu_init()`
 * @return  FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_mcu_init_device(display_mcu_handle_t display);
/**
 * @brief Deinitialize the LCD panel
 *
 * @param[in] display Display handle, which is created by `display_mcu_init()`
 * @return  FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_mcu_del_device(display_mcu_handle_t display);
/**
 * @brief Draw bitmap on display using mcu functions.
 *
 * @param[in] display Display handle, which is created by `display_mcu_init()`
 * @param[in] x_start Start index on x-axis (x_start included)
 * @param[in] y_start Start index on y-axis (y_start included)
 * @param[in] x_end End index on x-axis (x_end not included)
 * @param[in] y_end End index on y-axis (y_end not included)
 * @param[in] color_data RGB color data that will be dumped to the specific window range
 * @return  FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_mcu_draw_bitmap(display_mcu_handle_t display, int x_start, int y_start, int x_end, int y_end, const void *color_data);
/**
 * @brief Mirror the LCD panel on specific axis
 *
 * @note Combined with `display_device_swap_xy()`, one can realize screen rotation
 *
 * @param[in] display Display handle, which is created by `display_mcu_init()`
 * @param[in] mirror_x Whether the panel will be mirrored about the x axis
 * @param[in] mirror_y Whether the panel will be mirrored about the y axis
 * @return
 *          - FUNCTION_RETURN_OK on success
 *          - FUNCTION_RETURN_UNSUPPORTED if this function is not supported by the panel
 */
FUNCTION_RETURN_T display_mcu_mirror(display_mcu_handle_t display, bool mirror_x, bool mirror_y);
/**
 * @brief Swap/Exchange x and y axis
 *
 * @note Combined with `display_device_mirror()`, one can realize screen rotation
 *
 * @param[in] display Display handle, which is created by `display_mcu_init()`
 * @param[in] swap_axes Whether to swap the x and y axis
 * @return
 *          - FUNCTION_RETURN_OK on success
 *          - FUNCTION_RETURN_UNSUPPORTED if this function is not supported by the panel
 */
FUNCTION_RETURN_T display_mcu_swap_xy(display_mcu_handle_t display, bool swap_axes);
/**
 * @brief Set extra gap in x and y axis
 *
 * The gap is the space (in pixels) between the left/top sides of the LCD panel and the first row/column respectively of the actual contents displayed.
 *
 * @note Setting a gap is useful when positioning or centering a frame that is smaller than the LCD.
 *
 * @param[in] display Display handle, which is created by `display_mcu_init()`
 * @param[in] x_gap Extra gap on x axis, in pixels
 * @param[in] y_gap Extra gap on y axis, in pixels
 * @return
 *          - FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_mcu_set_gap(display_mcu_handle_t display, int x_gap, int y_gap);
/**
 * @brief Invert the color (bit-wise invert the color data line)
 *
 * @param[in] display Display handle, which is created by `display_mcu_init()`
 * @param[in] invert_color_data Whether to invert the color data
 * @return
 *          - FUNCTION_RETURN_OK on success
 */
FUNCTION_RETURN_T display_mcu_invert_color(display_mcu_handle_t display, bool invert_color_data);
/**
 * @brief Turn off the display
 *
 * @param[in] display Display handle, which is created by `display_mcu_init()`
 * @param[in] off Whether to turn off the screen
 * @return
 *          - FUNCTION_RETURN_OK on success
 *          - FUNCTION_RETURN_UNSUPPORTED if this function is not supported by the panel
 */
FUNCTION_RETURN_T display_mcu_disp_off(display_mcu_handle_t display, bool off);
/**
 * @brief Manually trigger once transmission of the frame buffer to the LCD panel.
 * 
 * @note Should only be used when @c relax_on_idle was set to true.
 * 
 * @param[in] display Display handle, which is created by @c display_mcu_init()
 * @return
 *          - FUNCTION_RETURN_OK on success
 *          - FUNCTION_RETURN_UNSUPPORTED if this function is not supported by the panel
 */
FUNCTION_RETURN_T display_mcu_refresh(display_mcu_handle_t display);
/**
 * @brief Get an allocated frame buffer of the LCD panel.
 * 
 * @param[in] display Display handle, which is created by @c display_mcu_init()
 * @param index_fb      Index of the frame buffer (0 to (DISPLAY_NUM_FB (default 2)) - 1)
 * @param fb            Pointer to the pointer where the frame buffer pointer should be stored.
 *          - FUNCTION_RETURN_OK on success
 *          - FUNCTION_RETURN_PARAM_ERROR if any pointer is NULL or index_fb is out of range.
 *          - FUNCTION_RETURN_UNSUPPORTED if this function is not supported by the panel
 */
FUNCTION_RETURN_T display_mcu_get_framebuffer(display_mcu_handle_t display, uint8_t index_fb, void** fb);

#endif // MODULE_ENABLE_DISPLAY

#endif // __DISPLAY__INTERNAL__DISPLAY_INTERNAL_H__