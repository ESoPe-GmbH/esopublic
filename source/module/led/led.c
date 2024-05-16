// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file led.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "led.h"
#if MODULE_ENABLE_LED
#include "mcu/sys.h"
#include "module/list/list.h"

#if MODULE_ENABLE_DEBUG_CONSOLE
#include "module/console/console.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Handles the blinking of the led.
 *
 * @param s		Pointer to the led struct to handle
 */
static void _handle(led_t *s);

#if MODULE_ENABLE_DEBUG_CONSOLE
/**
 * Console handle for debugging LEDs.
 * @param data			Pointer to the console's data.
 * @param args			Pointer to the argument's list.
 * @param args_len		Number of elements in the argument's list.
 * @return				FUNCTION_RETURN_OK if argument is valid, other value otherwise.
 */
static FUNCTION_RETURN _console(console_data_t* data, char** args, uint8_t args_len);
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// List of initialized LED's.
static list_t _list = {0};
/// Is cleared when led_register is called for the first time. Is used to ensure the list is initialized only once and the led console is only added once.
static bool _first_register = true;
/// Structure for the console command
static console_command_t _cmd = {
		.command = "led",
		.fnc_exec = _console,
		.use_array_param = true,
		.explanation = "Control the LED"
};


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void led_init(led_t *s, MCU_IO_PIN pin, bool is_inverted)
{
	// Initialize structure
	s->pin.pin = pin;
	s->pin.is_inverted = is_inverted;
	s->timestamp = 0;
	s->blinking_interval_ms = 0;
	s->is_on = false;
	s->pulsing_interval_ms = 0;

	// Initialize pin
	mcu_io_set_dir(s->pin.pin, MCU_IO_DIR_OUT);
	MCU_IO_SET_HANDLER(s->pin, MCU_IO_LOW);

	// Initialize task
	system_init_object_task(&s->task, false, (void(*)(void*))_handle, s);
}

FUNCTION_RETURN led_register(led_t* s, const char* name)
{
	if(_first_register)
	{
		_first_register = false;
		list_init(&_list, s, &s->next);
		console_add_command(&_cmd);
	}
	s->task.name = name;
	return list_add(&_list, s);
}

led_t* led_get_pointer(const char* name)
{
	if(name == NULL)
		return NULL;
	led_t* ptr = list_get_first(&_list);
	while(ptr)
	{
		if(name != NULL && strcmp(name, ptr->task.name) == 0)
			return ptr;
		ptr = list_get_next(&_list, ptr);
	}
	return NULL;
}

void led_set_blinking_ms(led_t *s, uint32_t interval)
{
	s->blinking_interval_ms = interval;
    s->pulsing_interval_ms = 0;

	if(s->blinking_interval_ms > 0)
		system_add_task(&s->task);
	else
		system_remove_task(&s->task);
}

void led_set_blinking_hz(led_t *s, uint32_t interval)
{
	if(interval > 1000)
		interval = 1000;

	if(interval > 0) // T[s] = 1 / F[Hz] -> T[ms] = 1000 / F[Hz] -> T[ms] is complete cycle duration, toggle every half of it.
		s->blinking_interval_ms = (1000 / interval) / 2;
	else
		s->blinking_interval_ms = 0;
    s->pulsing_interval_ms = 0;

	if(s->blinking_interval_ms > 0)
		system_add_task(&s->task);
	else
		system_remove_task(&s->task);
}

void led_set_pulsing_ms(led_t *s, uint32_t on_time, uint32_t off_time)
{
    s->blinking_interval_ms = off_time;
    s->pulsing_interval_ms = on_time;

    if(s->blinking_interval_ms > 0 && s->pulsing_interval_ms > 0)
        system_add_task(&s->task);
    else
        system_remove_task(&s->task);
}

void led_set(led_t *s, bool switch_on)
{
	MCU_IO_SET_HANDLER(s->pin, switch_on);
	s->is_on = switch_on;
	system_remove_task(&s->task);
}

void led_toggle(led_t *s)
{
    s->is_on ^= 1;
    MCU_IO_SET_HANDLER(s->pin, s->is_on);
	system_remove_task(&s->task);
}

void led_on(led_t *s)
{
    s->is_on = true;
	MCU_IO_SET_HANDLER(s->pin, MCU_IO_HIGH);
	system_remove_task(&s->task);
}

void led_off(led_t *s)
{
    s->is_on = false;
	MCU_IO_SET_HANDLER(s->pin, MCU_IO_LOW);
	system_remove_task(&s->task);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _handle(led_t *s)
{
    if(s->pulsing_interval_ms > 0) // Pulsing mode
    {
        if(s->is_on)
        {
            if( (system_get_tick_count() - s->timestamp) >= s->pulsing_interval_ms)
            {
                s->timestamp = system_get_tick_count();
                s->is_on = false;
                MCU_IO_SET_HANDLER(s->pin, s->is_on);
            }
        }
        else
        {
            if( (system_get_tick_count() - s->timestamp) >= s->blinking_interval_ms)
            {
                s->timestamp = system_get_tick_count();
                s->is_on = true;
                MCU_IO_SET_HANDLER(s->pin, s->is_on);
            }
        }
    }
    else // Blinking mode
    {
        if( (system_get_tick_count() - s->timestamp) >= s->blinking_interval_ms)
        {
            s->timestamp = system_get_tick_count();
            MCU_IO_TOGGLE_HANDLER(s->pin);
        }
    }
}

#if MODULE_ENABLE_DEBUG_CONSOLE
static FUNCTION_RETURN _console(console_data_t* data, char** args, uint8_t args_len)
{
	if(args_len < 1)
		return FUNCTION_RETURN_PARAM_ERROR;

	if(strcmp(args[0], "list") == 0)
	{
		led_t* ptr = list_get_first(&_list);
		// TODO: Modify this command to use console_set_response_dynamic -> Low priority because it is unused by testsystem
		if(ptr == NULL)
		{
			comm_printf(data->comm, "none");
		}
		else
		{
			while(ptr)
			{
				comm_printf(data->comm, "ledrsp list \"%s", ptr->task.name);
				ptr = list_get_next(&_list, ptr);
				if(ptr != NULL)
					comm_putc(data->comm, ',');
			}
			comm_putc(data->comm, '"');
			comm_putc(data->comm, '\n');
		}
		return FUNCTION_RETURN_OK;
	}
	else if(args_len == 2)
	{
		if(strcmp(args[0], "on") == 0)
		{
			led_t* led = led_get_pointer(args[1]);
			if(led)
			{
				led_on(led);
				return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 50, "on \\\"%s\\\"", led->task.name);
			}
		}
		else if(strcmp(args[0], "off") == 0)
		{
			led_t* led = led_get_pointer(args[1]);
			if(led)
			{
				led_off(led);
				return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 50, "off \\\"%s\\\"", led->task.name);
			}
		}
		else
			return FUNCTION_RETURN_NOT_FOUND;
	}
	else if(args_len == 4)
	{
		if(strcmp(args[0], "pulse") == 0)
		{
			led_t* led = led_get_pointer(args[1]);
			if(led)
			{
				uint32_t on_time = strtoul(args[2], NULL, 10);
				uint32_t off_time = strtoul(args[3], NULL, 10);
				led_set_pulsing_ms(led, on_time, off_time);
				return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 50, "pulse \\\"%s\\\"", led->task.name);
			}
		}
		else
			return FUNCTION_RETURN_NOT_FOUND;
	}
	else
		return FUNCTION_RETURN_NOT_FOUND;

	return FUNCTION_RETURN_PARAM_ERROR;
}
#endif

#endif
