/**
 * @file display_internal_esp32s3.h
 * @author Tim Koczwara
 * 
 * @brief 
 * 
 * @version 1.00 (09.04.2025)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_DISPLAY_INTERNAL_ESP32S3_H_
#define __MODULE_DISPLAY_INTERNAL_ESP32S3_H_

#include "module_public.h"

#if MODULE_ENABLE_DISPLAY && MCU_TYPE == MCU_ESP32

#include "display_internal.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#include "driver/spi_master.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#ifndef DISPLAY_MAX_PCLK
#if CONFIG_IDF_TARGET_ESP32P4
/// Maximum clock for PCLK on ESP32P4 is not known yet
#define DISPLAY_MAX_PCLK        20000000
#elif CONFIG_IDF_TARGET_ESP32S3
/// Maximum clock for PCLK is 40MHz on ESP32S3, but only works well with 14MHz. Define other values in config to try out optimizations.
#define DISPLAY_MAX_PCLK        14000000
#endif
#endif

#ifndef DISPLAY_NUM_FB
/// Number of frame buffers the display uses
#define DISPLAY_NUM_FB          0
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct display_mcu_data_s
{
    /// Pointer to original config
    const display_common_hardware_t* config;
    /// Handle for the display, because it is needed in the callback for transaction done.
    display_handle_t display;
    /// SPI device handle for configuring RGB+SPI displays.
    spi_device_handle_t spi;
    /// Panel handle needed for calling the esp_lcd_panel functions.
    esp_lcd_panel_handle_t panel_handle;
    /// Panel configuration that is set during init.
    esp_lcd_rgb_panel_config_t panel_config;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    /// Event callbacks for the panel
    esp_lcd_rgb_panel_event_callbacks_t panel_event_callbacks;
#endif
    /// @brief Pointer to frame buffers
    void* fb[DISPLAY_NUM_FB];
    /// User data which would be passed to on_event's user_ctx. Leave NULL if you do not need it.
    void *on_event_ctx;
    /// Callback invoked when one frame buffer has transferred done.
    display_event_cb_t f_on_event;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


#endif // MODULE_ENABLE_DISPLAY_INTERNAL_ESP32S3

#endif /* __MODULE_DISPLAY_INTERNAL_ESP32S3_H_ */