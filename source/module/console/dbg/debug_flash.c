// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/***
 * @file debug_flash.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
 **/

#include "module_public.h"
#if MODULE_ENABLE_DEBUG_CONSOLE && MODULE_ENABLE_FLASH_INFO
#include "module/convert/string.h"
#include "module/flash_info/flash_info.h"
#include "module/console/console.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define IS_CMD(str)		(strstr(line, str) == line)

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
FUNCTION_RETURN debug_flash_execute(console_data_t* data, char* line)
{
	uint32_t tmp;
	uint8_t mac[6];
	char* ptr = NULL;

	if(IS_CMD("hwid"))
	{
		line += 5;
		if(IS_CMD("set"))
		{
			tmp = strtol(&line[4], &ptr, 10);
			flash_info_set_hardware_id(tmp);
			flash_info_save();
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 30, "hwid set %u", tmp);
		}
		else if(IS_CMD("get"))
		{
			tmp = flash_info_get_hardware_id();
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 30, "hwid get %u", tmp);
		}
		else
		{
			return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Set/get expected");
		}
	}
	else if (IS_CMD("hwrev"))
	{
		line += 6;
		if (IS_CMD("set"))
		{
			tmp = strtol(&line[4], &ptr, 10);
			flash_info_set_hardware_revision(tmp);
			flash_info_save();
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 30, "hwrev set %u", tmp);
		}
		else if (IS_CMD("get"))
		{
			tmp = flash_info_get_hardware_revision();
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 30, "hwrev get %u", tmp);
		}
		else
		{
			return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Set/get expected");
		}
	}
	else if (IS_CMD("mac"))
	{
		line += 4;
		if (IS_CMD("set"))
		{
			line += 4;
//			dbg_printf(DBG_STRING, "LINE: %s\n", line);
			mac[0] = string_ascii_to_uint8(line[0]) * 16 + string_ascii_to_uint8(line[1]);
			line += 3;
			mac[1] = string_ascii_to_uint8(line[0]) * 16 + string_ascii_to_uint8(line[1]);
			line += 3;
			mac[2] = string_ascii_to_uint8(line[0]) * 16 + string_ascii_to_uint8(line[1]);
			line += 3;
			mac[3] = string_ascii_to_uint8(line[0]) * 16 + string_ascii_to_uint8(line[1]);
			line += 3;
			mac[4] = string_ascii_to_uint8(line[0]) * 16 + string_ascii_to_uint8(line[1]);
			line += 3;
			mac[5] = string_ascii_to_uint8(line[0]) * 16 + string_ascii_to_uint8(line[1]);


			flash_info_set_mac_address((uint8_t*)&mac);
			flash_info_save();
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 30, "mac set %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
		else if (IS_CMD("get"))
		{
			flash_info_get_mac_address((uint8_t*)&mac);
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 30, "mac get %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
		else
		{
			return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Set/get expected");
		}
	}
	else if(IS_CMD("save"))
	{
		bool saved = flash_info_save();
		if(saved)
			return console_set_response_static(data, FUNCTION_RETURN_OK, "save");
		else
			return console_set_response_static(data, FUNCTION_RETURN_NOT_READY, "Saving failed");
	}
		
	return FUNCTION_RETURN_NOT_FOUND;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif
