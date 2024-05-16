// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file console.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */


#include "module_public.h"
#if MODULE_ENABLE_CONSOLE

#include "console.h"

#include "mcu/sys.h"
#include "module/convert/string.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Task of the debug console
//static console_data_t			_data;

static char*					_args_ptr[CONSOLE_MAX_ARGUMENTS];

/// Pointer to the first registered command.
static console_command_t*			_first_command;

static console_command_t			_command_help;

static bool 						_is_first_init = true;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief	The handle function receives data over the uart and handles received commands.
 *
 * @param obj					Pointer to the debug console data structure to use.
 **/
static void console_handle(console_data_t* data);
/**
 *
 * @param data		Pointer to the console the command should be checked on.
 * @param cmd		Command to be checked if it is whitelisted.
 * @return			true if command passes the whitelistcheck or false if command should be ignored.
 */
static bool _passes_whitelist(console_data_t* data, const char* cmd);
/**
 * @brief	Handles the command that was received in the line string.
 *
 * @param obj					Pointer to the debug console data structure to use.
 * @param line					0-terminated string that contains the received line that should be handled.
 */
static void console_handle_command(console_data_t* data, char* line);
/**
 * @brief	Prints a list of all registered application commands.
 *
 * @param line		Line of the command, starting after the command.
 */
FUNCTION_RETURN console_help_execute(console_data_t* data, char* line);
/**
 * Handles the return value of a console function to print parameter error, etc.
 * @param data		Pointer to the console.
 * @param cmd		Command that had the error
 * @param ret		Return value of a function.
 */
static void _handle_return_value(console_data_t* data, char* cmd, FUNCTION_RETURN ret);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void console_init(console_data_t* data, comm_t* comm)
{
	// Set internal variables
//	memset(data, 0, sizeof(console_data_t));
	data->comm = comm;
#if MCU_TYPE == MCU_ESP32
	data->line_buffer = mcu_heap_calloc(1, CONSOLE_LINE_BUFFER_SIZE);
#endif

	if(_is_first_init)
	{
		_is_first_init = false;

		// Only needed once:

		// Add the help command -> Is used to show all registered applications
		_command_help.command = "help";
		_command_help.fnc_exec = console_help_execute;
		_command_help.explanation = "Prints the help.";
		_command_help.use_array_param = false;
		console_add_command(&_command_help);
	}
#if CONSOLE_ENABLE_CRC
	crc_init_handler(&data->crc, 0x1021, 0xFFFF, 0x0000);
#endif

	// Start system task
	system_init_object_task(&data->task, true, (void(*)(void*))console_handle, data);
}

void console_stop(console_data_t* data)
{
	if(data == NULL)
		return;

	system_remove_task(&data->task);
#if MCU_TYPE == MCU_ESP32
	if(data->line_buffer)
	{
		mcu_heap_free(data->line_buffer);
		data->line_buffer = NULL;
	}
#endif
}

void console_add_command(console_command_t* cmd_obj)
{
	if(cmd_obj == NULL)
		return;

	if(_first_command == NULL)
	{
		_first_command = cmd_obj;
		_first_command->next = NULL;
	}
	else
	{
		console_command_t* tmp = _first_command;

		while(tmp->next != NULL)
		{
			if(tmp == cmd_obj)
				return;

			tmp = (console_command_t*)tmp->next;
		}

		tmp->next = cmd_obj;
		cmd_obj->next = NULL;
	}
}

void console_remove_command(console_command_t* cmd_obj)
{
	if(cmd_obj == NULL || _first_command == NULL)
		return;

	if(_first_command == cmd_obj)
		_first_command = (console_command_t*)_first_command->next;
	else
	{
		console_command_t* tmp = _first_command;

		while(tmp->next != cmd_obj && tmp->next != NULL)
			tmp = (console_command_t*)tmp->next;

		if(tmp->next == cmd_obj)
			tmp->next = cmd_obj->next;
	}
}

void console_set_byte_callback(console_data_t* data, console_cb_byte f, uint32_t timeout_ms, uint8_t escape_character)
{
	data->cb_byte = f;
	data->timeout_ms_cb_byte = timeout_ms;
	data->escape_character_cb_byte = escape_character;
}

void console_set_byte_num_callback(console_data_t* data, console_cb_byte f, uint32_t num)
{
	data->cb_byte = f;
	data->byte_num_cb = num;
}

void console_set_hold_callback(console_data_t* data, console_cb_hold f)
{
	data->f_hold = f;
}

FUNCTION_RETURN console_set_response_dynamic(console_data_t* data, FUNCTION_RETURN ret, uint16_t max_len, const char* format, ...)
{
	if(data == NULL || format == NULL)
		return FUNCTION_RETURN_PARAM_ERROR;

	// Don't know why someone should call this twice, but just in case someone does....
	if(data->message_heap && data->message)
		mcu_heap_free(data->message);

	data->has_response = true;

	// If len is 0, nothing is allocated and format is taken as a completely dynamic buffer.
	if(max_len == 0)
	{
		data->message_heap = true;
		data->message = (char*)format;		
		return ret;
	}

	data->message = mcu_heap_calloc(1, max_len);
	if(data->message)
	{
		va_list vl;
		data->message_heap = true;
		va_start(vl, format);
		int16_t len = string_vnprintf(data->message, max_len, format, vl);
		va_end(vl);
		
		if(len == max_len || data->message[len])
		{
			// This means the terminating 0 was not added. 
			data->message_heap = false;
			mcu_heap_free(data->message);
			data->message = NULL;
			return FUNCTION_RETURN_INSUFFICIENT_MEMORY;
		}
		return ret;
	}

	return FUNCTION_RETURN_INSUFFICIENT_MEMORY;
}

FUNCTION_RETURN console_set_response_static(console_data_t* data, FUNCTION_RETURN ret, const char* message)
{
	if(data == NULL)
		return FUNCTION_RETURN_PARAM_ERROR;
		
	// Don't know why someone should call dynamic first and static for same response, but just in case someone does....
	if(data->message_heap && data->message)
	{
		data->message_heap = false;
		mcu_heap_free(data->message);
		data->message = NULL;
	}

	data->message = (char*)message;
	data->has_response = true;

	return ret;
}

#if CONSOLE_ENABLE_CRC
uint16_t console_get_last_crc_value(console_data_t* data)
{
	return data->crc_value;
}

bool console_has_valid_crc(console_data_t* data)
{
	return data->crc_valid;
}
#endif

bool console_can_receive(console_data_t* data)
{
	if(data == NULL)
		return false;

	return data->f_hold == NULL || (data->f_hold != NULL && !data->f_hold(data));
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void console_handle(console_data_t* data)
{
	uint8_t c;

	if(data == NULL)
		return;

	while(comm_data_available(data->comm))
	{
		if(!console_can_receive(data))
		{
			return;
		}
		
		c = comm_getc(data->comm);
		if(data->cb_byte)
		{
			if(data->byte_num_cb > 0)
			{
				((console_cb_byte)data->cb_byte)(data, c, data->byte_num_cb == 1);
				data->byte_num_cb--;
				if(data->byte_num_cb == 0)
				{
					// Last expected byte was received -> Stop the direct receive
					data->cb_byte = NULL;
				}
				continue;
			}
			if(c != data->escape_character_cb_byte)
			{
				((console_cb_byte)data->cb_byte)(data, c, false);
				data->timestamp = system_get_tick_count();
				continue;
			}
			else
			{
				((console_cb_byte)data->cb_byte)(data, c, true);
				data->cb_byte = NULL;
				dbg_printf(DBG_STRING, "Console Escaped -> Back to normal\n");
				continue;
			}
		}

		if(c == '\r')
			c = '\n'; 	// Handle Carriage return as linefeed!

		if(c != '\n' && (c < 0x20 || c >= 0xF8)) // Ignore if not UTF-8
//		if(c != '\n' && (c < 0x20 || c > 0x7E)) // If it is not a ASCII character, ignore it
			continue;

		if(c == '\n')
		{
			if(data->line_cnt > 0)
			{
				data->line_buffer[data->line_cnt] = 0;
#if CONSOLE_ENABLE_CRC
				if(data->line_cnt > 4)
				{
					data->crc_value = crc_calc(&data->crc, (uint8_t*)data->line_buffer, data->line_cnt - 4);
					data->crc_valid = data->crc_value == strtol(&data->line_buffer[data->line_cnt - 4], NULL, 16);
				}
#endif
				console_handle_command(data, data->line_buffer);
			}
			data->line_cnt = 0;
			memset(data->line_buffer, 0, CONSOLE_LINE_BUFFER_SIZE);
		}
		else // Character needs to be added to the line
		{
			if(data->line_cnt < CONSOLE_LINE_BUFFER_SIZE - 1 /* -1 because of terminating zero */)
			{
				data->line_buffer[data->line_cnt] = c;
				data->line_cnt++;
			}
		}
		data->timestamp = system_get_tick_count();
	}
	if(!comm_data_available(data->comm))
	{
		if(data->cb_byte && data->byte_num_cb == 0)
		{
			if( (system_get_tick_count() - data->timestamp) >= data->timeout_ms_cb_byte )
			{
				((console_cb_byte)data->cb_byte)(data, 0, true);
				data->cb_byte = NULL;
				dbg_printf(DBG_STRING, "Console Timeout -> Back to normal\n");
			}
		}
		else if(data->line_cnt > 0)
		{
			if( (system_get_tick_count() - data->timestamp) >= CONSOLE_DROP_LINE_MS )
			{
				data->timestamp = system_get_tick_count();
				dbg_printf(DBG_STRING, "Drop Command: \"%s\"\n", data->line_buffer);
				data->line_buffer[data->line_cnt] = 0;
				data->line_cnt = 0;
				memset(data->line_buffer, 0, CONSOLE_LINE_BUFFER_SIZE);
			}
		}
	}
}

static bool _passes_whitelist(console_data_t* data, const char* cmd)
{
	if(data->command_whitelist == NULL)
		return true;

	for(int i = 0; i < data->num_whitelist; i++)
	{
		if(strcmp(cmd, data->command_whitelist[i]) == 0)
			return true;
	}

	return false;
}

static void console_handle_command(console_data_t* data, char* line)
{
	console_command_t* tmp = _first_command;

	data->has_response = false;
	data->message = NULL;
	data->message_heap = false;

	while(tmp != NULL)
	{
		if(tmp->fnc_exec != NULL)
		{
			char* ptr = line + strlen(tmp->command);
			if(strncmp(tmp->command, line, strlen(tmp->command)) == 0 && (*ptr == ' ' || *ptr == '\0'))
			{
//				dbg_printf(DBG_STRING, "Handle Command: \"%s\"\n", data->line_buffer);
				if(!_passes_whitelist(data, tmp->command))
				{
					// Command did not pass the whitelist check -> Ignore it!
					continue;
				}

				while(*ptr == ' ')
					ptr++;

				if(!tmp->use_array_param)
				{
					FUNCTION_RETURN ret = ((FUNCTION_RETURN(*)(console_data_t*, char*))tmp->fnc_exec)(data, ptr);
					_handle_return_value(data, tmp->command, ret);
				}
				else
				{
					if(*ptr)
					{
						uint8_t args_length = 0;
						bool in_string = false;
						memset(_args_ptr, 0, sizeof(_args_ptr));
						_args_ptr[0] = ptr;

						while(*ptr != 0)
						{
							if(*ptr == '\"')
							{
								in_string = false;
								*ptr = 0;
							}
							else if(!in_string && *ptr == ' ')
							{
								while(*ptr == ' ')
								{
									*ptr++ = 0;
								}
								// If max length is reached break before ++ because the ++ in the if below the while will trigger
								// the ++.
								if(args_length == CONSOLE_MAX_ARGUMENTS)
								{
									*ptr = 0;
									continue;
								}
								args_length++;
								if(*ptr == '\"') // First quote
								{
									in_string = true;
									*ptr++ = 0;
								}
								if(*ptr != 0)
								{
									_args_ptr[args_length] = ptr;
									if(in_string && *ptr == '\"') // Next is second quote
										continue;
								}
							}
							ptr++;
						}

						if(_args_ptr[args_length])// && _args_ptr[args_length][0])
							args_length++;

						FUNCTION_RETURN ret = ((FUNCTION_RETURN(*)(console_data_t*, char**, uint8_t))tmp->fnc_exec)(data, _args_ptr, args_length);
						_handle_return_value(data, tmp->command, ret);
					}
					else
					{
						FUNCTION_RETURN ret = ((FUNCTION_RETURN(*)(console_data_t*, char**, uint8_t))tmp->fnc_exec)(data, _args_ptr, 0);
						_handle_return_value(data, tmp->command, ret);
					}
				}

				if(data->message_heap && data->message)
				{
					mcu_heap_free(data->message);
					data->message = NULL;
					data->message_heap = false;
				}

				return;
			}
		}
		tmp = (console_command_t*)tmp->next;
	}

    if(data->debug_line)
    {
        comm_printf(COMM_DEBUG, "\\.%s\n", line);
    }

	if(!data->suppress_invalid_command)
	{
		data->message = "Invalid Command"; 
		_handle_return_value(data, line, FUNCTION_RETURN_NOT_FOUND);
	}
}

FUNCTION_RETURN console_help_execute(console_data_t* data, char* line)
{
	uint16_t j = 0;
	console_command_t* tmp = _first_command;

	if(_first_command == NULL)
	{
		comm_printf(data->comm, "No commands registered\n");
		comm_flush(data->comm);
		return FUNCTION_RETURN_NOT_READY;
	}

	comm_printf(data->comm, "The following commands can be used:\n");
	while(tmp != NULL)
	{
		comm_printf(data->comm, "%s", tmp->command);
		j = strlen(tmp->command);
		for(; j < 10; j++)
			comm_putc(data->comm, ' ');
		comm_puts(data->comm, tmp->explanation);
		comm_putc(data->comm, '\n');

		tmp = (console_command_t*)tmp->next;
	}
	comm_flush(data->comm);
	return FUNCTION_RETURN_OK;
}

static void _handle_return_value(console_data_t* data, char* cmd, FUNCTION_RETURN ret)
{
	if(!data->has_response)
		return;
		
	const char* error_string = "UNKNOWN";

	switch(ret)
	{
		case FUNCTION_RETURN_OK:
			error_string = "OK";
			break;
		case FUNCTION_RETURN_PARAM_ERROR:
			error_string = "PARAM_ERROR";
			break;
		case FUNCTION_RETURN_NOT_READY:
			error_string = "NOT_READY";
			break;
		case FUNCTION_RETURN_UNSUPPORTED:
			error_string = "UNSUPPORTED";
			break;
		case FUNCTION_RETURN_NOT_FOUND:
			error_string = "NOT_FOUND";
			break;
		case FUNCTION_RETURN_INSUFFICIENT_MEMORY:
			error_string = "INSUFFICIENT_MEMORY";
			break;
		case FUNCTION_RETURN_UNAUTHORIZED:
			error_string = "UNAUTHORIZED";
			break;
		case FUNCTION_RETURN_NO_RESPONSE:
			error_string = "NO_RESPONSE";
			break;
		case FUNCTION_RETURN_WRONG_DEVICE:
			error_string = "WRONG_DEVICE";
			break;
		case FUNCTION_RETURN_EXECUTION_ERROR:
			error_string = "EXECUTION_ERROR";
			break;
		default:		
			error_string = "UNKNOWN";
			break;
	}

	comm_printf(data->comm, "res %d \"%s\" \"%s\"\n", ret, error_string, data->message);
	comm_flush(data->comm);
}

#endif

