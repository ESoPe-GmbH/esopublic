/**
 * 	@file switch_button.h
 * 	@copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			A Switch button is an on/off button with 2 States.
 *			The button has a knob that is on the left or right side of the button, depending on the state.
 *
 *  @version	1.01 (07.12.2018)
 *  	- Back- and Foregroundcolor for disabled switch buttons can be set
 *
 *  @version	1.00 (19.06.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef SWITCH_BUTTON_H_
#define SWITCH_BUTTON_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI

#include "mcu/mcu.h"
#include "color.h"
#include "component.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @enum SWITCH_BUTTON_STATE
 *
 * The switch button has 2 states. State 1 and State 2. On State 1, the knob is on the left side and text 1 is printed on the right
 * side of it. On State 2, the knob is on the right side and text 2 is printed on the left side of it.
 * The Text
 */
typedef enum
{
	SWITCH_BUTTON_STATE_1 = 0x000,		///< Knob is on the left side and text 1 is shown.

	SWITCH_BUTTON_STATE_2 = 0xFFFF		///< Knob is on the right side and text 2 is shown.

}SWITCH_BUTTON_STATE;

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
	component_t component;					///< Component Object used for painting this object on the screen

	char* text;								///< Pointer to the text that should be printed on the button. Needs a 0xFF separator in
											///< the middle to seperate the 2 texts.

	SWITCH_BUTTON_STATE state;				///< State of the switch button. See SWITCH_BUTTON_STATE for details.

	uint16_t font;							///< font index used for the text on the button.

	color_t color_background;				///< Background color of the button when it is not pressed.

	color_t color_foreground;				///< Background color of the button when it is not pressed.

	color_t color_text;						///< Text color of the button when it is not pressed.

	color_t	color_background_disabled;		///< Background color of the button when it is disabled.

	color_t color_text_disabled;			///< Text color of the button when it is disabled.

	uint32_t option;						///< Option that is written to the eve. Should not be modified directly.

	void(*action_callback)(void*, SWITCH_BUTTON_STATE);
											///< Pointer to the function that is called when the button is pressed.
											///< The first parameter is a pointer to the switch_button_t that was pressed.
											///< void* is a placeholder because this switch_button_t structure cannot use
											///< itself inside. The second parameter is the changed SWITCH_BUTTON_STATE.

}switch_button_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes the switch button object and writes default values into it for the colors and fonts.
 *			To set a callback function for the button use button_set_action afterwards.
 *			The button is set visible and enabled by default. As default, SWITCH_BUTTON_STATE_1 is selected.
 *
 *			If the pointer to the button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param x						x Position on the screen
 * @param y						y Position on the screen
 * @param width					width of the button in pixel
 * @param text					Pointer to the text that should be printed on the button.
 * 								The text must be formatted like "Text 1" "\xff" "Text 2". This is needed to set the
 * 								state.
 *
 * @attention	(x + width) should be less or equal screen_get_width() if it should be completely visible.
 * @attention	(y + height) should be less or equal screen_get_height() if it should be completely visible.
 **/
void switch_button_init(switch_button_t* obj, int32_t x, int32_t y, uint16_t width, char* text);

/**
 * @brief	Changes the pointer to the text that is printed on the switch_button. The text is formated for both
 * 			switch button states. The Texts are separated by a 0xFF.
 * 				Example:
 * 					switch_button_set_text(obj, "Text 1" "\xFF" "Text 2");
 *
 *			If the pointer to the switch_button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the switch_button object.
 * @param text					Pointer to the text that should be printed on the switch_button.
 */
void switch_button_set_text(switch_button_t* obj, char* text);

/**
 * @brief	Changes the visibility of the switch_button.
 *			If the visibility value is different from the previously set visibility value, a screen repaint is triggered.
 *
 *			If the pointer to the switch_button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the switch_button object.
 * @param b						true: Button is visible, false: Button is not visible.
 */
void switch_button_set_visible(switch_button_t* obj, bool b);

/**
 * @brief	Sets the switch_button enabled or disabled. If the switch_button is disabled, no further action is possible.
 *			If the enabled value is different from the previously set enabled value, a screen repaint is triggered.
 *
 *			If the pointer to the switch_button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the switch_button object.
 * @param b						true: Button is enabled, false: Button is not enabled.
 */
void switch_button_set_enabled(switch_button_t* obj, bool b);

/**
 * @brief	Sets the state of the switch button to SWITCH_BUTTON_STATE_1 or SWITCH_BUTTON_STATE_2.
 * 			If the state is different from the previous state, a screen repaint is triggered.
 *
 *			If the pointer to the switch_button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the switch_button object.
 * @param state					SWITCH_BUTTON_STATE_1: Text 1 is shown and the knob is on the left side.
 * 								SWITCH_BUTTON_STATE_2: Text 2 is shown and the knob is on the right side.
 */
void switch_button_set_state(switch_button_t* obj, SWITCH_BUTTON_STATE state);

/**
 * @brief	Sets the state of the switch button to SWITCH_BUTTON_STATE_1 or SWITCH_BUTTON_STATE_2 with a boolean value.
 * 			If b is true, SWITCH_BUTTON_STATE_2 is set, else SWITCH_BUTTON_STATE_1 is set.
 * 			If the state is different from the previous state, a screen repaint is triggered.
 *
 *			If the pointer to the switch_button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the switch_button object.
 * @param state					false: SWITCH_BUTTON_STATE_1: Text 1 is shown and the knob is on the left side.
 * 								true: SWITCH_BUTTON_STATE_2: Text 2 is shown and the knob is on the right side.
 */
void switch_button_set_state_2_active(switch_button_t* obj, bool b);

/**
 * @brief	Sets a callback function that is called when the switch_button is pressed.
 * 			The callback function has the switch_button_object of the pressed switch_button as first parameter
 * 			and the state of the switch button as the second parameter.
 *
 *			If the pointer to the switch_button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the switch_button object.
 * @param pressed_callback		Pointer to the callback function that has a pointer to the pressed switch_button as first parameter
 * 								and the state of the switch button as the second parameter.
 */
void switch_button_set_action(switch_button_t* obj, void(*pressed_callback)(switch_button_t*, SWITCH_BUTTON_STATE));

/**
 * @brief	Changes the font of the switch_button.
 *
 *			If the pointer to the switch_button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the switch_button object.
 * @param font					Index for ROM font (16-31) or RAM font (0-14).
 */
void switch_button_set_font(switch_button_t* obj, uint16_t font);

/**
 * @brief	Changes the background color of the switch_button when it is not pressed.
 *
 *			If the pointer to the switch_button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the switch_button object.
 * @param color					Background color of the switch_button when it is not pressed.
 */
void switch_button_set_backgroundcolor(switch_button_t* obj, color_t color);

/**
 * @brief	Changes the text color of the switch_button when it is not pressed.
 *
 *			If the pointer to the switch_button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the switch_button object.
 * @param color					Text color of the switch_button when it is not pressed.
 */
void switch_button_set_textcolor(switch_button_t* obj, color_t color);

/**
 * @brief	Changes the background color of the switch_button when it disabled.
 *
 *			If the pointer to the switch_button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the switch_button object.
 * @param color					Background color of the switch_button when it is disabled.
 */
void switch_button_set_backgroundcolor_disbled(switch_button_t* obj, color_t color);

/**
 * @brief	Changes the text color of the switch_button when it is disabled.
 *
 *			If the pointer to the switch_button object is NULL, nothing happens.
 *
 * @param obj					Pointer to the switch_button object.
 * @param color					Text color of the switch_button when it is disabled.
 */
void switch_button_set_textcolor_disabled(switch_button_t* obj, color_t color);

#endif

#endif /* SWITCH_BUTTON_H_ */
