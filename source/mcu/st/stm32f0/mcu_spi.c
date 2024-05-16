// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_spi.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_STM32F0

#include "../stm32f0/mcu_internal.h"
#include <string.h>

#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0

//static void MX_SPI1_Init(void);

uint8_t mcu_spi_divider[4] = {1, 2, 4, 8};

uint8_t mcu_spi_uart_divider[4] = {1, 4, 16, 64};

uint8_t mcu_current_spi_handler = 0;
mcu_spi_interface_handler_ctx mcu_spi_interface_handler[MCU_SPI_TOTAL_COUNT];
mcu_spi_handler_ctx mcu_spi_handler[MCU_PERIPHERY_DEVICE_COUNT_SPI];

//SPI_HandleTypeDef hspi1;

#define MCU_SPI_INIT_HANDLER(h, n)												\
				{																\
						h->num = n;												\
						h->clock = 0;											\
						h->is_uart = false;										\
						h->hspi.Instance = SPI ## n;							\
						__HAL_RCC_SPI ## n ## _CLK_ENABLE();					\
				}



mcu_spi_t mcu_spi_init(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx, MCU_IO_PIN clk, MCU_IO_PIN cs)
{	
	GPIO_InitTypeDef mosi = {0};
	GPIO_InitTypeDef miso = {0};
	GPIO_InitTypeDef sck = {0};
//	GPIO_InitTypeDef nss = {0};
	mcu_spi_interface_handler_ctx* handle;
	mcu_spi_handler_ctx* h_spi;

	if(mcu_current_spi_handler >= MCU_PERIPHERY_DEVICE_COUNT_SPI)
		MCU_RETURN_ERROR(MCU_ERROR_SPI_NOT_AVAILABLE, NULL)

	if(mcu_current_spi_handler == 0)
		memset(&mcu_spi_interface_handler[0], 0, sizeof(mcu_spi_interface_handler));

	h_spi = &mcu_spi_handler[mcu_current_spi_handler];

	if(num >= MCU_SPI_TOTAL_COUNT)
		MCU_RETURN_ERROR(MCU_ERROR_SPI_NOT_AVAILABLE, NULL)

	handle = &mcu_spi_interface_handler[num];

	// If the interface was already initialized -> check if uses the same pins as the one that should be used
	if(handle->initialized)
	{
		if(!(tx == handle->tx && rx == handle->rx && clk == handle->clk))
			MCU_RETURN_ERROR(MCU_ERROR_SPI_INVALID, NULL);
	}
	else
	{

		handle->initialized = true;
		handle->tx = tx;
		handle->rx = rx;
		handle->clk = clk;

		if(num == 1)
		{
			if(!(tx == PA_7 || tx == PB_5 || tx == PE_15))
				MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);
			if(!(rx == PA_6 || rx == PB_4 || rx == PE_14))
				MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);
			if(!(clk == PA_5 || clk == PB_3 || clk == PE_13))
				MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);

			MCU_SPI_INIT_HANDLER(handle, 1);

			if(tx == PE_15)
				mosi.Alternate = GPIO_AF1_SPI1;
			else
				mosi.Alternate = GPIO_AF0_SPI1;
			if(rx == PE_14)
				miso.Alternate = GPIO_AF1_SPI1;
			else
				miso.Alternate = GPIO_AF0_SPI1;
			if(clk == PE_13)
				sck.Alternate = GPIO_AF1_SPI1;
			else
				sck.Alternate = GPIO_AF0_SPI1;
		}
		else if(num == 2)
		{
			if(!(tx == PB_15 || tx == PC_3 || tx == PD_4))
				MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);
			if(!(rx == PB_14 || rx == PC_2 || rx == PD_3))
				MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);
			if(!(clk == PB_10 || clk == PB_13 || clk == PD_1))
				MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);

			MCU_SPI_INIT_HANDLER(handle, 2);

			if(tx == PB_15)
				mosi.Alternate = GPIO_AF0_SPI2;
			else
				mosi.Alternate = GPIO_AF1_SPI2;
			if(tx == PB_14)
				miso.Alternate = GPIO_AF0_SPI2;
			else
				miso.Alternate = GPIO_AF1_SPI2;
			if(tx == PB_10)
				sck.Alternate = GPIO_AF5_SPI2;
			else if(tx == PB_13)
				sck.Alternate = GPIO_AF0_SPI2;
			else
				sck.Alternate = GPIO_AF1_SPI2;
		}
	}

	mosi.Pin = (1 << (tx & 0xFF));
	mosi.Mode = GPIO_MODE_AF_PP;
	mosi.Pull = GPIO_NOPULL;
	mosi.Speed = GPIO_SPEED_FREQ_HIGH;

	miso.Pin = (1 << (rx & 0xFF));
	miso.Mode = GPIO_MODE_AF_PP;
	miso.Pull = GPIO_PULLUP;
	miso.Speed = GPIO_SPEED_FREQ_HIGH;

	sck.Pin = (1 << (clk & 0xFF));
	sck.Mode = GPIO_MODE_AF_PP;
	sck.Pull = GPIO_NOPULL;
	sck.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(mcu_internal_get_port(tx), &mosi);
	HAL_GPIO_Init(mcu_internal_get_port(rx), &miso);
	HAL_GPIO_Init(mcu_internal_get_port(clk), &sck);

	handle->hspi.Init.Mode = SPI_MODE_MASTER;
	handle->hspi.Init.Direction = SPI_DIRECTION_2LINES;
	handle->hspi.Init.DataSize = SPI_DATASIZE_8BIT;
	handle->hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
	handle->hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
	handle->hspi.Init.NSS = SPI_NSS_HARD_OUTPUT;
	handle->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	handle->hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
	handle->hspi.Init.TIMode = SPI_TIMODE_DISABLE;
	handle->hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	handle->hspi.Init.CRCPolynomial = 7;
	handle->hspi.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	handle->hspi.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
	if (HAL_SPI_Init(&handle->hspi) != HAL_OK)
	{
		MCU_RETURN_ERROR(MCU_ERROR_UART_INVALID, NULL);
	}

	h_spi->spi = handle;
	h_spi->cs = cs;

	mcu_io_set_dir(cs, MCU_IO_DIR_OUT);
	mcu_io_set(cs, 1);
	
	mcu_current_spi_handler++;
	return (mcu_spi_t)h_spi;
}

MCU_RESULT mcu_spi_set_param(mcu_spi_t h, MCU_SPI_MODE mode, uint32_t frq)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_SPI_INVALID, MCU_ERROR_SPI_INVALID);

	mcu_spi_interface_handler_ctx* handle = ((mcu_spi_handler_ctx*)h)->spi;
	
	switch(mode)
	{
	case MCU_SPI_MODE_0:
		handle->hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
		handle->hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
		break;
	case MCU_SPI_MODE_1:
		handle->hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
		handle->hspi.Init.CLKPhase = SPI_PHASE_2EDGE;
		break;
	case MCU_SPI_MODE_2:
		handle->hspi.Init.CLKPolarity = SPI_POLARITY_HIGH;
		handle->hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
		break;
	case MCU_SPI_MODE_3:
		handle->hspi.Init.CLKPolarity = SPI_POLARITY_HIGH;
		handle->hspi.Init.CLKPhase = SPI_PHASE_2EDGE;
		break;
	}

	return mcu_spi_set_clock(h, frq);
}

MCU_RESULT mcu_spi_set_clock(mcu_spi_t h, uint32_t frq)
{
	static const uint32_t div[8] = {2, 4, 8, 16, 32, 64, 128, 256};
	static const uint32_t div_value[8] = {
			SPI_BAUDRATEPRESCALER_2, SPI_BAUDRATEPRESCALER_4, SPI_BAUDRATEPRESCALER_8, SPI_BAUDRATEPRESCALER_16,
			SPI_BAUDRATEPRESCALER_32, SPI_BAUDRATEPRESCALER_64, SPI_BAUDRATEPRESCALER_128, SPI_BAUDRATEPRESCALER_256};
	uint8_t i = 0;
//	uint32_t div_selected = 256;
	mcu_spi_interface_handler_ctx* handle = ((mcu_spi_handler_ctx*)h)->spi;

	if(h==NULL)			
		MCU_RETURN_ERROR(MCU_ERROR_SPI_INVALID, MCU_ERROR_SPI_INVALID);

	handle->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	for(i = 0; i < 8; i++)
	{
		if(frq >= (mcu_get_frq_peripheral() / div[i]))
		{
			handle->hspi.Init.BaudRatePrescaler = div_value[i];
//			div_selected = div[i];
			break;
		}
	}
//	DBG_INFO("FRQ=%d\n", mcu_get_frq_peripheral() / div_selected);

	HAL_SPI_Init(&((mcu_spi_handler_ctx*)h)->spi->hspi);

	return MCU_OK;
}

void mcu_spi_set_chip_select(mcu_spi_t h, uint8_t state)
{
	if(h==NULL) return;
//	comm_printf(COMM_DEBUG, "CS[%08x / %08x] %02x\n", h, ((mcu_spi_handler_ctx*)h)->spi, state);
	mcu_io_set(((mcu_spi_handler_ctx*)h)->cs, state);
}

uint8_t mcu_spi_send_with_cs(mcu_spi_t h, uint8_t letter)
{
	mcu_io_set(((mcu_spi_handler_ctx*)h)->cs, 0);
	mcu_spi_send(h, letter);
	mcu_io_set(((mcu_spi_handler_ctx*)h)->cs, 1);
	return letter;
}

uint8_t mcu_spi_send(mcu_spi_t h, uint8_t letter)
{
	uint8_t spi_read = 0;

	if(h==NULL)
		return 0;

	HAL_SPI_TransmitReceive(&((mcu_spi_handler_ctx*)h)->spi->hspi, &letter, &spi_read, 1, 1000);
//	comm_printf(COMM_DEBUG, "TXRX[%08x / %08x] %02x / %02x\n", h, ((mcu_spi_handler_ctx*)h)->spi, letter, spi_read);

	return spi_read;
}

void mcu_spi_send_buffer(mcu_spi_t h, uint8_t* w_buf, uint8_t* r_buf, uint32_t len)
{
    if(h==NULL || (w_buf == NULL && r_buf == NULL) || len == 0)
    {
        return;
    }

	HAL_StatusTypeDef ret;
	if(w_buf == NULL)
		ret = HAL_SPI_Receive(&((mcu_spi_handler_ctx*)h)->spi->hspi, r_buf, len, 1000);
	else if(r_buf == NULL)
		ret = HAL_SPI_Transmit(&((mcu_spi_handler_ctx*)h)->spi->hspi, w_buf, len, 1000);
	else
		ret = HAL_SPI_TransmitReceive(&((mcu_spi_handler_ctx*)h)->spi->hspi, w_buf, r_buf, len, 1000);

	if(ret != HAL_OK)
		DBG_ERROR(" [HAL ERROR %d]\n", ret);
}

uint32_t mcu_spi_get_frq(mcu_spi_t h)
{
	if(h == NULL)
		return 0;

	return ((mcu_spi_handler_ctx*)h)->spi->clock;
}


///**
//  * @brief SPI1 Initialization Function
//  * @param None
//  * @retval None
//  */
//static void MX_SPI1_Init(void)
//{
//
//  /* USER CODE BEGIN SPI1_Init 0 */
//
//  /* USER CODE END SPI1_Init 0 */
//
//  /* USER CODE BEGIN SPI1_Init 1 */
//
//  /* USER CODE END SPI1_Init 1 */
//  /* SPI1 parameter configuration*/
//  hspi1.Instance = SPI1;
//  hspi1.Init.Mode = SPI_MODE_MASTER;
//  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
//  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
//  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
//  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
//  hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
//  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
//  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
//  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
//  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//  hspi1.Init.CRCPolynomial = 7;
//  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
//  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
//  if (HAL_SPI_Init(&hspi1) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /* USER CODE BEGIN SPI1_Init 2 */
//
//  /* USER CODE END SPI1_Init 2 */
//
//}


/**
* @brief SPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
//void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
//{
//  GPIO_InitTypeDef GPIO_InitStruct = {0};
//  if(hspi->Instance==SPI1)
//  {
//  /* USER CODE BEGIN SPI1_MspInit 0 */
//
//  /* USER CODE END SPI1_MspInit 0 */
//    /* Peripheral clock enable */
//    __HAL_RCC_SPI1_CLK_ENABLE();
//
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    /**SPI1 GPIO Configuration
//    PA4     ------> SPI1_NSS
//    PA5     ------> SPI1_SCK
//    PA6     ------> SPI1_MISO
//    PA7     ------> SPI1_MOSI
//    */
//    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//  /* USER CODE BEGIN SPI1_MspInit 1 */
//
//  /* USER CODE END SPI1_MspInit 1 */
//  }
//
//}
//
///**
//* @brief SPI MSP De-Initialization
//* This function freeze the hardware resources used in this example
//* @param hspi: SPI handle pointer
//* @retval None
//*/
//void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
//{
//  if(hspi->Instance==SPI1)
//  {
//  /* USER CODE BEGIN SPI1_MspDeInit 0 */
//
//  /* USER CODE END SPI1_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_SPI1_CLK_DISABLE();
//
//    /**SPI1 GPIO Configuration
//    PA4     ------> SPI1_NSS
//    PA5     ------> SPI1_SCK
//    PA6     ------> SPI1_MISO
//    PA7     ------> SPI1_MOSI
//    */
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
//
//  /* USER CODE BEGIN SPI1_MspDeInit 1 */
//
//  /* USER CODE END SPI1_MspDeInit 1 */
//  }
//
//}

#endif	// MCU_PERIPHERY_ENABLE_SPI

#endif // #if MCU_TYPE == R5F563
