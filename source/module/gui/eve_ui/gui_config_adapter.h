/**
 * 	@file gui_config_adapter.h
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Creates the gui config from the KConfig settings. This is used when esopublic is used as esp component.
 *
 *  @version	1.00 (21.03.2025)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef GUI_CONFIG_ADAPTER_H_
#define GUI_CONFIG_ADAPTER_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI && CONFIG_ESOPUBLIC_ENABLE

#include "color.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for screens
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default screen background color
#define GUI_CONFIG_DEFAULT_SCREEN_BACKCOLOR				        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_DEFAULT_SCREEN_BACKCOLOR)

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for more then one component
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default button, switch button and text font.
/// Fonts 16 - 31 are the eve default fonts. Readable Fonts are 20 - 25 and 26 - 31.
#define GUI_CONFIG_DEFAULT_FONT							        CONFIG_EVE_UI_DEFAULT_FONT
/// Default line skip in multi line text, button objects.
#define GUI_CONFIG_DEFAULT_LINE_SKIP					        CONFIG_EVE_UI_DEFAULT_LINE_SKIP
/// If no touch is available, components can be focused to show a border on selected components
#define GUI_CONFIG_ENABLE_FOCUS							        CONFIG_EVE_UI_ENABLE_FOCUS
/// Size of the border for focused components
#define GUI_CONFIG_FOCUS_BORDER_SIZE					        CONFIG_EVE_UI_FOCUS_BORDER_SIZE
/// Default color for the border of focused components
#define GUI_CONFIG_FOCUS_BORDER_COLOR					        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_FOCUS_BORDER_COLOR)

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for buttons
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default button type.
/// See BUTTON_TYPE enumeration in button.h for possible values.
#define BUTTON_DEFAULT_TYPE								        CONFIG_EVE_UI_BUTTON_DEFAULT_TYPE
/// Default background color for buttons.
#define BUTTON_DEFAULT_BACKGROUND_COLOR					        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_BUTTON_DEFAULT_BACKGROUND_COLOR)
/// Default background color for disabled buttons.
#define BUTTON_DEFAULT_BACKGROUND_COLOR_DISABLED		        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_BUTTON_DEFAULT_BACKGROUND_COLOR_DISABLED)
/// Default background color for pressed buttons.
#define BUTTON_DEFAULT_BACKGROUND_COLOR_PRESSED			        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_BUTTON_DEFAULT_BACKGROUND_COLOR_PRESSED)
/// Default text color for buttons.
#define BUTTON_DEFAULT_TEXT_COLOR						        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_BUTTON_DEFAULT_TEXT_COLOR)
/// Default text color for disabled buttons.
#define BUTTON_DEFAULT_TEXT_COLOR_DISABLED				        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_BUTTON_DEFAULT_TEXT_COLOR_DISABLED)
/// Default text color for pressed buttons.
#define BUTTON_DEFAULT_TEXT_COLOR_PRESSED				        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_BUTTON_DEFAULT_TEXT_COLOR_PRESSED)
/// Default button and switch button font.
/// Fonts 16 - 31 are the eve default fonts. Readable Fonts are 20 - 25 and 26 - 31.
#define BUTTON_DEFAULT_TEXT_FONT						        CONFIG_EVE_UI_BUTTON_DEFAULT_TEXT_FONT
/// Default shadow color for buttons. Only for rectangle and round types. Not for the api type.
#define BUTTON_DEFAULT_SHADOW_COLOR						        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_BUTTON_DEFAULT_SHADOW_COLOR)
/// Default highlight color for buttons. Only for rectangle and round types. Not for the api type.
#define BUTTON_DEFAULT_HIGHLIGHT_COLOR					        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_BUTTON_DEFAULT_HIGHLIGHT_COLOR)
/// Default line skip in multi line button objects.
#define BUTTON_DEFAULT_LINE_SKIP						        CONFIG_EVE_UI_BUTTON_DEFAULT_LINE_SKIP

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for circle
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default background color for circles.
#define CIRCLE_DEFAULT_BACKGROUND						        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_CIRCLE_DEFAULT_BACKGROUND)

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for rectangles
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default background color for rectangles.
#define RECT_DEFAULT_BACKGROUND							        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_RECT_DEFAULT_BACKGROUND)

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for switch buttons
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default background color for switch buttons.
#define SWITCH_BUTTON_DEFAULT_BACKGROUND_COLOR			        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_SWITCH_BUTTON_DEFAULT_BACKGROUND_COLOR)
/// Default foreground color for switch buttons.
#define SWITCH_BUTTON_DEFAULT_FOREGROUND_COLOR			        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_SWITCH_BUTTON_DEFAULT_FOREGROUND_COLOR)
/// Default text color for switch buttons.
#define SWITCH_BUTTON_DEFAULT_TEXT_COLOR				        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_SWITCH_BUTTON_DEFAULT_TEXT_COLOR)
/// Default background color for disabled switch buttons.
#define SWITCH_BUTTON_DEFAULT_BACKGROUND_COLOR_DISABLED		    COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_SWITCH_BUTTON_DEFAULT_BACKGROUND_COLOR_DISABLED)
/// Default text color for disabled switch buttons.
#define SWITCH_BUTTON_DEFAULT_TEXT_COLOR_DISABLED			    COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_SWITCH_BUTTON_DEFAULT_TEXT_COLOR_DISABLED)
/// Default switch button font.
#define SWITCH_BUTTON_DEFAULT_TEXT_FONT					        CONFIG_EVE_UI_SWITCH_BUTTON_DEFAULT_TEXT_FONT

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for text objects
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default color for text objects.
#define TEXT_DEFAULT_COLOR								        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_TEXT_DEFAULT_COLOR)
/// Default color for disabled text
#define TEXT_DEFAULT_COLOR_DISABLED						        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_TEXT_DEFAULT_COLOR_DISABLED)
/// Default text font.
#define TEXT_DEFAULT_FONT								        CONFIG_EVE_UI_TEXT_DEFAULT_FONT
/// Default line skip in multi line text objects.
#define TEXT_DEFAULT_LINE_SKIP							        CONFIG_EVE_UI_TEXT_DEFAULT_LINE_SKIP

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for edit text objects
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default color for the text in the edit box when unfocused
#define EDIT_TEXT_DEFAULT_COLOR_UNFOCUSED						COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_EDIT_TEXT_DEFAULT_COLOR_UNFOCUSED)
/// Default color for the text in the edit box when focused
#define EDIT_TEXT_DEFAULT_COLOR_FOCUSED							COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_EDIT_TEXT_DEFAULT_COLOR_FOCUSED)
/// Default color of the hidden text
#define EDIT_TEXT_DEFAULT_COLOR_HIDE_TEXT						COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_EDIT_TEXT_DEFAULT_COLOR_HIDE_TEXT)
/// Default text font.
#define EDIT_TEXT_DEFAULT_FONT									CONFIG_EVE_UI_EDIT_TEXT_DEFAULT_FONT
/// Default background color when edit text is not focused
#define EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_UNFOCUSED			COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_UNFOCUSED)
/// Default background color when edit text is focused
#define EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_FOCUSED				COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_FOCUSED)
/// Default background color when edit text is disabled
#define EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_DISABLED				COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_EDIT_TEXT_DEFAULT_BACKGROUND_COLOR_DISABLED)
/// Default text color when edit text is disabled
#define EDIT_TEXT_DEFAULT_COLOR_DISABLED						COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_EDIT_TEXT_DEFAULT_COLOR_DISABLED)
/// Default border color for edit text
#define EDIT_TEXT_DEFAULT_COLOR_BORDER							COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_EDIT_TEXT_DEFAULT_COLOR_BORDER)

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for toast objects
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Number of milliseconds a toast is shown by default
#define TOAST_DEFAULT_DURATION									CONFIG_EVE_UI_TOAST_DEFAULT_DURATION
/// Background color of the toast
#define TOAST_DEFAULT_BACKGROUND_COLOR							COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_TOAST_DEFAULT_BACKGROUND_COLOR)
/// Border color of the toast
#define TOAST_DEFAULT_BORDER_COLOR								COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_TOAST_DEFAULT_BORDER_COLOR)
/// Alpha value for the toast to make it transparent
#define TOAST_DEFAULT_BACKGROUND_ALPHA							CONFIG_EVE_UI_TOAST_DEFAULT_BACKGROUND_ALPHA
/// Default text color of a toast
#define TOAST_DEFAULT_TEXT_COLOR								COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_TOAST_DEFAULT_TEXT_COLOR)
/// Default font of a toast to use
#define TOAST_DEFAULT_FONT										CONFIG_EVE_UI_TOAST_DEFAULT_FONT

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration for keys
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default background color for keys.
#define KEY_DEFAULT_BACKGROUND_COLOR					        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_KEY_DEFAULT_BACKGROUND_COLOR)
/// Default background color for disabled keys.
#define KEY_DEFAULT_BACKGROUND_COLOR_DISABLED		            COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_KEY_DEFAULT_BACKGROUND_COLOR_DISABLED)
/// Default background color for pressed keys.
#define KEY_DEFAULT_BACKGROUND_COLOR_PRESSED			        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_KEY_DEFAULT_BACKGROUND_COLOR_PRESSED)
/// Default text color for keys.
#define KEY_DEFAULT_TEXT_COLOR						            COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_KEY_DEFAULT_TEXT_COLOR)
/// Default text color for disabled keys.
#define KEY_DEFAULT_TEXT_COLOR_DISABLED				            COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_KEY_DEFAULT_TEXT_COLOR_DISABLED)
/// Default text color for pressed keys.
#define KEY_DEFAULT_TEXT_COLOR_PRESSED				            COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_KEY_DEFAULT_TEXT_COLOR_PRESSED)
/// Default key font.
#define KEY_DEFAULT_TEXT_FONT						            CONFIG_EVE_UI_KEY_DEFAULT_TEXT_FONT
/// Default shadow color for keys.
#define KEY_DEFAULT_SHADOW_COLOR						        COLOR_RGB_FROM_HEX(CONFIG_EVE_UI_KEY_DEFAULT_SHADOW_COLOR)
/// Default line skip in multi line key objects.
#define KEY_DEFAULT_LINE_SKIP						            CONFIG_EVE_UI_KEY_DEFAULT_LINE_SKIP

#endif // MODULE_ENABLE_GUI && CONFIG_ESOPUBLIC_ENABLE

#endif /* GUI_CONFIG_H_ */
