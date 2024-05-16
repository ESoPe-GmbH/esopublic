/**
 * 	@file rect.h
 *  @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			This is the rectangle gui component. It can be used to draw backgrounds, so it should be added as
 *			one of the first components.
 *
 *  @version	1.00 (08.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef RECT_H_
#define RECT_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "color.h"
#include "component.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @struct rect_t
 *
 * This structure is used for painting a rectangle on the screen.
 */
typedef struct
{
	component_t component;	///< Component Object used for painting this object on the screen
	int32_t x1;				///< x-coordinate of the endpoint (x0 + width)
	int32_t y1;				///< y-coordinate of the endpoint (y0 + height)
	uint8_t line_width;			///< line width of the rectangle. The bigger this value is, the more curved is the edge.
	color_t c;				///< Color object for the background color.
}rect_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes a rectangle object which can be used to draw backgrounds.
 * 			The default color for a rectangle is black.
 *
 * @param obj				Pointer to the rectangle object
 * @param x					x-coordinate of the rectangle.
 * @param y					y-coordinate of the rectangle.
 * @param width				width of the rectangle
 * @param height			height of the rectangle
 * @param line_width		line width of the rectangle. The bigger this value is, the more curved is the edge.
 **/
void rect_init(rect_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, uint8_t line_width);

/**
 * @brief	Sets the color of the rectangle object
 *
 * @param obj				Pointer to the rectangle object
 * @param c					Color object for the background color.
 */
void rect_set_color(rect_t* obj, color_t c);

/**
 * @brief	Sets the line width of the rectangle object.
 *
 * @param obj				Pointer to the rectangle object
 * @param line_width		line width of the rectangle. The bigger this value is, the more curved is the edge.
 */
void rect_set_line_width(rect_t* obj, uint8_t line_width);

#endif

#endif /* RECT_H_ */
