// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/***
 * @file mcu_rtc.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_STM32F0 && MCU_PERIPHERY_ENABLE_RTC

#include "../../sys.h"
#include "module/rtc/rtc.h"
#include "../stm32f0/mcu_internal.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
#define PREDIV_A		0
#define PREDIV_S		(LSE_VALUE / (PREDIV_A + 1) - 1)

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MCU_DEBUG_ENABLE
static rtc_time_t mcu_rtc_time;

static uint32_t timestamp;
#endif

RTC_HandleTypeDef hrtc;

static bool _rtc_initialized = false;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

///**
// * Waits for the number of clock cycles of clock frequency in the parameter.
// *
// * @param clock_frq			Clock frequency of the clock
// * @param cycles			Number of clock cycles to wait.
// */
//static void mcu_rtc_wait_time(const uint32_t clock_frq, const uint32_t cycles);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void mcu_rtc_init(bool use_ext_subclock) // Parameter not used, define used
{
#if MCU_PERIPHERY_ENABLE_EXT_QUARTZ
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
#endif
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	HAL_StatusTypeDef ret;

	DBG_INFO("mcu_rtc_init(%d)\n", MCU_PERIPHERY_ENABLE_EXT_QUARTZ);

	/* USER CODE BEGIN RTC_Init 1 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_RCC_RTC_ENABLE();

//#if MCU_PERIPHERY_ENABLE_EXT_QUARTZ
//
////	if(!(*(volatile uint32_t *) (BDCR_RTCEN_BB)))
//
//	__HAL_RCC_BACKUPRESET_FORCE();
//	__HAL_RCC_BACKUPRESET_RELEASE();
//
//	/** Initializes the CPU, AHB and APB busses clocks
//	*/
//	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
//	RCC_OscInitStruct.LSEState = RCC_LSE_ON; // Enable external quartz
//	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE; // Do not change PLL
//
//	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
//
//	if (ret != HAL_OK)
//	{
//		DBG_ERROR("Error enabling LSE: %d\n", ret);
//	}
//	else
//#endif
	{
//#if MCU_PERIPHERY_ENABLE_EXT_QUARTZ
//		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
//		PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
//		ret = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
//		if (ret != HAL_OK)
//		{
//			// TODO: Error Handling?
//			DBG_ERROR("Error enabling LSE: %d\n", ret);
//		}
//		else
//#endif
		{
			/* USER CODE END RTC_Init 1 */
			/** Initialize RTC Only
			*/
			hrtc.Instance = RTC;
			hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
#if MCU_PERIPHERY_ENABLE_EXT_QUARTZ
			hrtc.Init.AsynchPrediv = PREDIV_A;
#else
			hrtc.Init.AsynchPrediv = 120;
#endif
			hrtc.Init.SynchPrediv = PREDIV_S;
			hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
			hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
			hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
			ret = HAL_RTC_Init(&hrtc);
			if (ret != HAL_OK)
			{
				// TODO: Error
				DBG_ERROR("Error initializing RTC: %d\n", ret);
			}
			else
			{
			    if (HAL_RTCEx_EnableBypassShadow(&hrtc) != HAL_OK) {
			    	DBG_ERROR("EnableBypassShadow error\n");
			    }
				_rtc_initialized = true;

//				/** Initialize RTC and set the Time and Date
//				*/
//				sTime.Hours = 0x0;
//				sTime.Minutes = 0x0;
//				sTime.Seconds = 0x0;
//				sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
//				sTime.StoreOperation = RTC_STOREOPERATION_RESET;
//				if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
//				{
//					// TODO: Error
//					DBG_ERROR("Error setting time\n");
//				}
//				sDate.WeekDay = RTC_WEEKDAY_MONDAY;
//				sDate.Month = RTC_MONTH_JANUARY;
//				sDate.Date = 0x1;
//				sDate.Year = 0x0;
//				if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
//				{
//					// TODO: Error
//					DBG_ERROR("Error setting date\n");
//				}

			#if MCU_DEBUG_ENABLE
				dbg_printf(DBG_STRING, "RTC clock started: %dms\n", (system_get_tick_count() - timestamp));
			#endif

				DBG_INFO("RTC State: %d\n", HAL_RTC_GetState(&hrtc));
			}
		}
	}
}

void mcu_rtc_set_time(rtc_time_t *time)
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

	if(time == NULL || !_rtc_initialized)
		return;

	/** Initialize RTC and set the Time and Date
	*/
	sTime.Hours = time->tm_hour;
	sTime.Minutes = time->tm_min;
	sTime.Seconds = time->tm_sec;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_SET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		DBG_ERROR("Error setting time\n");
		// TODO: Error
	}
	if(time->tm_msec != 0)
	{
		if(HAL_RTCEx_SetSynchroShift(&hrtc, RTC_SHIFTADD1S_SET, ((uint32_t)RTC->SSR - (time->tm_msec * (hrtc.Init.SynchPrediv+1) / 1000))&0x7FFF) != HAL_OK)
			DBG_ERROR("Error setting subseconds\n");
	}

	// Fill RTC structures
	if (time->tm_wday == 0) { /* Sunday specific case */
		sDate.WeekDay    = RTC_WEEKDAY_SUNDAY;
	} else {
		sDate.WeekDay    = time->tm_wday;
	}
//	sDate.WeekDay = time->tm_wday + 1;
	sDate.Month = time->tm_mon + 1;
	sDate.Date = time->tm_mday;
	sDate.Year = time->tm_year - 68;
	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		DBG_ERROR("Error setting date\n");
		// TODO: Error
	}

}

void mcu_rtc_get_time(rtc_time_t *time)
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

	if(time == NULL || !_rtc_initialized)
		return;

	/* Since the shadow registers are bypassed we have to read the time twice and compare them until both times are the same */
	uint32_t Read_time = RTC->TR & RTC_TR_RESERVED_MASK;
	uint32_t Read_date = RTC->DR & RTC_DR_RESERVED_MASK;
	uint32_t Read_SubSeconds = (uint32_t)(RTC->SSR);

	while ((Read_time != (RTC->TR & RTC_TR_RESERVED_MASK)) || (Read_date != (RTC->DR & RTC_DR_RESERVED_MASK)) || (Read_SubSeconds != (RTC->SSR))) {
		Read_time = RTC->TR & RTC_TR_RESERVED_MASK;
		Read_date = RTC->DR & RTC_DR_RESERVED_MASK;
		Read_SubSeconds = (uint32_t)(RTC->SSR);
	}

	time->tm_mday = RTC_Bcd2ToByte((uint8_t)(Read_date & (RTC_DR_DT | RTC_DR_DU)));
	time->tm_mon  = RTC_Bcd2ToByte((uint8_t)((Read_date & (RTC_DR_MT | RTC_DR_MU))  >> 8)) - 1;
	time->tm_year = RTC_Bcd2ToByte((uint8_t)((Read_date & (RTC_DR_YT | RTC_DR_YU))  >> 16)) + 68;
	time->tm_hour = RTC_Bcd2ToByte((uint8_t)((Read_time & (RTC_TR_HT  | RTC_TR_HU))  >> 16));
	time->tm_min  = RTC_Bcd2ToByte((uint8_t)((Read_time & (RTC_TR_MNT | RTC_TR_MNU)) >> 8));
	time->tm_sec  = RTC_Bcd2ToByte((uint8_t)((Read_time & (RTC_TR_ST  | RTC_TR_SU))  >> 0));

	if(Read_SubSeconds > PREDIV_S)
	{
		time->tm_sec -= 1;
		Read_SubSeconds -= PREDIV_S;
	}

	time->tm_msec = (PREDIV_S - Read_SubSeconds) * 1000 / (PREDIV_S + 1);

//	if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) == HAL_OK && HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) == HAL_OK)
//	{
//		time->tm_hour = sTime.Hours;
//		time->tm_min = sTime.Minutes;
//		time->tm_sec = sTime.Seconds;
//		time->tm_wday = sDate.WeekDay - 1;
//		time->tm_mday = sDate.Date;
//		time->tm_mon = sDate.Month - 1;
//		time->tm_year = sDate.Year;
//		time->tm_msec = (sTime.SecondFraction - sTime.SubSeconds) * 1000 / (sTime.SecondFraction + 1);
//	}
//	else
//		DBG_ERROR("Error getting time\n");
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
//static void _set_subseconds(int32_t milliseconds_offset)
//{
//	if(milliseconds_offset == 0)
//		return;
//
//	uint32_t add = RTC_SHIFTADD1S_SET;
//	uint32_t subseconds;
//	//SSR Should be 0x7FFF since it was reset by RTC_Set_Time
//	subseconds = ((uint32_t)RTC->SSR - (milliseconds_offset * (hrtc.Init.SynchPrediv+1) / 1000) & 0x7FFF);
//	DBG_VERBOSE("Shift register %04X\n", subseconds);
//
//	if(HAL_RTCEx_SetSynchroShift(&hrtc, RTC_SHIFTADD1S_RESET, ((uint32_t)RTC->SSR - (milliseconds_offset * (hrtc.Init.SynchPrediv+1) / 1000))&0x7FFF) != HAL_OK)
//		DBG_ERROR("Error setting milliseconds_offset\n");
//}
/**
 * @brief RTC MSP Initialization
* This function configures the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    /* Peripheral clock enable */
//    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */
//	DBG_INFO("RTC Enable\n");

  /* USER CODE END RTC_MspInit 1 */
  }

}

/**
* @brief RTC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{
  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }

}

#endif
