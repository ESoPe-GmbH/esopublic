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

#if CONFIG_IDF_TARGET_ESP32P4

#include "module/lcd_touch/lcd_touch_calibration.h"
#include "module/lcd_touch/driver/st1633i/st1633i.h"
#include "module/display/sld/display_sld.h"
#include "module/eeprom/eeprom_i2c.h"


#include "esp_partition.h"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
    

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

// TODO: Interface ESP-Hosted? S0=GPIO39, S1=GPIO40, S2=GPIO41, D3=GPIO42, CLK=GPIO43, CMD=GPIO44

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
            .r = {GPIO27, GPIO34, GPIO26, GPIO33, GPIO25, GPIO32, GPIO24, GPIO31},
            .g = {GPIO23, GPIO30, GPIO22, GPIO29, GPIO21, GPIO28, GPIO20, GPIO12},
            .b = {GPIO19, GPIO11, GPIO18, GPIO10, GPIO17, GPIO9, GPIO16, GPIO8},
            .pclk = GPIO5,
            .de = GPIO6,
            .hsync = GPIO15,
            .vsync = GPIO7,
            .data_width = 24,
            .disp_en = GPIO50
        }
    },
    .backlight = 
    {
        .timer_unit = 0,
        .timer_channel = 0,
        .output_pin = GPIO45
    },
    .touch = 
    {        
        .i2c = &_i2c_touch,
        .io_reset = GPIO51,
    }
};

static mcu_uart_hw_config_t _uart_hw_config_peripheral = {
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

static mcu_uart_config_t _uart_config_peripheral = {
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

// flash_device_t board_flash_device_data;

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

	i2c_init(&_i2c_touch, 0, GPIO52, GPIO13);
	i2c_set_frq(&_i2c_touch, 400000);

    mcu_io_set_dir(GPIO14, MCU_IO_DIR_IN);
    _sld_hw.touch.io_int = mcu_io_interrupt_init(1, GPIO14);
	
    board_lcd = display_sld_init_hardware(&_sld_hw);
    DBG_INFO("Display %s initialized\n", board_lcd == NULL ? "not" : board_lcd->screen_diagonal);

	board_uart_peripheral = mcu_uart_create(&_uart_hw_config_peripheral, &_uart_config_peripheral);
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