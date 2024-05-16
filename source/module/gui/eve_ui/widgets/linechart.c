/**
 * @file linechart.c
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI

#include "linechart.h"
#include "module/convert/string.h"
#include "module/util/assert.h"
#include "module/gui/eve/eve_copro.h"
#include "module/gui/eve_ui/font.h"
#include <math.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Draw the line chart on the display
 *
 * @param c     Pointer to the linechart
 * @param p     Coordinates of the parent component on the screen
 */
void _paint_chart(linechart_t* c, eve_ui_point_t p);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN_T linechart_init(linechart_t* chart, const linechart_options_t* options, eve_ui_location_t location)
{
    ASSERT_RET_NOT_NULL(chart, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(options, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    component_init(&chart->component, COMPONENT_TYPE_LINECHART, (component_paint_cb_t)_paint_chart);

    // Copy options into the chart data.
    memcpy(&chart->options, options, sizeof(linechart_options_t));
    chart->options.line_options = mcu_heap_calloc(options->num_lines, sizeof(line_options_t));
    ASSERT_RET_NOT_NULL(chart->options.line_options, NO_ACTION, FUNCTION_RETURN_INSUFFICIENT_MEMORY);
    memcpy(chart->options.line_options, options->line_options, sizeof(line_options_t) * options->num_lines);

    chart->component.size = location.size;
    chart->component.origin = location.origin;

    if(chart->options.axis_x.show_label && (chart->options.axis_x.str_label == NULL || chart->options.axis_x.str_label[0] == 0))
    {
        chart->options.axis_x.show_label = false;
    }

    if(chart->options.axis_y.show_label && (chart->options.axis_y.str_label == NULL || chart->options.axis_y.str_label[0] == 0))
    {
        chart->options.axis_y.show_label = false;
    }

    return FUNCTION_RETURN_OK;
}

void linechart_free(linechart_t* chart)
{
    if(chart->options.line_options)
    {
        mcu_heap_free(chart->options.line_options);
        chart->options.line_options = NULL;
    }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static int32_t _scale_axis(axis_options_t* axis, int32_t value, int32_t range_pixel)
{
    int32_t range_value = axis->max_value - axis->min_value;
    int32_t ret = (value - axis->min_value) * range_pixel / range_value;
    return ret;
}

static int32_t _scale_y(linechart_t* c, int32_t value, int32_t range_pixel)
{
    return _scale_axis(&c->options.axis_y, value, range_pixel);
}

static int32_t _scale_x(linechart_t* c, int32_t value, int32_t range_pixel)
{
    return _scale_axis(&c->options.axis_x, value, range_pixel);
}

static FUNCTION_RETURN_T _get_value(linechart_t* c, line_options_t* l, int32_t index, int32_t* value)
{
    switch(l->value_type)
    {
        case LINECHART_VALUE_FLOAT:
        {
            float fvalue = 0;
            FUNCTION_RETURN_T ret = ringbuffer_get(l->values, &fvalue, index);
            *value = (int32_t)roundf(fvalue);
            return ret;
        }
        case LINECHART_VALUE_INT32:
        case LINECHART_VALUE_INT16:
        case LINECHART_VALUE_INT8:
        {
            *value = 0;
            return ringbuffer_get(l->values, value, index);
        }
        case LINECHART_VALUE_CUSTOM:
        {
            if(l->f_get_value)
            {
                return l->f_get_value(c, index, value);
            }

            // Invalid value
            return FUNCTION_RETURN_PARAM_ERROR;
        }
        default:
            // Invalid value
            return FUNCTION_RETURN_UNSUPPORTED;
    }
    return FUNCTION_RETURN_OK;
}

void _paint_chart(linechart_t* c, eve_ui_point_t p)
{
    if(!c->component.is_visible)
    {
        return;
    }    
        
	eve_t* eve = component_get_eve(&c->component);

    int32_t xl, xr, yt, yb;

    p.x += c->component.origin.x;
    p.y += c->component.origin.y;

    // Calculate the four outer coorinates of the chart.
    // xl/yt ---- xr/yt
    //   |          |
    // xl/yb ---- xr/yb
    xl = p.x + c->options.padding.left;
    xr = p.x + c->component.size.width - c->options.padding.right - c->options.padding.left;
    yb = p.y + c->component.size.height - c->options.padding.bottom - c->options.padding.top;
    yt = p.y + c->options.padding.top;

    // Calculate the y-Coordinate of the x-Axis, because if negative y-Values exist, we do not draw it at the bottom!
    // If label or tick marks need to be drawn, it also needs to be moved.
    int32_t x_axis_offset = 0;
    int32_t x_label_height = 0;
    int32_t x_font_height = 0;
    if(c->options.axis_x.show_label)
    {
        x_font_height = font_get_height(eve, c->options.axis_x.font_label) + 1;
    }
    if(c->options.axis_x.tick_marks > 0)
    {
        if(c->options.axis_x.label_tick_marks)
        {
            if(x_label_height == 0)
            {
                x_label_height = font_get_height(eve, c->options.axis_x.font_label) + 10;
            }
        }

        if(x_label_height < 5)
        {
            x_label_height = 5;
        }
    }
    yb -= x_label_height;

    // if(c->options.axis_y.min_value < 0)
    {
        x_axis_offset = _scale_y(c, 0, yb - yt);
    }

    // Calculate the x-Coordinate of the y-Axis, because if negative x-Values exist, we do not draw it at the left!
    // If label or tick marks need to be drawn, it also needs to be moved.
    int32_t y_axis_offset = 0;
    int32_t y_label_width = 0;
    if(c->options.axis_y.show_label)
    {
        y_label_width = font_get_width_string(eve, c->options.axis_y.font_label, c->options.axis_y.str_label) + 10;
    }
    if(c->options.axis_y.tick_marks > 0)
    {
        if(c->options.axis_y.label_tick_marks)
        {
            char* line = mcu_heap_calloc(1, 200);
            if(line)
            {
                char* ptr = line;
                int32_t start = 0;

                if(c->options.axis_y.min_value < 0)
                {
                    start = (int32_t)(c->options.axis_y.min_value / c->options.axis_y.tick_marks) * c->options.axis_y.tick_marks;
                }
                
                do
                {
                    ptr += string_nprintf(ptr, 200, "%d\n", start);
                    start += c->options.axis_y.tick_marks;
                }while(start < c->options.axis_y.max_value);

                int32_t w = font_get_width_string(eve, c->options.axis_y.font_label, line) + 10;
                mcu_heap_free(line);

                if(y_label_width < w)
                {
                    y_label_width = w;
                }
            }
        }

        if(y_label_width < 5)
        {
            y_label_width = 5;
        }
    }
    xl += y_label_width;
    // if(c->options.axis_x.min_value < 0)
    {
        y_axis_offset = _scale_x(c, 0, xr - xl);
    }

    // Only try to draw the lines if the option pointer is not NULL.
    if(c->options.line_options)
    {        
	    eve_copro_check_command_buffer(eve, 4);

        // Set Pixel precision: 16
        eve_copro_write_command(eve, EVE_VERTEX_FORMAT(4));

        for(int i = 0; i < c->options.num_lines; i++)
        {
	        eve_copro_check_command_buffer(eve, 12);

            eve_copro_set_color(eve, c->options.line_options[i].color);
            // Set line width
            eve_copro_write_command(eve, EVE_LINE_WIDTH(c->options.line_options[i].line_width));

	        eve_copro_write_command(eve, EVE_BEGIN(EVE_LINE_STRIP));
            
	        eve_copro_check_command_buffer(eve, 4 * c->options.axis_x.max_value);

            // Start drawing from the bottom/left
            for(int j = 0; j < c->options.axis_x.max_value; j++)
            {
                int32_t value;
                FUNCTION_RETURN ret = _get_value(c, &c->options.line_options[i], j, &value);
                if(ret != FUNCTION_RETURN_OK)
                {
                    // Stop reading values for this line
                    break;
                }

                if(value > c->options.axis_y.max_value)
                {
                    value = c->options.axis_y.max_value + 1;
                }
                else if(value < c->options.axis_y.min_value)
                {
                    value = c->options.axis_y.min_value - 1;
                }

                int32_t x0 = (xl * 16) + _scale_x(c, j, (xr - xl) * 16); 
                int32_t y0 = (yb * 16) - _scale_y(c, value, (yb - yt) * 16);

	            eve_copro_write_command(eve, EVE_VERTEX2F(x0, y0));
            }

	        eve_copro_check_command_buffer(eve, 4);
            eve_copro_write_command(eve, EVE_END());
        }
    }

    // Draw the axis above the lines!

    eve_copro_check_command_buffer(eve, 4 * 6);
	// Set color
	eve_copro_set_color(eve, c->options.color);
	eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
	// Set line width
	eve_copro_write_command(eve, EVE_LINE_WIDTH(c->options.line_width_axis));
	// Draw axis
	eve_copro_write_command(eve, EVE_BEGIN(EVE_LINES));
    // Y-Axis
	eve_copro_write_command(eve, EVE_VERTEX2F(xl + y_axis_offset, yt));
	eve_copro_write_command(eve, EVE_VERTEX2F(xl + y_axis_offset, yb));
    if(c->options.axis_y.tick_marks > 0)
    {
        int32_t start = 0;
        if(c->options.axis_y.min_value < 0)
        {
            start = (int32_t)(c->options.axis_y.min_value / c->options.axis_y.tick_marks) * c->options.axis_y.tick_marks;
        }
        
        eve_copro_check_command_buffer(eve, abs((c->options.axis_y.max_value - c->options.axis_y.min_value) / c->options.axis_y.tick_marks) * 4);
        int32_t x0 = xl + _scale_x(c, 0, xr - xl);
        for(int32_t i = start; i <= c->options.axis_y.max_value; i += c->options.axis_y.tick_marks)
        {
            int32_t y0 = yb - _scale_y(c, i, yb - yt);
            eve_copro_write_command(eve, EVE_VERTEX2F(x0 - 5, y0));
            eve_copro_write_command(eve, EVE_VERTEX2F(x0, y0));
        }
    }
    // X-Axis
    eve_copro_check_command_buffer(eve, 4 * 2);

	eve_copro_write_command(eve, EVE_VERTEX2F(xl, yb - x_axis_offset));
	eve_copro_write_command(eve, EVE_VERTEX2F(xr, yb - x_axis_offset));
    if(c->options.axis_x.tick_marks > 0)
    {
        int32_t start = 0;
        if(c->options.axis_x.min_value < 0)
        {
            start = (int32_t)(c->options.axis_x.min_value / c->options.axis_x.tick_marks) * c->options.axis_x.tick_marks;
        }
        
        eve_copro_check_command_buffer(eve, abs(c->options.axis_x.max_value / c->options.axis_x.tick_marks) * 4);
        int32_t y0 = yb - x_axis_offset;
        for(int32_t i = start; i <= c->options.axis_x.max_value; i += c->options.axis_x.tick_marks)
        {
            int32_t x0 = xl + _scale_x(c, i, xr - xl);
            eve_copro_write_command(eve, EVE_VERTEX2F(x0, y0 + 5));
            eve_copro_write_command(eve, EVE_VERTEX2F(x0, y0));
        }
    }
    eve_copro_check_command_buffer(eve, 4);
	eve_copro_write_command(eve, EVE_END());

    // Y-Axis label
    if(c->options.axis_y.show_label)
    {
        eve_copro_text(eve, xl + y_axis_offset + 3, yt, c->options.axis_y.font_label, EVE_OPT_TEXT_DEFAULT, c->options.axis_y.str_label);
    }
    if(c->options.axis_y.tick_marks > 0 && c->options.axis_y.label_tick_marks)
    {
        int32_t start = 0;
        if(c->options.axis_y.min_value < 0)
        {
            start = (int32_t)(c->options.axis_y.min_value / c->options.axis_y.tick_marks) * c->options.axis_y.tick_marks;
        }
        
        eve_copro_check_command_buffer(eve, abs((c->options.axis_y.max_value - c->options.axis_y.min_value) / c->options.axis_y.tick_marks) * 4);
        int32_t x0 = xl + y_axis_offset;
        for(int32_t i = start; i <= c->options.axis_y.max_value; i += c->options.axis_y.tick_marks)
        {
            if(i == 0 && c->options.axis_x.min_value < 0)
            {
                continue;
            }
            int32_t y0 = yb - _scale_y(c, i, yb - yt);
            char str[20];
            string_printf(str, "%d", i);
            eve_copro_text(eve, x0 - 10, y0, c->options.axis_y.font_label, EVE_OPT_TEXT_CENTERY | EVE_OPT_TEXT_RIGHTX, str);
        }
    }

    // X-Axis label
    if(c->options.axis_x.show_label)
    {
        eve_copro_text(eve, xr, yb - x_axis_offset - x_font_height, c->options.axis_x.font_label, EVE_OPT_TEXT_RIGHTX, c->options.axis_x.str_label);
    }
    if(c->options.axis_x.tick_marks > 0 && c->options.axis_x.label_tick_marks)
    {
        int32_t start = 0;
        if(c->options.axis_y.min_value < 0)
        {
            start = (int32_t)(c->options.axis_x.min_value / c->options.axis_x.tick_marks) * c->options.axis_x.tick_marks;
        }
        
        eve_copro_check_command_buffer(eve, abs((c->options.axis_x.max_value - c->options.axis_x.min_value) / c->options.axis_x.tick_marks) * 4);
        int32_t y0 = yb - x_axis_offset + 10;
        for(int32_t i = start; i <= c->options.axis_x.max_value; i += c->options.axis_x.tick_marks)
        {
            if(i == 0 && c->options.axis_y.min_value < 0)
            {
                continue;
            }
            int32_t x0 = xl + _scale_x(c, i, xr - xl);
            char str[20];
            string_printf(str, "%d", i);
            eve_copro_text(eve, x0, y0, c->options.axis_x.font_label, EVE_OPT_TEXT_CENTERX, str);
        }
    }

    // Draw the legend above
    if(c->options.legend.is_visible)
    {
        eve_ui_point_t p = c->options.legend.origin;
        int32_t font_height = font_get_height(eve, c->options.legend.font);
        if(c->options.legend.str_title)
        {
            eve_copro_text(eve, xl + p.x, yt + p.y, c->options.legend.font, EVE_OPT_TEXT_DEFAULT, c->options.legend.str_title);
            p.y += font_height + 2;
        }
        for(int i = 0; i < c->options.num_lines; i++)
        {
            eve_copro_check_command_buffer(eve, 4 * 7);
            // Line color
            eve_copro_set_color(eve, c->options.line_options[i].color);
            // Set line width
            eve_copro_write_command(eve, EVE_LINE_WIDTH(c->options.line_options[i].line_width));
            // Draw line in color
            eve_copro_write_command(eve, EVE_BEGIN(EVE_LINES));
            eve_copro_write_command(eve, EVE_VERTEX2F(xl + p.x, yt + p.y + font_height / 2));
            eve_copro_write_command(eve, EVE_VERTEX2F(xl + p.x + 20, yt + p.y + font_height / 2));
            eve_copro_write_command(eve, EVE_END());
            // Draw text
            eve_copro_set_color(eve, c->options.color);
            eve_copro_text(eve, xl + p.x + 22, yt + p.y, c->options.legend.font, EVE_OPT_TEXT_DEFAULT, c->options.line_options[i].name);
            p.y += font_height + 2;
        }
    }
}
#endif