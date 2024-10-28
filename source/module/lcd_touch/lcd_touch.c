/**
 * @file touch.c
 **/

#include "lcd_touch.h"

#if MODULE_ENABLE_LCD_TOUCH

#include "module/comm/dbg.h"
#include "module/convert/convert.h"
#include "module/util/assert.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct lcd_touch_s
{
    /// Interface of the touch device.
    const struct lcd_touch_interface_s* interface;
    /// @brief Configuration of the touch
    struct lcd_touch_config_s config;
    /// @brief Handle of the touch device
    lcd_touch_device_handle_t device;
    /// @brief Is set and cleared whenever fingers are removed or touched.
    bool is_touched;
    /// @brief Observers that get notified when a finger touches or is released.
    list_t observer;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Call to trigger notifications to observers
 * 
 * @param h         Touch device handle
 * @param event     Pointer to the event containing the touched fingers. @c user_ctx is changed based on the observer internally-
 */
static void _notify_observer(lcd_touch_handle_t h, lcd_touch_observer_event_t* event);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN_T lcd_touch_create(lcd_touch_device_handle_t device, const struct lcd_touch_interface_s* interface, const struct lcd_touch_config_s* config, lcd_touch_handle_t* handle)
{
    ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(interface, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(config, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(handle, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    *handle = mcu_heap_calloc(1, sizeof(struct lcd_touch_s));
    ASSERT_RET_NOT_NULL(*handle, NO_ACTION, FUNCTION_RETURN_INSUFFICIENT_MEMORY);

    (*handle)->interface = interface;

    memcpy(&(*handle)->config, config, sizeof(struct lcd_touch_config_s));
    (*handle)->device = device;

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T lcd_touch_free(lcd_touch_handle_t* handle)
{
    ASSERT_RET_NOT_NULL(handle, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    mcu_heap_free(*handle);

    *handle = NULL;

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T lcd_touch_add_observer(lcd_touch_handle_t h, const lcd_touch_observer_config_t* config)
{
    ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    lcd_touch_observer_config_t* c = mcu_heap_calloc(1, sizeof(lcd_touch_observer_config_t));

    ASSERT_RET_NOT_NULL(c, NO_ACTION, FUNCTION_RETURN_INSUFFICIENT_MEMORY);

    memcpy(c, config, sizeof(lcd_touch_observer_config_t));
    
    list_add_element(&h->observer, c);

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T lcd_touch_remove_observer(lcd_touch_handle_t h, const lcd_touch_observer_config_t* config)
{
    ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(config, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    
    lcd_touch_observer_config_t* e = list_first_element(&h->observer);
    while(e)
    {
        // Remove this exact config
        // if(e == config)
        // {
        //     list_remove_element(&h->observer, e);
        //     return FUNCTION_RETURN_OK;
        // }
        // else 
        if(config->user_ctx) // Remove this exact user context
        {
            // Remove only if user context is the same and either no callback was specified or the callback matches
            if(config->user_ctx == e->user_ctx && (config->f_cb == NULL || (config->f_cb == e->f_cb)) )
            {
                list_remove_element(&h->observer, e);
                return FUNCTION_RETURN_OK;
            }
        }
        else if(config->f_cb && config->f_cb == e->f_cb) // Remove based on the function
        {
            list_remove_element(&h->observer, e);
            return FUNCTION_RETURN_OK;
        }
        
        e = list_next_element(&h->observer, e);
    }

    return FUNCTION_RETURN_NOT_FOUND;
}

FUNCTION_RETURN_T lcd_touch_enter_sleep(lcd_touch_handle_t h)
{
    ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    if(h->interface->enter_sleep)
    {
        return h->interface->enter_sleep(h->device);
    }

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T lcd_touch_exit_sleep(lcd_touch_handle_t h)
{
    ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    if(h->interface->exit_sleep)
    {
        return h->interface->exit_sleep(h->device);
    }

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T lcd_touch_read_data(lcd_touch_handle_t h)
{
    ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    if(h->interface->read_data)
    {
        return h->interface->read_data(h->device);
    }

    return FUNCTION_RETURN_OK;
}

bool lcd_touch_get_xy(lcd_touch_handle_t h, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    ASSERT_RET_NOT_NULL(h, NO_ACTION, false);

    if(h->interface->get_xy)
    {
        bool touched = h->interface->get_xy(h->device, x, y, strength, point_num, max_point_num);

        if(!touched)
        {
            *point_num = 0;
            lcd_touch_observer_event_t event = {0};
            _notify_observer(h, &event);
            return false;
        }
        
        if(h->config.process_xy)
        {
            touched = h->config.process_xy(h, x, y, strength, point_num, max_point_num);
        }

        if(!touched)
        {
            *point_num = 0;
            lcd_touch_observer_event_t event = {0};
            _notify_observer(h, &event);
            return false;
        }

        if(h->config.flags.mirror_x || h->config.flags.mirror_y || h->config.flags.swap_xy)
        {
            for(int i = 0; i < *point_num; i++)
            {
                if(h->config.flags.mirror_x)
                {
                    x[i] = h->config.x_max - x[i];
                }

                if(h->config.flags.mirror_y)
                {
                    y[i] = h->config.y_max - y[i];
                }

                if(h->config.flags.swap_xy)
                {
                    uint16_t tmp = x[i];
                    x[i] = y[i];
                    y[i] = tmp;
                }
            }
        }

        lcd_touch_observer_event_t event = {
            .point_num = *point_num,
            .strength = strength,
            .x = x,
            .y = y
        };
        _notify_observer(h, &event);
        
        // for(int i = 0; i < *point_num; i++)
        // {
        //     DBG_INFO("[%d] x=%d y=%d\n", i, x[i], y[i]);   
        // }

        return true;
    }

    return false;
}

FUNCTION_RETURN_T lcd_touch_set_flags(lcd_touch_handle_t h, struct lcd_touch_flags_s flags)
{
    ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    h->config.flags = flags;

    if(h->interface->set_flags)
    {
        return h->interface->set_flags(h->device, flags);
    }

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T lcd_touch_get_flags(lcd_touch_handle_t h, struct lcd_touch_flags_s* flags)
{
    ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    *flags = h->config.flags;

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T lcd_touch_del(lcd_touch_handle_t h)
{
    ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    if(h->interface->del)
    {
        return h->interface->del(h->device);
    }

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T lcd_get_dimensions(lcd_touch_handle_t h, uint16_t* x_max, uint16_t* y_max)
{
    ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(x_max, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(y_max, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    *x_max = h->config.x_max;
    *y_max = h->config.y_max;

    return FUNCTION_RETURN_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _notify_observer(lcd_touch_handle_t h, lcd_touch_observer_event_t* event)
{
    bool touch_changed = (event->point_num > 0) == h->is_touched;
    h->is_touched = (event->point_num > 0);

    if(!h->is_touched && !touch_changed)
    {
        // Do not keep track on untouched display.
        return;
    }

    lcd_touch_observer_config_t* e = list_first_element(&h->observer);

    while(e)
    {
        // Notify observer when it is configured to track the fingers or if the touch is pressed/released.
        if(e->f_cb && (e->track_finger || touch_changed))
        {
            event->user_ctx = e->user_ctx;
            e->f_cb(h, event);
        }
        
        e = list_next_element(&h->observer, e);
    }
}

#endif
