/***
 * @file color.c
 * @copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/
#include "color.h"
#if MODULE_ENABLE_UTIL_COLOR
#include "module/comm/dbg.h"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static float _get_hue(const color_rgb_t* color);

static void _min_max_rgb(uint8_t* min, uint8_t* max, uint8_t r, uint8_t g, uint8_t b);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN color_rgb_to_hsv(const color_rgb_t* in, color_hsv_t* out)
{
    if(in == NULL || out == NULL)
        return FUNCTION_RETURN_PARAM_ERROR;

    uint8_t min = 0, max = 0;
    _min_max_rgb(&min, &max, in->r, in->g, in->b);

    out->h = _get_hue(in);
    out->s = (float)((max == 0) ? 0 : 1.0f - ((float)min / (float)max));
    out->v = (float)max/255.0f;

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN color_hsv_to_rgb(const color_hsv_t* in, color_rgb_t* out)
{
    if(in == NULL || out == NULL)
        return FUNCTION_RETURN_PARAM_ERROR;
    
    float r, g, b;
    if(in->s == 0)
    {
        r = in->v;
        g = in->v;
        b = in->v;
    }
    else
    {
        uint32_t i;
        float f, p, q, t;

        float hue;
        if (in->h == 360)
            hue = 0;
        else
            hue = in->h / 60;

        i = (uint32_t)hue;
        f = hue - i;

        p = in->v * (1.0 - in->s);
        q = in->v * (1.0 - (in->s * f));
        t = in->v * (1.0 - (in->s * (1.0 - f)));

        switch (i)
        {
        case 0:
            r = in->v;
            g = t;
            b = p;
            break;

        case 1:
            r = q;
            g = in->v;
            b = p;
            break;

        case 2:
            r = p;
            g = in->v;
            b = t;
            break;

        case 3:
            r = p;
            g = q;
            b = in->v;
            break;

        case 4:
            r = t;
            g = p;
            b = in->v;
            break;

        default:
            r = in->v;
            g = p;
            b = q;
            break;
        }
    }

    out->r = (uint8_t) (r * 255);
    out->g = (uint8_t) (g * 255);
    out->b = (uint8_t) (b * 255);
    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN color_rgb_change_brightness(color_rgb_t* color, uint8_t brightness_percent)
{
    if(color == NULL)
        return FUNCTION_RETURN_PARAM_ERROR;

    color_hsv_t hsv = {};
    color_rgb_to_hsv(color, &hsv);
    hsv.v = brightness_percent / 100.0f;
    color_hsv_to_rgb(&hsv, color);

    return FUNCTION_RETURN_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static float _get_hue(const color_rgb_t* color)
{
    if(color == NULL)
        return 0.0f;

    uint8_t r = color->r,
            g = color->g, 
            b = color->b;

    uint8_t min = 0, max = 0;

    if (r == g && g == b)
        return 0.0f;

    _min_max_rgb(&min, &max, r, g, b);

    float delta = max - min;
    float hue;

    if (r == max)
        hue = (g - b) / delta;
    else if (g == max)
        hue = (b - r) / delta + 2.0f;
    else
        hue = (r - g) / delta + 4.0f;

    hue *= 60.0f;
    if (hue < 0.0f)
        hue += 360.0f;

    return hue;
}

static void _min_max_rgb(uint8_t* min, uint8_t* max, uint8_t r, uint8_t g, uint8_t b)
{
    if (r > g)
    {
        *max = r;
        *min = g;
    }
    else
    {
        *max = g;
        *min = r;
    }
    if (b > *max)
    {
        *max = b;
    }
    else if (b < *min)
    {
        *min = b;
    }
}

#endif
