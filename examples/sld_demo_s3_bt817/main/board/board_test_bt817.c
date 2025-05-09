/**
 * @file board_test_bt817.c
 **/

#include "board_test.h"
#include "sdkconfig.h"
#include "module/console/dbg/debug_console.h"

#if CONFIG_IDF_TARGET_ESP32S3 && CONFIG_SLD_C_W_S3_BT817
#include "board.h"
#include "module/gui/eve/eve_spi.h"
#include "module/eeprom/eeprom_i2c.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Callback function that is called upon "test start" to disable the logic of the application.
 * 
 * @param obj           Custom object pointer from debug_console_test_t.
 * @param data          Pointer to the console.
 * @param args          List of arguments.
 * @param args_len      Number of arguments.
 */
static void _dbc_test_handle(void* obj, console_data_t* data, char** args, uint8_t args_len);

/**
* @brief Is called to check if the I2C EEPROM on BT817 testadapter was found.
* 
* @param name 			Name of the flag in the testsystem "eeprom_bt817_i2c"
* @return int 			1 if found, 0 on error.
*/
static int _flag_eeprom(const char* name);

/**
 * @brief Is called to trigger the audio test
 * 
 * @param name 			Name of the flag in the testsystem "audio"
 * @return int 			1 if found, 0 on error.
 */
static int _flag_audio(const char* name);
/**
 * @brief Handle for the protothread that tests audio output
 * 
 * @param pt            Protothread pointer
 * @return int          Protothread return value
 */
static int _handle_audio(struct pt* pt);
/**
 * @brief Is called to check if the EVE is found and initialized.
 * 
 * @param name          Name of the flag in the testsystem "eve"
 * @return int          1 if found, 0 on error.
 */
static int _flag_eve(const char* name);
/**
 * @brief Is called to check if the flash connected to the eve is working.
 * 
 * @param name          Name of the flag in the testsystem "flash_eve"   
 * @return int          1 if found, 0 on error.
 */
static int _flag_eve_flash(const char* name);
/**
 * @brief Is called to check if the display was initialized successfully.
 * 
 * @param name          Name of the flag in the testsystem "display"
 * @return int          1 if found, 0 on error.
 */
static int _flag_display(const char* name);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Handler for test start.
static debug_console_test_t _dbc_test;
/// @brief Task for testing audio
static system_task_t		_task_audio;
/// @brief Is set to true if the testadapter eeprom could be read successfully.
static bool _found_eeprom = false;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void board_test_init(void)
{
    debug_console_register_test_callback(&_dbc_test, NULL, _dbc_test_handle);

    char data[30] = {0};
    FUNCTION_RETURN ret;
    // Create handle to read the eeprom and free it after reading again
    const eeprom_i2c_config_t eeprom_config = EEPROM_I2C_M24C01(&board_pmod_i2c.i2c, 0, 0, 0);
    eeprom_device_t eeprom = eeprom_i2c_init(&eeprom_config);
    if(eeprom)
    {
        ret = eeprom_i2c_read(eeprom, 0, data, sizeof(data));
        if(ret == FUNCTION_RETURN_OK)
        {
            DBG_INFO("EEPROM data: %s\n", data);
            _found_eeprom = strcmp(data, "TESTADAPTER_BT817") == 0;
        }
        else
        {
            DBG_ERROR("Error reading eeprom\n");
        }
        eeprom_i2c_free(eeprom);        
    }
    
	debug_console_register_flag("eeprom", _flag_eeprom);
	debug_console_register_flag("audio", _flag_audio);
	debug_console_register_flag("eve", _flag_eve);
	debug_console_register_flag("flash_eve", _flag_eve_flash);
	debug_console_register_flag("display", _flag_display);

    system_task_init_protothread(&_task_audio, false, _handle_audio, NULL);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _dbc_test_handle(void* obj, console_data_t* data, char** args, uint8_t args_len)
{
	// Free the PMOD interface to test the pins manually.
	pmod_free(&board_pmod_spi_1);
	pmod_free(&board_pmod_spi_2);
	pmod_free(&board_pmod_uart);
}

static int _flag_eeprom(const char* name)
{
	return _found_eeprom;
}

static int _flag_audio(const char* name)
{
    system_task_add(&_task_audio);

	return true;
}

static int _handle_audio(struct pt* pt)
{
	PT_BEGIN(pt);
    eve_t* eve = &board_screen_device.eve;
    mcu_io_set(board_io_audio_enable, 1);
    eve_spi_write_8(eve, EVE_REG_VOL_SOUND, 0x20);
    // eve_spi_write_16(eve, EVE_REG_SOUND, (0x6C<<8) | 0x41);
    eve_spi_write_16(eve, EVE_REG_SOUND, (84<<8) | 0x02);
    eve_spi_write_8(eve, EVE_REG_PLAY, 1);
	PT_YIELD_MS(pt, 500);
    eve_spi_write_8(eve, EVE_REG_VOL_SOUND, 0);
    eve_spi_write_16(eve, EVE_REG_SOUND, 0);
    eve_spi_write_8(eve, EVE_REG_PLAY, 1);
	PT_YIELD_MS(pt, 5);
    mcu_io_set(board_io_audio_enable, 0);
	PT_END(pt);
}

static int _flag_eve(const char* name)
{
	return board_screen_device.eve.status == EVE_STATUS_OK || board_screen_device.eve.status == EVE_STATUS_READING_EDID_FAILED;
}

static int _flag_eve_flash(const char* name)
{
	return eve_get_flash_status(&board_screen_device.eve) == EVE_FLASH_STATUS_FULL;
}

static int _flag_display(const char* name)
{
	return board_screen_device.eve.status == EVE_STATUS_OK;
}

#endif