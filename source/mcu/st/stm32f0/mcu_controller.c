// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_controller.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_STM32F0

#include "../stm32f0/mcu_internal.h"
#include "stm32f0xx_hal_tim.h"
//#include "stm32f0xx_hal.h"
//#include "stm32f3xx_hal.h"
//#include "stm32f071xb.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define _NUM_PLL_MULTIPLIER				16

#define _NUM_HCLK_DIVIDER				5

#define _NUM_SYSCLK_DIVIDER				10

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

MCU_RESULT mcu_last_error = MCU_OK;

uint32_t mcu_frq_ext_hz = 0;
uint32_t mcu_frq_cpu_hz = 0;
uint32_t mcu_frq_peripheral_hz = 0;
uint32_t mcu_frq_flash_hz = 0;

#if MCU_PERIPHERY_ENABLE_WATCHDOG
/// Task for watchdog
system_task_t _task;

static IWDG_HandleTypeDef _hiwdg;
#endif

#if MCU_PERIPHERY_ENABLE_WAIT_TIMER
static TIM_HandleTypeDef _htim_wait;
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void MX_DMA_Init(void);

static void _remap_table(void);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
uint32_t mcu_get_frq_external(void){ 			return mcu_frq_ext_hz; }
uint32_t mcu_get_frq_cpu(void){ 				return mcu_frq_cpu_hz; }
uint32_t mcu_get_frq_peripheral(void){ 			return mcu_frq_peripheral_hz; }

void mcu_init_max_internal()
{
	_remap_table();
	mcu_init(0, 200000000UL, 100000000UL);
}

void mcu_init_max_external(uint32_t frq_ext)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	_remap_table();

	//	/// Reset of all peripherals, Initializes the Flash interface and the Systick.
	HAL_Init();

	/** Configure the main internal regulator output voltage
	*/
	__HAL_RCC_PWR_CLK_ENABLE();

	/** Configure LSE Drive Capability
	*/
	HAL_PWR_EnableBkUpAccess();
#if MCU_PERIPHERY_ENABLE_EXT_QUARTZ
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
#endif
//	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	//	__HAL_RCC_BACKUPRESET_FORCE();
	//	__HAL_RCC_BACKUPRESET_RELEASE();

	  /** Initializes the CPU, AHB and APB busses clocks
	  */
#if MCU_PERIPHERY_ENABLE_EXT_QUARTZ
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14|RCC_OSCILLATORTYPE_LSE
	                              |RCC_OSCILLATORTYPE_HSE;
#else
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14|RCC_OSCILLATORTYPE_LSI
	                              |RCC_OSCILLATORTYPE_HSE;
#endif
#if MCU_PERIPHERY_ENABLE_WATCHDOG
	  RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_LSI; // Independent watchdog depends on LSI (pun intended).
#endif
	  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
	  RCC_OscInitStruct.HSI14CalibrationValue = 16;
	  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
#if MCU_PERIPHERY_ENABLE_EXT_QUARTZ
	  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
#endif
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
	  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
	  HAL_RCC_OscConfig(&RCC_OscInitStruct);
	  /** Initializes the CPU, AHB and APB busses clocks
	  */
	  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1;
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);
	  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
	                              |RCC_PERIPHCLK_RTC;
	  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
	  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
#if MCU_PERIPHERY_ENABLE_EXT_QUARTZ
	  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
#else
	  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
#endif
	  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	mcu_frq_cpu_hz = 48000000;
	mcu_frq_peripheral_hz = 48000000;

	mcu_init(frq_ext, 48000000UL, 48000000UL);	// ICLK Max 48MHz and PCLK Max 48 MHz
}

void mcu_init(uint32_t frq_ext, uint32_t frq_cpu, uint32_t frq_peripheral)
{
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();

	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_RCC_SYSCFG_CLK_ENABLE();

	MX_DMA_Init();

#if MCU_PERIPHERY_ENABLE_WATCHDOG
	_hiwdg.Instance = IWDG;
	  _hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
	  _hiwdg.Init.Window = 4095;
	  _hiwdg.Init.Reload = 4095;
	  if (HAL_IWDG_Init(&_hiwdg) != HAL_OK)
	  {
		  // Error Handler?
	  }

	system_init_void_task(&_task, true, mcu_watchdog_trigger);
	mcu_watchdog_trigger();
#endif

#if MCU_PERIPHERY_ENABLE_WAIT_TIMER
	_htim_wait.Instance = TIM1;
	_htim_wait.Init.Prescaler = 48-1;
	_htim_wait.Init.CounterMode = TIM_COUNTERMODE_UP;
	_htim_wait.Init.Period = 65535-1;
	_htim_wait.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	_htim_wait.Init.RepetitionCounter = 0;
	_htim_wait.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&_htim_wait);

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&_htim_wait, &sClockSourceConfig);

	TIM_MasterConfigTypeDef sMasterConfig = {0};
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&_htim_wait, &sMasterConfig);

    __HAL_RCC_TIM1_CLK_ENABLE();

	HAL_TIM_Base_Start(&_htim_wait);
#endif
}


void mcu_enable_interrupt(void)
{
	__enable_irq();
}

void mcu_disable_interrupt(void)
{
	__disable_irq();
}

void mcu_soft_reset(void)
{
	NVIC_SystemReset();
}

uint32_t mcu_enter_sleep_mode(uint32_t sleep_time)
{
	return 0;
}

void mcu_io_set_port_dir(MCU_IO_PIN p, uint8_t d)
{
	GPIO_TypeDef* port = mcu_internal_get_port(p);
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_All;
	GPIO_InitStruct.Mode = d == MCU_IO_DIR_OUT ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void mcu_io_set_dir(MCU_IO_PIN p, MCU_IO_DIRECTION d)
{
	GPIO_TypeDef* port = mcu_internal_get_port(p);
	uint16_t pin_num = (uint8_t)(p & 0x00FF);

//	GPIO_InitTypeDef GPIO_InitStruct = {0};
//	GPIO_InitStruct.Pin = (1 << pin_num);
//	GPIO_InitStruct.Mode = d == MCU_IO_DIR_OUT ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//	HAL_GPIO_Init(port, &GPIO_InitStruct);
//	return;


	if(port)
	{
		uint32_t temp;
		// Set pin direction
		temp = port->MODER;
		temp &= ~(GPIO_MODER_MODER0 << (pin_num * 2));
		if(d == MCU_IO_DIR_OUT)
			temp |= (GPIO_MODE_OUTPUT_PP << (pin_num * 2));
		port->MODER = temp;

		if(d == MCU_IO_DIR_OUT)
		{
			// Set a high speed at default...
			temp = port->OSPEEDR;
			temp &= ~(GPIO_OSPEEDER_OSPEEDR0 << (pin_num * 2));
			temp |= (GPIO_SPEED_FREQ_HIGH << (pin_num * 2));
			port->OSPEEDR = temp;

			// Clear pin alternate function
			temp = port->OTYPER;
			temp &= ~(GPIO_OTYPER_OT_0 << pin_num) ;
			port->OTYPER = temp;
		}
	}
}

void mcu_io_set_pullup(MCU_IO_PIN p, bool pullup_active)
{
	uint16_t pin_num = (uint8_t)(p & 0x00FF);
	GPIO_TypeDef* port = mcu_internal_get_port(p);

	if(port)
	{
		uint32_t temp = port->PUPDR;
	    temp &= ~(GPIO_PUPDR_PUPDR0 << (pin_num *2)); // Clear Pull-up register
	    if(pullup_active)
	    	temp |= (1 << (pin_num * 2));
	    port->PUPDR = temp;

	}
}

void mcu_io_set(MCU_IO_PIN p, uint8_t d)
{
	uint16_t port_val = (uint8_t)(p & 0x00FF);
	uint16_t gpio = GPIO_PIN_All;
	GPIO_TypeDef* port = mcu_internal_get_port(p);

	if(port)
	{
//		HAL_GPIO_WritePin(port, (1 << port_val), d ? GPIO_PIN_SET : GPIO_PIN_RESET);
		if(port_val < 0x10)				// Single pin
			gpio = (1 << port_val);

		if(d)
			port->BSRR = gpio;
		else
			port->BRR = gpio;
	}
}

void mcu_io_toggle(MCU_IO_PIN p)
{
	uint16_t port_val = (uint8_t)(p & 0x00FF);
	uint16_t gpio = GPIO_PIN_All;
	GPIO_TypeDef* port = mcu_internal_get_port(p);

	if(port)
	{
		if(port_val < 0x10)				// Single pin
			gpio = (1 << port_val);

		if ((port->ODR & gpio) == gpio)
			port->BRR = gpio;
		else
			port->BSRR = gpio;
	}
}

uint8_t mcu_io_get(MCU_IO_PIN p)
{
	uint16_t port_val = (uint8_t)(p & 0x00FF);
	uint16_t mode = (1 << port_val * 2);
	uint16_t gpio = GPIO_PIN_All;
	GPIO_TypeDef* port = mcu_internal_get_port(p);

	if(port)
	{
		if(port_val < 0x10)				// Single pin
		{
			gpio = (1 << port_val);

			if(port->MODER & mode) // Is output
				return (port->ODR & gpio) != 0; // Return output data register
			else
				return (port->IDR & gpio) != 0; // Return input data register
		}
		// Complete port
		return port->IDR & gpio;
	}

	return 0;
}

#if MCU_PERIPHERY_ENABLE_WATCHDOG
void mcu_watchdog_init(void (*f)(void))
{

	mcu_watchdog_trigger();	// Start with first Trigger
}

void mcu_watchdog_trigger(void)
{
	HAL_IWDG_Refresh(&_hiwdg);
}
#endif

GPIO_TypeDef* mcu_internal_get_port(MCU_IO_PIN port)
{
	port |= 0xFF;

	if(port == PA)
		return GPIOA;
	else if(port == PB)
		return GPIOB;
	else if(port == PC)
		return GPIOC;
	else if(port == PD)
		return GPIOD;
	else if(port == PE)
		return GPIOE;

	return NULL;
}

#if MCU_PERIPHERY_ENABLE_WAIT_TIMER
void mcu_wait_us(uint16_t delay)
{
	__HAL_TIM_SET_COUNTER(&_htim_wait, 0);
//	HAL_TIM_Base_Start(&_htim_wait);
	while (__HAL_TIM_GET_COUNTER(&_htim_wait) < delay);
//	HAL_TIM_Base_Stop(&_htim_wait);
}

bool mcu_wait_us_until(uint16_t wait_max, bool(*f)(void*), void* obj)
{
	__HAL_TIM_SET_COUNTER(&_htim_wait, 0);
//	HAL_TIM_Base_Start(&_htim_wait);
	while (__HAL_TIM_GET_COUNTER(&_htim_wait) < wait_max)
	{
		if(f && f(obj))
		{
//			HAL_TIM_Base_Stop(&_htim_wait);
			return true;
		}
	}
//	HAL_TIM_Base_Stop(&_htim_wait);
	return false;
}

void mcu_wait_ms(uint16_t delay)
{
	volatile uint32_t timestamp = system_get_tick_count();

	while( (system_get_tick_count() - timestamp) < delay);
}

bool mcu_wait_ms_until(uint16_t wait_max, bool(*f)(void*), void* obj)
{
	volatile uint32_t timestamp = system_get_tick_count();

	while( (system_get_tick_count() - timestamp) < wait_max)
	{
		if(f && f(obj))
		{
			return true;
		}
	}
	return false;
}
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _remap_table(void)
{
    // Copy interrupt vector table to the RAM.
    volatile uint32_t *VectorTable = (volatile uint32_t *)0x20000000;
    uint32_t ui32_VectorIndex = 0;

    for(ui32_VectorIndex = 0; ui32_VectorIndex < 48; ui32_VectorIndex++)
    {
        VectorTable[ui32_VectorIndex] = *(volatile uint32_t*)((uint32_t)0x08008000 + (ui32_VectorIndex << 2));
    }

    __HAL_RCC_AHB_FORCE_RESET();

    //  Enable SYSCFG peripheral clock
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    __HAL_RCC_AHB_RELEASE_RESET();

    // Remap RAM into 0x0000 0000
    __HAL_SYSCFG_REMAPMEMORY_SRAM();
}

/**
* Initializes the Global MSP.
*/
void HAL_MspInit(void)
{
/* USER CODE BEGIN MspInit 0 */

/* USER CODE END MspInit 0 */


/* System interrupt init*/

/* USER CODE BEGIN MspInit 1 */

/* USER CODE END MspInit 1 */
}
/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

}

#endif // #if MCU_TYPE
