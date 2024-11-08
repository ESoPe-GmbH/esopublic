/**
 * @file lcd_touch_esp32.c
 **/

#include "lcd_touch_esp32.h"
#if MODULE_ENABLE_LCD_TOUCH && MCU_TYPE == MCU_ESP32 && defined(KERNEL_USES_SLINT)

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

static esp_err_t _read_data(esp_lcd_touch_handle_t tp);
    
static bool _get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN_T lcd_touch_esp32_create(lcd_touch_handle_t touch_handle, esp_lcd_touch_handle_t* esp_touch_handle)
{
    ASSERT_RET_NOT_NULL(touch_handle, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(esp_touch_handle, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    _esp_lcd_touch.config.driver_data = touch_handle;
    lcd_touch_get_dimensions(touch_handle, &_esp_lcd_touch.config.x_max, &_esp_lcd_touch.config.y_max);

    *esp_touch_handle = &_esp_lcd_touch;
    
    return FUNCTION_RETURN_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static esp_err_t _read_data(esp_lcd_touch_handle_t tp)
{
    lcd_touch_handle_t th = (lcd_touch_handle_t)tp->config.driver_data;
    return lcd_touch_read_data(th) == FUNCTION_RETURN_OK ? ESP_OK : ESP_FAIL;
}

static bool _get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    lcd_touch_handle_t th = (lcd_touch_handle_t)tp->config.driver_data;
    return lcd_touch_get_xy(th, x, y, strength, point_num, max_point_num);
}

#endif