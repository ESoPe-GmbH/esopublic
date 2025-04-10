/***
 * @file button.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "button.h"
#include "../eve/eve_register.h"
#include "../eve/eve_copro.h"
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

/// Variable for calculations
static uint16_t	button_tmp16 = 0;

/// Pointer to a global action callback function.
/// Might be needed if each button action needs to be logged globally.
static void (*button_global_action_callback)(button_t*) = NULL;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * Paints the button onto the screen.
 * The position, color etc. must be set before it is painted with screen_paint or screen_repaint.
 *
 * @param obj			Pointer to the button object that needs to be painted
 * @param p				Coordinates of the parent component.
 */
static void button_paint(button_t* obj, eve_ui_point_t p);

/**
 * Changes the pressed status of the button. The function should only be used internally in the eve module.
 *
 * @param obj			Pointer to the button object.
 * @param b				true: Button is pressed, false: Button is not pressed.
 */
static void button_set_pressed(button_t* obj, bool b);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void button_init(button_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, char* text)
{
	if(obj == NULL)
		return;

	// Initialize component object
	component_init((component_t*)obj, COMPONENT_TYPE_BUTTON, (component_paint_cb_t)button_paint);

	// Set callback for the function that is called when the button is pressed.
	obj->component.pressed_callback = (component_pressed_cb_t)button_set_pressed;
	// Set coordinates
	obj->component.origin.x = x;
	obj->component.origin.y = y;
	// Set size
	obj->component.size.width = width;
	obj->component.size.height = height;
	// Set text
	obj->text = text;
	obj->figure = NULL;
	// Set Default Type
	obj->type = BUTTON_DEFAULT_TYPE;
	// Set default values
	obj->font = BUTTON_DEFAULT_TEXT_FONT;
	obj->color_background = color_get(BUTTON_DEFAULT_BACKGROUND_COLOR);
	obj->color_background_pressed = color_get(BUTTON_DEFAULT_BACKGROUND_COLOR_PRESSED);
	obj->color_text = color_get(BUTTON_DEFAULT_TEXT_COLOR);
	obj->color_text_pressed = color_get(BUTTON_DEFAULT_TEXT_COLOR_PRESSED);
	obj->color_background_disabled = color_get(BUTTON_DEFAULT_BACKGROUND_COLOR_DISABLED);
	obj->color_text_disabled = color_get(BUTTON_DEFAULT_TEXT_COLOR_DISABLED);
	obj->option = 0;
	obj->pressed = false;
	obj->component.is_focusable = true;
	obj->option_text = EVE_OPT_TEXT_CENTER;
	obj->figure_margin = 4;
	obj->enable_text_shadow = true;
}

void button_set_type(button_t* obj, BUTTON_TYPE type)
{
	if(obj == NULL)
		return;

	obj->type = type;

	screen_repaint_by_component((component_t*)obj);
}

void button_set_text(button_t* obj, char* text)
{
	if(obj == NULL)
		return;

	obj->text = text;
}

void button_set_text_horizontal_alignment(button_t* obj, TEXT_H_ALIGNMENT align)
{
	if(obj == NULL)
		return;

	switch(align)
	{
		case TEXT_H_ALIGNMENT_LEFT:
			obj->option_text &= ~(EVE_OPT_TEXT_CENTERX | EVE_OPT_TEXT_RIGHTX);	// Remove horizontal center and right alignment
		break;

		case TEXT_H_ALIGNMENT_CENTER:
			obj->option_text &= ~EVE_OPT_TEXT_RIGHTX;	// Remove right alignment
			obj->option_text |= EVE_OPT_TEXT_CENTERX;		// Set horizontal center alignment
		break;

		case TEXT_H_ALIGNMENT_RIGHT:
			obj->option_text &= ~EVE_OPT_TEXT_CENTERX;	// Remove horizontal center
			obj->option_text |= EVE_OPT_TEXT_RIGHTX;		// Set horizontal right alignment
		break;
	}
}

void button_set_text_vertical_alignment(button_t* obj, TEXT_V_ALIGNMENT align)
{
	if(obj == NULL)
		return;

	switch(align)
	{
		case TEXT_V_ALIGNMENT_TOP:
			obj->option_text &= ~EVE_OPT_TEXT_CENTERY;		// Remove vertical center alignment -> top is default
		break;

		case TEXT_V_ALIGNMENT_CENTER:
			obj->option_text |= EVE_OPT_TEXT_CENTERY;		// Set vertical center alignment
		break;

		case TEXT_V_ALIGNMENT_BOTTOM:
			obj->option_text &= ~EVE_OPT_TEXT_CENTERY;
			// TODO: Implement bottom alignment for text in buttons
		break;
	}
}

void button_set_text_bold(button_t* obj, bool is_bold)
{
	if(obj == NULL)
		return;

	obj->is_bold = is_bold;
}

void button_set_visible(button_t* obj, bool b)
{
	if(obj == NULL)
		return;

	if(obj->component.is_visible == b)
		return;

	obj->component.is_visible = b;

}

void button_set_enabled(button_t* obj, bool b)
{
	if(obj == NULL)
		return;

	if(obj->component.is_enabled == b)
		return;

	obj->component.is_enabled = b;

//	dbg_printf(DBG_STRING, "button_set_enabled(%s, %d)\n", obj->text, b);

	// Repaint screen so that the button is enabled/disabled
	screen_repaint_by_component((component_t*)obj);
}

void button_set_action(button_t* obj, void(*pressed_callback)(button_t*))
{
	if(obj == NULL)
		return;

	obj->action_callback = (void(*)(void*))pressed_callback;
}

void button_set_global_action(void(*pressed_callback)(button_t*))
{
	button_global_action_callback = (void(*)(button_t*))pressed_callback;
}

void button_set_font(button_t* obj, uint16_t font)
{
	if(obj == NULL)
		return;

	obj->font = font;
}

void button_set_backgroundcolor(button_t* obj, color_t color)
{
	if(obj == NULL)
		return;

	obj->color_background = color;
}

void button_set_textcolor(button_t* obj, color_t color)
{
	if(obj == NULL)
		return;

	obj->color_text = color;
}

void button_set_backgroundcolor_pressed(button_t* obj, color_t color)
{
	if(obj == NULL)
		return;

	obj->color_background_pressed = color;
}

void button_set_figure(button_t* obj, component_t* c, BUTTON_FIGURE_POS figure_pos)
{
	if(obj == NULL)
		return;

	obj->figure = c;
	obj->figure_pos = figure_pos;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
static void button_paint(button_t* obj, eve_ui_point_t p)
{
	uint16_t w, h;
	eve_t* eve = component_get_eve((component_t*)obj);
	bool has_text = true;

	if(eve == NULL || obj == NULL)	// Do not paint anything if object is invalid or invisible
		return;

	w = obj->component.size.width;
	h = obj->component.size.height;

	p = component_get_origin(&obj->component, p);

	eve_copro_add_tag(eve, (component_t*)obj);

	switch(obj->type)
	{
		case BUTTON_TYPE_API:
			if(!obj->component.is_enabled)
			{
				// Set the text color
				eve_copro_set_color(eve, obj->color_text_disabled);
				// Set the button background color (foreground command is needed for this...)
				eve_copro_set_foreground(eve, obj->color_background_disabled);
			}
			else if(obj->pressed)
			{
				// Set the text color
				eve_copro_set_color(eve, obj->color_text_pressed);
				// Set the button background color (foreground command is needed for this...)
				eve_copro_set_foreground(eve, obj->color_background_pressed);
			}
			else
			{
				// Set the text color
				eve_copro_set_color(eve, obj->color_text);
				// Set the button background color (foreground command is needed for this...)
				eve_copro_set_foreground(eve, obj->color_background);
			}
			eve_copro_set_gradientcolor(eve, obj->color_background);
			eve_copro_button(eve, p.x, p.y, w, h, obj->option, obj->font, obj->text);
			has_text = false;

		break;

		case BUTTON_TYPE_API_LIKE:

			eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
			// Set line width
			eve_copro_write_command(eve, EVE_LINE_WIDTH(16 * 3));
			// Draw shadow rectangle
			eve_copro_set_color(eve, color_get(BUTTON_DEFAULT_SHADOW_COLOR));
			eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));
			eve_copro_write_command(eve, EVE_VERTEX2F(p.x + 5, p.y + 5));
			eve_copro_write_command(eve, EVE_VERTEX2F(p.x + w - 2, p.y + h - 2));
			eve_copro_write_command(eve, EVE_END());
			if(obj->component.is_enabled)
			{
				// Draw highlight rectangle
				eve_copro_set_color(eve, color_get(BUTTON_DEFAULT_HIGHLIGHT_COLOR));
				eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));
				eve_copro_write_command(eve, EVE_VERTEX2F(p.x + 3, p.y + 3));
				eve_copro_write_command(eve, EVE_VERTEX2F(p.x + w - 4, p.y + h - 4));
				eve_copro_write_command(eve, EVE_END());
			}
			 // Set the button background color
			if(!obj->component.is_enabled)
				eve_copro_set_color(eve, obj->color_background_disabled);
			else if(obj->pressed)
				eve_copro_set_color(eve, obj->color_background_pressed);
			else
				eve_copro_set_color(eve, obj->color_background);

			// Draw main button
			eve_copro_write_command(eve, EVE_VERTEX_FORMAT(1)); // Pixel precision: 2
			eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));
			eve_copro_write_command(eve, EVE_VERTEX2F(2 * (p.x + 3) + 1, 2 * (p.y + 3) + 1));
			eve_copro_write_command(eve, EVE_VERTEX2F(2 * (p.x + w - 3), 2 * (p.y + h - 3)));
			eve_copro_write_command(eve, EVE_END());

//			eve_copro_set_color(eve, color_get(BUTTON_DEFAULT_SHADOW_COLOR));
//			eve_copro_text(	eve,
//									x + (w / 2) - 1,
//									y + (h / 2) - 1,
//									obj->font,
//									EVE_OPT_TEXT_CENTER,
//									obj->text
//									);
		break;

		case BUTTON_TYPE_RECT:

			eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
			// Set line width
			eve_copro_write_command(eve, EVE_LINE_WIDTH(16 * 1));

//			eve_copro_set_color(eve, color_get(BUTTON_DEFAULT_SHADOW_COLOR));
//			// Draw shadow rectangle
//			eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));					// Start rectangle
//			eve_copro_write_command(eve, EVE_VERTEX2F(16 * (obj->component.origin.x + x + 1), 16 * (obj->component.origin.y + y + 1)));	// Start point of rectangle
//			eve_copro_write_command(eve, EVE_VERTEX2F(16 * (obj->component.origin.x + x + obj->component.size.width + 1), 16 * (obj->component.origin.y + y + obj->component.size.height + 1)));	// End point of rectangle
//			eve_copro_write_command(eve, EVE_END());

			 // Set the button background color
			if(!obj->component.is_enabled)
				eve_copro_set_color(eve, obj->color_background_disabled);
			else if(obj->pressed)
				eve_copro_set_color(eve, obj->color_background_pressed);
			else
				eve_copro_set_color(eve, obj->color_background);

			// Draw rectangle
			eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));					// Start rectangle
			eve_copro_write_command(eve, EVE_VERTEX2F(p.x, p.y));	// Start point of rectangle
			eve_copro_write_command(eve, EVE_VERTEX2F((p.x + w), (p.y + h)));	// End point of rectangle
			eve_copro_write_command(eve, EVE_END());
		break;

		case BUTTON_TYPE_ROUND:

			if(obj->component.size.height < obj->component.size.width)
				button_tmp16 = (obj->component.size.height / 2);
			else
				button_tmp16 = (obj->component.size.width / 2);

			eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
			// Set line width
			eve_copro_write_command(eve, EVE_LINE_WIDTH(16 * button_tmp16));

			eve_copro_set_color(eve, color_get(COLOR_BLACK));
			// Draw shadow rectangle
			eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));					// Start rectangle
			eve_copro_write_command(eve, EVE_VERTEX2F((p.x + button_tmp16 + 1), (p.y + button_tmp16 + 1)));	// Start point of rectangle
			eve_copro_write_command(eve, EVE_VERTEX2F((p.x + w - button_tmp16 + 1), (p.y + button_tmp16 + 1)));	// End point of rectangle
			eve_copro_write_command(eve, EVE_END());

			 // Set the button background color
			if(!obj->component.is_enabled)
				eve_copro_set_color(eve, obj->color_background_disabled);
			else if(obj->pressed)
				eve_copro_set_color(eve, obj->color_background_pressed);
			else
				eve_copro_set_color(eve, obj->color_background);

			// Draw rectangle
			eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));					// Start rectangle
			eve_copro_write_command(eve, EVE_VERTEX2F((p.x + button_tmp16), p.y + button_tmp16));	// Start point of rectangle
			eve_copro_write_command(eve, EVE_VERTEX2F((p.x + w - button_tmp16), (p.y + button_tmp16)));	// End point of rectangle
			eve_copro_write_command(eve, EVE_END());
		break;
	}

	if(has_text)
	{
		uint16_t x0, y0;

		x0 = p.x + (w / 2);
		y0 = p.y + (h / 2);

		// Left aligned
		if((obj->option_text & (EVE_OPT_TEXT_CENTERX | EVE_OPT_TEXT_RIGHTX)) == 0)
		{
			x0 = p.x + obj->figure_margin;
			if(obj->figure != NULL && obj->figure->is_visible && (obj->figure_pos & BUTTON_FIGURE_POS_LEFT) == BUTTON_FIGURE_POS_LEFT)
				x0 += obj->figure->size.width;
		}
		else if((obj->option_text & EVE_OPT_TEXT_RIGHTX) == EVE_OPT_TEXT_RIGHTX)
		{
			x0 = p.x + w - obj->figure_margin;
			if(obj->figure != NULL && obj->figure->is_visible && (obj->figure_pos & BUTTON_FIGURE_POS_RIGHT) == BUTTON_FIGURE_POS_RIGHT)
				x0 -= obj->figure->size.width;
		}

		if((obj->option_text & EVE_OPT_TEXT_CENTERY) == 0)
		{
			y0 = p.y + obj->figure_margin;
		}

		if(obj->enable_text_shadow)
		{
			eve_copro_set_color(eve, color_get(BUTTON_DEFAULT_SHADOW_COLOR));
			eve_copro_text(	eve,	obj->is_bold ? x0 + 2 : x0 + 1,	y0+1,	obj->font, obj->option_text, obj->text);
		}

		 // Set the text background color
		if(!obj->component.is_enabled)
			eve_copro_set_color(eve, obj->color_text_disabled);
		else if(obj->pressed)
			eve_copro_set_color(eve, obj->color_text_pressed);
		else
			eve_copro_set_color(eve, obj->color_text);

		eve_copro_text(	eve,	x0,	y0,	obj->font, obj->option_text, obj->text);
		if(obj->is_bold)
			eve_copro_text(	eve,	x0+1, y0,	obj->font, obj->option_text, obj->text);
	}

	if(obj->figure != NULL && obj->figure->is_visible)
	{
		eve_ui_point_t origin_image = 
		{
			// X-Coordinate for the image (center by default)
			.x = p.x + obj->figure->origin.x,
			// Y-Coordinate for the image (center by default)
			.y = p.y + obj->figure->origin.y
		};

		if((obj->figure_pos & BUTTON_FIGURE_POS_CENTER_X) == BUTTON_FIGURE_POS_CENTER_X)
		{
			// Center the image horizontally
			origin_image.x = p.x + ((obj->component.size.width - obj->figure->size.width) / 2);
			obj->figure->origin.x = 0;
		}
		else if((obj->figure_pos & BUTTON_FIGURE_POS_LEFT) == BUTTON_FIGURE_POS_LEFT)
		{
			origin_image.x = p.x + obj->figure_margin;
			obj->figure->origin.x = 0;
		}
		else if((obj->figure_pos & BUTTON_FIGURE_POS_RIGHT) == BUTTON_FIGURE_POS_RIGHT)
		{
			origin_image.x = p.x + w - obj->figure->size.width - obj->figure_margin;
			obj->figure->origin.x = 0;
		}

		if((obj->figure_pos & BUTTON_FIGURE_POS_CENTER_Y) == BUTTON_FIGURE_POS_CENTER_Y)
		{
			// Center the image vertically
			origin_image.y = p.y + ((obj->component.size.height - obj->figure->size.height) / 2);
			obj->figure->origin.y = 0;
		}
		else if((obj->figure_pos & BUTTON_FIGURE_POS_TOP) == BUTTON_FIGURE_POS_TOP)
		{
			origin_image.y = p.y + obj->figure_margin;
			obj->figure->origin.y = 0;
		}
		else if((obj->figure_pos & BUTTON_FIGURE_POS_BOTTOM) == BUTTON_FIGURE_POS_BOTTOM)
		{
			origin_image.y = p.y + h - obj->figure->size.height - obj->figure_margin;
			obj->figure->origin.y = 0;
		}

		eve_copro_use_last_tag(eve);
		obj->figure->parent_component = (component_t*)obj;

		obj->figure->paint_function(obj->figure, origin_image);
	}

	eve_copro_clear_tag(eve);
}

static void button_set_pressed(button_t* obj, bool b)
{
	if(obj == NULL)
		return;

	if(!obj->component.is_enabled) // no action when not enabled
		return;

	if(obj->pressed == b)
		return;

	obj->pressed = b;

	if(!b)
	{
		// Trigger global button action listener
		if(button_global_action_callback != NULL)
			button_global_action_callback(obj);

		// Trigger the action that was set for the button itself
		if(obj->action_callback != NULL)
			obj->action_callback(obj);
	}

	// Repaint screen to show the pressed color
	screen_repaint_by_component((component_t*)obj);
}

#endif
