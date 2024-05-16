/***
 * @file text.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "text.h"
#include "../eve/eve_register.h"
#include "../eve/eve_copro.h"
#include "font.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief	This function is used to draw the text object on the display.
 * 			If obj is NULL, this function does nothing.
 * 			This function is called automatically during a screen repaint inside the screen_handle.
 *
 * @param obj			Pointer to the text object that should be drawn.
 * @param p				Coordinate of the parent component.
 */
static void text_paint(text_t* obj, eve_ui_point_t p);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void text_init(text_t* obj, int32_t x, int32_t y, char* text)
{
	if(obj == NULL)
		return;

	component_init((component_t*)obj, COMPONENT_TYPE_TEXT, (component_paint_cb_t)text_paint);

	obj->component.origin.x = x;
	obj->component.origin.y = y;
	obj->text = text;
	obj->font = TEXT_DEFAULT_FONT;
	obj->line_skip = TEXT_DEFAULT_LINE_SKIP;
	obj->color = color_get(TEXT_DEFAULT_COLOR);
	obj->color_disabled = color_get(TEXT_DEFAULT_COLOR_DISABLED);
	obj->option = EVE_OPT_TEXT_DEFAULT;
	obj->bold = false;
}

void text_set_text(text_t* obj, char* text)
{
	if(obj == NULL)
		return;

	obj->text = text;
}

void text_set_font(text_t* obj, uint16_t font)
{
	if(obj == NULL)
		return;

	obj->font = font;
}

void text_set_bold(text_t* obj, bool is_bold)
{
	if(obj == NULL)
		return;

	obj->bold = is_bold;
}

void text_set_color(text_t* obj, color_t c)
{
	if(obj == NULL)
		return;

	obj->color = c;
}

void text_set_horizontal_alignment(text_t* obj, TEXT_H_ALIGNMENT align)
{
	if(obj == NULL)
		return;

	obj->option &= ~(EVE_OPT_TEXT_CENTERX | EVE_OPT_TEXT_RIGHTX);	// Remove horizontal center and right alignment
	switch(align)
	{
		case TEXT_H_ALIGNMENT_LEFT:
			// Nothing to do for left
		break;

		case TEXT_H_ALIGNMENT_CENTER:
			obj->option |= EVE_OPT_TEXT_CENTERX;		// Set horizontal center alignment
		break;

		case TEXT_H_ALIGNMENT_RIGHT:
			obj->option |= EVE_OPT_TEXT_RIGHTX;		// Set horizontal right alignment
		break;
	}
}

void text_set_vertical_alignment(text_t* obj, TEXT_V_ALIGNMENT align)
{
	if(obj == NULL)
		return;

	obj->v_align = align;

	switch(align)
	{
		case TEXT_V_ALIGNMENT_TOP:
			obj->option &= ~EVE_OPT_TEXT_CENTERY;		// Remove vertical center alignment -> top is default
		break;

		case TEXT_V_ALIGNMENT_CENTER:
			obj->option |= EVE_OPT_TEXT_CENTERY;		// Set vertical center alignment
		break;

		case TEXT_V_ALIGNMENT_BOTTOM:
			obj->option &= ~EVE_OPT_TEXT_CENTERY;		// Remove vertical center alignment -> top is default
		break;
	}
}

void text_set_line_skip(text_t* obj, uint8_t line_skip)
{
	if(obj == NULL)
		return;

	obj->line_skip = line_skip;
}

void text_set_visible(text_t* obj, bool b)
{
	if(obj == NULL)
		return;

	obj->component.is_visible = b;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
static void text_paint(text_t* obj, eve_ui_point_t p)
{
	eve_t* eve = component_get_eve((component_t*)obj);

	if(eve == NULL || obj->text == NULL || obj->text[0] == 0)
		return;

	p.y += obj->component.origin.y;
	p.x += obj->component.origin.x;

	if(obj->v_align == TEXT_V_ALIGNMENT_BOTTOM)
	{
		p.y -= font_get_height(eve, obj->font);
	}

	eve_copro_set_color(eve, obj->component.is_enabled ? obj->color : obj->color_disabled);
	eve_copro_text(eve, p.x, p.y, obj->font, obj->option, obj->text);
	if(obj->bold)
		eve_copro_text(eve, p.x + 1, p.y, obj->font, obj->option, obj->text);
}

#endif
