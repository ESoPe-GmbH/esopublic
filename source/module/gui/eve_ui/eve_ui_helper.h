/**
 * @file eve_ui_helper.h
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * @author Tim Koczwara
 * 
 * @brief   Helper structures for eve ui.
 * 
 * @version 1.00 (26.02.2024)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_GUI_EVE_UI_HELPER_H_
#define __MODULE_GUI_EVE_UI_HELPER_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "module/enum/function_return.h"


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief x/y coordinates to declare a point on the screen.
 */
typedef struct eve_ui_point_s
{
    /// @brief x-coordinate from the top-left of the screen.
    int32_t x;
    /// @brief y-coordinate from the top-left of the screen.
    int32_t y;
}eve_ui_point_t;

/**
 * @brief Width and height of a component. 
 */
typedef struct eve_ui_size_s
{
    /// @brief Width of a component
    uint16_t width;
    /// @brief Height of a component
    uint16_t height;
}eve_ui_size_t;

/**
 * @brief Declares the coordinates and size of the element.
 */
typedef struct eve_ui_location_s
{
    /// @brief Coordinates of the element
    eve_ui_point_t origin;
    /// @brief Size of the element
    eve_ui_size_t size;
}eve_ui_location_t;

/**
 * @brief Offset of a component to all sites.
 */
typedef struct eve_ui_offset_s
{
    /// @brief x offset from the left of the screen.
    int32_t left;
    /// @brief x offset from the right of the screen.
    int32_t right;
    /// @brief y offset from the top of the screen.
    int32_t top;
    /// @brief y offset from the bottom of the screen.
    int32_t bottom;
}eve_ui_offset_t;

#endif // MODULE_ENABLE_GUI

#endif /* __MODULE_GUI_EVE_UI_HELPER_H_ */