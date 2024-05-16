/***
 * @file panel.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "panel.h"
#include "screen.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Internal paint function that goes through the dynamic component list of the own rectangle and drawing
 * 			all components that were added there.
 *
 * @param obj			Pointer to the panel object.
 * @param p				Coordinate of the parent component.
 */
static void panel_paint(panel_t* obj, eve_ui_point_t p);
/**
 * @brief Event handler that is called when component events are triggered.
 * 
 * @param obj			Pointer to the component of the panel.
 * @param event 		Type of event that occurred.
 * @param data 			Pointer to the data regarding the event.
 */
static void _event(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data);

extern void component_trigger_event(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void panel_init(panel_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, bool draw_background)
{
	if(obj == NULL)
		return;

	component_init((component_t*)obj, COMPONENT_TYPE_PANEL, (component_paint_cb_t)panel_paint);
	obj->component.f_cb_event_internal = _event;
	rect_init(&obj->rect_bg, 0, 0, width, height, 1);

	obj->component.origin.x = x;
	obj->component.origin.y = y;
	obj->component.size.width = width;
	obj->component.size.height = height;
	obj->parent_component = &obj->rect_bg.component;
	obj->rect_bg.component.is_visible = draw_background;
	obj->rect_bg.component.parent_component = &obj->component;
}

void panel_add_component(panel_t* obj, component_t* comp)
{
	if(obj == NULL || comp == NULL)
		return;

	obj->parent_component->next_component = comp;
	obj->parent_component = comp;

	// Set Panel as parent for the component
	comp->parent_component = (void*)obj;
}

void panel_set_background_color(panel_t* obj, color_t c)
{
	rect_set_color(&obj->rect_bg, c);
	obj->rect_bg.component.is_visible = true;
}

void panel_disable_background(panel_t* obj)
{
	obj->rect_bg.component.is_visible = false;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
static void panel_paint(panel_t* obj, eve_ui_point_t p)
{
	component_t* comp;
#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	eve_t* eve = component_get_eve(&obj->component);
#endif
	screen_t* s = screen_get_from_component(&obj->component);

	if(obj == NULL || !obj->component.is_visible)	// Do not paint anything if object is invalid or invisible
		return;

	// Draw all panel components, starting with the own rectangle background
	comp = (component_t*)&obj->rect_bg;
	p.x += obj->component.origin.x;
	p.y += obj->component.origin.y;

	while(comp != NULL)
	{
		if(comp->paint_function != NULL && comp->is_visible)
		{
			//comp->paint_function(comp, x + obj->component.origin.x, y + obj->component.origin.y);// Screen uses its x and y coordinates offset for x and y.
			comp->paint_function(comp, p);
			s->screen_device->needs_cyclic_repaint |= component_needs_cyclic_repaint(comp);

#if SCREEN_ENABLE_FOCUSED_COMPONENTS
			if(comp == s->focused_component)
			{
				int32_t x1, y1;

				component_t* c = s->focused_component;
				x1 = p.x + c->x;
				y1 = p.y + c->y;

				// Set color
				eve_copro_set_color(eve, color_get(GUI_CONFIG_FOCUS_BORDER_COLOR));
				eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
				// Set line width
				eve_copro_write_command(eve, EVE_LINE_WIDTH(16 * 1));
				eve_copro_write_command(eve, EVE_BEGIN(EVE_LINES));
				eve_copro_write_command(eve, EVE_VERTEX2F(x1, y1-1));
				eve_copro_write_command(eve, EVE_VERTEX2F(x1 + c->width - 1, y1-1));

				eve_copro_write_command(eve, EVE_VERTEX2F(x1 + c->width, y1-1));
				eve_copro_write_command(eve, EVE_VERTEX2F(x1 + c->width, y1 + c->height));

				eve_copro_write_command(eve, EVE_VERTEX2F(x1 + c->width, y1 + c->height+1));
				eve_copro_write_command(eve, EVE_VERTEX2F(x1, y1 + c->height+1));

				eve_copro_write_command(eve, EVE_VERTEX2F(x1-1, y1 + c->height));
				eve_copro_write_command(eve, EVE_VERTEX2F(x1-1, y1));
				eve_copro_write_command(eve, EVE_END());
			}
#endif // SCREEN_ENABLE_FOCUSED_COMPONENTS
		}
		comp = comp->next_component;
	}

}

static void _event(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data)
{
	panel_t* p = __containerof(c, panel_t, component);
	component_t* comp = &p->rect_bg.component;
	
	while(comp)
	{
		component_trigger_event(comp, event, data);
		comp = comp->next_component;
	}
}

#endif
