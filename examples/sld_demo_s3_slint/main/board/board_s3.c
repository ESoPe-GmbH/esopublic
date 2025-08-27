/**
 * 	@file 	board.c
 * 	@copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten
 *
 *	@brief 	Implements the function board_init from the board.h and initializes the variables declared in the board_specific.h
 **/

#include <string.h>
#include "board.h"
#include "mcu/sys.h"
#include "module/console/dbg/debug_console.h"
#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32S3 && !CONFIG_SLD_C_W_S3_BT817

#include "module/lcd_touch/lcd_touch_calibration.h"
#include "module/lcd_touch/driver/st1633i/st1633i.h"
#include "module/display/sld/display_sld.h"
#include "module/eeprom/eeprom_i2c.h"

#include "esp_partition.h"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define LCD_IO_R4 12
#define LCD_IO_R3 19
#define LCD_IO_R2 13
#define LCD_IO_R1 20
#define LCD_IO_R0 14

#define LCD_IO_G5 10
#define LCD_IO_G4 18
#define LCD_IO_G3 11
#define LCD_IO_G2 3
#define LCD_IO_G1 21
#define LCD_IO_G0 8

#define LCD_IO_B4 7
#define LCD_IO_B3 16
#define LCD_IO_B2 46
#define LCD_IO_B1 17
#define LCD_IO_B0 9

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
    

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

// GPIO_EXTERNAL

#if MODULE_ENABLE_COMM
/// Interface for debug prints and console.
static comm_t		_comm_debug;
#endif

static i2c_t _i2c_touch;

static display_sld_hardware_t _sld_hw = 
{
    .display = 
    {
        .display = DISPLAY_DEVICE_SLD,
        .interface = DISPLAY_INTERFACE_RGB,
        .rgb = 
        {
            .r = {LCD_IO_R0, LCD_IO_R1, LCD_IO_R2, LCD_IO_R3, LCD_IO_R4, PIN_NONE, PIN_NONE, PIN_NONE},
            .g = {LCD_IO_G0, LCD_IO_G1, LCD_IO_G2, LCD_IO_G3, LCD_IO_G4, LCD_IO_G5, PIN_NONE, PIN_NONE},
            .b = {LCD_IO_B0, LCD_IO_B1, LCD_IO_B2, LCD_IO_B3, LCD_IO_B4, PIN_NONE, PIN_NONE, PIN_NONE},
            .pclk = GPIO4,
            .de = GPIO5,
            .hsync = GPIO15,
            .vsync = GPIO6,
            .data_width = 16,
            .disp_en = GPIO42
        }
    },
    .backlight = 
    {
        .timer_unit = 0,
        .timer_channel = 0,
        .output_pin = GPIO39
    },
    .touch = 
    {        
        .i2c = &_i2c_touch,
        .io_reset = GPIO2,
    }
};

static mcu_uart_hw_config_t _uart_hw_config_485 = {
	.unit = 1,
	.io_tx = GPIO48,
	.io_rx = GPIO47,
	.io_rts = PIN_NONE,
	.io_cts = PIN_NONE,
	.receive_buffer_size = 8192,
	.receive_interrupt_level = MCU_INT_LVL_MED,
	.transmit_buffer_size = 8192,
	.transmit_interrupt_level = MCU_INT_LVL_MED
};

static mcu_uart_config_t _uart_config_485 = {
	.baudrate = 250000,
	.databits = 8,
	.parity = 'N',
	.stopbits = 1,
	.mode = MCU_UART_MODE_UART_NO_FLOW_CONTROL
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External variables -> Contains the variables stated in the board_specific.h.
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

display_sld_handle_t board_lcd = NULL;

mcu_uart_t board_uart_peripheral;

comm_t board_comm_peripheral;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Function
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void board_init(void)
{
	// Initialize the mcu with an external clock on the maximum speed.
	mcu_init_max_internal();

#if MODULE_ENABLE_COMM
	// Create uarts and assign it to a comm handler.
	_comm_debug.device_handler = mcu_uart_init(MCU_UART_INIT_DEBUG_PARAM);
	mcu_uart_create_comm_handler(_comm_debug.device_handler, &_comm_debug);

	// Set UART for debug
	mcu_uart_set_param(_comm_debug.device_handler, 500000, 8, 'N', 1);
	dbg_set_comm(&_comm_debug);
#endif

	i2c_init(&_i2c_touch, 0, GPIO1, GPIO41);
	i2c_set_frq(&_i2c_touch, 400000);

    mcu_io_set_dir(GPIO40, MCU_IO_DIR_IN);
    _sld_hw.touch.io_int = mcu_io_interrupt_init(1, GPIO40);
	
    board_lcd = display_sld_init_hardware(&_sld_hw);
    DBG_INFO("Display %s initialized\n", board_lcd == NULL ? "not" : board_lcd->screen_diagonal);

	board_uart_peripheral = mcu_uart_create(&_uart_hw_config_485, &_uart_config_485);
	mcu_uart_set_param(board_uart_peripheral, 115200, 8, 'N', 1);
	mcu_uart_create_comm_handler(board_uart_peripheral, &board_comm_peripheral);

	// Enable Interrupts
	mcu_enable_interrupt();
}

void board_set_backlight(float pwm)
{
    mcu_pwm_set_duty_cycle(board_lcd->backlight, (uint32_t)(pwm * 100.0));
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Function
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif