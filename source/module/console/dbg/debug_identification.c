// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file debug_identification.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_DEBUG_CONSOLE
#include "module/console/console.h"
#include "debug_console.h"
#if DEBUG_CONSOLE_ENABLE_IDENTIFICATION
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
static debug_identification_cb_t _identification_cb;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
FUNCTION_RETURN debug_identification_execute(console_data_t* data, char** args, uint8_t args_len)
{
    if (_identification_cb)
    {
        FUNCTION_RETURN_T ret = _identification_cb() ? FUNCTION_RETURN_OK : FUNCTION_RETURN_WRONG_DEVICE;
        return console_set_response_static(data, ret, "");
    }
    return console_set_response_static(data, FUNCTION_RETURN_NOT_FOUND, "");
}

FUNCTION_RETURN debug_console_register_identification(debug_identification_cb_t f)
{
    DBG_ASSERT(f != NULL, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Function cannot be NULL\n");

    _identification_cb = f;

    return FUNCTION_RETURN_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // DEBUG_CONSOLE_ENABLE_IDENTIFICATION

#endif // MODULE_ENABLE_DEBUG_CONSOLE