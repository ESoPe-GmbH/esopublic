/***
 * @file eve_console.c
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"

#if MODULE_ENABLE_GUI && MODULE_ENABLE_CONSOLE

#include "../eve_ui/screen.h"
#include "mcu/sys.h"
#include "module/convert/string.h"
#include "module/console/console.h"
#if MODULE_ENABLE_MMC
#include "module/mmc/mmc.h"
#endif
#if MODULE_ENABLE_RTC
#include "module/rtc/rtc.h"
#endif
#if EVE_COPRO_ENABLE_FLASH_CONSOLE && MODULE_ENABLE_WEBCLIENT
#include "module/network/protocol/http/webclient.h"
#endif
#include "eve_spi.h"
#include "eve_memory.h"

#if EVE_COPRO_ENABLE_FLASH_CONSOLE && MODULE_ENABLE_SECURITY && SECURITY_ENABLE_MBEDTLS
#include "mbedtls/sha256.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define DIR_SCREEN	"screen"

/// Number of bytes to reserve in RAM of eve to store downloaded data before writing it into the flash.
#define FLASH_RAM_SIZE		4096

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static FUNCTION_RETURN_T _execute(console_data_t* data, char** args, uint16_t args_len);

#if EVE_COPRO_ENABLE_DUMP

extern void eve_copro_tag_interrupt(eve_t* obj, uint8_t tag);	// Description in eve_copro.c

#if MODULE_ENABLE_MMC
static bool debug_dump_open_file(console_data_t* data, FIL* f);

static void debug_dump_close_file(console_data_t* data, FIL* f);
#endif

static void debug_dump_eve(console_data_t* data, comm_t* c);
#endif

#if EVE_COPRO_ENABLE_FLASH_CONSOLE && MODULE_ENABLE_WEBCLIENT
static void _webclient_cb_event(webclient_t* wc, WEBCLIENT_EVENT_T event, const webclient_event_data_t* event_data);
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static console_command_t _console_command =
{
	.command = "eve",
	.use_array_param = true,
	.fnc_exec = _execute,	
#if EVE_COPRO_ENABLE_DUMP
	.explanation = "eve dump -> Saves the current displaylist to mmc\neve dump print -> Prints the current displaylist to the comm interface.",
#else
	.explanation = "No commands available at the moment. For dumps set EVE_COPRO_ENABLE_DUMP to true",
#endif
};

#if EVE_COPRO_ENABLE_DUMP

static comm_t debug_dump_comm_handler;
#if MODULE_ENABLE_MMC
static bool debug_dump_to_file = true;
#endif
static rtc_time_t debug_dump_time;

#if MODULE_ENABLE_MMC
static FIL _f;

static uint8_t _buffer_ram[512];
#endif

/// Buffer for storing the dl list when dumps are needed.
/// Not static because it is used externally in eve_copro.
uint32_t eve_console_dl_list[2048];

/// Flag for enabling live views by sending Swap for each display swap
/// Not static because it is used externally in eve_copro.
bool eve_console_enable_live = false;

#endif

#if EVE_COPRO_ENABLE_FLASH_CONSOLE && MODULE_ENABLE_WEBCLIENT
/// @brief Pointer for the webclient that is used to download files
static webclient_t* _webclient;

static uint32_t _ram_address;

static uint32_t _flash_address;

static uint32_t _written_to_ram = 0;
#endif

#if EVE_COPRO_ENABLE_FLASH_CONSOLE && MODULE_ENABLE_SECURITY && SECURITY_ENABLE_MBEDTLS
static mbedtls_sha256_context _sha256;
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void eve_console_init(eve_t* eve)
{
#if EVE_COPRO_ENABLE_FLASH_CONSOLE && MODULE_ENABLE_WEBCLIENT
	_ram_address = eve_memory_get_address(eve, FLASH_RAM_SIZE);
	eve_memory_register_address(eve, _ram_address, FLASH_RAM_SIZE);
#endif
	console_add_command(&_console_command);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static FUNCTION_RETURN_T _execute(console_data_t* data, char** args, uint16_t args_len)
{
#if EVE_COPRO_ENABLE_DUMP
	eve_t* eve = &screen_get_default_device()->eve;
#endif
	if(args_len == 0)
	{
		return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Missing arguments for eve");
	}

#if EVE_COPRO_ENABLE_FLASH_CONSOLE
	if(strcmp(args[0], "flash") == 0)
	{
		if(args_len == 1)
		{
			return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Missing arguments for eve flash");
		}

		if(strcmp(args[1], "erase") == 0)
		{
			eve_t* eve = &screen_get_default_device()->eve;
			FUNCTION_RETURN ret;
			eve_copro_flash_erase(eve);
			uint32_t timestamp = system_get_tick_count();
			do
			{
				ret = eve_copro_wait_for_execution(eve);
#if MCU_PERIPHERY_ENABLE_WATCHDOG
				mcu_watchdog_trigger();
#endif
			}while(ret == FUNCTION_RETURN_TIMEOUT && (system_get_tick_count() - timestamp) < 30000);

			return console_set_response_static(data, ret, "flash erase");
		}

#if EVE_COPRO_ENABLE_FLASH_CONSOLE && MODULE_ENABLE_SECURITY && SECURITY_ENABLE_MBEDTLS
		if(strcmp(args[1], "hash") == 0)
		{
			// Calculate and return the hash over a certain area in eve memory
			// eve hash <address> <length>

			if(!(args[2][0] = '0' && args[2][1] == 'x'))
			{
				return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Argument 2 of eve flash hash has to start with 0x");
			}

			eve_t* eve = &screen_get_default_device()->eve;
			uint32_t flash_address = strtoul(&args[2][2], NULL, 16);
			uint32_t length = strtoul(args[3], NULL, 10);
			uint8_t* buffer = mcu_heap_calloc(1, 4096);
			uint8_t hash[32];
			if(buffer == NULL)
			{
				return console_set_response_static(data, FUNCTION_RETURN_INSUFFICIENT_MEMORY, "Cannot allocate buffer");
			}

			mbedtls_sha256_init(&_sha256);
			mbedtls_sha256_starts_ret(&_sha256, false);

			for(int i = 0; i < length; i += 4096)
			{
				uint32_t remaining = length - i;
				if(remaining > 4096)
				{
					remaining = 4096;
				}
				eve_copro_flash_read_to_ram(eve, _ram_address, flash_address + i, 4096);
				eve_spi_read_data(eve, _ram_address, buffer, remaining);
				mbedtls_sha256_update_ret(&_sha256, buffer, remaining);
#if MCU_PERIPHERY_ENABLE_WATCHDOG
				mcu_watchdog_trigger();
#endif
				if(i > 0 && (i & 0xFFFFF) == 0)
				{
					DBG_INFO("Processed %d of %d\n", i, length);
				}
			}
			
			mbedtls_sha256_finish_ret(&_sha256, hash);
			mbedtls_sha256_free(&_sha256);

			mcu_heap_free(buffer);

			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 200, "flash hash %08x %u %32Q", flash_address, length, hash);
		}
#endif
#if MODULE_ENABLE_WEBCLIENT
		if(strcmp(args[1], "download") == 0)
		{
			// Download a file and store it on a certain address in eve memory
			// eve download <address> <url>

			if(!(args[2][0] = '0' && args[2][1] == 'x'))
			{
				return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Argument 2 of eve flash download has to start with 0x");
			}

			_flash_address = strtoul(&args[2][2], NULL, 16);

			if(_webclient)
			{
				return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Webclient is active");
			}

			_webclient = webclient_create(0);
#if EVE_COPRO_ENABLE_FLASH_CONSOLE && MODULE_ENABLE_SECURITY && SECURITY_ENABLE_MBEDTLS
			mbedtls_sha256_init(&_sha256);
			mbedtls_sha256_starts_ret(&_sha256, false);
#endif

			webclient_options_t options = 
			{
				.mode = WEBCLIENT_MODE_CALLBACK,
				.url = args[3],
				.cb_event = _webclient_cb_event,
			};
			FUNCTION_RETURN ret = webclient_download(_webclient, &options);
			return console_set_response_static(data, ret, "flash download");
		}
#endif
	}
#endif

	if(strcmp(args[0], "video") == 0)
	{
		uint32_t flash_address = strtoul(&args[1][2], NULL, 16);
		eve_t* eve = &screen_get_default_device()->eve;
		eve_copro_flashsource(eve, flash_address);
		eve_copro_playvideo(eve, EVE_OPT_PLAYVIDEO_FULLSCREEN | EVE_OPT_PLAYVIDEO_FLASH | EVE_OPT_PLAYVIDEO_SOUND | EVE_OPT_PLAYVIDEO_NOTEAR, NULL, 0);
		return console_set_response_static(data, FUNCTION_RETURN_OK, "video");
	}

	if(args_len == 3 && strcmp(args[0], "audio") == 0 && strcmp(args[1], "volume") == 0)
	{
		uint32_t volume = strtoul(args[2], NULL, 16);
		eve_t* eve = &screen_get_default_device()->eve;

		eve_spi_write_8(eve, EVE_REG_VOL_SOUND, volume);
		eve_spi_write_8(eve, EVE_REG_VOL_PB, volume);
		
		return console_set_response_static(data, FUNCTION_RETURN_OK, "audio volume");
	}

#if EVE_COPRO_ENABLE_DUMP
	if(strcmp(args[0], "dump") == 0)
	{
		comm_t* c = &debug_dump_comm_handler;

	#if MODULE_ENABLE_MMC
		debug_dump_to_file = !(args_len > 1 && (strcmp(args[1], "print") == 0));
	#endif

	#if MODULE_ENABLE_MMC
		if(debug_dump_to_file)
			mmc_create_comm_handler(&_f, &debug_dump_comm_handler);
		else
			c = data->comm;
	#else
		c = data->comm;
	#endif

	#if MODULE_ENABLE_MMC
		if(debug_dump_open_file(data, c->device_handler))
		{
			debug_dump_eve(data, c);
			debug_dump_close_file(data, c->device_handler);
		}
	#else
		debug_dump_eve(data, c);
	#endif
	}
	else if(strcmp(args[0], "live") == 0)
	{
		eve_console_enable_live = (args[1][0] == '1');
	}
	else if(strcmp(args[0], "memrequest") == 0)
	{
		eve->memory_changed = true;
	}
	else if(strcmp(args[0], "tag") == 0 && args_len == 2)
	{
		eve_copro_tag_interrupt(eve, strtol(args[1], NULL, 10));
		eve_copro_tag_interrupt(eve, 0);
	}
	else
		comm_printf(data->comm, "Unknown subcommand %s. Is the subcomman enabled per define?\n", args[0]);
#else
	comm_printf(data->comm, "Unknown subcommand %s. Is the subcommand enabled per define?\n", args[0]);
#endif
	return FUNCTION_RETURN_OK;
}


#if EVE_COPRO_ENABLE_DUMP

#if MODULE_ENABLE_MMC
static bool debug_dump_open_file(console_data_t* data, FIL* f)
{
	FRESULT ret = FR_INVALID_OBJECT;
	if(debug_dump_to_file)
	{
#if MODULE_ENABLE_RTC
		rtc_get_time(&debug_dump_time);
#else
		memset(&debug_dump_time, 0, sizeof(debug_dump_time));
#endif

		string_printf((char*)_buffer_ram, DIR_SCREEN "/%04d-%02d-%02d[%02d_%02d_%02d].eve", debug_dump_time.tm_year + RTC_EPOCH_YR, debug_dump_time.tm_mon + 1, debug_dump_time.tm_mday, debug_dump_time.tm_hour, debug_dump_time.tm_min, debug_dump_time.tm_sec);

		mmc_mkdir(DIR_SCREEN, false);

		ret = mmc_open_file((char*)_buffer_ram, f, OPEN_NORMAL);
		if(ret == FR_OK)
		{
			comm_printf(data->comm, "Dump to file \"%s\"\n", _buffer_ram);
			return true;
		}
		else
		{

			comm_printf(data->comm, "Cannot open dump file %d\n", ret);
			return false;
		}
	}

	return true;
}

static void debug_dump_close_file(console_data_t* data, FIL* f)
{
	if(debug_dump_to_file)
		mmc_close_file(f);
	comm_printf(data->comm, "Dump finished\n");
}
#endif

static void debug_dump_eve(console_data_t* data, comm_t* c)
{
	uint32_t i;
	uint32_t val = 0xFFFFFFFF;
	uint32_t size = 0;
	uint32_t ramsize = 0;
	bool needs_ram = false;
	eve_t* eve = &screen_get_default_device()->eve;
#if EVE_USE_FT81X
	static const char* ft8xx_num = "810";
#else
	static const char* ft8xx_num = "800";
#endif

	for(i = 0; i < sizeof(eve_console_dl_list) / sizeof(uint32_t) && val > 0; i++)
	{
		val = eve_console_dl_list[i];
//		comm_printf(c, "D=%08x\n", (uint32_t)val);
		size += 4;
		needs_ram |= ((val & 0xFF000000) == 0x01000000); // If list contains BITMAP_SOURCE something with drawing from ram is used on screen
	}
//	comm_printf(c, "D=d\n", size); // Number of bytes to send as line

	if(needs_ram && eve->memory_changed)
		ramsize = eve->memory.ram_pointer - EVE_RAM_G;
	else
		ramsize = 0;

	comm_printf(c, "Dump FT%s[w=%d, h=%d, r=%d, d=%d]\n",
			ft8xx_num,
			screen_device_get_width(screen_get_default_device()),
			screen_device_get_height(screen_get_default_device()),
			ramsize,
			size);

	if(ramsize > 0)
	{
		uint32_t offset = 0;
		eve->memory_changed = false;
//		comm_printf(c, "R=d\n", ramsize); // Number of bytes to send as line
		for(i = 0; i < (eve->memory.ram_pointer - EVE_RAM_G); i+=sizeof(_buffer_ram))
		{
			uint32_t to_send = ramsize - offset;
			if(to_send > sizeof(_buffer_ram))
				to_send = sizeof(_buffer_ram);
			offset += to_send;

			eve_spi_read_data(eve, EVE_RAM_G + i, _buffer_ram, to_send);
			comm_put(c, _buffer_ram, to_send);
//			comm_printf(c, "R=%#Q\n", sizeof(_buffer_ram), _buffer_ram);
		}
	}

	comm_put(c, (uint8_t*)eve_console_dl_list, size); // DL List as binary
}

#endif

#if EVE_COPRO_ENABLE_FLASH_CONSOLE && MODULE_ENABLE_WEBCLIENT
static void _webclient_cb_event(webclient_t* wc, WEBCLIENT_EVENT_T event, const webclient_event_data_t* event_data)
{
	eve_t* eve = &screen_get_default_device()->eve;
	size_t length = event_data->length;

	switch(event)
	{
		case WEBCLIENT_EVENT_HEADER_RECEIVED:
			DBG_INFO("Start downloading %u bytes\n", event_data->content_length);
		break;

		case WEBCLIENT_EVENT_DOWNLOAD:
			// DBG_INFO("Download %u bytes (%u of %u)\n", event_data->length, event_data->content_received, event_data->content_length);
#if EVE_COPRO_ENABLE_FLASH_CONSOLE && MODULE_ENABLE_SECURITY && SECURITY_ENABLE_MBEDTLS
			mbedtls_sha256_update_ret(&_sha256, event_data->data, event_data->length);
#endif
			size_t offset = 0;
			while(length > 0)
			{
				size_t length_to_write = length;
				if(length_to_write + _written_to_ram > FLASH_RAM_SIZE)
				{
					length_to_write = FLASH_RAM_SIZE - _written_to_ram;
				}
				
				// DBG_INFO("Write %d bytes to %08x\n", length_to_write, _ram_address + _written_to_ram);
				eve_spi_write_data(eve, _ram_address + _written_to_ram, &event_data->data[offset], length_to_write, false);

				length -= length_to_write;
				offset += length_to_write;

				if(_written_to_ram + length_to_write == FLASH_RAM_SIZE)
				{			
					// Update frame
					// DBG_INFO("Write flash at %08x\n", _flash_address);
					eve_copro_flash_update_from_ram(eve, _ram_address, _flash_address, FLASH_RAM_SIZE);
					_flash_address += FLASH_RAM_SIZE;
					_written_to_ram = 0;
				}
				else
				{
					_written_to_ram += length_to_write;
				}		
			}
		break;

		case WEBCLIENT_EVENT_FINISHED:
			if(event_data->res == WEBCLIENT_RESULT_SUCCESS)
			{
#if EVE_COPRO_ENABLE_FLASH_CONSOLE && MODULE_ENABLE_SECURITY && SECURITY_ENABLE_MBEDTLS
				uint8_t hash[32];
				mbedtls_sha256_finish_ret(&_sha256, hash);
				mbedtls_sha256_free(&_sha256);
				DBG_INFO("Download finished successfully. SHA-256: %32Q\n", hash);
#else
				DBG_INFO("Download finished successfully\n");
#endif
				if(_written_to_ram > 0)
				{
					// Pad the other ram data in this sector to be 0, since currently it holds the data of previous write.
					size_t padding = FLASH_RAM_SIZE - _written_to_ram;
					if(padding > 0)
					{
						uint8_t* buffer = mcu_heap_calloc(1, padding);
						if(buffer)
						{
							eve_spi_write_data(eve, _ram_address + _written_to_ram, buffer, padding, false);
							mcu_heap_free(buffer);
						}
					}
					// Update flash with last data
					// DBG_INFO("Write flash at %08x\n", _flash_address);
					eve_copro_flash_update_from_ram(eve, _ram_address, _flash_address, FLASH_RAM_SIZE);
					_flash_address += _written_to_ram;
				}
			}
			else
			{
				DBG_ERROR("Download failed: %u\n", event_data->res);
			}
		break;

		case WEBCLIENT_EVENT_DONE:
			DBG_INFO("Free webclient\n");
			webclient_free(_webclient);
			_webclient = NULL;
		break;
	}

}
#endif

#endif

