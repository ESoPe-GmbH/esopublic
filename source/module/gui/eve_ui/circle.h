/**
 * 	@file circle.h
 *  @copyright Urheberrecht 2014-2025 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			This is the circle gui component. It can be used to draw circles.
 *
 *  @version	1.00 (30.01.2025)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef CIRCLE_H_
#define CIRCLE_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "color.h"
#include "component.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @struct circle_t
 *
 * This structure is used for painting a circle on the screen.
 */
typedef struct
{
	/// Component Object used for painting this object on the screen
	component_t component;	
	/// Radius of the circle
	float radius;			
	/// Color for the background.
	color_t c;				
}circle_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes a circle which can be used to draw backgrounds.
 * 			The default color for a circle is black if not specified otherwise using CIRCLE_DEFAULT_BACKGROUND.
 *
 * @param obj				Pointer to the circle
 * @param x					x-coordinate of the circle.
 * @param y					y-coordinate of the circle.
 * @param radius			Radius of the circle
 **/
void circle_init(circle_t* obj, int32_t x, int32_t y, float radius);

/**
 * @brief	Sets the color of the circle
 *
 * @param obj				Pointer to the circle
 * @param c					Color object for the background color.
 */
void circle_set_color(circle_t* obj, color_t c);

/**
 * @brief	Sets the radius of the circle.
 *
 * @param obj				Pointer to the circle
 * @param radius			Radius of the circle
 */
void circle_set_radius(circle_t* obj, float radius);

#endif

#endif /* CIRCLE_H_ */
