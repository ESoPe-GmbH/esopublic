/***
 * @file edit_text.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "edit_text.h"
#include "font.h"
#include "screen.h"
#include "../eve/eve_register.h"
#include "../eve/eve_copro.h"
#include "module/convert/string.h"
#include "module/comm/dbg.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
static char _str_display_text[100];

static bool _is_cursor;				///< Cursor is currently shown

static system_task_t _task; ///< Task for toggling

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief	This function is used to draw the edit_text object on the display.
 * 			If obj is NULL, this function does nothing.
 * 			This function is called automatically during a screen repaint inside the screen_handle.
 *
 * @param obj			Pointer to the edit_text object that should be drawn.
 * @param p				Coordinates of the parent component.
 */
static void edit_text_paint(edit_text_t* obj, eve_ui_point_t p);
/**
 * @brief	Callback function for keyboard presses. Is used to add characters or delete
 *
 * @param obj
 * @param scancode
 * @param c
 */
static void _keyboard_callback(edit_text_t* obj, uint8_t scancode, char c);
/**
 * Sets the edit text as pressed or released
 * @param obj	Pointer to the edit text object
 * @param b		true: edit text is pressed, false: edit text was released
 */
static void _set_pressed(edit_text_t* obj, bool b);
/**
 * Toggles the cursor.
 * @param pt	Pointer to protothread
 * @return		Protothread return value.
 */
static int _handle(struct pt* pt);
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void edit_text_init(edit_text_t* obj, int32_t x, int32_t y, uint16_t w, uint16_t h, char* text_buffer, uint16_t text_buffer_size)
{
	if(obj == NULL)
		return;

	component_init((component_t*)obj, COMPONENT_TYPE_EDIT_TEXT, (component_paint_cb_t)edit_text_paint);

	obj->component.origin.x = x;
	obj->component.origin.y = y;
	obj->component.size.width = w;
	obj->component.size.height = h;
	obj->component.is_focusable = true;
	obj->component.keyboard_callback = (component_keyboard_cb_t)_keyboard_callback;
	obj->component.pressed_callback = (component_pressed_cb_t)_set_pressed;
	obj->font = EDIT_TEXT_DEFAULT_FONT;
#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	obj->color_text_focused = color_get(EDIT_TEXT_DEFAULT_COLOR_FOCUSED);
	obj->color_bg_focused = color_get(EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_FOCUSED);
#endif
	obj->color_text_unfocused = color_get(EDIT_TEXT_DEFAULT_COLOR_UNFOCUSED);
	obj->color_text_disabled = color_get(EDIT_TEXT_DEFAULT_COLOR_DISABLED);
	obj->color_bg_unfocused = color_get(EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_UNFOCUSED);
	obj->color_bg_disabled = color_get(EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_DISABLED);
	obj->color_border = color_get(EDIT_TEXT_DEFAULT_COLOR_BORDER);
	obj->draw_back_unfocused = true;
	obj->option = EVE_OPT_TEXT_CENTERY;
	obj->bold = false;
	obj->show_cursor = true;
	obj->text_margin = 4;
	obj->text_buffer = text_buffer;
	obj->text_buffer_size = text_buffer_size;
	obj->hide_text = NULL;
	obj->color_hide_text = color_get(EDIT_TEXT_DEFAULT_COLOR_HIDE_TEXT);
	obj->is_password = false;
	obj->min_length = 0;

	if(_task.f_pt != _handle)
		system_init_protothread_task(&_task, true, _handle);
}

void edit_text_set_edit_text(edit_text_t* obj, char* edit_text)
{
	if(obj == NULL)
		return;

	if(edit_text == NULL)
		obj->text_buffer[0] = 0;
	else
	{
		memset(obj->text_buffer, 0, obj->text_buffer_size);
		strncpy(obj->text_buffer, edit_text, obj->text_buffer_size - 1);
	}
	screen_repaint_by_component((component_t*)obj);
}


void edit_text_set_hidden_text(edit_text_t* obj, char* text)
{
	if(obj == NULL)
		return;

	obj->hide_text = text;
}

void edit_text_set_color_hidden_text(edit_text_t* obj, color_t c)
{
	if(obj == NULL)
		return;

	obj->color_hide_text = c;
}

void edit_text_set_font(edit_text_t* obj, uint16_t font)
{
	if(obj == NULL)
		return;

	obj->font = font;
}

void edit_text_set_bold(edit_text_t* obj, bool is_bold)
{
	if(obj == NULL)
		return;

	obj->bold = is_bold;
}

void edit_text_set_color_focused(edit_text_t* obj, color_t c)
{
	if(obj == NULL)
		return;

#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	obj->color_text_focused = c;
#endif
}

void edit_text_set_color_unfocused(edit_text_t* obj, color_t c)
{
	if(obj == NULL)
		return;

	obj->color_text_unfocused = c;
}

void edit_text_set_background_color_focused(edit_text_t* obj, color_t c)
{
	if(obj == NULL)
		return;

#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	obj->color_bg_focused = c;
#endif
}

void edit_text_set_background_color_unfocused(edit_text_t* obj, color_t c)
{
	if(obj == NULL)
		return;

	obj->color_bg_unfocused = c;
}

void edit_text_set_background_color_border(edit_text_t* obj, color_t c)
{
	if(obj == NULL)
		return;

	obj->color_border = c;
}

void edit_text_set_horizontal_alignment(edit_text_t* obj, EDIT_TEXT_H_ALIGNMENT align)
{
	if(obj == NULL)
		return;

	switch(align)
	{
		case EDIT_TEXT_H_ALIGNMENT_LEFT:
			obj->option &= ~(EVE_OPT_TEXT_CENTERX | EVE_OPT_TEXT_RIGHTX);	// Remove horizontal center and right alignment
		break;

		case EDIT_TEXT_H_ALIGNMENT_CENTER:
			obj->option |= EVE_OPT_TEXT_CENTERX;		// Set horizontal center alignment
		break;

		case EDIT_TEXT_H_ALIGNMENT_RIGHT:
			obj->option |= EVE_OPT_TEXT_RIGHTX;		// Set horizontal right alignment
		break;
	}
}

void edit_text_set_vertical_alignment(edit_text_t* obj, EDIT_TEXT_V_ALIGNMENT align)
{
	if(obj == NULL)
		return;

	switch(align)
	{
		case EDIT_TEXT_V_ALIGNMENT_TOP:
			obj->option &= ~EVE_OPT_TEXT_CENTERY;		// Remove vertical center alignment -> top is default
		break;

		case EDIT_TEXT_V_ALIGNMENT_CENTER:
			obj->option |= EVE_OPT_TEXT_CENTERY;		// Set vertical center alignment
		break;
	}
}

void edit_text_set_line_skip(edit_text_t* obj, uint8_t line_skip)
{
	if(obj == NULL)
		return;

	obj->line_skip = line_skip;
}

void edit_text_set_visible(edit_text_t* obj, bool b)
{
	if(obj == NULL)
		return;

	obj->component.is_visible = b;
}

void edit_text_is_password(edit_text_t* obj, bool b)
{
	if(obj == NULL)
		return;

	obj->is_password = b;
}

void edit_text_set_pressed_action(edit_text_t* obj, void(*pressed_callback)(edit_text_t*))
{
	if(obj == NULL)
		return;

	obj->action_callback = (void(*)(void*))pressed_callback;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
static void edit_text_paint(edit_text_t* obj, eve_ui_point_t p)
{
	int32_t x1, y1;
	int32_t x0, y0;
	eve_t* eve = component_get_eve((component_t*)obj);
#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	bool is_focused = component_is_focused((component_t*)obj);
#endif
	bool is_enabled = obj->component.is_enabled;

	bool can_show_cursor = true;

	char* str;

	if(eve == NULL || obj == NULL)
		return;

	p.y += obj->component.origin.y;
	p.x += obj->component.origin.x;

	x1 = (p.x + obj->component.size.width);
	y1 = (p.y + obj->component.size.height);

	eve_copro_add_tag(eve, (component_t*)obj);

	_str_display_text[0] = 0;

	if(obj->text_buffer != NULL && obj->text_buffer[0] != 0)
	{
//		memset(_str_display_text, 0, sizeof(_str_display_text));
		if(obj->is_password)
		{
			memset(_str_display_text, '*', strlen(obj->text_buffer));
			_str_display_text[strlen(obj->text_buffer)] = 0;
		}
		else
			strncpy(_str_display_text, obj->text_buffer, obj->text_buffer_size);

		str = _str_display_text;
	}
	else if(obj->hide_text != NULL && obj->hide_text[0] != 0)
		str = obj->hide_text;
	else
		str = _str_display_text;

	if(str == obj->hide_text || !obj->component.is_enabled)
		can_show_cursor = false; // Suppress cursor while hidden text is shown or component is disabled

	if(obj->show_cursor
#if SCREEN_ENABLE_FOCUSED_COMPONENTS
			&& is_focused
#endif
			)
	{
		if(_is_cursor && can_show_cursor)
		{
			// Add the cursor only if the text buffer is not full!
			if(strlen(obj->text_buffer) < obj->text_buffer_size - 1)
//				obj->is_cursor = false;
//			else
				strcat(_str_display_text, "_");
		}
	}
//	else
//	{
//		if(obj->is_cursor)
//		{
//			obj->is_cursor = false;
//			//obj->text_buffer[strlen(obj->text_buffer) - 1] = 0;
//		}
//	}
#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	if(obj->draw_back_unfocused || is_focused)
	{
		eve_copro_set_color(eve, is_focused ? obj->color_bg_focused : (is_enabled ? obj->color_bg_unfocused : obj->color_bg_disabled));
		eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
		// Set line width
		eve_copro_write_command(eve, EVE_LINE_WIDTH(16 * 1));
		// Draw rectangle
		eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));			// Start rectangle
		eve_copro_write_command(eve, EVE_VERTEX2F(x, y));				// Start point of rectangle
		eve_copro_write_command(eve, EVE_VERTEX2F(x1, y1));			// End point of rectangle
		eve_copro_write_command(eve, EVE_END());						// End rectangle
	}
#else
	if(obj->draw_back_unfocused)
	{
		eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
		eve_copro_set_color(eve, obj->color_border);
		// Set line width
		eve_copro_write_command(eve, EVE_LINE_WIDTH(16 * 2));
		// Draw rectangle
		eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));			// Start rectangle
		eve_copro_write_command(eve, EVE_VERTEX2F(p.x, p.y));				// Start point of rectangle
		eve_copro_write_command(eve, EVE_VERTEX2F(x1, y1));			// End point of rectangle
		eve_copro_write_command(eve, EVE_END());						// End rectangle

		eve_copro_set_color(eve, is_enabled ? obj->color_bg_unfocused : obj->color_bg_disabled);
		// Set line width
		eve_copro_write_command(eve, EVE_LINE_WIDTH(16 * 1));
		// Draw rectangle
		eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));			// Start rectangle
		eve_copro_write_command(eve, EVE_VERTEX2F(p.x, p.y));				// Start point of rectangle
		eve_copro_write_command(eve, EVE_VERTEX2F(x1, y1));			// End point of rectangle
		eve_copro_write_command(eve, EVE_END());						// End rectangle
	}
#endif
	if(str)
	{
		// Set Scissor
		eve_copro_write_command(eve, EVE_SCISSOR_XY(p.x, p.y));
		eve_copro_write_command(eve, EVE_SCISSOR_SIZE(obj->component.size.width, obj->component.size.height));

#if SCREEN_ENABLE_FOCUSED_COMPONENTS
		eve_copro_set_color(eve, str == obj->hide_text ? obj->color_hide_text : is_focused ? obj->color_text_focused : (is_enabled ? obj->color_text_unfocused : obj->color_text_disabled));
#else
		eve_copro_set_color(eve, str == obj->hide_text ? obj->color_hide_text : (is_enabled ? obj->color_text_unfocused : obj->color_text_disabled));
#endif
		x0 = p.x + (obj->component.size.width / 2);
		y0 = p.y + (obj->component.size.height / 2);

		// Left aligned
		if((obj->option & (EVE_OPT_TEXT_CENTERX | EVE_OPT_TEXT_RIGHTX)) == 0)
		{
			x0 = p.x + obj->text_margin;
		}
		else if((obj->option & EVE_OPT_TEXT_RIGHTX) == EVE_OPT_TEXT_RIGHTX)
		{
			x0 = p.x + obj->component.size.width - obj->text_margin;
		}
		else if((obj->option & EVE_OPT_TEXT_CENTERX) == EVE_OPT_TEXT_CENTERX && _is_cursor && can_show_cursor)
		{
			x0 += font_get_width(eve, obj->font, '_') / 2;
		}
		if((obj->option & EVE_OPT_TEXT_CENTERY) == 0)
		{
			y0 = p.y + obj->text_margin;
		}

		eve_copro_text(	eve,	x0,	y0,	obj->font, obj->option, str);
		if(obj->bold)
			eve_copro_text(	eve,	x0+1,	y0,	obj->font, obj->option, str);

		// Reset Scissor
		eve_copro_write_command(eve, EVE_SCISSOR_XY(0, 0));
		eve_copro_write_command(eve, EVE_SCISSOR_SIZE(eve->eve_display_width, eve->eve_display_height));
	}
}

static void _keyboard_callback(edit_text_t* obj, uint8_t scancode, char c)
{
	uint16_t len = strlen(obj->text_buffer);

	if(!obj->component.is_enabled)
		return;

//	if(obj->is_cursor)
//	{
//		len--;
//		obj->text_buffer[len] = 0;
//		obj->is_cursor = false;
//		obj->timestamp_cursor = system_get_tick_count();
//	}

	if(scancode == 0x2A && len > obj->min_length) // Backspace
	{
		len--;
	}
	else if(c >= ' ' && c < 0x7F && len < obj->text_buffer_size - 2)
		obj->text_buffer[len++] = c;
	else if(c == 0xDF) // ß
		obj->text_buffer[len++] = 0x19;
	else if(c == 0xC4) // Ä
		obj->text_buffer[len++] = 0x1A;
	else if(c == 0xD6) // Ö
		obj->text_buffer[len++] = 0x1B;
	else if(c == 0xDC) // Ü
		obj->text_buffer[len++] = 0x1C;
	else if(c == 0xE4) // ä
		obj->text_buffer[len++] = 0x1D;
	else if(c == 0xF6) // ö
		obj->text_buffer[len++] = 0x1E;
	else if(c == 0xFC) // ü
		obj->text_buffer[len++] = 0x1F;
	else
		return;

	obj->text_buffer[len] = 0;
	screen_repaint_by_component((component_t*)obj);
}

static void _set_pressed(edit_text_t* obj, bool b)
{
	if(obj == NULL)
		return;

	if(!obj->component.is_enabled) // no action when not enabled
		return;

	if(!b)
	{
		// Trigger the action that was set for the button itself
		if(obj->action_callback != NULL)
			obj->action_callback(obj);
	}
}

static int _handle(struct pt* pt)
{
	PT_BEGIN(pt);

	do
	{
		PT_YIELD_MS(pt, 500);
		_is_cursor ^= 1;
	}while(true);

	PT_END(pt);
}

#endif
