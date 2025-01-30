/***
 * @file circle.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "circle.h"
#include "../eve/eve_register.h"
#include "../eve/eve_copro.h"

#if !defined(CIRCLE_DEFAULT_BACKGROUND)
#define CIRCLE_DEFAULT_BACKGROUND 		COLOR_BLACK
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * Paints the circle onto the screen.
 * The position, color etc. must be set before it is painted with screen_paint or screen_repaint.
 *
 * @param obj		Pointer to the circle that needs to be painted
 * @param p			Coordinates of the parent component.
 */
static void circle_paint(circle_t* obj, eve_ui_point_t p);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void circle_init(circle_t* obj, int32_t x, int32_t y, float radius)
{
	if(obj == NULL)
		return;

	component_init((component_t*)obj, COMPONENT_TYPE_CIRCLE, (component_paint_cb_t)circle_paint);

	obj->component.origin.x = x;
	obj->component.origin.y = y;
	obj->component.alignment = COMPONENT_ALIGNMENT_CENTER;
	obj->component.size.width = radius * 2;
	obj->component.size.height = radius * 2;
	obj->radius = radius;
	obj->c = color_get(CIRCLE_DEFAULT_BACKGROUND);
}

void circle_set_color(circle_t* obj, color_t c)
{
	if(obj == NULL)
		return;

	obj->c = c;
}

void circle_set_radius(circle_t* obj, float radius)
{
	if(obj == NULL)
		return;

	obj->radius = radius;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
static void circle_paint(circle_t* obj, eve_ui_point_t p)
{
	eve_t* eve = component_get_eve((component_t*)obj);

	if(eve == NULL || obj == NULL)
		return;

	eve_copro_set_color(eve, obj->c);
	eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); 
	eve_copro_write_command(eve, EVE_POINT_SIZE((int)(16.0 * obj->radius)));
	eve_copro_write_command(eve, EVE_BEGIN(EVE_POINTS));
	eve_copro_write_command(eve, EVE_VERTEX2F((obj->component.origin.x + p.x), (obj->component.origin.y + p.y)));
	eve_copro_write_command(eve, EVE_END());									
}

#endif
