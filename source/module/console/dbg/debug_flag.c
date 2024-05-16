// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file debug_flag.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_DEBUG_CONSOLE
#include "module/console/console.h"
#include "debug_console.h"
#if DEBUG_CONSOLE_ENABLE_FLAG
#include "module/convert/string.h"
#include "module/convert/base64.h"
#include "module/convert/math.h"
#include "module/list/list.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct _flag_s _flag_t;

struct _flag_s
{
	const char* name;

    debug_flag_cb_t f;

	_flag_t* next;
};

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

void debug_flag_init(void)
{
	_flag_t* f = NULL;
	list_init(&_list, f, &f->next);
}

FUNCTION_RETURN debug_flag_execute(console_data_t* data, char** args, uint8_t args_len)
{
    if(args_len == 1)
    {
        _flag_t* f = list_get_first(&_list);

        while(f)
        {
            if(strcmp(f->name, args[0]) == 0)
            {
                return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 200, "%s %d", f->name, f->f(f->name));
            }
            f = list_get_next(&_list, f);
        }
        return console_set_response_static(data, FUNCTION_RETURN_NOT_FOUND, "");
    }
    return console_set_response_static(data, FUNCTION_RETURN_NOT_FOUND, "");
}

FUNCTION_RETURN debug_console_register_flag(const char* name, debug_flag_cb_t f)
{
    DBG_ASSERT(name != NULL, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Name cannot be NULL\n");
    DBG_ASSERT(f != NULL, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Function cannot be NULL\n");

    _flag_t* flag = mcu_heap_calloc(1, sizeof(_flag_t));

    DBG_ASSERT(flag != NULL, NO_ACTION, FUNCTION_RETURN_INSUFFICIENT_MEMORY, "No heap to register %s\n", name);

    flag->f = f;
    flag->name = name;

    list_add(&_list, flag);
    return FUNCTION_RETURN_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // DEBUG_CONSOLE_ENABLE_FLAG

#endif // MODULE_ENABLE_DEBUG_CONSOLE