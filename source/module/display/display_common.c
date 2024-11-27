/**
 * 	@file display_common.c
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten
 *  @author 	Tim Koczwara
 *
 *  @brief	Defines an enumeration that can be used as generic return values for functions.
 *			
 *  @version	1.00 (19.08.2022)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#include "module_public.h"

#if MODULE_ENABLE_DISPLAY

#include "display_common.h"
#include "internal/display_internal.h"
#include "module/comm/dbg.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

inline FUNCTION_RETURN_T display_device_reset(display_handle_t display)
{
    return display_mcu_reset(display->mcu);
}

inline FUNCTION_RETURN_T display_device_init(display_handle_t display)
{
    return display_mcu_init_device(display->mcu);
}

inline FUNCTION_RETURN_T display_device_del(display_handle_t display)
{
    return display_mcu_del_device(display->mcu);
}

inline void display_set_event_callback(display_handle_t display, display_event_cb_t f, void* ctx)
{
    return display_mcu_set_event_callback(display->mcu, f, ctx);
}

int32_t display_device_get_width(display_handle_t display)
{
    ASSERT_RET_NOT_NULL(display, NO_ACTION, -1);
    if(display->swap_xy)
    {
        return display->device_config.rgb.v_res; // Swapped! Return vertical resolution
    }
    else
    {
        return display->device_config.rgb.h_res; // Return horizontal resolution
    }
}

int32_t display_device_get_height(display_handle_t display)
{
    ASSERT_RET_NOT_NULL(display, NO_ACTION, -1);
    if(display->swap_xy)
    {
        return display->device_config.rgb.h_res; // Swapped! Return horizontal resolution
    }
    else
    {
        return display->device_config.rgb.v_res; // Return vertical resolution
    }
}

inline FUNCTION_RETURN_T display_device_draw_bitmap(display_handle_t display, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    return display_mcu_draw_bitmap(display->mcu, x_start, y_start, x_end, y_end, color_data);
}

inline FUNCTION_RETURN_T display_device_mirror(display_handle_t display, bool mirror_x, bool mirror_y)
{
    FUNCTION_RETURN_T ret = display_mcu_mirror(display->mcu, mirror_x, mirror_y);   
    if(ret == FUNCTION_RETURN_OK)
    {
        display->mirror_x = mirror_x;
        display->mirror_y = mirror_y;
    }
    return ret;
}

inline FUNCTION_RETURN_T display_device_get_mirror(display_handle_t display, bool* mirror_x, bool* mirror_y)
{
    ASSERT_RET_NOT_NULL(display, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(mirror_x, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(mirror_y, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    *mirror_x = display->mirror_x;
    *mirror_y = display->mirror_y;
    return FUNCTION_RETURN_OK;
}

inline FUNCTION_RETURN_T display_device_swap_xy(display_handle_t display, bool swap_axes)
{
    FUNCTION_RETURN_T ret = display_mcu_swap_xy(display->mcu, swap_axes);
    if(ret == FUNCTION_RETURN_OK)
    {
        display->swap_xy = swap_axes;
    }
    return ret;
}

inline FUNCTION_RETURN_T display_device_get_swap_xy(display_handle_t display, bool* swap_axes)
{
    ASSERT_RET_NOT_NULL(display, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(swap_axes, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    *swap_axes = display->swap_xy;
    return FUNCTION_RETURN_OK;
}

inline FUNCTION_RETURN_T display_device_set_gap(display_handle_t display, int x_gap, int y_gap)
{
    return display_mcu_set_gap(display->mcu, x_gap, y_gap);
}

inline FUNCTION_RETURN_T display_device_invert_color(display_handle_t display, bool invert_color_data)
{
    return display_mcu_invert_color(display->mcu, invert_color_data);
}

inline FUNCTION_RETURN_T display_device_disp_off(display_handle_t display, bool off)
{
    return display_mcu_disp_off(display->mcu, off);
}

inline FUNCTION_RETURN_T display_device_refresh(display_handle_t display)
{
    return display_mcu_refresh(display->mcu);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // MODULE_ENABLE_DISPLAY
