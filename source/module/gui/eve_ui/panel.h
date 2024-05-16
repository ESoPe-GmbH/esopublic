/**
 * 	@file panel.h
 *  @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief	The panel component is a container class like a screen object and can be used to make a container of components
 *  		to use with different screens. Note that each screen needs it own panel object for this! Otherwise it will not work
 *  		if more screens share one panel object.
 *
 *  		A Panel can be drawn with a background or without a background.
 *
 *  		Remember that the x/y coordinates of components added to a panel are relative to the panel and not absolute
 *  		to the screen!
 *
 *	@version	1.01 (07.12.2018)
 *		- Added checking of component_needs_cyclic_repaint
 *  @version	1.00 (12.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef PANEL_H_
#define PANEL_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "color.h"
#include "component.h"
#include "rect.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @struct panel_t
 *
 * The panel object is a container class like a screen object. It uses a dynamic list to paint components added to the panel.
 * It has a rectangle that can be drawn as a background for the panel.
 *
 * Remember that the x/y coordinates of components added to a panel are relative to the panel and not absolute
 * to the screen!
 */
typedef struct
{
	component_t component;				///< Component Object used for painting this object on the screen.

	rect_t rect_bg;						///< Is used to draw a background to the panel when paint_background is true.

	component_t* parent_component;			///< Pointer to the last object added to the panel.
}panel_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes the panel object with the parameters and resets the dynamic component list that is used
 *			to store the components that are drawn on this panel.
 *
 * @param obj						Pointer to the panel object
 * @param x							x Coordinate of the panel on the screen (or on another panel).
 * @param y							y Coordinate of the panel on the screen (or on another panel).
 * @param width						Width of the panel. Only relevant if a background should be drawn.
 * @param height					Height of the panel. Only relevant if a background should be drawn.
 * @param draw_background			true: The background is drawn with a color, false: The background is not drawn.
 **/
void panel_init(panel_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, bool draw_background);

/**
 * @brief	Adds a component to the panel and modifies its x/y coordinates, so that the coordinates are relative to the
 * 			panels position and not absolute coordinates on the screen.
 *
 * @param obj						Pointer to the panel object
 * @param comp						Pointer to the component that should be added to the panel.
 */
void panel_add_component(panel_t* obj, component_t* comp);

/**
 * @brief	Sets a background color for the panel and sets the background to visible.
 *
 *
 * @param obj						Pointer to the panel object
 * @param c							Color of the background
 */
void panel_set_background_color(panel_t* obj, color_t c);

/**
 * @brief	Disables the background of the panel, which makes components below the panel visible.
 *
 * @param obj						Pointer to the panel object
 */
void panel_disable_background(panel_t* obj);

#endif

#endif /* PANEL_H_ */
