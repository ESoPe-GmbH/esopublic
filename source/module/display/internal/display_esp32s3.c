/**
 * 	@file display_esp32s3.c
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten
 *  @author 	Tim Koczwara
 *
 *  @brief	MCU specific implementation of the lcd driver for ESP32S3
 *			
 *  @version	1.00 (19.08.2022)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#include "module.h"

#if MODULE_ENABLE_DISPLAY && MCU_TYPE == MCU_ESP32

#include "../internal/display_internal.h"
#include "module/comm/dbg.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "driver/spi_master.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


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
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Called when panel finishes transferring color data
 * 
 * @param[in] panel LCD panel handle, returned from `esp_lcd_new_rgb_panel`
 * @param[in] edata Panel event data, fed by driver
 * @param[in] user_ctx User data, passed from `esp_lcd_rgb_panel_config_t`
 * @return Whether a high priority task has been waken up by this function
 */
static bool _on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define P(pin)        (pin == PIN_NONE ? -1 : pin)

display_mcu_handle_t display_mcu_init(const display_common_hardware_t* config, display_handle_t display)
{
    esp_err_t ret;
    display_mcu_handle_t mcu = mcu_heap_calloc(1, sizeof(struct display_mcu_data_s));
    DBG_ASSERT(mcu, NO_ACTION, NULL, "Cannot create display mcu pointer\n");

    DBG_VERBOSE("Copy mcu config\n");

    if(config->interface == DISPLAY_INTERFACE_RGB)
    {
        mcu->display = display;
        mcu->config = config;
        mcu->panel_config.clk_src = LCD_CLK_SRC_PLL160M;
        mcu->panel_config.timings.pclk_hz = display->device_config.rgb.pclk_hz;
        mcu->panel_config.timings.h_res = display->device_config.rgb.h_res;
        mcu->panel_config.timings.v_res = display->device_config.rgb.v_res;
        mcu->panel_config.timings.hsync_pulse_width = display->device_config.rgb.hsync_pulse_width;
        mcu->panel_config.timings.hsync_back_porch = display->device_config.rgb.hsync_back_porch;
        mcu->panel_config.timings.hsync_front_porch = display->device_config.rgb.hsync_front_porch;
        mcu->panel_config.timings.vsync_pulse_width = display->device_config.rgb.vsync_pulse_width;
        mcu->panel_config.timings.vsync_back_porch = display->device_config.rgb.vsync_back_porch;
        mcu->panel_config.timings.vsync_front_porch = display->device_config.rgb.vsync_front_porch;
        mcu->panel_config.timings.flags.hsync_idle_low = display->device_config.rgb.flags.hsync_idle_low;
        mcu->panel_config.timings.flags.vsync_idle_low = display->device_config.rgb.flags.vsync_idle_low;
        mcu->panel_config.timings.flags.de_idle_high = display->device_config.rgb.flags.de_idle_high;
        mcu->panel_config.timings.flags.pclk_active_neg = display->device_config.rgb.flags.pclk_active_neg;
        mcu->panel_config.timings.flags.pclk_idle_high = display->device_config.rgb.flags.pclk_idle_high;
        mcu->panel_config.data_width = config->rgb.data_width;
        mcu->panel_config.sram_trans_align = display->mcu_config.rgb.esp32s3.sram_trans_align;
        mcu->panel_config.psram_trans_align = display->mcu_config.rgb.esp32s3.psram_trans_align;

        mcu->panel_config.hsync_gpio_num = P(config->rgb.hsync);
        mcu->panel_config.vsync_gpio_num = P(config->rgb.vsync);
        mcu->panel_config.de_gpio_num = P(config->rgb.de);
        mcu->panel_config.pclk_gpio_num = P(config->rgb.pclk);
        mcu->panel_config.disp_gpio_num = P(config->rgb.disp_en);
        mcu->panel_config.flags.disp_active_low = display->mcu_config.rgb.esp32s3.flags.disp_active_low;
        mcu->panel_config.flags.fb_in_psram = display->mcu_config.rgb.esp32s3.flags.fb_in_psram;
        mcu->panel_config.data_gpio_nums[0]  = P(config->rgb.b[0]);
        mcu->panel_config.data_gpio_nums[1]  = P(config->rgb.b[1]);
        mcu->panel_config.data_gpio_nums[2]  = P(config->rgb.b[2]);
        mcu->panel_config.data_gpio_nums[3]  = P(config->rgb.b[3]);
        mcu->panel_config.data_gpio_nums[4]  = P(config->rgb.b[4]);
        mcu->panel_config.data_gpio_nums[5]  = P(config->rgb.g[0]);
        mcu->panel_config.data_gpio_nums[6]  = P(config->rgb.g[1]);
        mcu->panel_config.data_gpio_nums[7]  = P(config->rgb.g[2]);
        mcu->panel_config.data_gpio_nums[8]  = P(config->rgb.g[3]);
        mcu->panel_config.data_gpio_nums[9]  = P(config->rgb.g[4]);
        mcu->panel_config.data_gpio_nums[10] = P(config->rgb.g[5]);
        mcu->panel_config.data_gpio_nums[11] = P(config->rgb.r[0]);
        mcu->panel_config.data_gpio_nums[12] = P(config->rgb.r[1]);
        mcu->panel_config.data_gpio_nums[13] = P(config->rgb.r[2]);
        mcu->panel_config.data_gpio_nums[14] = P(config->rgb.r[3]);
        mcu->panel_config.data_gpio_nums[15] = P(config->rgb.r[4]);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        mcu->panel_config.flags.refresh_on_demand = display->mcu_config.rgb.esp32s3.flags.relax_on_idle;
#else
        mcu->panel_config.on_frame_trans_done = _on_vsync_event;
        mcu->panel_config.user_ctx = mcu;
        mcu->panel_config.flags.relax_on_idle = display->mcu_config.rgb.esp32s3.flags.relax_on_idle;
#endif
        
        DBG_VERBOSE("Create RGB panel\n");
        
        ret = esp_lcd_new_rgb_panel(&mcu->panel_config, &mcu->panel_handle);
        DBG_ASSERT(ret == ESP_OK, goto error, NULL, "esp_lcd_new_rgb_panel failed\n");
        DBG_ASSERT(mcu->panel_handle, goto error, NULL, "Cannot create panel handle\n");

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        mcu->panel_event_callbacks.on_vsync = _on_vsync_event;
        ret = esp_lcd_rgb_panel_register_event_callbacks(mcu->panel_handle, &mcu->panel_event_callbacks, mcu);
        DBG_ASSERT(ret == ESP_OK, goto error, NULL, "esp_lcd_rgb_panel_register_event_callbacks failed\n");      
#endif
        
        DBG_VERBOSE("Create SPI\n");

        spi_bus_config_t buscfg = 
        {
            .miso_io_num = P(config->rgb.miso),
            .mosi_io_num = P(config->rgb.mosi),
            .sclk_io_num = P(config->rgb.clk),
            .quadwp_io_num = -1,
            .quadhd_io_num = -1
        };
        spi_device_interface_config_t devcfg = 
        {
            .address_bits = 1,
            .spics_io_num = P(config->rgb.cs),
            .queue_size = 1, 
            .clock_speed_hz = APB_CLK_FREQ / 8,
            .mode = 0
        };

        //Initialize the SPI bus
        ret=spi_bus_initialize(config->rgb.spi_unit, &buscfg, SPI_DMA_DISABLED);
        DBG_ASSERT(ret == ESP_OK, goto error, NULL, "spi_bus_initialize failed\n");

        //Attach the interface to the SPI bus
        ret=spi_bus_add_device(config->rgb.spi_unit, &devcfg, &mcu->spi);
        DBG_ASSERT(ret == ESP_OK, goto error, NULL, "spi_bus_add_device failed\n");
    }
    else if(config->interface == DISPLAY_INTERFACE_8080)
    {
        // TODO: Implement Intel 8080 display.
    }

    return mcu;

error:
    if(mcu->spi)
        spi_bus_remove_device(mcu->spi);

    if(mcu->panel_handle)
        esp_lcd_panel_del(mcu->panel_handle);

    if(mcu)
        mcu_heap_free(mcu);

    return NULL;
}
#undef P

void display_mcu_rgb_spi_write_command(display_mcu_handle_t mcu, unsigned char command)
{
    spi_transaction_t t = {0};

    t.addr = 0;
	// Number of bits to transmit  
    t.length = 8;		
	// Data to transmit
    t.tx_buffer = &command;

	// Transmit!
    spi_device_polling_transmit(mcu->spi, &t);  
}

void display_mcu_rgb_spi_write_data(display_mcu_handle_t mcu, unsigned char data)
{
    spi_transaction_t t = {0};

    t.addr = 1;
	// Number of bits to transmit  
    t.length = 8;		
	// Data to transmit
    t.tx_buffer = &data;

	// Transmit!
    spi_device_polling_transmit(mcu->spi, &t);  
}

// TODO: Below is only RGB compatible!

FUNCTION_RETURN_T display_mcu_reset(display_mcu_handle_t mcu)
{
    return ESP_OK == esp_lcd_panel_reset(mcu->panel_handle) ? FUNCTION_RETURN_OK : FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_init_device(display_mcu_handle_t mcu)
{
    return ESP_OK == esp_lcd_panel_init(mcu->panel_handle) ? FUNCTION_RETURN_OK : FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_del_device(display_mcu_handle_t mcu)
{
    return ESP_OK == esp_lcd_panel_del(mcu->panel_handle) ? FUNCTION_RETURN_OK : FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_draw_bitmap(display_mcu_handle_t mcu, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{    
    return ESP_OK == esp_lcd_panel_draw_bitmap(mcu->panel_handle, x_start, y_start, x_end + 1, y_end + 1, color_data) ? FUNCTION_RETURN_OK : FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_mirror(display_mcu_handle_t mcu, bool mirror_x, bool mirror_y)
{
    return ESP_OK == esp_lcd_panel_mirror(mcu->panel_handle, mirror_x, mirror_y) ? FUNCTION_RETURN_OK : FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_swap_xy(display_mcu_handle_t mcu, bool swap_axes)
{
    return ESP_OK == esp_lcd_panel_swap_xy(mcu->panel_handle, swap_axes) ? FUNCTION_RETURN_OK : FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_set_gap(display_mcu_handle_t mcu, int x_gap, int y_gap)
{
    return ESP_OK == esp_lcd_panel_set_gap(mcu->panel_handle, x_gap, y_gap) ? FUNCTION_RETURN_OK : FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_invert_color(display_mcu_handle_t mcu, bool invert_color_data)
{
    return ESP_OK == esp_lcd_panel_invert_color(mcu->panel_handle, invert_color_data) ? FUNCTION_RETURN_OK : FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_disp_off(display_mcu_handle_t mcu, bool off)
{
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    return ESP_OK == esp_lcd_panel_disp_on_off(mcu->panel_handle, !off) ? FUNCTION_RETURN_OK : FUNCTION_RETURN_UNSUPPORTED;
#else
    return ESP_OK == esp_lcd_panel_disp_off(mcu->panel_handle, off) ? FUNCTION_RETURN_OK : FUNCTION_RETURN_UNSUPPORTED;
#endif
}

FUNCTION_RETURN_T display_mcu_refresh(display_mcu_handle_t mcu)
{
    return ESP_OK == esp_lcd_rgb_panel_refresh(mcu->panel_handle) ? FUNCTION_RETURN_OK : FUNCTION_RETURN_UNSUPPORTED;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static bool _on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;
    display_mcu_handle_t mcu = user_data;
    display_event_data_t event = {0};
    
    if(mcu->config->on_frame_trans_done)
        mcu->config->on_frame_trans_done(mcu->display, &event, mcu->config->user_ctx);

    return high_task_awoken == pdTRUE;
}

#endif // MODULE_ENABLE_DISPLAY && MCU_TYPE == MCU_ESP32