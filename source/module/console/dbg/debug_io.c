// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file debug_network.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_DEBUG_CONSOLE
#include "module/console/console.h"
#include "debug_console.h"
#if DEBUG_CONSOLE_ENABLE_IO
#include "debug_buffer.h"
#include "module/convert/string.h"
#if MCU_TYPE == MCU_ESP32
#include "driver/gpio.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Parses an I/O string name to an MCU_IO_PIN value.
 *
 * @param str	Name of the pin like PC_7.
 * @return		MCU_IO_PIN value of the pin. If str is invalid, PIN_NONE is returned.
 */
MCU_IO_PIN _parse_from_string(char* str);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
FUNCTION_RETURN debug_io_execute(console_data_t* data, char** args, uint8_t args_len)
{
	bool is_high;
	bool is_out;
	if(args_len > 1)
	{
		MCU_IO_PIN io = _parse_from_string(args[1]);
		if(strcmp(args[0], "dir") == 0 && args_len > 2 && ((is_out = (strcmp(args[2], "out") == 0)) || (strcmp(args[2], "in") == 0)) && io != PIN_NONE)
		{
			mcu_io_set_dir(io, is_out ? MCU_IO_DIR_OUT : MCU_IO_DIR_IN);
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 30, "dir %s %s", args[1], is_out ? "OUT" : "IN");
		}
		else if(strcmp(args[0], "get") == 0 && io != PIN_NONE)
		{
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 30, "get %s %s", args[1], mcu_io_get(io) ? "H" : "L");
		}
		else if(strcmp(args[0], "set") == 0 && args_len > 2 && ((is_high = (args[2][0] == 'h' || args[2][0] == 'H'))
				|| (args[2][0] == 'l' || args[2][0] == 'L')) && io != PIN_NONE)
		{
			mcu_io_set(io, is_high);
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 30, "set %s %s", args[1], mcu_io_get(io) ? "H" : "L");
		}
		else if(strcmp(args[0], "toggle") == 0 && io != PIN_NONE)
		{
			mcu_io_toggle(io);
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 30, "set %s %s", args[1], mcu_io_get(io) ? "H" : "L");
		}
		else if(strcmp(args[0], "reset") == 0 && io != PIN_NONE)
		{
			mcu_io_reset(io);
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 30, "reset %s", args[1]);
		}
		else
			return FUNCTION_RETURN_NOT_FOUND;
	}
	#if MCU_TYPE == MCU_ESP32
	else if(args_len >= 1 && strcmp(args[0], "dump") == 0)
	{
		if(args_len == 1)
		{
			// Dump all valid GPIOs
			esp_err_t err = gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK);
			if(err == ESP_OK)
			{
				return console_set_response_static(data, FUNCTION_RETURN_OK, "dump");
			}
			else
			{
				return console_set_response_dynamic(data, FUNCTION_RETURN_EXECUTION_ERROR, 50, 
					"dump failed: %s", esp_err_to_name(err));
			}
		}
		else
		{
			// Dump specific GPIOs
			uint64_t gpio_mask = 0;
			
			for(int i = 1; i < args_len; i++)
			{
				int gpio_num = strtol(args[i], NULL, 10);
				
				// Check if GPIO number is valid using the SOC mask
				if(gpio_num < 0 || gpio_num > SOC_GPIO_IN_RANGE_MAX || 
				   !(SOC_GPIO_VALID_GPIO_MASK & (1ULL << gpio_num)))
				{
					return console_set_response_dynamic(data, FUNCTION_RETURN_PARAM_ERROR, 50, 
						"Invalid GPIO: %d", gpio_num);
				}
				
				gpio_mask |= (1ULL << gpio_num);
			}
			
			esp_err_t err = gpio_dump_io_configuration(stdout, gpio_mask);
			if(err == ESP_OK)
			{
				return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 50, 
					"dump %d GPIO(s)", args_len - 1);
			}
			else
			{
				return console_set_response_dynamic(data, FUNCTION_RETURN_EXECUTION_ERROR, 50, 
					"dump failed: %s", esp_err_to_name(err));
			}
		}
	}
	#endif
	else 
	{
		return FUNCTION_RETURN_NOT_FOUND;
	}

	return FUNCTION_RETURN_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

MCU_IO_PIN _parse_from_string(char* str)
{
#if MCU_TYPE == MCU_ESP32
	return strtol(str, NULL, 10);
#else
	MCU_IO_PIN io = PIN_NONE;
	uint16_t i_port;
	uint8_t i_pin;
	char port;
	if(*str == 'P')
		str++;
	port = string_to_lower(str[0]);
	if(port >= 'a' && port <= 'z')
		i_port = 0x0A + port - 'a'; // Port A is starting as 10, B is 11 and so on -> It is NOT only hex! PH_0 might also exist!
	else
		i_port = strtol(str, NULL, 10);

	while(*str && *str != '_')
		str++;
	if(*str == '_')
	{
		str++;
		i_pin = strtol(str, NULL, 10);
#if MCU_TYPE == MCU_STM32F3 || MCU_TYPE == MCU_STM32F4 || MCU_TYPE == MCU_STM32F7 || MCU_TYPE == SYNERGY
		io = (MCU_IO_PIN)((i_port << 8) + (i_pin & 0xFF));
#else
		io = (MCU_IO_PIN)((i_port << 8) + ((1 << i_pin) & 0xFF));
#endif
	}
	else
		io = (MCU_IO_PIN)(i_port << 8);

	return io;
#endif
}

#endif // DEBUG_CONSOLE_ENABLE_IO

#endif // MODULE_ENABLE_DEBUG_CONSOLE
