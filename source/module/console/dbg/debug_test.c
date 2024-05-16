// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file debug_uart
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_DEBUG_CONSOLE
#include "module/console/console.h"
#include "debug_console.h"
#if DEBUG_CONSOLE_ENABLE_TEST
#include "debug_buffer.h"
#include "module/convert/string.h"
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


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void debug_test_init(void)
{
	debug_console_test_t* test = NULL;
	list_init(&_list, test, &test->next);
}

FUNCTION_RETURN debug_test_execute(console_data_t* data, char** args, uint8_t args_len)
{
	if(args_len >= 2)
	{
		if(strcmp(args[0], "start") == 0)
		{
			if(strcmp(args[1], DEBUG_CONSOLE_TEST_PASSWORD) != 0)
			{
				return console_set_response_static(data, FUNCTION_RETURN_UNAUTHORIZED, "Invalid password");
			}

			debug_console_test_t* ptr = list_get_first(&_list);
			while(ptr)
			{
				if(ptr->f)
					ptr->f(ptr->obj, data, &args[2], args_len - 2);

				ptr = list_get_next(&_list, ptr);
			}

			return console_set_response_static(data, FUNCTION_RETURN_OK, "start");
		}
		else
			return FUNCTION_RETURN_NOT_FOUND;
	}
	else
		return FUNCTION_RETURN_NOT_FOUND;

	return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN debug_console_register_test_callback(debug_console_test_t* dbt, void* obj, debug_console_test_cb f)
{
	if(dbt == NULL || f == NULL)
		return FUNCTION_RETURN_PARAM_ERROR;

	dbt->obj = obj;
	dbt->f = f;

	return list_add(&_list, dbt);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


#endif // DEBUG_CONSOLE_ENABLE_IO

#endif // MODULE_ENABLE_DEBUG_CONSOLE
