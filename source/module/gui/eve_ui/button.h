/**
 * 	@file button.h
 *  @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			This is used for creating a button on the screen.
 *			Once the button is initialized and added to the screen it is shown and calls a callback function when the button
 *			is pressed.
 *
 *  @version	1.00 (08.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef BUTTON_H_
#define BUTTON_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "color.h"
#include "component.h"
#include "text.h"
#include "image.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @enum BUTTON_TYPE
 *
 * Can be used to draw different types of buttons.
 */
typedef enum
{
	BUTTON_TYPE_API = 0,			///< Draws a button that is created by eve api.

	BUTTON_TYPE_RECT,				///< Draws a button with a flat rectangle background.

	BUTTON_TYPE_ROUND,				///< Draws a button with round edges. If width and height are equal, the button is completely round.

	BUTTON_TYPE_API_LIKE			///< Draws a button that looks like an API button just without gradient.
	 	 	 	 	 	 	 	 	///< Should be used to save resources if no gradient is needed, because API needs LOTS of resources in the display list.
}BUTTON_TYPE;

/**
 * @enum BUTTON_FIGURE_POS
 * Position of the figure on the button.
 */
typedef enum
{
	/// Position of the figure is manually taken from the figure's x and y components
	BUTTON_FIGURE_POS_CUSTOM = 0,
	/// Figure is centered horizontally
	BUTTON_FIGURE_POS_CENTER_X = 0x0001,
	/// Figure is centered vertically
	BUTTON_FIGURE_POS_CENTER_Y = 0x0002,
	/// Figure is centered horizontally and vertically.
	BUTTON_FIGURE_POS_CENTER = 0x0003,
	/// Figure is positioned right
	BUTTON_FIGURE_POS_RIGHT = 0x0004,
	/// Figure is positioned left
	BUTTON_FIGURE_POS_LEFT = 0x0008,
	/// Figure is positioned to the top
	BUTTON_FIGURE_POS_TOP = 0x0010,
	/// Figure is positioned to the bottom
	BUTTON_FIGURE_POS_BOTTOM = 0x0020
}BUTTON_FIGURE_POS;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @struct button_t
 *
 * This structure is used for painting a button on the screen and handling the pressing of the button via touch.
 *
 * The structure contains colors for the pressed state (variable pressed is true) and the not pressed state (variable
 * pressed is false). These colors are automatically used when someone presses or releases the button.
 */
typedef struct
{
	component_t component;				///< Component Object used for painting this object on the screen

	char* text;								///< Pointer to the text that should be printed on the button.

	EVE_OPT_TEXT option_text;					///< Option for the text (See EVE_OPT_TEXT).
											///< Is changed with text_set_horizontal_alignment and text_set_vertical_alignment.

	bool is_bold;							///< Indicates whether the text is printed bold or not
	
	bool enable_text_shadow;	///< Indicates whether the text has shadow or not. Shadow is always drawn on api buttons.

	uint16_t font;							///< font index used for the text on the button.

	color_t color_background;				///< Background color of the button when it is not pressed.

	color_t color_text;					///< Text color of the button when it is not pressed.

	color_t color_background_pressed;		///< Background color of the button when it is pressed.

	color_t color_text_pressed;			///< Text color of the button when it is pressed.

	color_t color_background_disabled;	///< Background color of the button when it is disabled.

	color_t color_text_disabled;			///< Text color of the button when it is pressed.

	uint32_t option;						///< Option that is written to the eve. Should not be modified directly.

	bool pressed;							///< true: Button is pressed, false: Button is not pressed.

	void(*action_callback)(void*);			///< Pointer to the function that is called when the button is pressed.
											///< The parameter is a pointer to the button_t that was pressed.
											///< void* is a placeholder because this button_t structure cannot use
											///< itself inside.

	BUTTON_TYPE type;						///< Type of the button. Default is BUTTON_TYPE_DEFAULT.

	component_t* figure;					///< Pointer to the figure shown on the button.

	BUTTON_FIGURE_POS figure_pos;			///< Position of the figure on the button.

	uint16_t figure_margin;					///< Margin of the figure from the buttons border

}button_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes the button object and writes default values into it for the colors and fonts.
 *			To set a callback function for the button use button_set_action afterwards.
 *			The button is set visible and not selected by default.
 *
 *			If the pointer to the button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param x						x Position on the screen
 * @param y						y Position on the screen
 * @param width					width of the button in pixel
 * @param height				height of the button in pixel.
 * @param text					Pointer to the text that should be printed on the button.
 *
 * @attention	(x + width) should be less or equal screen_get_width() if it should be completely visible.
 * @attention	(y + height) should be less or equal screen_get_height() if it should be completely visible.
 **/
void button_init(button_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, char* text);

/**
 * @brief	Changes the button type.
 *
 * @param obj					Pointer to the button object.
 * @param type					Type of the button that should be drawn. See BUTTON_TYPE for details.
 */
void button_set_type(button_t* obj, BUTTON_TYPE type);

/**
 * @brief	Changes the pointer to the text that is printed on the button.
 *
 *			If the pointer to the button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param text					Pointer to the text that should be printed on the button.
 */
void button_set_text(button_t* obj, char* text);

/**
 * @brief	Sets the horizontal alignment of the text.
 * 			The horizontal alignment depends on the x-coordinate:
 * 		 		Right aligned -> x is the coordinate for the right side of the text.
 * 		 		Left aligned -> x is the coordinate for the left side of the text.
 * 		 		Center aligned -> x is the coordinate at the center of the text.
 *
 *			If the pointer to the text object is NULL, nothing happens.
 *
 * @attention Not working with BUTTON_TYPE_API
 *
 * @param obj			Pointer to the text object that should be drawn.
 * @param align			Horizontal alignment value. See TEXT_H_ALIGNMENT for details.
 */
void button_set_text_horizontal_alignment(button_t* obj, TEXT_H_ALIGNMENT align);

/**
 * @brief	Sets the vertical alignment of the text.
 * 			The vertical alignment depends on the x-coordinate:
 * 		 		Top aligned -> y is the coordinate at the top of the text.
 * 		 		Center aligned -> y is the coordinate at the center of the text.
 *
 *			If the pointer to the text object is NULL, nothing happens.
 *
 * @attention Not working with BUTTON_TYPE_API
 *
 * @param obj			Pointer to the text object that should be drawn.
 * @param align			Vertical alignment value. See TEXT_V_ALIGNMENT for details.
 */
void button_set_text_vertical_alignment(button_t* obj, TEXT_V_ALIGNMENT align);

/**
 * @brief	Sets the text to bold by printing it double with an offset of 1
 *
 * @param obj			Pointer to the button object that should be drawn.
 * @param is_bold		true: Bold, false: Not bold, obviously
 */
void button_set_text_bold(button_t* obj, bool is_bold);

/**
 * @brief	Changes the visibility of the button.
 *			If the visibility value is different from the previously set visibility value, a screen repaint is triggered.
 *
 *			If the pointer to the button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param b						true: Button is visible, false: Button is not visible.
 */
void button_set_visible(button_t* obj, bool b);

/**
 * @brief	Sets the button enabled or disabled. If the button is disabled, no further action is possible.
 *			If the enabled value is different from the previously set enabled value, a screen repaint is triggered.
 *
 *			If the pointer to the button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param b						true: Button is enabled, false: Button is not enabled.
 */
void button_set_enabled(button_t* obj, bool b);

/**
 * @brief	Sets a callback function that is called when the button is pressed.
 * 			The callback function has the button_object of the pressed button as a parameter.
 *
 *			If the pointer to the button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param pressed_callback		Pointer to the callback function that has a pointer to the pressed button as a parameter.
 */
void button_set_action(button_t* obj, void(*pressed_callback)(button_t*));

/**
 * @brief	Sets a callback function that is called when any button is pressed.
 * 			The global callback is triggered before the action callback that is set for each button with button_set_action.
 * 			The callback function has the button_object of the pressed button as a parameter.
 *
 * @param pressed_callback		Pointer to the callback function that has a pointer to the pressed button as a parameter.
 */
void button_set_global_action(void(*pressed_callback)(button_t*));

/**
 * @brief	Changes the font of the button.
 *
 *			If the pointer to the button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param font					Index for ROM font (16-31) or RAM font (0-14).
 */
void button_set_font(button_t* obj, uint16_t font);

/**
 * @brief	Changes the background color of the button when it is not pressed.
 *
 *			If the pointer to the button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param color					Background color of the button when it is not pressed.
 */
void button_set_backgroundcolor(button_t* obj, color_t color);

/**
 * @brief	Changes the text color of the button when it is not pressed.
 *
 *			If the pointer to the button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param color					Text color of the button when it is not pressed.
 */
void button_set_textcolor(button_t* obj, color_t color);

/**
 * @brief	Changes the background color of the button when it is pressed.
 *
 *			If the pointer to the button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param color					Background color of the button when it is pressed.
 */
void button_set_backgroundcolor_pressed(button_t* obj, color_t color);

/**
 * @brief	Changes the text color of the button when it is pressed.
 *
 *			If the pointer to the button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param color					Text color of the button when it is pressed.
 */
void button_set_textcolor_pressed(button_t* obj, color_t color);

/**
 * @brief	Sets a figure component to the button that is shown on the button.
 * 			If an component is used to be shown on the button,
 * 			its x/y coordinates define the x/y position on the button if not
 * 			centered. x=0, y=0 of the component would mean that it is left/top aligned on the button.
 *
 * 			If using this, the component must not be added to a panel or to the screen, otherwise 2 components will be shown.
 *
 * @param obj					Pointer to the button object.
 * @param c						Pointer to the component.
 * @param figure_pos			Figure/Images position on the button. Since this is a bitmask, multiple values can be used with "or". Be careful to not
 * 								use combinations like "right | left" or "top | bottom", it is meant for things like "right | top".
 */
void button_set_figure(button_t* obj, component_t* c, BUTTON_FIGURE_POS figure_pos);

void button_set_image(button_t* obj, image_t* img, bool center_image);

#endif

#endif /* BUTTON_H_ */
