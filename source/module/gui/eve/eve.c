/***
 * @file eve.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "eve.h"
#include "eve_spi.h"
#include "eve_register.h"
#include "eve_memory.h"
#include "../eve_ui/screen.h"
#include <string.h>
#include "../eve_ui/font.h"
#include "module/comm/dbg.h"
#if MODULE_ENABLE_DISPLAY && DISPLAY_ENABLE_SLD
#include "module/display/sld/sld_edid.h"
#endif
#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
#include "module/lcd_touch/driver/st1633i/st1633i.h"
#endif
#include <math.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Pointer to the first initializes eve
static eve_t* _first_eve = NULL;

static const uint8_t _touch_fw_ili2130[] = { // Firmware für Ilitek ILI2130 Touch - V.09
	26,255,255,255,32,32,48,0,4,0,0,0,2,0,0,0,26,255,255,255,0,176,48,0,4,0,0,0,94,2,0,0,34,255,255,255,0,176,48,0,120,218,93,84,81,104,28,101,16,158,127,255,203,105,46,114,217,45,165,136,100,245,118,189,120,180,215,66,105,42,34,40,204,36,109,105,19,74,17,17,131,72,243,255,119,241,46,119,123,18,130,47,34,209,78,125,240,105,97,241,197,42,152,226,131,167,72,192,39,131,72,95,4,107,148,82,201,131,80,138,16,240,161,148,62,53,47,41,130,22,174,243,111,174,162,101,153,127,230,159,127,230,159,153,127,190,217,243,30,0,112,218,82,156,53,106,214,183,97,242,185,5,206,218,190,93,179,129,80,205,166,166,38,146,211,215,172,147,156,28,38,206,118,207,62,76,30,74,169,209,237,180,85,194,180,37,55,44,249,54,91,210,75,105,75,248,50,116,211,214,20,102,141,50,234,165,151,42,218,190,12,153,73,77,25,143,224,139,16,152,172,33,209,59,208,149,60,218,53,86,236,219,41,147,73,22,179,16,244,124,91,192,180,87,196,49,158,195,57,56,135,46,150,110,248,54,72,242,85,162,234,229,231,69,95,196,154,120,0,135,73,96,222,173,84,141,98,221,92,169,28,192,121,156,192,114,190,134,201,12,118,64,219,9,212,54,179,101,220,207,77,116,21,237,19,238,170,152,199,3,252,58,106,19,227,83,195,147,39,255,61,153,144,147,204,184,204,70,25,36,39,221,201,154,79,136,148,181,129,215,16,90,95,84,82,3,92,226,160,247,117,197,229,30,152,239,42,248,81,31,79,226,183,82,45,116,215,33,179,125,236,195,97,62,3,117,62,139,138,15,241,148,209,246,144,120,148,240,231,74,31,127,20,11,197,117,118,222,117,62,131,87,114,59,200,237,246,52,91,176,37,55,92,195,13,252,65,232,55,248,221,173,195,221,53,216,196,62,94,65,119,223,29,137,88,231,13,132,222,13,241,168,231,113,220,217,6,222,128,11,92,224,79,165,223,127,33,161,94,188,137,91,162,13,204,99,172,59,59,194,139,67,62,50,228,133,33,215,67,238,13,185,202,249,14,222,194,40,2,72,13,137,148,53,110,97,76,255,249,148,226,127,112,146,128,119,192,195,29,152,164,163,234,56,197,143,124,71,213,36,29,35,109,183,33,230,87,161,34,84,21,138,164,215,39,168,140,167,232,89,217,121,232,225,96,0,173,192,12,6,169,145,126,55,28,38,157,102,205,188,73,69,44,240,11,56,79,39,212,41,117,78,157,86,206,119,146,231,96,132,3,3,221,5,58,137,111,71,169,201,107,90,94,141,92,7,245,138,94,124,95,101,214,105,235,248,97,52,5,123,82,17,171,230,3,122,75,208,17,38,23,212,115,114,71,96,102,80,55,2,243,73,212,71,232,126,44,62,135,69,123,132,211,222,168,80,81,8,90,14,103,242,166,138,216,33,160,204,151,225,43,250,62,90,69,135,88,232,5,246,29,170,89,75,97,226,240,251,147,114,186,130,96,171,140,171,248,43,207,14,245,46,151,47,233,62,40,121,161,227,244,56,235,149,172,1,124,149,54,105,91,144,114,85,57,217,183,155,164,104,27,198,248,172,68,184,27,185,155,64,230,204,23,212,64,243,118,20,216,5,154,65,55,45,35,12,237,63,163,247,112,87,252,198,248,50,238,70,175,208,44,157,6,247,98,122,241,160,196,27,23,223,18,254,29,29,20,132,57,25,98,121,79,169,227,158,130,139,247,72,250,62,45,57,116,10,158,146,221,168,231,229,235,238,197,117,65,227,37,26,231,117,201,233,18,201,30,110,210,103,36,245,183,171,241,51,158,207,215,37,119,157,207,32,176,116,90,38,233,23,233,206,68,206,181,9,100,158,194,4,25,197,94,55,231,226,170,217,135,238,111,226,180,227,18,59,76,6,131,135,235,55,244,127,235,18,214,99,148,88,32,187,55,226,215,60,109,221,247,135,218,63,253,244,180,146,124,22,188,7,87,155,115,212,0,0,26,255,255,255,32,32,48,0,4,0,0,0,0,0,0,0
};

#if EVE_GENERATION > 2
/// Blob that needs to be stored in the external flash to be able to use it.
extern const uint8_t eve_blob_content[4096];
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief 	Initializes the register of the eve.
 */
static bool eve_init_chip(eve_t* obj);

/**
 * @brief	Checks the interrupt register if an interrupt occured and calls the eve_copro functions for touch interrupt
 * 			and command flag empty interrupt.
 *
 * 			Function is called inside eve_handle and does not need to be called outside this module.
 *
 * @pre		The eve_spi_init function must be called first.
 **/
static void eve_spi_handle(eve_t* obj);

/**
 * @brief Callback function for the pin interrupt. Sets the interrupt flag, nothing else.
 */
static void eve_spi_pin_int(eve_t* obj);

/**
 * @brief Calls the callback function if it is not NULL.
 */
static void eve_throw_error(eve_t* obj, EVE_ERROR err, const char* msg);

/**
 * @brief Write the touch firmware into the ram of the BT817
 * 
 * @param obj 				Pointer to eve handler
 * @param touch_fw 			Pointer to the byte array containing the touch firmware
 * @param size_of_touch_fw 	Size of the touch firmware in byte, as retrieved by sizeof(touch_fw).
 */
static void _write_touch_fw(eve_t* obj, const uint8_t* touch_fw, size_t size_of_touch_fw);
#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
static int _touch_task(struct pt* pt);
#endif
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Module Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

extern void eve_console_init(eve_t* eve);

extern void eve_copro_set_command_complete(eve_t* obj);	// Description in eve_copro.c

extern void eve_copro_tag_interrupt(eve_t* obj, uint8_t tag);	// Description in eve_copro.c

extern void eve_copro_touch_interrupt(eve_t* obj, uint16_t x, uint16_t y);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

bool eve_init(eve_t* obj, eve_hw_interface_t* hw, EVE_DISPLAY_TYPE type, bool rotate, bool has_touch, void* error_obj, eve_error_cb_t f_error)
{
	bool is_new_pd = true;
	if(obj == NULL)
		return false;
#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
	system_task_init_protothread(&obj->touch_task, false, _touch_task, obj);
#endif
	// Initialize variables
	obj->eve_dli = 0;
	obj->eve_display_width = 0;
	obj->eve_display_height = 0;
	obj->eve_is_initialized = false;
	obj->eve_spi_int_triggered = false;
	obj->eve_display_is_rotated = rotate;
	memcpy(&obj->hw, hw, sizeof(eve_hw_interface_t));
	obj->eve_copro_cli = 0;
	obj->eve_copro_cli_last_written = 0;
	obj->eve_copro_current_button_cnt = 0;
	obj->eve_copro_selected_button = NULL;
	obj->eve_copro_is_ready_flag = true;
	obj->eve_copro_release_touch = false;
	obj->eve_copro_touch_timestamp = 0;
	obj->eve_copro_touch_callback = NULL;
	obj->eve_copro_touch_callback_object = NULL;
	obj->selected_key = 0;
	obj->key_callback = NULL;
	obj->key_callback_object = NULL;
	obj->type = type;
	obj->error_callback = f_error;
	obj->error_obj = error_obj;
	obj->in_reintialization = false;
	obj->has_touch = has_touch;
	obj->is_powered = true;
	obj->next = NULL;
	obj->force_repaint = true;
	obj->first_reset_action_obj = NULL;
	obj->touch_mode = has_touch ? 3 : 0; // Set touch sampling mode
#if !EVE_USE_FT81X
	obj->vertex_format = 4;
#else
#if EVE_GENERATION == 3
	obj->api_level = 1;
#elif EVE_GENERATION == 4
	obj->api_level = 2;
#endif
#endif
	memset(obj->memory_files, 0, sizeof(obj->memory_files));
#if EVE_COPRO_DEBUG_COMMAND_COUNT
	obj->eve_copro_cmd_cnt = 0;
#endif

	eve_memory_clear(obj);

	// If the first eve is not set -> set this eve object as first
	if(_first_eve == NULL)
	{
		_first_eve = obj;
	}
	// Else: There is already a first f800 initialized -> add this eve object to the end of the list of eve objects!
	else
	{
		eve_t* fobj = _first_eve;
		while(fobj->next != NULL)
		{
			if(fobj->hw.io_pd == obj->hw.io_pd)
				is_new_pd = false;
			fobj = fobj->next;
		}
		fobj->next = obj;
	}

#if !EVE_USE_FT81X
	mcu_spi_set_param(hw->spi, MCU_SPI_MODE_3, 1000000);
#else
	mcu_spi_set_param(hw->spi, MCU_SPI_MODE_0, 1000000);
#endif
	mcu_io_interrupt_set_param(hw->io_int, (void*)obj, (void(*)(void*))eve_spi_pin_int, MCU_INT_LVL_MED, MCU_IO_INT_EDGE_LOW);

	// Do only set the io pin and make a reset if it is not a shared power down!
	if(is_new_pd)
	{
		mcu_io_set_dir(hw->io_pd, MCU_IO_DIR_OUT);	// Set Powerdown pin as output
		mcu_io_set(hw->io_pd, MCU_IO_LOW);			// Enable Powerdown
		MCU_IO_SET_HANDLER(hw->io_h_pwr, 0);
		mcu_wait_ms(20);							// Reset must be at least 5ms -> make it 6ms
		mcu_io_set(hw->io_pd, MCU_IO_HIGH);			// Disable Powerdown
		MCU_IO_SET_HANDLER(hw->io_h_pwr, 1);
		mcu_wait_ms(20);							// Reset must be at least 5ms -> make it 6ms
	}

	if(!eve_init_chip(obj))
	{
		eve_throw_error(obj, EVE_ERROR_INITIALIZATION_FAILED, "Initialization failed");
		return false;
	}

	obj->eve_is_initialized = true;

#if MODULE_ENABLE_CONSOLE
	if(_first_eve == obj)
	{
		eve_console_init(obj);
	}
#endif

	system_init_object_task(&obj->eve_spi_task, true, (void(*)(void*))eve_spi_handle, (void*)obj);

	return true;
}

void eve_init_touch(eve_t* obj)
{
#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
	if(obj->hw.external_touch.i2c)
	{
		// External touch connected to eve

		DBG_INFO("External touch connected to eve\n");

		if(obj->touch_device)
		{
			st1633i_free(obj->touch_device);
			obj->touch_device = NULL;
		}
		if(obj->touch)
		{
			lcd_touch_free(&obj->touch);
		}

        // Capacitive touch is used
		obj->touch_device = st1633i_create(&obj->hw.external_touch);
        if(obj->touch_device)
        {
            // TODO: Set flags based on display
            struct lcd_touch_config_s touch_config = 
            {
                .flags = {.mirror_x = false, .mirror_y = false, .swap_xy = false},
                .x_max = obj->eve_display_width,
                .y_max = obj->eve_display_height
            };
            lcd_touch_create(obj->touch_device, &st1633i_lcd_touch_interface, &touch_config, &obj->touch);
			
			eve_spi_write_8(obj, EVE_REG_CPURESET, 2);
			eve_spi_write_32(obj, EVE_REG_TOUCH_CONFIG, 0x00004000);
			eve_spi_write_8(obj, EVE_REG_CPURESET, 0);

			system_task_add(&obj->touch_task);
        }
		return;
	}
#else
		DBG_ERROR("Touch is not supported\n");
		return;
#endif
//	dbg_printf(DBG_STRING, "eve_init_touch(%d)\n", obj->type);
	switch(obj->type)
	{
		case EVE_DISPLAY_TYPE_4_3_480_272:

			if(obj->eve_display_is_rotated)
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0xfffff8c4);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0x00000001);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0x01daccfa);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0xffffffc1);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0xfffff9a5);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0x00fbce2c);
			}
			else
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0x00007F1D);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0xFFFFFFE3);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0xFFF2D1F3);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0xFFFFFFE6);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0xFFFFB45E);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0x011BCCFA);
			}

		break;

		case EVE_DISPLAY_TYPE_5_7_320_240:

			if(obj->eve_display_is_rotated)
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0xFFFFADCF);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0x00000013);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0x01432B3B);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0x00000156);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0x00004299);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0xFFF27C2D);
			}
			else
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0x00005233);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0x00000196);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0xfff7ad2d);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0x0000026f);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0xffffbf60);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0x00efd115);

//				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0x000053A4);
//				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0xFFFFFDE7);
//				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0x0005AECE);
//				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0xFFFFFF54);
//				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0xFFFFBDF4);
//				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0x00F4DB7E);
			}

		break;

		case EVE_DISPLAY_TYPE_ETML035023UDRA:

			_write_touch_fw(obj, _touch_fw_ili2130, sizeof(_touch_fw_ili2130));

			if(obj->eve_display_is_rotated)
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0xfffffb23);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0xfffffffe);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0x0137519c);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0x00000018);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0xfffff942);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0x00f2a9a4);
			}
			else
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0x000004f3);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0x00000069);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0xfffc338f);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0xffffffec);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0x00000647);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0x00057ce8);
			}
			
		break;

		case EVE_DISPLAY_TYPE_ETML043023UDRA:

			_write_touch_fw(obj, _touch_fw_ili2130, sizeof(_touch_fw_ili2130));

			if(obj->eve_display_is_rotated)
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0xfffff8b1);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0x0000001e);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0x01d69d48);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0xffffffad);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0xfffff8a5);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0x0111f811);
			}
			else
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0x00000777);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0xffffff9d);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0x000ba292);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0xffffffdd);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0x0000077b);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0x0003a20d);
			}

		break;

#if EVE_USE_FT81X
		case EVE_DISPLAY_TYPE_ETML050023UDRA:

			_write_touch_fw(obj, _touch_fw_ili2130, sizeof(_touch_fw_ili2130));

			if(obj->eve_display_is_rotated)
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0xfffff353);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0x0000001c);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0x031dfc48);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0xffffff91);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0xfffff365);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0x01e39cf5);
			}
			else
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0x00000c9a);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0xffffffc7);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0x000bc708);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0x00000002);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0x00000d9c);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0xfff5b427);
			}
			
		break;

		case EVE_DISPLAY_TYPE_ETML070023UDBA:

			_write_touch_fw(obj, _touch_fw_ili2130, sizeof(_touch_fw_ili2130));

			if(obj->eve_display_is_rotated)
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0xfffff34d);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0xffffffc5);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0x0326caf9);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0xfffffff4);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0xfffff383);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0x01d9023e);
			}
			else
			{
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_A, 0x00000c44);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_B, 0xfffffffe);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_C, 0x0000e956);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_D, 0x0000003c);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_E, 0x00000c92);
				eve_spi_write_32(obj, EVE_REG_TOUCH_TRANSFORM_F, 0x000a746d);
			}
			
		break;
#endif

		default: // No Touch
			return;
	}

	eve_spi_write_16(obj, EVE_REG_TOUCH_RZTHRESH, 1200);	// Sensitivity of the touch

	// Set other touch default values from eve datasheet -> Should not be necessary, but used for recalibration
	eve_spi_write_8(obj, EVE_REG_TOUCH_MODE, obj->touch_mode);	// Touch-screen sampling mode
	eve_spi_write_8(obj, EVE_REG_TOUCH_ADC_MODE, 1);			// Select single ended (low power) or differential (accurate) sampling
	eve_spi_write_16(obj, EVE_REG_TOUCH_CHARGE, 0x1770);		// Touch-screen charge time, units of 6 clocks
	eve_spi_write_8(obj, EVE_REG_TOUCH_SETTLE, 3);				// Touch-screen settle time, units of 6 clocks
	eve_spi_write_8(obj, EVE_REG_TOUCH_OVERSAMPLE, 7);			// Touch-screen oversample factor
}

void eve_switch_power(eve_t* obj, bool b)
{
	if(obj == NULL || obj->is_powered == b)
		return;

	obj->is_powered = b;

	if(b)
	{
		// Wait for at least 20ms between power off and power on...
		while( (system_get_tick_count() - obj->power_off_timestamp) < 20)
		{
			// Do nothing...
		}
	}

	mcu_io_set(obj->hw.io_pd, b);
	MCU_IO_SET_HANDLER(obj->hw.io_h_pwr, b); // Power On/Off
	if(b)
	{
		obj->in_reintialization = true;

		mcu_wait_ms(20); // Wait so that display can start-up

		if(!eve_init_chip(obj))
		{
			string_printf(obj->msg, "Re-Initialization of %d failed", obj->type);
			eve_throw_error(obj, EVE_ERROR_REINITIALIZATION_FAILED, obj->msg);
		}
		else
		{
			eve_reset_action_t* tmp = obj->first_reset_action_obj;
			obj->eve_copro_is_ready_flag = true;
			obj->eve_copro_release_touch = false;
			obj->eve_copro_cli = 0;
			obj->eve_copro_cli_last_written = 0;
			eve_memory_clear(obj);
			font_initalize_default_fonts(obj);
			obj->force_repaint = true;

			while(tmp != NULL)
			{
				if(tmp->f)
					tmp->f(tmp->p1, tmp->p2);
				tmp = tmp->next;
			}
		}

		obj->in_reintialization = false;

		system_add_task(&obj->eve_spi_task);
	}
	else
	{
		system_remove_task(&obj->eve_spi_task);
		obj->power_off_timestamp = system_get_tick_count();
	}
}

void eve_register_reset_action(eve_t* obj, eve_reset_action_t* action)
{
	if(obj->first_reset_action_obj == NULL)
	{
		// No action in list -> set as first action!
		obj->first_reset_action_obj = action;
	}
	else
	{
		// There is already a first object! Check if action is already in list!
		eve_reset_action_t* tmp = obj->first_reset_action_obj;

		while(tmp->next != NULL)
		{
			// If action was already added -> just return because it is already in the list.
			if(tmp == action)
				return;

			tmp = (eve_reset_action_t*)tmp->next;
		}
		// Action might be the first object if this function is called two times in the row!
		if(tmp != action)
			tmp->next = action; // Only added to the list if it is not added to itself
	}
	// Added as last action -> Set next to NULL
	action->next = NULL;
}

void eve_do_reset_actions(eve_t* obj)
{
	eve_reset_action_t* tmp = obj->first_reset_action_obj;
	obj->eve_copro_is_ready_flag = true;
	obj->eve_copro_release_touch = false;
	obj->eve_copro_cli = 0;
	obj->eve_copro_cli_last_written = 0;
	eve_memory_clear(obj);
	font_initalize_default_fonts(obj);
	obj->force_repaint = true;

	while(tmp != NULL)
	{
		if(tmp->f)
			tmp->f(tmp->p1, tmp->p2);
		tmp = tmp->next;
	}
}

void eve_set_touch_enabled(eve_t* obj, bool b)
{
	obj->touch_mode = b ? 3 /*Continuous*/ : 0/*Off*/;
	eve_spi_write_8(obj, EVE_REG_TOUCH_MODE, obj->touch_mode);	// Touch-screen sampling mode
}

void eve_dl_start(eve_t* obj)
{
	obj->eve_dli = 0;
}

void eve_dl(eve_t* obj, uint32_t cmd)
{
	eve_spi_write_32(obj, EVE_RAM_DL + obj->eve_dli, cmd);
	obj->eve_dli += 4;
}

void eve_dl_swap(eve_t* obj)
{
	eve_spi_write_32(obj, EVE_REG_DLSWAP, EVE_VAL_DLSWAP_FRAME);
}

#if EVE_GENERATION > 2

FUNCTION_RETURN_T eve_init_flash(eve_t* eve)
{
	EVE_FLASH_STATUS_T flash_status; //  Flash status. 0: INIT, 1: DETACHED, 2: BASIC, 3: FULL
	uint16_t result = -1; // Result of flashfast
	
	// Wait for init to complete...
	do
	{
		flash_status = eve_get_flash_status(eve);		
	}while(flash_status == EVE_FLASH_STATUS_INIT);
	
	// Attach the flash if necessary
	if(flash_status == EVE_FLASH_STATUS_DETACHED)
	{
		DBG_INFO("Attach flash\n");
		eve_copro_flash_attach(eve);
		
		flash_status = eve_get_flash_status(eve);
	}

	// if(flash_status == EVE_FLASH_STATUS_BASIC)
	// {
	// 	result = eve_copro_flash_fast(eve);
	// 	if(result != 0)
	// 		DBG_ERROR("Flash Fast: %04x\n", result);
		
	// 	flash_status = eve_get_flash_status(eve);
	// }	
	

	if(flash_status == EVE_FLASH_STATUS_BASIC)
	{
		uint8_t* buffer = mcu_heap_calloc(1, 4096);
		uint32_t ptr_ram = EVE_RAM_G + EVE_RAM_SIZE - 4096;
		uint32_t ptr_flash = 0;
		uint32_t length = 4096;
		
		eve_copro_flash_read_to_ram(eve, 0x00080000, 0, 4096);
		eve_spi_read_data(eve, 0x00080000, buffer, 4096);

		bool is_correct = memcmp(buffer, eve_blob_content, 4096) == 0;

		DBG_INFO("Blob: %s\n", is_correct ? "is ok" : "is not ok");

		// Write blob to RAM
		eve_spi_write_data(eve, ptr_ram, eve_blob_content, length, false);

		if(!is_correct)
		{
			DBG_INFO("Update Flash\n");
	
			// Update one sector from ram
			eve_copro_flash_update_from_ram(eve, ptr_ram, ptr_flash, length);
			
			flash_status = eve_get_flash_status(eve);
	
			if(flash_status != EVE_FLASH_STATUS_BASIC)
			{
				DBG_INFO("Attach flash\n");
				eve_copro_flash_attach(eve);
	
				flash_status = eve_get_flash_status(eve);
			}	
		}		

		// Free the buffer again
		mcu_heap_free(buffer);
	}

	if(flash_status == EVE_FLASH_STATUS_BASIC)
	{
		result = eve_copro_flash_fast(eve);
		if(result != 0)
			DBG_ERROR("Flash Fast: %04x\n", result);
		
		flash_status = eve_get_flash_status(eve);
	}	

	switch(flash_status)
	{
		case EVE_FLASH_STATUS_FULL: // Flash in fullspeed mode, this is what we need.
			return FUNCTION_RETURN_OK;

		case EVE_FLASH_STATUS_INIT: // Init means eve is still initializing the flash.
			return FUNCTION_RETURN_NOT_READY;

		case EVE_FLASH_STATUS_DETACHED:	// No flash was attached!
			return FUNCTION_RETURN_NOT_FOUND;

		case EVE_FLASH_STATUS_BASIC: // Flash could not be switched to full mode
			switch(result)
			{
				case 0:
					return FUNCTION_RETURN_OK;

				case 0xE001: // Flash is not supported
					return FUNCTION_RETURN_UNSUPPORTED;

				case 0xE002: // No Header detected in sector 0
					return FUNCTION_RETURN_WRITE_ERROR;

				case 0xE003: // Sector 0 data failed integrity check
					return FUNCTION_RETURN_INTEGRITYCHECK_FAILED;

				case 0xE004: // Device / blob mismatch
					return FUNCTION_RETURN_WRONG_DEVICE;

				case 0xE005: // Failed full-speed test
					return FUNCTION_RETURN_DEVICE_ERROR;

				default: 	// Unnown error
					DBG_ERROR("Unknown flashfast return %04x\n", result);
					return FUNCTION_RETURN_EXECUTION_ERROR;
			}
			break;
		
		default:	// Invalid flash status
			DBG_ERROR("Unknown flash status %d\n", flash_status);
			return FUNCTION_RETURN_EXECUTION_ERROR;
	}
}

EVE_FLASH_STATUS_T eve_get_flash_status(eve_t* eve)
{
	return eve_spi_read_8(eve, EVE_REG_FLASH_STATUS);
}

uint32_t eve_get_flash_size(eve_t* eve)
{
	return eve_spi_read_32(eve, EVE_REG_FLASH_SIZE);
}

#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static bool eve_init_chip(eve_t* obj)
{
	// uint8_t pclk_divider = 5;
	// PCLK Frequency to calculate the register for
	uint32_t pclk_hz = 0;
	uint32_t tmp32;
	uint8_t regval = 0;

	mcu_spi_set_clock(obj->hw.spi, EVE_SPI_SPEED_INIT);

#if EVE_DEBUG_SPI_SPEED
	dbg_printf(DBG_STRING, "EVE SPI Speed: %u\n", mcu_spi_get_frq(obj->hw.spi));
#endif

	eve_spi_send_host_command(obj, EVE_HOST_CMD_ACTIVE);
	eve_spi_send_host_command(obj, EVE_HOST_CMD_CLKEXT);
#if !EVE_USE_FT81X
	eve_spi_send_host_command(obj, EVE_HOST_CMD_CLK48M);
#endif

	mcu_spi_set_clock(obj->hw.spi, EVE_SPI_SPEED);
	
#if EVE_USE_FT81X
	obj->spi_width_flags = MCU_SPI_TRANS_FLAGS_NONE;
#endif

	// 300ms max startup time, then 0x7C must be readable!
	tmp32 = system_get_tick_count();
	while( (system_get_tick_count() - tmp32) < 300 && (regval = eve_spi_read_8(obj, EVE_REG_ID)) != 0x7C)
	{
		mcu_wait_ms(1);
	}
	if(regval != 0x7C)	// Identification register needs to be 0x7C
	{
		string_printf(obj->msg, "Invalid EVE Chip version %02x", regval);
		if(!obj->in_reintialization)
		{
			eve_throw_error(obj, EVE_ERROR_INVALID_CHIP_VERSION, obj->msg);
		}
		else
		{			
			DBG_ERROR("%s\n", obj->msg);
		}

		return false;
	}

#if EVE_USE_FT81X
	obj->spi_width_flags = MCU_SPI_TRANS_FLAGS_NONE;
	if(obj->hw.enable_quad_spi)
	{
		// eve_spi_write_32(obj, EVE_REG_SPI_WIDTH, 2);
		// uint32_t val = eve_spi_read_32(obj, EVE_REG_SPI_WIDTH);
		// if(val == 2)
		// {
			// DBG_INFO("Use Quad SPI\n");
		// 	obj->spi_width_flags = MCU_SPI_TRANS_FLAGS_QIO;
		// }
		// else
		// {
		// 	DBG_INFO("No Quad SPI, value: %02x\n", val);
		// }
	}
#endif

	switch(obj->type)
	{
		case EVE_DISPLAY_TYPE_4_3_480_272:

			obj->eve_display_width = 480;
			obj->eve_display_height = 272;

			eve_spi_write_16(obj, EVE_REG_HCYCLE, 548);
			eve_spi_write_16(obj, EVE_REG_HOFFSET, 43);
			eve_spi_write_16(obj, EVE_REG_HSIZE, 480);
			eve_spi_write_16(obj, EVE_REG_HSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_HSYNC1, 41);
			eve_spi_write_16(obj, EVE_REG_VCYCLE, 292);
			eve_spi_write_16(obj, EVE_REG_VOFFSET, 12);
			eve_spi_write_16(obj, EVE_REG_VSIZE, 272);
			eve_spi_write_16(obj, EVE_REG_VSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_VSYNC1, 10);

			pclk_hz = 12000000;

		break;

		case EVE_DISPLAY_TYPE_5_7_320_240:

			obj->eve_display_width = 320;
			obj->eve_display_height = 240;

			eve_spi_write_16(obj, EVE_REG_HCYCLE, 408);
			eve_spi_write_16(obj, EVE_REG_HOFFSET, 33);
			eve_spi_write_16(obj, EVE_REG_HSIZE, 320);
			eve_spi_write_16(obj, EVE_REG_HSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_HSYNC1, 30);
			eve_spi_write_16(obj, EVE_REG_VCYCLE, 263);
			eve_spi_write_16(obj, EVE_REG_VOFFSET, 4);
			eve_spi_write_16(obj, EVE_REG_VSIZE, 240);
			eve_spi_write_16(obj, EVE_REG_VSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_VSYNC1, 3);

			pclk_hz = 7500000;

		break;

		case EVE_DISPLAY_TYPE_ETML035023UDRA:

			obj->eve_display_width = 320;
			obj->eve_display_height = 240;

			eve_spi_write_16(obj, EVE_REG_HCYCLE, 371);
			eve_spi_write_16(obj, EVE_REG_HOFFSET, 43);
			eve_spi_write_16(obj, EVE_REG_HSIZE, 320);
			eve_spi_write_16(obj, EVE_REG_HSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_HSYNC1, 8);
			eve_spi_write_16(obj, EVE_REG_VCYCLE, 260);
			eve_spi_write_16(obj, EVE_REG_VOFFSET, 12);
			eve_spi_write_16(obj, EVE_REG_VSIZE, 240);
			eve_spi_write_16(obj, EVE_REG_VSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_VSYNC1, 8);

			pclk_hz = 8500000;

			eve_spi_write_8(obj, EVE_REG_SWIZZLE, 3);
			eve_spi_write_8(obj, EVE_REG_PCLK_POL, 1);

		break;

		case EVE_DISPLAY_TYPE_ETML043023UDRA:

			obj->eve_display_width = 480;
			obj->eve_display_height = 272;

			eve_spi_write_16(obj, EVE_REG_HCYCLE, 531);
			eve_spi_write_16(obj, EVE_REG_HOFFSET, 43);
			eve_spi_write_16(obj, EVE_REG_HSIZE, 480);
			eve_spi_write_16(obj, EVE_REG_HSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_HSYNC1, 8);
			eve_spi_write_16(obj, EVE_REG_VCYCLE, 292);
			eve_spi_write_16(obj, EVE_REG_VOFFSET, 12);
			eve_spi_write_16(obj, EVE_REG_VSIZE, 272);
			eve_spi_write_16(obj, EVE_REG_VSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_VSYNC1, 8);

			pclk_hz = 15000000;

			eve_spi_write_8(obj, EVE_REG_SWIZZLE, 3);
			eve_spi_write_8(obj, EVE_REG_PCLK_POL, 0);

		break;

#if EVE_USE_FT81X
		case EVE_DISPLAY_TYPE_7_800_480: // Higher resoultion only supported on FT81X

			obj->eve_display_width = 800;
			obj->eve_display_height = 480;

			eve_spi_write_16(obj, EVE_REG_HCYCLE, 928);
			eve_spi_write_16(obj, EVE_REG_HOFFSET, 88);
			eve_spi_write_16(obj, EVE_REG_HSIZE, 800);
			eve_spi_write_16(obj, EVE_REG_HSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_HSYNC1, 48);

			eve_spi_write_16(obj, EVE_REG_VCYCLE, 525);
			eve_spi_write_16(obj, EVE_REG_VOFFSET, 32);
			eve_spi_write_16(obj, EVE_REG_VSIZE, 480);
			eve_spi_write_16(obj, EVE_REG_VSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_VSYNC1, 3);

			pclk_hz = 30000000;

			eve_spi_write_8(obj, EVE_REG_SWIZZLE, 0);
			eve_spi_write_8(obj, EVE_REG_PCLK_POL, 1);
			eve_spi_write_8(obj, EVE_REG_CSPREAD, 0);
			eve_spi_write_8(obj, EVE_REG_DITHER, 1);

		break;

		case EVE_DISPLAY_TYPE_ETML050023UDRA:

			obj->eve_display_width = 800;
			obj->eve_display_height = 480;

			eve_spi_write_16(obj, EVE_REG_HCYCLE, 816);
			eve_spi_write_16(obj, EVE_REG_HOFFSET, 8);
			eve_spi_write_16(obj, EVE_REG_HSIZE, 800);
			eve_spi_write_16(obj, EVE_REG_HSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_HSYNC1, 8);

			eve_spi_write_16(obj, EVE_REG_VCYCLE, 496);
			eve_spi_write_16(obj, EVE_REG_VOFFSET, 8);
			eve_spi_write_16(obj, EVE_REG_VSIZE, 480);
			eve_spi_write_16(obj, EVE_REG_VSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_VSYNC1, 8);

			pclk_hz = 30000000;

			eve_spi_write_8(obj, EVE_REG_SWIZZLE, 3);
			eve_spi_write_8(obj, EVE_REG_PCLK_POL, 1);
			eve_spi_write_8(obj, EVE_REG_CSPREAD, 0);
			eve_spi_write_8(obj, EVE_REG_DITHER, 1);

		break;

		case EVE_DISPLAY_TYPE_ETML070023UDBA:

			obj->eve_display_width = 800;
			obj->eve_display_height = 480;

			eve_spi_write_16(obj, EVE_REG_HCYCLE, 860);
			eve_spi_write_16(obj, EVE_REG_HOFFSET, 16);
			eve_spi_write_16(obj, EVE_REG_HSIZE, 800);
			eve_spi_write_16(obj, EVE_REG_HSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_HSYNC1, 44);

			eve_spi_write_16(obj, EVE_REG_VCYCLE, 528);
			eve_spi_write_16(obj, EVE_REG_VOFFSET, 5);
			eve_spi_write_16(obj, EVE_REG_VSIZE, 480);
			eve_spi_write_16(obj, EVE_REG_VSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_VSYNC1, 43);

			pclk_hz = 30000000;

			eve_spi_write_8(obj, EVE_REG_SWIZZLE, 3);
			eve_spi_write_8(obj, EVE_REG_PCLK_POL, 0);
			eve_spi_write_8(obj, EVE_REG_CSPREAD, 0);
			eve_spi_write_8(obj, EVE_REG_DITHER, 1);

		break;

#if MODULE_ENABLE_DISPLAY && DISPLAY_ENABLE_SLD
		case EVE_DISPLAY_TYPE_SMM:
		{			
			DBG_ASSERT(obj->hw.external_touch.i2c, NO_ACTION, false, "I2C is missing for auto configuration\n");
			sld_edid_t edid;
			FUNCTION_RETURN ret = sld_edid_read(obj->hw.external_touch.i2c, &edid);
			DBG_ASSERT(ret == FUNCTION_RETURN_OK, NO_ACTION, false, "Read EDID failed\n");

			sld_debug_print(&edid);

			obj->has_touch = (edid.touch == SLD_TOUCH_INTERFACE_CAPACITIVE);

			obj->eve_display_width = edid.rgb.h_res;
			obj->eve_display_height = edid.rgb.v_res;

			eve_spi_write_16(obj, EVE_REG_HCYCLE, edid.rgb.h_res + edid.rgb.hsync_front_porch + edid.rgb.hsync_back_porch);
			eve_spi_write_16(obj, EVE_REG_HOFFSET, edid.rgb.hsync_back_porch);
			eve_spi_write_16(obj, EVE_REG_HSIZE, edid.rgb.h_res);
			eve_spi_write_16(obj, EVE_REG_HSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_HSYNC1, edid.rgb.hsync_front_porch);

			eve_spi_write_16(obj, EVE_REG_VCYCLE, edid.rgb.v_res + edid.rgb.vsync_front_porch + edid.rgb.vsync_back_porch);
			eve_spi_write_16(obj, EVE_REG_VOFFSET, edid.rgb.vsync_back_porch);
			eve_spi_write_16(obj, EVE_REG_VSIZE, edid.rgb.v_res);
			eve_spi_write_16(obj, EVE_REG_VSYNC0, 0);
			eve_spi_write_16(obj, EVE_REG_VSYNC1, edid.rgb.vsync_front_porch);

			pclk_hz = edid.rgb.pclk_hz;
			// Limit PCLK to 30MHz, because we had some issues with higher frequencies
			if(pclk_hz > 30000000)//(edid.rgb.v_res * edid.rgb.h_res * 60))
			{
				pclk_hz = 30000000;//(edid.rgb.v_res * edid.rgb.h_res * 60);
			}

			eve_spi_write_8(obj, EVE_REG_SWIZZLE, 0);
			eve_spi_write_8(obj, EVE_REG_PCLK_POL, 0);
			eve_spi_write_8(obj, EVE_REG_CSPREAD, 0);
			eve_spi_write_8(obj, EVE_REG_DITHER, 1);
		}
		break;
#endif // MODULE_ENABLE_DISPLAY && DISPLAY_ENABLE_SLD

#endif

		default:
			string_printf(obj->msg, "Invalid EVE Display Type %u", obj->type);
			if(!obj->in_reintialization)
			{
				eve_throw_error(obj, EVE_ERROR_INVALID_DISPLAYTYPE, obj->msg);
			}
			else
			{
				DBG_ERROR("%s\n", obj->msg);
			}

		return false;
	}

	eve_spi_write_32(obj, EVE_RAM_DL + 0, EVE_CLEAR_COLOR_RGB(0, 0, 0));
	eve_spi_write_32(obj, EVE_RAM_DL + 4, EVE_CLEAR(1, 1, 1));
	eve_spi_write_32(obj, EVE_RAM_DL + 8, EVE_DISPLAY());
	eve_spi_write_8(obj, EVE_REG_DLSWAP, EVE_VAL_DLSWAP_FRAME);

	// Calculate pclk
	{
		double pll;
		uint32_t best_reg_pll = 0;
		uint32_t best_reg_pclk = 0;
		double best_error = 228000000.0;

		// PLL Multiplier maximum = 19, because 228MHz is maximum PLL frequency
		for (int pll_reg = 1; pll_reg <= 19; pll_reg++) 
		{
			pll = 12000000.0 * pll_reg;

			for (int pclk_reg = 1; pclk_reg <= 255; pclk_reg++) 
			{ 
				double pclk = pll / (pclk_reg * 2.0);
				double error = (double)pclk_hz - pclk;

				if (error < best_error && error >= 0.0) 
				{
					// DBG_INFO("PCLK: %u, PLL: %u, PCLK_DIV: %u, Error: %m, Best Error: %m\n", (uint32_t)(pclk), pll_reg, pclk_reg, (int32_t)(error * 100.0), (int32_t)(best_error * 100.0));
					best_error = error;
					best_reg_pll = pll_reg;
					best_reg_pclk = pclk_reg;
				}
			}
		}
		uint16_t reg_pclk_freq = (best_reg_pll << 4) | best_reg_pclk;
		if(best_reg_pll >= 14) // [11:10] = 3 for PLL 160 - 228 MHz
		{
			reg_pclk_freq |= 0xC00;
		}
		else if(best_reg_pll >= 7) // [11:10] = 2 for PLL 80 - 160MHz
		{
			reg_pclk_freq |= 0x800;
		}
		else if(best_reg_pll >= 4) // [11:10] = 1 for PLL 40 - 80MHz
		{
			reg_pclk_freq |= 0x400;
		}

		DBG_INFO("PCLK: %u, PLL: %u, PCLK_DIV: %u, REG: %3X\n", (uint32_t)((12000000 * best_reg_pll) / (2 * best_reg_pclk)), best_reg_pll, best_reg_pclk, reg_pclk_freq);
		
		// Set PCLK Divider to 1 to use PLL
		eve_spi_write_8(obj, EVE_REG_PCLK, 1);	
		eve_spi_write_16(obj, EVE_REG_PCLK_FREQ, reg_pclk_freq);
	}

	eve_spi_write_8(obj, EVE_REG_ROTATE, obj->eve_display_is_rotated); // Rotate the display if necessary

#if EVE_USE_FT81X
	eve_spi_write_16(obj, EVE_REG_GPIOX_DIR, 0x8002);		// Set LCD Enable Pin as Output
	eve_spi_write_16(obj, EVE_REG_GPIOX, 0x8002 | eve_spi_read_16(obj, EVE_REG_GPIOX));				// Enable LCD via Enable Pin
#else
	eve_spi_write_8(obj, EVE_REG_GPIO_DIR, 0x80);		// Set LCD Enable Pin as Output
	eve_spi_write_8(obj, EVE_REG_GPIO, 0x80 | eve_spi_read_8(obj, EVE_REG_GPIO));				// Enable LCD via Enable Pin
#endif

#if EVE_DEBUG_SPI_SPEED
	dbg_printf(DBG_STRING, "EVE SPI Speed: %u\n", mcu_spi_get_frq(obj->hw.spi));
#endif

	// From Glyn ADAM manual -> Wait for stability. Might lead to make the workaround below unnecessary.
	mcu_wait_ms(100);

	// Well there is a bug in EVE where the value for REG_PWM_DUTY is ignored sometimes, so set it multiple times until
	// the correct value is read! Try it for 300ms max. Seems only to happen for this register.
	tmp32 = system_get_tick_count();
	do
	{
		eve_spi_write_8(obj, EVE_REG_PWM_DUTY, 0);
	}while(eve_spi_read_8(obj, EVE_REG_PWM_DUTY) != 0 && ((system_get_tick_count() - tmp32) < 300));

	eve_spi_write_16(obj, EVE_REG_PWM_HZ, 1000);

	// Mask Interrupts -> Tag, Touch and Swap
	if(obj->has_touch)
		eve_spi_write_8(obj, EVE_REG_INT_MASK, EVE_VAL_INT_SWAP | EVE_VAL_INT_TAG | EVE_VAL_INT_CONVCOMPLETE);
	else // Mask Interrupts -> Only Swap
		eve_spi_write_8(obj, EVE_REG_INT_MASK, EVE_VAL_INT_SWAP);
	eve_spi_write_8(obj, EVE_REG_INT_EN, 1);				// Activate Interrupts

	if(obj->has_touch)
		eve_init_touch(obj);

	eve_spi_write_8(obj, EVE_REG_CPURESET, 1);
	eve_spi_write_32(obj, EVE_REG_CMD_READ, 0);
	eve_spi_write_32(obj, EVE_REG_CMD_WRITE, 0);
	eve_spi_write_8(obj, EVE_REG_CPURESET, 0);
	
	// Wait for reset to finish
	mcu_wait_us(10000);

	// Set the api level
	eve_copro_set_apilevel(obj, obj->api_level);

	dbg_printf(DBG_STRING, "EVE[%d]initialized\n", obj->type);

	return true;
}

static void eve_spi_handle(eve_t* obj)
{
	static uint8_t int_value = 0;

	if(obj == NULL)
		return;

	if((system_get_tick_count() - obj->int_timestamp) >= 20)
	{
		obj->int_timestamp = system_get_tick_count();
		obj->eve_spi_int_triggered = true;
	}

	if(obj->eve_spi_int_triggered)
	{
		obj->eve_spi_int_triggered = false;
		int_value = eve_spi_read_8(obj, EVE_REG_INT_FLAGS);

		if(int_value > 0)
		{
			if(obj->has_touch)
			{
				if(int_value & EVE_VAL_INT_TAG)
				{
					uint8_t tmp8 = eve_spi_read_8(obj, EVE_REG_TOUCH_TAG);
					eve_copro_tag_interrupt(obj, tmp8);
				}

				if(int_value & EVE_VAL_INT_CONVCOMPLETE)
				{
					uint32_t tmp32 = eve_spi_read_32(obj, EVE_REG_TOUCH_SCREEN_XY);
					eve_copro_touch_interrupt(obj, (tmp32 >> 16) & 0xFFFF, tmp32 & 0xFFFF);
				}
			}

			if(int_value & EVE_VAL_INT_SWAP)
			{
				eve_copro_set_command_complete(obj);
				if(eve_spi_read_16_twice(obj, EVE_REG_CMD_READ) >= 0xFFF)
				{
#if EVE_USE_FT81X
					eve_spi_read_data(obj, EVE_RAM_ERR_REPORT, (uint8_t*)obj->msg, sizeof(obj->msg));
#else
					string_printf(obj->msg, "Coprocessor Error -> Reset");
#endif
					eve_throw_error(obj, EVE_ERROR_INVALID_COPROCESSOR_ERROR, obj->msg);

					// Make a hardware reset!
					eve_switch_power(obj, false);
					eve_switch_power(obj, true);
				}
			}
		}
		obj->int_timestamp = system_get_tick_count();
	}
}

static void eve_spi_pin_int(eve_t* obj)
{
	if(obj == NULL)
		return;

	obj->eve_spi_int_triggered = true;
}

static void eve_throw_error(eve_t* obj, EVE_ERROR err, const char* msg)
{
	if(obj->error_callback != NULL)
		obj->error_callback(obj->error_obj, err, msg);
}

static void _write_touch_fw(eve_t* obj, const uint8_t* touch_fw, size_t size_of_touch_fw)
{	
	eve_memory_write_to(obj, EVE_RAM_CMD, touch_fw, size_of_touch_fw);
	eve_spi_write_32(obj, EVE_REG_CMD_WRITE, size_of_touch_fw);

	uint32_t timestamp = system_get_tick_count();
	bool finished_update = false;
	do
	{
		finished_update = (eve_spi_read_16_twice(obj, EVE_REG_CMD_READ) == eve_spi_read_16_twice(obj, EVE_REG_CMD_WRITE));
	}while( (system_get_tick_count() - timestamp) < 500 && !finished_update);
}

#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
static int _touch_task(struct pt* pt)
{
	static uint16_t x_old[5] = {0};
	static uint16_t y_old[5] = {0};
	static uint16_t strength_old[5] = {0};
	static uint8_t point_num_old = 0;
	static int i;

	eve_t* eve = pt->obj;

	PT_BEGIN(pt);

	// Write a touch released once to start the touch engine
	eve_spi_write_16(eve, EVE_REG_EHOST_TOUCH_X, 0x8000);
	eve_spi_write_16(eve, EVE_REG_EHOST_TOUCH_Y, 0x8000);
	eve_spi_write_8(eve, EVE_REG_EHOST_TOUCH_ID, 0);
	eve_spi_write_8(eve, EVE_REG_EHOST_TOUCH_ID, 0xF);

	while(1)
	{
		PT_YIELD_MS(pt, 10);
		lcd_touch_read_data(eve->touch);
		uint16_t x[5] = {0};
		uint16_t y[5] = {0};
		uint16_t strength[5] = {0};
		uint8_t point_num = 0;
		lcd_touch_get_xy(eve->touch, x, y, strength, &point_num, 5);

		bool changed = (point_num != point_num_old);
		changed |= (memcmp(x, x_old, sizeof(x)) != 0);
		changed |= (memcmp(y, y_old, sizeof(y)) != 0);
		changed |= (memcmp(strength, strength_old, sizeof(strength)) != 0);
		
		if(changed)
		{
			memcpy(x_old, x, sizeof(x));
			memcpy(y_old, y, sizeof(y));
			memcpy(strength_old, strength, sizeof(strength));
			point_num_old = point_num;

			// Wait until touch engine is ready to accept a touch
			PT_YIELD_UNTIL(pt, eve_spi_read_8(eve, EVE_REG_EHOST_TOUCH_ACK) == 1);

			if(point_num_old == 0)
			{
				// Set this for a touch release event
				eve_spi_write_16(eve, EVE_REG_EHOST_TOUCH_X, 0x8000);
				eve_spi_write_16(eve, EVE_REG_EHOST_TOUCH_Y, 0x8000);
				eve_spi_write_8(eve, EVE_REG_EHOST_TOUCH_ID, 0);
				eve_spi_write_8(eve, EVE_REG_EHOST_TOUCH_ID, 0xF);
			}
			else
			{
				// Write all touches to the touch engine
				for(i = 0; i < point_num_old; i++)
				{
					eve_spi_write_16(eve, EVE_REG_EHOST_TOUCH_X, x_old[i]);
					eve_spi_write_16(eve, EVE_REG_EHOST_TOUCH_Y, y_old[i]);
					eve_spi_write_8(eve, EVE_REG_EHOST_TOUCH_ID, i);
					eve_spi_write_8(eve, EVE_REG_EHOST_TOUCH_ID, 0xF);					
				}
			}
		}		
	}

	PT_END(pt);
}
#endif

#endif
