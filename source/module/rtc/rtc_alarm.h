/**
 * 	@file 		rtc.h
 * 	@copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten
 *  @author 	Tim Koczwara
 *
 *  @brief		Has alarm functions for the RTC.
 *              You can set an alarm using a contex structure.
 *              Uses a background task that triggers the next alarm one the time for the alarm is reached.
 *              When using a mcu that supports rtc alarm, this will be used as a trigger instead of the background task (Current implementation only uses background task!).
 *
 *  @version	1.00 (01.02.2023, Tim Koczwara)
 *              - Initial implementation
 *
 *
 ******************************************************************************/
 
#ifndef RTC_ALARM_HEADER_FIRST_INCLUDE_GUARD
#define RTC_ALARM_HEADER_FIRST_INCLUDE_GUARD

#include "module_public.h"

#if MODULE_ENABLE_RTC

#include "module/enum/function_return.h"
#include "module/rtc/rtc.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

// Activate selftest for unittest
#if TEST_RTC_RTC
#undef RTC_ALARM_SELFTEST
#define RTC_ALARM_SELFTEST      1
#endif

#ifndef RTC_ALARM_SELFTEST
#define RTC_ALARM_SELFTEST      0
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// RTC alarm handle
typedef struct rtc_alarm_s* rtc_alarm_handle_t;
/**
 * @brief Configuration for the alarm.
 * `alarm_time` and `f` are mandatory to set. `user` is optional.
 */
typedef struct rtc_alarm_config_s rtc_alarm_config_t;

/// RTC alarm callback
/// @param alarm    Pointer to the alarm handle that was triggered.
/// @param config   Pointer to the configuration that was used when the alarm was set-up.
typedef void(*rtc_alarm_cb_t)(const rtc_alarm_handle_t alarm, const rtc_alarm_config_t* config);

/**
 * @brief Configuration for the alarm.
 * `alarm_time` and `f` are mandatory to set. `user` is optional.
 */
struct rtc_alarm_config_s
{
    /// Time at which the alarm shall be triggered.
    rtc_time_t alarm_time;
    /// User defined pointer.
    void* user;
    /// Function to call when the alarm is triggered.
    /// The function is always called from a protothread and never from the mcu interrupt directly.
    rtc_alarm_cb_t f;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Create an alarm handle. Does not add the alarm.
 * 
 * @return      Alarm handle or NULL if failed to create handle.
 */
rtc_alarm_handle_t rtc_alarm_create(void);
/**
 * @brief Free an alarm handle. If this is currently set-up for alarm, the alarm is stopped.
 * 
 * @param alarm     Handle as created using `rtc_alarm_create`.
 */
void rtc_alarm_free(rtc_alarm_handle_t alarm);
/**
 * @brief Setup an alarm that should trigger when the time is reached.
 * Only one alarm can be setup with a single handle. To use multiple alarms, you need multiple handles.
 * To change the alarm that was set-up, you need to call `rtc_alarm_stop` and then call `rtc_alarm_setup` to configure the new alarm.
 * The alarm is automatically stopped once the callback was triggered. Therefore the callback can be used to setup a following timer.
 * 
 * @param alarm     Handle as created using `rtc_alarm_create`.
 * @param config    Pointer to the config that is used to configure the alarm. The content is copied into the alarm handle.
 * @retval FUNCTION_RETURN_OK           Alarm was setup and callback will be triggered when set time is reached.
 * @retval FUNCTION_RETURN_PARAM_ERROR  The handle or config pointer were NULL. Is also returned if the function callback in the config is NULL or the alarm_time in the config is 0.
 * @retval FUNCTION_RETURN_NOT_READY    Is returned if an alarm was already set-up. You need to stop the current using `rtc_alarm_stop` before setting it again.
 */
FUNCTION_RETURN_T rtc_alarm_setup(rtc_alarm_handle_t alarm, const rtc_alarm_config_t* config);
/**
 * @brief Stops the alarm when it was setup and is not needed anymore.
 * The handle can be re-used to setup another alarm later.
 * 
 * @param alarm     Handle as created using `rtc_alarm_create`.
 * @retval FUNCTION_RETURN_OK           Alarm was stopped or alarm was not setup before.
 * @retval FUNCTION_RETURN_PARAM_ERROR  The handle was NULL.
 */
FUNCTION_RETURN_T rtc_alarm_stop(rtc_alarm_handle_t alarm);
/**
 * @brief Returns true if a setup alarm is triggered. Is cleared when `rtc_alarm_stop` is called.
 * 
 * @param alarm     Handle as created using `rtc_alarm_create`.
 * @return true     Alarm was triggered.
 * @return false    Alarm was not triggered.
 */
bool rtc_alarm_is_triggered(rtc_alarm_handle_t alarm);
#if RTC_ALARM_SELFTEST
/**
 * @brief Execute selfcheck of the rtc alarm module.
 * 
 * @retval FUNCTION_RETURN_OK           Selfcheck was sucessfull.
 * @retval other                        Selfcheck failed.
 */
FUNCTION_RETURN_T rtc_alarm_selfcheck(void);
#endif

#endif

#endif
