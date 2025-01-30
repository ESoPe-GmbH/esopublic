/**
 * 	@file component.h
 *  @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief	Component is like a super class in object oriented programming.
 *  		Each component structure like a button or text structure MUST have a component_t as the first variable
 *  		inside its structure. This causes that the pointer to a button is also the pointer to its component_t.
 *  		Which makes it easy to use the component, when having only the button or text object. This is necessary for
 *  		screen and panel objects!
 *
 *	@version	1.01 (07.12.2018)
 *		- component_needs_cyclic_repaint added
 *  @version	1.00 (11.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "common.h"
#include "gui_config.h"
#include "../eve/eve.h"
#include "eve_ui_helper.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Invalid component (default value). For example if it was not initialized.
#define COMPONENT_TYPE_INVALID				0

/// Type for screen components.
#define COMPONENT_TYPE_SCREEN				1

/// Type for panel components.
#define COMPONENT_TYPE_PANEL				2

/// Type for button components.
#define COMPONENT_TYPE_BUTTON				3

/// Type for image components.
#define COMPONENT_TYPE_IMAGE				4

/// Type for rectangle components.
#define COMPONENT_TYPE_RECTANGLE			5

/// Type for switch button components.
#define COMPONENT_TYPE_SWITCH_BUTTON		6

/// Type for text components.
#define COMPONENT_TYPE_TEXT					7

/// Type for edit text components.
#define COMPONENT_TYPE_EDIT_TEXT			8

/// Type for video components.
#define COMPONENT_TYPE_VIDEO				9

/// Type for commandbar components.
#define COMPONENT_TYPE_COMMANDBAR			10

/// Type for linechart components.
#define COMPONENT_TYPE_LINECHART			12

/// Type for circle components
#define COMPONENT_TYPE_CIRCLE				13

/// Type for custom components, created by the user
#define COMPONENT_TYPE_CUSTOM				255

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enum
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Events that can happen on components. These values are used for a bitmask to make sure multiple events might be subscribed.
 */
typedef enum component_event_e
{
	/// @brief Component is loaded onto the screen. Is called when the screen or panel becomes visible.
	COMPONENT_EVENT_LOAD = 0x0001,
	/// @brief Handle that is called periodically while the component is visible.
	COMPONENT_EVENT_HANDLE = 0x0002,
	/// @brief Component is unloaded from the screen. Is calles when the screen or panel changes or becomes invisible.
	COMPONENT_EVENT_UNLOAD = 0x0004
}COMPONENT_EVENT_T;

// TODO: At the moment we have alignments for components separately. This is planned to be supported by all in the future.
/**
 * @enum COMPONENT_ALIGNMENT
 * 		Enumerates the possible horizontal alignments for the component object.
 * 		The values can be OR'ed to use at a bitmask. 
 * 		The horizontal alignment depends on the x-coordinate:
 * 		 	Left aligned -> x is the coordinate for the left side of the component.
 * 		 	Right aligned -> x is the coordinate for the right side of the component.
 * 		 	Center X aligned -> x is the coordinate at the center of the component.
 * 		 	Top aligned -> y is the coordinate at top of the component.
 * 		 	Bottom aligned -> y is the coordinate at bottom of the component.
 * 		 	Center Y aligned -> y is the coordinate at the center of the component.
 * 		 	Center aligned -> x and y is the coordinate at the center of the component.
 */
typedef enum
{
	/// The component is left aligned -> x is the coordinate for the left side of the component.	
	/// Default x-alignment for components.
	COMPONENT_ALIGNMENT_LEFT = 0x01,		
	/// The component is right aligned -> x is the coordinate for the right side of the component.
	COMPONENT_ALIGNMENT_RIGHT = 0x02,	
	/// The component is center aligned -> x is the coordinate at the center of the component.
	COMPONENT_ALIGNMENT_CENTER_X = 0x04,	
	/// The component is top aligned -> y is the coordinate at top of the component.	
	/// Default y-alignment for components.
	COMPONENT_ALIGNMENT_TOP = 0x10,		
	/// The component is bottom aligned -> y is the coordinate at bottom of the component.
	COMPONENT_ALIGNMENT_BOTTOM = 0x20,
	/// The component is center aligned -> y is the coordinate at the center of the component.
	COMPONENT_ALIGNMENT_CENTER_Y = 0x40,
	/// The component is center aligned -> y is the coordinate at the center of the component.
	COMPONENT_ALIGNMENT_CENTER = 0x44,
}COMPONENT_ALIGNMENT_T;

/**
 * @brief Defines for the edges of a component.
 */
typedef enum component_edge_e
{
	/// @brief Value for the top/left coordinate
	COMPONENT_EDGE_LEFT_TOP,
	/// @brief Value for the bottom/left coordinate
	COMPONENT_EDGE_LEFT_BOTTOM,
	/// @brief Value for the top/right coordinate
	COMPONENT_EDGE_RIGHT_TOP,
	/// @brief Value for the bottom/right coordinate
	COMPONENT_EDGE_RIGHT_BOTTOM
}COMPONET_EDGE_T;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief UI component as the base class of other components like text and button.
typedef struct component_s component_t;

/**
 * @brief Structure containing data regarding to the events of @c COMPONENT_EVENT_T.
 */
typedef struct component_event_s component_event_t;

/**
 * @brief Callback function that is used to paint a component.
 * The callback is called using the component's pointer as first argument. If you have a custom component, put the component structure as the first element of the
 * custom components structure. This way you can use your components type as first parameter.
 * 
 * @param c				Pointer to the component.
 * @param location		Absolute Coordinates of the components parent from the screen top-left. Those are the top-left coordinates.
 */
typedef void (*component_paint_cb_t)(component_t* c, eve_ui_point_t location);
/**
 * @brief Function that is called from the eve component when the component was tagged and pressed or released.
 * 
 * @param c				Pointer to the component.
 * @param is_pressed	true when it was pressed or false when it was released.
 */
typedef void (*component_pressed_cb_t)(component_t* c, bool is_pressed);
/**
 * @brief Function that is called when the component is focused and a character of the keyboard was entered.
 * 
 * @param c				Pointer to the component.
 * @param hid			HID scancode of the pressed key
 * @param ascii			ASCII representation of the pressed key or 0 when the scancode is a special key like backspace.
 */
typedef void (*component_keyboard_cb_t)(component_t* c, uint8_t hid, char ascii);
/**
 * @brief Function that is called on different events, based on which events the component subscribed.
 * 
 * @param c				Pointer to the component.
 * @param event			Type of event that occurred.
 * @param data			Pointer to the data regarding the event.
 */
typedef void (*component_event_cb_t)(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @struct component_t
 *
 * Structure for storing information that every component like, button, screen or text is using.
 * Component supports a dynamically linked list, by having a pointer to itself and the next component inside the list.
 * This is used for a screen and panel object to draw every component, that is added to them.
 *
 * Does also contain data which is needed by all components, like the coordinates, a callback for the painting function
 * and a visibility flag.
 */
struct component_s
{
	/// Type of the component as defined with COMPONENT_TYPE_ define. Is needed to differentiate between the components.
	uint8_t type;
	/// Points to next component on the screen or is NULL if there is no next component.
	component_t* next_component;
	/// Points to the component where this component is shown on.
	/// If parent_component is NULL, the component is either a screen or it was not added to
	/// another component.
	component_t* parent_component;				
	/// Pointer for the user to store more info.
	void* user;
	/// @brief Coordinates of the component on the screen.
	eve_ui_point_t origin;
	/// @brief Size of the component on the screen.
	eve_ui_size_t size;
	/// If true, component is painted on the screen, else the component is not visible.
	bool is_visible;
	/// If true, the component is enabled. This can be used for button type components.
	bool is_enabled;
	/// If true, focused component can be selected on the screen.
	bool is_focusable;					
	/// Pointer to the paint function of the component or NULL if component has no paint function. 
	component_paint_cb_t paint_function;
	/// Pointer to a function that is called from the eve component when the component was tagged and pressed or released.
	component_pressed_cb_t pressed_callback;	
	/// Pointer to a function that is called when the component is focused and a character of the keyboard was entered.
	component_keyboard_cb_t keyboard_callback;
	/// Pointer to a memory file object for this component if it is needed.
	eve_memory_file_t* mem_file_ptr;
	/// @brief Pointer to a function that is called on different events.
	component_event_cb_t f_cb_event;
	/// @brief Pointer to a function that is called on different events. Pointer is set and used for subcomponents using component.
	component_event_cb_t f_cb_event_internal;
	/// @brief Bitmask for subscribed events based on @c COMPONENT_EVENT_T.
	uint16_t subscribed_events;
	/// @brief Is set when the component is put on the screen and cleared when the component is removed from the screen.
	bool is_loaded;
	/// Alignment of the component.
	uint8_t alignment;
};

struct component_event_s
{
	/// @brief The current screen device.
	screen_device_t* device;
	/// @brief The currently visible screen.
	screen_t* screen;
	// Events @c COMPONENT_EVENT_LOAD, @c COMPONENT_EVENT_UNLOAD, @c COMPONENT_EVENT_HANDLE have no event data.
	// Union for different event structures.
	union
	{
		// TODO: Data for events.

		/// @brief Dummy because at the moment we have no events.
		uint32_t dummy;
	};
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes a component with default values.
 * 			The component is visible by default and the next_component pointer is NULL.
 *
 * @param obj					Pointer to the component object.
 * @param type					Type of the component. See the COMPONENT_TYPE_ defines.
 * @param f						Pointer to the paint callback function. See @c component_paint_cb_t for details on the parameter.
 **/
void component_init(component_t* obj, uint8_t type, component_paint_cb_t f);
/**
 * @brief 	Set an event callback to the component. The type of subscribed events is declared via bitmask.
 * 
 * @param obj					Pointer to the component object.
 * @param f_cb_event 			Pointer to the function that is triggered on event.
 * @param events 				Bitmask of events that should trigger the callback.
 */
void component_set_event_callback(component_t* obj, component_event_cb_t f_cb_event, COMPONENT_EVENT_T events);
/**
 * @brief	Changes the visibility of a component.
 *			If the visibility value is different from the previously set visibility value, a screen repaint is triggered.
 *
 *			If the pointer to the component object is NULL, nothing happens.
 *
 * @param obj					Pointer to the component object.
 * @param b						true: Component is visible, false: Component is not visible.
 */
void component_set_visible(component_t* obj, bool is_visible);
/**
 * @brief 	Sets the alignment for the component.
 * 
 * @param obj					Pointer to the component object.
 * @param alignment 			Alignment for the component. This is a bitmask value to set the x and y alignment at once.
 */
void component_set_alignment(component_t* obj, COMPONENT_ALIGNMENT_T alignment);
/**
 * @brief	Sets the component enabled or disabled. If the component is disabled, no further action is possible.
 *			If the enabled value is different from the previously set enabled value, a screen repaint is triggered.
 *
 *			If the pointer to the component object is NULL, nothing happens.
 *
 * @param obj					Pointer to the component object.
 * @param b						true: Component is enabled, false: Component is not enabled.
 */
void component_set_enabled(component_t* obj, bool b);

/**
 * @brief	Returns the eve object of a component or NULL if no eve was found.
 * 			Therefore a component needs to be added to a screen or a panel on a screen since only the screen
 * 			knows the eve_t. Therefore this function searches the components up to the top for the screen
 * 			that contains the eve_t and returns it.
 *
 * @param obj					Pointer to the component object.
 * @return						Pointer to the eve_t that shows the component or NULL if it is not found.
 */
eve_t* component_get_eve(component_t* obj);

/**
 * @brief 	Indicates whether the component is focused on the screen.
 * @param obj		Pointer to the component
 * @return			true: Component is focused, false: Component is not focused
 */
bool component_is_focused(component_t* obj);

/**
 * @brief	Function to change the location of a component.
 * 			Does nothing if obj is NULL.
 *
 * @param obj					Pointer to the component object.
 * @param point					Coordinates of the component.
 */
void component_set_location(component_t* obj, eve_ui_point_t point);

/**
 * @brief	Returns the x-Position of the component.
 * 			If include_width is false, the left x coordinate of the component is returned.
 * 			If include_width is true, the right x coordinate of the component is returned.
 *
 * @param obj					Pointer to the component object.
 * @param include_width			false: The left x coordinate of the component is returned.
 * 								true: The right x coordinate of the component is returned.
 * @return						x-Coordinate of the component.
 */
uint16_t component_get_x(component_t* obj, bool include_width);

/**
 * @brief	Returns the y-Position of the component.
 * 			If include_height is false, the top y coordinate of the component is returned.
 * 			If include_height is true, the bottom y coordinate of the component is returned.
 *
 * @param obj					Pointer to the component object.
 * @param include_height		false: The top y coordinate of the component is returned.
 * 								true: The bottom y coordinate of the component is returned.
 * @return						y-Position of the component.
 */
uint16_t component_get_y(component_t* obj, bool include_height);

/**
 * @brief 	Returns the width of the component.
 *
 * @param obj					Pointer to the component object.
 * @return						Width of the component.
 */
uint16_t component_get_width(component_t* obj);

/**
 * @brief 	Returns the height of the component.
 *
 * @param obj					Pointer to the component object.
 * @return						Height of the component.
 */
uint16_t component_get_height(component_t* obj);

/**
 * @brief Calculates and returns the point where the component should be drawn (top/left), based on the alignment and width/height.
 * 
 * @param obj					Pointer to the component object.
 * @param p						Coordinate for the component on the screen.
 * @return eve_ui_point_t 		Coordinate of the top left where the component should be started to be drawn.
 */
eve_ui_point_t component_get_origin(component_t* obj, eve_ui_point_t p);

/**
 * @brief Calculates and returns an edge coordinate of the component.
 * 
 * @param obj					Pointer to the component object.
 * @param edge					Edge for which the coordinate is needed.
 * @param p						Coordinate for the component on the screen.
 * @return eve_ui_point_t 		Coordinate for the edge of the component
 */
eve_ui_point_t component_get_edge(component_t* obj, COMPONET_EDGE_T edge, eve_ui_point_t p);

/**
 * Returns true if the component needs a cyclic repaint
 * @param obj		Pointer to the component
 * @return			true if the component needs a cyclic repaint
 */
bool component_needs_cyclic_repaint(component_t* obj);

#endif

#endif /* COMPONENT_H_ */
