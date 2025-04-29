// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file dbg.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Contains functions for debugging that should be used by all modules.
 *			When using this functions, a timestamp of the millisecond counter and
 *
 *	@version	1.10 (19.01.2022)
 * 				 - Modified to be used in esopekernel
 *	@version	1.09 (23.11.2021)
 * 		- Added DBG_ASSERT
 *	@version	1.08 (09.11.2021)
 *		- Added TCP support for debugging.
 * 		- Remove dbg_none
 *	@version	1.07 (15.02.2020)
 *		- Added check for free rtos to add semaphore
 *	@version	1.06 (30.11.2018)
 *		- Added dbg_nop for DBG_INFO, etc.
 *	@version	1.05 (13.11.2018)
 *		- Solved bug where the millisecond prints are destroyed when a new debug file is opened
 *	@version	1.04 (06.08.2018)
 *		- Added automatic disable of Devlog if no mmc is enabled
 *	@version	1.03 (06.08.2018)
 *		- Added Debug Level for DBG_INFO, DBG_VERBOSE and DBG_ERROR -> Needs C99!
 *	@version	1.02 (07.06.2018)
 *		- Added module.h support
 *		- Corrected devlog name
 *		- Used rtc_get_time instead of mcu_rtc_get_time
 *		- Ready for new mmc module (FF_USE_LFN instead of _USE_LFN).
 *	@version	1.01 (24.05.2018)
 *		- Added LFN support (needed for devlog)
 *		- Added devlog subdirectories for year/month
 *		- Added devlog continue flag in filename
 *		- dbg_printf with two dbg_string parts instead of one -> less rom usage
 *		- Added _DBG_STRING_HIDE_PATH
 *  @version	1.00 (04.01.2015)
 *  	- Intial release -> Contains dbg_ functions from the comm.h 2.05
 *  	- Added possibility to use sd card for saving.
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef _DBG_H_
#define _DBG_H_

#include "module_public.h"
#if MODULE_ENABLE_COMM

#include "comm.h"
#include "module/convert/string.h" // Needed for strrchr
#include "module/enum/function_return.h"
#include <stdarg.h>
#include "module/util/assert.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Debug levels
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the comm module
#define DBG_STR_VERSION		"1.10"

/// Used as alternative for comm_debug. The upper letters make it more important.
#define COMM_DEBUG	comm_debug

// Default configuration for the dbg_tcp_config_s structure.
#define DBG_DEFAULT_TCP_CONFIG() \
{\
	.port = 56893, \
	.buffer_rx_fifo = 1024, \
	.buffer_rx_socket = 1024, \
	.buffer_tx = 4096 \
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Macros for debug string
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Returns the value of x
#define LINE2STRING_(x) #x

/// Used to make Line as a string inside DBG_STRING
#define LINE2STRING(x) LINE2STRING_(x)

/**
 * Debug String for printing the current position inside the Project with the format 'Filename, Line: '
 * The debug can be used with xprintf or xputs like @code xprintf(COMM_DEBUG, DBG_STRING "Test\n"); @endcode.
 * In the dbg_printf function it should be used as the first parameter. dbg_printf(DBG_STRING, "Test\n");
 **/
#if MCU_TYPE == PC_EMU
#define DBG_STRING		\
	strrchr((const char*)("\\" __FILE__), '\\') + 1, (const char*)(LINE2STRING(__LINE__))
#else
#define DBG_STRING		\
	strrchr((const char*)("/" __FILE__), '/') + 1, LINE2STRING(__LINE__)
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Debug print macros
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Prints nothing
#define DBG_NONE(...)			{}

#if DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE
// Macro that can be used instead of dbg_printf(DBG_STRING, "", ..);
#if MCU_TYPE == PC_EMU
#define DBG_VERBOSE(str, ...)		dbg_printf(DBG_STRING, (char*)str, ##__VA_ARGS__)
#else
#define DBG_VERBOSE(...)		dbg_printf(DBG_STRING, __VA_ARGS__)
#endif
#else
#define DBG_VERBOSE(...)		do{}while(0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
// Macro that can be used instead of dbg_printf(DBG_STRING, "", ..);
#if MCU_TYPE == PC_EMU
#define DBG_INFO(str, ...)		dbg_printf(DBG_STRING, (char*)str, ##__VA_ARGS__)
#else
#define DBG_INFO(...)		dbg_printf(DBG_STRING, __VA_ARGS__)
#endif
#else
#define DBG_INFO(...)			do{}while(0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
// Macro that can be used instead of dbg_printf(DBG_STRING, "", ..);
#if MCU_TYPE == PC_EMU
#define DBG_ERROR(str, ...)		dbg_printf(DBG_STRING, (char*)str, ##__VA_ARGS__)
#else
#define DBG_ERROR(...)		dbg_printf(DBG_STRING, __VA_ARGS__)
#endif
#else
#define DBG_ERROR(...)			do{}while(0)
#endif

#if !MODULE_ENABLE_MMC && defined(DBG_USE_MMC_LOG)
#undef DBG_USE_MMC_LOG
/// No MMC is enables, so disable MMC logs. DO NOT CHANGE THIS VALUE TO TRUE!
#define DBG_USE_MMC_LOG 						false
#endif

/**
 * @brief Macro for asserting a certain boolean expression. If this expression is not met, the error message m is printed and the given return value r is returned.
 * @param b     Boolean expression to evaluate
 * @param a     Action to execute before returning (e.g. free a buffer). Keep empty to do nothing
 * @param r     Return value when expression is not met
 * @param ...   Error message when expression is not met. Uses the variable argument list for the error printing.
 */
#define DBG_ASSERT(b, a, r, ...)        \
    if(!(b)) \
    {\
        DBG_ERROR(__VA_ARGS__); \
        a; \
        return r; \
    }

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if DBG_USE_TCP

/// Configuration data for the tcp debug interface.
typedef struct dbg_tcp_config_s
{
	/// Port of the TCP debugging interface
	uint16_t port;
	/// Size of the receive fifo
	size_t buffer_rx_fifo;
	/// Size of the buffer used to read data from the socket before putting it into the fifo.
	size_t buffer_rx_socket;
	/// Size of the transmit buffer used to buffer data before sending it.
	size_t buffer_tx;
	/// Network interface to activate the tcp debugging on. When NULL, the default network interface is used.
	void* nwk;
}dbg_tcp_config_t;

#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

extern comm_t* comm_debug;	///< Pointer to the debug comm_t, which is set with comm_set_debug

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if DBG_USE_MMC_LOG
	/**
	 * Initializes a log file that is used for debug prints.
	 */
	void dbg_init_logfile(void);
#endif

#if DBG_USE_TCP
/**
 * @brief Initialized the TCP debugging on a certain local tcp server port. The buffer will be dynamically allocated by the given tx and rx size.
 * The debugging takes place using direct TCP connection. Do NOT enable it in an unsafe environment!
 * 
 * When a client connects to the TCP server, the debug comm is switched to the TCP connection instead of the default comm port.
 * 
 * @param config		Configuration for the tcp debug interface.
 */
FUNCTION_RETURN dbg_init_tcp(const dbg_tcp_config_t* config);
#endif

/**
 * @brief	Sets the comm_t for the debug device.
 *
 * 	The comm_t set by this function will be used for debug prints when comm_debug is used as a parameter or
 * 	when dbg_printf is called. If the comm_t is NULL the debug prints are disabled.
 *
 * @pre		The parameter h should be initialized before this function is called.
 *
 * @param h			Pointer to the comm_t used for debug outputs.
 **/
void dbg_set_comm(comm_t *h);

/**
 * @brief	Sends a formatted string to the device comm_debug is assigned to.
 *
 * 		Expands xprintf with filename, linenumber and the milliseconds since the program started.
 * 		The data is printed to comm_debug. So comm_set_debug should be called first.
 *
 * @pre		The comm_interface_t inside the comm_t needs to be initialized
 * @pre		The comm_t needs to be initialized
 *
 * @param dbg_string		Should be DBG_STRING, so that the correct File and Line is printed.
 * @param str				A string with the format defined in @link vxprintf vxprintf@endlink.
 * @param ...				Different parameters according to the format of str.
 **/
void 	dbg_printf(const char *dbg_string1, const char *dbg_string2, const char *str, ...);

/**
 * @brief	Sends a formatted string to the device comm_debug is assigned to.
 *
 * 		Expands xprintf with filename, linenumber and the milliseconds since the program started.
 * 		The data is printed to comm_debug. So comm_set_debug should be called first.
 *
 * @pre		The comm_interface_t inside the comm_t needs to be initialized
 * @pre		The comm_t needs to be initialized
 *
 * @param dbg_string		Should be DBG_STRING, so that the correct File and Line is printed.
 * @param str				A string with the format defined in @link vxprintf vxprintf@endlink.
 * @param vl				List with variable arguments according to the format of str.
 **/
void 	dbg_vprintf(const char *dbg_string1, const char *dbg_string2, const char *str, va_list vl);

#if DBG_USE_MMC_LOG
/**
 * @brief Returns the
 */
char*	dbg_get_curr_filename(void);

/**
 * @brief Closes the current logfile and creates a new log file to work with.
 */
void dbg_new_file(void);

#endif

#endif

#endif /* DBG_H_ */
