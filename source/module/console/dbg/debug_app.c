// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file debug_network.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_DEBUG_CONSOLE
#include "module/version/version.h"
#include "module/console/console.h"
#include "debug_buffer.h"
#include "module/convert/base64.h"
#if MODULE_ENABLE_SECURITY
#include "module/security/security.h"
#if SECURITY_ENABLE_MBEDTLS && MCU_TYPE != PC_EMU
#define ENABLE_HASH			true
#include "module/flash/flash.h"
#include "mbedtls/sha256.h"
#endif
#if MCU_TYPE == MCU_ESP32
#include "esp_system.h"
#endif
#endif
#if MCU_TYPE == R5F523
#include <machine.h>
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

#if MCU_TYPE == R5F523
#pragma inline_asm _nop
static _nop()
{
	NOP
}
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
FUNCTION_RETURN debug_app_execute(console_data_t* data, char** args, uint8_t args_len)
{
	if(args_len > 0)
	{
		if(strcmp(args[0], "version") == 0)
		{
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 20, "version %s", version_get_string());
		}
		else if(strcmp(args[0], "reset") == 0)
		{
			comm_printf(data->comm, "res 0 \"OK\" \"reset\"\n\n\n\n\n"); // Well, make sure on UART at least one line feed is out.
														// On other comm channels the response might not be received, so don't bet on it.
			mcu_soft_reset();
		}
#if MCU_PERIPHERY_ENABLE_WATCHDOG
		else if(strcmp(args[0], "stop") == 0) // Can be used to test watchdogs.
		{
			comm_printf(data->comm, "apprsp stop\n");
			while(1)
			{
#if MCU_TYPE == R5F523
				_nop();
//				asm("nop");
#else
				asm volatile("nop");
#endif
			}
		}
#endif
#if MCU_TYPE == MCU_ESP32
		else if(strcmp(args[0], "heap") == 0)
		{
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 50, "heap %u", esp_get_free_heap_size());
		}
#endif
#if ENABLE_HASH
    else if(strcmp(args[0], "hash") == 0)
    {
        if(args_len < 3)
		{
			return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Provide a start address and length");
		}

        mbedtls_sha256_context _hash_sha_256;
        uint8_t buffer[64];

        uint32_t addr = strtoul(args[1], NULL, 16);
        uint32_t len_total = strtoul(args[2], NULL, 10);

        if(len_total == 0)
        {
			return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Len has to be greater 0");
        }

		uint32_t len = sizeof(buffer);
		mbedtls_sha256_init(&_hash_sha_256);

		mbedtls_sha256_starts_ret(&_hash_sha_256, false);

//            while((len = mmc_gets(&_file, buffer, sizeof(buffer))) > 0)

		for(;len_total > 0; len_total -= len, addr += len)
		{
			flash_read((FLASH_PTR_TYPE)addr, buffer, len);
			if(len > len_total)
				len = len_total;
//            	DBG_INFO("Hash %d\n", len);
			mbedtls_sha256_update_ret(&_hash_sha_256, buffer, len);
		}

		mbedtls_sha256_finish_ret(&_hash_sha_256, buffer);
		len = base64_encodebuffer_direct(buffer, 32);
//        	DBG_INFO("Len %d\n", len);

		FUNCTION_RETURN ret = console_set_response_dynamic(data, FUNCTION_RETURN_OK, 100, "hash %#s", len, buffer);

		mbedtls_sha256_free(&_hash_sha_256);

		return ret;
    }
#endif
		else
			return FUNCTION_RETURN_NOT_FOUND;
	}
	else
		return FUNCTION_RETURN_NOT_FOUND;
	return FUNCTION_RETURN_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // MODULE_ENABLE_NETWORK

