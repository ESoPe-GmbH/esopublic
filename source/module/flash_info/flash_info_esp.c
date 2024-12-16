// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file flash_info.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
 */

#include "flash_info.h"

#if MODULE_ENABLE_FLASH_INFO && MCU_TYPE == MCU_ESP32

#include "module/crc/crc.h"
#include <string.h>
#include "module/comm/comm.h"
#include "esp_err.h"
#include "nvs_flash.h"

#if !FLASH_INFO_DEBUG
#undef DBG_INFO
#define DBG_INFO(...)
#endif

//------------------------------------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------------------------------------

#define _NVS_HARDWARE_ID		"hwid"

//------------------------------------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------------------------------------

static nvs_handle_t 	_nvs;

static uint32_t 		_hardware_id;

//------------------------------------------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------------------------------------------

/**
 * @brief	Tries to load the stored information from the flash.
 */
static void flash_info_load(void);

//------------------------------------------------------------------------------------------------------------
// External functions
//------------------------------------------------------------------------------------------------------------

void flash_info_init(void)
{
	esp_err_t ret = nvs_open("flash_info", NVS_READWRITE, &_nvs);
	if(ret == ESP_OK)
	{
		flash_info_load();
	}
	else
	{
		DBG_ERROR("NVS Error 0x%04x\n", ret);
		// Default values...
		_hardware_id = 0;
	}
}

bool flash_info_save(void)
{
	esp_err_t ret =	nvs_set_u32(_nvs, _NVS_HARDWARE_ID, _hardware_id);

	return (ret == ESP_OK);
}


uint8_t flash_info_get_flash_info_version(void){		return 0;				}

uint32_t flash_info_get_hardware_id(void){				return _hardware_id;			}

uint16_t flash_info_get_tested_year(void){				return 0;			}

uint8_t flash_info_get_tested_month(void){				return 0;		}

uint8_t flash_info_get_tested_day(void){				return 0;			}

uint8_t flash_info_get_tested_hour(void){				return 0;			}

uint8_t flash_info_get_tested_minute(void){				return 0;		}

void flash_info_set_hardware_id(uint32_t id){			_hardware_id = id;			}

void flash_info_set_tested_date(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute)
{

}

void flash_info_set_mac_address(uint8_t* mac)
{

}

void flash_info_get_mac_address(uint8_t* mac)
{
	memset(mac, 0, 6);
}

//------------------------------------------------------------------------------------------------------------
// Internal functions
//------------------------------------------------------------------------------------------------------------

static void flash_info_load(void)
{
	esp_err_t ret;
	if(ESP_OK != (ret = nvs_get_u32(_nvs, _NVS_HARDWARE_ID, &_hardware_id)))
	{
		DBG_INFO("NVS Error 0x%04x (HWID)\n", ret);
		_hardware_id = 0;
	}
}

#endif // #if MCU_PERIPHERY_ENABLE_FLASH
