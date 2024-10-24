/**
 * @file lvgl_helper.c
 **/

#include "lvgl_helper.h"
#if defined(KERNEL_USES_LVGL)

#include "module/comm/dbg.h"
#include "module/util/assert.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

    
static void _task_window(void* param);

static void _lv_tick_interrupt(void *arg) ;
/**
 * @brief Callback function to transfer a buffer onto the diusplay
 * 
 * @param drv           Pointer to display driver
 * @param area          Pointer to area that needs to be drawn
 * @param color_map     Bitmap for the area.
 */
static void _lv_display_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);

static void _read_cb(lv_indev_t* indev, lv_indev_data_t* data);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Config that is used for the lvgl.
static lvgl_helper_config_t _config = {0};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN lvgl_helper_init(const lvgl_helper_config_t* config)
{
    memcpy(&_config, config, sizeof(lvgl_helper_config_t));

    if(config->display)
    {
        xTaskCreatePinnedToCore(_task_window, "DISP", 8192, &_config, portPRIVILEGE_BIT | (configMAX_PRIORITIES - 1), NULL, 1);
    }
    
    
    return FUNCTION_RETURN_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


static void _task_window(void* param)
{
    lvgl_helper_config_t* config = (lvgl_helper_config_t*)param;
    lv_display_t* display;
    lv_indev_t* indev_drv = NULL;

    DBG_INFO("Initialize LVGL library\n");
    lv_init();

    DBG_INFO("Create %dx%d\n", display_device_get_width(config->display), display_device_get_height(config->display));
    display = lv_display_create(display_device_get_width(config->display), display_device_get_height(config->display));
    lv_display_set_rotation(display, config->rotation);
    lv_display_set_flush_cb(display, _lv_display_flush_cb);
    lv_display_set_user_data(display, config->display);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    
    /* Allocate draw buffers on the heap. In this example we use two partial buffers of 1/10th size of the screen */
    lv_color_t * buf1 = NULL;
    lv_color_t * buf2 = NULL;

    uint32_t buf_size = display_device_get_width(config->display) * display_device_get_height(config->display) * lv_color_format_get_size(lv_display_get_color_format(display));

    buf1 = lv_malloc(buf_size);
    if(buf1 == NULL) 
    {
        DBG_ERROR("display draw buffer malloc failed\n");
        lv_display_delete(display);
        vTaskDelete(NULL);
        return;
    }

    buf2 = lv_malloc(buf_size);
    if(buf2 == NULL) 
    {
        DBG_ERROR("display buffer malloc failed\n");
        lv_display_delete(display);
        lv_free(buf1);
        vTaskDelete(NULL);
        return;
    }

    const mcu_timer_config_t timer_config = 
    {
        .auto_start = true,
        .f = _lv_tick_interrupt,
        .frq_hz = 1000,
        .lvl = MCU_INT_LVL_HI,
        .obj = NULL,
        .unit = config->timer_unit
    };

    mcu_timer_create(&timer_config);

    lv_display_set_buffers(display, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_FULL);
    
    if(config->touch)
    {
        indev_drv = lv_indev_create();
        lv_indev_set_type(indev_drv, LV_INDEV_TYPE_POINTER);
        lv_indev_set_driver_data(indev_drv, config->touch);
        lv_indev_set_read_cb(indev_drv, _read_cb);
    }

    if(config->f_ui_init)
    {
        config->f_ui_init(display);
    }
    
    DBG_INFO("Start Display loop\n");

    // uint32_t timestamp_elapsed = 0;
    uint32_t timestamp_touch = 0;
    uint32_t timestamp_timer = 0;

    while(1)
    {                
        if(system_get_tick_count() - timestamp_timer >= 2)
        {
            timestamp_timer = system_get_tick_count();
            
            /* The task running lv_task_handler should have lower priority than that running `lv_tick_inc` */
            lv_task_handler();
        }
        
        if(indev_drv && system_get_tick_count() - timestamp_touch >= 100)
        {
            timestamp_touch = system_get_tick_count();

            // Read the touch
            lv_indev_read(indev_drv);
        }

        vTaskDelay(1);
    }
}

static void _lv_tick_interrupt(void *arg) 
{
    (void) arg;

    lv_tick_inc(1);
}

static void _lv_display_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    display_handle_t display_handle = (display_handle_t) lv_display_get_user_data(disp);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // pass the draw buffer to the driver
    display_device_draw_bitmap(display_handle, offsetx1, offsety1, offsetx2, offsety2, px_map);
    lv_display_flush_ready(disp);
}

static void _read_cb(lv_indev_t* indev, lv_indev_data_t* data)
{
    lcd_touch_handle_t touch = lv_indev_get_driver_data(indev);
    lcd_touch_read_data(touch);
    uint16_t x, y, strength;
    uint8_t point_num;
    if(lcd_touch_get_xy(touch, &x, &y, &strength, &point_num, 1))
    {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

#endif // MODULE_ENABLE_LVGL_HELPER
