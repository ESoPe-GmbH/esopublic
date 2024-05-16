/**
 * 	@file   mcu_uart.c
 *  @copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "mcu/mcu.h"

#if MCU_TYPE == PC_EMU
#include "mcu_internal.h"

#include "module/fifo/fifo.h"
#include "module/comm/comm.h"
#include <string.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#include "mcu_uart_tools.h"
#endif
#include <stdio.h>

#undef interface

#if MCU_PERIPHERY_DEVICE_COUNT_UART>0

#if !(MCU_TYPE==PC_EMU)
	#error "Dieses Modul geh�rt zum PC! Es wurde der falsche Prozessor ausgew�hlt."
#endif

#if MCU_PERIPHERY_UART_ENABLE_COMM_MODE
	bool mcu_uart_interface_is_created = false;
	comm_interface_t mcu_uart_comm_interface;
#endif
uint8_t mcu_current_uart_handler = 0;
mcu_uart_handler_ctx mcu_uart_handler[MCU_PERIPHERY_DEVICE_COUNT_UART];
mcu_uart_handler_ctx *mcu_uart_handler_hash[MCU_UART_TOTAL_COUNT] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static void mcu_uart_interrupt_n(uint8_t num, uint8_t data);

static void _apply_default_uart_config(mcu_uart_config_t* config);

static int _handle_esoprog(struct pt* pt);

// Es kann die UARTs 0-8 geben, also 9 Werte!

#if MCU_PERIPHERY_UART_ENABLE_COMM_MODE
void mcu_uart_create_comm_handler(mcu_uart_t h, comm_t *ch)
{
	if(ch==NULL)	return;
	if(!mcu_uart_interface_is_created)
	{
		comm_init_interface(&mcu_uart_comm_interface);
		mcu_uart_comm_interface.xputc = mcu_uart_putc;
		mcu_uart_comm_interface.xgetc = mcu_uart_getc;
		mcu_uart_comm_interface.xputs = (void (*)(void *, uint8_t *, uint16_t))mcu_uart_puts;
		mcu_uart_comm_interface.xgets = mcu_uart_gets;
		mcu_uart_comm_interface.data_present = mcu_uart_available;
		mcu_uart_interface_is_created = false;
	}
	ch->device_handler = h;
	ch->interface = &mcu_uart_comm_interface;
}
#endif



DWORD WINAPI Threadgetchar(LPVOID lpParam) //Fuer Terminal eingaben
{
  char c;
  do {
    c=getchar();
    mcu_uart_interrupt_n(0, c);
  } while (1);
  return 0;
}


#define MCU_UART_INIT_HANDLER(h, n)												\
				{																\
						h->hw.unit = n;										\
						h->alternate_receive = NULL;							\
						mcu_uart_handler_hash[n] = h;							\
				}


#define MCU_UART_CHECK_SET_RETURN(out, in)											\
				P ## out ##:

#define CAST_UART(h)	((mcu_uart_handler_ctx*)h)

#if MCU_PERIPHERY_ENABLE_COMM_MODE_UART
bool mcu_uart_interface_is_created = false;
comm_interface_t mcu_uart_comm_interface;
#endif

#if MCU_PERIPHERY_ENABLE_COMM_MODE_UART
void mcu_uart_create_comm_handler(mcu_uart_t h, comm_t *ch)
{
	if (ch == NULL)
		return;
	if (!mcu_uart_interface_is_created)
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

mcu_uart_t mcu_uart_create(mcu_uart_hw_config_t* hw, mcu_uart_config_t* config)
{
	mcu_uart_t handle;

	// Clear if maximum number of uarts is reached.
	if(mcu_current_uart_handler >= MCU_PERIPHERY_DEVICE_COUNT_UART)
		MCU_RETURN_ERROR(handle, MCU_ERROR_UART_NOT_AVAILABLE, NULL); 

	handle = &mcu_uart_handler[mcu_current_uart_handler];
	memset(handle, 0, sizeof(struct mcu_uart_s));

	fifo_init(&handle->fifo, 1, handle->rx_buffer, MCU_UART_MAX_BUFFER_SIZE);	
	_apply_default_uart_config(&handle->config);
 
	memcpy(&handle->hw, hw, sizeof(mcu_uart_hw_config_t));
	memcpy(&handle->config, config, sizeof(mcu_uart_config_t));				

	mcu_uart_set_config(handle, &handle->config);

#if MCU_UART_ENABLE_ESOPROG
	if(handle->hw.name) // Is set to use for esoprog
	{
		webclient_init(&handle->wc, network_interface_get_default(), NULL);

		char url[100];
		snprintf(url, 100, "ws://%s/ws", handle->hw.name);
		handle->ws_buffer.rx.data = mcu_heap_calloc(1, MCU_UART_MAX_BUFFER_SIZE);
		handle->ws_buffer.rx.len = MCU_UART_MAX_BUFFER_SIZE;
		handle->ws_buffer.tx.data = mcu_heap_calloc(1, MCU_UART_MAX_BUFFER_SIZE);
		handle->ws_buffer.tx.len = MCU_UART_MAX_BUFFER_SIZE;
		handle->config_changed = true;
		FUNCTION_RETURN ret = webclient_open_websocket(&handle->wc, &handle->ws, &handle->ws_buffer, url, "esoprog", 60000, 10000);
		if(ret == FUNCTION_RETURN_OK)
		{
			DBG_ERROR("Create task for esoprog\n");
			system_task_init_protothread(&handle->task_esoprog, true, _handle_esoprog, handle);
		}
		else
		{
			DBG_ERROR("Cannot create webclient for esoprog\n");
		}
	}
#endif

	mcu_current_uart_handler++;
	return (mcu_uart_t)handle;	
}

mcu_uart_t mcu_uart_init(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx)
{
	mcu_uart_handler_ctx* handle;
	if(mcu_current_uart_handler>=MCU_PERIPHERY_DEVICE_COUNT_UART)		MCU_RETURN_ERROR_GLB(MCU_ERROR_UART_NOT_AVAILABLE, NULL) // Es gibt keine freie UART mehr!
	handle = &mcu_uart_handler[mcu_current_uart_handler];	// F�r einfachere Handhabung

	MCU_UART_INIT_HANDLER(handle, num);
	fifo_init(&handle->fifo, 1, handle->rx_buffer, MCU_UART_MAX_BUFFER_SIZE);	

	memset(&handle->fifo, 0, sizeof(fifo_t));
	mcu_current_uart_handler++;
	return (mcu_uart_t)handle;
}

MCU_RESULT mcu_uart_set_param(mcu_uart_t h, uint32_t baud, uint8_t databits, uint8_t parity, uint8_t stopbits)
{
    int p = 0;
    int b = 0;
    int s = 0;
	if(h==NULL)		MCU_RETURN_ERROR_GLB(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

    if(((mcu_uart_handler_ctx*)h)->hw.unit == 0)
    {
        HANDLE hThreadTimer;

        hThreadTimer = CreateThread( NULL,  0, Threadgetchar, (LPVOID)0, 0, NULL);

        if (hThreadTimer == NULL)
        {
            printf("Timer nicht gestartet\n");
        }

        return 0xFFFF; //PC Ausgabe
    }

    switch(parity)
    {
        case 'N':   p = P_NONE;     break;
        case 'E':   p = P_EVEN;     break;
        case 'O':   p = P_ODD;      break;
        default:    p = P_NONE;     break;
    }
    switch(databits)
    {
        case 8:     b = D_8BIT;     break;
        case 7:     b = D_7BIT;     break;
        default:    b = D_8BIT;     break;
    }
    switch(stopbits)
    {
        case 1:     s = S_1BIT;     break;
        case 2:     s = S_2BIT;     break;
    }

	((mcu_uart_handler_ctx*)h)->config.baudrate = baud;

	if(h->hw.name)
	{
		h->config.baudrate = baud;
		h->config.databits = databits;
		h->config.parity = parity;
		h->config.stopbits = stopbits;
		h->config_changed = true;
        return MCU_OK;
	}

    if(!ComOpen (((mcu_uart_handler_ctx*)h)->hw.unit - 1,   baud,   p,  s,   b))
    {
        printf("Com %d not open\n", ((mcu_uart_handler_ctx*)h)->hw.unit);
        return MCU_OK;
    }
    else
    {
        printf("Com %d open\n", ((mcu_uart_handler_ctx*)h)->hw.unit);
        MCU_RETURN_ERROR_GLB(MCU_ERROR_UART_INVALID, 0);
    }
}


MCU_RESULT mcu_uart_set_config(mcu_uart_t handle, mcu_uart_config_t* config)
{
	if(handle == NULL)
		MCU_RETURN_ERROR(handle, MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);
	if(config == NULL)
		MCU_RETURN_ERROR(handle, MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	if(config != &handle->config)
	{
		memcpy(&handle->config, config, sizeof(mcu_uart_config_t));
		_apply_default_uart_config(&handle->config);
	}

	return mcu_uart_set_param(handle, handle->config.baudrate, handle->config.databits, handle->config.parity, handle->config.stopbits);
}

const mcu_uart_config_t* mcu_uart_get_config(mcu_uart_t h)
{
	if(h == NULL)
		MCU_RETURN_ERROR(h, MCU_ERROR_UART_INVALID, NULL);

	return &h->config;
}

MCU_RESULT mcu_uart_set_baudrate(mcu_uart_t h, uint32_t baudrate)
{
	if(h==NULL)		MCU_RETURN_ERROR_GLB(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	//((mcu_uart_handler_ctx*)h)->baud = baud;
	return MCU_OK;
}

uint32_t mcu_uart_get_baudrate(mcu_uart_t h)
{
	if(h==NULL)		MCU_RETURN_ERROR_GLB(MCU_ERROR_UART_INVALID, 0);
	return ((mcu_uart_handler_ctx*)h)->config.baudrate;
}

MCU_RESULT mcu_uart_set_buffer(mcu_uart_t h, MCU_INT_LVL lvl, uint8_t *data, uint16_t len)
{
	mcu_uart_handler_ctx* handle;
	if(h==NULL)					MCU_RETURN_ERROR_GLB(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);
	if(data==NULL || len==0)	MCU_RETURN_ERROR(CAST_UART(h), MCU_ERROR_UART_RECEIVE_INVALID, MCU_ERROR_UART_RECEIVE_INVALID);
	handle = (mcu_uart_handler_ctx*)h;
    // fifo_init(&handle->fifo, 1, data, len);				// Der Buffer enth�lt 1 Byte gro�e Elemente
	return MCU_OK;
}

MCU_RESULT mcu_uart_set_alternate_receive(mcu_uart_t h, MCU_INT_LVL lvl, void (*f)(void*, int), void* obj)
{
	mcu_uart_handler_ctx* handle;
	if(h==NULL)		MCU_RETURN_ERROR_GLB(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);
	if(f==NULL)		MCU_RETURN_ERROR(CAST_UART(h), MCU_ERROR_UART_RECEIVE_INVALID, MCU_ERROR_UART_RECEIVE_INVALID);
	handle = (mcu_uart_handler_ctx*)h;
	handle->alternate_receive = f;
	handle->obj = obj;
	return MCU_OK;
}

MCU_RESULT mcu_uart_set_direct_transmit_interrupt(mcu_uart_t h)
{
	return MCU_OK;
}

MCU_RESULT mcu_uart_enable_cts(mcu_uart_t h, MCU_IO_PIN io_cts)
{

	return MCU_OK;
}

bool mcu_uart_transmit_ready(mcu_uart_t h)
{
	if(h == NULL)
		return false;

#if MCU_UART_ENABLE_ESOPROG
	if(h->hw.name)
	{
		return h->tx[h->tx_cnt].tx_length < MCU_UART_MAX_BUFFER_SIZE;
	}
#endif

	return true;
}

void mcu_uart_putc(mcu_uart_t h, int data)
{
	if(h==NULL)		return;
	
#if MCU_UART_ENABLE_ESOPROG
	if(h->hw.name) // Is set to use for esoprog
	{
		uint8_t buffer_index = h->tx_cnt;
		// Add byte to next free buffer
		for(int i = 0; i < MCU_UART_NUM_BUFFER; i++, buffer_index = (buffer_index + 1) % MCU_UART_NUM_BUFFER)
		{
			if(h->tx[buffer_index].tx_length < MCU_UART_MAX_BUFFER_SIZE)
			{
				// DBG_INFO("Add %02x to %d in %d\n", data, h->tx[buffer_index].tx_length, buffer_index);
				h->tx[buffer_index].tx_buffer[h->tx[buffer_index].tx_length] = data;
				h->tx[buffer_index].tx_length++;
				return;
			}
		}
		// if(h->tx[h->tx_cnt].tx_length >= MCU_UART_MAX_BUFFER_SIZE)
		// 	return;

		// h->tx[h->tx_cnt].tx_buffer[h->tx[h->tx_cnt].tx_length] = data;
		// h->tx[h->tx_cnt].tx_length++;
	}
    else 
#endif
	{
		if(((mcu_uart_handler_ctx*)h)->hw.unit == 0) //PC Ausgabe
		{
			printf("%c", data);
		}
		else
			ComWrite(((mcu_uart_handler_ctx*)h)->hw.unit - 1, data);
	}
}

void mcu_uart_puts(mcu_uart_t h, uint8_t *data, uint16_t len)
{
	while(len--)
		mcu_uart_putc(h, *data++);
}

int mcu_uart_available(mcu_uart_t h)
{
	if(h==NULL)		
		return 0;

	if(h->hw.name == NULL)
	{
		while(ComGetReadCount (((mcu_uart_handler_ctx*)h)->hw.unit - 1))
		{
			mcu_uart_interrupt_n(((mcu_uart_handler_ctx*)h)->hw.unit, 0x00);
		}
	}

	return fifo_data_available(&((mcu_uart_handler_ctx*)h)->fifo);
}

int mcu_uart_getc(mcu_uart_t h)
{
	int rcv;
	if(h==NULL)		
		return 0;

	fifo_get(&((mcu_uart_handler_ctx*)h)->fifo, (uint8_t*)&rcv);
	
	return rcv;
}

int mcu_uart_gets(mcu_uart_t h, uint8_t* buf, uint16_t len)
{
	int cnt = 0;

	if(mcu_uart_available(h) == 0)
		return 0;

	while(len-- && fifo_data_available(&((mcu_uart_handler_ctx*)h)->fifo))
	{
		fifo_get(&((mcu_uart_handler_ctx*)h)->fifo, buf);
		buf += 1;//((mcu_uart_handler_ctx*)h)->fifo.element_size;	// 9-Bit +=2 vs 7,8-Bit +=1
		cnt++;
	}
	return cnt;
}

void mcu_uart_clear_rx(mcu_uart_t h)
{
	if(h==NULL)		return;
	fifo_clear(&((mcu_uart_handler_ctx*)h)->fifo);
}

static void mcu_uart_interrupt_n(uint8_t num, uint8_t data)
{
	int rcv;
	mcu_uart_handler_ctx* h;
	if(num>=MCU_UART_TOTAL_COUNT)			
		return;

	if(mcu_uart_handler_hash[num]==NULL)	
		return;

	h = mcu_uart_handler_hash[num];

    if(num == 0 || h->hw.name) // PC UART or Esoprog
    {
        rcv = data;
    }
    else
    {
        rcv = ComRead (h->hw.unit - 1);
    }

	if(h->alternate_receive)
		(h->alternate_receive)(h->obj, rcv);
	else
		fifo_put(&h->fifo, (uint8_t*)&rcv);
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

#if MCU_UART_ENABLE_ESOPROG
static int _handle_esoprog(struct pt* pt)
{
	struct mcu_uart_s* uart = pt->obj;
	PT_BEGIN(pt);
	DBG_INFO("EsoProg %s: BEGIN\n", uart->hw.name);

	// Wait until websocket is initialized
	PT_WAIT_UNTIL(pt, websocket_get_state(&uart->ws) != WEBSOCKET_STATE_INIT && websocket_get_state(&uart->ws) != WEBSOCKET_STATE_BUSY);
	
	DBG_INFO("EsoProg %s: Begin loop\n", uart->hw.name);
	do
	{
		// Wait until websocket is not busy
		PT_YIELD_UNTIL(pt, websocket_get_state(&uart->ws) != WEBSOCKET_STATE_BUSY);
		// If it is established -> Check for a received dataframe
		if(websocket_get_state(&uart->ws) == WEBSOCKET_STATE_ESTABLISHED)
		{
			WEBSOCKET_TYPE_T type;
			uint32_t len;
			if((len = websocket_receive_a(&uart->ws, &type)) > 0)
			{
				// DBG_INFO("Received: type=%d length=%d\n", type, len);
				// Only handle text dataframes!
				if(type == WEBSOCKET_TYPE_BINARY)
				{
					uint32_t offset = 0;
					for(offset = 0; offset < len; offset++)
					{
						// mcu_uart_interrupt_n(uart->hw.unit, uart->ws.buf->rx.data[offset]);

						if(uart->alternate_receive)
							(uart->alternate_receive)(uart->obj, uart->ws.buf->rx.data[offset]);
						else
							fifo_put(&uart->fifo, &uart->ws.buf->rx.data[offset]);
					}
				}
				// else: Ignore text frames...
			}

			if(uart->config_changed)
			{
				uart->settings.tx_length = snprintf(uart->settings.tx_buffer, MCU_UART_MAX_BUFFER_SIZE, 
					"{"\
    				"\"type\": \"settings\","\
    				"\"content\": {"\
        				"\"uart_bd\": %u,"\
        				"\"uart_db\": %u,"\
        				"\"uart_p\": \"%c\","\
        				"\"uart_sb\": %u,"\
        				"\"uart_485\": %s}}",
						uart->config.baudrate,
						uart->config.databits,
						uart->config.parity,
						uart->config.stopbits,
						(uart->config.mode == MCU_UART_MODE_UART_485_FULL_DUPLEX || uart->config.mode == MCU_UART_MODE_UART_485_HALF_DUPLEX) ? "true" : "false");
				
				
				FUNCTION_RETURN ret = websocket_send(&uart->ws, WEBSOCKET_TYPE_TEXT, uart->settings.tx_buffer, uart->settings.tx_length);
				if(ret == FUNCTION_RETURN_OK)
				{
					DBG_INFO("Send %s\n", uart->settings.tx_buffer);
					uart->config_changed = false;
				}
			}
			else if(uart->tx[uart->tx_cnt].tx_length > 0 && ( (system_get_tick_count() - uart->timestamp_sent) > 50))
			{
				FUNCTION_RETURN ret = websocket_send(&uart->ws, WEBSOCKET_TYPE_BINARY, uart->tx[uart->tx_cnt].tx_buffer, uart->tx[uart->tx_cnt].tx_length);
				if(ret == FUNCTION_RETURN_OK)
				{
					uart->timestamp_sent = system_get_tick_count();
					// DBG_INFO("Send %d from %d: %#Q\n", uart->tx[uart->tx_cnt].tx_length, uart->tx_cnt, uart->tx[uart->tx_cnt].tx_length > 32 ? 32 : uart->tx[uart->tx_cnt].tx_length, uart->tx[uart->tx_cnt].tx_buffer);
					uart->tx[uart->tx_cnt].tx_length = 0;
					uart->tx_cnt = (uart->tx_cnt + 1) % MCU_UART_NUM_BUFFER;
				}
			}
		}
		// Once the websocket is closed -> stop the receiving.
	}while(websocket_get_state(&uart->ws) != WEBSOCKET_STATE_CLOSED);

	DBG_INFO("EsoProg %s: END\n", uart->hw.name);
	PT_END(pt);
}
#endif

#endif // MCU_PERIPHERY_DEVICE_COUNT_UART>0

#endif