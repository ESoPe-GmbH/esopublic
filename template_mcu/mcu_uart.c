/**
 * 	@file 		mcu_uart.c
 * 	@copyright Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_NEW_NAME && MCU_PERIPHERY_DEVICE_COUNT_UART > 0

#include "mcu_internal.h"
#include "module/fifo/fifo.h"
#include <string.h>

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
 * @brief Apply default configuration to values that are not configured.
 * 
 * @param config Pointer to the config to modify.
 */
static void _apply_default_uart_config(mcu_uart_config_t* config);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_ENABLE_COMM_MODE_UART
	bool mcu_uart_interface_is_created = false;
	comm_interface_t mcu_uart_comm_interface;
#endif

uint8_t mcu_current_uart_handler = 0;
struct mcu_uart_s mcu_uart_handler[MCU_PERIPHERY_DEVICE_COUNT_UART] = {0};
static struct mcu_uart_s* _uart_handler_hash[MCU_UART_MAX_NUM + 1] = {0};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_ENABLE_COMM_MODE_UART
void mcu_uart_create_comm_handler(mcu_uart_t h, comm_t *ch)
{
	if(ch==NULL)	return;
	if(!mcu_uart_interface_is_created)
	{
		comm_init_interface(&mcu_uart_comm_interface);
		mcu_uart_comm_interface.xputc = (comm_putc_t)mcu_uart_putc;
		mcu_uart_comm_interface.xgetc = (comm_getc_t)mcu_uart_getc;
		mcu_uart_comm_interface.xputs = (comm_puts_t)mcu_uart_puts;
		mcu_uart_comm_interface.xgets = (comm_gets_t)mcu_uart_gets;
		mcu_uart_comm_interface.data_present = (comm_available_t)mcu_uart_available;
		mcu_uart_comm_interface.transmit_ready = (comm_transmit_ready_t)mcu_uart_transmit_ready;
		mcu_uart_interface_is_created = true;
	}
	ch->device_handler = h;
	ch->interface = &mcu_uart_comm_interface;
}
#endif

mcu_uart_t mcu_uart_init(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx)
{
	mcu_uart_t handle;

	// Clear if maximum number of uarts is reached.
	if(mcu_current_uart_handler >= MCU_PERIPHERY_DEVICE_COUNT_UART || num > MCU_UART_MAX_NUM)
		MCU_RETURN_ERROR(MCU_ERROR_UART_NOT_AVAILABLE, NULL); 

	handle = &mcu_uart_handler[mcu_current_uart_handler];
	memset(handle, 0, sizeof(struct mcu_uart_s));

	_apply_default_uart_config(&handle->config);
 
	handle->hw.unit = num;		
	handle->hw.io_tx = tx;
	handle->hw.io_rx = rx;	
	handle->hw.io_rts = PIN_NONE;
	handle->hw.io_cts = PIN_NONE;
	handle->hw.transmit_buffer_size = 0;		
	handle->hw.receive_buffer_size = 8192;					
	
	// TODO: Initialize UART if needed, if not done in mcu_uart_set_config				

	mcu_uart_set_pin_gpio(handle, false);

	mcu_io_set_pullup(rx, true);
	mcu_uart_set_config(handle, &handle->config);

	_uart_handler_hash[num] = handle;

	mcu_current_uart_handler++;
	return (mcu_uart_t)handle;
}

mcu_uart_t mcu_uart_create(mcu_uart_hw_config_t* hw, mcu_uart_config_t* config)
{
	mcu_uart_t handle;

	// Clear if maximum number of uarts is reached.
	if(mcu_current_uart_handler >= MCU_PERIPHERY_DEVICE_COUNT_UART)
		MCU_RETURN_ERROR(MCU_ERROR_UART_NOT_AVAILABLE, NULL); 

	handle = &mcu_uart_handler[mcu_current_uart_handler];
	memset(handle, 0, sizeof(struct mcu_uart_s));
	
	_apply_default_uart_config(&handle->config);
	
	memcpy(&handle->hw, hw, sizeof(mcu_uart_hw_config_t));
	memcpy(&handle->config, config, sizeof(mcu_uart_config_t));				
	
	// TODO: Initialize UART if needed, if not done in mcu_uart_set_config

	mcu_uart_set_pin_gpio(handle, false);

	mcu_io_set_pullup(handle->hw.io_rx, true);
	mcu_uart_set_config(handle, &handle->config);

	mcu_current_uart_handler++;
	return (mcu_uart_t)handle;	
}

void mcu_uart_free(mcu_uart_t h)
{
	// Make uart pins unusable
	mcu_uart_set_pin_gpio(h, true);
	// TODO: Stop and free UART
	memset(h, 0, sizeof(struct mcu_uart_s));
}

const mcu_uart_config_t* mcu_uart_get_config(mcu_uart_t h)
{
	if(h == NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);

	return &h->config;
}

MCU_RESULT mcu_uart_set_config(mcu_uart_t handle, mcu_uart_config_t* config)
{
	if(handle == NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);
	if(config == NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	if(config != &handle->config)
	{
		memcpy(&handle->config, config, sizeof(mcu_uart_config_t));
		_apply_default_uart_config(&handle->config);
	}

	// TODO: Apply UART config

	return MCU_OK;
}

MCU_RESULT mcu_uart_enable_cts(mcu_uart_t h, MCU_IO_PIN io_cts)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	mcu_io_set_dir(io_cts, MCU_IO_DIR_IN);
	mcu_io_set_pullup(io_cts, true);

	// TODO: Set CTS function or clear CTS function
	
	h->hw.io_cts = io_cts;

	return MCU_OK;
}

MCU_RESULT mcu_uart_enable_rts(mcu_uart_t h, MCU_IO_PIN io_rts)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	// TODO: Set RTS function or clear RTS function

	h->hw.io_rts = io_rts;

	return MCU_OK;
}

MCU_RESULT mcu_uart_set_param(mcu_uart_t h, uint32_t baud, uint8_t databits, uint8_t parity, uint8_t stopbits)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);
	

//	DBG_INFO("UART %d Parameter: %d %d %c %d\n", h->num, baud, databits, parity, stopbits);
	h->config.baudrate = baud;
	h->config.databits = databits;
	h->config.parity = parity;
	h->config.stopbits = stopbits;

	// TODO: Apply new parameter

	return mcu_uart_set_baudrate(h, baud);
}

const uint32_t mcu_uart_divider[4] = {1, 4, 16, 64};

MCU_RESULT mcu_uart_set_baudrate(mcu_uart_t h, uint32_t baudrate)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	h->config.baudrate = baudrate;

	// TODO: Apply new baudrate

	return MCU_OK;
}

uint32_t mcu_uart_get_baudrate(mcu_uart_t h)
{
	if(h == NULL)		
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, 0);

	return h->config.baudrate;
}

void mcu_uart_set_pin_gpio(mcu_uart_t h, bool b)
{
	if(h==NULL)		return;

	if(b)
	{
		// TODO: Set UART Pins to GPIO mode so they can be controlled manually
	}
	else
	{
		// TODO: Set UART pins to peripheral mode
	}
}

MCU_RESULT mcu_uart_set_buffer(mcu_uart_t h, MCU_INT_LVL lvl, uint8_t *data, uint16_t len)
{
	// TODO: Implement buffer for receiving data
	return MCU_ERROR_NOT_SUPPORTED;
}

MCU_RESULT mcu_uart_set_alternate_receive(mcu_uart_t h, MCU_INT_LVL lvl, void (*f)(void*, int), void* obj)
{
	// TODO: Set callback for a receive of each byte
	return MCU_ERROR_NOT_SUPPORTED;
}

MCU_RESULT mcu_uart_set_receive_event(mcu_uart_t h, void (*f)(void*), void* obj)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	h->receive_notify = f;
	h->obj_receive_notify = obj;

	return MCU_OK;
}

MCU_RESULT mcu_uart_set_direct_transmit_interrupt(mcu_uart_t h)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	h->config.use_direct_transmit_interrupt = true;

	// TODO: Enable transmit interrupt

	return MCU_OK;
}

bool mcu_uart_transmit_ready(mcu_uart_t h)
{
	if(h==NULL)
		return false;

	if(!h->config.use_direct_transmit_interrupt)
		return true;
	
	// TODO: Check if data can be send
	return true;
}

void mcu_uart_putc(mcu_uart_t h, int data)
{
	if(h==NULL)
		return;

	// TODO: Write byte on UART
}

void mcu_uart_puts(mcu_uart_t h, uint8_t *data, uint16_t len)
{
	if(h == NULL || len == 0)
		return;

	// TODO: Read byte from UART
}

int mcu_uart_available(mcu_uart_t h)
{
	size_t len;
	if(h==NULL)
		return 0;

	// TODO: Check number of received bytes

	return 0;
}

int mcu_uart_getc(mcu_uart_t h)
{
	uint8_t rcv = 0;

	if(h==NULL)
		return 0;

	// TODO: Read a single byte

	return rcv;
}

int mcu_uart_gets(mcu_uart_t h, uint8_t* buf, uint16_t len)
{
	int cnt = 0;

	if(h==NULL)
		return 0;

	// TODO: Read multiple bytes to fill the buffer
	
	return cnt;
}

void mcu_uart_clear_rx(mcu_uart_t h)
{
	if(h==NULL)
		return;

	// TODO: Clear receive buffer
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _apply_default_uart_config(mcu_uart_config_t* config)
{
	if(config->baudrate == 0)
		config->baudrate = 500000;
	if(config->parity == 0)
		config->parity = 'N';
	if(config->databits == 0)
		config->databits = 8;
	if(config->stopbits == 0)
		config->stopbits = 1;
	if(config->percent_rts_set == 0)
		config->percent_rts_set = 90;
	if(config->percent_rts_clear == 0)
		config->percent_rts_clear = 10;
}

#endif // MCU_PERIPHERY_ENABLE_UART
