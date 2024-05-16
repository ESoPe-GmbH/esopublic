/***
 * @file ws2812.c
 * @copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "ws2812.h"

#if MODULE_ENABLE_LED_WS2812 && MCU_TYPE == MCU_ESP32

#include "mcu/sys.h"

#include "module/comm/dbg.h"
#include "module/util/color.h"
#include "driver/spi_master.h"
#include "esp32/rom/gpio.h"

#include <string.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#ifndef WS2812_USE_DOUBLE_BUFFER
#define WS2812_USE_DOUBLE_BUFFER    1
#endif

/// Number of SPI bits to transmit a single pixel color bit is (PIXEL_HIGH_BITS + PIXEL_LOW_BITS).
/// We have a ration of PIXEL_HIGH_BITS at 1 and PIXEL_LOW_BITS at 0 in the spi to transmit a single 1 bit.
/// We have a ration of PIXEL_LOW_BITS at 1 and PIXEL_HIGH_BITS at 0 in the spi to transmit a single 0 bit.
#define PIXEL_HIGH_BITS		7
/// Number of SPI bits to transmit a single pixel color bit is (PIXEL_HIGH_BITS + PIXEL_LOW_BITS).
/// We have a ration of PIXEL_HIGH_BITS at 1 and PIXEL_LOW_BITS at 0 in the spi to transmit a single 1 bit.
/// We have a ration of PIXEL_LOW_BITS at 1 and PIXEL_HIGH_BITS at 0 in the spi to transmit a single 0 bit.
#define PIXEL_LOW_BITS		3
/// Number of spi bits to transmit via SPI for a single rgb pixel.
#define BIT_PER_LED			((PIXEL_HIGH_BITS + PIXEL_LOW_BITS) * 24)
/// Number of bytes to transmit via SPI for a single rgb pixel.
#define BYTE_PER_LED        (BIT_PER_LED / 8)
/// 
/// Number of bytes to add to the buffer for performing a reset.
/// WS2812 has a 50µs pause in the protocol to reset the protocol to the first LED.
/// This can be set to a corresponding value based on the SPI frequency.
/// If controller is permanently adding pixels, this need to be greater 0. If controller sends pixels only every few milliseconds, this can be left on 0.
/// On 8MHz SPI frequency this can be set to 50 (8MHz = 8 Bit per microsecond = 1 Byte per microsecond => 50 byte for 50 microseconds)
#define LED_RESET_US		0

#if !((BIT_PER_LED % 8) == 0)
#error "The bit per LED need to be a multiple of 8 for the protocol to work."
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Buffer structure for SPI buffered data.
typedef struct led_buffer_s
{
	/// @brief Pointer to the DMA buffer that is filled with the SPI data.
	uint8_t* buffer;
	/// @brief Current index of the buffer where the next pixel should be written to.
	uint32_t buffer_index;
}led_buffer_t;

struct ws2812_s
{
	/// SPI bus configuration for ESP
	spi_bus_config_t bus;
	/// Bus select pin of this spi handler
	spi_device_interface_config_t businterface;
	/// Created device handle for the SPI using the config.
	spi_device_handle_t dev;
#if WS2812_USE_DOUBLE_BUFFER
    /// LED SPI structures to use as buffer. We send one via SPI and can fill the other one in the mean time with new pixel information.
    led_buffer_t led_buffer[2];
#else
    /// LED SPI structure to use as buffer.
    led_buffer_t led_buffer[1];
#endif
    /// @brief Current led_buffer index that is filled with pixels and will be sent next.
    uint32_t led_buffer_index;
	/// @brief Number of LEDs on the WS2812.
	uint32_t num_led;
}; 

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Transmit a single color of a pixel to the WS2812 LEDs.
 * 
 * @param ws2812 		Handle for the RGB LED as created by `ws2812_create`.
 * @param v 			Value for a single color of a pixel.
 * @param ptr 			Pointer to the buffer where to put the WS2812 protocol info to.
 * @param bytecount 	Pointer to the offset of bytes from ptr position.
 * @param bitcount 		Pointer to the offset of bits from current byte.
 */
static void _add_single_pixel(ws2812_handle_t ws2812, uint8_t v, uint8_t* ptr, uint8_t* bytecount, uint8_t* bitcount);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

ws2812_handle_t ws2812_create(const ws2812_hw_config_t* config)
{
    ws2812_handle_t ws2812 = mcu_heap_calloc(1, sizeof(struct ws2812_s));

	ws2812->bus.miso_io_num = -1;
	ws2812->bus.mosi_io_num = config->dout;
	ws2812->bus.sclk_io_num = -1;
	ws2812->bus.quadwp_io_num = -1;
	ws2812->bus.quadhd_io_num = -1;    

	ws2812->businterface.spics_io_num = -1;
	// Queue size is 1 to ensure we only have a single SPI transaction ongoing. Even if we use 2 buffer, when the second one is send the first must be idle. This is achieved with queue size 1!
	ws2812->businterface.queue_size = 1;
	ws2812->businterface.mode = 0;
	ws2812->businterface.clock_speed_hz = SPI_MASTER_FREQ_8M;
    
	// 50 for 50us reset pulse
	ws2812->led_buffer[0].buffer = heap_caps_calloc(BYTE_PER_LED + LED_RESET_US, config->num_led, MALLOC_CAP_DMA);
	ws2812->led_buffer[0].buffer_index = LED_RESET_US;
#if WS2812_USE_DOUBLE_BUFFER
	ws2812->led_buffer[1].buffer = heap_caps_calloc(BYTE_PER_LED + LED_RESET_US, config->num_led, MALLOC_CAP_DMA);
	ws2812->led_buffer[1].buffer_index = LED_RESET_US;
#endif

	ws2812->num_led = config->num_led;

    return ws2812;
}

void ws2812_free(ws2812_handle_t ws2812)
{
    DBG_ASSERT(ws2812, NO_ACTION, NO_RETURN, "ws2812 cannot be NULL\n");

    if(ws2812->dev)
    {
        spi_bus_remove_device(ws2812->dev);
        ws2812->dev = NULL;
        spi_bus_free(2);
    }

    mcu_heap_free(ws2812->led_buffer[0].buffer);
#if WS2812_USE_DOUBLE_BUFFER
    mcu_heap_free(ws2812->led_buffer[1].buffer);
#endif
    mcu_heap_free(ws2812);
}

FUNCTION_RETURN_T ws2812_init(ws2812_handle_t ws2812)
{
	esp_err_t ret;
    DBG_ASSERT(ws2812, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "ws2812 cannot be NULL\n");

	//Initialize the SPI bus
	ret=spi_bus_initialize(2, &ws2812->bus, SPI_DMA_CH_AUTO);
    DBG_ASSERT(ret == ESP_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "spi_bus_initialize failed\n");

    //Attach the interface to the SPI bus
    ret=spi_bus_add_device(2, &ws2812->businterface, &ws2812->dev);
    DBG_ASSERT(ret == ESP_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "spi_bus_add_device failed\n");

    return FUNCTION_RETURN_OK;    
}

void ws2812_reset_pixel(ws2812_t* ws2812)
{
    DBG_ASSERT(ws2812, NO_ACTION, NO_RETURN, "ws2812 cannot be NULL\n");
#if !WS2812_USE_DOUBLE_BUFFER
    // TODO: Ensure spi operation is not busy!
#endif
	ws2812->led_buffer[ws2812->led_buffer_index].buffer_index = LED_RESET_US;
}

void ws2812_add_pixel(ws2812_t* ws2812, color_rgb_t color)
{
    DBG_ASSERT(ws2812, NO_ACTION, NO_RETURN, "ws2812 cannot be NULL\n");
    // Ensure whole LED pixel fits into the buffer!
    DBG_ASSERT(ws2812->led_buffer[ws2812->led_buffer_index].buffer_index <= (BYTE_PER_LED + LED_RESET_US) * (ws2812->num_led - 1), NO_ACTION, NO_RETURN, "Failed to add pixel\n");
#if !WS2812_USE_DOUBLE_BUFFER
    // TODO: Ensure spi operation is not busy!
#endif

	uint8_t* ptr = &ws2812->led_buffer[ws2812->led_buffer_index].buffer[ws2812->led_buffer[ws2812->led_buffer_index].buffer_index];
	uint8_t bytecount = 0;
	uint8_t bitcount = 7;

	memset(ptr, 0, BYTE_PER_LED);

	_add_single_pixel(ws2812, color.g, ptr, &bytecount, &bitcount);
	_add_single_pixel(ws2812, color.r, ptr, &bytecount, &bitcount);
	_add_single_pixel(ws2812, color.b, ptr, &bytecount, &bitcount);

    ws2812->led_buffer[ws2812->led_buffer_index].buffer_index += bytecount;
}

void ws2812_add_pixels(ws2812_t* ws2812, color_rgb_t rgb, uint32_t num)
{
    DBG_ASSERT(ws2812, NO_ACTION, NO_RETURN, "ws2812 cannot be NULL\n");
    DBG_ASSERT(num > 0, NO_ACTION, NO_RETURN, "Cannot add 0 pixel\n");
    DBG_ASSERT(num <= ws2812->num_led, NO_ACTION, NO_RETURN, "Too many pixel to add for LED\n");
    // Ensure whole LED pixel fits into the buffer!
    DBG_ASSERT(ws2812->led_buffer[ws2812->led_buffer_index].buffer_index <= (BYTE_PER_LED + LED_RESET_US) * (ws2812->num_led - num), NO_ACTION, NO_RETURN, "Failed to add pixel\n");
#if !WS2812_USE_DOUBLE_BUFFER
    // TODO: Ensure spi operation is not busy!
#endif

    for(uint32_t i = 0; i < num; i++)
    {
        uint8_t* ptr = &ws2812->led_buffer[ws2812->led_buffer_index].buffer[ws2812->led_buffer[ws2812->led_buffer_index].buffer_index];
        uint8_t bytecount = 0;
        uint8_t bitcount = 7;

        memset(ptr, 0, BYTE_PER_LED);

        _add_single_pixel(ws2812, rgb.g, ptr, &bytecount, &bitcount);
        _add_single_pixel(ws2812, rgb.r, ptr, &bytecount, &bitcount);
        _add_single_pixel(ws2812, rgb.b, ptr, &bytecount, &bitcount);
        
        ws2812->led_buffer[ws2812->led_buffer_index].buffer_index += bytecount;
    }
}

void ws2812_show(ws2812_t* ws2812)
{
	esp_err_t ret;
    spi_transaction_t t = {0};

	if(ws2812->led_buffer[ws2812->led_buffer_index].buffer_index <= LED_RESET_US)
	{
		return;
	}
	// DBG_INFO("buffer_index = %d\n", ws2812->led_buffer[ws2812->led_buffer_index].buffer_index);

	// Number of bits to transmit  
    t.length = ws2812->led_buffer[ws2812->led_buffer_index].buffer_index * 8;		
	// Data to transmit
    t.tx_buffer = ws2812->led_buffer[ws2812->led_buffer_index].buffer;
	// Data to receive
	t.rx_buffer = NULL;	
	// Transmit!
	ret = spi_device_queue_trans(ws2812->dev, &t, portMAX_DELAY);
	//Should have had no issues.
	DBG_ASSERT(ret == ESP_OK, NO_ACTION, NO_RETURN, "Cannot start SPI\n");
#if WS2812_USE_DOUBLE_BUFFER
    // Switch LED buffer since current is sent via DMA
    ws2812->led_buffer_index ^= 1;
    // Reset other buffer
	ws2812->led_buffer[ws2812->led_buffer_index].buffer_index = LED_RESET_US;
#endif
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _add_single_pixel(ws2812_handle_t ws2812, uint8_t v, uint8_t* ptr, uint8_t* bytecount, uint8_t* bitcount)
{
	// Number of SPI Bits inside the buffer to transmit a single 1/0 bit for a pixel.
	const uint8_t bit_per_pixel_bit = BYTE_PER_LED / 3;

	for(uint8_t i = 0; i < 8; i++)
	{
		// We have a ration of PIXEL_HIGH_BITS at 1 and PIXEL_LOW_BITS at 0 in the spi to transmit a single 1 bit.
		// We have a ration of PIXEL_LOW_BITS at 1 and PIXEL_HIGH_BITS at 0 in the spi to transmit a single 0 bit.
		// We use the MSB first for each pixel bit and shift the next one into the MSB for checking.

		for(uint8_t j = 0; j < bit_per_pixel_bit; j++)
		{
			if((v & 0x80) > 0)
			{
				if(j < PIXEL_HIGH_BITS)
				{
					ptr[*bytecount] |= (1 << *bitcount);
				}
			}
			else
			{				
				if(j < PIXEL_LOW_BITS)
				{
					ptr[*bytecount] |= (1 << *bitcount);
				}
			}
			if(*bitcount == 0)
			{
				(*bytecount)++;
				(*bitcount) = 7;
			}
			else
				(*bitcount)--;
		}

		v <<= 1;
	}
}

#endif // MODULE_ENABLE_LED_WS2812
