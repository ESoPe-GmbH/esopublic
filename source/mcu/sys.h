// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		sys.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		Core function of the project, contains the main file. You need to implement the following functions externally:
 *  				void app_main_init(void);
 *  					-> Is called to initialize all application specific settings. The board_init is called auomatically in the main here.
 *  				void app_main_handle(void);
 *  					-> Is called periodically from the main while. Use this if main file should handle something.
 *
 *  			This module is used to have a global millisecond counter that can be used in all modules. By using this, time relevant modules do not use a timer for themselves.
 *				Be careful, because the counter is 32-bit and might overflow.
 *
 *				Be also careful that you have enough timer declared in mcu_config.h! You should set it to at least 2 because the mcu has also a timer for the active wait!
 *
 *	@version	1.05 (12.03.2021)
 *				 - Merge of ESP, ST and PC compatibility.
 *				 - Rename of the functions for the task and sleep mode to make the naming compatible with the naming convention.
 *				 - Added deprecated task defines for supporting old modules that do not use the new naming convention.
 *  @version    1.04 (03.07.2019)
 *               - system_add_task calls PT_INIT always, even if task was already added to start the task at the beginning!
 *               - Added SYSTEM_ENABLE_APP_MAIN_HANDLE to enable/disable app_main_handle
 *	@version	1.03 (24.05.2018)
 *				 - Now contains main with functions app_main_init and app_main_handle that needs to be implemented externally.
 *	@version	1.02 (13.09.2014)
 *				 - Task structure and functions (init, add, remove, handle) added.
 *	@version	1.01 (21.08.2013)
 *				 - Translated comments to english
 *				 - Changed unsigned long system_get_tick_count(void) to uint32_t system_get_tick_count(void).
 *				 - Renamed system_timter_millisec to system_timer_millisec.
 *  @version	1.00 (19.09.2011)
 *  			 - Initial release
 *
 ******************************************************************************/

#ifndef SYSTEM_HEADER_FIRST_INCLUDE_GUARD
#define SYSTEM_HEADER_FIRST_INCLUDE_GUARD

#include "mcu.h"
#include "pt/pt.h"
#include "pt/pt-sem.h"

#if CONFIG_ENABLE_ESOPUBLIC

// Use config from KConfig settings

/// If set to a value > 0, all Task that take longer than SYSTEM_DEBUG_TASK_TIME_MS milliseconds will be printed by their name.
#define SYSTEM_DEBUG_TASK_TIME_MS					CONFIG_SYSTEM_DEBUG_TASK_TIME_MS
/// Enables/disables use of app_main_handle
#define SYSTEM_ENABLE_APP_MAIN_HANDLE				(defined(CONFIG_SYSTEM_ENABLE_APP_MAIN_HANDLE) && CONFIG_SYSTEM_ENABLE_APP_MAIN_HANDLE)
/// Enables / disables deprecated task functions.
#define SYSTEM_ENABLE_DEPRECATED_TASK_FUNCTIONS		(defined(CONFIG_SYSTEM_ENABLE_DEPRECATED_TASK_FUNCTIONS) && CONFIG_SYSTEM_ENABLE_DEPRECATED_TASK_FUNCTIONS)
/// Enable / disable the print statistic function
#define SYSTEM_ENABLE_PRINT_STATISTIC				(defined(CONFIG_SYSTEM_ENABLE_PRINT_STATISTIC) && CONFIG_SYSTEM_ENABLE_PRINT_STATISTIC)
/// Enable / disable sleep mode functions
#define SYSTEM_ENABLE_SLEEP_MODE					(defined(CONFIG_SYSTEM_ENABLE_SLEEP_MODE) && CONFIG_SYSTEM_ENABLE_SLEEP_MODE)
/// Enable/disable debug prints during init, add and remove
#define SYSTEM_ENABLE_DEBUG_PRINTS					(defined(CONFIG_SYSTEM_ENABLE_DEBUG_PRINTS) && CONFIG_SYSTEM_ENABLE_DEBUG_PRINTS)
/// Enables/disables monitoring of tasks
#define SYSTEM_ENABLE_MONITORING					(defined(CONFIG_SYSTEM_ENABLE_MONITORING) && CONFIG_SYSTEM_ENABLE_MONITORING)
/// Number of recordings to store for monitoring the tasks
#define SYSTEM_MONITOR_NUM_RECORDINGS				CONFIG_SYSTEM_MONITOR_NUM_RECORDINGS

#else // CONFIG_ENABLE_ESOPUBLIC

// Use sys_config.h from config
#include "sys_config.h"

#ifndef SYSTEM_DEBUG_TASK_TIME_MS
/// If set to a value > 0, all Task that take longer than SYSTEM_DEBUG_TASK_TIME_MS milliseconds will be printed by their name.
#define SYSTEM_DEBUG_TASK_TIME_MS					0
#endif

#ifndef SYSTEM_ENABLE_APP_MAIN_HANDLE
/// Enables/disables use of app_main_handle
#define SYSTEM_ENABLE_APP_MAIN_HANDLE   			true
#endif

#ifndef SYSTEM_ENABLE_DEPRECATED_TASK_FUNCTIONS
/// Enables / disables deprecated task functions.
#define SYSTEM_ENABLE_DEPRECATED_TASK_FUNCTIONS		true
#endif

#ifndef SYSTEM_ENABLE_PRINT_STATISTIC
/// Enable / disable the print statistic function
#define SYSTEM_ENABLE_PRINT_STATISTIC				true
#endif

#ifndef SYSTEM_ENABLE_SLEEP_MODE
/// Enable / disable sleep mode functions
#define SYSTEM_ENABLE_SLEEP_MODE					false
#endif

#ifndef SYSTEM_ENABLE_MONITORING
/// Enables/disables monitoring of tasks
#define SYSTEM_ENABLE_MONITORING					false
#endif

#ifndef SYSTEM_ENABLE_DEBUG_PRINTS
/// Enable/disable debug prints during init, add and remove
#define SYSTEM_ENABLE_DEBUG_PRINTS					false
#endif

#if SYSTEM_ENABLE_MONITORING
#ifndef SYSTEM_MONITOR_NUM_RECORDINGS
/// Number of recordings to store for monitoring the tasks
#define SYSTEM_MONITOR_NUM_RECORDINGS				100
#endif
#endif

#endif // CONFIG_ENABLE_ESOPUBLIC

#if SYSTEM_ENABLE_PRINT_STATISTIC
#include "module/comm/comm_type.h"
#endif

/**
 * @struct system_task_t
 *
 * This structure is used for calling handle functions of registered tasks.
 * If a module uses the task structure the main function does not need to call its handle function because
 * the handle will be automatically called inside the system_handle function.
 *
 * The variables inside the structure are private and must not be changed outside of the system functions!
 */
typedef struct system_task_s system_task_t;

/**
 * @fn void (*)(void*)
 * @brief Callback function for a single task handle.
 * The handle will be called periodically by using an user-defined parameter that is set during @ref system_task_init_handle.
 * @param 	User-defined pointer that is handled as a void pointer for compatibility.
 */
typedef void (*system_task_cb_handle_t)(void*);
/**
 * @fn int (*)(struct pt*)
 * @brief Callback function for a protothread task.
 * The protothread task will be called periocally until the protothread ends.
 * The user-defined parameter that is set in @ref system_task_init_protothread can be accessed by calling pt->obj.
 * @param 	Protothread structure pointer that needs to be used inf @ref PT_BEGIN, @ref PT_END and the other PT_ macros.
 * @return 	Protothread return value.
 */
typedef int (*system_task_cb_protothread_t)(struct pt*);
/**
 * @fn void (*)(struct system_task_s*)
 * @brief Callback function for a removed task.
 * The function will be called if not null when the task is removed from the task list. Use it to clear your resources.
 * The user-defined parameter that is set in @ref system_task_init_protothread can be accessed by calling pt->obj.
 * @param 	task Pointer to the removed task
 */
typedef void (*system_task_cb_remove_t)(system_task_t* task);

/**
 * @struct system_task_t
 *
 * This structure is used for calling handle functions of registered tasks.
 * If a module uses the task structure the main function does not need to call its handle function because
 * the handle will be automatically called inside the system_handle function.
 *
 * The variables inside the structure are private and must not be changed outside of the system functions!
 * The only variable you can modify are name and f_remove
 */
struct system_task_s
{
	/// Protothread structure. Must always be the first element inside the system_task_t!
	struct pt protothread;
	/// Type of the task
	uint8_t type;
	/// Pointer to a string containing the name of the task
	const char* name;
	/// Indicates whether task is currently active.
	bool is_active;
	/// Union for the task handle/protothread functions
	union
	{
		/// Pointer to the handle function.
		system_task_cb_handle_t f_handle;
		/// Pointer to the protothread function.
		system_task_cb_protothread_t f_pt;
	};
	/// Set this to have a callback when task is removed from task list.
	system_task_cb_remove_t f_remove;
	/// Internal Pointer to the next task. Is used to make a list of the tasks that will be handled inside system_handle.
	system_task_t* next_task;
};

typedef uint32_t system_prevention_flag_t;

/**
 * @brief 	Returns the time that passed since system_init was called in milliseconds.
 *
 * @pre	 system_init must be called before and it must have returned true or else this function will always return 0.
 *
 * @return			Milliseconds passed since system_init was called.
 **/
uint32_t system_get_tick_count(void);

/**
 * Initializes the task structure as a task for a handle that is called periodically with a user defined object as a parameter.
 * If add_to_tasklist is true then @see system_task_add is called at the end of the initialization.
 *
 * @pre		Make sure to initialize the task object to {0}, so it does not have some ram garbage inside.
 * @post	If add_to_tasklist is false, you need to add the task manually with @see system_task_add.
 * @param task					Pointer to the task structure that need to be initialized.
 * @param add_to_tasklist		true: Will add the task to the task list inside.
 * 								false: Will not add the task to the task list. If the task is already in the task list it will be removed from it.
 * @param f						Pointer to a function that is used as a handle. Function will be called periodically without any checks.
 * @param obj					Pointer to a user defined object that will be given to the handle as parameter.
 */
void system_task_init_handle(system_task_t* task, bool add_to_tasklist, system_task_cb_handle_t f, void* obj);
/**
 * Initializes the task structure as a task for a protothread task that is called periodically until the protothread ends.
 * The obj can be accessed inside the task by calling pt->obj.
 * If add_to_tasklist is true then @see system_task_add is called at the end of the initialization.
 *
 * @pre		Make sure to initialize the task object to {0}, so it does not have some ram garbage inside.
 * @post	If add_to_tasklist is false, you need to add the task manually with @see system_task_add.
 * @param task					Pointer to the task structure that need to be initialized.
 * @param add_to_tasklist		true: Will add the task to the task list inside.
 * 								false: Will not add the task to the task list. If the task is already in the task list it will be removed from it.
 * @param f						Pointer to a function that is used as a handle. Function will be called periodically without any checks.
 * @param obj					Pointer to a user defined object that will be given to the handle as parameter.
 */
void system_task_init_protothread(system_task_t* task, bool add_to_tasklist, system_task_cb_protothread_t f, void* obj);

/**
 * @brief	Set a name for the task.
 *
 * @param task					Pointer to the task structure.
 * @param name					Pointer to a string containing the name. Should be constant because only the pointer is kept. No data is copied.
 */
void system_task_set_name(system_task_t* task, const char* name);

/**
 * @brief	Adds the task to the task list. If add_to_tasklist in the init function was set to true, this function does not need to be called.
 * 			If the task is already inside the task list or if task is NULL, nothing happens.
 *
 * @param task					Pointer to the task structure that need to be added.
 */
void system_task_add(system_task_t* task);

/**
 * @brief	Removes the task from the task list.
 * 			If the task is not inside the task list or if task is NULL, nothing happens.
 *
 * @param task					Pointer to the task structure that need to be removed.
 */
void system_task_remove(system_task_t* task);

/**
 * Indicates whether the task is currently added to task list for handling inside the main loop.
 * @param task      Pointer to the task
 * @retval true     Task is handled in list.
 * @retval false    Task is not handled in list.
 */
bool system_task_is_active(system_task_t* task);
#if SYSTEM_ENABLE_PRINT_STATISTIC
/**
 * Prints information about all open tasks to the comm interface.
 *
 * @param comm		Pointer to the comm interface to print the information on.
 */
void system_task_print_statistic(comm_t* comm);
#endif

#if SYSTEM_ENABLE_MONITORING
// TODO: Documentation
void system_task_recording_start(void);
// TODO: Documentation
void system_task_recording_stop(void* comm);
#endif

#if SYSTEM_ENABLE_SLEEP_MODE
/**
 * Enables the use of the sleep mode control. This will enable an automatic sleep mode entering at the end of system_handle
 * when no sleep_prevention_flag is set with @ref system_set_sleep_prevention_flag. If sleep_duration is set to 0, this function
 * will not enable the sleep mode control.
 *
 * @param sleep_duration	Time in milliseconds for which the mcu sleeps after system_handle if no sleep prevention flag is set.\n
 * 							Sleep mode control is deactivated if the value is 0.
 */
void system_sleep_mode_enable(uint32_t sleep_duration);
/**
 * Prevents the @ref system_handle from entering the sleep mode until all prevention flags are cleared with @ref system_clear_sleep_prevention_flag.
 *
 * @param flag				Sleep prevention flag that was registered with @ref system_register_for_sleep_prevention.
 */
void system_sleep_prevention_set_flag(system_prevention_flag_t flag);
/**
 * Enables the @ref system_handle for entering the sleep mode if all flags that were set with @ref system_set_sleep_prevention_flag
 * were cleared with this function.
 *
 * @param flag				Sleep prevention flag that was registered with @ref system_register_for_sleep_prevention.
 */
void system_sleep_prevention_clear_flag(system_prevention_flag_t flag);
/**
 * Registers for a sleep prevention flag. If not all sleep prevention flags are registered, the next free sleep prevention flag
 * will be returned.
 *
 * @return					Next free sleep prevention flag or 0 if there is no free sleep prevention flag.
 */
system_prevention_flag_t system_sleep_prevention_register_flag(void);
/**
 * Unregisters a registered sleep prevention flag that was returned with @ref system_register_for_sleep_prevention.
 *
 * @param flag				Sleep prevention flag that was registered with @ref system_register_for_sleep_prevention.
 */
void system_sleep_prevention_unregister_flag(system_prevention_flag_t flag);
#endif

#if SYSTEM_ENABLE_DEPRECATED_TASK_FUNCTIONS

///@deprecated Use @ref system_task_init_handle instead. Same parameters, only additional NULL for the user object.
#define system_init_void_task(t, b, f)						system_task_init_handle(t, b, f, NULL)
///@deprecated Use @ref system_task_init_handle instead. Same parameters.
#define system_init_object_task(t, b, f, o)					system_task_init_handle(t, b, f, o)
///@deprecated Use @ref system_task_init_protothread instead. Same parameters, only additional NULL for the user object.
#define system_init_protothread_task(t, b, f)				system_task_init_protothread(t, b, f, NULL)
///@deprecated Use @ref system_task_init_protothread instead. Same parameters.
#define system_init_protothread_object_task(t, b, f, o)		system_task_init_protothread(t, b, f, o)
///@deprecated Use @ref system_task_add instead.
#define system_add_task										system_task_add
///@deprecated Use @ref system_task_remove instead.
#define system_remove_task									system_task_remove

#endif

#endif
