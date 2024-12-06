/**
 * @file rtc_alarm.c
 * @copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten
 */
 
#include "rtc_alarm.h"

#if MODULE_ENABLE_RTC

#include <string.h>
#include "module/comm/dbg.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Context structure for a single alarm.
struct rtc_alarm_s
{
    /// Configuration for this alarm.
    rtc_alarm_config_t config;
    /// Is set to true if alarm was set-up
    bool is_added;
    /// Is set when alarm is triggered from task and cleared on `rtc_alarm_stop`.
    bool triggered;
    /// Pointer to the next alarm is multiple alarms are set-up.
    rtc_alarm_handle_t next;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Task for the alarm. Checks if the next alarm can be triggered and triggers it's callback function.
 * When no further alarm is set, the task stops automatically.
 * 
 * @param pt        Pointer to protothread.
 * @return int      Protothread return value.
 */
static int _handle_alarm(struct pt* pt);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Single task for all alarms.
static system_task_t _task = {.name = "rtc_alarm"};
/// Is set to true when first alarm was created and therefore the task was initialized.
static bool _initialized = false;
/// The first alarm or the larm that will be triggered next. Is set when `rtc_alarm_setup` is called.
static rtc_alarm_handle_t _first_alarm = NULL;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

rtc_alarm_handle_t rtc_alarm_create(void)
{
    if(!_initialized)
    {
        _initialized = true;
        system_task_init_protothread(&_task, false, _handle_alarm, NULL);
    }

    rtc_alarm_handle_t alarm = mcu_heap_calloc(1, sizeof(struct rtc_alarm_s));

    return alarm;
}

void rtc_alarm_free(rtc_alarm_handle_t alarm)
{
    ASSERT_RET(alarm, NO_ACTION, NO_RETURN, "Alarm handle cannot be null\n");
    rtc_alarm_stop(alarm);
    mcu_heap_free(alarm);
}

FUNCTION_RETURN_T rtc_alarm_setup(rtc_alarm_handle_t alarm, const rtc_alarm_config_t* config)
{
    ASSERT_RET(alarm, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Alarm handle cannot be null\n");
    ASSERT_RET(config, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Config pointer cannot be null\n");
    ASSERT_RET(!alarm->is_added, NO_ACTION, FUNCTION_RETURN_NOT_READY, "Alarm handle cannot be null\n");
    rtc_time_t time_null = {0};
    ASSERT_RET(rtc_compare(&time_null, &config->alarm_time) < 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid alarm time\n");

    memcpy(&alarm->config, config, sizeof(rtc_alarm_config_t));
    alarm->triggered = false;

    if(_first_alarm == NULL)
    {
        _first_alarm = alarm;
        alarm->next = NULL;
    }
    else
    {
        // Alarms are always in the order of the next trigger time.
        // The one that is triggered next is always the first in the list.
        
        if(rtc_compare(&config->alarm_time, &_first_alarm->config.alarm_time) < 0)
        {
            // New alarm is first one!
            alarm->next = _first_alarm;
            _first_alarm = alarm;
        }
        else
        {
            // New alarm is later than the first!
            // We look for the first one inside the list that should trigger after this alarm and put ours before it.
            rtc_alarm_handle_t a = _first_alarm;
            rtc_alarm_handle_t a_old;
            do
            {
                a_old = a;
                a = a->next;
            }while(a && rtc_compare(&config->alarm_time, &a->config.alarm_time) >= 0);

            if(a)
            {
                // a_old should trigger before alarm and a should trigger after alarm.
                a_old->next = alarm;
                alarm->next = a;
            }
            else
            {
                // a is NULL, so we are the last one inside the list.
                a_old->next = alarm;
                alarm->next = NULL;
            }
        }
    }
    alarm->is_added = true;
    system_task_add(&_task);
    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T rtc_alarm_stop(rtc_alarm_handle_t alarm)
{
    ASSERT_RET(alarm, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Alarm handle cannot be null\n");

    if(!alarm->is_added)
        return FUNCTION_RETURN_OK;
    
    alarm->is_added = false;
    alarm->triggered = false;

    if(_first_alarm == alarm)
    {
        _first_alarm = alarm->next;
    }
    else
    {
        rtc_alarm_handle_t a = _first_alarm;
        // Skip until a points to the alarm before the target.
        while(a && a->next != alarm)
        {
            a = a->next;
        }
        // If alarm was found -> Take the alarm out of the list.
        if(a && a->next == alarm)
        {
            a->next = alarm->next;
        }
    }

    alarm->next = NULL;

    // Remove task if last alarm was removed.
    if(_first_alarm == NULL)
        system_remove_task(&_task);

    return FUNCTION_RETURN_OK;
}

bool rtc_alarm_is_triggered(rtc_alarm_handle_t alarm)
{
    ASSERT_RET(alarm, NO_ACTION, false, "Alarm handle cannot be null\n");
    return alarm->triggered;
}

#if RTC_ALARM_SELFTEST
FUNCTION_RETURN_T rtc_alarm_selfcheck(void)
{
    DBG_ASSERT(_first_alarm == NULL && !system_task_is_active(&_task), NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Cannot start selfcheck when alarm already in use!\n");

    rtc_alarm_handle_t alarm1 = rtc_alarm_create();
    DBG_ASSERT(alarm1, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "error creating handle\n");
    rtc_alarm_handle_t alarm2 = rtc_alarm_create();
    DBG_ASSERT(alarm2, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "error creating handle\n");
    rtc_alarm_handle_t alarm3 = rtc_alarm_create();
    DBG_ASSERT(alarm3, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "error creating handle\n");
    rtc_alarm_handle_t alarm4 = rtc_alarm_create();
    DBG_ASSERT(alarm3, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "error creating handle\n");
    
    // Check error handling on wrong parameter
    rtc_alarm_config_t config_invalid = {0};    
    DBG_ASSERT(rtc_alarm_setup(NULL, NULL) == FUNCTION_RETURN_PARAM_ERROR, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup Pointer check failed\n");
    DBG_ASSERT(rtc_alarm_setup(alarm1, NULL) == FUNCTION_RETURN_PARAM_ERROR, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup Pointer check failed\n");
    DBG_ASSERT(rtc_alarm_setup(NULL, &config_invalid) == FUNCTION_RETURN_PARAM_ERROR, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup Pointer check failed\n");
    DBG_ASSERT(rtc_alarm_setup(alarm1, &config_invalid) == FUNCTION_RETURN_PARAM_ERROR, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup time check failed\n");
    
    DBG_ASSERT(rtc_alarm_stop(NULL) == FUNCTION_RETURN_PARAM_ERROR, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Stop Pointer check failed\n");    
    DBG_ASSERT(rtc_alarm_stop(alarm1) == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Stop on unset alarm failed\n");

    // Check setting a first config
    rtc_alarm_config_t config1 = {.alarm_time = RTC_INIT(2022, 3, 2, 12, 30, 0, 0)};    
    DBG_ASSERT(rtc_alarm_setup(alarm1, &config1) == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup alarm failed\n");
    DBG_ASSERT(_first_alarm == alarm1, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup alarm failed\n");
    DBG_ASSERT(rtc_compare(&_first_alarm->config.alarm_time, &config1.alarm_time) == 0, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup alarm failed\n");
    DBG_ASSERT(system_task_is_active(&_task), NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup alarm failed\n");

    // Check stop the first alarm
    DBG_ASSERT(rtc_alarm_stop(alarm1) == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Stop alarm failed\n");
    DBG_ASSERT(_first_alarm == NULL, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Stop alarm failed\n");
    DBG_ASSERT(!system_task_is_active(&_task), NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Stop alarm failed\n");

    // Add alarm again and check with further alarms: alarm3 < alarm1 < alarm2, but added in order alarm1, alarm2, alarm3
    DBG_ASSERT(rtc_alarm_setup(alarm1, &config1) == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup alarm failed\n");
    DBG_ASSERT(_first_alarm == alarm1, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup alarm failed\n");
    rtc_alarm_config_t config2 = {.alarm_time = RTC_INIT(2022, 3, 2, 13, 30, 0, 0)};    
    DBG_ASSERT(rtc_alarm_setup(alarm2, &config2) == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Adding alarm failed\n");
    // Ensure above order: alarm1 < alarm2
    DBG_ASSERT(_first_alarm == alarm1, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup alarm failed\n");
    DBG_ASSERT(alarm1->next == alarm2, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");

    rtc_alarm_config_t config3 = {.alarm_time = RTC_INIT(2022, 3, 2, 11, 30, 0, 0)};    
    DBG_ASSERT(rtc_alarm_setup(alarm3, &config3) == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Adding alarm failed\n");
    DBG_ASSERT(system_task_is_active(&_task), NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup alarm failed\n");
    // Ensure above order: alarm3 < alarm1 < alarm2
    DBG_ASSERT(_first_alarm == alarm3, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm3->next == alarm1, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm1->next == alarm2, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm2->next == NULL, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");

    rtc_alarm_config_t config4 = {.alarm_time = RTC_INIT(2022, 3, 2, 14, 30, 0, 0)};    
    DBG_ASSERT(rtc_alarm_setup(alarm4, &config4) == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Adding alarm failed\n");
    DBG_ASSERT(system_task_is_active(&_task), NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Setup alarm failed\n");
    // Ensure all are added
    DBG_ASSERT(alarm1->is_added == true, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm2->is_added == true, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm3->is_added == true, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm4->is_added == true, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");

    // Ensure above order: alarm3 < alarm1 < alarm2 < alarm4
    DBG_ASSERT(_first_alarm == alarm3, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm3->next == alarm1, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm1->next == alarm2, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm2->next == alarm4, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm4->next == NULL, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");

    // Test removing from between
    DBG_ASSERT(rtc_alarm_stop(alarm2) == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Stop alarm failed\n");
    // Ensure above order: alarm3 < alarm1 < alarm4
    DBG_ASSERT(_first_alarm == alarm3, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm3->next == alarm1, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm1->next == alarm4, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm4->next == NULL, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm2->is_added == false, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");

    // Test removing from end
    DBG_ASSERT(rtc_alarm_stop(alarm4) == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Stop alarm failed\n");
    // Ensure above order: alarm3 < alarm1
    DBG_ASSERT(_first_alarm == alarm3, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm3->next == alarm1, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm1->next == NULL, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm4->is_added == false, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");

    // Test removing from start
    DBG_ASSERT(rtc_alarm_stop(alarm3) == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Stop alarm failed\n");
    // Ensure above order: alarm1
    DBG_ASSERT(_first_alarm == alarm1, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm3->next == NULL, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm1->next == NULL, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm3->is_added == false, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(system_task_is_active(&_task), NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Ensure task still active failed\n");

    // Test removing last one
    DBG_ASSERT(rtc_alarm_stop(alarm1) == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Stop alarm failed\n");
    // Ensure above order: alarm1
    DBG_ASSERT(_first_alarm == NULL, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(alarm1->is_added == false, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Order check failed\n");
    DBG_ASSERT(!system_task_is_active(&_task), NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Stop alarm failed\n");

    // Free handles
    rtc_alarm_free(alarm1);
    rtc_alarm_free(alarm2);
    rtc_alarm_free(alarm3);
    rtc_alarm_free(alarm4);
    // Everything worked, rtc alarm is again in the configuration as if it was unused
    
    return FUNCTION_RETURN_OK;
}
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static int _handle_alarm(struct pt* pt)
{
    PT_BEGIN(pt);

    do
    {
        // Wait until alarm time is reached
        PT_YIELD_UNTIL(pt, rtc_reached_timestamp(&_first_alarm->config.alarm_time));
        // Stop alarm and trigger it's callback.
        // Use temporary variable because _first_alarm pointer might change in rtc_alarm_stop.
        rtc_alarm_handle_t alarm = _first_alarm;
        // Stop first to ensure rtc_alarm_setup can be used in the callback!
        rtc_alarm_stop(alarm);
        // Trigger the callback
        alarm->triggered = true;
        // Trigger only the callback if callback is set.
        if(alarm->config.f)
            alarm->config.f(alarm, &alarm->config);
    }
    while(_first_alarm);

    PT_END(pt);
}

#endif
