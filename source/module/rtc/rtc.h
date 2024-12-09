// Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		rtc.h
 * 	@copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten
 *  @author 	Tim Koczwara, Heinrich Pettenpohl
 *
 *  @brief		Some functions from the original rtc module -> Not the complete module!
 *
 *  @version    1.36 (23.03.2023, Tim Koczwara)
 *              - Added \ref rtc_was_synchronized
 *  @version    1.35 (03.03.2023, Tim Koczwara)
 *              - Added \ref rtc_set_gmt_time_from_utc
 *              - Added \ref rtc_set_utc_time_from_gmt 
 *              - Changed \ref rtc_set_utc_time to deprecated, use \ref rtc_set_gmt_time_from_utc instead.
 *  @version	1.34 (02.02.2023, Tim Koczwara)
 *              - Added rtc_compare and modified rtc_reached_timestamp to use rtc_compare
 *              - Added RTC_INIT
 *              - Added rtc_time_ms and rtc_mktime_ms
 *  @version	1.33 (21.12.2022, Tim Koczwara)
 *              - Added rtc time synchronization timestamp
 *              - Added rtc fractions
 *  @version	1.32 (05.06.2018, Tim Koczwara)
 *  			- rtc_is_leap_year moved
 *  			- can be used if no rtc_get_time is defined
 *  @version	1.31 (17.06.2014, Tim Koczwara)
 *
 *
 ******************************************************************************/
 
#ifndef RTC_HEADER_FIRST_INCLUDE_GUARD
#define RTC_HEADER_FIRST_INCLUDE_GUARD

#include "module_public.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Independent of module enable defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// First year of calculation in rtc_time
#define RTC_EPOCH_YR			1900
/// Number of seconds in a minute
#define SECONDS_IN_MINUTE		(60)
/// Number of seconds in an hour
#define SECONDS_IN_HOUR			(3600)
/// Number of seconds in a day
#define SECONDS_IN_DAY			(86400)

#if MODULE_ENABLE_RTC

#include "module/enum/function_return.h"

#ifdef _RENESAS_SYNERGY_
#include <time.h>
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#ifndef RTC_SYNCHRONIZE_DURATION
/// Synchronization interval in seconds for the rtc, before a synchronized clock is seen as relative.
#define RTC_SYNCHRONIZE_DURATION    (60 * 60)
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Initializer for rtc_time_t using the human readable time.
/// @param y    Complete year like 2023
/// @param mon  Month 1-12
/// @param d    Day in month 1 to 31
/// @param h    Hour of day 0 to 23
/// @param min  Minute of hour 0 to 59
/// @param s    Second of minute 0 to 59
/// @param ms   Milliseconds of second 0 to 999.
#define RTC_INIT(y, mon, d, h, min, s, ms)  {.tm_year = y - RTC_EPOCH_YR, .tm_mon = mon - 1, .tm_mday = d, .tm_hour = h, .tm_min = min, .tm_sec = s, .tm_msec = ms}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Possible events that are raised on rtc observer.
 */
typedef enum RTC_EVENT_T
{
    /// Time was set
    RTC_EVENT_TIME_SET = 0,
    /// Time was synchronized with external rtc
    RTC_EVENT_TIME_SYNC_EXT_RTC = 1,
    /// Synchronization timestamp was set, so synchronization with external server happened.
    RTC_EVENT_TIME_SYNC_EXPLICIT = 2,
}RTC_EVENT_T;

/**
 * Synchronization state of the own rtc. 
 */
typedef enum RTC_SYNC_E
{
    /// Clock is unsynchronized or synchronization is unknown. Default after reset
    RTC_SYNC_UNSYNCHRONIZED = 0,
    /// The current clock is only loosly synchronized (for example if it is manually set by a user) or the synchronized time was not re-synchronized.
    RTC_SYNC_INFORMATIVE,
    /// The rtc is synchronized with an external server.
    RTC_SYNC_SYNCHRONIZED
}RTC_SYNC_T;

/// RTC observer type
typedef struct rtc_observer_s rtc_observer_t;

/// RTC event callback
/// First param: Pointer to the observer structure for which this event is called.
/// Second param: Event type
typedef void(*rtc_event_cb_t)(rtc_observer_t*, RTC_EVENT_T);

/// rtc observer structure
struct rtc_observer_s
{
    /// Private: Pointer to next observer
    rtc_observer_t* next;
    /// Private: Pointer to callback function
    rtc_event_cb_t cb;
    /// Public: Pointer to a data structure, can be set externally if needed.
    void* data;
};

/// RTC sync type
typedef struct rtc_sync_s rtc_sync_ext_rtc_t;
/// RTC synchronization structure.
/// First param: Pointer to the synchronization structure.
/// Second param: Pointer to the rtc time structure that needs to be filled with the concrete time.
typedef FUNCTION_RETURN (*rtc_sync_cb_t)(rtc_sync_ext_rtc_t*, rtc_time_t* );

/// RTC synchronization structure
/// Create it and set it to the rtc to get periodic resynchronization of the rtc with an external clock source.
struct rtc_sync_s
{
    /// Public: Pointer to a data structure, can be set externally if needed.
    void* user;
    /// Interval in ms after which the time should be resynced. On rtc_get_time the interval is checked and if needed the time will be synced via callback.
    /// Set interval to 0 to have a synchronization on every call of rtc_get_time every time.
    /// When interval is > 0, rtc_set_time is called internally on every synchronization to also inform the observers with RTC_EVENT_TIME_SYNC.
    /// When interval is 0, rtc_set_time is not called and no observers are informed.
    uint32_t interval;
    /// Timestamp in ms on which the last synchronization happened.
    uint32_t timestamp_last_sync;
    /// Callback function that is called on rtc_get_time when it is time for syncing the clock.
    rtc_sync_cb_t cb;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * Sets the time from the parameter into the rtc.
 *
 * @param t		Pointer to the structure that contains the time.
 */
FUNCTION_RETURN rtc_set_time(rtc_time_t* t);
/**
 * Loads the time from the rtc and puts it into the pointer time structure.
 *
 * @param t		Pointer to the structure where the current time is put into.
 */
FUNCTION_RETURN rtc_get_time(rtc_time_t* t);
/**
 * @brief Set the synchronize duration for the rtc. This indicates how long a synchronization state stays on synchronized before it becomes informative, when not resynced.
 * RTC_SYNCHRONIZE_DURATION is the default value for the duration.
 * @param value     Duration in seconds.
 */
void rtc_set_synchronize_duration(time_t value);
/**
 * @brief Mark current time as the timestamp of time synchronization.
 * Might be relevant for protocols like NTP, where a reference timestamp is needed. 
 * @param sync      Valid values are RTC_SYNC_SYNCHRONIZED and RTC_SYNC_INFORMATIVE.
 */
void rtc_set_sync_timestamp_current(RTC_SYNC_T sync);
/**
 * @brief Get the last time synchronization timestamp.
 * Might be relevant for protocols like NTP, where a reference timestamp is needed. 
 * 
 * @param t             Pointer to the time structure where the synchronization timestamp should be written into.
 * @param sync          Pointer to the synchronization state of the rtc.
 * @retval FUNCTION_RETURN_OK when the time was synchronized before.
 * @retval FUNCTION_RETURN_PARAM_ERROR when both pointer are NULL.
 * @retval FUNCTION_RETURN_NOT_READY when the time was not synchronized since the system started.
 */
FUNCTION_RETURN_T rtc_get_sync_timestamp(rtc_time_t* t, RTC_SYNC_T* sync);
/**
 * @brief Indicates whether the rtc was synchronized at some point or not. It does NOT mean, that rtc is still synchronized, use \c rtc_get_sync_timerstamp to check it.
 * 
 * @return true         RTC was synchronized after last boot.
 * @return false        RTC has never been synchronized.
 */
bool rtc_was_synchronized(void);
/**
 * Registers a callback function as a rtc observer. If one of the parameter is NULL, nothing happens.
 * @param o         Structure of the observer. If needed, set data pointer manually to your data structure.
 * @param cb        Callback function for the event.
 */
void rtc_register_observer(rtc_observer_t* o, rtc_event_cb_t cb);

/**
 * @brief Sets a synchronization callback to the rtc. The structure will be used by the rtc internally to resynchronize the clock using the provided
 *        callback in the structure. You can control it by setting the cb and interval variable inside the structure.
 *        The timestamp is modified by the rtc on resync.
 * 
 * @param sync      Pointer to the structure for synchronization.
 */
void rtc_set_sync_external_rtc(rtc_sync_ext_rtc_t* sync);
/**
 * Sets the time from the gmt time in the parameter to store it in the rtc as UTC.
 * The function calculates it to german GMT+2 or GMT+1 in regards to daylight saving time. One of the parameters must be not zero.
 *
 * @param time			Optional: Pointer to the current UTC time.
 * @param epoch         Optional: Current UTC time in unix epoch format
 * @param has_daylight_saving_time Indicates whether the timezone to set has a daylight saving time or not.
 */
void rtc_set_utc_time_from_gmt(rtc_time_t* time, int64_t epoch, int8_t timezone, bool has_daylight_saving_time);
/**
 * Sets the time from the utc time in the parameter to store it in the rtc as GMT.
 * The function calculates it to german GMT+2 or GMT+1 in regards to daylight saving time. One of the parameters must be not zero.
 *
 * @param time			Optional: Pointer to the current UTC time.
 * @param epoch         Optional: Current UTC time in unix epoch format
 * @param has_daylight_saving_time Indicates whether the timezone to set has a daylight saving time or not.
 */
void rtc_set_gmt_time_from_utc(rtc_time_t* time, int64_t epoch, int8_t timezone, bool has_daylight_saving_time);
/**
 * Sets the time from the utc time in the parameter.
 * The function calculates it to german GMT+2 or GMT+1 in regards to daylight saving time. One of the parameters must be not zero.
 *
 * @param time			Optional: Pointer to the current UTC time.
 * @param epoch         Optional: Current UTC time in unix epoch format
 * 
 * @deprecated Use \ref rtc_set_gmt_time_from_utc instead.
 */
inline void rtc_set_utc_time(rtc_time_t* time, int64_t epoch, int8_t timezone)
{
    rtc_set_gmt_time_from_utc(time, epoch, timezone, true);
}
/**
 * Calculates if the date is in the daylight saving time (MESZ) or not (MEZ / CET).
 * @param time	Pointer to the time that should be checked.
 * @return		true: MESZ should be used
 * 				false: MEZ / CET should be used
 */
bool rtc_is_daylight_saving_time(rtc_time_t* time);

/**
 * Returns the number of days inside the given month of the given year.
 *
 * @param month		Month to check (1 - 12)
 * @param year		Year where the month is in (needed for checking leap years).
 * @return			0 when the month is invalid or 1 - 31 when the month is valid.
 */
uint8_t rtc_get_days(uint8_t month, uint16_t year);

/**
 * Changes the Timestamp to a date that was x days ago, where x is the parameter days.
 *
 * @param t			Pointer to the structure containing the timestamp that needs to be changed.
 * @param days		Number of days to go back from the timestamp.
 */
void rtc_go_back_days(rtc_time_t* t, uint16_t days);

/**
 * Creates an timestamp out of the rtc_time_t. The timestamp is the number of seconds that passed since the 1.1.2000.
 *
 * @param t					Pointer to the time structure containing the time that needs to be converted.
 * @return					Timestamp in seconds since 1.1.2000.
 */
int64_t rtc_mktime(const rtc_time_t *t);
/**
 * Creates an timestamp out of the rtc_time_t. The timestamp is the number of milliseconds that passed since the epoch start.
 * 
 * @param t					Pointer to the time structure containing the time that needs to be converted.
 * @return					Timestamp in milliseconds since epoch start.
 */
int64_t rtc_mktime_ms(const rtc_time_t* t);

/**
 * Returns the current time as int64_t containing the number of milliseconds that passed since the epoch start.
 * @return			Current time in milliseconds since epoch start.
 */
int64_t rtc_get_current_time(void);

/**
 * Returns the current time as int64_t.
 * @return			Current time
 */
int64_t rtc_get_current_time_ms(void);
/**
 * Creates a rtc_time_t structure from an int64_t millisecond timestamp that was created with rtc_mktime_ms.
 * 
 * @param t					Timestamp in milliseconds since epoch.
 * @return					Time structure containing the time from the timestamp.
 */
rtc_time_t rtc_time_ms(int64_t t);
/**
 * Creates a rtc_time_t structure from an int64_t timestamp that was created with rtc_mktime.
 *
 * @param t					Timestamp in seconds since 1.1.2000.
 * @return					Time structure containing the time from the timestamp.
 */
rtc_time_t rtc_time(int64_t t);

/**
 * Checks if the year in the parameter is a leap year.
 *
 * @param year				Year that needs to be checked.
 * @return					true: Year is a leap year.
 * 							false: Year is not a leap year.
 */
bool rtc_is_a_leap_year(uint32_t year);

/**
 * Checks if the timestamp in the parameter is reached by comparing it to the current time.
 *
 * @param time				Pointer to the time structure that needs to be compared with the current time.
 * @return					true: Time is reached
 * 							false: Time is not reached.
 */
bool rtc_reached_timestamp(const rtc_time_t *time);

/**
 * @brief Compare two rtc timestamps. Make sure none of it is NULL!
 * 
 * @param t1                First timestamp to compare
 * @param t2                Second timestamp to compare
 * 
 * @return > 0              If t1 is greater than t2
 * @return 0                If t1 equals t2
 * @return < 0              If t1 is lower than t2
 */
int rtc_compare(const rtc_time_t* t1, const rtc_time_t* t2);

/**
 * Calculates a timestamp offset from the current time by adding the hours and minutes from the parameter to it.
 *
 * @param time				Pointer to the time structure where the new time is written to.
 * @param interval_hour		Number of hours that should be added to the time.
 * @param interval_minute	Number of minutes that should be added to the time.
 */
void rtc_calculate_timestamp_offset(rtc_time_t *time, uint8_t interval_hour, uint8_t interval_minute);

/**
 * Calculates a timestamp offset from the given time by adding the hours from the parameter to it.
 *
 * @param time				Pointer to the time structure where the old time is loaded from and the new time is writtens to.
 * @param interval_hour		Number of hours that should be added to the time.
 */
void rtc_add_hours_to_date(rtc_time_t *time, uint32_t interval_hour);

/**
 * Returns the day of week (0-6) starting with sunday.
 * @param y		Year
 * @param m		Month
 * @param d		Day
 * @return		day of week (0-6) starting with sunday.
 */
uint8_t rtc_get_day_of_week(int y, int m, int d);

#endif

#endif
