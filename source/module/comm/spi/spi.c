/***
 * @file spi.c
 * @copyright Urheberrecht 2021-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "spi.h"
#include "module/comm/dbg.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t _transceive_byte(spi_hw_t* hw, uint8_t send);
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void spi_init(spi_hw_t* hw)
{
	DBG_ASSERT(hw, NO_ACTION, NO_RETURN, "hw cannot be NULL\n");

	if(hw->device == NULL)
	{
		// No device poiner -> Use bitbanging

		mcu_io_set_dir(hw->cs, MCU_IO_DIR_OUT);
		mcu_io_set_dir(hw->clk, MCU_IO_DIR_OUT);
		mcu_io_set_dir(hw->mosi, MCU_IO_DIR_OUT);
		mcu_io_set_dir(hw->miso, MCU_IO_DIR_IN);
		mcu_io_set_pullup(hw->miso, true);

		mcu_io_set(hw->cs, MCU_IO_HIGH);
		mcu_io_set(hw->clk, MCU_IO_LOW);
	}
	else
	{
		// Device pointer -> Use mcu_spi
#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0
		mcu_spi_set_param(hw->device, hw->mode, hw->frq);
#endif
	}
}

FUNCTION_RETURN spi_set_param(spi_hw_t* hw, MCU_SPI_MODE_T mode, uint32_t frq)
{
	DBG_ASSERT(hw, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "hw cannot be NULL\n");

	hw->mode = mode;
	hw->frq = frq;

	if(hw->device == NULL)
	{
		// No device poiner -> Use bitbanging
	}
	else
	{
		// Device pointer -> Use mcu_spi
#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0
		if(MCU_OK != mcu_spi_set_param(hw->device, hw->mode, hw->frq))
			return FUNCTION_RETURN_EXECUTION_ERROR;
#else
		return FUNCTION_RETURN_UNSUPPORTED;
#endif
	}
	return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN spi_set_clock(spi_hw_t* hw, uint32_t frq)
{
	DBG_ASSERT(hw, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "hw cannot be NULL\n");
	
	hw->frq = frq;

	if(hw->device == NULL)
	{
		// No device poiner -> Use bitbanging
	}
	else
	{
#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0
		// Device pointer -> Use mcu_spi
		if(MCU_OK != mcu_spi_set_clock(hw->device, hw->frq))
			return FUNCTION_RETURN_EXECUTION_ERROR;
#else
		return FUNCTION_RETURN_UNSUPPORTED;
#endif
	}
	return FUNCTION_RETURN_OK;
}

void spi_set_chip_select(spi_hw_t* hw, uint8_t state)
{
	DBG_ASSERT(hw, NO_ACTION, NO_RETURN, "hw cannot be NULL\n");

	if(hw->device == NULL)
	{
		// No device poiner -> Use bitbanging
		mcu_io_set(hw->cs, state);
	}
	else
	{
#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0
		// Device pointer -> Use mcu_spi
		mcu_spi_set_chip_select(hw->device, state);
#endif
	}
}

void spi_tx_rx(spi_hw_t* hw, const uint8_t* tx_buf, uint8_t* rx_buf, uint16_t len)
{
	DBG_ASSERT(hw, NO_ACTION, NO_RETURN, "hw cannot be NULL\n");

	uint8_t recv, send;

	if(!hw->chip_select_external)
		mcu_io_set(hw->cs, MCU_IO_LOW);

	if(hw->device == NULL)
	{
		// No device poiner -> Use bitbanging

		for(uint16_t i = 0; i < len; i++)
		{
			send = 0xFF;
			if(tx_buf != NULL)
			{
				send = tx_buf[i];
			}
			recv = _transceive_byte(hw, send);

			if(rx_buf != NULL)
			{
				rx_buf[i] = recv;
			}
		}
	}
	else
	{
#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0
		// Device pointer -> Use mcu_spi
		mcu_spi_send_buffer(hw->device, (uint8_t*)tx_buf, rx_buf, len);
#endif
	}

	if(!hw->chip_select_external)
		mcu_io_set(hw->cs, MCU_IO_HIGH);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t _transceive_byte(spi_hw_t* hw, uint8_t send)
{
	uint8_t receive = 0;

	for(uint8_t bit = 0x80; bit; bit >>= 1)
	{
		mcu_io_set(hw->clk, MCU_IO_HIGH);
		mcu_io_set(hw->mosi, (send & bit) ? MCU_IO_HIGH : MCU_IO_LOW);
		
		mcu_io_set(hw->clk, MCU_IO_LOW);

		if(mcu_io_get(hw->miso) == MCU_IO_HIGH)
			receive |= bit;
	}

	return receive;
}
