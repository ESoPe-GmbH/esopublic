/**
 * @file app_main.c
 * @copyright Urheberrecht 2018-2024 ESoPe GmbH, Alle Rechte vorbehalten
 */
#include "module/comm/dbg.h"
#include "module/version/version.h"
#include "module/console/dbg/debug_console.h"
#include "module/flash_info/flash_info.h"
#include "board/board.h"
#include "board/board_test.h"
#include "app_webserver.h"

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
 * @brief Initialize the graphical framework and start an internal task for handling the display.
 * 
 * @return true     Initialization was successfull and UI is shown to the user.
 * @return false    Initialization failed.
 */
extern bool app_ui_init(void);
#if defined(KERNEL_USES_SLINT)
/**
 * @brief Stops the UI task
 */
extern void app_ui_stop(void);
/**
 * @brief Callback function that is called upon "test start" to disable the logic of the application.
 * 
 * @param obj           Custom object pointer from debug_console_test_t.
 * @param data          Pointer to the console.
 * @param args          List of arguments.
 * @param args_len      Number of arguments.
 */
static void _dbc_test_handle(void* obj, console_data_t* data, char** args, uint8_t args_len);
#endif
/**
 * @brief Callback for backlight command
 * 
 * @param data          Console data
 * @param args          List of arguments
 * @param args_len      Number of elements in args
 * @return FUNCTION_RETURN FUNCTION_RETURN_OK on success, other on error
 */
static FUNCTION_RETURN _cmd_callback(console_data_t* data, char** args, uint8_t args_len);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(KERNEL_USES_SLINT)
/// Handler for test start.
static debug_console_test_t _dbc_test;
#endif

/// Structure for the pwm console command
static console_command_t _cmd = 
{
    .command = "backlight",
    .fnc_exec = _cmd_callback,
    .use_array_param = true,
    .explanation = "Set the backlight duty cycle"
};

static console_data_t _console_data_peripheral;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void app_main_init(void)
{
    version_set(25002, 2);

    DBG_INFO("Init SLD_Demo (Version %s Serial %u [" __DATE__ " " __TIME__ "]). Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten\n", version_get_string(), flash_info_get_hardware_id());

    // Attach the console to the peripheral UART.
    console_init(&_console_data_peripheral, &board_comm_peripheral);

#if CONFIG_IDF_TARGET_ESP32P4
    app_camera_init();
#endif

    app_ui_init();

#if MCU_PERIPHERY_ENABLE_WIFI
    app_webserver_init();
#endif

    board_test_init();
#if defined(KERNEL_USES_SLINT)
    debug_console_register_test_callback(&_dbc_test, NULL, _dbc_test_handle);
#endif
	console_add_command(&_cmd);
}

#if SYSTEM_ENABLE_APP_MAIN_HANDLE
void app_main_handle(void)
{
}
#endif
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(KERNEL_USES_SLINT)
static void _dbc_test_handle(void* obj, console_data_t* data, char** args, uint8_t args_len)
{
	DBG_INFO("Enter testmode\n");
    app_ui_stop();
}
#endif

static FUNCTION_RETURN _cmd_callback(console_data_t* data, char** args, uint8_t args_len)
{
	if(args_len >= 1)
	{
		uint8_t dc = strtol(args[0], NULL, 10);
        
        board_set_backlight((float)dc);

		return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 5, "%u", dc);
	}
	return FUNCTION_RETURN_PARAM_ERROR;
}