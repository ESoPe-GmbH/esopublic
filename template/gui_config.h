/**
 * 	@file gui_config.h
 *  @copyright Urheberrecht 2015-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			
 *  @version	1.01 (07.12.2018)
 *  	- Default values for switch button disabled colors
 *
 *  @version	1.00 (16.01.2015)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef GUI_CONFIG_H_
#define GUI_CONFIG_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for screens
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default screen background color
#define GUI_CONFIG_DEFAULT_SCREEN_BACKCOLOR				        COLOR_WHITE

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for more then one component
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default button, switch button and text font.
/// Fonts 16 - 31 are the eve default fonts. Readable Fonts are 20 - 25 and 26 - 31.
#define GUI_CONFIG_DEFAULT_FONT							        28
/// Default line skip in multi line text, button objects.
#define GUI_CONFIG_DEFAULT_LINE_SKIP					        5
/// If no touch is available, components can be focused to show a border on selected components
#define GUI_CONFIG_ENABLE_FOCUS							        true
/// Size of the border for focused components
#define GUI_CONFIG_FOCUS_BORDER_SIZE					        1
/// Default color for the border of focused components
#define GUI_CONFIG_FOCUS_BORDER_COLOR					        COLOR_WHITE

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for buttons
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default button type.
/// See BUTTON_TYPE enumeration in button.h for possible values.
#define BUTTON_DEFAULT_TYPE								        BUTTON_TYPE_API_LIKE
/// Default background color for buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define BUTTON_DEFAULT_BACKGROUND_COLOR					        COLOR_DARK_GRAY
/// Default background color for disabled buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define BUTTON_DEFAULT_BACKGROUND_COLOR_DISABLED		        COLOR_DIM_GRAY
/// Default background color for pressed buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define BUTTON_DEFAULT_BACKGROUND_COLOR_PRESSED			        COLOR_DIM_GRAY
/// Default text color for buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define BUTTON_DEFAULT_TEXT_COLOR						        COLOR_WHITE
/// Default text color for disabled buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define BUTTON_DEFAULT_TEXT_COLOR_DISABLED				        COLOR_DARK_GRAY
/// Default text color for pressed buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define BUTTON_DEFAULT_TEXT_COLOR_PRESSED				        COLOR_WHITE
/// Default button and switch button font.
/// Fonts 16 - 31 are the eve default fonts. Readable Fonts are 20 - 25 and 26 - 31.
#define BUTTON_DEFAULT_TEXT_FONT						        GUI_CONFIG_DEFAULT_FONT
/// Default shadow color for buttons. Only for rectangle and round types. Not for the api type.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define BUTTON_DEFAULT_SHADOW_COLOR						        COLOR_BLACK
/// Default shadow color for buttons. Only for rectangle and round types. Not for the api type.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define BUTTON_DEFAULT_HIGHLIGHT_COLOR					        COLOR_WHITE
/// Default line skip in multi line button objects.
#define BUTTON_DEFAULT_LINE_SKIP						        GUI_CONFIG_DEFAULT_LINE_SKIP

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for rectangles
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default background color for rectangles.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define RECT_DEFAULT_BACKGROUND							        COLOR_BLACK

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for switch buttons
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default background color for switch buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define SWITCH_BUTTON_DEFAULT_BACKGROUND_COLOR			        30, 30, 30
/// Default foreground color for switch buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define SWITCH_BUTTON_DEFAULT_FOREGROUND_COLOR			        COLOR_SILVER
/// Default foreground color for switch buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define SWITCH_BUTTON_DEFAULT_TEXT_COLOR				        COLOR_WHITE
/// Default background color for disabled switch buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define SWITCH_BUTTON_DEFAULT_BACKGROUND_COLOR_DISABLED		    COLOR_DIM_GRAY
/// Default text color for disabled switch buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define SWITCH_BUTTON_DEFAULT_TEXT_COLOR_DISABLED			    COLOR_DARK_GRAY
/// Default switch button font.
/// Fonts 16 - 31 are the eve default fonts. Readable Fonts are 20 - 25 and 26 - 31.
#define SWITCH_BUTTON_DEFAULT_TEXT_FONT					        GUI_CONFIG_DEFAULT_FONT

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for text objects
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default color for text objects.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define TEXT_DEFAULT_COLOR								        COLOR_BLACK
/// Default color for disabled text
#define TEXT_DEFAULT_COLOR_DISABLED						        COLOR_TEXT_GREY
/// Default text font.
/// Fonts 16 - 31 are the eve default fonts. Readable Fonts are 20 - 25 and 26 - 31.
#define TEXT_DEFAULT_FONT								        GUI_CONFIG_DEFAULT_FONT
/// Default line skip in multi line text objects.
#define TEXT_DEFAULT_LINE_SKIP							        GUI_CONFIG_DEFAULT_LINE_SKIP

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for edit text objects
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default color for the text in the edit box when unfocused
#define EDIT_TEXT_DEFAULT_COLOR_UNFOCUSED						COLOR_BLACK
/// Default color for the text in the edit box when focused
#define EDIT_TEXT_DEFAULT_COLOR_FOCUSED							COLOR_BLACK
/// Default color of the hidden text
#define EDIT_TEXT_DEFAULT_COLOR_HIDE_TEXT						COLOR_TEXT_GREY
/// Default text font.
/// Fonts 16 - 31 are the eve default fonts. Readable Fonts are 20 - 25 and 26 - 31.
#define EDIT_TEXT_DEFAULT_FONT									28
/// Default background color when edit text is not focused
#define EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_UNFOCUSED			COLOR_WHITE
/// Default background color when edit text is focused
#define EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_FOCUSED				COLOR_WHITE
/// Default background color when edit text is disabled
#define EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_DISABLED				COLOR_TEXT_GREY
/// Default text color when edit text is disabled
#define EDIT_TEXT_DEFAULT_COLOR_DISABLED						COLOR_DIM_GRAY
/// Default border color for edit text
#define EDIT_TEXT_DEFAULT_COLOR_BORDER							COLOR_BLACK

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for toast objects
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Number of milliseconds a toast is shown by default
#define TOAST_DEFAULT_DURATION									5000
/// Background color of the toast
#define TOAST_DEFAULT_BACKGROUND_COLOR							COLOR_WHITE
/// Border color of the toast
#define TOAST_DEFAULT_BORDER_COLOR								COLOR_BLACK
/// Alpha value for the toast to make it transparent
#define TOAST_DEFAULT_BACKGROUND_ALPHA							0xB0
/// Default text color of a toast
#define TOAST_DEFAULT_TEXT_COLOR								COLOR_BLACK
/// Default font of a toast to use
#define TOAST_DEFAULT_FONT										26

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for keys
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default background color for buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define KEY_DEFAULT_BACKGROUND_COLOR					        COLOR_SILVER
/// Default background color for disabled buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define KEY_DEFAULT_BACKGROUND_COLOR_DISABLED		            COLOR_DIM_GRAY
/// Default background color for pressed buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define KEY_DEFAULT_BACKGROUND_COLOR_PRESSED			        COLOR_RED_ESOPE
/// Default text color for buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define KEY_DEFAULT_TEXT_COLOR						            COLOR_BLACK
/// Default text color for disabled buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define KEY_DEFAULT_TEXT_COLOR_DISABLED				            COLOR_DARK_GRAY
/// Default text color for pressed buttons.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define KEY_DEFAULT_TEXT_COLOR_PRESSED				            COLOR_WHITE
/// Default button and switch button font.
/// Fonts 16 - 31 are the eve default fonts. Readable Fonts are 20 - 25 and 26 - 31.
#define KEY_DEFAULT_TEXT_FONT						            GUI_CONFIG_DEFAULT_FONT
/// Default shadow color for buttons. Only for rectangle and round types. Not for the api type.
/// See color.h for possible colors or use r, g, b as 3 values separated by comma.
#define KEY_DEFAULT_SHADOW_COLOR						        COLOR_BLACK
/// Default line skip in multi line button objects.
#define KEY_DEFAULT_LINE_SKIP						            GUI_CONFIG_DEFAULT_LINE_SKIP

#endif

#endif /* GUI_CONFIG_H_ */
