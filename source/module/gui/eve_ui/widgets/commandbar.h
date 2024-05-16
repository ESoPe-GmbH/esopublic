/**
 * @file commandbar.h
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * @author Tim Koczwara
 * 
 * @brief 
 * 
 * @version 1.00 (01.03.2024)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_COMMANDBAR_H_
#define __MODULE_COMMANDBAR_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "module/enum/function_return.h"
#include "module/gui/eve_ui/component.h"
#include "module/gui/eve_ui/button.h"
#include "module/gui/eve_ui/rect.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Orientation of the command bar.
typedef enum
{
    /// @brief Command bar items are placed horizontally.
    COMMANDBAR_ORIENTATION_HORIZONTAL,
    /// @brief Command bar items are placed vertically.
    COMMANDBAR_ORIENTATION_VERTICAL   
}COMMANDBAR_ORIENTATION_T;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Type for the command bar.
typedef struct commandbar_s commandbar_t;

/// @brief Options for a single item on the command bar.
typedef struct commandbar_item_s commandbar_item_t;

/// @brief Callback for the click on an item.
typedef void(*commandbar_item_click_cb_t)(commandbar_item_t* item);

/// @brief Options for a single item on the command bar.
struct commandbar_item_s
{
    // Public:

    /// @brief User specific pointer
    void* user;
    /// @brief Function callback for the click on an item.
    commandbar_item_click_cb_t cb_click;
    /// @brief Pointer to the string that is displayed on the button.
    const char* caption;
    /// @brief Font of the caption
    uint8_t caption_font;
    /// @brief Icon that is displayed on the button.
    component_t* icon;
    /// @brief Position of the icon on the item.
    BUTTON_FIGURE_POS icon_pos;
    /// @brief Margin of the item inside the command bar.
    eve_ui_offset_t margin;
    
    // Private:

    /// @brief Reference to the commandbar it is added to.
    commandbar_t* commandbar;
    /// @brief Next command bar item in the dynamic list. Is managed by this module and does not need to be initialized by the user.
    commandbar_item_t* next;
    /// @brief Button that is used internally to draw the command bar. Must not be manipulated by the user.
    button_t internal_button;
};

/// @brief Options for the command bar.
typedef struct commandbar_options_s
{
    /// @brief Margin of the command bar.
    eve_ui_offset_t margin;
    /// @brief Orientation of the command bar.
    COMMANDBAR_ORIENTATION_T orientation;
    /// @brief Pointer to a list of items that should be shown.
    /// This will be the initial list where next does not need to be set. Next will be placed internally.
    commandbar_item_t* items;
    /// @brief Number of elements in @c items.
    size_t num_items;
    /// @brief Pointer to a list of items that should be shown as the footer.
    /// This will be the initial list where next does not need to be set. Next will be placed internally.
    commandbar_item_t* footer;
    /// @brief Number of elements in @c footer.
    size_t num_footer;
}commandbar_options_t;

/// @brief Command bar object structure
struct commandbar_s
{
    /// @brief Base component for the command bar.
    component_t component;
    /// @brief Background of the commandbar
    rect_t rect_background;
    /// @brief Options of the command bar.
    commandbar_options_t options;
    /// @brief Pointer to first item in the command bar.
    commandbar_item_t* items;
    /// @brief Pointer to first item in the command bar footer.
    commandbar_item_t* footer;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Intializes the module
**/
FUNCTION_RETURN_T commandbar_init(commandbar_t* c, const commandbar_options_t* options, eve_ui_location_t location);
/**
 * @brief 
 * 
 * @param c 
 * @param child 
 */
void commandbar_item_add(commandbar_t* c, commandbar_item_t* item);
/**
 * @brief 
 * 
 * @param c 
 * @param child 
 */
void commandbar_item_remove(commandbar_t* c, commandbar_item_t* item);
/**
 * @brief 
 * 
 * @param c 
 * @param child 
 */
void commandbar_footer_add(commandbar_t* c, commandbar_item_t* item);
/**
 * @brief 
 * 
 * @param c 
 * @param child 
 */
void commandbar_footer_remove(commandbar_t* c, commandbar_item_t* item);

#endif // MODULE_ENABLE_GUI

#endif /* __MODULE_COMMANDBAR_H_ */