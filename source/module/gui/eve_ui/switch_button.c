/***
 * @file switch_button.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "switch_button.h"

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "component.h"
#include "screen.h"
#include "../eve/eve_copro.h"

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
 * Paints the switch button onto the screen.
 * The position, color etc. must be set before it is painted with screen_paint or screen_repaint.
 *
 * @param obj			Pointer to the switch button object that needs to be painted
 * @param p				Coordinates of the parent component.
 */
static void switch_button_paint(switch_button_t* obj, eve_ui_point_t p);

/**
 * Callback function for a callback when the button is pressed or released. Changes the state internally.
 *
 * @param obj			Pointer to the switch button object.
 * @param b				true: Button is pressed, false: Button is not pressed.
 */
static void switch_button_set_pressed(switch_button_t* obj, bool b);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void switch_button_init(switch_button_t* obj, int32_t x, int32_t y, uint16_t width, char* text)
{
	if(obj == NULL)
		return;

	// Initialize component object
	component_init((component_t*)obj, COMPONENT_TYPE_SWITCH_BUTTON, (component_paint_cb_t)switch_button_paint);

	// Set callback for the function that is called when the button is pressed.
	obj->component.pressed_callback = (component_pressed_cb_t)switch_button_set_pressed;
	// Set coordinates
	obj->component.origin.x = x;
	obj->component.origin.y = y;
	// Set size
	obj->component.size.width = width;
	// Set text
	obj->text = text;
	// Set default values
	obj->font = SWITCH_BUTTON_DEFAULT_TEXT_FONT;
	obj->color_background = color_get(SWITCH_BUTTON_DEFAULT_BACKGROUND_COLOR);
	obj->color_foreground = color_get(SWITCH_BUTTON_DEFAULT_FOREGROUND_COLOR);
	obj->color_text = color_get(SWITCH_BUTTON_DEFAULT_TEXT_COLOR);
	obj->color_background_disabled = color_get(SWITCH_BUTTON_DEFAULT_BACKGROUND_COLOR_DISABLED);
	obj->color_text_disabled = color_get(SWITCH_BUTTON_DEFAULT_TEXT_COLOR_DISABLED);
	obj->option = 0;
	obj->state = SWITCH_BUTTON_STATE_1;
}

void switch_button_set_text(switch_button_t* obj, char* text)
{
	if(obj == NULL)
		return;

	obj->text = text;
}

void switch_button_set_visible(switch_button_t* obj, bool b)
{
	if(obj == NULL)
		return;

	if(obj->component.is_visible == b)
		return;

	obj->component.is_visible = b;

	screen_repaint((screen_device_t*)component_get_eve((component_t*)obj));	// Repaint screen so that the button is invisible
}

void switch_button_set_enabled(switch_button_t* obj, bool b)
{
	if(obj == NULL)
		return;

	if(obj->component.is_enabled == b)
		return;

	obj->component.is_enabled = b;

	screen_repaint((screen_device_t*)component_get_eve((component_t*)obj));	// Repaint screen so that the button is invisible
}

void switch_button_set_state(switch_button_t* obj, SWITCH_BUTTON_STATE state)
{
	if(obj == NULL)
		return;

	if(obj->state == state)
		return;

	obj->state = state;

	screen_repaint((screen_device_t*)component_get_eve((component_t*)obj));	// Repaint screen so that the button is invisible
}

void switch_button_set_state_2_active(switch_button_t* obj, bool b)
{
	if(b)
		switch_button_set_state(obj, SWITCH_BUTTON_STATE_2);
	else
		switch_button_set_state(obj, SWITCH_BUTTON_STATE_1);
}

void switch_button_set_action(switch_button_t* obj, void(*f)(switch_button_t*, SWITCH_BUTTON_STATE))
{
	if(obj == NULL)
		return;

	obj->action_callback = (void(*)(void*, SWITCH_BUTTON_STATE))f;
}

void switch_button_set_font(switch_button_t* obj, uint16_t font)
{
	if(obj == NULL)
		return;

	obj->font = font;
}

void switch_button_set_backgroundcolor(switch_button_t* obj, color_t color)
{
	if(obj == NULL)
		return;

	obj->color_background = color;
}

void switch_button_set_textcolor(switch_button_t* obj, color_t color)
{
	if(obj == NULL)
		return;

	obj->color_text = color;
}

void switch_button_set_backgroundcolor_disabled(switch_button_t* obj, color_t color)
{
	if(obj == NULL)
		return;

	obj->color_background_disabled = color;
}

void switch_button_set_textcolor_disabled(switch_button_t* obj, color_t color)
{
	if(obj == NULL)
		return;

	obj->color_text_disabled = color;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void switch_button_paint(switch_button_t* obj, eve_ui_point_t p)
{
	eve_t* eve = component_get_eve((component_t*)obj);

	if(obj == NULL)	// Do not paint anything if object is invalid or invisible
		return;

	if(obj->text == NULL)
		return;

	eve_copro_add_tag(eve, (component_t*)obj);

	// Set the text color
	if(obj->component.is_enabled)
		eve_copro_set_color(eve, obj->color_text);
	else
		eve_copro_set_color(eve, obj->color_text_disabled);
	// Set the button foreground color
	eve_copro_set_foreground(eve, obj->color_foreground);
	// Set the button background color
	if(obj->component.is_enabled)
		eve_copro_set_background(eve, obj->color_background);
	else
		eve_copro_set_background(eve, obj->color_background_disabled);

	// Draw a button to the screen
	eve_copro_switch_button(eve, obj, p.x, p.y);

	eve_copro_clear_tag(eve);
}

static void switch_button_set_pressed(switch_button_t* obj, bool b)
{
	if(obj == NULL)
		return;

	if(!obj->component.is_enabled) // no action when not enabled
		return;

	if(!b) // Ignore release events
		return;

	if(obj->state == SWITCH_BUTTON_STATE_1)
		obj->state = SWITCH_BUTTON_STATE_2;
	else
		obj->state = SWITCH_BUTTON_STATE_1;

	if(obj->action_callback != NULL)
		obj->action_callback(obj, obj->state);

	screen_repaint((screen_device_t*)component_get_eve((component_t*)obj));	// Repaint screen to show the pressed color
}

#endif
