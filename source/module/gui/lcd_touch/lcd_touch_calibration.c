/**
 * @file lcd_touch_calibration.c
 **/

#include "lcd_touch_calibration.h"

#if MODULE_ENABLE_LCD_TOUCH

#include "module/convert/convert.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static bool _common_process(uint16_t *x, uint16_t *y, uint8_t *point_num, uint8_t max_point_num, int x_max, int y_max, float a, float b, float c, float d, float e, float f);

static int _get_x(int x, int y, float a, float b, float c, int x_max);

static int _get_y(int x, int y, float d, float e, float f, int y_max);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

bool lcd_touch_calibration_process_etml035023udra(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    // TODO: Get values -> Below values are from 4.3"
    float a = -0.0292;
    float b = 0.0002;
    float c = 478.2111;
    float d = -0.0005;
    float e = -0.0276;
    float f = 273.0244;

    return _common_process(x, y, point_num, max_point_num, 320, 240, a, b, c, d, e, f);
}

bool lcd_touch_calibration_process_etml043023udra(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    float a = -0.0292;
    float b = 0.0002;
    float c = 478.2111;
    float d = -0.0005;
    float e = -0.0276;
    float f = 273.0244;

    return _common_process(x, y, point_num, max_point_num, 480, 272, a, b, c, d, e, f);
}

bool lcd_touch_calibration_process_etml050023udra(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    // TODO: Get values -> Below values are from 4.3"
    float a = -0.0292;
    float b = 0.0002;
    float c = 478.2111;
    float d = -0.0005;
    float e = -0.0276;
    float f = 273.0244;

    return _common_process(x, y, point_num, max_point_num, 800, 480, a, b, c, d, e, f);
}

bool lcd_touch_calibration_process_etml070023udra(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    // TODO: Get values -> Below values are from 4.3"

    float a = -0.0292;
    float b = 0.0002;
    float c = 478.2111;
    float d = -0.0005;
    float e = -0.0276;
    float f = 273.0244;

    return _common_process(x, y, point_num, max_point_num, 800, 480, a, b, c, d, e, f);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static bool _common_process(uint16_t *x, uint16_t *y, uint8_t *point_num, uint8_t max_point_num, int x_max, int y_max, float a, float b, float c, float d, float e, float f)
{
    int i_max = MATH_MIN(max_point_num, *point_num);

    for(int i = 0; i < i_max; i++)
    {
        x[i] = _get_x(x[i], y[i], a, b, c, 480);
        y[i] = _get_y(x[i], y[i], d, e, f, 272);
    }

    return i_max > 0;
}

static int _get_x(int x, int y, float a, float b, float c, int x_max)
{
    float x1_d = (float)x * a + (float)y * b + c;

    if(x1_d < 0.0)
    {
        return 0;
    }
    else if(x1_d > x_max)
    {
        return x_max;
    }
    else
    {
        return (int)roundf(x1_d);
    }
}

static int _get_y(int x, int y, float d, float e, float f, int y_max)
{
    float y1_d = (float)x * d + (float)y * e + f;

    if(y1_d < 0.0)
    {
        return 0;
    }
    else if(y1_d > y_max)
    {
        return y_max;
    }
    else
    {
        return (int)roundf(y1_d);
    }
}

#endif