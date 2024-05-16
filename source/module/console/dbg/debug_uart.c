// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file debug_uart
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_DEBUG_CONSOLE
#include "module/console/console.h"
#include "debug_console.h"
#if DEBUG_CONSOLE_ENABLE_UART
#include "debug_buffer.h"
#include "module/convert/string.h"
#include "module/convert/math.h"
#include "module/convert/base64.h"
#include "module/list/list.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static list_t _list;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Try to get the UART to the name/num parameter. If it is a number, the num of the uarts in the list is searched, otherwise the name is searched.
 * @param arg	String buffer of the name/num of the uart.
 * @return		NULL if no UART is found, otherwise pointer to the UART.
 */
static debug_console_uart_t* _get_uart(char* arg);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void debug_uart_init(void)
{
	debug_console_uart_t* uart = NULL;
	list_init(&_list, uart, &uart->next);
}

FUNCTION_RETURN debug_uart_execute(console_data_t* data, char** args, uint8_t args_len)
{
	if(args_len >= 1)
	{
		if(strcmp(args[0], "list") == 0)
		{
			debug_console_uart_t* ptr = list_get_first(&_list);
			while(ptr)
			{
				comm_printf(data->comm, "urc list %d \"%s\"\n", ptr->num, ptr->name);
				ptr = list_get_next(&_list, ptr);
			}
			
			return console_set_response_static(data, FUNCTION_RETURN_OK, "fin");
		}
		else if(args_len == 3)
		{
			debug_console_uart_t* uart = _get_uart(args[1]);
			if(uart == NULL)
			{
				return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Invalid UART");
			}

			if(strcmp(args[0], "init") == 0)
			{
				uint32_t baud = strtoul(args[2], NULL, 10);

				mcu_uart_set_param(uart->uart, baud, 8, 'N', 1);
				if(uart->buffer && uart->buffer_size > 0)
					mcu_uart_set_buffer(uart->uart, MCU_INT_LVL_MED, uart->buffer, uart->buffer_size);

				return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 20, "init %d", baud);
			}
			else if(strcmp(args[0], "put") == 0)
			{
				uint32_t len = base64_decodebuffer_direct((uint8_t*)args[2], strlen(args[2]));

				if(len > 0)
				{
					mcu_uart_puts(uart->uart, (uint8_t*)args[2], len);
					// Well... block here until it is finished. Since this function is for testing purposes and testing is sequential, this is no problem.
					while(!mcu_uart_transmit_ready(uart->uart));
				}

				return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 20, "put %d", len);
			}
			else if(strcmp(args[0], "get") == 0)
			{
				uint8_t buffer[135];
				uint32_t max_len = strtoul(args[2], NULL, 10);
				uint32_t len;
				if(max_len == 0)
				{
					return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Cannot read 0 byte");
				}
				len = mcu_uart_gets(uart->uart, buffer, MATH_MIN(max_len, 100));

				len = base64_encodebuffer_direct(buffer, len);

				if(len > 0)
					return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 200, "get \\\"%#s\\\"", len, buffer);
				else
					return console_set_response_static(data, FUNCTION_RETURN_OK, "get \\\"\\\"");
			}
			else if(strcmp(args[0], "clear") == 0)
			{
				if(strcmp(args[2], "rx") == 0)
				{
					mcu_uart_clear_rx(uart->uart);
					return console_set_response_static(data, FUNCTION_RETURN_OK, "clear rx");
				}
				else
				{
					return console_set_response_static(data, FUNCTION_RETURN_UNSUPPORTED, "Only rx supported");
				}
			}
		}
	}

	return FUNCTION_RETURN_NOT_FOUND;
}

FUNCTION_RETURN debug_console_register_uart(debug_console_uart_t* dbu, mcu_uart_t uart, const char* name, uint8_t num, uint8_t* buffer, uint16_t buffer_size)
{
	if(dbu == NULL || uart == NULL || name == NULL)
		return FUNCTION_RETURN_PARAM_ERROR;

	dbu->uart = uart;
	dbu->name = name;
	dbu->num = num;
	dbu->buffer = buffer;
	dbu->buffer_size = buffer_size;

	return list_add(&_list, dbu);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static debug_console_uart_t* _get_uart(char* arg)
{
	debug_console_uart_t* ptr = list_get_first(&_list);
	if(string_is_decimal(arg, strlen(arg), false))
	{
		uint32_t num = strtoul(arg, NULL, 10);
		while(ptr)
		{
			if(ptr->num == num)
				return ptr;
			ptr = list_get_next(&_list, ptr);
		}
	}
	else
	{
		while(ptr)
		{
			if(strcmp(arg, ptr->name) == 0)
				return ptr;
			ptr = list_get_next(&_list, ptr);
		}
	}
	return NULL;
}

#endif // DEBUG_CONSOLE_ENABLE_IO

#endif // MODULE_ENABLE_DEBUG_CONSOLE
