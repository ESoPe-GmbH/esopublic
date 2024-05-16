// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_timer.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_STM32F0

#include "../stm32f0/mcu_internal.h"
//#include "interrupt_handlers.h"
#include <string.h>

#if MCU_PERIPHERY_DEVICE_COUNT_TIMER > 0

//// CMTU0_CMT0
//#pragma interrupt (Excep_CMT0_CMI0(vect=28))
//void Excep_CMT0_CMI0(void);
//
//// CMTU1_CMT1
//#pragma interrupt (Excep_CMT1_CMI1(vect=29))
//void Excep_CMT1_CMI1(void);
//
//// CMTU2_CMT2
//#pragma interrupt (Excep_CMT2_CMI2(vect=30))
//void Excep_CMT2_CMI2(void);
//
//// CMTU3_CMT3
//#pragma interrupt (Excep_CMT3_CMI3(vect=31))
//void Excep_CMT3_CMI3(void);

static const uint32_t _divider_cmt[4] = {8, 32, 128, 512};

//static const uint32_t _divider_mtu[4] = {1, 4, 16, 64};

#define MCU_TIMER_INIT_HANDLER(h, n, mtu)										\
				{																\
						h->num = n;												\
						h->is_mtu = mtu;										\
						h->callback = f;										\
						h->obj = obj;											\
						h->frq = 0;												\
						mcu_timer_handler_hash[n] = h;							\
				}
				
mcu_timer_handler_ctx *mcu_timer_handler_hash[MCU_TIMER_TOTAL_COUNT];
static bool _timer_initialized = false;
uint8_t mcu_current_timer_handler = 0;
mcu_timer_handler_ctx mcu_timer_handler[MCU_PERIPHERY_DEVICE_COUNT_TIMER];

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);


mcu_timer_t mcu_timer_init(MCU_INT_LVL lvl, uint32_t frq_hz, void (*f)(void*), void* obj, bool auto_start)
{
	mcu_timer_handler_ctx* handle;

	if(!_timer_initialized)
	{
		_timer_initialized = true;
		memset(mcu_timer_handler_hash, 0, sizeof(mcu_timer_handler_hash));
	}

	if(mcu_current_timer_handler >= MCU_PERIPHERY_DEVICE_COUNT_TIMER)
		MCU_RETURN_ERROR(MCU_ERROR_TMR_NOT_AVAILABLE, NULL);

	handle = &mcu_timer_handler[mcu_current_timer_handler];	// Für einfachere Handhabung
//	switch(mcu_current_timer_handler)
//	{
//		case 0:	MCU_TIMER_SET_CMT(0);	MCU_INTERRUPT_SET_CMT(0); 			break;
//		case 1:	MCU_TIMER_SET_CMT(1);	MCU_INTERRUPT_SET_CMT(1); 			break;
//		case 2:	MCU_TIMER_SET_CMT(2);	MCU_INTERRUPT_SET_CMT_SW(128); 		break;
//		case 3:	MCU_TIMER_SET_CMT(3);	MCU_INTERRUPT_SET_CMT_SW(129); 		break;
////		case 2:
////			MCU_TIMER_SET_MTU(2);
////			_MTU_SET_TIOR(2);
////			break;
////		case 3:
////			MCU_TIMER_SET_MTU(3);
////			_MTU_SET_TIOR_HL(3);
////			break;
////		case 4:
////			MCU_TIMER_SET_MTU(4);
////			_MTU_SET_TIOR_HL(4);
////			break;
////		case 5:
////			MCU_TIMER_SET_MTU(0);
////			MTU0.TIER2.BYTE = 0;
////			_MTU_SET_TIOR_HL(0);
////			break;
////		case 6:
////			MCU_TIMER_SET_MTU(1);
////			_MTU_SET_TIOR(1);
////			break;
//		default: 				MCU_RETURN_ERROR(MCU_ERROR_TMR_NOT_AVAILABLE, NULL);
//	}

	if(MCU_OK != mcu_timer_set_frq(handle, frq_hz))
		return NULL;

	if(auto_start)	mcu_timer_start(handle);
	
	mcu_current_timer_handler++;

	return (mcu_timer_t)handle;
}

static void mcu_timer_set_start(mcu_timer_t h, uint8_t state)
{
	if(h==NULL)	return;
//	switch(((mcu_timer_handler_ctx*)h)->num)
//	{
//		case 0:	CMT.CMSTR0.BIT.STR0	= state;	break;
//		case 1:	CMT.CMSTR0.BIT.STR1	= state;	break;
//		case 2:	CMT.CMSTR1.BIT.STR2	= state;	break;
//		case 3:	CMT.CMSTR1.BIT.STR3	= state;	break;
//	}
}

void mcu_timer_start(mcu_timer_t h){	mcu_timer_set_start(h, 1);	}
void mcu_timer_stop(mcu_timer_t h){	mcu_timer_set_start(h, 0);	}

MCU_RESULT mcu_timer_set_frq(mcu_timer_t h, uint32_t frq_hz)
{
	uint8_t i, divider;
	uint32_t timer_value = 0xFFFFFFFF;
	if(h==NULL)
		return MCU_ERROR_TMR_NOT_AVAILABLE;

//	for(i=0; i<4; i++)
//	{
//		divider = i;	// Teiler einstellen
//		timer_value = mcu_frq_peripheral_hz / (frq_hz * _divider_cmt[divider]);	// Mit Teiler berechnen
//		if(timer_value < 0xFFFF)	// Prüfen obs passt...
//			break;	// Wert passt in das Register, also abbrechen!
//	}
//
//	if(timer_value > 0x10000)
//		MCU_RETURN_ERROR(MCU_ERROR_TMR_FRQ_INVALID, NULL);	// Da der Timer nur 16 Bit ist, darf der Wert nicht h�her sein! Beim einsetzen wird noch -1 gerechnet.
//
//	switch(((mcu_timer_handler_ctx*)h)->num)
//	{
//		case 0: MCU_TIMER_SET_FRQ(0); break;
//		case 1: MCU_TIMER_SET_FRQ(1); break;
//		case 2: MCU_TIMER_SET_FRQ(2); break;
//		case 3: MCU_TIMER_SET_FRQ(3); break;
//		return MCU_ERROR_TMR_NOT_AVAILABLE;
//	}
//
//	((mcu_timer_handler_ctx*)h)->frq = mcu_frq_peripheral_hz / ((timer_value + 1) * _divider_cmt[divider]);

	return MCU_OK;
}

uint32_t mcu_timer_get_frq(mcu_timer_t h)
{
	if(h==NULL)	return 0;
	return ((mcu_timer_handler_ctx*)h)->frq;
}
//
//void INT_Excep_CMT0_CMI0(void)
//{
//	MCU_TIMER_INTERRUPT_N(0);
//}
//void INT_Excep_CMT1_CMI1(void)
//{
//	MCU_TIMER_INTERRUPT_N(1);
//}
////void Excep_CMT2_CMI2(void)
//void INT_Excep_PERIB_INTB128(void)
//{
//	MCU_TIMER_INTERRUPT_N(2);
//}
////void Excep_CMT3_CMI3(void)
//void INT_Excep_PERIB_INTB129(void)
//{
//	MCU_TIMER_INTERRUPT_N(3);
//}


/**
* @brief TIM_PWM MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_pwm: TIM_PWM handle pointer
* @retval None
*/
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{
  if(htim_pwm->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspInit 0 */

  /* USER CODE END TIM3_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();
  /* USER CODE BEGIN TIM3_MspInit 1 */

  /* USER CODE END TIM3_MspInit 1 */
  }

}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
  if(htim->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspPostInit 0 */

  /* USER CODE END TIM3_MspPostInit 0 */

    __HAL_RCC_GPIOC_CLK_ENABLE();

  /* USER CODE BEGIN TIM3_MspPostInit 1 */

  /* USER CODE END TIM3_MspPostInit 1 */
  }

}
/**
* @brief TIM_PWM MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_pwm: TIM_PWM handle pointer
* @retval None
*/
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm)
{
  if(htim_pwm->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspDeInit 0 */

  /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();
  /* USER CODE BEGIN TIM3_MspDeInit 1 */

  /* USER CODE END TIM3_MspDeInit 1 */
  }

}

#endif

#endif // #if MCU_TYPE == R5F563
