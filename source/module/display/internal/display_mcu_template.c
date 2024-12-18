/**
 * 	@file display_TEMPLATE.c 
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten
 *  @author 	Tim Koczwara
 *
 *  @brief	MCU specific implementation of the lcd driver for TEMPLATE.
 *			
 *  @version	1.00 (19.08.2022)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#include "module_public.h"

#if MODULE_ENABLE_DISPLAY && 0 // Replace this by MCU of your mcu.

#include "../internal/display_internal.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defintion
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#ifndef DISPLAY_NUM_FB
/// Number of frame buffers the display uses
#define DISPLAY_NUM_FB          0
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct display_mcu_data_s
{
    /// @brief Pointer to frame buffers
    void* fb[DISPLAY_NUM_FB];
    /// User data which would be passed to on_event's user_ctx. Leave NULL if you do not need it.
    void *on_event_ctx;
    /// Callback invoked when one frame buffer has transferred done.
    display_event_cb_t f_on_event;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

display_mcu_handle_t display_mcu_init(const display_common_hardware_t* config, const display_mcu_config_t* mcu_config, const display_device_config_t* device_config)
{

    return NULL;
}

void display_mcu_set_event_callback(display_mcu_handle_t mcu, display_event_cb_t f, void* ctx)
{

}

void display_mcu_rgb_spi_write_command(display_mcu_handle_t mcu, unsigned char command)
{

}

void display_mcu_rgb_spi_write_data(display_mcu_handle_t mcu, unsigned char data)
{
    
}

FUNCTION_RETURN_T display_mcu_reset(display_mcu_handle_t mcu)
{

    return FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_init(display_mcu_handle_t mcu)
{

    return FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_del(display_mcu_handle_t mcu)
{

    return FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_draw_bitmap(display_mcu_handle_t mcu, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    return FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_draw_bitmap(display_mcu_handle_t mcu, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    return FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_mirror(display_mcu_handle_t mcu, bool mirror_x, bool mirror_y)
{

    return FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_swap_xy(display_mcu_handle_t mcu, bool swap_axes)
{

    return FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_set_gap(display_mcu_handle_t mcu, int x_gap, int y_gap)
{

    return FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_invert_color(display_mcu_handle_t mcu, bool invert_color_data)
{

    return FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_disp_off(display_mcu_handle_t mcu, bool off)
{
    
    return FUNCTION_RETURN_UNSUPPORTED;
}

FUNCTION_RETURN_T display_mcu_get_framebuffer(display_mcu_handle_t mcu, uint8_t index_fb, void** fb)
{
    ASSERT_RET_NOT_NULL(mcu, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    // ASSERT_RET(index_fb < DISPLAY_NUM_FB, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "index_fb = %d does not exist\n", index_fb);
    ASSERT_RET_NOT_NULL(fb, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    return FUNCTION_RETURN_UNSUPPORTED;
}

#endif // MODULE_ENABLE_DISPLAY
