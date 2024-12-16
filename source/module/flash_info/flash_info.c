// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file flash_info.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
 */

#include "flash_info.h"

#if MODULE_ENABLE_FLASH_INFO && MCU_PERIPHERY_ENABLE_DATA_FLASH && MCU_TYPE != MCU_ESP32 && MCU_TYPE != PC_EMU

#include "module/crc/crc.h"
#include <string.h>
#include "module/comm/comm.h"

#if !FLASH_INFO_DEBUG
#undef DBG_INFO
#define DBG_INFO(...) dbg_nop()
#endif

//------------------------------------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------------------------------------

/// Offset of the data start byte inside the flash_info data
#define FL_DATA_START_OFFSET			(0)

/// Offset of the crc inside the flash_info data
#define FL_DATA_CRC_OFFSET				(FLASH_DATA_SIZE - 3)

/// Offset of the end byte inside the flash_info data
#define FL_DATA_END_OFFSET				(FLASH_DATA_SIZE - 1)

/// Pointer to the position of the start byte inside the flash_info data
#define FL_DATA_START					*((uint8_t*)(fl_data_all + FL_DATA_START_OFFSET))

/// Pointer to the position of the crc inside the flash_info data
#define FL_DATA_CRC						*((uint16_t*)(fl_data_all + FL_DATA_CRC_OFFSET))

/// Pointer to the position of the end byte inside the flash_info data
#define FL_DATA_END						*((uint8_t*)(fl_data_all + FL_DATA_END_OFFSET))

/// Pointer to the position of the flash_info version inside the flash_info data
#define FL_DATA_VERSION					*((uint8_t*)(fl_data_all + FL_DATA_START_OFFSET + 1))

/// Pointer to the position of the hardware id inside the flash_info data
#define FL_DATA_HARDWARE_ID				*((uint32_t*)(fl_data_all + FL_DATA_START_OFFSET + 2))

/// Pointer to the position of the testing date year inside the flash_info data
#define FL_DATA_TESTED_YEAR				*((uint16_t*)(fl_data_all + FL_DATA_START_OFFSET + 6))

/// Pointer to the position of the testing date month inside the flash_info data
#define FL_DATA_TESTED_MONTH			*((uint8_t*)(fl_data_all + FL_DATA_START_OFFSET + 8))

/// Pointer to the position of the testing date day inside the flash_info data
#define FL_DATA_TESTED_DAY				*((uint8_t*)(fl_data_all + FL_DATA_START_OFFSET + 9))

/// Pointer to the position of the testing date hour inside the flash_info data
#define FL_DATA_TESTED_HOUR				*((uint8_t*)(fl_data_all + FL_DATA_START_OFFSET + 10))

/// Pointer to the position of the testing date minute inside the flash_info data
#define FL_DATA_TESTED_MINUTE			*((uint8_t*)(fl_data_all + FL_DATA_START_OFFSET + 11))

/// Pointer to the position of the mac address inside the flash_info data
#define FL_DATA_MAC_ADDRESS				*((uint8_t*)(fl_data_all + FL_DATA_START_OFFSET + 12))

#if FLASH_USE_CUSTOM_DATA

	/// Offset of the custom data start byte inside the flash_info data
	#define FL_DATA_CUSTOM_START_OFFSET		(FLASH_DATA_SIZE)

	/// Offset of the custom data crc inside the flash_info data
	#define FL_DATA_CUSTOM_CRC_OFFSET		(FLASH_DATA_SIZE + FLASH_CUSTOM_DATA_SIZE - 3)

	/// Offset of the custom data end byte inside the flash_info data
	#define FL_DATA_CUSTOM_END_OFFSET		(FLASH_DATA_SIZE + FLASH_CUSTOM_DATA_SIZE - 1)

	/// Pointer to the position of the start byte inside the flash_info custom data
	#define FL_DATA_CUSTOM_START			*((uint8_t*)(fl_data_all + FL_DATA_CUSTOM_START_OFFSET))

	/// Pointer to the position of the crc inside the flash_info custom data
	#define FL_DATA_CUSTOM_CRC				*((uint16_t*)(fl_data_all + FL_DATA_CUSTOM_CRC_OFFSET))

	/// Pointer to the position of the end byte inside the flash_info custom data
	#define FL_DATA_CUSTOM_END				*((uint8_t*)(fl_data_all + FL_DATA_CUSTOM_END_OFFSET))

#endif

//------------------------------------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------------------------------------

/// crc_t used for the crc calculation of the flash_info data
crc_t flash_crc_handler;

/// Buffer storing the data that is written to and read from the flash
uint8_t fl_data_all[FLASH_DATA_SIZE + FLASH_CUSTOM_DATA_SIZE];

/// Buffer used for verification in flash_info_save
static uint8_t _verify_buffer[10];

#if FLASH_USE_CUSTOM_DATA

	/// Counts the data that is reserved by the user using flash_info_get_byte_array,
	/// flash_info_get_4_byte_ptr and flash_info_get_2_byte_ptr.
	uint16_t fl_custom_data_cnt = FL_DATA_CUSTOM_START_OFFSET + 1;	// Element 0 would be 0x02

#endif

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
	memset(fl_data_all, 0, sizeof(fl_data_all));
	crc_init_handler(&flash_crc_handler, 0x1021, 0xFFFF, 0x0000);
	flash_info_load();
}

bool flash_info_save(void)
{
	uint8_t tries = 0;

	FL_DATA_START = 0x02;
	FL_DATA_VERSION = FLASH_INFO_VERSION;
	FL_DATA_CRC = crc_calc(&flash_crc_handler,
									fl_data_all + FL_DATA_START_OFFSET,
									FL_DATA_CRC_OFFSET - FL_DATA_START_OFFSET);
	FL_DATA_END = 0x03;

#if FLASH_USE_CUSTOM_DATA && !FLASH_CUSTOM_DATA_READ_ONLY

	// Custom Data extra sichern
	FL_DATA_CUSTOM_START = 0x02;

	FL_DATA_CUSTOM_CRC = crc_calc(&flash_crc_handler,
									fl_data_all + FL_DATA_CUSTOM_START_OFFSET,
									FL_DATA_CUSTOM_CRC_OFFSET - FL_DATA_CUSTOM_START_OFFSET);
	FL_DATA_CUSTOM_END = 0x03;
	
#endif // #if FLASH_USE_CUSTOM_DATA && !FLASH_CUSTOM_DATA_READ_ONLY

	do
	{
#if MCU_TYPE == R5F571 || MCU_TYPE == RSYNERGY
		// RX71M has multiple small dataflash blocks (64 Byte), therefore it must erase multiple blocks
		for(uint32_t i = 0; (BLOCK_DB(i) - BLOCK_DB(0)) < (FL_DATA_CUSTOM_END_OFFSET + 1); i++)
		{
			if(mcu_flash_erase((ERASE_PTR_TYPE) BLOCK_DB(i)))
				DBG_INFO("Erased\n");
			else
				DBG_ERROR("Erase failed\n");
		}
#else
		if(mcu_flash_erase((ERASE_PTR_TYPE) FLASH_INFO_BLOCK))
			DBG_INFO("Erased\n");
		else
			DBG_ERROR("Erase failed\n");
#endif

		if(mcu_flash_write(	(FLASH_PTR_TYPE) FLASH_INFO_BLOCK_START_ADDRESS, (BUF_PTR_TYPE) fl_data_all, sizeof(fl_data_all) ))
		{
			bool error_found = false;
			for(uint16_t i = 0; i < sizeof(fl_data_all) && !error_found; i+=sizeof(_verify_buffer))
			{
				mcu_flash_read(	(FLASH_PTR_TYPE)(FLASH_INFO_BLOCK_START_ADDRESS + i), (BUF_PTR_TYPE)_verify_buffer, sizeof(_verify_buffer));
				for(uint16_t j = 0; j+i < sizeof(fl_data_all) && j < sizeof(_verify_buffer) && !error_found; j++)
					error_found |= (fl_data_all[i+j] != _verify_buffer[j]);
			}
			if(!error_found)
			{
				DBG_INFO("Flash saved OK\n");
				return true;
			}
			else
				DBG_ERROR("Flash saved failed\n");
		}
		else
			DBG_ERROR("Flash save failed\n");

		tries++;

	}while(tries <= FLASH_SAVE_TRIES);

	return false;
}


uint8_t flash_info_get_flash_info_version(void){		return FL_DATA_VERSION;				}		

uint32_t flash_info_get_hardware_id(void){				return FL_DATA_HARDWARE_ID;			}

uint16_t flash_info_get_tested_year(void){				return FL_DATA_TESTED_YEAR;			}

uint8_t flash_info_get_tested_month(void){				return FL_DATA_TESTED_MONTH;		}

uint8_t flash_info_get_tested_day(void){				return FL_DATA_TESTED_DAY;			}

uint8_t flash_info_get_tested_hour(void){				return FL_DATA_TESTED_HOUR;			}

uint8_t flash_info_get_tested_minute(void){				return FL_DATA_TESTED_MINUTE;		}

void flash_info_set_hardware_id(uint32_t id){			FL_DATA_HARDWARE_ID = id;			}

void flash_info_set_tested_date(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute)
{
	FL_DATA_TESTED_YEAR = year;
	FL_DATA_TESTED_MONTH = month;
	FL_DATA_TESTED_DAY = day;
	FL_DATA_TESTED_HOUR = hour;
	FL_DATA_TESTED_MINUTE = minute;
}

void flash_info_set_mac_address(uint8_t* mac)
{
	memcpy(&FL_DATA_MAC_ADDRESS, mac, 6);
}

void flash_info_get_mac_address(uint8_t* mac)
{
	memcpy(mac, &FL_DATA_MAC_ADDRESS, 6);
}
#if FLASH_USE_CUSTOM_DATA

uint32_t* flash_info_get_4_byte_ptr(void)
{
	return (uint32_t*) flash_info_get_byte_array(4);
}

uint16_t* flash_info_get_2_byte_ptr(void)
{
	return (uint16_t*) flash_info_get_byte_array(2);
}

uint8_t* flash_info_get_byte_array(uint16_t size)
{
	if(fl_custom_data_cnt >= (sizeof(fl_data_all) - 3 - size))	// CRC, Endbyte und der Byte Pointer
		return NULL;
	else
	{
		fl_custom_data_cnt += size;
		return (uint8_t*)(fl_data_all + fl_custom_data_cnt - size);
	}
}

#endif

//------------------------------------------------------------------------------------------------------------
// Internal functions
//------------------------------------------------------------------------------------------------------------

static void flash_info_load(void)
{
	uint16_t tmp_crc;

	mcu_flash_read(	(FLASH_PTR_TYPE)(FLASH_INFO_BLOCK_START_ADDRESS + FL_DATA_START_OFFSET),
					(BUF_PTR_TYPE)(fl_data_all + FL_DATA_START_OFFSET),
					FLASH_DATA_SIZE);

	// Lade Flash Info Data
	tmp_crc = crc_calc(	&flash_crc_handler,
						(uint8_t*)(fl_data_all + FL_DATA_START_OFFSET),
						FL_DATA_CRC_OFFSET - FL_DATA_START_OFFSET);

	if(		*((uint8_t*)(fl_data_all + FL_DATA_START_OFFSET)) == 0x02
		&& 	*((uint8_t*)(fl_data_all + FL_DATA_END_OFFSET)) == 0x03
		&& 	*((uint16_t*)(fl_data_all + FL_DATA_CRC_OFFSET)) == tmp_crc
		)
	{
		DBG_INFO("Flash loaded\n");
	}
	else
	{
		memset(fl_data_all + FL_DATA_START_OFFSET, 0, FLASH_DATA_SIZE);
		DBG_ERROR("Flash load failed\n");
	}

#if FLASH_USE_CUSTOM_DATA

	mcu_flash_read(	(FLASH_PTR_TYPE)(FLASH_INFO_BLOCK_START_ADDRESS + FL_DATA_CUSTOM_START_OFFSET),
					(BUF_PTR_TYPE)(fl_data_all + FL_DATA_CUSTOM_START_OFFSET),
					FLASH_CUSTOM_DATA_SIZE);

#if !FLASH_CUSTOM_DATA_READ_ONLY

	// Lade Flash Info Custom Data
	tmp_crc = crc_calc(					&flash_crc_handler,
										(uint8_t*)(fl_data_all + FL_DATA_CUSTOM_START_OFFSET),
										FL_DATA_CUSTOM_CRC_OFFSET - FL_DATA_CUSTOM_START_OFFSET);

	if(		*((uint8_t*)(fl_data_all + FL_DATA_CUSTOM_START_OFFSET)) == 0x02
		&& 	*((uint8_t*)(fl_data_all + FL_DATA_CUSTOM_END_OFFSET)) == 0x03
		&& 	*((uint16_t*)(fl_data_all + FL_DATA_CUSTOM_CRC_OFFSET)) == tmp_crc
		)
	{
		DBG_INFO("Custom Flash loaded\n");
	}
	else
	{
		memset(fl_data_all + FL_DATA_CUSTOM_START_OFFSET, 0, FLASH_CUSTOM_DATA_SIZE);
		DBG_ERROR("Custom Flash load failed\n");
	}

#endif // #if !FLASH_CUSTOM_DATA_READ_ONLY

#endif // #if FLASH_USE_CUSTOM_DATA
}

#endif // #if MCU_PERIPHERY_ENABLE_FLASH
