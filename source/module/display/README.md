# Display

This module contains display abstraction to control lcd panels in context with U/I libraries like [Slint](https://slint.dev/) or [LVGL](https://github.com/lvgl/lvgl).

NOTE: This module does not control the backlight, you have to do this yourself.

In board.c/.h you create a `display_common_hardware_t` object that contains the necessary hardware instructions for the display.
Call the corresponding `display_common_init` function. This will internally initialize the hardware, call the display drivers specific init and return the display handle.

This module only encapsulated the hardware driver based on mcu, since mcu have very hardware close implementations for lcd driver.
Under `internal` you can find the HAL for the display interactions of these devices.
These source files are guarded using the `MCU_TYPE` to ensure they are only compiled when the corresponding mcu is used.

Since we might have lots of different displays on different MCUs in the feature, this module is structured by directories for each displays and their header/sources inside it.
In module_config.h you can enable/disable each display driver, so compilation will not be blown by lots of unneded displays.

Current implementation status:

Display | Tested MCUs
--- | ---


## Initialization

Sample board with RGB settings:

```c

// External variable that can then be used.
display_handle_t board_display_handle;

// Internal variable for configuring the hardware
static const display_common_hardware_t _display_config = 
{
 .display = DISPLAY_DEVICE_KD021WVFPD003,
 .interface = DISPLAY_INTERFACE_RGB,
 .on_frame_trans_done = NULL,
 .rgb = {
  // Num RGB pins
  .data_width = 16,
  // RGB pins
        .disp_en = PIN_NONE,
        .pclk = GPIO21,
        .vsync = GPIO5,
        .hsync = GPIO4,
        .de = GPIO42,
  .b[0] = GPIO1,
  .b[1] = GPIO2,
  .b[2] = GPIO3,
  .b[3] = GPIO7,
  .b[4] = GPIO15,
  .g[0] = GPIO16,
  .g[1] = GPIO8,
  .g[2] = GPIO6,
  .g[3] = GPIO19,
  .g[4] = GPIO20,
  .g[5] = GPIO9,
  .r[0] = GPIO10,
  .r[1] = GPIO11,
  .r[2] = GPIO12,
  .r[3] = GPIO13,
  .r[4] = GPIO14,
  // SPI pins
  .spi_unit = 1,
  .miso = GPIO40,
  .mosi = GPIO39,
  .clk = GPIO45,
  .cs = GPIO41,
 }
};

// Call in board_init
board_display_handle = display_common_init(&_display_config);

// In your app_lcd or whatever it will be called:
// Initialize display and set the rotation, gaps and stuff. See display_device_* functions to see what is possible.

display_device_reset(board_display_handle);
display_device_init(board_display_handle);
display_device_invert_color(board_display_handle, false);
display_device_set_gap(board_display_handle, 0, 0);
display_device_swap_xy(board_display_handle, true);
display_device_mirror(board_display_handle, false, true);

// Now you can initialize LVGL and call our draw function in its flush function.

/// Display handle that is needed to get current screen.
static lv_disp_t* _display = NULL;
/// contains internal graphic buffer(s) called draw buffer(s)
static lv_disp_draw_buf_t _disp_buf; 
/// contains callback functions
static lv_disp_drv_t _disp_drv;      

lv_init();
// alloc draw buffers used by LVGL
// it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
lv_color_t *buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * (EXAMPLE_LCD_H_RES / 10) * sizeof(lv_color_t), MALLOC_CAP_DMA);
assert(buf1);
lv_color_t *buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * (EXAMPLE_LCD_H_RES / 10) * sizeof(lv_color_t), MALLOC_CAP_DMA);
assert(buf2);
// initialize LVGL draw buffers
lv_disp_draw_buf_init(&_disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * (EXAMPLE_LCD_H_RES / 10));

lv_disp_drv_init(&_disp_drv);
_disp_drv.hor_res = EXAMPLE_LCD_H_RES;       // Your display width
_disp_drv.ver_res = EXAMPLE_LCD_V_RES;       // Your display height
_disp_drv.flush_cb = _lvgl_flush_cb;         // This will register below function, so we can call our draw!
_disp_drv.draw_buf = &_disp_buf;              
_disp_drv.user_data = board_display_handle;
_display = lv_disp_drv_register(&_disp_drv);

static void _lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    // esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    display_handle_t display_handle = (display_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // pass the draw buffer to the driver
    display_device_draw_bitmap(display_handle, offsetx1, offsety1, offsetx2, offsety2, color_map);
    lv_disp_flush_ready(drv);
}

// For LVGL to refresh you must also use a timer and a task

// Sample with ESP32 timer, but mcu_timer would work the same, once it is implemented for ESP32.
const esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &_increase_lvgl_tick, // Function below
    .name = "lvgl_tick"
};
esp_timer_handle_t lvgl_tick_timer = NULL;
ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000)); // EXAMPLE_LVGL_TICK_PERIOD_MS is 2 in this example
system_init_protothread_task(&_task, true, _refresh_ui_timer); // Function below

static int _refresh_ui_timer(struct pt* pt)
{
    PT_BEGIN(pt);

    while(true)
    {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        PT_YIELD_MS(pt, 10); // Timing is here 100Hz. Can be lower or higher
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }

    PT_END(pt);
}

static void _increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS); // EXAMPLE_LVGL_TICK_PERIOD_MS is 2 in this example
}

```

## LVGL

TBD
