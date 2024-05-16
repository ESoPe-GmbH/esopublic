/**
 * @file sys_config.h
 * @author Tim Koczwara (tim.koczwara@esope.de)
 * @brief Selection of modules to use inside this application.
 * @version 1.0
 * @date 06.05.2022
 * 
 * @copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * 
 */

#ifndef __SYS_CONFIG_H_GUARD__
#define __SYS_CONFIG_H_GUARD__

/// If set to a value > 0, all Task that take longer than SYSTEM_DEBUG_TASK_TIME_MS milliseconds will be printed by their name.
#define SYSTEM_DEBUG_TASK_TIME_MS					0

/// Enables/disables use of app_main_handle
#define SYSTEM_ENABLE_APP_MAIN_HANDLE   			true

/// Enables / disables deprecated task functions.
#define SYSTEM_ENABLE_DEPRECATED_TASK_FUNCTIONS		true

/// Enable / disable the print statistic function
#define SYSTEM_ENABLE_PRINT_STATISTIC				true

/// Enable / disable sleep mode functions
#define SYSTEM_ENABLE_SLEEP_MODE					false

/// Enables/disables monitoring of tasks
#define SYSTEM_ENABLE_MONITORING					false

/// Enable/disable debug prints during init, add and remove
#define SYSTEM_ENABLE_DEBUG_PRINTS					false

#if SYSTEM_ENABLE_MONITORING
/// Number of recordings to store for monitoring the tasks
#define SYSTEM_MONITOR_NUM_RECORDINGS				100
#endif

#endif