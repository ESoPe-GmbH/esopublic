// Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file debug_esp.c
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_DEBUG_CONSOLE
#include "module/console/console.h"
#include "debug_console.h"
#if DEBUG_CONSOLE_ENABLE_ESP
#include "esp_partition.h"
#include "esp_ota_ops.h"

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

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN debug_esp_execute(console_data_t *data, char **args, uint8_t args_len)
{
    if (args_len > 0)
    {
        if (strcmp(args[0], "hash") == 0)
        {
            uint8_t sha256_hash[32];
            const esp_partition_t *p = esp_ota_get_boot_partition();

            if (p == NULL)
            {
                return console_set_response_static(data, FUNCTION_RETURN_EXECUTION_ERROR, "Partition not found!");
            }

            esp_err_t err = esp_partition_get_sha256(p, sha256_hash);

            if (err == ESP_OK)
            {
                return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 70, "hash %32Q", sha256_hash);
            }
            else
            {
                return console_set_response_dynamic(data, FUNCTION_RETURN_EXECUTION_ERROR, 40, "Cannot get hash of partition: %04x", err);
            }
        }
        else
        {
            return FUNCTION_RETURN_NOT_FOUND;
        }
    }
    else
    {
        return FUNCTION_RETURN_NOT_FOUND;
    }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // DEBUG_CONSOLE_ENABLE_ESP

#endif // MODULE_ENABLE_DEBUG_CONSOLE