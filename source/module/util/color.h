/**
 * 	@file 	color.h
 * 	@copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Felix Herrmann
 *
 *  @brief
 *			Contains structure to define a color and functions to convert between different color representations.
 *
 *  @version	1.00 (19.01.2023)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/
#ifndef __COLOR__FIRST_INCL
#define __COLOR__FIRST_INCL

#include "module_public.h"
#include "module/enum/function_return.h"

//------------------------------------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------
// Type Definition
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// Config
//------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure and Enum
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Type to represent a color in rgb format.
 * 
 * You can either access the bytes for r, g and b separately or as a single 32-Bit value having b as the least significant byte up to r.
 * This will allow you to use "%06x" inside a printf to print them as color hex codes that are used in the web.
 */
typedef union
{
    struct
    {
        /// Value for blue
        uint8_t b;
        /// Value for green
        uint8_t g;
        /// Value for red
        uint8_t r;
        /// Unused byte inside the uint32 representation
        uint8_t reserved;
    };
    /// 32-Bit representation of RGB code with B as lowest byte.
    uint32_t value;
}color_rgb_t;

/**
 * @brief HSV representation of a color (hue, saturation, value).
 * 
 */
typedef struct
{
    /// Value for hue
    float h;
    /// Value for saturation
    float s;
    /// Value that indicates the brightness of the color.
    float v;
}color_hsv_t;

//------------------------------------------------------------------------------------------------------------
// External functions
//------------------------------------------------------------------------------------------------------------

#if MODULE_ENABLE_UTIL_COLOR

FUNCTION_RETURN color_rgb_to_hsv(const color_rgb_t* in, color_hsv_t* out);
FUNCTION_RETURN color_hsv_to_rgb(const color_hsv_t* in, color_rgb_t* out);
FUNCTION_RETURN color_rgb_change_brightness(color_rgb_t* color, uint8_t brightness_percent);

#endif

#endif // __COLOR__FIRST_INCL