/***
 * @file rect.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "rect.h"
#include "../eve/eve_register.h"
#include "../eve/eve_copro.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * Paints the rectangle onto the screen.
 * The position, color etc. must be set before it is painted with screen_paint or screen_repaint.
 *
 * @param obj		Pointer to the rectangle object that needs to be painted
 * @param p			Coordinates of the parent component.
 */
static void rect_paint(rect_t* obj, eve_ui_point_t p);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void rect_init(rect_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, uint8_t line_width)
{
	if(obj == NULL)
		return;

	component_init((component_t*)obj, COMPONENT_TYPE_RECTANGLE, (component_paint_cb_t)rect_paint);

	obj->component.origin.x = x;
	obj->component.origin.y = y;
	obj->component.size.width = width;
	obj->component.size.height = height;
	obj->line_width = line_width;
	obj->c = color_get(RECT_DEFAULT_BACKGROUND);
}

void rect_set_color(rect_t* obj, color_t c)
{
	if(obj == NULL)
		return;

	obj->c = c;
}

void rect_set_line_width(rect_t* obj, uint8_t line_width)
{
	if(obj == NULL)
		return;

	obj->line_width = line_width;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
static void rect_paint(rect_t* obj, eve_ui_point_t p)
{
	eve_t* eve = component_get_eve((component_t*)obj);

	if(eve == NULL || obj == NULL)
		return;

	obj->x1 = (p.x + obj->component.origin.x + (int32_t)obj->component.size.width);
	obj->y1 = (p.y + obj->component.origin.y + (int32_t)obj->component.size.height);

	// Set color
	eve_copro_set_color(eve, obj->c);
	eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
	// Set line width
	eve_copro_write_command(eve, EVE_LINE_WIDTH(16 * obj->line_width));
	// Draw rectangle
	eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));					// Start rectangle
	eve_copro_write_command(eve, EVE_VERTEX2F((obj->component.origin.x + p.x), (obj->component.origin.y + p.y)));	// Start point of rectangle
	eve_copro_write_command(eve, EVE_VERTEX2F(obj->x1, obj->y1));	// End point of rectangle
	eve_copro_write_command(eve, EVE_END());									// End rectangle
}

#endif
