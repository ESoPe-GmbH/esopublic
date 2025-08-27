/**
 * @file eve_memory.c
 * @copyright Urheberrecht 2017-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "eve_memory.h"
#include "eve.h"
#include "eve_spi.h"
#include "eve_copro.h"
#include "module/comm/dbg.h"
#include <string.h>
#if EVE_MMC_READ_BUFFER_SIZE > 0
#include "module/mmc/mmc.h"
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

#if EVE_MMC_READ_BUFFER_SIZE > 0
/// Temporary file object for loading files into memory.
static FIL _f;

/// Buffer used for loading files into memory
static uint8_t _mmc_read_buffer[EVE_MMC_READ_BUFFER_SIZE];
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if EVE_MMC_READ_BUFFER_SIZE > 0
/**
 * @brief	Loads a file from the sd card and writes it into the ram of the eve chip.
 *
 * @param eve		Pointer to the eve
 * @param obj		Pointer to the eve memory object that should be loaded.
 * @return 			true if it worked or false if an error occured.
 * 					An error might be if the file does not exist or when the ram has not enough space to store the file content.
 */
static bool _load_from_file(eve_t* eve, eve_memory_file_t* obj);
#endif

/**
 * @brief	Loads a file from the rom and writes it into the ram of the eve chip.
 *
 * @param obj		Pointer to the eve memory object that should be loaded.
 * @return 			true if it worked or false if an error occured.
 * 					An error might be a NULL pointer or when the ram has not enough space to store the file content.
 */
static bool _load_from_flash(eve_t* eve, eve_memory_file_t* obj);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void eve_memory_clear(eve_t* eve)
{
	uint16_t i;
	memset(&eve->memory, 0, sizeof(eve_memory_t));
	// TODO: Implement logic
	eve->memory.ram_pointer = EVE_RAM_G;
	dbg_printf(DBG_STRING, "EVE Memory Clear\n");

	for(i = 0; i < EVE_MEMORY_FILES_MAX; i++)
		eve->memory_files[i].address = (uint32_t)-1;

#if EVE_COPRO_ENABLE_DUMP
	eve->memory_changed = true;
#endif
}

eve_memory_file_t* eve_memory_register(eve_t* eve, const char* filename, const uint8_t* data, uint32_t length)
{
	uint16_t i;
	if(eve == NULL)
		return NULL;

	for(i = 0; i < EVE_MEMORY_FILES_MAX; i++)
	{
		// New memory object!
		if(eve->memory_files[i].filename == NULL)
		{
			eve->memory_files[i].filename = filename;
			eve->memory_files[i].data = data;
			eve->memory_files[i].data_length = length;
			eve->memory_files[i].flash_address = 0;	// Set to invalid address
//			dbg_printf(DBG_STRING, "Register %s at %d\n", filename, i);
			return &eve->memory_files[i];
		}
		if(strcmp(eve->memory_files[i].filename, filename) == 0)
		{
//			dbg_printf(DBG_STRING, "Return %s at %d\n", filename, i);
			return &eve->memory_files[i];
		}
	}
	return NULL;
}

eve_memory_file_t* eve_memory_register_from_external_flash(eve_t* eve, const char* filename, uint32_t flash_address, uint32_t length)
{
	uint16_t i;
	if(eve == NULL)
		return NULL;

	for(i = 0; i < EVE_MEMORY_FILES_MAX; i++)
	{
		// New memory object!
		if(eve->memory_files[i].filename == NULL)
		{
			eve->memory_files[i].filename = filename;
			eve->memory_files[i].flash_address = flash_address;
			eve->memory_files[i].data = NULL;
			eve->memory_files[i].data_length = length;
//			dbg_printf(DBG_STRING, "Register %s at %d\n", filename, i);
			return &eve->memory_files[i];
		}
		if(strcmp(eve->memory_files[i].filename, filename) == 0)
		{
//			dbg_printf(DBG_STRING, "Return %s at %d\n", filename, i);
			return &eve->memory_files[i];
		}
	}
	return NULL;
}

uint32_t eve_memory_get_address(eve_t* eve, uint32_t space_needed)
{
	// If space exceeds usable RAM -> return error -1
	if((eve->memory.ram_pointer + space_needed) > EVE_RAM_SIZE)
		return (uint32_t)-1;

	// TODO: Implement logic
	return (eve->memory.ram_pointer + 3) & 0xFFFFFFFC; // Align 4-Byte
}

uint32_t eve_memory_write(eve_t* eve, const uint8_t* data, uint32_t size)
{
	uint32_t address = eve_memory_get_address(eve, size);
	if(address == (uint32_t)-1)
		return address;

	eve_memory_write_to(eve, address, data, size);

	return address;
}

void eve_memory_write_to(eve_t* eve, uint32_t address, const uint8_t* data, uint32_t size)
{
//	dbg_printf(DBG_STRING, "Write %d bytes[%08x] to %08x\n", size, data, address);

	eve_spi_write_data(eve, address, data, size, false);
	eve_memory_register_address(eve, address, size);
}

void eve_memory_register_address(eve_t* eve, uint32_t address, uint32_t size)
{
	// TODO: Implement logic
	eve->memory.ram_pointer += size;

	// Align 4-Byte
	eve->memory.ram_pointer += 3;
	eve->memory.ram_pointer &= 0xFFFFFFFC;

#if EVE_COPRO_ENABLE_DUMP
	eve->memory_changed = true;
#endif

//	dbg_printf(DBG_STRING, "%d bytes in use\n", eve->memory.ram_pointer);
}

bool eve_memory_write_file_to(eve_t* eve, eve_memory_file_t* mfo)
{
	if(eve == NULL || mfo == NULL)
		return false;

	if(mfo->data == NULL && mfo->flash_address == 0)
	{
#if MODULE_ENABLE_MMC
		return _load_from_file(eve, mfo);
#else
		return false;
#endif
	}
	else
		return _load_from_flash(eve, mfo);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


#if EVE_MMC_READ_BUFFER_SIZE > 0
static bool _load_from_file(eve_t* eve, eve_memory_file_t* obj)
{
	uint32_t fsize = 0;
	uint32_t bytes_written = 0;
	uint32_t fsize2 = 0;
	bool ret_value = true;

	// If an address was set, the image was written into the memory
	// It should be ensured that removing the file from the ram should invalidate the address.
	if(obj->address != 0xFFFFFFFF)
		return true;

//	dbg_printf(DBG_STRING, "image_load_from_file(%s)\n", obj->filename);

	if(!mmc_is_inserted(obj->filename))
	{
#if EVE_DEBUG_ERROR
		dbg_printf(DBG_STRING, "No SD-card inserted\n");
#endif
		return false;		// Cannot load image without mmc
	}

	if(FR_OK != mmc_open_existing_file(obj->filename, &_f, OPEN_NORMAL))
	{
#if EVE_DEBUG_ERROR
		dbg_printf(DBG_STRING, "File %s could not be opened\n");
#endif
		return false;		// Cannot load image when file cannot be opened
	}

	fsize = mmc_get_filesize(&_f);
	fsize2 = fsize;

//	dbg_printf(DBG_STRING, "File %s opened: %d Bytes\n", obj->filename, fsize);


	if(fsize != obj->data_length)
	{
#if EVE_DEBUG_ERROR
		dbg_printf(DBG_STRING, "Filesize %s does not match the format!\n", obj->filename);
#endif
		mmc_close_file(&_f);
		return false;
	}

	fsize2 = (fsize + 3) & 0xFFFFFFFC;	// 4 Byte alignment
	obj->address = eve_memory_get_address(eve, fsize2);

	if(obj->address == 0xFFFFFFFF)
	{
		mmc_close_file(&_f);
#if EVE_DEBUG_ERROR
		dbg_printf(DBG_STRING, "File %s does not fit in memory\n", obj->filename);
#endif
		return false;
	}
//	dbg_printf(DBG_STRING, "Current Pointer: %u\n", obj->address);

	// Start transmitting jpg
	while(fsize > 0 && ret_value)
	{
		uint16_t len = EVE_MMC_READ_BUFFER_SIZE;
		if(fsize < len)
			len = fsize;
		fsize -= len;

		if(len != mmc_gets(&_f, _mmc_read_buffer, len))
		{
			mmc_close_file(&_f);
#if EVE_DEBUG_ERROR
		dbg_printf(DBG_STRING, "File %s has a read error\n", obj->filename);
#endif
			ret_value = false;
			break;
		}

		eve_memory_write_to(eve, obj->address + bytes_written, _mmc_read_buffer, len);

		bytes_written += len;
	}

	mmc_close_file(&_f);

//	dbg_printf(DBG_STRING, "Bytes loaded: %u\n", bytes_written);
//	dbg_printf(DBG_STRING, "Next Pointer: %u\n", eve->memory.ram_pointer);


	if(obj->address != 0xFFFFFFFF)
		eve_memory_register_address(eve, obj->address, fsize2);

	return ret_value;
}
#endif

static bool _load_from_flash(eve_t* eve, eve_memory_file_t* obj)
{
	// If an address was set, the image was written into the memory
	// It should be ensured that removing the file from the ram should invalidate the address.
	if(obj->address != 0xFFFFFFFF)
		return true;

//	dbg_printf(DBG_STRING, "_load_from_flash(%s)\n", obj->filename);

	if((obj->data == NULL && obj->flash_address == 0) || obj->data_length == 0)
	{
#if EVE_DEBUG_ERROR
		dbg_printf(DBG_STRING, "Data not available\n");
#endif
		return false;
	}

	// Get a new address
	obj->address = eve_memory_get_address(eve, (obj->data_length + 3) & 0xFFFFFFFC);

	// If no address is set -> File cannot be loaded
	if(obj->address == 0xFFFFFFFF)
	{
#if EVE_DEBUG_ERROR
		dbg_printf(DBG_STRING, "File %s does not fit in memory\n", obj->filename);
#endif
		return false;
	}

	// dbg_printf(DBG_STRING, "Current Pointer for %s: %u\n", obj->filename, obj->address);

	if(obj->data != NULL)
	{
		// Write the data from the flash into the eve memory
		eve_memory_write_to(eve, obj->address, obj->data, obj->data_length);
	}
	else
	{
		// Write the data from the flash into the eve memory	
		eve_copro_flash_read_to_ram(eve, obj->address, obj->flash_address, obj->data_length);
	}

//	dbg_printf(DBG_STRING, "Bytes loaded: %u\n", obj->data_length);
//	dbg_printf(DBG_STRING, "Next Pointer: %u\n", eve->memory.ram_pointer);

	return true;
}

#endif
