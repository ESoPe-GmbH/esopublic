/**
 * 	@file screen.h
 *  @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		When using the screen module, it is necessary you have initialized the eve chip via eve_init in
 *  			eve.h and call eve_handle() inside the main loop before screen_handle.
 *
 *  			The screen module has 2 purposes. First is the interface to the complete screen with the following functions:
 *  			- screen_handle			-> Must be called often, repainting is only inside this function!
 *  			- screen_on				-> Switches the display on
 *  			- screen_off			-> Switches the display completely off
 *  			- screen_set_dimming	-> Can be used to dim the brightness of the display.
 *  			- screen_get_width		-> Returns the width of the display in pixel
 *  			- screen_get_height		-> Returns the height of the display in pixel
 *
 *  			Second purpose is to offer a screen objects which works like a panel for the whole display. Like a Frame for
 *  			a fullscreen application. The screen can always have one screen object active which can be repainted if
 *  			necessary. Since the eve uses an interrupt to signal when the last painting routine was finished, this
 *  			module does only send the screen data inside the screen_handle function, so make sure it is called often!
 *
 *  			The screen object functions are:
 *  			- screen_init_object	-> Initializes a screen object. Must be called first for every screen object.
 *  			- screen_add_component	-> Adds a component to the screen object. A component can be a text, button, etc.
 *  			- screen_paint			-> Sets the new display object and sets a flag for repainting.
 *  			- screen_repaint		-> Sets a flag for repainting the last set display object.
 *
 *				1. 	To implement a screen, declare a screen_t.
 *				2. 	Initialize the screen object with screen_init.
 *					If you need a handle function, that is called when the screen is visible,
 *					add it in the screen_init function.
 *				3.	Add components like buttons, etc. via screen_add_component to the screen.
 *
 *				Sample:
@code
	screen_t 	screen_x_object;
	button_t 	screen_x_button_a;
	text_t	screen_x_text_a;

	void screen_x_handle(void)	// Handle function only if needed -> If not needed set NULL in screen_init
	{
		//... Handle something
	}
	//...

	// Initializes the screen object, sets the background color to white and adds a handle function.
	screen_init(&screen_x_object, color_get(COLOR_WHITE), screen_x_handle, NULL);

	// Initialize the components
	text_init(&screen_x_text_a, 10, 20, "ESoPe");
	button_esope_init(&screen_x_button_a, 10,  100, 100, 70, "Test 1");

	// Add components to the screen_object
	screen_add_component(&screen_x_object, (component_t*)&screen_x_text_a);
	screen_add_component(&screen_x_object, (component_t*)&screen_x_button_a);

	// Paint the screen component -> Only necessary in init for boot screen, not in the init for other screens.
	screen_paint(&screen_x_object);
@endcode
 *
 *	@version 	1.01 (07.12.2018)
 *		- Added cyclic repainting of screens for components where component_needs_cyclic_repaint is true.
 *		- Added suppresion of panel top for specific screens.
 *  @version	1.00 (08.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef SCREEN_H_
#define SCREEN_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI

#include "common.h"

#include "gui_config.h"
#include "color.h"
#include "component.h"
#include "panel.h"
#include "text.h"

#include "module/enum/function_return.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
#define SCREEN_REPAINT_DELAY_MS					16	///< Defines the screen fastest repaint time. If set to 40, the screen
													///< refreshes fastest every 40ms, which results in 25 frames per second.

#define SCREEN_REPAINT_CYCLIC_MS				16 ///< Interval for repainting screens that contain components for which a cyclic repaint is needed.

#define SCREEN_ENABLE_FINGER_TRACING			false	///< If set to true, a circle is drawn at the position of the finger.

#define SCREEN_ENABLE_FOCUSED_COMPONENTS		false	///< If set to true, components can be focussed.

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
typedef enum
{
	SCREEN_TOAST_POSITION_TOP,
	SCREEN_TOAST_POSITION_CENTER,
	SCREEN_TOAST_POSITION_BOTTOM,
}SCREEN_TOAST_POSITION;

/// Type for a screen toast
typedef struct screen_toast_s screen_toast_t;

/// Type for a screen device configuration
typedef struct screen_device_config_s screen_device_config_t;

/// Type for callback function that have a screen pointer as a parameter
typedef void(*screen_callback_t)(screen_t*, uint32_t);

/**
 * @brief Callback for an error of the screen device.
 * @param dev		Pointer to the screen device that had an error.
 * @param err		Code of the error.
 * @param msg		String message of the error.
 */
typedef void(*screen_device_error_t)(screen_device_t* dev, EVE_ERROR err, const char* msg);

/**
 * @struct screen_toast_t
 *
 * A structure containing elements for a toast.
 */
struct screen_toast_s
{
	/// Panel that stores the components for the toast.
	panel_t panel;

	/// Inner background for the toast.
	rect_t rect_inner;

	/// Text object for the text
	text_t text;

	/// Is set when the toast shall be visible.
	bool is_shown;

	/// Is set when the toast shall be shown until it is touched.
	bool is_permanent;

	/// Is set when the toast shall be visible after the screen is changed.
	bool show_delayed;

	/// Timestamp of the start when the toast is shown. Used to calculate when the toast shall be hidden.
	uint32_t timestamp_start;

	/// Duration in milliseconds in which the toast is shown.
	uint32_t duration;

	/// Position of the toast on the screen
	SCREEN_TOAST_POSITION position;

	/// Function callback that is used on toasts that need to be touched in order to disappear.
	screen_callback_t f_callback;

	/// Object for the parameter of the callback function.
	screen_t* callback_obj;

};

/// Structure for the configuration of the screen device during initialization.
struct screen_device_config_s
{
	/// Hardware interface for the EVE chip containing spi, etc.
	eve_hw_interface_t hw;
	/// Display type that is used. This determines the display resolution as well.
	EVE_DISPLAY_TYPE type;
	/// Indicates whether display should be rotated by 180°C.
	/// false: 0° rotation
	/// true: 180° rotation
	bool rotate;
	/// Indicates whether touch should be initialized or not for the display.
	bool has_touch;
	/// Pointer to a function that is called when an error occurs.
 	/// First parameter is a pointer to the screen device object, second parameter is the error code of the error.
 	/// The function might already be called during initialization, if the initialization fails.
	screen_device_error_t f_error;
	/// Set this if the dimming for the display is inverted.
	bool has_inverted_dimming;
};

/**
 * @brief Object of the screen device.
 */
struct screen_device_s
{
	/// EVE object used by the screen device.
	eve_t eve;

	/// Config the device was initialized with.
	screen_device_config_t config;

	/// Pointer to the screen object that is currently selected for painting.
	/// Is set by screen_paint and can be repainted with screen_repaint.
	screen_t*	screen_current_object;

	/// Pointer to the screen object that was painted in the last paint Operation.
	/// Is used to detect if the screen object changed.
	screen_t*	screen_last_painted_object;

	/// Is set inside the screen_paint and screen_repaint function and cleared when the screen is painted
	/// inside the handle function.
	bool screen_shall_repaint;

	/// Used to ignore starting with first focus
	bool repaint_changed;

	/// Is set once the screen is painted and never cleared again. Is checked to determine if the screen should be painted
	/// immediately in the first paint function or after some time.
	bool screen_first_painted;

	/// Is used to delay the repainting of the screen. The timestamp has the time of the last repaint.
	uint32_t screen_paint_timestamp;

	/// Task Object for the system handle.
	system_task_t screen_task;

	/// Toast object that can be shown over the screen.
	screen_toast_t toast;

	/// Callback function for handling errors.
	screen_device_error_t f_error;

	/// Option given in prepaint functions
	uint32_t option;

	/// Pointer to a panel that is shown on top all screens. Is NULL if no pnl_top exists
	panel_t* pnl_top;

	/// Indicates whether pnl_top will be drawn.
	bool show_pnl_top;

	/// Is set to true if a component is on the current screen that needs a cyclic repaint (e.g. edit_text for blinking cursors).
	bool needs_cyclic_repaint;

	/// Is used to set a component that is not painted along with the other components, but at the end. This is needed for video, so it is painted last.
	component_t* component_video;

#if SCREEN_ENABLE_FINGER_TRACING
	/// Stores the x Position of the last touch event
	uint16_t x;

	/// Stores the y Position of the last touch event
	uint16_t y;

	/// Stores the timestamp of the last touch event
	uint32_t touch_timestamp;
#endif

};

/**
 * @struct screen_s
 *
 * A screen object needs to be declared for each screen that needs to be painted inside the application.
 * It is recommended to create a source file for every screen that uses a screen_t.
 *
 * The structure contains an component container as well as a pointer to the last component added to the component
 * next pointer to add components faster in screen_add_component. It also contains a color for the background color of the
 * screen and a pointer to a handle function, that is called when the screen is visible.
 */
struct screen_s
{
	/// Component object for this screen object.
	component_t component;			
	/// Pointer to the last component added via screen_add_component.
	component_t* last_component;		
#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	/// Pointer to the component that is focused.
	component_t* focused_component;	
	/// Pointer to the component that is focused by default
	component_t* default_focused;		
#endif
	/// Color of the background when the screen is painted.
	color_t color_background;			
	/// Custom pointer can be set by the screen implementation for own structure.
	void* user;
	/// Pointer to a handle function that is called when the screen object is set with screen_paint. The pointer can be NULL if no handle function is needed.
	void (*handle_callback)(screen_t*, uint32_t);
	/// Pointer to a function that is called before the screen is painted, when another screen was shown before.
	void (*pre_paint_callback)(screen_t*, uint32_t);	
	/// Pointer to a function that is called when the screen is touched. Parameters are x and y coordinate
	void (*touch_callback)(screen_t*, int32_t, int32_t);		
	/// Pointer to the screen device that is used for painting this screen.
	screen_device_t* screen_device;	
	/// Callback for pressed keys
	void (*key_callback)(void*, char);		
	/// Indicates whether pnl top is shown above the screen or not
	bool show_pnl_top;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions - For Screen Device Objects
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates a screen device struct and returns the pointer to it.
 * 
 * @return Pointer to the screen device or NULL if failed.
 */
screen_device_t* screen_device_create(void);
/**
 * @brief Frees the screen device pointer.
 * 
 * @param dev 				Pointer to the screen device object.
 */
void screen_device_free(screen_device_t* dev);
/**
 * @brief	Initializes this module. Sets the parameters into the spi_handler and writes the initialization routine
 * 			to the eve chip. If the chip does not respond correctly to a command, this function returns false,
 * 			indicating that the display will not work correctly. If the display type is invalid (not implemented),
 * 			the function returns false as well.
 *
 * @param obj				Pointer to the screen device object.
 * @param config			Pointer to the configuration of the display. Can be in stack, since config is copied internally.
 *
 * @return					FUNCTION_RETURN_OK: Initialization was successfull and eve chip responded correctly.
 * 							FUNCTION_RETURN_EXECUTION_ERROR: Invalid display type or eve chip did not respond correctly.
 **/
FUNCTION_RETURN screen_device_init(screen_device_t* obj, screen_device_config_t* config);

/**
 * @brief	Returns the screen that is currently set for painting. The screen object might not be painted yet.
 *
 * @param obj				Pointer to the screen device object.
 * @return					Pointer to the screen object.
 */
screen_t* screen_device_get_current_screen(screen_device_t* obj);

/**
 * @brief	Sets the error handler for the screen device.
 *
 * @param obj				Pointer to the screen device object.
 * @param f_error			Pointer to a function that is called when an error occurs.
 * 							First parameter is a pointer to the screen device object, second parameter is the error code of the error.
 * 							The function might have been already be called during initialization, if the initialization fails.
 */
void screen_device_set_error_handler(screen_device_t* obj, screen_device_error_t f_error);

/**
 * @brief	Switches the power for the screen device and display on/off.
 * @param obj				Pointer to the screen device object.
 * @param b					true: Power is turned on.\n
 * 							false: Power is turned off.
 */
void screen_switch_power(screen_device_t* obj, bool b);

/**
 * @brief	Switches the display on. Does only enable the display itself, not the screen device!
 * @param obj				Pointer to the screen device object.
 */
void screen_device_on(screen_device_t* obj);

/**
 * @brief	Switches the display off. Does only disable the display itself, not the screen device!
 * @param obj				Pointer to the screen device object.
 */
void screen_device_off(screen_device_t* obj);

/**
 * @brief	Dims the brightness of the display. The brightness is set in percent.
 *
 * @param obj				Pointer to the screen device object.
 * @param percent			Brightness of the display in percent.
 */
void screen_device_set_dimming(screen_device_t* obj, uint8_t percent);

/**
 * @brief	Returns the width of the display in pixel. Only valid after eve_init was called.
 *
 * @param obj				Pointer to the screen device object.
 * @return					Width of the display in pixel or 0 if eve_init was not called.
 */
uint16_t screen_device_get_width(screen_device_t* obj);


/**
 * @brief	Returns the height of the display in pixel. Only valid after eve_init was called.
 *
 * @param obj				Pointer to the screen device object.
 * @return					Height of the display in pixel or 0 if eve_init was not called.
 */
uint16_t screen_device_get_height(screen_device_t* obj);

/**
 * @brief	Sets the screen device that is returned with screen_get_default_device.
 * 			Whenever screen_device_init is called, the screen device for screen_get_default_device changes.
 * @param obj				Pointer to the screen device object.
 */
void screen_set_screen_device(screen_device_t* obj);

/**
 * @brief	Returns a pointer to the default screen device.
 * 			When screen_device_init was not called, NULL is returned.
 * 			When screen_device_init is called, the pointer to the initialized screen device is returned.
 * 			When screen_set_screen_device is called, the pointer to the set screen device is returned.
 * @return	NULL when no screen_device_init was called.
 * 			Otherwise it is the pointer to the screen device last set in screen_device_init or screen_set_screen_device.
 */
screen_device_t* screen_get_default_device(void);

/**
 * @brief 	Re-Calibrates the touch of the display. Is used as a workaround function after known
 * 			components like coin acceptor which disturb the touch were in use.
 *
 * @param obj				Pointer to the screen device object.
 */
void screen_device_recalibrate_touch(screen_device_t* obj);

/**
 * @brief	Resets the first painted variable indicating the next screen should be painted immediately
 */
void screen_reset_first_painted(screen_device_t* obj);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions - For Toast on Screen Objects
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief	Shows a toast on the screen. The toast can have multiple lines. It can be shown at top or at the bottom.
 * 			Lines can be separated with a linefeed in the text string. The toast is shown for the specified time.
 *
 * @param obj				Pointer to the screen object that shall be overlayed with the toast.
 * @param text				Text string that should be shown.
 * @param at_top			true: Toast is shown at the top of the screen.
 * 							false: Toast is shown at the bottom of the screen.
 * @param duration			Duration in milliseconds that the toast is shown.
 */
void screen_show_toast(screen_t* obj, char* text, SCREEN_TOAST_POSITION pos, uint32_t duration);

/**
 * @brief	Shows a toast on the screen. The toast can have multiple lines. It can be shown at top or at the bottom.
 * 			Lines can be separated with a linefeed in the text string. The toast is shown until it is touched.
 *
 * @param obj				Pointer to the screen object that shall be overlayed with the toast.
 * @param text				Text string that should be shown.
 * @param at_top			true: Toast is shown at the top of the screen.
 * 							false: Toast is shown at the bottom of the screen.
 * @param f					Pointer to a function that is called when the toast is touched and hidden.
 * 							Uses the obj parameter of this call as a parameter in the callback.
 */
void screen_show_toast_permanent(screen_t* obj, char* text, SCREEN_TOAST_POSITION pos, screen_callback_t f);

/**
 * @brief	Shows a toast on the screen after the screen is changed to the next screen. The toast can have multiple lines. It can be shown at top or at the bottom.
 * 			Lines can be separated with a linefeed in the text string. The toast is shown for the specified time.
 *
 * 			Might be used if the toast shall not be shown on the currently visible screen but after changing to another screen (for example if a callback function of
 * 			a numpad is triggered and the numpad is still visible, but the correct value shall be shown on the screen after the numpad is left).
 *
 * @param obj				Pointer to the screen object that shall be overlayed with the toast.
 * @param text				Text string that should be shown.
 * @param at_top			true: Toast is shown at the top of the screen.
 * 							false: Toast is shown at the bottom of the screen.
 * @param duration			Duration in milliseconds that the toast is shown.
 */
void screen_show_toast_delayed(screen_t* obj, char* text, SCREEN_TOAST_POSITION pos, uint32_t duration);

void screen_hide_toast(screen_device_t* obj);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions - For Screen Objects
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief	Initializes a screen object by setting the variables in the parameter to the structure and clearing its
 * 			dynamic component list.
 *
 * @param obj					Pointer to the screen object
 * @param c						Background color of the screen when painting the screen.
 * @param handle_callback		Handle function that is called in screen_handle when this screen object is set via screen_paint.
 * 								If the callback function is NULL, the callback is not called in screen_handle.
 * @param pre_paint_callback	When the screen object, that should be painted was changed with screen_paint, this callback function is
 * 								called directly before the new screen is painted inside the screen_handle. This function can then be used
 * 								to set default values inside the screen components or renew the values. It is only called if the screen object
 * 								is changed, not when repaint is called!
 **/
void screen_init_object(screen_t* obj, color_t c, screen_callback_t handle_callback, screen_callback_t pre_paint_callback);

/**
 * @brief	Adds a component to the screen object. The screen_object must be initializes before this is called.
 *			See example below for the simple way of adding a component. The initialization function is not shown.
@code
	screen_t 	screen_x_object;
	button_t 	screen_x_button_a;

	// Screen and button must be initialized before calling the add function

	screen_add_component(&screen_x_object, (component_t*)&screen_x_button_a);
@endcode
 *
 * @param obj				Pointer to the screen object
 * @param comp				Pointer to the component that should be added.
 * 							Pointer to button_t, text_t, etc. can be simply casted to component_t pointer.
 */
void screen_add_component(screen_t* obj, component_t* comp);

void screen_set_touch_event(screen_t* obj, void(*f)(screen_t*, int32_t, int32_t));

void screen_set_key_listener(screen_t* obj, void(*f)(screen_t*, char));

/**
 * @brief	Returns the width of the display in pixel. Only valid after eve_init was called.
 *
 * @return					Width of the display in pixel or 0 if eve_init was not called.
 */
uint16_t screen_get_width(screen_t* obj);


/**
 * @brief	Returns the height of the display in pixel. Only valid after eve_init was called.
 *
 * @return					Height of the display in pixel or 0 if eve_init was not called.
 */
uint16_t screen_get_height(screen_t* obj);

/**
 * @brief	Returns the pointer to the eve object that is used for drawing the screen.
 *
 * @return					Pointer to the eve object or NULL if no eve object is set.
 */
eve_t* screen_get_eve(screen_t* obj);

/**
 * @brief	Returns the screen object of a component or NULL if no screen was found.
 * 			Therefore a component needs to be added to a screen or a panel on a screen.
 * 			Therefore this function searches the components up to the top for the screen and returns it.
 *
 * @param obj					Pointer to the component object.
 * @return						Pointer to the screen_t that shows the component or NULL if it is not found.
 */
screen_t* screen_get_from_component(component_t* obj);

/**
 * @brief	Sets an screen object for painting on the screen and requests a repaint.
 * 			If this is the first screen object, the screen is painted immediately! Otherwise the screen is
 * 			painted inside the screen_handle function.
 *
 * @param obj		Pointer to the screen to paint
 * @param option	Optional value that is used as second parameter in the prepaint and handle function
 */
void screen_paint(screen_t* obj, uint32_t option);

/**
 * @brief	Requests a repaint of the current screen object.
 * 			The repainting is done inside the screen_handle once the eve is ready for it.
 */
void screen_repaint_by_component(component_t* obj);

/**
 * @brief	Requests a repaint of the current screen object.
 * 			The repainting is done inside the screen_handle once the eve is ready for it.
 */
void screen_repaint(screen_device_t* device);

/**
 * @brief	Requests a repaint of the current screen object and clears the last painted screen object.
 * 			This is for example needed if the screen object changes its behavior internally and when a button is pressed
 * 			the press should be ignored on the repainted screen.
 * 			The repainting is done inside the screen_handle once the eve is ready for it.
 *
 * @param obj			Pointer to the screen object.
 */
void screen_repaint_changed(screen_t* obj);
#if SCREEN_ENABLE_FOCUSED_COMPONENTS
/**
 * @brief	Sets a component to be the default focused object when the screen is painted.
 * 			Does only work if component is added to the screen.
 *
 * @param obj			Pointer to the screen object.
 * @param comp			Component to be focused.
 */
void screen_set_focus_set_default(screen_t* obj, component_t* comp);
/**
 * Mainly needed without touch screen. Can be used to shift the focus to another focusable component (e.g. Button).
 * Shifts to next component in list.
 *
 * @param obj			Pointer to the screen object.
 */
void screen_focus_next_component(screen_t* obj);
/**
 * Mainly needed without touch screen. Can be used to shift the focus to another focusable component (e.g. Button).
 * Shifts to previous component in list.
 *
 * @param obj			Pointer to the screen object.
 */
void screen_focus_previous_component(screen_t* obj);
/**
 * Sets the current focus to a specific component on the screen.
 *
 * @param obj			Pointer to the screen object.
 * @param comp			Component to be focused.
 */
void screen_focus_set(screen_t* obj, component_t* comp);
/**
 * Returns the currently focused component on the screen
 * @param obj		Pointer to the screen to check for focused component
 * @return			Pointer to the focused component
 */
component_t* screen_get_focused_component(screen_t* obj);
#endif // SCREEN_ENABLE_FOCUSED_COMPONENTS
/**
 * @brief 	Registers a panel that is shown on top of all screens. The initialization of the panel defines coordinates and size.
 * @param dev		Pointer to the screen device where the top panel is used.
 * @param pnl		Pointer to the panel. Set to NULL if the panel should be unregistered.
 */
void screen_top_panel_register(screen_device_t* dev, panel_t* pnl);
/**
 * @brief	Can be used to temporarily hide the top panel.
 * @param dev		Pointer to the screen device where the top panel is used.
 * @param b			true: Top panel is shown (if a top panel is registered)
 * 					false: Top panel is hidden.
 */
void screen_top_panel_set_visible(screen_device_t* dev, bool b);
#if SCREEN_ENABLE_FOCUSED_COMPONENTS
/**
 * Function used to add a key pressed on a keyboard to the current screen. If the currently focused component has a callback for this,
 * the key will be worked inside the component.
 *
 * @param obj			Pointer to the screen device
 * @param scancode		HID scancode of the pressed key
 * @param c				ASCII representation of the HID scancode or 0 if the key is a special key like backspace or F1.
 */
void screen_add_key(screen_device_t* obj, uint8_t scancode, char c);
#endif // SCREEN_ENABLE_FOCUSED_COMPONENTS
#endif

#endif /* SCREEN_H_ */
