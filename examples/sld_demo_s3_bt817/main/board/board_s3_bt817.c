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

#if CONFIG_IDF_TARGET_ESP32S3 && CONFIG_SLD_C_W_S3_BT817

#include "module/lcd_touch/lcd_touch_calibration.h"
#include "module/lcd_touch/driver/st1633i/st1633i.h"
#include "module/display/sld/display_sld.h"
#include "module/eeprom/eeprom_i2c.h"
#include "module/gui/eve/eve.h"
#if KERNEL_USES_LVGL || KERNEL_USES_SLINT
#include "module/gui/eve/eve_lcd.h"
#endif
#include "module/gui/eve/eve.h"
#include "module/gui/eve_ui/screen.h"

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

// GPIO_EXTERNAL

#if MODULE_ENABLE_COMM
/// Interface for debug prints and console.
static comm_t		_comm_debug;
#endif

static i2c_t _i2c_touch;

static eve_hw_interface_t _eve_hw = {0};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External variables -> Contains the variables stated in the board_specific.h.
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

display_sld_handle_t board_lcd = NULL;

mcu_uart_t board_uart_peripheral;

comm_t board_comm_peripheral;

screen_device_t board_screen_device;

MCU_IO_PIN board_io_audio_enable = GPIO14;

pmod_t board_pmod_spi_1 = 
{
	.interface = PMOD_INTERFACE_SPI,
	.is_extended = true,
	.interface_num = 1,
	.spi_mosi = GPIO10,
	.spi_miso = GPIO9,
	.spi_sck = GPIO11,
	.spi_cs = GPIO12,
	.spi_cs2 = GPIO13,
	.spi_cs3 = GPIO14,
	.spi_reset = GPIO47,
	.spi_int = GPIO21
};

pmod_t board_pmod_spi_2 = 
{
	.interface = PMOD_INTERFACE_SPI,
	.is_extended = true,
	.interface_num = 1,
	.spi_mosi = GPIO10,
	.spi_miso = GPIO9,
	.spi_sck = GPIO11,
	.spi_cs = GPIO19,
	.spi_cs2 = GPIO20,
	.spi_cs3 = PIN_NONE,
	.spi_reset = GPIO46,
	.spi_int = GPIO3
};

pmod_t board_pmod_spi_3 = 
{
	.interface = PMOD_INTERFACE_SPI,
	.is_extended = true,
	.interface_num = 2,
	.spi_mosi = GPIO16,
	.spi_miso = GPIO17,
	.spi_sck = GPIO18,
	.spi_cs = GPIO15,
	.spi_cs2 = GPIO7,
	.spi_cs3 = GPIO6,
	.spi_reset = GPIO4,
	.spi_int = GPIO5
};

pmod_t board_pmod_i2c = 
{
	.interface = PMOD_INTERFACE_I2C,
	.i2c_sda = GPIO45,
	.i2c_scl = GPIO39,
	.i2c_int = GPIO41,
	.i2c_reset = GPIO40
};

pmod_t board_pmod_uart = 
{
	.interface = PMOD_INTERFACE_UART,
	.interface_num = 1,
	.uart_txd = GPIO1,
	.uart_rxd = GPIO2,
	.uart_cts = PIN_NONE,
	.uart_rts = GPIO42
};

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

    i2c_init(&_i2c_touch, 1, GPIO48, GPIO8);
	i2c_set_frq(&_i2c_touch, 400000);

	pmod_init(&board_pmod_spi_1); 
	pmod_init(&board_pmod_spi_2);
	pmod_init(&board_pmod_spi_3);
	pmod_init(&board_pmod_i2c);
	pmod_init(&board_pmod_uart);
	
	board_uart_peripheral = board_pmod_uart.uart;
	mcu_uart_set_param(board_uart_peripheral, 115200, 8, 'N', 1);
	mcu_uart_create_comm_handler(board_uart_peripheral, &board_comm_peripheral);
	
	_eve_hw.spi = board_pmod_spi_3.spi;
	_eve_hw.io_pd = board_pmod_spi_3.spi_reset;
	mcu_io_set_pullup(board_pmod_spi_3.spi_int, true);
	_eve_hw.io_int = mcu_io_interrupt_init(5, board_pmod_spi_3.spi_int);
	_eve_hw.io_sound_enable.pin = board_io_audio_enable;
	_eve_hw.io_h_pwr.pin = PIN_NONE;
	_eve_hw.enable_quad_spi = false;
	_eve_hw.external_touch.i2c = &_i2c_touch;
	_eve_hw.external_touch.io_reset = PIN_NONE;
	_eve_hw.external_touch.io_int = NULL;
	_eve_hw.external_touch.use_protothread = false;
		
    mcu_io_set(board_io_audio_enable, 0);
    mcu_io_set_dir(board_io_audio_enable, MCU_IO_DIR_OUT);

	screen_device_config_t config = 
	{
		.hw = _eve_hw,
		.has_touch = true,
		.rotate = false,
		.type = EVE_DISPLAY_TYPE_SMM
	};

	FUNCTION_RETURN ret = screen_device_init(&board_screen_device, &config);
	if(ret == FUNCTION_RETURN_OK)
	{
		DBG_INFO("Screen device initialized\n");

#if KERNEL_USES_LVGL || KERNEL_USES_SLINT
		ret = eve_lcd_create(&board_screen_device, &board_lcd);

		if(ret == FUNCTION_RETURN_OK)
		{
			DBG_INFO("eve_lcd_create OK\n");
    		// mcu_pwm_t backlight;
		}
		else
		{
			DBG_ERROR("eve_lcd_create failed\n");
		}
#endif
		
	}
	else
	{
		DBG_ERROR("Screen device not initialized\n");
	}

	if(board_screen_device.eve.status == EVE_STATUS_OK || board_screen_device.eve.status == EVE_STATUS_READING_EDID_FAILED)
	{
		ret = eve_init_flash(&board_screen_device.eve);
		DBG_INFO("eve_init_flash -> %d\n", ret);
	}

	// Enable Interrupts
	mcu_enable_interrupt();
}

void board_set_backlight(float pwm)
{
	if(board_screen_device.eve.status == EVE_STATUS_OK)
	{	
    	screen_device_set_dimming(&board_screen_device, (uint8_t) pwm);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Function
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif