/**
 * 	@file 	board.h
 * 	@copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten
 *
 * 	@author Tim Koczwara
 *
 *  @brief	Offers one function, that initializes all periphery that is assigned to the board inside board.c and board_specific.h.
 *
 *  @version  	1.02 (16.02.2013)
 *		- Converted comments in english.
 *  @version  	1.01 (03.04.2011)
 *		- board.h splitted to board_specific.h and board.h.
 *		- board.h contains only the board_init function.
 *  @version  	1.00
 *  	- Initial release
 *
 ******************************************************************************/
#ifndef BOARD_HEADER_FIRST_INCLUDE_GUARD
#define BOARD_HEADER_FIRST_INCLUDE_GUARD


#include "module_public.h"
#include "module_include_public.h"
#include "module/comm/i2c/i2c.h"
#include "module/display/sld/display_sld.h"
#include "sdkconfig.h"
#if CONFIG_IDF_TARGET_ESP32S3 && CONFIG_SLD_C_W_S3_BT817
#include "module/util/pmod.h"
#include "module/gui/eve/eve.h"
#include "module/gui/eve_ui/screen.h"
#endif

/// @brief Handle for SLD connection. Contains the display, the touch and the pwm handle (Backlight).
extern display_sld_handle_t board_lcd;
/// @brief Handle for the uart that can be used for external communication.
extern mcu_uart_t board_uart_peripheral;
/// @brief Comm handle for the peripheral communication, using board_uart_peripheral.
extern comm_t board_comm_peripheral;

// BT817 pcb has more I/O for external usage.
#if CONFIG_IDF_TARGET_ESP32S3 && CONFIG_SLD_C_W_S3_BT817
/// @brief Handle for the eve screen device to use the eve functions.
extern screen_device_t board_screen_device;
/// @brief I/O for enabling the audio amplifier.
extern MCU_IO_PIN board_io_audio_enable;
/// @brief PMOD interface for the SPI interface 1.
extern pmod_t board_pmod_spi_1;
/// @brief PMOD interface for the SPI interface 2.
extern pmod_t board_pmod_spi_2;
/// @brief PMOD interface for the SPI interface 3. This is connected to the bt817 chip.
extern pmod_t board_pmod_spi_3;
/// @brief PMOD interface for the I2C interface. This is connected to the touch controller.
extern pmod_t board_pmod_i2c;
/// @brief PMOD interface for the UART interface. This is connected to the external UART. This is also used as @c board_uart_peripheral.
extern pmod_t board_pmod_uart;
#endif

/**
 * @brief 		Initializes the mcu and all periphery assigned to the used board. Is automatically called in sys.c at first step of the main.
 */
void board_init(void);
/**
 * @brief       Sets the backlight brightness in percent.
 * 
 * @param pwm   Valid values 0.0 to 100.0 for the brightness of the backlight.
 */
void board_set_backlight(float pwm);

#endif
