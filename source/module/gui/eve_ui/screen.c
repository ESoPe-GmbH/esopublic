/***
 * @file screen.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI

#include "screen.h"
#include "mcu/mcu.h"
#include "mcu/sys.h"
#include "module/comm/dbg.h"
#include "../eve/eve.h"
#include "../eve/eve_spi.h"
#include "../eve/eve_copro.h"
#include "../eve/eve_register.h"
#include "font.h"
#include "module/util/assert.h"
#include <string.h>
#include "widgets/video.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static screen_device_t* screen_current_device = NULL;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief	Handle function for the screen is needed for repainting the screen. Does only repaint the screen if it was
 * 			requested via screen_paint or screen_repaint and if the eve co-processor is ready to accept command.
 * 			Asserts also that the screen is maximal repainted every SCREEN_REPAINT_DELAY_MS milliseconds.
 *
 * 			If the currently active screen object has a handle callback function, it is called inside this function.
 */
static void screen_handle(screen_device_t* obj);

/**
 * @brief	Repaints the screen using the currently set screen object.
 * 			Makes use of the dynamically component list by painting every component that was added to the screen object.
 */
static void screen_internal_repaint(screen_device_t* obj);

/**
 * @brief	Initializes the toast object of the screen device.
 *
 * @param obj		Pointer to the screen device object.
 */
static void screen_init_toast(screen_device_t* obj);

/**
 * @brief	Paints the toast object of the screen device if it should be shown.
 *
 * @param obj		Pointer to the screen device object.
 */
static void screen_paint_toast(screen_device_t* obj);

#if SCREEN_ENABLE_FINGER_TRACING
/**
 * @brief 	Paints a circle at the position of the finger.
 *
 * @param obj		Pointer to the screen device object.
 */
static void screen_paint_touch(screen_device_t* obj);
#endif

/**
 * @brief	Calls the eve_copro commands to start the co-processor command list.
 * 			Is called automatically in screen_internal_repaint. The screen is cleared and the background color for the
 * 			clear is set by parameter c.
 *
 * 			eve_copro functions can be used afterwards to draw graphics via co-processor on the screen.
 *
 * @param c			Background color of the screen when it is cleared.
 */
static void screen_start_painting(screen_device_t* obj, color_t c);

/**
 * @brief	Calls the eve_copro commands to finish the co-processor command list and start working with the commands.
 * 			Is called automatically in screen_internal_repaint.
 */
static void screen_finish_painting(screen_device_t* obj);

static void screen_touch_callback(screen_device_t* obj, bool b, int32_t x, int32_t y);

static void screen_key_callback(screen_device_t* obj, char c);

///**
// * @brief	Calls the eve_copro commands to start the display list.
// * 			Is called automatically in screen_internal_repaint. The screen is cleared and the background color for the
// * 			clear is set by parameter c.
// *
// * 			eve_dl commands can be used afterwards to draw graphics directly on the screen.
// *
// * @param c			Background color of the screen when it is cleared.
// */
//void screen_start_painting_gpu(screen_device_t* obj, color_t c);
//
///**
// * @brief	Calls the eve_copro commands to finish the display list and start working with the commands.
// * 			Is called automatically in screen_internal_repaint.
// */
//void screen_finish_painting_gpu(screen_device_t* obj);

/**
 * @brief	Shows a toast on the screen. The toast can have multiple lines. It can be shown at top or at the bottom.
 * 			Lines can be separated with a linefeed in the text string.
 *
 * @param obj				Pointer to the screen object that shall be overlayed with the toast.
 * @param text				Text string that should be shown.
 * @param at_top			true: Toast is shown at the top of the screen.
 * 							false: Toast is shown at the bottom of the screen.
 * @param duration			Duration in milliseconds that the toast is shown.
 * @param show_delayed		Set to true when the toast shall be shown on the next shown screen and not on the currently visible screen.
 * @param f					Pointer to the function that should be called when the toast is touched. If NULL, the toast touch is disabled.
 */
static void screen_show_toast_internal(screen_t* obj, char* text, SCREEN_TOAST_POSITION pos, uint32_t duration, bool sow_delayed, screen_callback_t f);

/**
 * @brief 	Callback function that is called if the eve had an error.
 *
 * @param obj				Pointer to the screen device on which the error occured.
 * @param err				Errorcode of the occured error.
 */
static void screen_device_eve_error(screen_device_t* obj, EVE_ERROR err, const char* msg);

static void _event(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

extern void component_trigger_event(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

screen_device_t* screen_device_create(void)
{
	return mcu_heap_calloc(1, sizeof(struct screen_device_s));
}

void screen_device_free(screen_device_t* dev)
{
	mcu_heap_free(dev);
}

// bool screen_device_init(screen_device_t* obj, eve_hw_interface_t* hw, EVE_DISPLAY_TYPE type, bool rotate, bool has_touch, void(*f_error)(screen_device_t*, EVE_ERROR))
FUNCTION_RETURN screen_device_init(screen_device_t* obj, screen_device_config_t* config)
{
	ASSERT_RET_NOT_NULL(obj, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
	ASSERT_RET_NOT_NULL(config, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

	obj->screen_current_object = NULL;
	obj->screen_last_painted_object = NULL;
	obj->screen_shall_repaint = false;
	obj->screen_first_painted = false;
	obj->screen_paint_timestamp = 0;
	obj->f_error = config->f_error;
	obj->pnl_top = NULL;
	obj->show_pnl_top = false;
	obj->option = 0;
	obj->needs_cyclic_repaint = false;
#if SCREEN_ENABLE_FINGER_TRACING
	obj->touch_timestamp = 0;
#endif
	memcpy(&obj->config, config, sizeof(screen_device_config_t));

	if(eve_init(&obj->eve, &config->hw, config->type, config->rotate, config->has_touch, (void*)obj, (eve_error_cb_t)screen_device_eve_error))
	{
		system_init_object_task(&obj->screen_task, true, (void(*)(void*))screen_handle, (void*)obj);
		eve_copro_set_touch_callback(&obj->eve, (void(*)(void*, bool, int32_t, int32_t))screen_touch_callback, (void*)obj);
		eve_copro_set_key_callback(&obj->eve, (void(*)(void*, char))screen_key_callback, (void*)obj);
		screen_current_device = obj;

		font_initalize_default_fonts(&obj->eve);

		screen_init_toast(obj);

		return FUNCTION_RETURN_OK;
	}
	dbg_printf(DBG_STRING, "EVE not initialized\n");

	return FUNCTION_RETURN_EXECUTION_ERROR;
}

screen_t* screen_device_get_current_screen(screen_device_t* obj)
{
	if(obj == NULL)
		return NULL;

	return obj->screen_current_object;
}

void screen_device_set_error_handler(screen_device_t* obj, screen_device_error_t f_error)
{
	if(obj == NULL)
		return;

	obj->f_error = f_error;
}

void screen_switch_power(screen_device_t* obj, bool b)
{
	if(obj == NULL || obj->eve.is_powered == b)
		return;

	// Add / Remove task for screen handling
	if(b)
	{
		dbg_printf(DBG_STRING, "Power-On Screen\n");
		obj->screen_first_painted = false;
		system_add_task(&obj->screen_task);
	}
	else
	{
		dbg_printf(DBG_STRING, "Power-Off Screen\n");
		system_remove_task(&obj->screen_task);
	}

	// Switch Power of display itself
	eve_switch_power(&obj->eve, b);
}

void screen_device_on(screen_device_t* obj)
{
	uint8_t gpio_value;

	if(obj == NULL)
		return;

	gpio_value = eve_spi_read_8(&obj->eve, EVE_REG_GPIO);
	gpio_value |= 0x80;
	eve_spi_write_8(&obj->eve, EVE_REG_GPIO, gpio_value);
}

void screen_device_off(screen_device_t* obj)
{
	uint8_t gpio_value;

	if(obj == NULL)
		return;

	gpio_value = eve_spi_read_8(&obj->eve, EVE_REG_GPIO);
	gpio_value &= ~0x80;
	eve_spi_write_8(&obj->eve, EVE_REG_GPIO, gpio_value);
}

void screen_device_set_dimming(screen_device_t* obj, uint8_t percent)
{
	if(obj == NULL)
		return;

	uint8_t value;

	if(obj->config.has_inverted_dimming)
	{
		value = (100 - (uint32_t)percent) * 0x80 / 100;
	}
	else
	{
		value = (uint32_t)percent * 0x80 / 100;
	}
	eve_spi_write_8(&obj->eve, EVE_REG_PWM_DUTY, value);
}

uint16_t screen_device_get_width(screen_device_t* obj)
{
	if(obj == NULL)
		return 0;

	return obj->eve.eve_display_width;
}

uint16_t screen_device_get_height(screen_device_t* obj)
{
	if(obj == NULL)
		return 0;

	return obj->eve.eve_display_height;
}

void screen_set_screen_device(screen_device_t* obj)
{
	screen_current_device = obj;
}

screen_device_t* screen_get_default_device(void)
{
	return screen_current_device;
}

void screen_device_recalibrate_touch(screen_device_t* obj)
{
	if(obj == NULL)
		return;

	eve_init_touch(&obj->eve);
}

void screen_init_object(screen_t* obj, color_t c, screen_callback_t handle_callback, screen_callback_t pre_paint_callback)
{
	if(obj == NULL)
		return;

//	dbg_printf(DBG_STRING, "screen_init_object(%08x)\n", obj);

	component_init((component_t*)obj, COMPONENT_TYPE_SCREEN, NULL);
	obj->component.f_cb_event_internal = _event;

	obj->screen_device = screen_current_device;

	obj->component.size.width = screen_device_get_width(obj->screen_device);
	obj->component.size.height = screen_device_get_height(obj->screen_device);
#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	obj->focused_component = NULL;
	obj->default_focused = NULL;
#endif
	obj->show_pnl_top = true;

	obj->last_component = &obj->component;
	obj->color_background = c;
	obj->handle_callback = handle_callback;
	obj->pre_paint_callback = pre_paint_callback;
}

void screen_add_component(screen_t* obj, component_t* comp)
{
	if(obj == NULL || comp == NULL)
		return;

	obj->last_component->next_component = comp;
	obj->last_component = comp;

	// Set screen as parent for the component
	comp->parent_component = &obj->component;

//	dbg_printf(DBG_STRING, "Added %08x[T=%d] to %08x\n", comp, comp->type, obj);
}

void screen_set_touch_event(screen_t* obj, void(*f)(screen_t*, int32_t, int32_t))
{
	if(obj == NULL)
		return;

	obj->touch_callback = f;
}

void screen_set_key_listener(screen_t* obj, void(*f)(screen_t*, char))
{
	if (obj == NULL)
		return;

	obj->key_callback = (void (*)(void*, char))f;
}

uint16_t screen_get_width(screen_t* obj)
{
	if(obj == NULL)
		return 0;

	return obj->component.size.width;
}

uint16_t screen_get_height(screen_t* obj)
{
	if(obj == NULL)
		return 0;

	return obj->component.size.height;
}

eve_t* screen_get_eve(screen_t* obj)
{
	if(obj == NULL || obj->screen_device == NULL)
		return NULL;

	return &obj->screen_device->eve;
}

screen_t* screen_get_from_component(component_t* obj)
{
	if(obj == NULL)
		return NULL;

	do
	{
		if(obj->type == COMPONENT_TYPE_SCREEN)
			return (screen_t*)obj;

		obj = (component_t*)obj->parent_component;
	}
	while(obj != NULL);

	return NULL;
}

void screen_paint(screen_t* obj, uint32_t option)
{
	if(obj == NULL)
		return;

	if(obj != obj->screen_device->screen_current_object || option != obj->screen_device->option)
		obj->screen_device->screen_last_painted_object = NULL;

	obj->screen_device->option = option;

	obj->screen_device->screen_current_object = obj;

	obj->screen_device->screen_shall_repaint = true;

#if SCREEN_ENABLE_FOCUSED_COMPONENTS
	if(obj->screen_device->screen_last_painted_object == NULL)
		obj->focused_component = NULL;
#endif

	// If the screen was never painted before call the handle function to paint the screen immediately.
	if(!obj->screen_device->screen_first_painted)
		screen_handle(obj->screen_device);
}

void screen_repaint_by_component(component_t* obj)
{
	screen_t* screen = screen_get_from_component(obj);

	if(screen)
	{
		screen->screen_device->screen_shall_repaint = true;
	}
	else
	{
		DBG_ERROR("Cannot find screen\n");
	}
}

void screen_repaint(screen_device_t* device)
{
	device->screen_shall_repaint = true;
}

void screen_repaint_changed(screen_t* obj)
{
	if(obj == NULL)
		return;

	obj->screen_device->screen_last_painted_object = NULL;
	obj->screen_device->screen_shall_repaint = true;
}

#if SCREEN_ENABLE_FOCUSED_COMPONENTS
void screen_set_focus_set_default(screen_t* obj, component_t* comp_focus)
{
	component_t* comp = &obj->component;
	do
	{
		comp = comp->next_component; // Next component

		if(comp == comp_focus)
		{
			obj->default_focused = comp_focus;
			return;
		}
	}
	while(comp != NULL);
}

void screen_focus_next_component(screen_t* obj)
{
	component_t* comp_start = obj->focused_component;
	component_t* comp = obj->focused_component;

//	dbg_printf(DBG_STRING, "screen_focus_next_component(%08x)\n", obj);

	if(obj == NULL || obj->component.next_component == NULL || obj->focused_component == NULL)
		return;

	do
	{
		if(comp->next_component == NULL)
			comp = obj->component.next_component; // First component
		else
			comp = comp->next_component;

		if(comp->is_focusable && comp->is_visible && comp->is_enabled)
		{
			obj->focused_component = comp;
			screen_repaint(obj->screen_device);
			return;
		}
	}
	while(comp != comp_start);
}

void screen_focus_previous_component(screen_t* obj)
{
	component_t* comp = obj->component.next_component;
	component_t* comptmp = NULL;

//	dbg_printf(DBG_STRING, "screen_focus_previous_component(%08x)\n", obj);

	if(obj == NULL || obj->component.next_component == NULL || obj->focused_component == NULL)
		return;

	while(comp != NULL)
	{
		if(comp == obj->focused_component)
		{
			// if comptmp != NULL -> comptmp contains pointer to focusable component BEFORE the currently focused!
			// Set it and return
			if(comptmp != NULL)
			{
				obj->focused_component = comptmp;
				screen_repaint(obj->screen_device);
				return;
			}
		}

		if(comp->is_focusable && comp->is_visible && comp->is_enabled)
		{
			if(comp != obj->focused_component)
				comptmp = comp;
		}
		comp = comp->next_component;
	}

	// if comptmp != NULL -> comptmp contains pointer to last focusable component on the screen
	// This is needed when first focusable component is the current one -> In this case the previous while searches for the last one
	if(comptmp != NULL)
	{
		obj->focused_component = comptmp;
		screen_repaint(obj->screen_device);
		return;
	}
}

void screen_focus_set(screen_t* obj, component_t* comp)
{
	if(obj == NULL)
		return;

	obj->focused_component = comp;
	screen_repaint(obj->screen_device);
}

component_t* screen_get_focused_component(screen_t* obj)
{
	if(obj == NULL)
		return NULL;

	return obj->focused_component;
}
#endif //SCREEN_ENABLE_FOCUSED_COMPONENTS

void screen_show_toast(screen_t* obj, char* text, SCREEN_TOAST_POSITION pos, uint32_t duration)
{
	screen_show_toast_internal(obj, text, pos, duration, false, NULL);
}

void screen_show_toast_permanent(screen_t* obj, char* text, SCREEN_TOAST_POSITION pos, screen_callback_t f)
{
	screen_show_toast_internal(obj, text, pos, 0, false, f);
}

void screen_show_toast_delayed(screen_t* obj, char* text, SCREEN_TOAST_POSITION pos, uint32_t duration)
{
	screen_show_toast_internal(obj, text, pos, duration, true, NULL);
}

void screen_hide_toast(screen_device_t* obj) {
	obj->toast.is_shown = false;
	screen_repaint(obj);
	// Call the callback and set it to NULL
	if (obj->toast.f_callback)
		obj->toast.f_callback(obj->toast.callback_obj, obj->option);

	obj->toast.f_callback = NULL;
	obj->toast.is_permanent = false;
}

void screen_reset_first_painted(screen_device_t* obj)
{
	obj->screen_first_painted = false;
}

void screen_top_panel_register(screen_device_t* dev, panel_t* pnl)
{
	if(dev == NULL)
		return;

	dev->pnl_top = pnl;
	dev->show_pnl_top = (pnl != NULL);

	screen_repaint(dev);
}

void screen_top_panel_set_visible(screen_device_t* obj, bool b)
{
	if(obj == NULL)
		return;

	obj->show_pnl_top = b;

	screen_repaint(obj);
}

#if SCREEN_ENABLE_FOCUSED_COMPONENTS
void screen_add_key(screen_device_t* obj, uint8_t scancode, char c)
{
	component_t* comp;
	if(obj == NULL)
		return;

	if(obj->screen_current_object == NULL)
		return;

	comp = ((screen_t*)obj->screen_current_object)->focused_component;
	if(comp == NULL || comp->keyboard_callback == NULL)
		return;

	comp->keyboard_callback(comp, scancode, c);
}
#endif
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void screen_handle(screen_device_t* obj)
{
	if(obj == NULL || !obj->eve.eve_is_initialized)
		return;

	eve_copro_handle(&obj->eve);

	if(obj->screen_current_object == NULL)
		return;

	// If toast is shown and it is not set as permanent, hide the toast after a specified time.
	if(obj->toast.is_shown && !obj->toast.is_permanent)
	{
		if( (system_get_tick_count() - obj->toast.timestamp_start) >= obj->toast.duration )
		{
//			dbg_printf(DBG_STRING, "Hide Toast\n");
			obj->toast.is_shown = false;
			obj->screen_shall_repaint = true;
		}
	}

	if(obj->needs_cyclic_repaint && ((screen_t*)obj->screen_current_object) != NULL && (system_get_tick_count() - obj->screen_paint_timestamp) >= SCREEN_REPAINT_CYCLIC_MS)
		obj->screen_shall_repaint = true;

	if(((screen_t*)obj->screen_last_painted_object) == ((screen_t*)obj->screen_current_object) && ((screen_t*)obj->screen_last_painted_object) != NULL && ((screen_t*)obj->screen_current_object)->handle_callback != NULL)
	{
		((screen_t*)obj->screen_current_object)->handle_callback(obj->screen_current_object, obj->option);
		
		component_event_t event_data = 
		{
			.device = obj,
			.screen = obj->screen_current_object
		};
		component_trigger_event(&obj->screen_current_object->component, COMPONENT_EVENT_HANDLE, &event_data);
	}

	// If the screen was painted at least once and the screen must not be forced to repaint...
	if(obj->screen_first_painted && !obj->eve.force_repaint)
	{
		// Return if the screen shall not be repainted
		if(!obj->screen_shall_repaint)
			return;

		// Do not repaint if eve component is not ready!
		// This is necessary for example if a video is played in fullscreen using playvideo command, because this only terminates the coprocessor when video is finished
		if(!eve_copro_is_ready(&obj->eve))
			return;

#if SCREEN_REPAINT_DELAY_MS
		// Do not repaint if the last repaint if the last repaint was in the last SCREEN_REPAINT_DELAY_MS milliseconds.
		if((system_get_tick_count() - obj->screen_paint_timestamp) < SCREEN_REPAINT_DELAY_MS)
			return;
#endif
	}
	// Screen was not painted before or this is a forced repaint!
	else
	{
		obj->screen_first_painted = true;
	}

	obj->screen_paint_timestamp = system_get_tick_count();
	obj->screen_shall_repaint = false;
	screen_internal_repaint(obj);
}

static void screen_internal_repaint(screen_device_t* obj)
{
	component_t* comp;

	if(obj == NULL || obj->screen_current_object == NULL)
		return;

	if(obj->screen_last_painted_object != obj->screen_current_object)
	{
		if(obj->toast.show_delayed)
		{
			obj->toast.show_delayed = false;
			obj->toast.is_shown = true; // Show toast when the screen is shown.
			obj->toast.timestamp_start = system_get_tick_count();
		}
		else
		{
			// If the toast is not shown permanently, it is hidden when the screen changes.
			if(!obj->toast.is_permanent)
				obj->toast.is_shown = false; // Hide toast when the screen is shown.
		}

//		obj->screen_ram_pointer = 0; // -> Do not set to 0 because we have no memory management here
		obj->screen_last_painted_object = obj->screen_current_object;
		eve_copro_touch_released(&obj->eve);
		eve_copro_set_screen_changed(&obj->eve);

		component_event_t event_data = 
		{
			.device = obj,
			.screen = obj->screen_current_object
		};
		
		component_trigger_event(&obj->screen_last_painted_object->component, COMPONENT_EVENT_UNLOAD, &event_data);
		component_trigger_event(&obj->screen_last_painted_object->component, COMPONENT_EVENT_LOAD, &event_data);

		if(((screen_t*)obj->screen_current_object)->pre_paint_callback != NULL)
			((screen_t*)obj->screen_current_object)->pre_paint_callback(obj->screen_current_object, obj->option);

		if(obj->screen_last_painted_object != obj->screen_current_object)
		{
			// The screen changed inside the pre-paint function -> recursive call and
			screen_internal_repaint(obj);
			return;
		}

#if SCREEN_ENABLE_FOCUSED_COMPONENTS
		if(((screen_t*)obj->screen_current_object)->focused_component == NULL)
		{
			comp = ((screen_t*)obj->screen_current_object)->default_focused;
			if(comp != NULL && comp->is_focusable && comp->is_visible && comp->is_enabled)
			{
				((screen_t*)obj->screen_current_object)->focused_component = ((screen_t*)obj->screen_current_object)->default_focused;
			}
			else
			{
				((screen_t*)obj->screen_current_object)->focused_component = NULL;
				comp = &((screen_t*)obj->screen_current_object)->component;
				while(comp != NULL)
				{
					if(comp->is_focusable && comp->is_visible && comp->is_enabled)
					{
						((screen_t*)obj->screen_current_object)->focused_component = comp;
						break;
					}
					comp = comp->next_component;
				}
			}
		}
#endif // SCREEN_ENABLE_FOCUSED_COMPONENTS
	}

	obj->needs_cyclic_repaint = false;
	comp = &((screen_t*)obj->screen_current_object)->component;
	screen_start_painting(obj, ((screen_t*)obj->screen_current_object)->color_background);

	obj->component_video = NULL;

	while(comp != NULL)
	{
		if(comp->type == COMPONENT_TYPE_VIDEO && comp->is_visible)
		{
			obj->component_video = comp;
			comp = comp->next_component;
			continue;
		}

		if(comp->paint_function != NULL && comp->is_visible)
		{
//			dbg_printf(DBG_STRING, "Paint %d\n", comp->type);
			comp->paint_function(comp, (eve_ui_point_t){0}); // Screen uses 0 offset for x and y.
			obj->needs_cyclic_repaint |= component_needs_cyclic_repaint(comp);

#if SCREEN_ENABLE_FOCUSED_COMPONENTS
			if(comp == ((screen_t*)obj->screen_current_object)->focused_component)
			{
				component_t* c = ((screen_t*)obj->screen_current_object)->focused_component;
				// Set color
				eve_copro_set_color(&obj->eve, color_get(GUI_CONFIG_FOCUS_BORDER_COLOR));
				eve_copro_write_command(&obj->eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
				// Set line width
				eve_copro_write_command(&obj->eve, EVE_LINE_WIDTH(16 * 1));
				eve_copro_write_command(&obj->eve, EVE_BEGIN(EVE_LINES));
				eve_copro_write_command(&obj->eve, EVE_VERTEX2F(c->x, c->y-1));
				eve_copro_write_command(&obj->eve, EVE_VERTEX2F(c->x + c->width - 1, c->y-1));

				eve_copro_write_command(&obj->eve, EVE_VERTEX2F(c->x + c->width, c->y-1));
				eve_copro_write_command(&obj->eve, EVE_VERTEX2F(c->x + c->width, c->y + c->height));

				eve_copro_write_command(&obj->eve, EVE_VERTEX2F(c->x + c->width, c->y + c->height+1));
				eve_copro_write_command(&obj->eve, EVE_VERTEX2F(c->x, c->y + c->height+1));

				eve_copro_write_command(&obj->eve, EVE_VERTEX2F(c->x-1, c->y + c->height));
				eve_copro_write_command(&obj->eve, EVE_VERTEX2F(c->x-1, c->y));
				eve_copro_write_command(&obj->eve, EVE_END());
			}
#endif // SCREEN_ENABLE_FOCUSED_COMPONENTS
		}
		comp = comp->next_component;
	}

	// Check to paint the top panel
	if(obj->show_pnl_top && obj->pnl_top != NULL && ((screen_t*)obj->screen_current_object)->show_pnl_top)
	{
		obj->pnl_top->component.parent_component = &obj->screen_current_object->component;
		obj->pnl_top->component.paint_function(&obj->pnl_top->component, (eve_ui_point_t){0});
	}

	screen_paint_toast(obj);

#if SCREEN_ENABLE_FINGER_TRACING
	screen_paint_touch(obj);
#endif

	if(obj->component_video)
	{
		video_t* v = __containerof(obj->component_video, video_t, component);
		
		if(v->options.fullscreen)
		{
			// Finish painting the screen before painting the video.
			screen_finish_painting(obj);
		}
		
		if(obj->component_video->paint_function)
		{
			obj->component_video->paint_function(obj->component_video, (eve_ui_point_t){0});
		}

		if(!v->options.fullscreen)
		{
			// Finish painting the screen after painting the video.
			screen_finish_painting(obj);
		}

		obj->component_video = NULL;
	}
	else
	{
		screen_finish_painting(obj);
	}


	if(obj->eve.force_repaint)
	{
		obj->eve.force_repaint = false;
		// Since it is a forces repaint, switch the device on
		screen_device_on(obj);
	}
}

static void screen_init_toast(screen_device_t* obj)
{
	panel_init(&obj->toast.panel, 10, 10, screen_device_get_width(obj) - 20, 1, true); // Height is set based on text
	rect_init(&obj->toast.rect_inner, 5, 5, obj->toast.panel.component.size.width - 10, 1, 5); // Height is set based on text

	rect_set_line_width(&obj->toast.panel.rect_bg, 5);
	rect_set_color(&obj->toast.panel.rect_bg, color_get_argb(TOAST_DEFAULT_BACKGROUND_ALPHA, TOAST_DEFAULT_BORDER_COLOR));
	rect_set_color(&obj->toast.rect_inner, color_get_argb(TOAST_DEFAULT_BACKGROUND_ALPHA, TOAST_DEFAULT_BACKGROUND_COLOR));

	panel_add_component(&obj->toast.panel, (component_t*)&obj->toast.rect_inner);

	text_init(&obj->toast.text, 10, 5, "");
	text_set_color(&obj->toast.text, color_get(TOAST_DEFAULT_TEXT_COLOR));
	text_set_font(&obj->toast.text, TOAST_DEFAULT_FONT);
	panel_add_component(&obj->toast.panel, (component_t*)&obj->toast.text);
}

static void screen_paint_toast(screen_device_t* obj)
{
	if(!obj->toast.is_shown)
		return;

//	obj->toast.panel.component.size.height = 15 + ((uint16_t)obj->toast.line_count * 20);
//	obj->toast.panel.rect_bg.component.size.height = 15 + ((uint16_t)obj->toast.line_count * 20);
	obj->toast.rect_inner.component.size.height = obj->toast.panel.component.size.height - 10;

	if(obj->toast.position == SCREEN_TOAST_POSITION_TOP)
		obj->toast.panel.component.origin.y = 10;
	else if(obj->toast.position == SCREEN_TOAST_POSITION_CENTER)
		obj->toast.panel.component.origin.y = screen_device_get_height(obj) / 2 - obj->toast.panel.component.size.height / 2;
	else
		obj->toast.panel.component.origin.y = screen_device_get_height(obj) - obj->toast.panel.component.size.height - 10;

	obj->toast.panel.component.parent_component = &obj->screen_current_object->component;

//	eve_copro_set_color_alpha(&obj->eve, SCREEN_TOAST_ALPHA);
	obj->toast.panel.component.paint_function(&obj->toast.panel.component, (eve_ui_point_t){0});
//	eve_copro_set_color_alpha(&obj->eve, 0xFF);
}

#if SCREEN_ENABLE_FINGER_TRACING
static void screen_paint_touch(screen_device_t* obj)
{
	eve_t* eve = &obj->eve;
	uint16_t size = 5;

	if( (system_get_tick_count() - obj->touch_timestamp) > 1000 )
		return;


	eve_copro_write_command(eve, EVE_LINE_WIDTH(16 * size));

	eve_copro_set_color(eve, color_get(COLOR_HOT_PINK));
	// Draw shadow rectangle
	eve_copro_write_command(eve, EVE_BEGIN(EVE_RECTS));					// Start rectangle
	eve_copro_write_command(eve, EVE_VERTEX2F(16 * (obj->x), 16 * (obj->y)));	// Start point of rectangle
	eve_copro_write_command(eve, EVE_VERTEX2F(16 * (obj->x), 16 * (obj->y)));	// End point of rectangle
	eve_copro_write_command(eve, EVE_END());
}
#endif

static void screen_start_painting(screen_device_t* obj, color_t c)
{
	if(obj == NULL)
		return;

	eve_copro_dlstart(&obj->eve, c);
	eve_copro_coldstart(&obj->eve);
}

static void screen_finish_painting(screen_device_t* obj)
{
	if(obj == NULL)
		return;

	eve_copro_write_command(&obj->eve, EVE_DISPLAY());
	eve_copro_swap(&obj->eve);
}

static void screen_touch_callback(screen_device_t* obj, bool b, int32_t x, int32_t y)
{
	if(obj == NULL || !b)
		return;

#if SCREEN_ENABLE_FINGER_TRACING
	obj->x = x;
	obj->y = y;
	obj->touch_timestamp = system_get_tick_count();
#endif

//	dbg_printf(DBG_STRING, "Screen touch callback (%d) %d / %d\n", b, x, y);
	if(obj->toast.is_shown)
	{
		if(x >= obj->toast.panel.component.origin.x
				&& x <= (obj->toast.panel.component.origin.x + obj->toast.panel.component.size.width)
				&& y >= obj->toast.panel.component.origin.y
				&& y <= (obj->toast.panel.component.origin.y + obj->toast.panel.component.size.height)
				)
		{
			screen_hide_toast(obj);
		}
	}

	if(obj->screen_current_object && obj->screen_current_object->touch_callback)
	{
		obj->screen_current_object->touch_callback(obj->screen_current_object, x, y);
	}
}

static void screen_key_callback(screen_device_t* obj, char c)
{
	if(obj == NULL)
		return;

//	dbg_printf(DBG_STRING, "Screen touch callback (%d) %d / %d\n", b, x, y);

	if(obj->screen_current_object != NULL && ((screen_t*)obj->screen_current_object)->key_callback != NULL)
	{
//		dbg_printf(DBG_STRING, "Screen touch callback\n", b);
		((screen_t*)obj->screen_current_object)->key_callback(obj->screen_current_object, c);
	}
}

//void screen_start_painting_gpu(screen_device_t* obj, color_t c)
//{
//	if(obj == NULL)
//		return;
//
//	eve_dl_start(&obj->eve);
//	eve_dl(&obj->eve, EVE_CLEAR_COLOR_RGB(c.r, c.g, c.b));
//	eve_dl(&obj->eve, EVE_CLEAR(1,1,1));
//}
//
//void screen_finish_painting_gpu(screen_device_t* obj)
//{
//	if(obj == NULL)
//		return;
//
//	eve_dl(&obj->eve, EVE_DISPLAY());
//	eve_dl_swap(&obj->eve);
//}

static void screen_show_toast_internal(screen_t* obj, char* text, SCREEN_TOAST_POSITION pos, uint32_t duration, bool show_delayed, screen_callback_t f)
{
	uint8_t i;
	uint32_t f_h;
	uint8_t linecount = 1;

	if(obj == NULL || text == NULL || text[0] == '\0')
		return;

	obj->screen_device->toast.f_callback = f;
	obj->screen_device->toast.is_permanent = (duration == 0);
	obj->screen_device->toast.callback_obj = obj;
	obj->screen_device->toast.position = pos;
	obj->screen_device->toast.duration = duration;
	obj->screen_device->toast.show_delayed = show_delayed;
	obj->screen_device->toast.is_shown = !show_delayed;
	obj->screen_device->toast.timestamp_start = system_get_tick_count();

	for(i = 0; text[i]; i++)
	{
		if(text[i] == '\n')
			linecount++;
	}

	if(i > 0 && text[i-1] == '\n')
		linecount--;

	text_set_text(&obj->screen_device->toast.text, text);

	f_h = font_get_height(&obj->screen_device->eve, TOAST_DEFAULT_FONT);

	obj->screen_device->toast.panel.component.size.height = 10 + (f_h * linecount);
	obj->screen_device->toast.panel.rect_bg.component.size.height = 10 + (linecount * f_h);


	screen_repaint(obj->screen_device);
}

static void screen_device_eve_error(screen_device_t* obj, EVE_ERROR err, const char* msg)
{
	obj->screen_shall_repaint = true;

	switch(err)
	{
		case EVE_ERROR_REINITIALIZATION_FAILED:
			system_remove_task(&obj->screen_task); // EVE stopped working, so stop the screen task!
		break;

		default:
			break;
	}

	if(obj->f_error != NULL)
		obj->f_error(obj, err, msg);
	else
		dbg_printf(DBG_STRING, "Screen Device %08xh Error %04x: %s\n", (uint32_t)obj, (uint32_t)err, msg);
}

static void _event(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data)
{
	component_t* comp = c->next_component;
	
	while(comp)
	{
		component_trigger_event(comp, event, data);
		comp = comp->next_component;
	}
}

#endif
