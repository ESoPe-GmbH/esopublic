/**
 * 	@file text.h
 * 	@copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			This module describes a text object and functions to manipulate it for drawing it onto the screen.
 *
 *  @version	1.00 (08.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef TEXT_H_
#define TEXT_H_

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
 * @enum TEXT_H_ALIGNMENT
 * 		Enumerates the possible horizontal alignments for the text object.
 * 		The horizontal alignment depends on the x-coordinate:
 * 		 	Right aligned -> x is the coordinate for the right side of the text.
 * 		 	Left aligned -> x is the coordinate for the left side of the text.
 * 		 	Center aligned -> x is the coordinate at the center of the text.
 */
typedef enum
{
	TEXT_H_ALIGNMENT_RIGHT,		///< The text is right aligned -> x is the coordinate for the right side of the text.
	TEXT_H_ALIGNMENT_LEFT,		///< The text is left aligned -> x is the coordinate for the left side of the text.
	TEXT_H_ALIGNMENT_CENTER		///< The text is center aligned -> x is the coordinate at the center of the text.
}TEXT_H_ALIGNMENT;

/**
 * @enum TEXT_V_ALIGNMENT
 * 		Enumerates the possible vertical alignments for the text object.
 * 		The vertical alignment depends on the x-coordinate:
 * 		 	Top aligned -> y is the coordinate at top of the text.
 * 		 	Center aligned -> y is the coordinate at the center of the text.
 */
typedef enum
{
	/// The text is top aligned -> y is the coordinate at top of the text.
	TEXT_V_ALIGNMENT_TOP,		
	/// The text is center aligned -> y is the coordinate at the center of the text.
	TEXT_V_ALIGNMENT_CENTER,		
	/// The text is bottom aligned -> y is the coordinate at bottom of the text.
	TEXT_V_ALIGNMENT_BOTTOM
}TEXT_V_ALIGNMENT;

/**
 * @struct text_t
 *
 * @brief	Object structure for a text object that should be drawn to the display (like a label object).
 *
 * 			The text variable needs to be a pointer to a 0-terminated string.
 *
 * 			The font is the index for ROM font (16-31) or RAM font (0-14).
 *
 * 			The color object is used for the text color.
 *
 * 			The x/y position of the text object depends on the alignment of the text:
 * 			Horizontal (x):
 * 			- Left aligned (option 0): x is the point at the left side of the text.
 * 			- Right aligned (option EVE_OPT_TEXT_RIGHTX): x is the point at the right side of the text.
 * 			- Center aligned (option EVE_OPT_TEXT_CENTERX): x is the point at the center of the text.
 * 			Vertical (x):
 * 			- Top aligned (option 0): y is the point at the upper side of the text.
 * 			- Center aligned (option EVE_OPT_TEXT_CENTERY): y is the point at the center of the text.
 *
 * 			If the option is EVE_OPT_TEXT_CENTER the x/y points to the vertical and horizontal center of the text.
 */
typedef struct text_s
{
	component_t component;	///< Component object for this text object (Contains coordinates, etc.)

	char* text;					///< Pointer to the text that is drawn by this component.

	uint16_t font;				///< The font is the index for ROM font (16-31) or RAM font (0-14).

	color_t color;			///< Text color

	color_t color_disabled;	///< Text color when text is disabled

	uint8_t line_skip;			///< Space between 2 lines when text is multi-line. Unit is pixel.

	bool bold;					///< Set to true for a pseudo bold (printed 2 times, with an x offset of 1)

	EVE_OPT_TEXT option;		///< Option for the text (See EVE_OPT_TEXT).
								///< Is changed with text_set_horizontal_alignment and text_set_vertical_alignment.
	/// Vertical alignment that was set via options. Default is top alignment.
	TEXT_V_ALIGNMENT v_align;
}text_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes a text object with default values.
 *				The default text color is TEXT_DEFAULT_COLOR.
 *				The default font is TEXT_DEFAULT_FONT.
 *				The default alignment of x/y is Left/Top.
 *
 *			If the pointer to the text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the text object that should be drawn.
 * @param x				x-Coordinate of the text (See description of text_t for details).
 * @param y				y-Coordinate of the text (See description of text_t for details).
 * @param text			Pointer to a string that should be drawn. The string is not copied into an internal buffer!
 * 						The text at the pointer position is drawn at paining time.
 **/
void text_init(text_t* obj, int32_t x, int32_t y, char* text);

/**
 * @brief	Sets the pointer to the string that should be printed.
 *
 *			If the pointer to the text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the text object that should be drawn.
 * @param text			Pointer to a string that should be drawn. The string is not copied into an internal buffer!
 * 						The text at the pointer position is drawn at paining time.
 */
void text_set_text(text_t* obj, char* text);

/**
 * @brief	Sets the font used to draw the text.
 *
 *			If the pointer to the text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the text object that should be drawn.
 * @param font			Index for ROM font (16-31) or RAM font (0-14).
 */
void text_set_font(text_t* obj, uint16_t font);

/**
 * @brief	Sets the text to bold by printing it double with an offset of 1
 *
 * @param obj			Pointer to the text object that should be drawn.
 * @param is_bold		true: Bold, false: Not bold, obviously
 */
void text_set_bold(text_t* obj, bool is_bold);

/**
 * @brief	Sets the color used to draw the text.
 *
 *			If the pointer to the text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the text object that should be drawn.
 * @param c				Color of the text
 */
void text_set_color(text_t* obj, color_t c);

/**
 * @brief	Sets the horizontal alignment of the text.
 * 			The horizontal alignment depends on the x-coordinate:
 * 		 		Right aligned -> x is the coordinate for the right side of the text.
 * 		 		Left aligned -> x is the coordinate for the left side of the text.
 * 		 		Center aligned -> x is the coordinate at the center of the text.
 *
 *			If the pointer to the text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the text object that should be drawn.
 * @param align			Horizontal alignment value. See TEXT_H_ALIGNMENT for details.
 */
void text_set_horizontal_alignment(text_t* obj, TEXT_H_ALIGNMENT align);

/**
 * @brief	Sets the vertical alignment of the text.
 * 			The vertical alignment depends on the x-coordinate:
 * 		 		Top aligned -> y is the coordinate at the top of the text.
 * 		 		Center aligned -> y is the coordinate at the center of the text.
 *
 *			If the pointer to the text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the text object that should be drawn.
 * @param align			Vertical alignment value. See TEXT_V_ALIGNMENT for details.
 */
void text_set_vertical_alignment(text_t* obj, TEXT_V_ALIGNMENT align);

/**
 * @brief	Sets the line skip for the text component that is used to seperate between
 * 			2 lines if the text is a multi-line text.
 *
 *			If the pointer to the text object is NULL, nothing happens.
 *
 * @param obj			Pointer to the text object that should be drawn.
 * @param line_skip		Pixel space between 2 lines when text is multi-line.
 */
void text_set_line_skip(text_t* obj, uint8_t line_skip);

/**
 * @brief	Changes the visibility of the text.
 *
 *			If the pointer to the text object is NULL, nothing happens.
 *
 * @param obj					Pointer to the text object.
 * @param b						true: Text is visible, false: Text is not visible.
 */
void text_set_visible(text_t* obj, bool b);

#endif

#endif /* TEXT_H_ */
