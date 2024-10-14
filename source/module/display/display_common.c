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

#include "module.h"

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

inline FUNCTION_RETURN_T display_device_draw_bitmap(display_handle_t display, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    return display_mcu_draw_bitmap(display->mcu, x_start, y_start, x_end, y_end, color_data);
}

inline FUNCTION_RETURN_T display_device_mirror(display_handle_t display, bool mirror_x, bool mirror_y)
{
    return display_mcu_mirror(display->mcu, mirror_x, mirror_y);   
}

inline FUNCTION_RETURN_T display_device_swap_xy(display_handle_t display, bool swap_axes)
{
    return display_mcu_swap_xy(display->mcu, swap_axes);
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
