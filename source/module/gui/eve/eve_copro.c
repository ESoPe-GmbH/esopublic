/***
 * @file eve_copro.c
 * @copyright Urheberrecht 2015-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "eve_copro.h"
#include "eve_register.h"
#include "eve_spi.h"
#include "eve.h"
#include "../eve_ui/font.h"
#include "../eve_ui/component.h"
#include "../eve_ui/screen.h"
#include "mcu/sys.h"
#include "module/comm/dbg.h"
#include "module/convert/math.h"

#if EVE_COPRO_ENABLE_SNAPSHOT && MODULE_ENABLE_MMC && MODULE_ENABLE_IMAGE
#include "module/mmc/mmc.h"
#include "module/image/bitmap.h"
#endif

#include <string.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Buffer used for multi lines
#define _TEMPORARY_TEXT_BUFFER_SIZE		100

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if _TEMPORARY_TEXT_BUFFER_SIZE > 0

/// Temporary buffer used for multi-line text
static char _tmp_text_buffer[_TEMPORARY_TEXT_BUFFER_SIZE];

#endif

#if EVE_COPRO_ENABLE_DUMP

extern uint32_t eve_console_dl_list[2048];

extern bool eve_console_enable_live;

#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Writes a 32-bit value to the command buffer and increments the command buffer pointer by 4.
 * 			Does not check the space of the buffer, so this must be checked internally.
 *
 * @param val				32-bit value that should be written into the command buffer.
 */
static void eve_copro_internal_write_command(eve_t* eve, uint32_t val);

// TODO: Description
static void eve_copro_internal_write_command_data(eve_t* eve, uint32_t val, const uint32_t* options, uint32_t num_options, const uint8_t* data, uint32_t sizeof_data);

static void _process_string(char* str, uint32_t len);
/**
 * @brief	Writes a string into the command buffer and increments the command buffer pointer by the length of the string
 * 			+ 1 for the terminating zero. So the string must be terminated with 0.
 *
 * @param str				0-terminated string that should be written into the buffer.
 * @param len				Length of the string + 1 for the 0-termination.
 */
static void eve_copro_write_string(eve_t* eve, const char *str, uint16_t len);

/**
 * @brief	Loads a ram font into a bitmap handler if needed
 * @param eve			Pointer to eve device
 * @param font			Font that is used
 */
static void _load_font(eve_t* eve, uint8_t font);

/**
 * @brief	Increments the write pointer to the command buffer by >count<, where count is the parameter.
 * 			Does align the write pointer to a 4-byte alignment internally. Since the command buffer is a 4096 byte
 * 			ringbuffer, the write pointer starts at 0 after 4095 bytes.
 *
 * @param count				Number of bytes that were written into the command buffer.
 */
static void eve_copro_increment_write_pointer(eve_t* eve, uint16_t count);

/**
 * @brief	Writes the command buffer write pointer into the write pointer register of the eve, which starts the co-processor
 * 			to work through the command buffer.
 */
static void ft_component_update_write_pointer(eve_t* eve);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Module Prototypes -> Functions that are called from other source files inside this module
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief 	Function is called from eve_spi if a command complete interrupt occurs. Sets the flag for the command complete
 * 			to true, which indicates that the screen can be repainted.
 */
void eve_copro_set_command_complete(eve_t* eve);

/**
 * @brief 	Function is called from eve_spi if a tag interrupt occurs. The tag indicates if a button is pressed or if
 * 			the finger is removed from the button.
 *
 * @param tag			0xFF: The screen is touched on a position that is no button
 * 						0x00: The screen is not touched
 * 						0x01 - 0xFE: Tag number of the touched component
 */
void eve_copro_tag_interrupt(eve_t* eve, uint8_t tag);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void eve_copro_reset(eve_t* eve)
{
	// Reset coprocessor
	eve_spi_write_8(eve, EVE_REG_CPURESET, 1);
	// Clear read/write pointer
	eve_spi_write_32(eve, EVE_REG_CMD_READ, 0);
	eve_spi_write_32(eve, EVE_REG_CMD_WRITE, 0);
	// Clear display list
	eve_spi_write_32(eve, EVE_REG_CMD_DL, 0);
	// End the reset
	eve_spi_write_8(eve, EVE_REG_CPURESET, 0);
	// Wait for reset to finish
	mcu_wait_us(10000);
	// Set the api level
	eve_copro_set_apilevel(eve, eve->api_level);
}

void eve_copro_handle(eve_t* eve)
{
	if(eve == NULL)
		return;

	if(eve->eve_copro_release_touch)
	{
		if( (system_get_tick_count() - eve->eve_copro_touch_timestamp) >= EVE_COPRO_TOUCH_MIN_MS )
		{
			eve->eve_copro_release_touch = false;
			eve_copro_touch_released(eve);
		}
	}
}

#if EVE_COPRO_USE_CALIBRATION
void eve_copro_calibrate_touch(eve_t* eve)
{
	uint32_t touch_a, touch_b, touch_c, touch_d, touch_e, touch_f;
	eve_copro_write_command(eve, 0xffffff00);
	eve_copro_write_command(eve, 0xffffff15);
	eve_copro_write_command(eve, 0xffffff01);
	ft_component_update_write_pointer(eve);
	while(eve_spi_read_16(eve, EVE_REG_CMD_READ) != eve_spi_read_16(eve, EVE_REG_CMD_WRITE));
	touch_a = eve_spi_read_32(eve, EVE_REG_TOUCH_TRANSFORM_A);
	touch_b = eve_spi_read_32(eve, EVE_REG_TOUCH_TRANSFORM_B);
	touch_c = eve_spi_read_32(eve, EVE_REG_TOUCH_TRANSFORM_C);
	touch_d = eve_spi_read_32(eve, EVE_REG_TOUCH_TRANSFORM_D);
	touch_e = eve_spi_read_32(eve, EVE_REG_TOUCH_TRANSFORM_E);
	touch_f = eve_spi_read_32(eve, EVE_REG_TOUCH_TRANSFORM_F);
	DBG_INFO("A: 0x%08x\n", touch_a);
	DBG_INFO("B: 0x%08x\n", touch_b);
	DBG_INFO("C: 0x%08x\n", touch_c);
	DBG_INFO("D: 0x%08x\n", touch_d);
	DBG_INFO("E: 0x%08x\n", touch_e);
	DBG_INFO("F: 0x%08x\n", touch_f);
}
#endif

void eve_copro_set_touch_callback(eve_t* eve, eve_copro_touch_cb_t f, void* f_obj)
{
	if(eve == NULL)
		return;

	eve->eve_copro_touch_callback = f;
	eve->eve_copro_touch_callback_object = f_obj;
}

void eve_copro_set_key_callback(eve_t* eve, void(*f)(void*, char), void* f_obj)
{
	if(eve == NULL)
		return;

	eve->key_callback = f;
	eve->key_callback_object = f_obj;
}

bool eve_copro_is_ready(eve_t* eve)
{
	if(eve == NULL)
		return false;

	return eve->eve_copro_is_ready_flag;
}

void eve_copro_write_command(eve_t* eve, uint32_t val)
{
	if(eve == NULL)
			return;

#if !EVE_USE_FT81X

	// To avoid incompatibilities because VERTEX_FORMAT is not supported by EVE, the value is simulated here

	if(val == 0x01) // Value for macros not needed on EVE
	{
		return;
	}
	else if((val & 0xFF000000) == EVE_VERTEX_FORMAT(0))
	{
		eve->vertex_format = val & 0x07;
//		dbg_printf(DBG_STRING, "VertexFormat=%d\n", eve->vertex_format);
		return;
	}
	else if((val & 0xF0000000) == EVE_VERTEX2F(0, 0))
	{
		int32_t x, y;
		// Extract current values for the command
		y = (val & 0x00007FFF);
		x = (val & 0x3FFF8000) >> 15;
		// Make the vertex format correct
		// 4 = 1/16 pixel precision which is the only supported value on EVE
		// 3 = 1/18 pixel precision -> Shift current x and y values by one to get the 1/16 pixel precision
		// ..
		// 1 = 1/1 pixel precision -> Shift current x and y values by 4 to get the 1/16 pixel precision
//		dbg_printf(DBG_STRING, "A VERTEX2F(%d, %d)\n", x, y);
		x <<= (4-eve->vertex_format);
		y <<= (4-eve->vertex_format);
		// Create command with new values
		val = EVE_VERTEX2F(x, y);
	}
	else if((val & 0xFF000000) == EVE_VERTEX2II(0, 0, 0, 0))
	{
		int32_t x, y, c, h, i;
		// Extract current values for the command
		c = (val & 0x0000007F);
		h = (val & 0x00000F80) >> 7;
		y = (val & 0x001FF000) >> 12;
		x = (val & 0x3FE00000) >> 21;
		// Make the vertex format correct
		// 4 = 1/16 pixel precision which is the only supported value on EVE
		// 3 = 1/18 pixel precision -> Shift current x and y values by one to get the 1/16 pixel precision
		// ..
		// 1 = 1/1 pixel precision -> Shift current x and y values by 4 to get the 1/16 pixel precision
//		dbg_printf(DBG_STRING, "A EVE_VERTEX2II(%d, %d)\n", x, y);
		for(i = 0; i < 4 - eve->vertex_format; i++)
		{
			x <<= 1;
			y <<= 1;
		}
//		dbg_printf(DBG_STRING, "A EVE_VERTEX2II(%d, %d)\n", x/16, y/16);
		// Create command with new values
		val = EVE_VERTEX2II(x, y, h, c);
	}

#endif

	eve_copro_check_command_buffer(eve, 4);
	eve_copro_internal_write_command(eve, val);
}

void eve_copro_write_commands(eve_t* eve, uint32_t* commands, uint32_t num_commands)
{
	eve_copro_check_command_buffer(eve, num_commands * 4);
	eve_spi_write_data(eve, EVE_REG_CMDB_WRITE, (uint8_t*)commands, num_commands * 4, false);
}

void eve_copro_write_command_data(eve_t* eve, const eve_copro_command_t* command)
{
	// TODO: Implement
}

bool eve_copro_has_empty_list(eve_t* eve)
{
	return eve_spi_read_16_twice(eve, EVE_REG_CMDB_SPACE) == 4092;
}

FUNCTION_RETURN eve_copro_wait_for_execution(eve_t* eve)
{
	return eve_copro_check_command_buffer(eve, 4092);
}

void eve_copro_dlstart(eve_t* eve, color_t c)
{
	uint16_t i;
	if(eve == NULL)
		return;
	eve->col = c;
	for(i = 0; i < 32; i++)
		eve->memory.font[i].handler_is_loaded = false;

#if EVE_COPRO_DEBUG_COMMAND_COUNT
	eve->eve_copro_cmd_cnt = 0;
#endif
	eve_copro_write_command(eve, 0xffffff00);
	eve_copro_write_command(eve, EVE_CLEAR_COLOR_RGB(c.r, c.g, c.b));
	eve_copro_write_command(eve, EVE_CLEAR(1,1,1));
	eve->eve_copro_current_button_cnt = 0;
#if !EVE_USE_FT81X
	eve->vertex_format = 4;
#endif
}

void eve_copro_coldstart(eve_t* eve)
{
	if(eve == NULL)
			return;

	eve_copro_write_command(eve, 0xffffff32);
}

void eve_copro_swap(eve_t* eve)
{
#if EVE_COPRO_ENABLE_DUMP
	uint16_t dl_list_size = eve_spi_read_16(eve, EVE_REG_CMD_DL);
#endif
	if(eve == NULL)
			return;

#if EVE_COPRO_ENABLE_DUMP
	memset(eve_console_dl_list, 0, sizeof(eve_console_dl_list));
	eve_spi_read_data(eve, EVE_RAM_DL, (uint8_t*)eve_console_dl_list, dl_list_size);
#endif

	eve_copro_write_command(eve, 0xffffff01);
	ft_component_update_write_pointer(eve);

#if EVE_COPRO_ENABLE_DUMP
	if(eve_console_enable_live)
		comm_printf(COMM_DEBUG, "Swap\n");
#endif

#if EVE_COPRO_DEBUG_COMMAND_COUNT
	dbg_printf(DBG_STRING, "CMD Count: %d\n", eve->eve_copro_cmd_cnt);
#endif
}

void eve_copro_set_bitmap(eve_t* eve, uint32_t address, uint32_t format, uint16_t width, uint16_t height)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 16);
	uint32_t options[3] = {
		address,
		(format & 0xFFFF) | ((uint32_t)(width & 0xFFFF) << 16),
		height
	};

	eve_copro_internal_write_command_data(eve, 0xffffff43, options, 3, NULL, 0);
}

void eve_copro_set_scale(eve_t* eve, float scale_x, float scale_y)
{
	if(eve == NULL)
		return;

	uint32_t commands[2] = 
	{
		EVE_BITMAP_TRANSFORM_A_8_8((uint32_t)roundf(256.0f / scale_x)),
		EVE_BITMAP_TRANSFORM_E_8_8((uint32_t)roundf(256.0f / scale_y))
	};
	eve_copro_write_commands(eve, commands, 2);

	// DBG_INFO("Scale x %d.%d x %d.%d\n", (commands[0] & 0xFF00) >> 8, (commands[0] & 0xFF) * 100 / 256, (commands[1] & 0xFF00) >> 8, (commands[1] & 0xFF) * 100 / 256);

	// Below is used when loadidentity and setmatrix are also used, above is used for current bitmap

	// eve_copro_check_command_buffer(eve, 20);
	// // Well, 0xFFFFFF28 is the command for set scale, but we need loadidentity and setmatrix too, so we use the options different here
	// uint32_t options[4] = {
	// 	0xFFFFFF28,
	// 	(uint32_t)round(65536.0 * scale_x),
	// 	(uint32_t)round(65536.0 * scale_y),
	// 	0xFFFFFF2A
	// };

	// eve_copro_internal_write_command_data(eve, 0xffffff26, options, 4, NULL, 0);
}

// TODO: eve_copro_loadimage_mmc -> Filename instead of buffer and loads the data from mmc 

void eve_copro_loadimage(eve_t* eve, uint32_t ptr, EVE_OPT_IMAGE opt_image, const uint8_t* data, uint32_t length)
{
	if(eve == NULL)
		return;


	uint32_t options[2] = {
		ptr,
		opt_image
	};

	eve_copro_internal_write_command_data(eve, 0xffffff24, options, 2, data, length);
}

void eve_copro_flash_erase(eve_t* eve)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 4);
	eve_copro_internal_write_command(eve, 0xffffff44);
	
	eve_copro_wait_for_execution(eve);
}

void eve_copro_flash_write(eve_t* eve, uint32_t ptr, const uint8_t* data, uint32_t length)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 12);
	uint32_t options[2] = {
		ptr,
		length
	};

	eve_copro_internal_write_command_data(eve, 0xffffff45, options, 2, data, length);
	
	eve_copro_wait_for_execution(eve);
}

void eve_copro_flash_write_from_ram(eve_t* eve, uint32_t ptr_ram, uint32_t ptr_flash, uint32_t length)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 16);
	uint32_t options[3] = {
		ptr_flash,
		ptr_ram,
		length
	};

	eve_copro_internal_write_command_data(eve, 0xffffff70, options, 3, NULL, 0);
	
	eve_copro_wait_for_execution(eve);
}

void eve_copro_flash_read_to_ram(eve_t* eve, uint32_t ptr_ram, uint32_t ptr_flash, uint32_t length)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 16);
	uint32_t options[3] = {
		ptr_ram,
		ptr_flash,
		length
	};

	eve_copro_internal_write_command_data(eve, 0xffffff46, options, 3, NULL, 0);

	eve_copro_wait_for_execution(eve);
}

void eve_copro_appendf(eve_t* eve, uint32_t ptr_flash, uint32_t length)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 12);
	uint32_t options[2] = {
		ptr_flash,
		length
	};

	eve_copro_internal_write_command_data(eve, 0xffffff59, options, 2, NULL, 0);
	
	eve_copro_wait_for_execution(eve);
}

void eve_copro_flash_update_from_ram(eve_t* eve, uint32_t ptr_ram, uint32_t ptr_flash, uint32_t length)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 16);
	uint32_t options[3] = {
		ptr_flash,
		ptr_ram,
		length
	};

	eve_copro_internal_write_command_data(eve, 0xffffff47, options, 3, NULL, 0);
	
	eve_copro_wait_for_execution(eve);
}

void eve_copro_flash_detach(eve_t* eve)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 4);		
	eve_copro_internal_write_command(eve, 0xffffff48);
	// Wait until command was processed
	eve_copro_wait_for_execution(eve);
}

void eve_copro_flash_attach(eve_t* eve)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 4);		
	eve_copro_internal_write_command(eve, 0xffffff49);
	// Wait until command was processed
	eve_copro_wait_for_execution(eve);
}

uint16_t eve_copro_flash_fast(eve_t* eve)
{
	if(eve == NULL)
		return -1;
	
	DBG_INFO("Write flash fast\n");
	eve_copro_wait_for_execution(eve);
	// Options is always 0, because the space in the command buffer will be used by eve to write the result to.
	uint32_t options = 0;
	eve_copro_internal_write_command_data(eve, 0xffffff4A, &options, 1, NULL, 0);
	DBG_INFO("Wait for processing...\n");
	// Wait until command was processed
	eve_copro_check_command_buffer(eve, 4092);
	
	DBG_INFO("Read result\n");
	// Read the current write position
	uint16_t pos = eve_spi_read_16(eve, EVE_REG_CMD_WRITE);
	// Get the position where the 0 of the options was written to.
	pos = (pos - 4) & 0xFFC;
	// Since we aligned the buffer by 4, we do not need to make it a multiple of 4.
	uint16_t result = eve_spi_read_16(eve, EVE_RAM_CMD + pos);

	const char* err = NULL;
	switch(result)
	{
		case 0:
			// Thats ok
			goto exit;

		case 0xE001:
			err = "Flash is not supported";
			break;

		case 0xE002:
			err = "No Header detected in sector 0";
			break;

		case 0xE003:
			err = "Sector 0 data failed integrity check";
			break;

		case 0xE004:
			err = "Device / blob mismatch";
			break;

		case 0xE005:
			err = "Failed full-speed test";
			break;

		default:
			err = "Unknown error";
			break;
	}
	
	DBG_ERROR("Flashfast error: %s\n", err);

exit:
	// Return 0 is OK, other values are errors.
	return result;
}

void eve_copro_flashsource(eve_t* eve, uint32_t ptr_flash)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 8);		
	eve_copro_internal_write_command_data(eve, 0xffffff4E, &ptr_flash, 1, NULL, 0);
	// Wait until command was processed
	eve_copro_wait_for_execution(eve);
}

void eve_copro_playvideo(eve_t* eve, EVE_OPT_PLAYVIDEO_T options, const uint8_t* data, size_t length)
{
	if(eve == NULL)
		return;

	if(options & EVE_OPT_PLAYVIDEO_SOUND)
	{
		MCU_IO_SET_HANDLER(eve->hw.io_sound_enable, 1);
		// TODO: Start task to disable sound when video finishes.
	}

	// Ensure 32-Bit variable for options.
	uint32_t options_32 = options;

	eve_copro_check_command_buffer(eve, 8);		
	eve_copro_internal_write_command_data(eve, 0xffffff3A, &options_32, 1, data, length);
	// Wait until command was processed
	eve_copro_wait_for_execution(eve);
	// Nop is only written into command list when video playback is finished!
	eve_copro_internal_write_command(eve, EVE_NOP());
}

uint32_t eve_copro_getptr(eve_t* eve)
{
	// dummy is always 0, because the space in the command buffer will be used by eve to write the result to.
	uint32_t dummy = 0;
	eve_copro_wait_for_execution(eve);
	eve_copro_internal_write_command_data(eve, 0xffffff23, &dummy, 1, NULL, 0);
	// Wait until command was processed
	eve_copro_wait_for_execution(eve);
	
	// Read the current write position
	uint16_t pos = eve_spi_read_16(eve, EVE_REG_CMD_WRITE);
	// Get the position where the 0 of the options was written to.
	pos = (pos - 4) & 0xFFC;
	// Since we aligned the buffer by 4, we do not need to make it a multiple of 4.
	uint32_t result = eve_spi_read_32(eve, EVE_RAM_CMD + pos);

	DBG_INFO("getptr: %08x\n", result);

	return result;
}

void eve_copro_getprops(eve_t* eve, uint32_t* ptr, eve_ui_size_t* size)
{
	// dummy is always 0, because the space in the command buffer will be used by eve to write the result to.
	uint32_t dummy[3] = {0};
	eve_copro_wait_for_execution(eve);
	eve_copro_internal_write_command_data(eve, 0xffffff25, dummy, 3, NULL, 0);
	// Wait until command was processed
	eve_copro_wait_for_execution(eve);
	
	// Read the current write position
	uint16_t pos = eve_spi_read_16(eve, EVE_REG_CMD_WRITE);
	// Get the position where the 0 of the options was written to.
	pos = (pos - 12) & 0xFFC;
	if(ptr)
	{
		*ptr = eve_spi_read_32(eve, EVE_RAM_CMD + pos);
	}
	if(size)
	{
		size->width = eve_spi_read_32(eve, EVE_RAM_CMD + ((pos + 4) & 0xFFC));
		size->width = eve_spi_read_32(eve, EVE_RAM_CMD + ((pos + 8) & 0xFFC));
	}
}

void eve_copro_set_apilevel(eve_t* eve, uint32_t level)
{
	eve->api_level = level;
	eve_copro_check_command_buffer(eve, 8);
	eve_copro_internal_write_command_data(eve, 0xFFFFFF63, &level, 1, NULL, 0);
	DBG_INFO("Set API Level %d\n", level);
}

void eve_copro_set_font(eve_t* eve, uint8_t font, uint32_t address)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 12);
	eve_copro_internal_write_command(eve, 0xffffff2b);
	eve_copro_internal_write_command(eve, font);
	eve_copro_internal_write_command(eve, address);
}

void eve_copro_set_font2(eve_t* eve, uint8_t font, uint32_t address, uint8_t firstchar)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 16);
	eve_copro_internal_write_command(eve, 0xffffff3b);
	eve_copro_internal_write_command(eve, font);
	eve_copro_internal_write_command(eve, address);
	eve_copro_internal_write_command(eve, firstchar);
}

void eve_copro_set_color(eve_t* eve, color_t c)
{
	if(eve == NULL)
		return;

	if(eve->col.a != c.a)
	{
		eve_copro_check_command_buffer(eve, 4);
		eve_copro_internal_write_command(eve, EVE_COLOR_A(c.a));
	}
	if(eve->col.r != c.r || eve->col.g != c.g || eve->col.b != c.b)
	{
		eve_copro_check_command_buffer(eve, 4);
		eve_copro_internal_write_command(eve, EVE_COLOR_RGB(c.r, c.g, c.b));
	}
	eve->col = c;
}

//void eve_copro_set_color_alpha(eve_t* eve, uint8_t alpha)
//{
//	if(eve == NULL)
//			return;
//
//	eve_copro_check_command_buffer(eve, 4);
//	eve_copro_internal_write_command(eve, EVE_COLOR_A(alpha));
//}

void eve_copro_set_foreground(eve_t* eve, color_t color)
{
	if(eve == NULL)
			return;

	eve_copro_check_command_buffer(eve, 12);
	eve_copro_internal_write_command(eve, EVE_COLOR_A(color.a));
	eve_copro_internal_write_command(eve, 0xffffff0a);
	eve_copro_internal_write_command(eve, ((uint32_t)color.r << 16) | ((uint32_t)color.g << 8) | ((uint32_t)color.b << 0) );
}

void eve_copro_set_background(eve_t* eve, color_t c)
{
	if(eve == NULL)
			return;

	eve_copro_check_command_buffer(eve, 12);
	eve_copro_internal_write_command(eve, EVE_COLOR_A(c.a));
	eve_copro_internal_write_command(eve, 0xffffff09);
	eve_copro_internal_write_command(eve, ((uint32_t)c.r << 16) | ((uint32_t)c.g << 8) | ((uint32_t)c.b << 0) );
}

void eve_copro_set_gradientcolor(eve_t* eve, color_t c)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 12);
	eve_copro_internal_write_command(eve, EVE_COLOR_A(c.a));
	eve_copro_internal_write_command(eve, 0xffffff34);
	eve_copro_internal_write_command(eve, ((uint32_t)c.r << 16) | ((uint32_t)c.g << 8) | ((uint32_t)c.b << 0) );
}

void eve_copro_gradient(	eve_t* eve,
								int32_t x1, int32_t y1, color_t c1,
								int32_t x2, int32_t y2, color_t c2)
{
	if(eve == NULL)
		return;

	eve_copro_check_command_buffer(eve, 20);
	eve_copro_internal_write_command(eve, 0xffffff0b);
	eve_copro_internal_write_command(eve, x1 | ((uint32_t)y1 << 16));
	eve_copro_internal_write_command(eve, ((uint32_t)c1.r << 16) | ((uint32_t)c1.g << 8) | ((uint32_t)c1.b << 0) );
	eve_copro_internal_write_command(eve, x2 | ((uint32_t)y2 << 16));
	eve_copro_internal_write_command(eve, ((uint32_t)c2.r << 16) | ((uint32_t)c2.g << 8) | ((uint32_t)c2.b << 0) );
}

void eve_copro_text(eve_t* eve, int32_t x, int32_t y, uint16_t font, EVE_OPT_TEXT options, const char *str)
{
#if _TEMPORARY_TEXT_BUFFER_SIZE > 0
	const char* ptr = str;
	char* dst = _tmp_text_buffer;
	uint16_t num_linefeed = 0;
#endif
	uint16_t len = (str == NULL) ? 0 : strlen(str) + 1;

	if(eve == NULL || font > 31 || str == NULL || len < 2)
		return;

	_load_font(eve, font);

#if _TEMPORARY_TEXT_BUFFER_SIZE > 0

	while(*ptr != 0)
	{
		if(*ptr == '\n')
			num_linefeed++;
		ptr++;
	}
	ptr = str;

	if(num_linefeed > 0)
	{
		if((options & EVE_OPT_TEXT_CENTERY) == EVE_OPT_TEXT_CENTERY)
		{
			// y coordinate is the center of the text -> Since it is now multiple lines, the center must be adjusted, otherwise it is
			// only the center of the first line
			uint16_t h = (num_linefeed) * font_get_height(eve, font);
			y -= h/2;
		}
		while(*ptr != 0)
		{
			if(*ptr == '\n')
			{
				*dst = 0;
				len = strlen(_tmp_text_buffer);
				if(len > 0)
				{
					eve_copro_check_command_buffer(eve, 12 + len);
					eve_copro_internal_write_command(eve, 0xffffff0c);
					eve_copro_internal_write_command(eve, x | ((uint32_t)y << 16));
					eve_copro_internal_write_command(eve, font | ((uint32_t)options << 16));
					eve_copro_write_string(eve, _tmp_text_buffer, len);
				}
				dst = _tmp_text_buffer;
				// TODO: Line Skip
				y += font_get_height(eve, font);
			}
			else
			{
				if(dst < (&_tmp_text_buffer[sizeof(_tmp_text_buffer) - 2]))
					*dst++ = *ptr;
			}
			ptr++;
		}

		if(dst < (_tmp_text_buffer + sizeof(_tmp_text_buffer)))
			*dst = 0;
		else
			_tmp_text_buffer[sizeof(_tmp_text_buffer) - 1] = 0;

		len = strlen(_tmp_text_buffer);
		if(len > 0)
		{
			eve_copro_check_command_buffer(eve, 12 + len);
			eve_copro_internal_write_command(eve, 0xffffff0c);
			eve_copro_internal_write_command(eve, x | ((uint32_t)y << 16));
			eve_copro_internal_write_command(eve, font | ((uint32_t)options << 16));
			eve_copro_write_string(eve, _tmp_text_buffer, len);
		}
	}
	else
	{
#endif
		// uint32_t options[2] = {
		// 		x | ((uint32_t)y << 16),
		// 		font | ((uint32_t)options << 16)
		// };

		// _process_string(str, len);
		// eve_copro_internal_write_command_data(eve, 0xffffff0c, options, 2, str ? (uint8_t*)str : (uint8_t*){0}, str ? strlen(str) + 1 : 1);

		eve_copro_check_command_buffer(eve, 12 + len);
		eve_copro_internal_write_command(eve, 0xffffff0c);
		eve_copro_internal_write_command(eve, x | ((uint32_t)y << 16));
		eve_copro_internal_write_command(eve, font | ((uint32_t)options << 16));
		eve_copro_write_string(eve, str, len);
		
#if _TEMPORARY_TEXT_BUFFER_SIZE > 0
	}
#endif
}

void eve_copro_number(eve_t* eve, int32_t x, int32_t y, uint16_t font, EVE_OPT_TEXT options, uint32_t num)
{

	if(eve == NULL || font > 31)
		return;

	_load_font(eve, font);

	eve_copro_check_command_buffer(eve, 16);
	eve_copro_internal_write_command(eve, 0xffffff2e);
	eve_copro_internal_write_command(eve, x | ((uint32_t)y << 16));
	eve_copro_internal_write_command(eve, font | ((uint32_t)options << 16));
	eve_copro_internal_write_command(eve, num);
}

//void eve_copro_key(eve_t* eve, key_t* obj, uint8_t x, uint8_t y)
//{
//	uint16_t len;
//
//	if(eve == NULL || obj == NULL)
//		return;
//
//	x += obj->component.origin.x;
//	y += obj->component.origin.y;
//
//	if(obj->text == NULL)
//		len = 1;
//	else
//		len = strlen(obj->text) + 1;
//	eve_copro_check_command_buffer(eve, 16 + len);
//	eve_copro_internal_write_command(eve, 0xffffff0e);
//	eve_copro_internal_write_command(eve, x| ((uint32_t)y << 16));
//	eve_copro_internal_write_command(eve, obj->component.size.width | ((uint32_t)obj->component.size.height << 16));
//	eve_copro_internal_write_command(eve, obj->font | ((uint32_t)(obj->option+eve->selected_key) << 16));
//	eve_copro_write_string(eve, obj->text, len);
//}

void eve_copro_button(eve_t* eve, int32_t x, int32_t y, uint16_t w, uint16_t h, uint16_t option, uint16_t font, char* text)
{
	uint16_t len;

	if(eve == NULL)
		return;

	if(text == NULL)
		len = 1;
	else
		len = strlen(text) + 1;

	_load_font(eve, font);

	eve_copro_check_command_buffer(eve, 16 + len);
	eve_copro_internal_write_command(eve, 0xffffff0d);
	eve_copro_internal_write_command(eve, x| ((uint32_t)y << 16));
	eve_copro_internal_write_command(eve, w | ((uint32_t)h << 16));
	eve_copro_internal_write_command(eve, font | ((uint32_t)option << 16));
	eve_copro_write_string(eve, text, len);
}

void eve_copro_slider(eve_t* eve, int32_t x, int32_t y, uint16_t w, uint16_t h, EVE_OPT_SLIDER option, uint16_t value, uint16_t range)
{
	uint16_t len;

	if(eve == NULL)
		return;

	uint32_t options[4] = 
	{
		x | ((uint32_t)y << 16),
		w | ((uint32_t)h << 16),
		option | ((uint32_t)value << 16),
		range
	};

	eve_copro_internal_write_command_data(eve, 0xffffff10, options, 4, NULL, 0);
}

void eve_copro_add_tag(eve_t* eve, component_t* obj)
{
	if(eve == NULL || obj == NULL || !eve->has_touch)
		return;

	if(eve->eve_copro_current_button_cnt >= EVE_COPRO_MAX_TAGS)
		return;

	eve->eve_copro_current_buttons[eve->eve_copro_current_button_cnt] = obj;
	eve->eve_copro_current_button_cnt++;

	eve_copro_write_command(eve, EVE_TAG(eve->eve_copro_current_button_cnt+128));
}

void eve_copro_use_last_tag(eve_t* eve)
{
	if(eve == NULL || !eve->has_touch)
		return;

	if(eve->eve_copro_current_button_cnt >= EVE_COPRO_MAX_TAGS)
		return;

	eve_copro_write_command(eve, EVE_TAG(eve->eve_copro_current_button_cnt+128));
}

void eve_copro_clear_tag(eve_t* eve)
{
	if(eve == NULL || !eve->has_touch)
		return;

	eve_copro_write_command(eve, EVE_TAG(0));
}

void eve_copro_switch_button(eve_t* eve, switch_button_t* obj, int32_t x, int32_t y)
{
	uint16_t len;

	if(eve == NULL || obj == NULL)
		return;

	x += obj->component.origin.x;
	y += obj->component.origin.y;

	len = strlen(obj->text) + 1;

	eve_copro_check_command_buffer(eve, 16 + len);
	eve_copro_internal_write_command(eve, 0xffffff12);
	eve_copro_internal_write_command(eve, x| ((uint32_t)y << 16));
	eve_copro_internal_write_command(eve, obj->component.size.width | ((uint32_t)obj->font << 16));
	eve_copro_internal_write_command(eve, obj->option | ((uint32_t)obj->state << 16));
	eve_copro_write_string(eve, obj->text, len);
}

void eve_copro_touch_released(eve_t* eve)
{
	if(eve == NULL)
		return;

	if(eve->eve_copro_selected_button != NULL)
	{
		if(((component_t*)(eve->eve_copro_selected_button))->pressed_callback != NULL)
			((component_t*)(eve->eve_copro_selected_button))->pressed_callback(eve->eve_copro_selected_button, false);
//		button_set_pressed(eve_copro_selected_button, false);
		eve->eve_copro_selected_button = NULL;
	}

	if (eve->selected_key != 0)
	{
		if (eve->key_callback != NULL)
			eve->key_callback(eve->key_callback_object, eve->selected_key);
		eve->selected_key  = 0;
	}
}

void eve_copro_set_screen_changed(eve_t* eve)
{
	if(eve == NULL)
		return;

//	uint32_t xy = eve_spi_read_32(EVE_REG_TOUCH_SCREEN_XY);
	// If the screen is changed while the display is touched -> ignore the touch because it belongs to the old screen button!
//	if(xy != 0x80008000)
//		eve_copro_ignore_first_touch = true;
}

#if EVE_COPRO_ENABLE_SNAPSHOT && MODULE_ENABLE_MMC && MODULE_ENABLE_IMAGE
static FIL _file;

//typedef struct
//{
//	uint16_t type;				/// Must be 19778 / "BM" / 0x42 0x4D
//	uint32_t size;				/// Size of the BMP File -> Not always correct
//	uint32_t reserved;			/// Reserved
//	uint32_t offset;			/// Offset where the image starts
//
//	uint32_t header_size;		/// Size of the header (should be 40)
//	int32_t width;				/// Width in pixel
//	int32_t height;				/// Height in pixel
//	uint16_t planes;			/// Always 1
//	uint16_t bit_count;			/// Colordepth in pixel
//	uint32_t compression;		/// Compression (0 for uncompressed)
//	uint32_t size_image;		/// Size of the image or 0.
//	int32_t pels_per_meter_x;	/// Horizontal resolution of target display. Can be 0.
//	int32_t pels_per_meter_y;	/// Vertical resolution of target display. Can be 0.
//	uint32_t clr_used;			/// Number of entries in color table or 0 if no color table is used.
//	uint32_t clr_important;		/// Number of used colors if bit_count is 1, 4 or 8. Otherwise 0.
//
//} bitmap_header_t;

static bitmap_header_t _bitmap;

static uint8_t _buffer_snapshot[EVE_COPRO_SNAPSHOT_BUFFER_SIZE];

void eve_copro_snapshot(eve_t* eve, const char* filename)
{
	mmc_delete_file(filename);
	if(FR_OK == mmc_open_file(filename, &_file, OPEN_NORMAL))
	{
		uint8_t pclk = eve_spi_read_8(eve, EVE_REG_PCLK);
		uint16_t w = eve->eve_display_width;
		uint16_t h = eve->eve_display_height;
		uint32_t fsize = w * h * 2;
		uint32_t fcount = 0;

		eve_spi_write_8(eve, EVE_REG_PCLK, 0);

		eve_copro_check_command_buffer(eve, 8);
		eve_copro_internal_write_command(eve, 0xffffff1F);
		eve_copro_internal_write_command(eve, 0);

		while(eve_spi_read_16_twice(eve, EVE_REG_CMD_READ) != eve_spi_read_16_twice(eve, EVE_REG_CMD_WRITE));

		_bitmap.type = 19778;
		_bitmap.size = 40 + fsize;
		_bitmap.offset = 40;
		_bitmap.header_size = 40;
		_bitmap.width = w;
		_bitmap.height = h;
		_bitmap.planes = 1;
		_bitmap.bit_count = 4;
		_bitmap.compression = 0;
		_bitmap.size_image = 0;
		_bitmap.pels_per_meter_x = 0;
		_bitmap.pels_per_meter_y = 0;
		_bitmap.clr_used = 0;
		_bitmap.clr_important = 0;
		mmc_puts(&_file, &_bitmap, 40);

		do
		{
			uint16_t len = sizeof(_buffer_snapshot);
			if( (fsize - fcount) < len)
				len = fsize - fcount;
			eve_spi_read_data(eve, EVE_RAM_G + fcount, _buffer_snapshot, len);
			mmc_puts(&_file, _buffer_snapshot, len);
			fcount += len;

		}while(fcount < fsize);

		mmc_close_file(&_file);

		eve_spi_write_8(eve, EVE_REG_PCLK, pclk);	// Set PCLK Divider

		eve_memory_clear();
		screen_repaint((screen_device_t*)eve);
	}
}
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
FUNCTION_RETURN eve_copro_check_command_buffer(eve_t* eve, uint32_t length)
{
	uint16_t freespace = 0;

	if(eve == NULL)
		return FUNCTION_RETURN_PARAM_ERROR;

	length = (length + 3) & ~3;

#if !EVE_USE_FT81X
	ft_component_update_write_pointer(eve);
#endif

	uint32_t timestamp = system_get_tick_count();
	do
	{
#if EVE_USE_FT81X

		freespace = eve_spi_read_16_twice(eve, EVE_REG_CMDB_SPACE);

#else

		uint16_t fullness = 0;
		fullness = (eve->eve_copro_cli - eve_spi_read_16_twice(eve, EVE_REG_CMD_READ)) & 4095;
		freespace = (4096 - 4) - fullness;
#endif

		if(freespace & 0x03)
		{
			DBG_ERROR("CoProcessor Error by unaligned freespace: %u\n", freespace);
			eve_copro_reset(eve);
			return FUNCTION_RETURN_DEVICE_RESET;
		}

		if( (system_get_tick_count() - timestamp) >= 1000)
		{
			timestamp = system_get_tick_count();
			DBG_INFO("Freespace: %u\n", freespace);
			return FUNCTION_RETURN_TIMEOUT;
		}
	}
	while(freespace < length);

	return FUNCTION_RETURN_OK;
}

static void eve_copro_internal_write_command(eve_t* eve, uint32_t val)
{
	if(eve == NULL)
		return;

#if EVE_USE_FT81X
	eve_spi_write_32(eve, EVE_REG_CMDB_WRITE, val);
#else
	eve_spi_write_32(eve, EVE_RAM_CMD + eve->eve_copro_cli, val);
#endif
	eve_copro_increment_write_pointer(eve, 4);
}

static void eve_copro_internal_write_command_data(eve_t* eve, uint32_t val, const uint32_t* options, uint32_t num_options, const uint8_t* data, uint32_t sizeof_data)
{
	if(eve == NULL)
		return;
	// Ensure command + options will fit
	eve_copro_check_command_buffer(eve, 4 + (num_options * 4));
	
#if EVE_USE_FT81X

	const uint8_t* m_data[2] = {
		(uint8_t*)&val,
		(uint8_t*)options,
		// (uint32_t*)data
	};

	uint32_t l_data[2] = {
		4,
		num_options * 4,
		// sizeof_data
	};

	eve_spi_write_multi_data(eve, EVE_REG_CMDB_WRITE, m_data, l_data, 2);
	eve_copro_increment_write_pointer(eve, 4 + (num_options * 4));

	if(data && sizeof_data > 0)
	{
		// DBG_INFO("Write %d data\n", sizeof_data);
		uint32_t offset = 0;
		while(offset < sizeof_data)
		{
			uint32_t l = sizeof_data - offset;
			uint32_t padding = 0;
			if(l > 3600)
				l = 3600;
			else
			{
				padding = (4 - (l & 3)) & 3;
				if(padding > 0)
					l &= ~3;
			}

			// DBG_INFO("eve_copro_check_command_buffer\n");
			eve_copro_check_command_buffer(eve, l);

			// DBG_INFO("Write offset %u -> %u\n"eb, offset, l);
			
			// DBG_INFO("Write to %06x = %#A\n", EVE_REG_CMDB_WRITE, l, &data[offset]);

			eve_spi_write_data(eve, EVE_REG_CMDB_WRITE, &data[offset], l, false);
			eve_copro_increment_write_pointer(eve, l);
			offset += l;

			// for(int i = 0; i < l; i+=4)
			// {
			// 	eve_copro_internal_write_command(eve, *(uint32_t*)&data[offset]);
			// 	eve_copro_increment_write_pointer(eve, 4);
			// 	offset += 4;
			// }
			
			if(padding > 0)
			{
				// DBG_INFO("Pad %d = %#A\n", padding, sizeof_data - offset, &data[offset]);
				eve_copro_check_command_buffer(eve, 4);
				val = 0;
				memcpy(&val, &data[offset], sizeof_data - offset);
				eve_spi_write_data(eve, EVE_REG_CMDB_WRITE, (uint8_t*)&val, 4, false);
				// eve_copro_increment_write_pointer(eve, l);
				// eve_copro_internal_write_command(eve, val);
				eve_copro_increment_write_pointer(eve, 4);
				offset += 4;
			}
		}
	}

	ft_component_update_write_pointer(eve);
		
#else

// 	eve_spi_write_multi_data(eve, EVE_RAM_CMD + eve->eve_copro_cli, m_data, l_data, 2);
// 	eve_copro_increment_write_pointer(eve, 4 + (num_options * 4) + sizeof_data);
	
	// Write command and options into the command buffer
	eve_copro_internal_write_command(eve, val);
	DBG_INFO("Write command %08x\n", val);
	eve_copro_increment_write_pointer(eve, 4);
	if(options && num_options > 0)
	{
		for( ; num_options > 0; num_options--)
		{
			DBG_INFO("Write option %08x\n", *options);
			eve_copro_internal_write_command(eve, *options);
			eve_copro_increment_write_pointer(eve, 4);
			options++;
		}
	}
	
	if(data && sizeof_data > 0)
	{
		uint32_t offset = 0;
		while(offset < sizeof_data)
		{
			uint32_t l = sizeof_data - offset;
			uint32_t padding = 0;
			if(l > 4000)
				l = 4000;
			else
			{
				padding = 4 - (l % 4);
				if(padding > 0)
					l-= padding;
			}

			eve_copro_check_command_buffer(eve, l);

			DBG_INFO("Write offset %u -> %u\n", offset, l);

			for(int i = 0; i < l; i+=4)
			{
				eve_copro_internal_write_command(eve, *(uint32_t*)&data[offset]);
				eve_copro_increment_write_pointer(eve, 4);
				offset += 4;
			}
			
			if(padding > 0)
			{
				DBG_INFO("Pad %d bytes\n", padding);
				val = 0;
				memcpy(&val, &data[offset], sizeof_data - offset);
				eve_copro_internal_write_command(eve, val);
				eve_copro_increment_write_pointer(eve, 4);
				offset += 4;
			}
		}
	}

	ft_component_update_write_pointer(eve);

#endif

	// eve_spi_write_32(eve, EVE_RAM_CMD + eve->eve_copro_cli, val);
	// eve_copro_increment_write_pointer(eve, 4);
}

static void _process_string(char* str, uint32_t len)
{
	if(str != NULL)
	{
		uint16_t i;
		for(i = 0; i < len; i++)
		{
			switch(str[i])
			{
			case 0xC3:
				switch(str[i+1])
				{
				case 0x9F:	str[i] = 0x19;	break;
				case 0x84:	str[i] = 0x1A;	break;
				case 0x96:	str[i] = 0x1B;	break;
				case 0x9C:	str[i] = 0x1C;	break;
				case 0xA4:	str[i] = 0x1D;	break;
				case 0xB6:	str[i] = 0x1E;	break;
				case 0xBC:	str[i] = 0x1F;	break;
				default: 	str[i] = 1;	break;
				}
				str[i+1] = 1;
				i++;
				break;
			case 0xC2:
				switch(str[i+1])
				{
				case 0xB0:	str[i] = 0x18;	break;
				default: 	str[i] = 1;	break;
				}
				str[i+1] = 1;
				i++;
				break;
			// Below would be for extended ASCII -> Compiler makes UTF-16 above
			case 0xB0:	str[i] = 0x18;	break;
			case 0xC4:	str[i] = 0x1A;	break;
			case 0xD6:	str[i] = 0x1B;	break;
			case 0xDC:	str[i] = 0x1C;	break;
			case 0xDF:	str[i] = 0x19;	break;
			case 0xE4:	str[i] = 0x1D;	break;
			case 0xF6:	str[i] = 0x1E;	break;
			case 0xFC:	str[i] = 0x1F;	break;
			}
		}
	}
}

static void eve_copro_write_string(eve_t* eve, const char *str, uint16_t len)
{
	if(eve == NULL)
		return;

	if(str != NULL && str[0])
	{		
		_process_string(str, len);

#if EVE_USE_FT81X
		len = strlen(str) + 1; // +1 because of terminating 0.
		if(len & 3)
		{
			// Length without the part that must be pad
			uint32_t len_a = len & ~3;
			uint32_t len_b = len & 3;
			// Padding bytes
			uint8_t padding_bytes[4] = {0};			
			// DBG_INFO("Write %08x %u %u %08x -> %s\n", str, len_a, len_b, padding_bytes, str);
			memcpy(padding_bytes, &str[len_a], len_b);
			// DBG_INFO("Write %#A %4A\n", len_a, str, padding_bytes);
			if(len_a == 0) // everything in padding_bytes
			{
				eve_spi_write_data(eve, EVE_REG_CMDB_WRITE, padding_bytes, 4, false);
			}
			else
			{
				eve_spi_write_multi_data(eve, EVE_REG_CMDB_WRITE, (const uint8_t*[2]){(uint8_t*)str, padding_bytes}, (uint32_t[2]){len_a, 4}, 2);
			}
		}
		else // No padding needed!
		{
			eve_spi_write_data(eve, EVE_REG_CMDB_WRITE, (uint8_t*)str, len, false);
		}
		
		// len = eve_spi_write_str_data(eve, EVE_REG_CMDB_WRITE, str, true);
		
		// len = eve_spi_write_str_data(eve, EVE_REG_CMDB_WRITE, str);
		// if(_print_verbose)
		// 	DBG_VERBOSE("Write %#A\n", len, str);
#else
		len = eve_spi_write_str_data(eve, EVE_RAM_CMD + eve->eve_copro_cli, str, false);
#endif
		eve_copro_increment_write_pointer(eve, len);
	}
	else
	{
#if EVE_USE_FT81X
		eve_spi_write_32(eve, EVE_REG_CMDB_WRITE, 0);
		eve_copro_increment_write_pointer(eve, 4);
#else
		eve_spi_write_8(eve, EVE_RAM_CMD + eve->eve_copro_cli, 0);
		eve_copro_increment_write_pointer(eve, 1);
#endif
	}
}

static void _load_font(eve_t* eve, uint8_t font)
{
	if(eve->memory.font[font].is_ram && !eve->memory.font[font].handler_is_loaded)
	{
#if EVE_USE_FT81X
		const uint8_t* data = eve->memory.font[font].data;

		eve_copro_check_command_buffer(eve, 40);
		eve_copro_internal_write_command(eve, EVE_BITMAP_HANDLE(font)); // associate font to font handle
		eve_copro_internal_write_command(eve, EVE_BITMAP_SOURCE(eve->memory.font[font].address + 128));
		// Set Layout format, stride and height -> Take values from metric
		eve_copro_internal_write_command(eve, EVE_BITMAP_LAYOUT_H(0,0));
		eve_copro_internal_write_command(eve, EVE_BITMAP_LAYOUT(
				*(uint32_t*)&data[128], // Format
				*(uint32_t*)&data[132], // Stride
				*(uint32_t*)&data[140]) // Height
				);
		// Set Layout width and height -> Take values from metric
		eve_copro_internal_write_command(eve, EVE_BITMAP_SIZE_H(0,0));
		eve_copro_internal_write_command(eve, EVE_BITMAP_SIZE(EVE_FILTER_NEAREST, EVE_WRAP_BORDER, EVE_WRAP_BORDER,
				*(uint32_t*)&data[136], // Width
				*(uint32_t*)&data[140]) // Height
				);

		// Set Font 2
		eve_copro_internal_write_command(eve, 0xffffff3b);
		eve_copro_internal_write_command(eve, font);
		eve_copro_internal_write_command(eve, eve->memory.font[font].address);
		eve_copro_internal_write_command(eve, eve->memory.font[font].first_character);
#else
		// On EVE only handle 0-15 can be used for custom fonts...
		// Also only set font is available, not set font 2
//		eve_copro_check_command_buffer(eve, 32);
//		eve_copro_internal_write_command(eve, EVE_BITMAP_HANDLE(font)); // associate font to font handle
//		eve_copro_internal_write_command(eve, EVE_BITMAP_SOURCE(eve->memory.font[font].address + 128));
//		// Set Layout format, stride and height -> Take values from metric
//		eve_copro_internal_write_command(eve, EVE_BITMAP_LAYOUT(
//				*(uint32_t*)&data[128], // Format
//				*(uint32_t*)&data[132], // Stride
//				*(uint32_t*)&data[140]) // Height
//				);
//		// Set Layout width and height -> Take values from metric
//		eve_copro_internal_write_command(eve, EVE_BITMAP_SIZE(EVE_FILTER_NEAREST, EVE_WRAP_BORDER, EVE_WRAP_BORDER,
//				*(uint32_t*)&data[136], // Width
//				*(uint32_t*)&data[140]) // Height
//				);
//
//		// Set Font 2
//		eve_copro_internal_write_command(eve, 0xffffff3b);
//		eve_copro_internal_write_command(eve, font);
//		eve_copro_internal_write_command(eve, eve->memory.font[font].address);
//		eve_copro_internal_write_command(eve, eve->memory.font[font].first_character);
#endif
		eve->memory.font[font].handler_is_loaded = true;
	}
}

static void eve_copro_increment_write_pointer(eve_t* eve, uint16_t count)
{
	if(eve == NULL)
		return;

	eve->eve_copro_cli_last_written = eve->eve_copro_cli;
	// Count from 0 - 4095 and makes a 4-byte alignment in case it is not 4-byte aligned
	eve->eve_copro_cli = (eve->eve_copro_cli + count + 3) & 0xFFC;
	eve->eve_copro_is_ready_flag = false;	// Write Pointer is changed
	eve->component_timestamp = system_get_tick_count();
#if EVE_COPRO_DEBUG_COMMAND_COUNT
	eve->eve_copro_cmd_cnt += count;
	eve->eve_copro_cmd_cnt = (eve->eve_copro_cmd_cnt + 3) & ~3;	// Makes a 4-byte alignment in case it is not 4-byte aligned
#endif
}

static void ft_component_update_write_pointer(eve_t* eve)
{
	if(eve == NULL || eve->eve_copro_cli_last_written == eve->eve_copro_cli)
		return;

	// eve_copro_check_command_buffer(eve, 0);
	// eve_spi_write_16(eve, EVE_REG_CMD_WRITE, eve->eve_copro_cli);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Module Functions -> Functions that are called from other source files inside this module
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void eve_copro_set_command_complete(eve_t* eve)
{
#if EVE_COPRO_DEBUG_DL_LIST
	static uint32_t last_dl_val = 0;
#endif
	if(eve == NULL)
		return;

#if EVE_COPRO_DEBUG_DL_LIST
	if(last_dl_val != eve_spi_read_32(eve, EVE_REG_CMD_DL))
	{
		last_dl_val = eve_spi_read_32(eve, EVE_REG_CMD_DL);
		dbg_printf(DBG_STRING, "EVE_REG_CMD_DL=%d\n", last_dl_val);
	}
#endif
	eve->eve_copro_is_ready_flag = true;
}

void eve_copro_tag_interrupt(eve_t* eve, uint8_t tag)
{
	if(eve == NULL)
		return;

	screen_repaint((screen_device_t*)eve);

	if( (system_get_tick_count() - eve->eve_copro_touch_timestamp) < EVE_COPRO_TOUCH_MIN_MS )
	{
		eve->eve_copro_touch_timestamp = system_get_tick_count();
		eve->eve_copro_release_touch = (tag == 0 || tag == 0xFF);
		eve->selected_key = 0;
		return;
	}

	if(tag == 0 || tag == 0xFF)			// 0xFF is a component without touch, 0 is released
	{
		eve_copro_touch_released(eve);	// Release a previously pressed component
		eve->eve_copro_touch_timestamp = system_get_tick_count();
		eve->selected_key = 0;
		return;
	}

	if (tag >= 128)
	{
		tag-= 129;
		if(eve->eve_copro_selected_button != eve->eve_copro_current_buttons[tag])
		{
			eve_copro_touch_released(eve);	// Release a previously pressed component

			eve->eve_copro_selected_button = eve->eve_copro_current_buttons[tag];

			if(eve->eve_copro_selected_button != NULL)
			{
				if(((component_t*)(eve->eve_copro_selected_button))->pressed_callback != NULL)
					((component_t*)(eve->eve_copro_selected_button))->pressed_callback(eve->eve_copro_selected_button, true);
			}
		}
		eve->selected_key = 0;
	}
	else // Tag is an ASCII Character from Keys
	{
		if (eve->selected_key != tag)
		{
			eve_copro_touch_released(eve);
			eve->selected_key = tag;
		}
	}

	eve->eve_copro_touch_timestamp = system_get_tick_count();
}

void eve_copro_touch_interrupt(eve_t* eve, int32_t x, int32_t y)
{
//	dbg_printf(DBG_STRING, "Touch %d/%d\n", x, y);
	bool is_touched;

#if EVE_USE_FT81X
	is_touched = x < 0x8000 && y < 0x8000;
#else
	is_touched = x < 0x0200 && y < 0x0200;
#endif

	if(eve != NULL && eve->eve_copro_touch_callback != NULL)
		eve->eve_copro_touch_callback(eve->eve_copro_touch_callback_object, is_touched, x, y);
}

#endif

