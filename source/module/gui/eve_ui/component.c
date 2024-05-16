/***
 * @file component.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "component.h"
#include "screen.h"
#include "font.h"
#include "module/comm/dbg.h"

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
 * @brief 
 * 
 * @param c 
 * @param event 
 * @param data 
 */
void component_trigger_event(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void component_init(component_t* obj, uint8_t type, component_paint_cb_t f)
{
	if(obj == NULL)
		return;
	
	obj->type = type;
	obj->next_component = NULL;
	obj->parent_component = NULL;
	obj->paint_function = f;
	obj->is_visible = true;
	obj->is_enabled = true;
	obj->origin.x = 0;
	obj->origin.y = 0;
	obj->pressed_callback = NULL;
	obj->is_focusable = false;
	obj->mem_file_ptr = NULL;
	obj->keyboard_callback = NULL;
	obj->alignment = COMPONENT_ALIGNMENT_LEFT | COMPONENT_ALIGNMENT_TOP;
}

void component_set_event_callback(component_t* obj, component_event_cb_t f_cb_event, COMPONENT_EVENT_T events)
{
	if(obj == NULL)
		return;

	obj->f_cb_event = f_cb_event;
	obj->subscribed_events = events;
}

void component_set_visible(component_t* obj, bool b)
{
	if(obj == NULL)
		return;

	if(obj->is_visible == b)
		return;

	obj->is_visible = b;

	screen_t* s = screen_get_from_component(obj);
	if(s)
	{
		component_event_t event_data = 
		{
			.device = s->screen_device,
			.screen = s
		};
		component_trigger_event(obj, obj->is_visible ? COMPONENT_EVENT_LOAD : COMPONENT_EVENT_UNLOAD, &event_data);

		screen_repaint_by_component(obj);
	}
	else
	{		
		DBG_ERROR("Screen: NULL\n");
		component_t* c = obj;
		do
		{
			DBG_ERROR(" -> Component: %08x Type %d -> Parent: %08x\n", obj, obj->type, obj->parent_component);
			c = c->next_component;
		}while(c);
		
	}

}

void component_set_alignment(component_t* obj, COMPONENT_ALIGNMENT_T alignment)
{
	if(obj == NULL)
		return;

	obj->alignment = alignment;
}

void component_set_enabled(component_t* obj, bool b)
{
	if(obj == NULL)
		return;

	if(obj->is_enabled == b)
		return;

	obj->is_enabled = b;

	screen_repaint_by_component(obj);
}

eve_t* component_get_eve(component_t* obj)
{
	return screen_get_eve(screen_get_from_component(obj));
}

bool component_is_focused(component_t* obj)
{
#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	if(obj == NULL)
		return false;

	return screen_get_focused_component(screen_get_from_component(obj)) == obj;
#else
	return false;
#endif
}

void component_set_location(component_t* obj, eve_ui_point_t location)
{
	if(obj == NULL)
		return;

	if(obj->origin.x == location.x && obj->origin.y == location.y)
		return;

	obj->origin.x = location.x;
	obj->origin.y = location.y;

	screen_repaint_by_component(obj);
}

uint16_t component_get_x(component_t* obj, bool include_width)
{
	if(obj == NULL)
		return 0;

	if(include_width)
		return (obj->origin.x + obj->size.width);
	else
		return obj->origin.x;
}

uint16_t component_get_y(component_t* obj, bool include_height)
{
	if(obj == NULL)
		return 0;

	if(include_height)
		return (obj->origin.y + obj->size.height);
	else
		return obj->origin.y;
}

uint16_t component_get_width(component_t* obj)
{
	if(obj == NULL)
		return 0;

	return obj->size.width;
}

uint16_t component_get_height(component_t* obj)
{
	if(obj == NULL)
		return 0;

	return obj->size.height;
}

eve_ui_point_t component_get_origin(component_t* obj, eve_ui_point_t p)
{
	p.x += obj->origin.x;
	p.y += obj->origin.y;

	if(obj->alignment & COMPONENT_ALIGNMENT_LEFT)
	{
		// p.x is unchanged
	}
	else if(obj->alignment & COMPONENT_ALIGNMENT_RIGHT)
	{
		p.x = p.x - obj->size.width;
	}
	else if(obj->alignment & COMPONENT_ALIGNMENT_CENTER_X)
	{
		p.x = p.x - (obj->size.width / 2);
	}
	
	if(obj->alignment & COMPONENT_ALIGNMENT_TOP)
	{
		// p.y is unchanged
	}
	else if(obj->alignment & COMPONENT_ALIGNMENT_BOTTOM)
	{
		p.y = p.y - obj->size.height;
	}
	else if(obj->alignment & COMPONENT_ALIGNMENT_CENTER_Y)
	{
		p.y = p.y - (obj->size.height / 2);
	}

	return p;
}

eve_ui_point_t component_get_edge(component_t* obj, COMPONET_EDGE_T edge, eve_ui_point_t p)
{
	p = component_get_origin(obj, p);
	switch(edge)
	{
		case COMPONENT_EDGE_LEFT_TOP:
			return p;
		case COMPONENT_EDGE_LEFT_BOTTOM:
			return (eve_ui_point_t)
				{ 
					.x = p.x, 
					.y = p.y + obj->size.height
				};
		case COMPONENT_EDGE_RIGHT_TOP:
			return (eve_ui_point_t)
				{ 
					.x = p.x + obj->size.width, 
					.y = p.y
				};
		case COMPONENT_EDGE_RIGHT_BOTTOM:
			return (eve_ui_point_t)
				{ 
					.x = p.x + obj->size.width, 
					.y = p.y + obj->size.height
				};
	}
	return p;
}

bool component_needs_cyclic_repaint(component_t* obj)
{
	return obj->type == COMPONENT_TYPE_EDIT_TEXT;
}

void component_trigger_event(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data)
{
	if(event == COMPONENT_EVENT_LOAD)
	{
		// Stop if component is not visible or was already set as loaded
		if(!c->is_visible || c->is_loaded)
		{
			return;
		}
		c->is_loaded = true;
	}

	if(event == COMPONENT_EVENT_UNLOAD)
	{
		// Stop if component was already unloaded
		if(!c->is_loaded)
		{
			return;
		}
		c->is_loaded = false;
	}

	// Notify subcomponents about the event if they set the pointer
	if(c->f_cb_event_internal)
	{
		c->f_cb_event_internal(c, event, data);
	}

	// Notify the user about the event if they subscribed to the event.
	if(c->f_cb_event && (c->subscribed_events & event))
	{
		c->f_cb_event(c, event, data);
	}
	// else
	// {
	// 	DBG_VERBOSE("Component %d event %d\n", c->type, event);
	// }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif
