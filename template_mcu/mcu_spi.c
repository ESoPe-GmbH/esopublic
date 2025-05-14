// Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_spi.c
 * 	@copyright Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_NEW_NAME && MCU_PERIPHERY_DEVICE_COUNT_SPI > 0

#include "mcu_internal.h"
#include "module/util/assert.h"
#include <string.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define MAX_TRANSACTIONS			50
				
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static uint8_t _initialized_spi_handlers = 0;
struct mcu_spi_interface_s mcu_spi_interface_handler[MCU_SPI_TOTAL_COUNT] = {0};
struct mcu_spi_s mcu_spi_handler[MCU_PERIPHERY_DEVICE_COUNT_SPI] = {0};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

mcu_spi_t mcu_spi_init(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx, MCU_IO_PIN clk, MCU_IO_PIN cs)
{
	return mcu_spi_init_quad(num, tx, rx, clk, cs, PIN_NONE, PIN_NONE);
}

mcu_spi_t mcu_spi_init_quad(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx, MCU_IO_PIN clk, MCU_IO_PIN cs, MCU_IO_PIN io2, MCU_IO_PIN io3)
{	
	struct mcu_spi_interface_s* handle;
	mcu_spi_t h_spi = NULL;

	if(_initialized_spi_handlers >= MCU_PERIPHERY_DEVICE_COUNT_SPI)
	{
		MCU_RETURN_ERROR(MCU_ERROR_SPI_NOT_AVAILABLE, NULL)
	}

	// Find a free spi handler
	for(int i = 0; i < MCU_PERIPHERY_DEVICE_COUNT_SPI && h_spi == NULL; i++)
	{
		if(!mcu_spi_handler[i].initialized)
		{
			h_spi = &mcu_spi_handler[i];
		}
	}

	// No free spi handler found. Return error
	if(h_spi == NULL)
	{
		MCU_RETURN_ERROR(MCU_ERROR_SPI_NOT_AVAILABLE, NULL)
	}

	if(num >= MCU_SPI_TOTAL_COUNT)
	{
		MCU_RETURN_ERROR(MCU_ERROR_SPI_NOT_AVAILABLE, NULL)
	}

	handle = &mcu_spi_interface_handler[num];

	// If the interface was already initialized -> check if uses the same pins as the one that should be used
	if(handle->initialized)
	{
		if(!(tx == handle->bus.mosi_io_num && rx == handle->bus.miso_io_num && clk == handle->bus.sclk_io_num))
		{
			MCU_RETURN_ERROR(MCU_ERROR_SPI_INVALID, NULL);
		}
	}
	else
	{
		handle->num = num;

		handle->initialized = true;

		// TODO: Initialize spi bus
	}

	h_spi->initialized = true;
	h_spi->spi = handle;
	h_spi->cs = cs;

	mcu_io_set_dir(cs, MCU_IO_DIR_OUT);
	mcu_io_set(cs, 1);
	
	_initialized_spi_handlers++;
	return (mcu_spi_t)h_spi;
}

void mcu_spi_free(mcu_spi_t h)
{
	if(h == NULL)
	{
		return;
	}

	// Check if bus is shared by multiple spi handlers
	bool bus_used_by_other = false;
	for(int i = 0; i < MCU_PERIPHERY_DEVICE_COUNT_SPI && !bus_used_by_other; i++)
	{
		if(h == &mcu_spi_handler[i] || !mcu_spi_handler[i].initialized)
		{
			continue;
		}
		if(h->spi == mcu_spi_handler[i].spi)
		{
			bus_used_by_other = true;
		}
	}

	// TODO: Stop and free the interface

	if(!bus_used_by_other)
	{
		// TODO: Stop and free the bus
	}
	
	// Clear the handle
	memset(h, 0, sizeof(struct mcu_spi_s));
	// Decrement number of used spi handlers.
	_initialized_spi_handlers--;
}

void* mcu_spi_get_device_handle(mcu_spi_t h)
{
	if(h == NULL)
		return NULL;

	return h->dev;
}

void mcu_spi_set_device_handle(mcu_spi_t h, void* dev)
{
	if(h == NULL)
		return;

	h->dev = dev;
}

uint8_t mcu_spi_get_num(mcu_spi_t h)
{
	if(h == NULL)
		return 0;

	return h->spi->num;
}

MCU_IO_PIN mcu_spi_get_cs(mcu_spi_t h)
{
	if(h == NULL)
		return 0;

	return h->cs;
}

MCU_RESULT mcu_spi_set_param(mcu_spi_t h, MCU_SPI_MODE_T mode, uint32_t frq)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_SPI_INVALID, MCU_ERROR_SPI_INVALID);
	
	// TODO: Set the parameter

	return mcu_spi_set_clock(h, frq);
}

MCU_RESULT mcu_spi_set_clock(mcu_spi_t h, uint32_t frq)
{
	if(h==NULL)			
		MCU_RETURN_ERROR(MCU_ERROR_SPI_INVALID, MCU_ERROR_SPI_INVALID);

	// TODO: Set the clock

	return MCU_OK;
}

uint32_t mcu_spi_get_frq(mcu_spi_t h)
{
	if(h == NULL)
		return 0;

	return 0; // TODO: Return the clock
}

uint8_t mcu_spi_send_with_cs(mcu_spi_t h, uint8_t letter)
{
	mcu_spi_set_chip_select(h, 0);
	mcu_spi_send(h, letter);
	mcu_spi_set_chip_select(h, 1);
	return letter;
}

uint8_t mcu_spi_send(mcu_spi_t h, uint8_t letter)
{
	uint8_t spi_read = 0;

	if(h==NULL)
		return 0;
		
	// TODO: Send and receive a single byte

//	comm_printf(COMM_DEBUG, "TXRX[%08x / %08x] %02x / %02x\n", h, h->spi, letter, spi_read);

	return spi_read;
}

void mcu_spi_send_buffer(mcu_spi_t h, uint8_t* w_buf, uint8_t* r_buf, uint32_t len)
{
	// TODO: Send and receive a complete buffer
}

void mcu_spi_set_chip_select(mcu_spi_t h, uint8_t state)
{
	if(h==NULL) return;
//	comm_printf(COMM_DEBUG, "CS[%08x / %08x] %02x\n", h, h->spi, state);
	mcu_io_set(h->cs, state);
}

FUNCTION_RETURN_T mcu_spi_transaction_start(mcu_spi_t h)
{
	ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
	
	mcu_io_set(h->cs, 0); // TODO: Remove if controlled by mcu
	
	// TODO: Begin a transaction on the spi bus, it max be executed after adding a transaction

	return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T mcu_spi_transaction_end(mcu_spi_t h)
{
	ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

	// TODO: Finish all transactions on the spi bus, so the user can afterwards access the receive buffer

	mcu_io_set(h->cs, 1);
	
	return ret;
}

FUNCTION_RETURN_T mcu_spi_transaction_add(mcu_spi_t h, mcu_spi_transaction_t t)
{
	ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
	ASSERT_RET(t.addr_length > 0 || t.cmd_length > 0 || t.dummy_length > 0 || t.r_buf_length > 0 || t.w_buf_length > 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Length cannot be 0.\n");

	// Ensure we don't have too many transactions active.
	ASSERT_RET(h->transaction_count < MAX_TRANSACTIONS, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Too many transactions\n");	

	// TODO: Add a transaction, best on DMA, User expects to read data after mcu_spi_transaction_end

	h->transaction_count++;
	//Should have had no issues.
	return FUNCTION_RETURN_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif	// MCU_TYPE == MCU_NEW_NAME && MCU_PERIPHERY_DEVICE_COUNT_SPI > 0