// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file debug_console.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *
 *	@version	1.01 (19.01.2022)
 * 	    - Modified to be used in esopekernel
 *  @version	1.00 (02.10.2015)
 *  	- Initial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef DEBUG_CONSOLE_H_
#define DEBUG_CONSOLE_H_

#include "module_public.h"

#if MODULE_ENABLE_DEBUG_CONSOLE

#include "module/comm/comm.h"
#include "module/enum/function_return.h"
#include "module/console/console.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the debug console module
#define DEBUG_CONSOLE_STR_VERSION "1.01"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if DEBUG_CONSOLE_ENABLE_UART
typedef struct debug_console_uart_s debug_console_uart_t;

struct debug_console_uart_s
{
	mcu_uart_t uart;

	const char* name;

	uint8_t num;

	uint8_t* buffer;

	uint16_t buffer_size;

	debug_console_uart_t* next;
};
#endif

#if DEBUG_CONSOLE_ENABLE_TEST

typedef void(*debug_console_test_cb)(void* obj, console_data_t* data, char** args, uint8_t arg_len);

typedef struct debug_console_test_s debug_console_test_t;

struct debug_console_test_s
{
	debug_console_test_cb f;

	void* obj;

	debug_console_test_t* next;
};
#endif

typedef int (*debug_flag_cb_t)(const char*);

typedef bool (*debug_identification_cb_t)(void);
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes the receive for the debug uart and starts a task for this module.
 *
 * @param h_uart			Handle of the uart that should be used for the debug console.
 **/
void debug_console_init(comm_t* comm);

#if DEBUG_CONSOLE_ENABLE_UART

FUNCTION_RETURN debug_console_register_uart(debug_console_uart_t* dbu, mcu_uart_t uart, const char* name, uint8_t num, uint8_t* buffer, uint16_t buffer_size);
#endif

#if DEBUG_CONSOLE_ENABLE_TEST

FUNCTION_RETURN debug_console_register_test_callback(debug_console_test_t* dbt, void* obj, debug_console_test_cb f);
#endif

#if DEBUG_CONSOLE_ENABLE_FLAG

FUNCTION_RETURN debug_console_register_flag(const char* name, debug_flag_cb_t f);
#endif

#if DEBUG_CONSOLE_ENABLE_IDENTIFICATION

FUNCTION_RETURN debug_console_register_identification(debug_identification_cb_t f);
#endif

#endif

#endif /* DEBUG_CONSOLE_H_ */
