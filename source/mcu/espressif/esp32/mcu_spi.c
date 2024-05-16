// Urheberrecht 2018-2021 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_spi.c
 * 	@copyright Urheberrecht 2018-2021 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_ESP32

#include "mcu_internal.h"
#include "module/util/assert.h"
#include <string.h>

#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0

#define MAX_TRANSACTIONS			50

static uint8_t _initialized_spi_handlers = 0;
struct mcu_spi_interface_s mcu_spi_interface_handler[MCU_SPI_TOTAL_COUNT] = {0};
struct mcu_spi_s mcu_spi_handler[MCU_PERIPHERY_DEVICE_COUNT_SPI] = {0};

mcu_spi_t mcu_spi_init(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx, MCU_IO_PIN clk, MCU_IO_PIN cs)
{	
	esp_err_t ret;	
	struct mcu_spi_interface_s* handle;
	mcu_spi_t h_spi = NULL;

	if(_initialized_spi_handlers >= MCU_PERIPHERY_DEVICE_COUNT_SPI)
		MCU_RETURN_ERROR(MCU_ERROR_SPI_NOT_AVAILABLE, NULL)

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
		MCU_RETURN_ERROR(MCU_ERROR_SPI_NOT_AVAILABLE, NULL)

	handle = &mcu_spi_interface_handler[num];

	// If the interface was already initialized -> check if uses the same pins as the one that should be used
	if(handle->initialized)
	{
		if(!(tx == handle->bus.mosi_io_num && rx == handle->bus.miso_io_num && clk == handle->bus.sclk_io_num))
			MCU_RETURN_ERROR(MCU_ERROR_SPI_INVALID, NULL);
	}
	else
	{
		handle->semaphore = xSemaphoreCreateMutex();

		handle->num = num;
		handle->bus.miso_io_num = rx;
		handle->bus.mosi_io_num = tx;
		handle->bus.sclk_io_num = clk;
		handle->bus.quadwp_io_num = -1;
		handle->bus.quadhd_io_num = -1;
		handle->bus.max_transfer_sz = 4092;

		handle->initialized = true;
		//Initialize the SPI bus
		ret=spi_bus_initialize(num, &handle->bus, SPI_DMA_CH_AUTO);
		ESP_ERROR_CHECK(ret);
	}

	h_spi->initialized = true;
	h_spi->spi = handle;
	h_spi->cs = cs;
	h_spi->businterface.spics_io_num = -1;
	h_spi->businterface.queue_size = MAX_TRANSACTIONS;
	h_spi->businterface.mode = 0;
	h_spi->businterface.clock_speed_hz = SPI_MASTER_FREQ_10M;

    //Attach the interface to the SPI bus
    ret=spi_bus_add_device(num, &h_spi->businterface, &h_spi->dev);
    ESP_ERROR_CHECK(ret);

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

	// Remove this device from the bus.
	spi_bus_remove_device(h->dev);
	// Clear chip select to be an GPIO. Since we do it manually, we need to free it ourself.
	gpio_reset_pin(h->cs);

	// If bus is only used by this SPI handler, clear the bus too.
	if(!bus_used_by_other)
	{
		// Free bus too...
		// This frees the GPIOs internally
		spi_bus_free(h->spi->num);

		vSemaphoreDelete(h->spi->semaphore);
		h->spi->semaphore = NULL;
		
		memset(h->spi, 0, sizeof(struct mcu_spi_interface_s));
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
	
	h->businterface.mode = mode;

	return mcu_spi_set_clock(h, frq);
}

MCU_RESULT mcu_spi_set_clock(mcu_spi_t h, uint32_t frq)
{
	if(h==NULL)			
		MCU_RETURN_ERROR(MCU_ERROR_SPI_INVALID, MCU_ERROR_SPI_INVALID);

	h->businterface.clock_speed_hz = frq;

	spi_bus_remove_device(h->dev);
	spi_bus_add_device(h->spi->num, &h->businterface, &h->dev);

	if(h->businterface.mode > 1)
	{
		// Send a dummy byte, otherwise, the first transmitted byte will have low clock phase
		mcu_spi_send(h, 0xFF);
	}

	return MCU_OK;
}

uint32_t mcu_spi_get_frq(mcu_spi_t h)
{
	if(h == NULL)
		return 0;

	return h->businterface.clock_speed_hz;
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
		
    esp_err_t ret;
    spi_transaction_t t = { 
		// Number of bits to transmit  
		.length = 8,
		.rxlength = 8,
		// Data to transmit
		.tx_buffer = &letter,
		// Data to receive
		.rx_buffer = &spi_read
	};

	// Transmit!
    ret=spi_device_polling_transmit(h->dev, &t);  
	//Should have had no issues.
    assert(ret==ESP_OK);

//	comm_printf(COMM_DEBUG, "TXRX[%08x / %08x] %02x / %02x\n", h, h->spi, letter, spi_read);

	return spi_read;
}

void mcu_spi_send_buffer(mcu_spi_t h, uint8_t* w_buf, uint8_t* r_buf, uint32_t len)
{
	esp_err_t ret;
    spi_transaction_t t = { 
		// Number of bits to transmit  
		.length = len * 8,
		.rxlength = len * 8,
		// Data to transmit
		.tx_buffer = w_buf,
		// Data to receive
		.rx_buffer = r_buf
	};
	
	// Transmit!
    ret=spi_device_polling_transmit(h->dev, &t);  
	//Should have had no issues.
    assert(ret==ESP_OK);
}

void mcu_spi_set_chip_select(mcu_spi_t h, uint8_t state)
{
	if(h==NULL) return;
//	comm_printf(COMM_DEBUG, "CS[%08x / %08x] %02x\n", h, h->spi, state);
	mcu_io_set(h->cs, state);
}

FUNCTION_RETURN_T mcu_spi_transaction_start(mcu_spi_t h)
{
	esp_err_t ret;
	ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
	
	xSemaphoreTake(h->spi->semaphore, portMAX_DELAY);

	ret = spi_device_acquire_bus(h->dev, portMAX_DELAY);
	mcu_io_set(h->cs, 0);
	h->transaction_count = 0;

	ASSERT_RET(ret == ESP_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Cannot acquire bus: %04x\n", ret);	

	return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T mcu_spi_transaction_end(mcu_spi_t h)
{
	esp_err_t ret = FUNCTION_RETURN_OK;
    spi_transaction_t *t;
	ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

	for(uint32_t i = 0; i < h->transaction_count; i++)
	{
		ret = spi_device_get_trans_result(h->dev, &t, portMAX_DELAY);
		if(ret != ESP_OK)
		{
			ret = FUNCTION_RETURN_EXECUTION_ERROR;
			DBG_ERROR("Invalid transaction result %04x\n", ret);
		}
		// ASSERT_RET(ret == ESP_OK, goto exit, FUNCTION_RETURN_EXECUTION_ERROR, "Invalid transaction result %04x\n", ret);
		// mcu_spi_transaction_t* t2 = t->user;
		// If rxflag is set we need to load the rx data.
		// if(t2->flags & MCU_SPI_TRANS_FLAGS_RXDATA)
		// {
		// 	DBG_INFO("Read %4A | %4A\n", t2->r_data, t->rx_data, t->rx_buffer);
		// 	if(t->rx_buffer != &t2->r_data[0])
		// 	{
		// 		memcpy(t2->r_data, t->rx_buffer, 4);
		// 	}
		// }
		// else
		// {
		// 	if(t->rx_buffer != t2->r_buf)
		// 	{
		// 		memcpy(t2->r_buf, t->rx_buffer, t2->r_buf_length);
		// 	}
		// }
		mcu_heap_free(t);
	}

	// spi_device_polling_end(h->dev, portMAX_DELAY);
	
	spi_device_release_bus(h->dev);
	mcu_io_set(h->cs, 1);
	
	xSemaphoreGive(h->spi->semaphore);

	return ret;
}

FUNCTION_RETURN_T mcu_spi_transaction_add(mcu_spi_t h, mcu_spi_transaction_t t)
{
	esp_err_t ret;

	ASSERT_RET_NOT_NULL(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
	// ASSERT_RET_NOT_NULL(t, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
	ASSERT_RET(t.addr_length > 0 || t.cmd_length > 0 || t.dummy_length > 0 || t.r_buf_length > 0 || t.w_buf_length > 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Length cannot be 0.\n");

	// Ensure we don't have too many transactions active.
	ASSERT_RET(h->transaction_count < MAX_TRANSACTIONS, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Too many transactions\n");	

    spi_transaction_ext_t* t_esp = mcu_heap_calloc(1, sizeof(spi_transaction_ext_t));

	ASSERT_RET_NOT_NULL(t_esp, NO_ACTION, FUNCTION_RETURN_INSUFFICIENT_MEMORY);
	
	// We control size of address, cmd and dummy here
	t_esp->base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_DUMMY;
	t_esp->base.cmd = t.cmd;
	t_esp->command_bits = t.cmd_length * 8;
	t_esp->base.addr = t.addr;
	t_esp->address_bits = t.addr_length * 8;
	t_esp->dummy_bits = t.dummy_length * 8;

	// If flag is set, copy w_data
	if(t.flags & MCU_SPI_TRANS_FLAGS_TXDATA)
	{
		// t_esp->base.flags |= SPI_TRANS_USE_TXDATA;
		// memcpy(t_esp->base.tx_data, t.w_data, 4);
		t_esp->base.tx_buffer = t.w_data;
	}
	else
	{		
		// Data to transmit
		t_esp->base.tx_buffer = t.w_buf;
	}
	// Number of bits to transmit  
	t_esp->base.length = t.w_buf_length * 8;	

	// If flag is set, copy r_data
	// if(!(t_esp->base.flags & SPI_TRANS_USE_RXDATA))
	{		
		// Data to receive
		t_esp->base.rx_buffer = t.r_buf;
	}
	// Number of bits to receive
	t_esp->base.rxlength = t.r_buf_length * 8;
	if(t_esp->base.length == 0)
		t_esp->base.length = t_esp->base.rxlength;

	t_esp->base.user = &t;
	// if(t.r_buf_length > 0)
	// {
	// 	DBG_INFO("rxlength=%u length=%u\n", )
	// }

	// Transmit!
	ret = spi_device_queue_trans(h->dev, &t_esp->base, portMAX_DELAY);
	ASSERT_RET(ret == ESP_OK, mcu_heap_free(t_esp), FUNCTION_RETURN_EXECUTION_ERROR, "Cannot queue transaction: ret=%04x wlen=%u rlen=%u\n", ret, t.w_buf_length, t.r_buf_length);	
	h->transaction_count++;
	//Should have had no issues.
	return FUNCTION_RETURN_OK;
}

#endif	// MCU_PERIPHERY_ENABLE_SPI

#endif // #if MCU_TYPE == R5F563
