/**
 * 	@file 		mcu_uart.c
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_ESP32

#include "../esp32/mcu_internal.h"
#include "module/fifo/fifo.h"
#include <string.h>
#include "hal/uart_ll.h"
#include <sdkconfig.h>
#include "driver/uart_select.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include "esp_rom_gpio.h"
#define gpio_pad_select_gpio	esp_rom_gpio_pad_select_gpio
#endif

#ifdef CONFIG_UART_ISR_IN_IRAM
#define UART_ISR_ATTR     IRAM_ATTR
#define UART_MALLOC_CAPS  (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#else
#define UART_ISR_ATTR
#define UART_MALLOC_CAPS  MALLOC_CAP_DEFAULT
#endif

#if MCU_PERIPHERY_DEVICE_COUNT_UART

/**
 * @brief Apply default configuration to values that are not configured.
 * 
 * @param config Pointer to the config to modify.
 */
static void _apply_default_uart_config(mcu_uart_config_t* config);

static void _uart_isr_event(uart_port_t uart_num, uart_select_notif_t uart_select_notif, BaseType_t *task_woken);

#if MCU_PERIPHERY_ENABLE_COMM_MODE_UART
	bool mcu_uart_interface_is_created = false;
	comm_interface_t mcu_uart_comm_interface;
#endif

uint8_t mcu_current_uart_handler = 0;
struct mcu_uart_s mcu_uart_handler[MCU_PERIPHERY_DEVICE_COUNT_UART] = {0};
static struct mcu_uart_s* _uart_handler_hash[MCU_UART_MAX_NUM + 1] = {0};

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

	mcu_uart_set_pin_gpio(handle, false);

	mcu_io_set_pullup(rx, true);
	mcu_uart_set_config(handle, &handle->config);

	uart_set_select_notif_callback(num, _uart_isr_event);

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
	uart_driver_delete(h->hw.unit);	
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

	uart_driver_delete(handle->hw.unit);	
	uart_config_t uart_config = {
		.baud_rate = handle->config.baudrate,
		.data_bits = handle->config.databits - 5, // Enum has value 0 for 5 databits.
		.parity    = handle->config.parity == 'O' ? UART_PARITY_ODD : handle->config.parity == 'E' ? UART_PARITY_EVEN : UART_PARITY_DISABLE,
		.stop_bits = handle->config.stopbits == 1 ? UART_STOP_BITS_1 : UART_STOP_BITS_2,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	if(handle->config.mode == MCU_UART_MODE_UART_RTS_CTS_FLOW_CONTROL)
	{
		if(handle->hw.io_cts != PIN_NONE && handle->hw.io_rts != PIN_NONE)
			uart_config.flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS;
		else if(handle->hw.io_cts != PIN_NONE)
			uart_config.flow_ctrl = UART_HW_FLOWCTRL_CTS;
		else if(handle->hw.io_rts != PIN_NONE)
			uart_config.flow_ctrl = UART_HW_FLOWCTRL_RTS;
		else
			uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
	}
	uart_param_config(handle->hw.unit, &uart_config);
	uart_driver_install(handle->hw.unit, handle->hw.receive_buffer_size, handle->hw.transmit_buffer_size, 0, NULL, 0);

	if(handle->config.mode == MCU_UART_MODE_UART_RTS_CTS_FLOW_CONTROL)
		uart_set_mode(handle->hw.unit, UART_MODE_UART);
	else if(handle->config.mode == MCU_UART_MODE_UART_485_HALF_DUPLEX || handle->config.mode == MCU_UART_MODE_UART_485_FULL_DUPLEX)
		uart_set_mode(handle->hw.unit, UART_MODE_RS485_HALF_DUPLEX);
	else
		uart_set_mode(handle->hw.unit, UART_MODE_UART);

	return MCU_OK;
}

MCU_RESULT mcu_uart_enable_cts(mcu_uart_t h, MCU_IO_PIN io_cts)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	mcu_io_set_dir(io_cts, MCU_IO_DIR_IN);
	mcu_io_set_pullup(io_cts, true);

	if(io_cts == PIN_NONE)
	{
		// If it was already not set, return
		if(h->hw.io_cts == PIN_NONE)
			return MCU_OK;

		gpio_pad_select_gpio((int16_t)h->hw.io_cts);
		if(h->hw.io_rts == PIN_NONE)
			uart_set_hw_flow_ctrl(h->hw.unit, UART_HW_FLOWCTRL_CTS, 122);
		else
			uart_set_hw_flow_ctrl(h->hw.unit, UART_HW_FLOWCTRL_CTS_RTS, 122);
	}
	else
	{
		if(h->hw.io_rts == PIN_NONE)
			uart_set_hw_flow_ctrl(h->hw.unit, UART_HW_FLOWCTRL_DISABLE, 122);
		else
			uart_set_hw_flow_ctrl(h->hw.unit, UART_HW_FLOWCTRL_RTS, 122);
	}
	h->hw.io_cts = io_cts;

	if(h->hw.io_cts != PIN_NONE || h->hw.io_rts != PIN_NONE)
	{
		int16_t tx = h->hw.io_tx == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_tx;
		int16_t rx = h->hw.io_rx == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_rx;
		int16_t rts = h->hw.io_rts == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_rts;
		int16_t cts = h->hw.io_cts == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_cts;
		uart_set_pin(h->hw.unit, tx, rx, rts, cts);
	}

	return MCU_OK;
}

MCU_RESULT mcu_uart_enable_rts(mcu_uart_t h, MCU_IO_PIN io_rts)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	if(io_rts == PIN_NONE)
	{
		// If it was already not set, return
		if(h->hw.io_rts == PIN_NONE)
			return MCU_OK;

		gpio_pad_select_gpio((int16_t)h->hw.io_rts);

		if(h->config.mode == MCU_UART_MODE_UART_RTS_CTS_FLOW_CONTROL)
		{
			if(h->hw.io_cts == PIN_NONE)
				uart_set_hw_flow_ctrl(h->hw.unit, UART_HW_FLOWCTRL_RTS, 122);
			else
				uart_set_hw_flow_ctrl(h->hw.unit, UART_HW_FLOWCTRL_CTS_RTS, 122);
		}
	}
	else
	{
		if(h->config.mode == MCU_UART_MODE_UART_RTS_CTS_FLOW_CONTROL)
		{
			if(h->hw.io_cts == PIN_NONE)
				uart_set_hw_flow_ctrl(h->hw.unit, UART_HW_FLOWCTRL_DISABLE, 122);
			else
				uart_set_hw_flow_ctrl(h->hw.unit, UART_HW_FLOWCTRL_CTS, 122);
		}
	}
	h->hw.io_rts = io_rts;

	if(h->hw.io_cts != PIN_NONE || h->hw.io_rts != PIN_NONE)
	{
		int16_t tx = h->hw.io_tx == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_tx;
		int16_t rx = h->hw.io_rx == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_rx;
		int16_t rts = h->hw.io_rts == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_rts;
		int16_t cts = h->hw.io_cts == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_cts;
		uart_set_pin(h->hw.unit, tx, rx, rts, cts);
	}

	return MCU_OK;
}

MCU_RESULT mcu_uart_set_param(mcu_uart_t h, uint32_t baud, uint8_t databits, uint8_t parity, uint8_t stopbits)
{
    uart_parity_t p = UART_PARITY_DISABLE;
    uart_stop_bits_t st = UART_STOP_BITS_1;
    uart_word_length_t db = UART_DATA_8_BITS;

//    uart_config_t uart_config = {
//        .baud_rate = baud,
//        .data_bits = db == 8 ? UART_DATA_8_BITS : UART_DATA_7_BITS,
//        .parity    = p == 'E' ? UART_PARITY_EVEN : p == 'O' ? UART_PARITY_ODD : UART_PARITY_DISABLE,
//        .stop_bits = st == 1 ? UART_STOP_BITS_1 : UART_STOP_BITS_2,
//        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
//    };

	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);
	
	switch(databits)
	{
		case 7:		db = UART_DATA_7_BITS; break;
		case 8:		db = UART_DATA_8_BITS; break;
		default:	MCU_RETURN_ERROR(MCU_ERROR_UART_DATABITS_INVALID, MCU_ERROR_UART_DATABITS_INVALID);
	}

	switch(parity)
	{
		case 'N':	p = UART_PARITY_DISABLE;	break;
		case 'O':	p = UART_PARITY_ODD; 		break;
		case 'E':	p = UART_PARITY_EVEN;		break;
		default:	MCU_RETURN_ERROR(MCU_ERROR_UART_PARITY_INVALID, MCU_ERROR_UART_PARITY_INVALID);
	}

	switch(stopbits)
	{
		case 1:		st = UART_STOP_BITS_1; break;
		case 2:		st = UART_STOP_BITS_2; break;
		default:	MCU_RETURN_ERROR(MCU_ERROR_UART_STOPBITS_INVALID, MCU_ERROR_UART_STOPBITS_INVALID);
	}
//	uart_param_config(h->num, &uart_config);

//	DBG_INFO("UART %d Parameter: %d %d %c %d\n", h->num, baud, databits, parity, stopbits);
	h->config.baudrate = baud;
	h->config.databits = databits;
	h->config.parity = parity;
	h->config.stopbits = stopbits;

	uart_set_word_length(h->hw.unit, db);
	uart_set_parity(h->hw.unit, p);
	uart_set_stop_bits(h->hw.unit, st);
	// uart_set_hw_flow_ctrl(h->hw.unit, UART_HW_FLOWCTRL_DISABLE, 0);
	// uart_set_mode(h->hw.unit, UART_MODE_UART);
	return mcu_uart_set_baudrate(h, baud);
//	return MCU_OK;
}

const uint32_t mcu_uart_divider[4] = {1, 4, 16, 64};

MCU_RESULT mcu_uart_set_baudrate(mcu_uart_t h, uint32_t baudrate)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	h->config.baudrate = baudrate;

	if(ESP_OK == uart_set_baudrate(h->hw.unit, baudrate))
		return MCU_OK;
	else
	{
		DBG_INFO("Error setting UART %d baudrate to %d\n", h->hw.unit, baudrate);
		MCU_RETURN_ERROR(MCU_ERROR_UART_BAUDRATE_INVALID, MCU_ERROR_UART_BAUDRATE_INVALID);
	}
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
		if(h->hw.io_tx != PIN_NONE)
			gpio_pad_select_gpio((int16_t)h->hw.io_tx);
		if(h->hw.io_rx != PIN_NONE)
			gpio_pad_select_gpio((int16_t)h->hw.io_rx);
		if(h->hw.io_rts != PIN_NONE)
			gpio_pad_select_gpio((int16_t)h->hw.io_rts);
		if(h->hw.io_cts != PIN_NONE)
			gpio_pad_select_gpio((int16_t)h->hw.io_cts);
	}
	else
	{
		int16_t tx = h->hw.io_tx == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_tx;
		int16_t rx = h->hw.io_rx == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_rx;
		int16_t rts = h->hw.io_rts == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_rts;
		int16_t cts = h->hw.io_cts == PIN_NONE ? UART_PIN_NO_CHANGE : h->hw.io_cts;
		uart_set_pin(h->hw.unit, tx, rx, rts, cts);
	}
}

MCU_RESULT mcu_uart_set_buffer(mcu_uart_t h, MCU_INT_LVL lvl, uint8_t *data, uint16_t len)
{
	return MCU_ERROR_NOT_SUPPORTED;
}

MCU_RESULT mcu_uart_set_alternate_receive(mcu_uart_t h, MCU_INT_LVL lvl, void (*f)(void*, int), void* obj)
{
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

	return MCU_OK;
}

bool mcu_uart_transmit_ready(mcu_uart_t h)
{
	if(h==NULL)
		return false;

	if(!h->config.use_direct_transmit_interrupt)
		return true;
	
	uart_dev_t* dev = UART_LL_GET_HW(h->hw.unit);
	return uart_ll_is_tx_idle(dev);
}

void mcu_uart_putc(mcu_uart_t h, int data)
{
	if(h==NULL)
		return;

	uart_write_bytes(h->hw.unit, (char*)&data, 1);
}

void mcu_uart_puts(mcu_uart_t h, uint8_t *data, uint16_t len)
{
	if(h == NULL || len == 0)
		return;

	uart_write_bytes(h->hw.unit, (char*)data, len);
}

int mcu_uart_available(mcu_uart_t h)
{
	size_t len;
	if(h==NULL)
		return 0;

	if(ESP_OK == uart_get_buffered_data_len(h->hw.unit, &len))
		return len;

	return 0;
}

int mcu_uart_getc(mcu_uart_t h)
{
	uint8_t rcv;

	if(h==NULL)
		return 0;

	uart_read_bytes(h->hw.unit, &rcv, 1, 0);

	return rcv;
}

int mcu_uart_gets(mcu_uart_t h, uint8_t* buf, uint16_t len)
{
	int cnt = 0;

	if(h==NULL)
		return 0;

	cnt = uart_read_bytes(h->hw.unit, buf, len, 0);
	
	return cnt;
}

void mcu_uart_clear_rx(mcu_uart_t h)
{
	if(h==NULL)
		return;

	uart_flush_input(h->hw.unit);
}

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

static UART_ISR_ATTR void _uart_isr_event(uart_port_t uart_num, uart_select_notif_t uart_select_notif, BaseType_t *task_woken)
{
	if(_uart_handler_hash[uart_num])
	{
		struct mcu_uart_s* uart = _uart_handler_hash[uart_num];

		switch(uart_select_notif)
		{
			case UART_SELECT_READ_NOTIF:

				if(uart->receive_notify)
					uart->receive_notify(uart->obj_receive_notify);

			break;

			default:
			break;
		}	
	}
}

#endif // MCU_PERIPHERY_ENABLE_UART

#endif // #if MCU_TYPE == R5F563
