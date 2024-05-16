/**
 * 	@file 	ws2812.h
 * 	@copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			The ws2812 module controls RGB LEDs with a specific protocol.
 *          Since this protocol is extremely time sensitive, we use source files for the implementations on different processors.
 *          In the internet most implementations are done using assembler. But a less CPU heavy approach is by using a SPI peripheral of the MCU.
 *          If it is set to DMA and only the MOSI pin is used for this peripheral, we can use it for the data transmission. Since this operation needs bigger buffer,
 *          this is not good for small microcontroller.
 * 
 *          The protocol is daisy chained. You send multiple LEDs at once. Each LED will consume one color value and sends the other color values to the next LED.
 *          
 *          The protocol has 24-bit per LED. Each color for the LED has 8-Bit. Each bit is transmitted in 1.25 microseconds.
 *          When the pin is 0.9us high and 0.35us low, the bit inside the color is 1.
 *          When the pin is 0.35us high and 0.9us low, the bit inside the color is 0.
 *           
 *          In our approach using SPI will be set to a frequency of 8MHz. This will be one tick every 125ns, so we are factor 10 faster than each pixel bit.
 *          The result is we have 10 bits to transmit on MOSI to send one bit. Therefore we set 7x1 to 3x0 for a 1 or 3x1 and 7x0 for a 0.
 *          By using the 7 to 3 ratio we have 0.875us and 0.375us as timing which is inside the tolerance for most LED.
 * 
 *          As a result we need to have a buffer with 240 bit (30 byte) per LED. For a neopixel ring using 24 LED, we will use a 720 byte buffer to set all LED to the new color.
 *          Also it takes 720 microseconds to send a single color to the RGB.
 *          Using the double buffer option, we will have the double buffer size.
 *
 *  @version	1.00 (19.01.2023)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef __MODULE_LED_RGB_H__
#define __MODULE_LED_RGB_H__

#include "module_public.h"
#if MODULE_ENABLE_LED_WS2812
#include "mcu/sys.h"
#include "module/enum/function_return.h"
#include "module/util/color.h"

//------------------------------------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------------------------------------
/// Version of the led module
#define LED_WS2812_STR_VERSION "1.00"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Configuration structure for ws2812_init.
typedef struct ws2812_hw_config_s
{
    /// Number of LEDs to use on ws2812.
    uint32_t num_led;
    /// Pin that is connected to the din of the RGB.
    MCU_IO_PIN dout;
}ws2812_hw_config_t;

/// Typedef for internal ws2812 structure.
typedef struct ws2812_s ws2812_t;
/// Handler for the internal ws2812 structure that is used as parameter for most functions.
typedef struct ws2812_s* ws2812_handle_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Create a handle and internal buffers for the ws2812.
 * To use it, you need to call ws2812_init with it.
 * 
 * @param config                Hardware configuration for the WS2812.
 * @return ws2812_handle_t      Handle that can be used in other ws2812_ functions. NULL if creation failed.
 */
ws2812_handle_t ws2812_create(const ws2812_hw_config_t* config);
/**
 * @brief Frees the handle, buffer and stops internal peripherals.
 * 
 * @param ws2812                Handle as created in `ws2812_create`.
 */
void ws2812_free(ws2812_handle_t ws2812);
/**
 * @brief Initialize the internal peripherals to use for the LED protocol.
 * 
 * @param ws2812                Handle as created in `ws2812_create`.
 * @return FUNCTION_RETURN_OK on success, other value on errors.
 */
FUNCTION_RETURN_T ws2812_init(ws2812_handle_t ws2812);
/**
 * @brief Reset the current buffer that stores the pixel information, so `ws2812_add_pixel` and `ws2812_add_pixels` will start from first LED.
 * Is automatically done at the end of `ws2812_show`.
 * 
 * @param ws2812                Handle as created in `ws2812_create`.
 */
void ws2812_reset_pixel(ws2812_t* ws2812);
/**
 * @brief Add a single pixel to the LED chain.
 * To make the pixels visible, you need to call `ws2812_show`.
 * 
 * @param ws2812                Handle as created in `ws2812_create`.
 * @param rgb                   Color for the LEDs.
 */
void ws2812_add_pixel(ws2812_t* ws2812, color_rgb_t rgb);
/**
 * @brief Add multiple pixels with the same color to the LED chain.
 * To make them visible, you need to call `ws2812_show`.
 * 
 * @param ws2812                Handle as created in `ws2812_create`.
 * @param rgb                   Color for the LEDs.
 * @param num                   Number of LEDs that need to have the color.
 */
void ws2812_add_pixels(ws2812_t* ws2812, color_rgb_t rgb, uint32_t num);
/**
 * @brief Sends the pixels added with `ws2812_add_pixel` and `ws2812_add_pixels` to the LEDs.
 * 
 * @param ws2812                Handle as created in `ws2812_create`.
 */
void ws2812_show(ws2812_t* ws2812);

#endif

#endif /* __MODULE_LED_RGB_H__ */
