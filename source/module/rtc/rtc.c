// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file rtc.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
 */
 
#include "rtc.h"

#if MODULE_ENABLE_RTC

#include <string.h>
#include "module/comm/dbg.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define _SIMULATE_FRACTIONS		1

#if MCU_PERIPHERY_ENABLE_RTC

#if defined(MCU_RTC_ALLOWS_FRACTIONS) && MCU_RTC_ALLOWS_FRACTIONS
#undef _SIMULATE_FRACTIONS
#define _SIMULATE_FRACTIONS 0
#endif

#endif
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Rtc time object used in internal functions.
static rtc_time_t rtc_current_time = {0};

/// Timestamp of the last synchronization.
static rtc_time_t _last_sync_time = {0};
/// Timestamp of the next needed synchronization to keep the synchronized state
static rtc_time_t _next_sync_time = {0};
/// Valid duration for a synchronization in seconds before it becomes informative.
static time_t _synchronize_duration_seconds = RTC_SYNCHRONIZE_DURATION;
/// Current synchronization state of the rtc.
static RTC_SYNC_T _sync_state = RTC_SYNC_UNSYNCHRONIZED;

/// Pointer to first rtc observer
static rtc_observer_t* _first_observer = NULL;

/// Pointer to sync structure.
static rtc_sync_ext_rtc_t* _sync_external_rtc = NULL;

/// Is set to true when syncing to raise the sync event in set time.
static bool _in_sync_external_rtc = false;

/// Indicates whether rtc was synchronized at some point.
static bool _was_synchronized = false;

/// Lookup table for the number of days inside a month.
/// Index 1 is 0 if the year is not a leap year or 1 if the year is a leap year.
/// Index 2 is for the month - 1 (0 - 11).
static const int rtc_ytab[2][12] =
{
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

#if _SIMULATE_FRACTIONS
/// Offset from the current millisecond timer that is stored to calculate the current fraction of timestamps.
static uint32_t _offset_fractions = 0;
#endif

static rtc_time_t _simulated_time = {0};

static bool _simulation_running = false;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Triggers an event to notify all observer.
 * @param event     Event that is triggered
 */
static void _trigger_observer_event(RTC_EVENT_T event);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void rtc_start_simulation(void)
{
	_simulation_running = true;
}

void rtc_stop_simulation(void)
{
	_simulation_running = false;
}

void rtc_set_simulation_time(rtc_time_t* t)
{
	if(t == NULL)
		return;

	_simulated_time = *t;
	_simulation_running	= true;
}

bool rtc_is_null(const rtc_time_t* t)
{
	if(t == NULL)
		return true;

	if(t->tm_year == 0 && t->tm_mon == 0 && t->tm_mday == 0 && t->tm_hour == 0 && t->tm_min == 0 && t->tm_sec == 0 && t->tm_msec == 0)
		return true;

	return false;
}

FUNCTION_RETURN rtc_set_time(rtc_time_t* t)
{
	if(t == NULL)
	{
		return FUNCTION_RETURN_PARAM_ERROR;
	}
	
#if MCU_PERIPHERY_ENABLE_RTC
	mcu_rtc_set_time(t);
#else
	rtc_current_time = *t;
#endif
#if _SIMULATE_FRACTIONS	
	// The offset of a full second was value ms before the current tick count.
	_offset_fractions = system_get_tick_count() - t->tm_msec;
#endif
	_trigger_observer_event(_in_sync_external_rtc ? RTC_EVENT_TIME_SYNC_EXT_RTC : RTC_EVENT_TIME_SET);
	return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN rtc_get_time(rtc_time_t* t)
{
	if(t == NULL)
	{
		return FUNCTION_RETURN_PARAM_ERROR;
	}

	if(_simulation_running)
	{
		*t = _simulated_time;
		return FUNCTION_RETURN_OK;
	}

	if(_sync_external_rtc && _sync_external_rtc->cb)
	{
		if(_sync_external_rtc->timestamp_last_sync == 0 || _sync_external_rtc->interval == 0 || (system_get_tick_count() - _sync_external_rtc->timestamp_last_sync) >= _sync_external_rtc->interval)
		{
			FUNCTION_RETURN ret;
			ret = _sync_external_rtc->cb(_sync_external_rtc, t);
			if(ret == FUNCTION_RETURN_OK && _sync_external_rtc->interval > 0)
			{
				_sync_external_rtc->timestamp_last_sync = system_get_tick_count();
				// Set the synchronized time internally and notify observers.
				_in_sync_external_rtc = true;
				ret = rtc_set_time(t);
				_in_sync_external_rtc = false;
			}
			// Return here because current time is written in t already.
			return ret;
		}
	}
#if MCU_PERIPHERY_ENABLE_RTC
	mcu_rtc_get_time(t);
#else
	*t = rtc_current_time;
#endif
#if _SIMULATE_FRACTIONS
	t->tm_msec = (system_get_tick_count() - _offset_fractions) % 1000;
#endif
	return FUNCTION_RETURN_OK;
}

void rtc_set_synchronize_duration(time_t value)
{
	_synchronize_duration_seconds = value;
}

void rtc_set_sync_timestamp_current(RTC_SYNC_T sync)
{
	DBG_ASSERT(sync == RTC_SYNC_INFORMATIVE || sync == RTC_SYNC_SYNCHRONIZED, NO_ACTION, NO_RETURN, "Invalid rtc synchronization state\n");
	rtc_get_time(&_last_sync_time);
	_sync_state = sync;
	if(_sync_state == RTC_SYNC_SYNCHRONIZED)
	{
		// Set next synchronization timestamp based on the defined rtc synchronize duration
		_next_sync_time = rtc_time(rtc_mktime(&_last_sync_time) + _synchronize_duration_seconds);
		_was_synchronized = true;
	}
	_trigger_observer_event(RTC_EVENT_TIME_SYNC_EXPLICIT);
}

FUNCTION_RETURN_T rtc_get_sync_timestamp(rtc_time_t* t, RTC_SYNC_T* sync)
{
	if(t == NULL && sync == NULL)
		return FUNCTION_RETURN_PARAM_ERROR;
		
	if(t)
		memcpy(t, &_last_sync_time, sizeof(rtc_time_t));

	if(sync)
	{
		if(_sync_state == RTC_SYNC_SYNCHRONIZED)
		{
			if(rtc_reached_timestamp(&_next_sync_time))
				*sync = RTC_SYNC_INFORMATIVE;
			else
				*sync = RTC_SYNC_SYNCHRONIZED;
		}		
		else
			*sync = _sync_state;
	}

	if(_sync_state == RTC_SYNC_UNSYNCHRONIZED)
		return FUNCTION_RETURN_NOT_READY;

	return FUNCTION_RETURN_OK;
}

bool rtc_was_synchronized(void)
{
	return _was_synchronized;
}

void rtc_register_observer(rtc_observer_t* o, rtc_event_cb_t cb)
{
    if(o == NULL || cb == NULL)
        return;

    o->cb = cb;
    if(_first_observer == NULL)
        _first_observer = o;
    else
    {
        rtc_observer_t* o2 = _first_observer;
        while(o2->next)
        {
            if(o2 == o) // Already added
                return;

            o2 = o2->next;
        }
        o2->next = o;
    }

    o->next = NULL;
}

void rtc_set_sync_external_rtc(rtc_sync_ext_rtc_t* sync)
{
	_sync_external_rtc = sync;
}

void rtc_set_utc_time_from_gmt(rtc_time_t* time, int64_t epoch, int8_t timezone, bool has_daylight_saving_time)
{
	rtc_time_t t2;
    int64_t t1;

    if(epoch != 0)
        t1 = epoch;
    else if(time != NULL)
        t1 = rtc_mktime(time);
    else
    {
        return;
    }
	t2 = rtc_time(t1);

	if(has_daylight_saving_time && rtc_is_daylight_saving_time(&t2))
	{
		t1 -= 3600L; // Subtract 1 hour for daylight saving time
	}
	t1 -= (3600L * (int32_t)timezone); // Subtract x hours based on timezone
	
	t2 = rtc_time(t1);
	t2.tm_timezone = 0;
	if(time)
		t2.tm_msec = time->tm_msec;
	rtc_set_time(&t2);
}

void rtc_set_gmt_time_from_utc(rtc_time_t* time, int64_t epoch, int8_t timezone, bool has_daylight_saving_time)
{
    int64_t t1;

    if(epoch != 0)
        t1 = epoch;
    else if(time != NULL)
        t1 = rtc_mktime(time);
    else
    {
        return;
    }

	rtc_time_t t2;
	t1 += (3600L * (int32_t)timezone); // Add 1 hour for CET (Central European Time)
	t2 = rtc_time(t1);
	if(has_daylight_saving_time && rtc_is_daylight_saving_time(&t2))
	{
		t1 += 3600L; // Add 1 hour for daylight saving time
		t2 = rtc_time(t1);
	}
	t2.tm_timezone = timezone;
	if(time)
		t2.tm_msec = time->tm_msec;
	rtc_set_time(&t2);
}

bool rtc_is_daylight_saving_time(rtc_time_t* time)
{
	if(time == NULL)
		return false;

	// Jan, Feb, Nov and Dec are out.
	if (time->tm_mon < 2 || time->tm_mon > 9)
		return false;
	// April to September are in
	if (time->tm_mon > 2 && time->tm_mon < 9)
		return true;
	// Only March and October remain
	{
		uint16_t year = time->tm_year < RTC_EPOCH_YR ? time->tm_year + RTC_EPOCH_YR : time->tm_year;
		uint8_t date_last_sunday = rtc_get_days(time->tm_mon, year/*time->tm_year*/);
		uint8_t dow = rtc_get_day_of_week( year/*time->tm_year*/, time->tm_mon, date_last_sunday);
		date_last_sunday -= dow;

		// Before last sunday in march and after last sunday in october -> No Daylight saving time
		if((time->tm_mon == 2 && time->tm_mday < date_last_sunday) || (time->tm_mon == 9 && time->tm_mday > date_last_sunday))
			return false;

		// After last sunday in march and before last sunday in october -> Daylight saving time
		if((time->tm_mon == 2 && time->tm_mday > date_last_sunday) || (time->tm_mon == 9 && time->tm_mday < date_last_sunday))
			return true;

		// Now it is the day where the daylight saving time changes!
		// Daylight saving time before 3 am
		if(time->tm_mon == 2)
			return time->tm_hour >= 2;
		// Daylight saving time before 2 am
		return time->tm_hour < 2;
	}
}

uint8_t rtc_get_days(uint8_t month, uint16_t year)
{
	if(month > 11)
		return 0;

	if(year < RTC_EPOCH_YR)
		year += RTC_EPOCH_YR;

	return rtc_ytab[(int)rtc_is_a_leap_year(year)][month];
}

void rtc_go_back_days(rtc_time_t* t, uint16_t days)
{
	int16_t days_left = days;
	do
	{
		if(days_left < 0)
		{
			t->tm_mday += (days_left * (-1));
			days_left = 0;
		}
		else if(days_left < t->tm_mday)
		{
			t->tm_mday -= days_left;
			return;
		}
		else
		{
			t->tm_mon--;
			if(t->tm_mon > 11)
			{
				t->tm_mon = 11;
				t->tm_year--;
			}
			days_left -= rtc_get_days(t->tm_mon, t->tm_year);
		}
	}while(days_left != 0);
}

int64_t rtc_mktime(const rtc_time_t *t)
{
    int64_t val = 0;
    uint16_t i;

    if(t->tm_mday == 0)
    	return 0;

    for(i = t->tm_year; i > 0; i--)
    {
    	val += 365;
        if( rtc_is_a_leap_year(i - 1) )
        	val++;
    }

    for(i = 0; i < t->tm_mon; i++)
    	val += rtc_ytab[(int)rtc_is_a_leap_year(t->tm_year)][i];

    val += t->tm_mday - 1;
    val *= 86400L;
    val += (t->tm_hour * 3600L);
    val += (t->tm_min * 60L);
    val += t->tm_sec;

    return val;
}

int64_t rtc_mktime_ms(const rtc_time_t* t)
{
    int64_t val = rtc_mktime(t);
	
	val *= 1000;
    val += t->tm_msec;

    return val;
}

int64_t rtc_get_current_time(void)
{
	rtc_time_t t1;
	rtc_get_time(&t1);
	return rtc_mktime(&t1);
}

int64_t rtc_get_current_time_ms(void)
{
	rtc_time_t t1;
	rtc_get_time(&t1);
	return rtc_mktime_ms(&t1);
}

rtc_time_t rtc_time_ms(int64_t t)
{
	rtc_time_t t1 = {0};

	uint32_t t2 = (uint32_t)(t / 1000UL);
	
	t1.tm_msec = t % 1000UL;

	// Remove the seconds from t
	t1.tm_sec = t2 % 60U;
	t2 /= 60U;

	// Remove the minutes from t2
	if(t2 > 0)
	{
		t1.tm_min = t2 % 60U;
		t2 /= 60U;
	}

	// Remove the hours from t2
	if(t2 > 0)
	{
		t1.tm_hour = t2 % 24U;
		t2 /= 24U;
	}

	// Remove the years from t2
	while(t2 >= 365)
	{
		if( rtc_is_a_leap_year(t1.tm_year) )
		{
			if(t2 >= 366)
			{
				t1.tm_year++;
				t2 -= 366U;
			}
			else
				break; // It is not a leap year so it must be 31.12. in the leap year before
		}
		else
		{
			t1.tm_year++;
			t2 -= 365U;
		}
	}

	// Left in t2: months in dates -> Remove the months!
	t1.tm_mon = 0;
	if(t2 > 0)
	{
		uint16_t i = 0;
		while(t2 >= rtc_ytab[(int)rtc_is_a_leap_year(t1.tm_year)][i])
		{
			t2 -= (uint32_t)rtc_ytab[(int)rtc_is_a_leap_year(t1.tm_year)][i];
			t1.tm_mon++;
			i++;
		}
	}

	// Left in t2: dates
	t1.tm_mday = t2 + 1;

	return t1;
}

rtc_time_t rtc_time(int64_t t)
{
	rtc_time_t t1 = {0};

	uint32_t t2 = (uint32_t) t;

	// Remove the seconds from t
	t1.tm_sec = t2 % 60U;
	t2 /= 60U;

	// Remove the minutes from t2
	if(t2 > 0)
	{
		t1.tm_min = t2 % 60U;
		t2 /= 60U;
	}

	// Remove the hours from t2
	if(t2 > 0)
	{
		t1.tm_hour = t2 % 24U;
		t2 /= 24U;
	}

	// Remove the years from t2
	while(t2 >= 365)
	{
		if( rtc_is_a_leap_year(t1.tm_year) )
		{
			if(t2 >= 366)
			{
				t1.tm_year++;
				t2 -= 366U;
			}
			else
				break; // It is not a leap year so it must be 31.12. in the leap year before
		}
		else
		{
			t1.tm_year++;
			t2 -= 365U;
		}
	}

	// Left in t2: months in dates -> Remove the months!
	t1.tm_mon = 0;
	if(t2 > 0)
	{
		uint16_t i = 0;
		while(t2 >= rtc_ytab[(int)rtc_is_a_leap_year(t1.tm_year)][i])
		{
			t2 -= (uint32_t)rtc_ytab[(int)rtc_is_a_leap_year(t1.tm_year)][i];
			t1.tm_mon++;
			i++;
		}
	}

	// Left in t2: dates
	t1.tm_mday = t2 + 1;

	return t1;
}

bool rtc_is_a_leap_year(uint32_t year)
{
	if(year < RTC_EPOCH_YR)
    	year += RTC_EPOCH_YR;
	
	if((year % 400) == 0 || ((year % 4) == 0 && (year % 100) != 0))
	   return 1;
	else
	   return 0;
}

bool rtc_reached_timestamp(const rtc_time_t *time)
{
	rtc_get_time(&rtc_current_time);

	return rtc_compare(time, &rtc_current_time) <= 0;
}

int rtc_compare(const rtc_time_t* t1, const rtc_time_t* t2)
{
	// Return 1 if t1 > t2
	// Return -1 if t1 < t2
	// Return 0 if t1 == t2

	if(t1->tm_year > t2->tm_year)	
		return 1;
	else if(t1->tm_year < t2->tm_year)
		return -1;

	if(t1->tm_mon > t2->tm_mon)		
		return 1;
	else if(t1->tm_mon < t2->tm_mon)	
		return -1;

	if(t1->tm_mday > t2->tm_mday)		
		return 1;
	else if(t1->tm_mday < t2->tm_mday)	
		return -1;

	if(t1->tm_hour > t2->tm_hour)		
		return 1;
	else if(t1->tm_hour < t2->tm_hour)	
		return -1;

	if(t1->tm_min > t2->tm_min)		
		return 1;
	else if(t1->tm_min < t2->tm_min)
		return -1;

	if(t1->tm_sec > t2->tm_sec)		
		return 1;
	else if(t1->tm_sec < t2->tm_sec)
		return -1;

	if(t1->tm_msec > t2->tm_msec)
		return 1;
	else if(t1->tm_msec < t2->tm_msec)
		return -1;

	return 0;						// The dates are the same
}

void rtc_calculate_timestamp_offset(rtc_time_t *time, uint8_t interval_hour, uint8_t interval_minute)
{
	uint32_t  minute_offset = 0, hour_offset = 0, day_offset = 0;

	rtc_get_time(&rtc_current_time);

	time->tm_mday = rtc_current_time.tm_mday;
	time->tm_mon = rtc_current_time.tm_mon;
	time->tm_year = rtc_current_time.tm_year;
	time->tm_sec = rtc_current_time.tm_sec;

	minute_offset = (interval_minute) % 60;
	hour_offset = interval_minute / 60;

	interval_hour += hour_offset;

	time->tm_min = (rtc_current_time.tm_min + minute_offset) % 60;
	if((rtc_current_time.tm_min + minute_offset) >= 60) // Increment hour in case of an overflow
		interval_hour++;

	time->tm_hour = (rtc_current_time.tm_hour + interval_hour) % 24; // Increment day in case of an overflow.

	if((rtc_current_time.tm_hour + interval_hour) >= 24)
		day_offset = (rtc_current_time.tm_hour + interval_hour - time->tm_hour) / 24;

	if(day_offset) // If the days must be changed too -> check also months and years
	{
		// Check if date change leads to a change in the month
		if(rtc_current_time.tm_mday + day_offset > rtc_ytab[(int)rtc_is_a_leap_year(rtc_current_time.tm_year)][rtc_current_time.tm_mon])
		{
			time->tm_mday = rtc_current_time.tm_mday + day_offset - rtc_ytab[(int)rtc_is_a_leap_year(rtc_current_time.tm_year)][rtc_current_time.tm_mon];

			if(rtc_current_time.tm_mon == 11)
			{
				time->tm_mon = 0;
				(time->tm_year)++;
			}
			else
				(time->tm_mon)++;
		}
		else
			(time->tm_mday) = rtc_current_time.tm_mday + day_offset;
	}
}

void rtc_add_hours_to_date(rtc_time_t *time, uint32_t interval_hour)
{
	uint32_t  day_offset = 0, month_offset=0, year_offset=0;
	rtc_time_t old_time;
	memcpy(&old_time, time, sizeof(old_time));
	time->tm_hour = (old_time.tm_hour + interval_hour) % 24; // Increment day in case of an overflow.

	if((old_time.tm_hour + interval_hour) >= 24)
		day_offset = (old_time.tm_hour + interval_hour - time->tm_hour) / 24;
	if(day_offset) // If the days must be changed too -> check also months and years
	{
		if(old_time.tm_mday + day_offset > rtc_ytab[(int)rtc_is_a_leap_year(old_time.tm_year)][old_time.tm_mon])
		{
			while(old_time.tm_mday + day_offset >= rtc_ytab[(int)rtc_is_a_leap_year(old_time.tm_year + year_offset)][old_time.tm_mon + month_offset])
			{
				day_offset -= rtc_ytab[(int)rtc_is_a_leap_year(old_time.tm_year + year_offset)][old_time.tm_mon + month_offset];

				month_offset++;
				if(old_time.tm_mon + month_offset > 11)
				{
					year_offset++;
					month_offset -= 12;
				}
			}
		}
		time->tm_mday = old_time.tm_mday + day_offset;
		time->tm_mon = old_time.tm_mon + month_offset;
		time->tm_year = old_time.tm_year + year_offset;
	}
}

uint8_t rtc_get_day_of_week(int y, int m, int d)
{
	static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	y -= m < 2;
	return (y + y/4 - y/100 + y/400 + t[m] + d) % 7;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _trigger_observer_event(RTC_EVENT_T event)
{
    rtc_observer_t* o2 = _first_observer;
    while(o2)
    {
        if(o2->cb)
            o2->cb(o2, event);
        o2 = o2->next;
    }
}

#endif
