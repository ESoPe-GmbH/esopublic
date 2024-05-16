/**
 * @file linechart.h
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * @author Tim Koczwara
 * 
 * @brief This module is used to display a linechart on the eve display.
 * 
 * @version 1.00 (23.02.2024)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_LINECHART_H_
#define __MODULE_LINECHART_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "module/enum/function_return.h"
#include "module/gui/eve_ui/component.h"
#include "module/fifo/ringbuffer.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Value types for the linechart indicating how the data will be read, that is displayed.
 * For @c LINECHART_VALUE_CUSTOM a callback function is called to retrieve each value. 
 * On the other types, the linechart expects, that a @c ringbuffer_t object is filled with the chart data and displays it.
 * The data type is needed to read from correct element size and read the values.
 */
typedef enum linechart_value_e
{
    /// @brief A callback function is called to retrieve each value. 
    LINECHART_VALUE_CUSTOM = 0,
    /// @brief Data is stored in a ringbuffer that contains 8-Bit values.
    LINECHART_VALUE_INT8,
    /// @brief Data is stored in a ringbuffer that contains 16-Bit values.
    LINECHART_VALUE_INT16,
    /// @brief Data is stored in a ringbuffer that contains 32-Bit values.
    LINECHART_VALUE_INT32,
    /// @brief Data is stored in a ringbuffer that contains float values.
    LINECHART_VALUE_FLOAT
}LINECHART_VALUE_T;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Type of the chart component. Can be casted to @c component_t
typedef struct linechart_s linechart_t;
/**
 * @brief Callback function to retrieve a single chart value in case @c CHAR_VALUE_CUSTOM was set in the options.
 * 
 * @param char      Pointer to the chart that is drawn.
 * @param index     Index of the value that is needed. Also the position on x Axis, but not in pixel.
 * @param value     Pointer to the value for the y-Axis. Should be between @c min_value_y and @c max_value_y in the options. Otherwise it will be cut to these values.
 * @retval FUNCTION_RETURN_OK   If value was successfully placed in @c value.
 * @retval other                If value could not be read. (e.g. invalid index)
 */
typedef FUNCTION_RETURN_T (*linechart_get_value_t)(linechart_t* chart, uint32_t index, int32_t* value);

/**
 * @brief Options for each line displayed inside the line chart.
 */
typedef struct line_options_s
{
    /// @brief Type of value that is used in the ringbuffer set as datasource or set to @c CHAR_VALUE_CUSTOM to have a callback called for each value.
    LINECHART_VALUE_T value_type;
    // Make those union, depending of @c value_type only one of those is used.
    union
    {
        /// @brief Callback to retrieve a single value while drawing the chart. Only usable when @c value_type is set to @c CHAR_VALUE_CUSTOM.
        linechart_get_value_t f_get_value;
        /// @brief Callback to retrieve a single value while drawing the chart. Only usable when @c value_type is set to @c CHAR_VALUE_INT* or @c CHAR_VALUE_FLOAT.
        ringbuffer_t* values;
    };
    /// @brief Color of the line.
    color_t color;
    /// @brief Line width of the line in 16th Pixel
    uint8_t line_width;
    /// @brief Name that is shown in the legend if it is active.
    const char* name;
}line_options_t;

/**
 * @brief Options for each axis of the line chart.
 */
typedef struct axis_options_s
{
    /// @brief Minimum value on the axis to show. Values of the datasource will be scaled from @c max_value - @c min_value to @c height - @c offset y.
    int32_t min_value;
    /// @brief Maximum value on the axisto show. Values of the datasource will be scaled from @c max_value - @c min_value to @c height - @c offset y
    int32_t max_value;
    /// @brief If set, a label at the end of the axis is shown below. This will automatically increase the offset from the bottom or left.
    bool show_label;
    /// @brief Pointer to a 0-terminated string of the label shown on the axis.
    const char* str_label;
    /// @brief Font of the label.
    uint16_t font_label;
    /// @brief Defines the steps in the values in which tick marks are shown. This is not in pixel, but in value.
    int32_t tick_marks;
    /// @brief Indicates whether labels with the value should be drawn at the position of the tick marks or not. This will automatically increase the offset from the bottom or left.
    bool label_tick_marks;
}axis_options_t;

/**
 * @brief Options for the legend that can be shown on the chart.
 */
typedef struct legend_options_s
{
    /// @brief Set to true to show the legend.
    bool is_visible;
    /// @brief Coordinate of the legend from the top-left on the chart.
    eve_ui_point_t origin;
    /// @brief Font of the legend.
    uint16_t font;
    /// @brief Title of the legend. Can be NULL.
    const char* str_title;
}legend_options_t;

/**
 * @brief Options for displaying the chart.
 */
typedef struct linechart_options_s
{
    /// @brief Padding from the left / top / right / bottom where the chart is drawn.
    eve_ui_offset_t padding;
    /// @brief Options for x-Axis
    axis_options_t axis_x;
    /// @brief Options for y-Axis
    axis_options_t axis_y;
    /// @brief Line width of both axis in 16th Pixel
    uint8_t line_width_axis;
    /// @brief Color of both axis.
    color_t color;
    /// @brief Options for the legend.
    legend_options_t legend;
    /// @brief Number of lines that should be drawn in the chart.
    uint8_t num_lines;
    /// @brief Pointer to an array of line options. One option for each line that should be drawn.
    line_options_t* line_options;
}linechart_options_t;

/**
 * @brief Structure of the linechart.
 */
struct linechart_s
{
    /// Component Object used for painting this object on the screen
	component_t component;			
    /// @brief Options that were set during init.
    linechart_options_t options;	
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Intializes the chart using the provided options.
 * 
 * @param chart     Pointer to the chart to initialize.
 * @param options   Pointer to the options to use. Options can be set in stack, since they are copied into the chart.
 * @param location  Coordinates and size in pixel on the chart's parent.
 * @return          FUNCTION_RETURN_OK on success or other value if init failed.
 */
FUNCTION_RETURN_T linechart_init(linechart_t* chart, const linechart_options_t* options, eve_ui_location_t location);
/**
 * @brief Frees internal buffers of the chart.
 * 
 * @param chart     Pointer to the chart that was initialized.
 */
void linechart_free(linechart_t* chart);

#endif // MODULE_ENABLE_CHART

#endif /* __MODULE_LINECHART_H_ */