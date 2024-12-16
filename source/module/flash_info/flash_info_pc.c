// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file flash_info.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
 */

#include "flash_info.h"

#if MODULE_ENABLE_FLASH_INFO && MCU_TYPE == PC_EMU

#include "module/crc/crc.h"
#include <string.h>
#include "module/comm/comm.h"

#if !FLASH_INFO_DEBUG
#undef DBG_INFO
#define DBG_INFO(...)
#endif

//------------------------------------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------------------------------------

static uint32_t 		_hardware_id;

//------------------------------------------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------
// External functions
//------------------------------------------------------------------------------------------------------------

void flash_info_init(void)
{
	_hardware_id = 0;
}

bool flash_info_save(void)
{
	return true;
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

#endif // #if MCU_PERIPHERY_ENABLE_FLASH
