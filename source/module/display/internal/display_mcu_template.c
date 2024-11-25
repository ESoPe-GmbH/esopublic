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
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct display_mcu_data_s
{
    uint8_t dummy;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

display_mcu_handle_t display_mcu_init(const display_common_hardware_t* config, const display_mcu_config_t* mcu_config, const display_device_config_t* device_config)
{

    return NULL;
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

#endif // MODULE_ENABLE_DISPLAY
