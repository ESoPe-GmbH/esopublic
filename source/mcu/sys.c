// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 	sys.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/
#include "sys.h"
#include "mcu.h"
#if MCU_TYPE == MCU_ESP32
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#if MCU_PERIPHERY_ENABLE_ETHERNET || MCU_PERIPHERY_ENABLE_WIFI
#include "esp_netif.h"
#endif
#include "esp_event.h"
#include "nvs_flash.h"
#endif
#include "module_public.h"
#if MODULE_ENABLE_FLASH_INFO
#include "module/flash_info/flash_info.h"
#endif
#if MODULE_ENABLE_SWUPDATE && MODULE_ENABLE_MMC && SW_UPDATE_MMC_ON_STARTUP
#include "module/swupdate/sw_update_mmc.h"
#include "module/version/version.h"
#endif

#if MODULE_ENABLE_DEBUG_CONSOLE && MODULE_ENABLE_CONSOLE
#include "module/comm/comm.h"
#include "module/console/console.h"
#include "module/console/dbg/debug_console.h"
#elif SYSTEM_ENABLE_PRINT_STATISTIC
#include "module/comm/comm.h"
#endif
#if MODULE_ENABLE_NETWORK
#include "module/network/network_interface.h"
#endif

#if MODULE_ENABLE_SECURITY
#include "module/security/security.h"
#endif

#if MCU_TYPE == RSYNERGY
#include "hal_data.h"
#endif

#if MCU_TYPE == PC_EMU
#include "pc/debug.h"
#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>	// Needed for GetTickCount()
#include "pc/windows.h"
#elif defined(__linux__)
#include <unistd.h>
#include <time.h>
#endif
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define _IS_ST()		(MCU_TYPE == MCU_STM32F3 || MCU_TYPE == MCU_STM32F4 || MCU_TYPE == MCU_STM32F7 || MCU_TYPE == MCU_STM32F0 || MCU_TYPE == MCU_STM32F1 || MCU_TYPE == MCU_STM32G0)

#if MCU_TYPE == MCU_STM32F3
#include "stm32f3xx_hal.h"
#endif
#if MCU_TYPE == MCU_STM32F4
#include "stm32f4xx_hal.h"
#endif
#if MCU_TYPE == MCU_STM32F7
#include "stm32f7xx_hal.h"
#endif
#if MCU_TYPE == MCU_STM32F0
#include "stm32f0xx_hal.h"
#endif
#if MCU_TYPE == MCU_STM32F1
#include "stm32f1xx_hal.h"
#endif
#if MCU_TYPE == MCU_STM32G0
#include "stm32g0xx_hal.h"
#endif

#define _NEED_TIMER_FOR_MS()		(MCU_TYPE != PC_EMU && MCU_TYPE != RSYNERGY && !_IS_ST() && MCU_TYPE != MCU_ESP32)

#if SYSTEM_ENABLE_MONITORING
#define _NUM_RECORDINGS		SYSTEM_MONITOR_NUM_RECORDINGS
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


/**
 * @enum SYSTEM_TASK_TYPE
 *
 * Defines possible task types that are used inside the task structure.
 *
 */
typedef enum
{
	/// The handled void function has a pointer as a parameter.
	SYSTEM_TASK_TYPE_HANDLE = 0,
	/// The handled int function has a protothread as a parameter.
	SYSTEM_TASK_TYPE_PROTOTHREAD
}SYSTEM_TASK_TYPE;

#if SYSTEM_ENABLE_MONITORING
// TODO: Documentation
typedef struct
{
	uint32_t timestamp;
	uint16_t line_before;
	uint16_t line_after;
	system_task_t* task;
}_recording_t;
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MCU_ENABLE_FREERTOS
/**
 * @brief Task to perform main functionality
 * 
 * @param arg Unused
 */
static void _task_main(void* arg);
#endif

/**
 * @brief   Initializes variables and starts the timer.
 *
 * @return          Returns false if the timer could not be initialized.
 **/
static bool _init(void);

/**
 * @brief   Calls the handles of all registered tasks.
 *          If sleep mode control was enabled with @ref system_enable_sleep_mode_control, the system will only enter the sleep mode
 *          when no prevention flag is set with @ref system_set_sleep_prevention_flag.
 */
static void _handle(void);

/**
 * @brief Recursive function to free all sub protothreads of a protothread.
 * 
 * @param pt 		Pointer to the protothread to check for sub protothreads.
 */
static void _free_subtasks(struct pt* pt);

#if PT_ENABLE_ENHANCED_DEBUG
/**
 * Returns the filename without the path.
 * @param name      Pointer to the task's filename.
 */
static char* _get_filename(const char* name);
#endif

#if MODULE_ENABLE_DEBUG_CONSOLE && MODULE_ENABLE_CONSOLE
/**
 * Callback function for the console.
 * @param data          Points to the console which triggered the command.
 * @param args          Pointer to strings that separate the received configuration command.
 * @param args_len      Number of received arguments.
 */
static FUNCTION_RETURN _console(console_data_t* data, char** args, uint8_t args_len);
#endif

#if SYSTEM_ENABLE_MONITORING
// TODO: Documentation
static void _add_recording(system_task_t* t, uint16_t line_pre);
#endif

#if _NEED_TIMER_FOR_MS()
/**
 * @brief Callback function for the millisecond timer.
 *
 * @param obj                   Unused pointer
 **/
static void system_timer_millisec(void* obj);
#endif

/**
 * @brief Board initialization as implemented in board_init. 
 */
extern void board_init(void);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Set to true once system_init is called.
static bool system_initialized = false;
#if _NEED_TIMER_FOR_MS()
/// Handle to the timer used for counting milliseconds.
static mcu_timer_t sys_timer;
#endif
#if !_IS_ST() && MCU_TYPE != MCU_ESP32
/// Millisecond counter. Is only set to 0 again when overflowing.
static uint32_t sys_msec_counter = 0;
#endif
/// Pointer to the first task. Used to loop the tasks.
static system_task_t* _first_task = NULL;
#if SYSTEM_ENABLE_SLEEP_MODE
/// Flag indicating whether sleep mode should be used or not when a loop on all system handlers is complete.
static bool _sleep_mode_control_active = false;
/// Number of milliseconds to sleep
static uint32_t _sleep_mode_time_ms = 0;
/// Bitmask of prevention flags that were registered.
static system_prevention_flag_t _prevention_used_flags = 0;
/// Bitmask of prevention flags that are set to prevent sleeping
static system_prevention_flag_t _prevention_active_flags = 0;
#endif
#if MODULE_ENABLE_DEBUG_CONSOLE && MODULE_ENABLE_CONSOLE
/// Structure for the console command
static console_command_t _cmd = {
        .command = "task",
        .fnc_exec = _console,
        .use_array_param = true,
        .explanation = "Subcommand: print for printing task information"
};
#endif
#if SYSTEM_ENABLE_MONITORING
// TODO: Documentation
static uint16_t _num_recordings = 0;
// TODO: Documentation
static _recording_t _recordings[_NUM_RECORDINGS];
#endif

#if MCU_TYPE == PC_EMU
bool _stop_execution = false;
#endif
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Main function
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Initialization function that need to be implemented as main initialization for modules in the main file.
 */
extern void app_main_init(void);

/**
 * Handle function that need to be implemented as main initialization for handling in the main file.
 */
extern void app_main_handle(void);

#ifdef _RENESAS_SYNERGY_
void hal_entry(void)
#elif defined(KERNELTEST) || defined(ESOPUBLICTEST)
void system_main(void)
#elif _IS_ST() || MCU_TYPE == PC_EMU
int main(void)
#elif MCU_TYPE == MCU_ESP32
void app_main(void)
#else
void main(void)
#endif
{
#if MCU_TYPE == PC_EMU
	debug_init();
#endif

#if MCU_TYPE == MCU_ESP32
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		nvs_flash_erase();
		ret = nvs_flash_init();
	}
#endif // MCU_TYPE == MCU_ESP32

	board_init();

#if MCU_TYPE == MCU_ESP32

#if MCU_PERIPHERY_ENABLE_ETHERNET || MCU_PERIPHERY_ENABLE_WIFI
	esp_netif_init();
#endif
	esp_event_loop_create_default();
#endif // MCU_TYPE == MCU_ESP32

	_init();

#if MCU_PERIPHERY_ENABLE_FLASH
	mcu_flash_init();
#endif

#if MODULE_ENABLE_FLASH_INFO
	flash_info_init();
#endif

#if MCU_PERIPHERY_ENABLE_RTC
#ifdef _RENESAS_SYNERGY_
	mcu_rtc_init(&ssp_rtc);
#else
	mcu_rtc_init(true);
#endif
#endif

#if MODULE_ENABLE_DEBUG_CONSOLE && MODULE_ENABLE_CONSOLE
	if(COMM_DEBUG != NULL)
	{
		debug_console_init(COMM_DEBUG);
		console_add_command(&_cmd);
	}
#endif

#if MODULE_ENABLE_SECURITY
	security_init();
#endif

#if MODULE_ENABLE_NETWORK && (!defined(NETWORK_SKIP_INIT_IN_SYS) || !NETWORK_SKIP_INIT_IN_SYS)
	network_interface_init();
#endif

	app_main_init();

#if MCU_ENABLE_FREERTOS 
	// On Free RTOS we do not follow main code here, we create a new task for the main code to be able to use idle task priority
	// Otherwise we might have to deal with task watchdog.
	xTaskCreate(_task_main, "sys_main", 4096, NULL, 10, NULL);
	vTaskDelete(NULL);
}

static void _task_main(void* arg)
{
#if MCU_TYPE == MCU_ESP32 && MCU_PERIPHERY_ENABLE_WATCHDOG
	mcu_watchdog_init(NULL);
#endif
#endif

#if MODULE_ENABLE_SWUPDATE && MODULE_ENABLE_MMC && SW_UPDATE_MMC_ON_STARTUP
	{
		uint16_t file_sw_version;

		if(sw_update_check_available_from_sd(&file_sw_version))
		{
			if(version_get_numeric() != file_sw_version) // Update is only available if the sw version is different.
			{
				dbg_printf(DBG_STRING, "Update %d -> %d\n", version_get_numeric(), file_sw_version);
	#if DBG_USE_MMC_LOG
				dbg_new_file(); // Flush log!
	#endif
				sw_update_from_sd(false); // Update and do not delete firmware file.
			}
		}
	}
#endif

#if MCU_TYPE == PC_EMU
	while(!_stop_execution)
#else
	while(true)
#endif
	{
#if SYSTEM_ENABLE_APP_MAIN_HANDLE
		app_main_handle();
#endif
#if MCU_ENABLE_FREERTOS 
		taskYIELD();
#endif
		_handle();
	}
#if defined(KERNELTEST) || defined(ESOPUBLICTEST)
	return;
#elif _IS_ST() || MCU_TYPE == PC_EMU
	return 0;
#endif
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

uint32_t system_get_tick_count(void)
{
#if MCU_TYPE == PC_EMU
#if defined(_WIN32)
	return GetTickCount() - sys_msec_counter;
#elif defined(__linux__)
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
#endif
#elif _IS_ST()
	return HAL_GetTick();
#elif MCU_TYPE == MCU_ESP32
	return (uint32_t) (xTaskGetTickCount() * portTICK_PERIOD_MS);
#else
	return sys_msec_counter;
#endif
}


void system_task_init_handle(system_task_t* task, bool add_to_tasklist, system_task_cb_handle_t f, void* obj)
{
	// Before initializing, remove the task from the list.
	system_task_remove(task);

	task->type = SYSTEM_TASK_TYPE_HANDLE;
	task->protothread.obj = obj;
	task->f_handle = f;
	task->next_task = NULL;
	task->is_active = false;
	PT_INIT(&task->protothread);

#if SYSTEM_ENABLE_DEBUG_PRINTS
	DBG_INFO("Task init handle: Task=%08x Name=%s f=%08x obj=%08x\n", task, task->name ? task->name : "NoName", f, obj);
#endif

	if(add_to_tasklist)
		system_add_task(task);
}

void system_task_init_protothread(system_task_t* task, bool add_to_tasklist, system_task_cb_protothread_t f, void* obj)
{
	// Before initializing, remove the task from the list.
	system_task_remove(task);

	task->type = SYSTEM_TASK_TYPE_PROTOTHREAD;
	task->protothread.obj = obj;
	task->f_pt = f;
	task->next_task = NULL;
	task->is_active = false;
	PT_INIT(&task->protothread);

#if SYSTEM_ENABLE_DEBUG_PRINTS
	DBG_INFO("Task init pt: Task=%08x Name=%s f=%08x obj=%08x\n", task, task->name ? task->name : "NoName", f, obj);
#endif

	if(add_to_tasklist)
		system_add_task(task);
}

void system_task_set_name(system_task_t* task, const char* name)
{
	if(task == NULL)
		return;

	task->name = name;
}

void system_task_add(system_task_t* task)
{
	if(task == NULL || task->is_active)
		return;

    task->is_active = true;
    PT_INIT(&task->protothread);

	if(_first_task == NULL)
	{
		_first_task = task;
#if SYSTEM_ENABLE_DEBUG_PRINTS
		DBG_INFO("Task add [Task=%08x Name=%s]\n", task, task->name ? task->name : "NoName");
#endif
	}
	else
	{
		// Task is not null and there is already at least one task inside the task list.
		system_task_t* tmp = _first_task;

		while(tmp->next_task != NULL)
		{
			// If task was already added -> just return because it is inside.
			if(tmp == task)
			{
#if SYSTEM_ENABLE_DEBUG_PRINTS
			    DBG_INFO("Task already added\n");
#endif
				return;
			}

			tmp = (system_task_t*)tmp->next_task;
		}
#if SYSTEM_ENABLE_DEBUG_PRINTS
		DBG_INFO("Task add [Task=%08x Name=%s] to [Task=%08x Name=%s]\n", task, task->name ? task->name : "NoName", tmp, tmp->name ? tmp->name : "NoName");
#endif
		tmp->next_task = task;
		task->next_task = NULL; // might be re-added and containing old next task!
	}
}

void system_task_remove(system_task_t* task)
{
	if(task == NULL || _first_task == NULL)
		return;

	task->is_active = false;

	if(task == _first_task)
	{
#if SYSTEM_ENABLE_DEBUG_PRINTS
		DBG_INFO("Task remove first [Task=%08x Name=%s]\n", task, task->name ? task->name : "NoName");
#endif			
		_first_task = (system_task_t*)_first_task->next_task;

		if(task->f_remove)
			task->f_remove(task);

		_free_subtasks(&task->protothread);
	}
	else
	{
		system_task_t* tmp = _first_task;

		// Search for the task
		while(tmp->next_task != task && tmp->next_task != NULL)
			tmp = (system_task_t*)tmp->next_task;

		// If task was found remove it.
		if(tmp->next_task == task)
		{
			tmp->next_task = task->next_task;
#if SYSTEM_ENABLE_DEBUG_PRINTS
			DBG_INFO("Task remove [Task=%08x Name=%s]\n", task, task->name ? task->name : "NoName");
#endif			
			if(task->f_remove)
				task->f_remove(task);
		}
		_free_subtasks(&task->protothread);
	}

	task->next_task = NULL;
}

bool system_task_is_active(system_task_t* task)
{
    if(task)
        return task->is_active;

    return false;
}

#if SYSTEM_ENABLE_PRINT_STATISTIC
void system_task_print_statistic(comm_t* comm)
{
	system_task_t* tmp = _first_task;
	uint16_t cnt = 0;

	comm_printf(comm, "Task List:\n");

	while(tmp != NULL)
	{
		struct pt* pt_sub = tmp->protothread.sub_pt;

#if PT_ENABLE_ENHANCED_DEBUG
		comm_printf(comm, "- Task 0x%08x[%s / %s] - Function %s[0x%08x] - Object 0x%08x - LC %d\n",
				tmp,
				tmp->name ? tmp->name : "NoName",
				_get_filename(tmp->protothread.filename),
				tmp->protothread.function,
				tmp->f_handle,
				tmp->protothread.obj,
				tmp->protothread.lc);
		while(pt_sub)
		{
			comm_printf(comm, " - File %s Function %s LC %d\n", _get_filename(pt_sub->filename), pt_sub->function, pt_sub->lc);
			pt_sub = pt_sub->sub_pt;
		}
#else
	    comm_printf(comm, "- Task 0x%08x[%s] - Function 0x%08x - Object 0x%08x - LC %d\n",
	    		tmp,
				tmp->name ? tmp->name : "NoName",
				tmp->f_handle,
				tmp->protothread.obj,
				tmp->protothread.lc);
	    while(pt_sub)
		{
			comm_printf(comm, " - LC %d\n", pt_sub->lc);
			pt_sub = pt_sub->sub_pt;
		}
#endif

		tmp = tmp->next_task;
		cnt++;
	}

	comm_printf(comm, "Number of Tasks: %d\n", cnt);
}
#endif

#if SYSTEM_ENABLE_MONITORING
void system_task_recording_start(void)
{
	_num_recordings = 0;
}

void system_task_recording_stop(void* comm)
{
	comm_printf(comm, "Recordings: %d (%d MAX)\n", _num_recordings, _NUM_RECORDINGS);
	for(int i = 0; i < _num_recordings && i < _NUM_RECORDINGS; i++)
	{
		_recording_t* r = &_recordings[i];
		comm_printf(comm, "[%d]: %s[%08x] h=%08x o=%08x line %d->%d\n",
				r->timestamp,
				r->task->name ? r->task->name : "(NO NAME)",
				r->task,
				r->task->f_handle,
				r->task->protothread.obj,
				r->line_before,
				r->line_after);
	}
}
#endif

#if SYSTEM_ENABLE_SLEEP_MODE

void system_sleep_mode_enable(uint32_t sleep_duration)
{
	_sleep_mode_control_active = (sleep_duration > 0);
	_sleep_mode_time_ms = sleep_duration;
}

void system_sleep_prevention_set_flag(system_prevention_flag_t flag)
{
	if((_prevention_used_flags & flag) == flag)
		_prevention_active_flags |= flag;

//	dbg_printf(DBG_STRING, "system_set_sleep_prevention_flag(%08x) -> %08x\n", flag, _prevention_active_flags);
}

void system_sleep_prevention_clear_flag(system_prevention_flag_t flag)
{
	if((_prevention_used_flags & flag) == flag)
		_prevention_active_flags &= flag;

//	dbg_printf(DBG_STRING, "system_clear_sleep_prevention_flag(%08x) -> %08x\n", flag, _prevention_active_flags);
}

system_prevention_flag_t system_sleep_prevention_register_flag(void)
{
	uint8_t i = 0;
	for(i = 0; i < 32; i++)
	{
		if((_prevention_used_flags & (system_prevention_flag_t)(1 << i)) == 0)
		{
			_prevention_used_flags |= (system_prevention_flag_t)(1 << i);
//			dbg_printf(DBG_STRING, "system_register_for_sleep_prevention -> %08x\n", (1 << i));
			return (system_prevention_flag_t)(1 << i);
		}
	}
	return 0;
}

void system_sleep_prevention_unregister_flag(system_prevention_flag_t flag)
{
	_prevention_used_flags &= flag;
	_prevention_active_flags &= flag;
}
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static bool _init(void)
{
	if(system_initialized)
		return true;

#if MCU_TYPE == PC_EMU && (defined(_WIN32) || defined(__CYGWIN__))
	sys_msec_counter = GetTickCount();

#elif !_IS_ST() && MCU_TYPE != MCU_ESP32
	sys_msec_counter = 0;
#endif

#if _NEED_TIMER_FOR_MS()
#if MCU_TYPE == RSYNERGY
	sys_timer.p_api->open(sys_timer.p_ctrl, sys_timer.p_cfg);
	mcu_wait_timer.p_api->open(mcu_wait_timer.p_ctrl, mcu_wait_timer.p_cfg);
#else
	sys_timer = mcu_timer_init(MCU_INT_LVL_HI, 1000, system_timer_millisec, NULL, true);

	if(sys_timer==NULL)
		return false;
#endif
#endif

	system_initialized = true;

	return true;
}

static void _handle(void)
{
#if SYSTEM_DEBUG_TASK_TIME_MS
	static uint32_t timestamp = 0;
#endif
	system_task_t* tmp = _first_task;
#if SYSTEM_ENABLE_MONITORING
	uint16_t line_pre = 0;
#endif

	while(tmp != NULL)
	{
		if(tmp->f_handle)
		{
#if SYSTEM_DEBUG_TASK_TIME_MS
			uint32_t diff = 0;
			timestamp = system_get_tick_count();
#endif
#if SYSTEM_ENABLE_MONITORING
			line_pre = tmp->protothread.lc;
#endif
			switch(tmp->type)
			{
				case SYSTEM_TASK_TYPE_HANDLE:
					tmp->f_handle(tmp->protothread.obj);
				break;

				case SYSTEM_TASK_TYPE_PROTOTHREAD:

					if(!PT_SCHEDULE(tmp->f_pt(&tmp->protothread)))
					{
						system_task_t* tmp2 = tmp;

						// Protothread ended -> Remove Task!
						system_task_remove(tmp);

						// Set current task to the last task because the current task is removed!
						tmp = (system_task_t*)tmp2;
					}

				break;
			}
#if SYSTEM_ENABLE_MONITORING
			_add_recording(tmp, line_pre);
#endif
#if SYSTEM_DEBUG_TASK_TIME_MS
			diff = system_get_tick_count() - timestamp;
			if(diff >= SYSTEM_DEBUG_TASK_TIME_MS)
			{
				dbg_printf(DBG_STRING, "%s[0x%08x] -> %dms\n", tmp->name, tmp->f_pt, diff);
			}
#endif
		}
        tmp = (system_task_t*)tmp->next_task;
	}

#if MCU_TYPE == PC_EMU  && (defined(_WIN32) || defined(__CYGWIN__))
	if(windows_get_exit_key())
	{
		_stop_execution = true;
	}
	{		
		static int cnt = 0;

		cnt++;

		if(cnt >= 100)
		{
			cnt = 0;
			windows_sleep(1); //1 ms
		}
	}
#endif

#if SYSTEM_ENABLE_SLEEP_MODE
	// Enter sleep mode for the defined time if it is enabled and no prevention flag is set.
	if(_sleep_mode_control_active && 0 == _prevention_active_flags)
	{
		uint32_t slept_ms = mcu_enter_sleep_mode(_sleep_mode_time_ms);
#if !_IS_ST()
		sys_msec_counter += slept_ms;
#endif
	}
#endif
}

static void _free_subtasks(struct pt* pt)
{
	if(pt->sub_pt)
	{
		_free_subtasks(pt->sub_pt);
		free(pt->sub_pt);
		pt->sub_pt = NULL;
	}
}

#if PT_ENABLE_ENHANCED_DEBUG
static char* _get_filename(const char* name)
{
    if(name == NULL)
        return "NULL";

    char* ptr = strrchr(name, '/');
    if(ptr)
        return ptr + 1;

    ptr = strrchr(name, '\\');
    if(ptr)
        return ptr + 1;

    return ptr;
}
#endif

#if MODULE_ENABLE_DEBUG_CONSOLE && MODULE_ENABLE_CONSOLE
static FUNCTION_RETURN _console(console_data_t* data, char** args, uint8_t args_len)
{
    bool parameter_is_invalid = false;

    if(args_len == 1 && strcmp(args[0], "print") == 0)
    {
#if SYSTEM_ENABLE_PRINT_STATISTIC
        system_task_print_statistic(data->comm);
		return console_set_response_static(data, FUNCTION_RETURN_OK, "");
#else
		return console_set_response_static(data, FUNCTION_RETURN_OK, "Printing the statistic is not enabled");
#endif
    }
    else
        parameter_is_invalid = true;

    if(parameter_is_invalid)
		return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Parameter invalid");
	return console_set_response_static(data, FUNCTION_RETURN_NOT_FOUND, "");
}
#endif

#if SYSTEM_ENABLE_MONITORING
static void _add_recording(system_task_t* t, uint16_t line_pre)
{
	if(_num_recordings < _NUM_RECORDINGS)
	{
		_recordings[_num_recordings].timestamp = system_get_tick_count();
		_recordings[_num_recordings].task = t;
		_recordings[_num_recordings].line_after = t->protothread.lc;
		_recordings[_num_recordings].line_before = line_pre;
		_num_recordings++;
	}
}
#endif

#if _NEED_TIMER_FOR_MS()
#if MCU_TYPE == RSYNERGY
void system_timer_millisec(timer_callback_args_t *p_args)
#else
static void system_timer_millisec(void* obj)
#endif
{
	sys_msec_counter++;	// Increments and might overflow.
}
#endif
