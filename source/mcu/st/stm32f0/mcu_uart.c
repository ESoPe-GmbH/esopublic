// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_uart.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_STM32F0

#include "../stm32f0/mcu_internal.h"
#include "module/fifo/fifo.h"
#include <string.h>

#if MCU_PERIPHERY_DEVICE_COUNT_UART

#if !(MCU_CONTROLLER_PIN_COUNT==48)
#error "mcu_uart only supports 48 pin version at the moment."
#endif

#define MCU_UART_INIT_HANDLER(h, n)													\
		{																	\
	h->num = n;													\
	h->alternate_receive = NULL;								\
	h->use_transmit_buffer = false;								\
	h->tx_lvl = MCU_INT_LVL_OFF;								\
	h->transmit_buffer_length = 0;								\
	h->transmit_buffer_cnt = 0;									\
	mcu_uart_handler_hash[n] = h;								\
		}

#if MCU_PERIPHERY_ENABLE_COMM_MODE_UART
bool mcu_uart_interface_is_created = false;
comm_interface_t mcu_uart_comm_interface;
#endif
uint8_t mcu_current_uart_handler = 0;
mcu_uart_handler_ctx mcu_uart_handler[MCU_PERIPHERY_DEVICE_COUNT_UART];
mcu_uart_handler_ctx *mcu_uart_handler_hash[MCU_UART_MAX_NUM + 1] = {0};

//static void mcu_uart_handle(mcu_uart_handler_ctx* h);

#if MCU_PERIPHERY_ENABLE_COMM_MODE_UART
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
		mcu_uart_interface_is_created = true;
	}
	ch->device_handler = h;
	ch->interface = &mcu_uart_comm_interface;
}
#endif

mcu_uart_t mcu_uart_init(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx)
{
	mcu_uart_handler_ctx* handle;

	if(mcu_current_uart_handler >= MCU_PERIPHERY_DEVICE_COUNT_UART)
		MCU_RETURN_ERROR(MCU_ERROR_UART_NOT_AVAILABLE, NULL); // Es gibt keine freie UART mehr!

	handle = &mcu_uart_handler[mcu_current_uart_handler];

	if(num == 1)
	{
		if(!(tx == PA_9 || tx == PB_6))
			MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);
		if(!(rx == PA_10 || rx == PB_7))
			MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);

		MCU_UART_INIT_HANDLER(handle, 1);
		handle->h_uart.Instance = USART1;

		// USART1 TX -> DMA2 Stream 7, Channel 7
		// USART1 RX -> DMA2 Stream 2/5, Channel 4
//		handle->dma_tx = DMA2_Stream7_IRQn;
//		handle->dma_rx = DMA2_Stream2_IRQn;
		handle->irq = USART1_IRQn;

//		handle->h_dma_tx.Instance = DMA2_Stream7;
//		handle->h_dma_tx.Init.Channel = DMA_CHANNEL_4;
//		handle->h_dma_rx.Instance = DMA2_Stream2;
//		handle->h_dma_rx.Init.Channel = DMA_CHANNEL_4;
	}
	else if(num == 2)
	{
		if(!(tx == PA_2 || tx == PA_14 || tx == PD_5))
			MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);
		if(!(rx == PA_3 || rx == PA_15 || rx == PD_6))
			MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);

		MCU_UART_INIT_HANDLER(handle, 2);
		handle->h_uart.Instance = USART2;
		// USART2 TX -> DMA1 Stream 6, Channel 4
		// USART2 RX -> DMA1 Stream 5, Channel 4
//		handle->dma_tx = DMA1_Stream6_IRQn;
//		handle->dma_rx = DMA1_Stream5_IRQn;
		handle->irq = USART2_IRQn;

//		handle->h_dma_tx.Instance = DMA1_Stream6;
//		handle->h_dma_tx.Init.Channel = DMA_CHANNEL_4;
//		handle->h_dma_rx.Instance = DMA1_Stream5;
//		handle->h_dma_rx.Init.Channel = DMA_CHANNEL_4;
	}
	else if(num == 3)
	{
		if(!(tx == PB_10 || tx == PC_4 || tx == PC_10 || tx == PD_8))
			MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);
		if(!(rx == PB_11 || rx == PC_5 || rx == PC_11 || rx == PD_9))
			MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);

		MCU_UART_INIT_HANDLER(handle, 3);
		handle->h_uart.Instance = USART3;

		// USART3 TX -> DMA1 Stream 3, Channel 4
		// USART3 RX -> DMA1 Stream 1, Channel 4
//		handle->dma_tx = DMA1_Stream3_IRQn;
//		handle->dma_rx = DMA1_Stream1_IRQn;
		handle->irq = USART3_4_IRQn;

//		handle->h_dma_tx.Instance = DMA1_Stream3;
//		handle->h_dma_tx.Init.Channel = DMA_CHANNEL_4;
//		handle->h_dma_rx.Instance = DMA1_Stream1;
//		handle->h_dma_rx.Init.Channel = DMA_CHANNEL_4;
	}
	else if(num == 4)
	{
		if(!(tx == PA_0 || tx == PC_10))
			MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);
		if(!(rx == PA_1 || rx == PC_11))
			MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);

		MCU_UART_INIT_HANDLER(handle, 4);
		handle->h_uart.Instance = USART4;

		// UART4 TX -> DMA1 Stream 4, Channel 4
		// UART4 RX -> DMA1 Stream 2, Channel 4
//		handle->dma_tx = DMA1_Stream4_IRQn;
//		handle->dma_rx = DMA1_Stream2_IRQn;
		handle->irq = USART3_4_IRQn;

//		handle->h_dma_tx.Instance = DMA1_Stream4;
//		handle->h_dma_tx.Init.Channel = DMA_CHANNEL_4;
//		handle->h_dma_rx.Instance = DMA1_Stream2;
//		handle->h_dma_rx.Init.Channel = DMA_CHANNEL_4;
	}
	else
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);

	handle->io_tx = tx;
	handle->io_rx = rx;

	handle->h_uart.Init.BaudRate = 115200;
	handle->h_uart.Init.WordLength = UART_WORDLENGTH_8B;
	handle->h_uart.Init.StopBits = UART_STOPBITS_1;
	handle->h_uart.Init.Parity = UART_PARITY_NONE;
	handle->h_uart.Init.Mode = UART_MODE_TX_RX;
	handle->h_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	handle->h_uart.Init.OverSampling = UART_OVERSAMPLING_16;
	handle->h_uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	handle->h_uart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

//	if(num < 7) // 7 and 8 do not use DMA!
//	{
//		handle->h_dma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
//		handle->h_dma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
//		handle->h_dma_tx.Init.MemInc = DMA_MINC_ENABLE;
//		handle->h_dma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//		handle->h_dma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
//		handle->h_dma_tx.Init.Mode = DMA_NORMAL;
//		handle->h_dma_tx.Init.Priority = DMA_PRIORITY_LOW;
//		handle->h_dma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
//
////		handle->h_dma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
////		handle->h_dma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
////		handle->h_dma_rx.Init.MemInc = DMA_MINC_ENABLE;
////		handle->h_dma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
////		handle->h_dma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
////		handle->h_dma_rx.Init.Mode = DMA_NORMAL;
////		handle->h_dma_rx.Init.Priority = DMA_PRIORITY_LOW;
////		handle->h_dma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
//	}

	mcu_uart_handler_hash[num] = handle;

	if (HAL_UART_Init(&handle->h_uart) != HAL_OK)
	{
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);
	}

//	HAL_NVIC_SetPriority(handle->dma_tx, 0, 0);
//	HAL_NVIC_EnableIRQ(handle->dma_tx);
	HAL_NVIC_SetPriority(handle->irq, 0, 0);
	HAL_NVIC_EnableIRQ(handle->irq);

	mcu_current_uart_handler++;
	return (mcu_uart_t)handle;
}

MCU_RESULT mcu_uart_enable_cts(mcu_uart_t h, MCU_IO_PIN io_cts)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	mcu_io_set_dir(io_cts, MCU_IO_DIR_IN);
	mcu_io_set_pullup(io_cts, true);

	// TODO: CTS/RTS

	return MCU_OK;
}

MCU_RESULT mcu_uart_set_param(mcu_uart_t h, uint32_t baud, uint8_t databits, uint8_t parity, uint8_t stopbits)
{
	mcu_uart_handler_ctx* handle;
	bool has_parity = false;
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	handle = (mcu_uart_handler_ctx*)h;

	switch(parity)
	{
	case 'N':	handle->h_uart.Init.Parity = UART_PARITY_NONE;	break;
	case 'O':	handle->h_uart.Init.Parity = UART_PARITY_ODD; 	has_parity = true; break;
	case 'E':	handle->h_uart.Init.Parity = UART_PARITY_EVEN;	has_parity = true; break;
	default:	MCU_RETURN_ERROR(MCU_ERROR_UART_PARITY_INVALID, MCU_ERROR_UART_PARITY_INVALID);
	}

	switch(databits)
	{
	case 7:		handle->h_uart.Init.WordLength = has_parity ? UART_WORDLENGTH_8B : UART_WORDLENGTH_7B; break;
	case 8: 	handle->h_uart.Init.WordLength = has_parity ? UART_WORDLENGTH_9B : UART_WORDLENGTH_8B; break;
	default:	MCU_RETURN_ERROR(MCU_ERROR_UART_DATABITS_INVALID, MCU_ERROR_UART_DATABITS_INVALID);
	}

	switch(stopbits)
	{
	case 1:		handle->h_uart.Init.StopBits = UART_STOPBITS_1; break;
	case 2:		handle->h_uart.Init.StopBits = UART_STOPBITS_2; break;
	default:	MCU_RETURN_ERROR(MCU_ERROR_UART_STOPBITS_INVALID, MCU_ERROR_UART_STOPBITS_INVALID);
	}

	return mcu_uart_set_baudrate(h, baud);
}

//const uint32_t mcu_uart_divider[4] = {1, 4, 16, 64};

MCU_RESULT mcu_uart_set_baudrate(mcu_uart_t h, uint32_t baudrate)
{
	mcu_uart_handler_ctx* handle;

	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	handle = (mcu_uart_handler_ctx*)h;

	handle->h_uart.Init.BaudRate = baudrate;

//	if(UART_SetConfig(&handle->h_uart) != HAL_OK)
	if (HAL_UART_Init(&handle->h_uart) != HAL_OK)
		MCU_RETURN_ERROR(MCU_ERROR_UART_BAUDRATE_INVALID, MCU_ERROR_UART_BAUDRATE_INVALID);

	return MCU_OK;
}

void mcu_uart_restore(mcu_uart_t h)
{
	mcu_uart_handler_ctx* handle;

	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, );

	handle = (mcu_uart_handler_ctx*)h;
	HAL_UART_Init(&handle->h_uart);
}

uint32_t mcu_uart_get_baudrate(mcu_uart_t h)
{
	if(h==NULL)		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, 0);
	return ((mcu_uart_handler_ctx*)h)->h_uart.Init.BaudRate;
}

void mcu_uart_set_pin_gpio(mcu_uart_t h, bool b)
{
	if(h==NULL)		return;

	if(b)
	{
		// TODO: Convert to GPIO function

	}
	else
	{
		// TODO: Convert to UART function

	}
}

MCU_RESULT mcu_uart_set_buffer(mcu_uart_t h, MCU_INT_LVL lvl, uint8_t *data, uint16_t len)
{
	mcu_uart_handler_ctx* handle;

	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	if(data==NULL || len==0)
		MCU_RETURN_ERROR(MCU_ERROR_UART_RECEIVE_INVALID, MCU_ERROR_UART_RECEIVE_INVALID);

	handle = (mcu_uart_handler_ctx*)h;

	fifo_init(&handle->fifo, 1, data, len);				// Der Buffer enthält 1 Byte große Elemente

	__HAL_UART_ENABLE_IT(&handle->h_uart, UART_IT_RXNE);

	return MCU_OK;
}

MCU_RESULT mcu_uart_set_direct_transmit_interrupt(mcu_uart_t h)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, MCU_ERROR_UART_INVALID);

	((mcu_uart_handler_ctx*)h)->use_transmit_buffer = true;
	((mcu_uart_handler_ctx*)h)->transmit_buffer_cnt = 0;

	return MCU_OK;
}

bool mcu_uart_transmit_ready(mcu_uart_t h)
{
	if(h==NULL)
		return true;

	return HAL_UART_GetState(&((mcu_uart_handler_ctx*)h)->h_uart) == HAL_UART_STATE_READY;
}

void mcu_uart_putc(mcu_uart_t h, int data)
{
	if(h==NULL)
		return;

	uint32_t tick = HAL_GetTick();
	while(HAL_UART_GetState(&((mcu_uart_handler_ctx*)h)->h_uart) != HAL_UART_STATE_READY && HAL_GetTick() - tick < 1000)
	{

	}

	if(HAL_UART_GetState(&((mcu_uart_handler_ctx*)h)->h_uart) != HAL_UART_STATE_READY)
	{
		HAL_UART_Abort(&((mcu_uart_handler_ctx*)h)->h_uart);
		//		HAL_UART_Init(&handle->h_uart);
		//		return;
	}

//	if(((mcu_uart_handler_ctx*)h)->num < 7)
//		HAL_UART_Transmit_DMA(&((mcu_uart_handler_ctx*)h)->h_uart, (uint8_t*)&data, 1);
//	else
//	{
		if(((mcu_uart_handler_ctx*)h)->use_transmit_buffer)
			HAL_UART_Transmit_IT(&((mcu_uart_handler_ctx*)h)->h_uart, (uint8_t*)&data, 1);
		else
			HAL_UART_Transmit(&((mcu_uart_handler_ctx*)h)->h_uart, (uint8_t*)&data, 1, 1000);
//	}
}

void mcu_uart_puts(mcu_uart_t h, uint8_t *data, int16_t len)
{
	if(h == NULL || len == 0)
		return;

	uint32_t tick = HAL_GetTick();
	while(HAL_UART_GetState(&((mcu_uart_handler_ctx*)h)->h_uart) != HAL_UART_STATE_READY && HAL_GetTick() - tick < 1000)
	{

	}

	if(HAL_UART_GetState(&((mcu_uart_handler_ctx*)h)->h_uart) != HAL_UART_STATE_READY)
	{
		HAL_UART_Abort(&((mcu_uart_handler_ctx*)h)->h_uart);
//		HAL_UART_Init(&handle->h_uart);
//		return;
	}

//	if(((mcu_uart_handler_ctx*)h)->num < 7)
//		HAL_UART_Transmit_DMA(&((mcu_uart_handler_ctx*)h)->h_uart, data, len);
//	else
//	{
		if(((mcu_uart_handler_ctx*)h)->use_transmit_buffer)
			HAL_UART_Transmit_IT(&((mcu_uart_handler_ctx*)h)->h_uart, data, len);
		else
			HAL_UART_Transmit(&((mcu_uart_handler_ctx*)h)->h_uart, data, len, 1000);
//	}
}

int mcu_uart_available(mcu_uart_t h)
{
	if(h==NULL)
		return 0;

	return (int)fifo_data_available(&((mcu_uart_handler_ctx*)h)->fifo);
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

	if(h==NULL)
		return 0;

	while(cnt < len && fifo_data_available(&((mcu_uart_handler_ctx*)h)->fifo))
	{
		fifo_get(&((mcu_uart_handler_ctx*)h)->fifo, buf);
		buf+=((mcu_uart_handler_ctx*)h)->fifo.element_size;	// 9-Bit +=2 vs 7,8-Bit +=1
		cnt++;
	}
	return cnt;
}

void mcu_uart_clear_rx(mcu_uart_t h)
{
	if(h==NULL)
		return;

	fifo_clear(&((mcu_uart_handler_ctx*)h)->fifo);
}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef tx = {0};
	GPIO_InitTypeDef rx = {0};
	mcu_uart_handler_ctx* h;

	tx.Mode = GPIO_MODE_AF_PP;
	tx.Pull = GPIO_NOPULL;
	tx.Speed = GPIO_SPEED_FREQ_HIGH;

	rx.Mode = GPIO_MODE_AF_PP;
	rx.Pull = GPIO_PULLUP;
	rx.Speed = GPIO_SPEED_FREQ_HIGH;

	if(huart->Instance==USART1)
	{
		h = mcu_uart_handler_hash[1];
		__HAL_RCC_USART1_CLK_ENABLE();

		// h->io_tx

		if(h->io_tx == PA_9)
			tx.Alternate = GPIO_AF1_USART1;
		else if(h->io_tx == PB_6)
			tx.Alternate = GPIO_AF0_USART1;
		if(h->io_rx == PA_10)
			rx.Alternate = GPIO_AF1_USART1;
		else if(h->io_rx == PB_7)
			rx.Alternate = GPIO_AF0_USART1;
	}
	else if(huart->Instance==USART2)
	{
		h = mcu_uart_handler_hash[2];
		__HAL_RCC_USART2_CLK_ENABLE();

		if(h->io_tx == PA_2 || h->io_tx == PA_14)
			tx.Alternate = GPIO_AF1_USART2;
		else
			tx.Alternate = GPIO_AF0_USART2;
		if(h->io_rx == PA_3 || h->io_rx == PA_15)
			rx.Alternate = GPIO_AF1_USART2;
		else
			rx.Alternate = GPIO_AF0_USART2;
	}
	else if(huart->Instance==USART3)
	{
		h = mcu_uart_handler_hash[3];
		__HAL_RCC_USART3_CLK_ENABLE();

		if(h->io_tx == PB_10)
			tx.Alternate = GPIO_AF4_USART3;
		else if(h->io_tx == PC_4 || h->io_tx == PC_10)
			tx.Alternate = GPIO_AF1_USART3;
		else
			tx.Alternate = GPIO_AF0_USART3;
		if(h->io_rx == PB_11)
			rx.Alternate = GPIO_AF4_USART3;
		else if(h->io_rx == PC_5 || h->io_rx == PC_11)
			rx.Alternate = GPIO_AF1_USART3;
		else
			rx.Alternate = GPIO_AF0_USART3;
	}
	else if(huart->Instance==USART4)
	{
		h = mcu_uart_handler_hash[4];
		__HAL_RCC_USART4_CLK_ENABLE();

		if(h->io_tx == PA_0)
			tx.Alternate = GPIO_AF4_USART4;
		else
			tx.Alternate = GPIO_AF0_USART4;
		if(h->io_rx == PA_1)
			rx.Alternate = GPIO_AF4_USART4;
		else
			rx.Alternate = GPIO_AF0_USART4;
	}
	else
		return;

	tx.Pin = 1 << (h->io_tx & 0xFF);
	rx.Pin = 1 << (h->io_rx & 0xFF);

	HAL_GPIO_Init(mcu_internal_get_port(h->io_tx), &tx);
	HAL_GPIO_Init(mcu_internal_get_port(h->io_rx), &rx);

//	if(h->num < 7)
//	{
//		HAL_DMA_Init(&h->h_dma_tx);
//		__HAL_LINKDMA(huart,hdmatx,h->h_dma_tx);
//	}
}

/**
 * @brief UART MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
	// TODO: De-Init? Not used before...
	//    __HAL_RCC_UART4_CLK_DISABLE();
	//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);
	//    HAL_DMA_DeInit(huart->hdmarx);
	//    HAL_DMA_DeInit(huart->hdmatx);
}
//void USART_IrqHandler(mcu_uart_handler_ctx* uart)
//{
//	UART_HandleTypeDef *huart = &uart->h_uart;
//
//	uint32_t isrflags   = READ_REG(huart->Instance->ISR);
//	uint32_t cr1its     = READ_REG(huart->Instance->CR1);
//	uint32_t cr3its     = READ_REG(huart->Instance->CR3);
//
//	uint32_t errorflags;
////	uint32_t errorcode;
//
//	/* If no error occurs */
//	errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE | USART_ISR_RTOF));
//
//	/* If some errors occur */
//	if ((errorflags != 0U) && (((cr3its & USART_CR3_EIE) != 0U) || ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != 0U)))
//	{
//		/* UART parity error interrupt occurred -------------------------------------*/
//		if (((isrflags & USART_ISR_PE) != 0U) && ((cr1its & USART_CR1_PEIE) != 0U))
//		{
//			__HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_PEF);
//
////			huart->ErrorCode |= HAL_UART_ERROR_PE;
//		}
//
//		/* UART frame error interrupt occurred --------------------------------------*/
//		if (((isrflags & USART_ISR_FE) != 0U) && ((cr3its & USART_CR3_EIE) != 0U))
//		{
//			__HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_FEF);
//
////			huart->ErrorCode |= HAL_UART_ERROR_FE;
//		}
//
//		/* UART noise error interrupt occurred --------------------------------------*/
//		if (((isrflags & USART_ISR_NE) != 0U) && ((cr3its & USART_CR3_EIE) != 0U))
//		{
//			__HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_NEF);
//
////			huart->ErrorCode |= HAL_UART_ERROR_NE;
//		}
//
//		/* UART Over-Run interrupt occurred -----------------------------------------*/
//		if (((isrflags & USART_ISR_ORE) != 0U)
//				&& (((cr1its & USART_CR1_RXNEIE) != 0U) ||
//						((cr3its & USART_CR3_EIE) != 0U)))
//		{
//			__HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF);
//
////			huart->ErrorCode |= HAL_UART_ERROR_ORE;
//		}
//
//		/* UART Receiver Timeout interrupt occurred ---------------------------------*/
//		if (((isrflags & USART_ISR_RTOF) != 0U) && ((cr1its & USART_CR1_RTOIE) != 0U))
//		{
//			__HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_RTOF);
//
////			huart->ErrorCode |= HAL_UART_ERROR_RTO;
//		}
//
////		/* Call UART Error Call back function if need be ----------------------------*/
////		if (huart->ErrorCode != HAL_UART_ERROR_NONE)
////		{
////			/* UART in mode Receiver --------------------------------------------------*/
////			if (((isrflags & USART_ISR_RXNE) != 0U)
////					&& ((cr1its & USART_CR1_RXNEIE) != 0U))
////			{
////				if (huart->RxISR != NULL)
////				{
////					huart->RxISR(huart);
////				}
////			}
////		}
//	}
//
//
////	if (huart->Instance->ISR & UART_FLAG_RXNE)  // A byte was received!
//	if (((isrflags & USART_ISR_RXNE) != 0U) && ((cr1its & USART_CR1_RXNEIE) != 0U))
//	{
//		uint8_t data;
//		__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_RXNE); // Clear the receive flag
//		data = huart->Instance->RDR; // Read the byte from the register
//
//		if((uart->fifo.write_pos + uart->fifo.element_size) % uart->fifo.max_len != uart->fifo.read_pos)
//		{
//			uint8_t* dst = uart->fifo.data + uart->fifo.write_pos;
//			*dst = data; // Read the byte from the register
//			uart->fifo.write_pos = (uart->fifo.write_pos + uart->fifo.element_size) % uart->fifo.max_len;
//			//			memcpy(bs->data+bs->write_pos, c, bs->element_size);
//			//			bs->write_pos = (bs->write_pos+bs->element_size)%bs->max_len;
//			//		(bs->entries)++;
//			//		mcu_enable_interrupt();
//			//			return true;
//		}
//		//		fifo_put8(&uart->fifo, data);  // Put the read byte into the fifo!
//	}
//	//	if(huart->Instance->ISR & UART_FLAG_ORE)
//	//	{
//	//		__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE); // Clear the flag
//	//	}
//	//	if(huart->Instance->ISR & USART_ISR_NE)
//	//	{
//	//		__HAL_UART_CLEAR_FLAG(huart, USART_ISR_NE); // Clear the flag
//	//	}
//	//	if(huart->Instance->ISR & USART_ISR_FE)
//	//	{
//	//		__HAL_UART_CLEAR_FLAG(huart, USART_ISR_FE); // Clear the flag
//	//	}
//	//	if(huart->Instance->ISR & USART_ISR_PE)
//	//	{
//	//		__HAL_UART_CLEAR_FLAG(huart, USART_ISR_PE); // Clear the flag
//	//	}
//	//
//	//#define UART_FLAG_ORE                       USART_ISR_ORE           /*!< UART overrun error                        */
//	//#define UART_FLAG_NE                        USART_ISR_NE            /*!< UART noise error                          */
//	//#define UART_FLAG_FE                        USART_ISR_FE            /*!< UART frame error                          */
//	//#define UART_FLAG_PE                        USART_ISR_PE            /*!< UART parity error                         */
//}

void USART_IrqHandler(mcu_uart_handler_ctx* uart)
{
	if(uart)
	{
		UART_HandleTypeDef *huart = &uart->h_uart;
		if (huart->Instance->ISR & UART_FLAG_RXNE)  // A byte was received!
		{
			uint8_t data;
			__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_RXNE); // Clear the receive flag
			data = huart->Instance->RDR; // Read the byte from the register
			fifo_put8(&uart->fifo, data);  // Put the read byte into the fifo!
		}
		HAL_UART_IRQHandler(&uart->h_uart);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart)
{
//	((mcu_uart_handler_ctx*)huart)->err_cnt++;
	if (__HAL_UART_GET_FLAG(huart, UART_FLAG_PE) != RESET)
	{
		__HAL_UART_CLEAR_PEFLAG(huart); /* Clear PE flag */
	}
	else if (__HAL_UART_GET_FLAG(huart, UART_FLAG_FE) != RESET)
	{
		__HAL_UART_CLEAR_FEFLAG(huart); /* Clear FE flag */
	}
	else if (__HAL_UART_GET_FLAG(huart, UART_FLAG_NE) != RESET)
	{
		__HAL_UART_CLEAR_NEFLAG(huart); /* Clear NE flag */
	}
	else if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) != RESET)
	{
		__HAL_UART_CLEAR_OREFLAG(huart); /* Clear ORE flag */
	}
}

void USART1_IRQHandler(void)
{
	USART_IrqHandler(mcu_uart_handler_hash[1]);
}

void USART2_IRQHandler(void)
{
	USART_IrqHandler(mcu_uart_handler_hash[2]);
}

void USART3_4_IRQHandler(void)
{
	USART_IrqHandler(mcu_uart_handler_hash[3]);
	USART_IrqHandler(mcu_uart_handler_hash[4]);
}

//void UART4_IRQHandler(void)
//{
//}

#endif // MCU_PERIPHERY_ENABLE_UART

#endif // #if MCU_TYPE == R5F563
