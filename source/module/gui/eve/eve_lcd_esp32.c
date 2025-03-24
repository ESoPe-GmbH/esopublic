/**
 * @file eve_lcd_esp32.c
 **/

#include "eve_lcd_esp32.h"

#if MODULE_ENABLE_GUI && MCU_TYPE == MCU_ESP32 && defined(KERNEL_USES_SLINT)

#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#include "module/convert/convert.h"
#include "module/comm/dbg.h"
#include "module/util/assert.h"

#include "eve_spi.h"


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define EVE_START_ADDRESS           0

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static esp_err_t _draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);

static esp_err_t _disp_on_off(esp_lcd_panel_t *panel, bool on_off);

static void _flush_display(eve_t* eve);

static esp_err_t _read_data(esp_lcd_touch_handle_t tp);
    
static bool _get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static esp_lcd_panel_t _panel = 
{
    .reset = NULL,
    .init = NULL,
    .del = NULL,
    .draw_bitmap = _draw_bitmap,
    .mirror = NULL,
    .swap_xy = NULL,
    .set_gap = NULL,
    .invert_color = NULL,
    .disp_on_off = _disp_on_off,
    .disp_sleep = NULL,
    .user_data = NULL
};

static esp_lcd_touch_t _esp_lcd_touch = 
{
    .enter_sleep = NULL,
    .exit_sleep = NULL,
    .read_data = _read_data,
    .get_xy = _get_xy,
#if (CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS > 0)
    .get_button_state = NULL,
#endif
    .set_swap_xy = NULL,
    .get_swap_xy = NULL,
    .set_mirror_x = NULL,
    .get_mirror_x = NULL,
    .set_mirror_y = NULL,
    .get_mirror_y = NULL,
    .del = NULL,
    .config = 
    {
        .x_max = 0, // Set in lcd_touch_esp32_create
        .y_max = 0, // Set in lcd_touch_esp32_create
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = GPIO_NUM_NC,
        .levels = {.reset = 0, .interrupt = 0},
        .flags = {.swap_xy = 0, .mirror_x = 0, .mirror_y = 0},
        .process_coordinates = NULL,
        .interrupt_callback = NULL,
        .user_data = NULL,
        .driver_data = NULL // Set in lcd_touch_esp32_create
    },
    .io = NULL,
    .data = 
    {
        0
    }
};

static bool _panel_changed = false;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

// esp_lcd_panel_handle_t eve_lcd_esp32_create(screen_device_t* device)
FUNCTION_RETURN_T eve_lcd_esp32_create(screen_device_t* device, esp_lcd_panel_handle_t* panel_handle, esp_lcd_touch_handle_t* touch_handle)
{
    ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(panel_handle, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    
    _panel.user_data = device;
    *panel_handle = &_panel;
    
    if(touch_handle)
    {
        _esp_lcd_touch.config.driver_data = device;
        *touch_handle = &_esp_lcd_touch;
    }
    
    return FUNCTION_RETURN_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static esp_err_t _draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    screen_device_t* device = (screen_device_t*)panel->user_data;
    eve_t* eve = &device->eve;    

    _panel_changed = true;
    
    // DBG_INFO("Draw %d/%d -> %d/%d\n", x_start, y_start, x_end, y_end);
    
    if(x_start == 0 && y_start == 0 && x_end == eve->eve_display_width && y_end == eve->eve_display_height)
    {
        DBG_INFO("Flush complete\n");
        eve_spi_write_data(eve, EVE_START_ADDRESS, (uint8_t*)color_data, eve->eve_display_width * eve->eve_display_height * 2, false);
        // Flush directly and remove later flush
        _flush_display(eve);
    }
    else
    {
        while(y_start < y_end)
        {
            uint32_t address = EVE_START_ADDRESS + (eve->eve_display_width * 2 * y_start) + (2 * x_start);
            uint16_t* data = (uint16_t*)mcu_heap_calloc(eve->eve_display_width, sizeof(uint16_t));
            if(data)
            {
                uint16_t* ptr = (uint16_t*)color_data;
                int width = x_end - x_start;
                // for(int i = 0; i < width; i++)
                // {
                //  data[i] = swap16(ptr[i]);
                // }
                eve_spi_write_data(eve, address, (uint8_t*)data, width * 2, false);
                mcu_heap_free(data);
            }
            y_start++;
        }

        // Flush at the end
        if(y_end == eve->eve_display_height)
        {
            _flush_display(eve);
        }

        // Flush delayed
        // _timestamp_draw = system_get_tick_count();
        // system_task_add(&_task_flush);
    }
    return ESP_OK;
}

static esp_err_t _disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    screen_device_t* device = (screen_device_t*)panel->user_data;
    DBG_INFO("_disp_on_off(%B)\n", on_off);
    if(on_off)
    {
        screen_device_on(device);
    }
    else
    {
        screen_device_off(device);
    }
    return ESP_OK;
}

static void _flush_display(eve_t* eve)
{
    // eve_copro_dlstart(eve, color_get(COLOR_WHITE));
    uint32_t _commands[] = 
    {
        // DL Start
        0xffffff00,
        // EVE_CLEAR_COLOR_RGB(0, 0, 0),
        EVE_CLEAR(1, 1, 1),

        // // Set color
        // EVE_COLOR_A(0xFF),
        // EVE_COLOR_RGB(0xFF, 0xFF, 0xFF),

        // Show Bitmap
        EVE_VERTEX_FORMAT(0), // Pixel precision: 1
        EVE_BITMAP_HANDLE(0),
        EVE_BITMAP_SOURCE(EVE_START_ADDRESS),
        EVE_BITMAP_SIZE_H(0, 0),
        EVE_BITMAP_SIZE(EVE_FILTER_NEAREST, EVE_WRAP_BORDER, EVE_WRAP_BORDER, eve->eve_display_width, eve->eve_display_height),
        EVE_BITMAP_LAYOUT_H(0, 0),
        // 7 = RGB565, Linestride = Width * 2, Number of lines
        EVE_BITMAP_LAYOUT(7, eve->eve_display_width * 2, eve->eve_display_height),
        // Set image on display
        EVE_BEGIN(EVE_BITMAPS),		// Draw bitmap
        EVE_VERTEX2F(0, 0),
        EVE_END(),

        // Swap
        0xffffff01,
        // Display
        0x00000000
    };
    eve_copro_write_commands(eve, _commands, sizeof(_commands) / sizeof(uint32_t));
    // eve_copro_swap(eve);
    // DBG_INFO("Flush display\n");
    // for(int i = 0; i < sizeof(_commands) / sizeof(uint32_t); i++)
    // {
    //     DBG_INFO("%08x\n", _commands[i]);
    // }

    // DBG_INFO("Flush %d/%d -> %d/%d\n", _x_start_min, _y_start_min, _x_end_max, _y_end_max);
    _panel_changed = false;
}

static esp_err_t _read_data(esp_lcd_touch_handle_t tp)
{
    screen_device_t* device = (screen_device_t*)tp->config.driver_data;
    eve_t* eve = &device->eve;
    
    if(_panel_changed)
    {
        _flush_display(eve);
    }
    
    // TODO: Multi-Touch?

    // Just read the data, ignoring the interrupt. Trying to read the interrupt first lead to lost touches.
    // Interrupt did not always trigger when the finger was removed. It worked by directly reading EVE_REG_TOUCH_SCREEN_XY.

    uint32_t tmp32 = 0xFFFFFFFF;
        
    tmp32 = eve_spi_read_32(eve, EVE_REG_TOUCH_SCREEN_XY);
    uint16_t x, y;
    x = (tmp32 >> 16) & 0xFFFF;
    y = tmp32 & 0xFFFF;

    // if(tp->data.coords[0].x != x || tp->data.coords[0].y != y)
    // {
    //     DBG_INFO("x=%d y=%d\n", x, y);
    // }

    tp->data.points = (x == 0x8000 && y == 0x8000) ? 0 : 1;
    tp->data.coords[0].x = x;
    tp->data.coords[0].y = y;


    return ESP_OK;
}

static bool _get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    *point_num = MATH_MIN(max_point_num, tp->data.points);
    for(int i = 0; i < *point_num; i++)
    {
        x[i] = tp->data.coords[i].x;
        y[i] = tp->data.coords[i].y;
        if(strength)
        {
            strength[i] = tp->data.coords[i].strength;
        }
    }
    
    return tp->data.points > 0;
}

#endif
