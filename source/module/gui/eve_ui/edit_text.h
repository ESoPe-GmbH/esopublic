/**
 * 	@file edit_text.h
 *  @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			This module describes a edit_text object and functions to manipulate it for drawing it onto the screen.
 *
 *	@version	1.02 (07.12.2018)
 *		- Cursor does not blink when component is disabled.
 *	@version 	1.01 (07.12.2018)
 *		- The cursor toggling is now independent from each component, so that all cursors on the current screen are shown in sync.
 *  @version	1.00 (08.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef EDIT_TEXT_H_
#define EDIT_TEXT_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "color.h"
#include "component.h"
#include "../eve/eve_copro.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Config
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @struct edit_text_t
 *
 * @brief	Object structure for a edit_text object that should be drawn to the display (like a label object).
 *
 * 			The edit_text variable needs to be a pointer to a 0-terminated string.
 *
 * 			The font is the index for ROM font (16-31) or RAM font (0-14).
 *
 * 			The color object is used for the edit_text color.
 *
 * 			The x/y position of the edit_text object depends on the alignment of the edit_text:
 * 			Horizontal (x):
 * 			- Left aligned (option 0): x is the point at the left side of the edit_text.
 * 			- Right aligned (option EVE_OPT_EDIT_TEXT_RIGHTX): x is the point at the right side of the edit_text.
 * 			- Center aligned (option EVE_OPT_EDIT_TEXT_CENTERX): x is the point at the center of the edit_text.
 * 			Vertical (x):
 * 			- Top aligned (option 0): y is the point at the upper side of the edit_text.
 * 			- Center aligned (option EVE_OPT_EDIT_TEXT_CENTERY): y is the point at the center of the edit_text.
 *
 * 			If the option is EVE_OPT_EDIT_TEXT_CENTER the x/y points to the vertical and horizontal center of the edit_text.
 */
typedef struct
{
	component_t component;	///< Component object for this edit_text object (Contains coordinates, etc.)

	char* hide_text;			/// Pointer to a text that is shown when text_buffer is empty

	char* text_buffer;			/// Buffer for the text

	uint16_t text_buffer_size;	/// Size of the text buffer

	bool show_cursor;			///< Enables use of a cursor that blinks while focused

	uint16_t font;				///< The font is the index for ROM font (16-31) or RAM font (0-14).

	color_t color_text_unfocused;		///< Text color when unfocused

#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	color_t color_text_focused;		///< Text color when focused
#endif

	color_t color_text_disabled;		///< Text color when disabled

	color_t color_bg_unfocused;		///< Background color when unfocused

#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	color_t color_bg_focused;			///< Background color when unfocused
#endif
	color_t color_bg_disabled;			///< Background color when disabled

	color_t color_hide_text;			///< Color of the hidden text

	color_t color_border;				///< Color of the border

	uint8_t line_skip;					///< Space between 2 lines when edit_text is multi-line. Unit is pixel.

	bool bold;							///< Set to true for a pseudo bold (printed 2 times, with an x offset of 1)

	bool draw_back_unfocused;			///< Draw background when unfocused

	EVE_OPT_TEXT option;				///< Option for the edit_text (See EVE_OPT_EDIT_TEXT).
										///< Is changed with edit_text_set_horizontal_alignment and edit_text_set_vertical_alignment.

	uint16_t text_margin;				///< Margin of the text inside the box

	bool is_password;

	uint8_t	min_length;

	void(*action_callback)(void*);		///< Pointer to the function that is called when the edit_text is pressed.
										///< The parameter is a pointer to the edit_text_t that was pressed.
										///< void* is a placeholder because this edit_text_t structure cannot use
										///< itself inside.

}edit_text_t;

/**
 * @enum EDIT_TEXT_H_ALIGNMENT
 * 		Enumerates the possible horizontal alignments for the edit_text object.
 * 		The horizontal alignment depends on the x-coordinate:
 * 		 	Right aligned -> x is the coordinate for the right side of the edit_text.
 * 		 	Left aligned -> x is the coordinate for the left side of the edit_text.
 * 		 	Center aligned -> x is the coordinate at the center of the edit_text.
 */
typedef enum
{
	EDIT_TEXT_H_ALIGNMENT_RIGHT,		///< The edit_text is right aligned -> x is the coordinate for the right side of the edit_text.
	EDIT_TEXT_H_ALIGNMENT_LEFT,		///< The edit_text is left aligned -> x is the coordinate for the left side of the edit_text.
	EDIT_TEXT_H_ALIGNMENT_CENTER		///< The edit_text is center aligned -> x is the coordinate at the center of the edit_text.
}EDIT_TEXT_H_ALIGNMENT;

/**
 * @enum EDIT_TEXT_V_ALIGNMENT
 * 		Enumerates the possible vertical alignments for the edit_text object.
 * 		The vertical alignment depends on the x-coordinate:
 * 		 	Top aligned -> y is the coordinate at top of the edit_text.
 * 		 	Center aligned -> y is the coordinate at the center of the edit_text.
 */
typedef enum
{
	EDIT_TEXT_V_ALIGNMENT_TOP,		///< The edit_text is top aligned -> y is the coordinate at top of the edit_text.
	EDIT_TEXT_V_ALIGNMENT_CENTER		///< The edit_text is center aligned -> y is the coordinate at the center of the edit_text.
}EDIT_TEXT_V_ALIGNMENT;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes a edit_text object with default values.
 *				The default alignment of x/y is Left/Top.
 *
 *			If the pointer to the edit_text object is NULL, nothing happens.
 *
 *
 * @param obj			Pointer to the edit_text object that should be drawn.
 * @param x				x-Coordinate of the edit_text (See description of edit_text_t for details).
 * @param y				y-Coordinate of the edit_text (See description of edit_text_t for details).
 * @param w				Width of the box
 * @param h				Height
 * @param text_buffer	Pointer to the buffer where the entered text is stored in
 * @param text_buffer_size	Size of the buffer, including the terminating 0
 **/
void edit_text_init(edit_text_t* obj, int32_t x, int32_t y, uint16_t w, uint16_t h, char* text_buffer, uint16_t text_buffer_size);

/**
 * @brief	Sets the pointer to the string that should be printed.
 *
 *			If the pointer to the edit_text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the edit_text object that should be drawn.
 * @param edit_text			Pointer to a string that should be drawn. The string is not copied into an internal buffer!
 * 						The edit_text at the pointer position is drawn at paining time.
 */
void edit_text_set_edit_text(edit_text_t* obj, char* edit_text);

void edit_text_set_hidden_text(edit_text_t* obj, char* text);

void edit_text_set_color_hidden_text(edit_text_t* obj, color_t c);

/**
 * @brief	Sets the font used to draw the edit_text.
 *
 *			If the pointer to the edit_text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the edit_text object that should be drawn.
 * @param font			Index for ROM font (16-31) or RAM font (0-14).
 */
void edit_text_set_font(edit_text_t* obj, uint16_t font);

/**
 * @brief	Sets the edit_text to bold by printing it double with an offset of 1
 *
 * @param obj			Pointer to the edit_text object that should be drawn.
 * @param is_bold		true: Bold, false: Not bold, obviously
 */
void edit_text_set_bold(edit_text_t* obj, bool is_bold);

/**
 * @brief	Sets the color used to draw the edit_text when edit text is focused.
 *
 *			If the pointer to the edit_text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the edit_text object that should be drawn.
 * @param c				Color of the edit_text
 */
void edit_text_set_color_focused(edit_text_t* obj, color_t c);

/**
 * @brief	Sets the color used to draw the edit_text when edit text is not focused.
 *
 *			If the pointer to the edit_text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the edit_text object that should be drawn.
 * @param c				Color of the edit_text
 */
void edit_text_set_color_unfocused(edit_text_t* obj, color_t c);

/**
 * @brief	Sets the horizontal alignment of the edit_text.
 * 			The horizontal alignment depends on the x-coordinate:
 * 		 		Right aligned -> x is the coordinate for the right side of the edit_text.
 * 		 		Left aligned -> x is the coordinate for the left side of the edit_text.
 * 		 		Center aligned -> x is the coordinate at the center of the edit_text.
 *
 *			If the pointer to the edit_text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the edit_text object that should be drawn.
 * @param align			Horizontal alignment value. See EDIT_TEXT_H_ALIGNMENT for details.
 */
void edit_text_set_horizontal_alignment(edit_text_t* obj, EDIT_TEXT_H_ALIGNMENT align);

/**
 * @brief	Sets the vertical alignment of the edit_text.
 * 			The vertical alignment depends on the x-coordinate:
 * 		 		Top aligned -> y is the coordinate at the top of the edit_text.
 * 		 		Center aligned -> y is the coordinate at the center of the edit_text.
 *
 *			If the pointer to the edit_text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the edit_text object that should be drawn.
 * @param align			Vertical alignment value. See EDIT_TEXT_V_ALIGNMENT for details.
 */
void edit_text_set_vertical_alignment(edit_text_t* obj, EDIT_TEXT_V_ALIGNMENT align);

/**
 * @brief	Sets the line skip for the edit_text component that is used to seperate between
 * 			2 lines if the edit_text is a multi-line edit_text.
 *
 *			If the pointer to the edit_text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the edit_text object that should be drawn.
 * @param line_skip		Pixel space between 2 lines when edit_text is multi-line.
 */
void edit_text_set_line_skip(edit_text_t* obj, uint8_t line_skip);

/**
 * @brief	Changes the visibility of the edit_text.
 *
 *			If the pointer to the edit_text object is NULL, nothing happens.
 *
 * @param obj					Pointer to the edit_text object.
 * @param b						true: Text is visible, false: Text is not visible.
 */
void edit_text_set_visible(edit_text_t* obj, bool b);
/**
 * Sets wether the edit text is used to enter a password
 * @param obj	Pointer to the edit_text object that should be drawn.
 * @param b		true: Asterisks are shown in place of the user input, false: the input is displayed verbatim
 */
void edit_text_is_password(edit_text_t* obj, bool b);
/**
 * Sets the color of the border of the edit text object
 * @param obj	Pointer to the edit_text object that should be drawn.
 * @param c		The color of the border of the edit text
 */
void edit_text_set_background_color_border(edit_text_t* obj, color_t c);

/**
 * Sets a callback function that is called when the edit text is pressed by touch.
 * @param obj				Pointer to the edit_text object that should be drawn.
 * @param pressed_callback	Pointer to the callback function that has a pointer to the pressed edit text as a parameter.
 */
void edit_text_set_pressed_action(edit_text_t* obj, void(*pressed_callback)(edit_text_t*));

#endif

#endif /* EDIT_TEXT_H_ */
