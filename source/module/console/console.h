// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file console.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Console module is used to handle applications via UART. Like a console on linux or DOS, applications
 *			can be registered which take execution commands like "list" and are executed afterwards. To make this happen,
 *			three informations are needed per application. First is the execution command itself. Second is a function callback
 *			which is called with the complete received line as a parameter. Third is a description for the application that is
 *			shown when help is entered.
 *			Example:
	@code
	static console_command_t _command;

	_command.command = "my_app";
	_command.fnc_exec = my_app_function;
	_command.explanation = "My app has the subcommands test1 and test2.\n-test1 Executes function 1.\n-test2 Executes function 2.";

	console_add_command(&_command);

	@endcode
 *
 *	@version	1.08 (17.01.2023)
 * 	    - Added CONSOLE_ASSERT_DYNAMIC and CONSOLE_ASSERT_STATIC
 *	@version	1.07 (19.01.2022)
 * 	    - Modified to be used in esopekernel
 *	@version 	1.06 (15.12.2021)
 * 		- Added console_can_receive
 * 	@version	1.05 (13.01.2019)
 * 		- Added hold callback set function
 * 		- Added byte num callback for receiving pre-defined number of bytes without console.
 *	@version	1.04 (17.04.2019)
 *		- Added CRC
 *	@version	1.03 (20.09.2018)
 *		- Made console_add command and console_remove_command globally accessible for all consoles.
 *	@version	1.02 (06.08.2018)
 *		- Made console_data_t externally to enable multiple console support if needed on different comm interfaces.
 *	@version	1.01 (07.06.2018)
 *		- Added module.h support
 *  @version	1.00 (02.10.2015)
 *  	- Initial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "module_public.h"
#if MODULE_ENABLE_CONSOLE
#include "module/comm/comm.h"
#include "module/enum/function_return.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Define
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the uart_tls module
#define CONSOLE_STR_VERSION "1.08"

#if CONSOLE_ENABLE_CRC
#include "module/crc/crc.h"
#endif

#ifndef NO_ACTION
/// Nothing done
#define NO_ACTION
#endif

/**
 * @brief Macro for asserting a certain boolean expression. If this expression is not met, the error is returned as a dynamic console response m is printed and the given return value r is returned.
 * @param b     Boolean expression to evaluate
 * @param a     Action to execute before returning (e.g. free a buffer). Keep empty to do nothing
 * @param r     Value that this function returns when the message can be set.
 * @param n		Maximum buffer len to allocate in heap for the message. 
 * 				If set to 0, you can send a dynamically allocated buffer in format, that already is completely built. 
 * 				It will then be sent without checking the format and afterwards the buffer will be freed. 
 * 				If the buffer is a static buffer, use CONSOLE_ASSERT_STATIC instead!
 * @param ...   Variable data that can be used for the format string.
 */
#define CONSOLE_ASSERT_DYNAMIC(data, b, a, r, n, ...)        \
    if(!(b)) \
    {\
        a; \
        return console_set_response_dynamic(data, r, n, __VA_ARGS__); \
    }
	
/**
 * @brief Macro for asserting a certain boolean expression. If this expression is not met, the error is returned as a static console response m is printed and the given return value r is returned.
 * @param b     Boolean expression to evaluate
 * @param a     Action to execute before returning (e.g. free a buffer). Keep empty to do nothing
 * @param r     Value that this function returns when the message can be set.
 * @param msg   Pointer to the message that should be sent in the response. 
 * 				Do not let this point to a local buffer in the stack, as this won't work, since content is not copied, but pointer is used.
 */
#define CONSOLE_ASSERT_STATIC(data, b, a, r, msg)        \
    if(!(b)) \
    {\
        a; \
        return console_set_response_static(data, r, msg); \
    }

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @struct console_command_t
 * @brief Structure that is used to define a command line command. It consists of the command, an execution function and an explanation.
 */

typedef struct
{
	/// Pointer to the command string e.g. "help"
	char* command;

	/// Pointer to the execution function of the command that takes the following of the command string
	/// (without a following space) as a parameter if use_array_param is true.
	/// If command is "help" and entered line is "help me", only "me" is the parameter of the execution function.
	/// If use_array_param is false, and the line for "help" is "help 1 2" the parameter is an array of the "1" and "2"
	/// strings with a second parameter that shows the number of arguments (2 in this case).
	/// A FUNCTION_RETURN is expected in both cases as a result. If FUNCTION_RETURN_OK is returned, console does nothing. Otherwise it will print
	/// the error using "!err" as a prefix.
	void* fnc_exec;

	/// Is used to define the parameter type of fnc_exec.
	/// If it is set to true, the parameter of fnc_exec will contain an array of the command arguments and the number of arguments.
	/// If it is set to false, the parameter is the line starting after command and the following spaces.
	bool use_array_param;

	/// Pointer to the explanation string for the command e.g. "This is the help command".
	char* explanation;

	/// Pointer to the next console_command_t. Used to build a list.
	/// Do NOT change the value of this variable externally!
	void* next;

}console_command_t;

/// Make the console data a type
typedef struct console_data_s console_data_t;
/**
 * Callback function for byte callbacks if a command needs to take over the complete uart receive.
 * @param b		Received byte
 */
typedef void (*console_cb_byte)(console_data_t* data, uint8_t b, bool is_end);
/**
 * Callback function that can be used to stop receiving on the console.
 * @param data				Pointer to the console data.
 */
typedef bool (*console_cb_hold)(console_data_t* data);

/**
 * @struct console_data_t
 * @brief Structure that is used to define a command line command.
 *
 * It consists of the command, an execution function and an explanation.
 */
struct console_data_s
{
	/// Task for the system handler.
	system_task_t 				task;

	/// Set this externally to a hold function that stops receiving on the UART while this function returns true.
	console_cb_hold 			f_hold;

	/// Set this to true externally when a received line should be printed on debug interface.
	bool                        debug_line;

    /// Set this to true if the console should not send invalid command
    bool                        suppress_invalid_command;

    /// Set this to use a custom response prefix instead of "<cmd>err" for responses. If null, the default "<cmd>err" will be used.
    // const char*					custom_response_prefix;

    /// If not NULL, point this to an array that is used for whitelisted commands. All received commands not matching the whitelist will be ignored.
    const char**				command_whitelist;

    /// Number of commands in whitelist. Has to be > 0 when command_whitelist is not NULL.
    int 						num_whitelist;

	/// Handle of the uart to use
	comm_t*						comm;

#if MCU_TYPE == MCU_ESP32
	/// Buffer for the currently received line. Is allocated when console is added and removed when console is stopped.
	char*						line_buffer;
#else
	/// Buffer for the currently received line.
	char 						line_buffer[CONSOLE_LINE_BUFFER_SIZE];
#endif

#if CONSOLE_ENABLE_CRC
	/// CRC of the line.
	crc_t						crc;

	/// Last calculated crc value
	uint16_t 					crc_value;

	/// Is set after calculating the crc by comparing it with the last 4 characters received.
	bool 						crc_valid;
#endif

	/// Counter for the current content of the line buffer.
	uint16_t					line_cnt;

	///	Timestamp used to drop incomplete commands.
	uint32_t 					timestamp;

	/// Callback function for bytes when a command needs the uart reception for itself
	console_cb_byte				cb_byte;
	/// Byte timeout after which the callback function is cleared
	uint32_t					timeout_ms_cb_byte;
	/// Escape byte on which the callback function is cleared.
	uint8_t						escape_character_cb_byte;
	/// Number of characters to receive directly.
	uint32_t 					byte_num_cb;

	/// Is set in console_set_response_x to point to the buffer containing the message to send. 
	/// Can point to buffer in stack or heap. If it is in heap, the message_heap is set and the message buffer is freed after sending the response.
	char* 						message;
	/// If message is allocated dynamically, the variable is set to free message after sending the message.
	bool 						message_heap;
	/// Is set when console_set_response is called. Otherwise no response is sent automatically.
	bool 						has_response;

	/// Can be set inside a console callback if the return value is FUNCTION_RETURN_OK. This is printed then as an argument to the response.
	/// If this is not set, nothing will be automatically sent as a response. Only NULL will prevent sending, an empty string will trigger sending a response.
	// const char*					success_message;

	/// Can be set inside a console callback if the return value is not FUNCTION_RETURN_OK. This is printed then as an argument to !err.
	// const char*					err_message;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes the receive for the console uart and starts a task for this module.
 *
 * @param data				Pointer to the console data.
 * @param h_uart			Handle of the uart that should be used for the console.
 **/
void console_init(console_data_t* data, comm_t* comm);
/**
 * Stops a console and it's task
 * @param data				Pointer to the console data.
 */
void console_stop(console_data_t* data);
/**
 * @brief	Add an application command to the console. Fill the data into cmd_obj before calling this function.
 * 			command must be set to describe the command of the app (e.g. "mmc").
 * 			fnc_exec is the function callback that is called when a line with the command is received. The parameter is the complete line.
 * 			explanation is the explanation for the command that is shown if the help command is entered.
 *
 * @param cmd_obj			Pointer to the command structure for the application command.
 */
void console_add_command(console_command_t* cmd_obj);
/**
 * @brief	Remove an application command from the console.
 *
 * @param cmd_obj			Pointer to the command structure for the application command.
 */
void console_remove_command(console_command_t* cmd_obj);
/**
 * @brief	Sets or clears an exclusive byte callback for the consoles uart.
 * 			When a callback function is set, the console just uses the callback for the received bytes and does not analyze the bytes itself.
 * 			The timeout is used to specify when to automatically clear the callback when no data is received.
 * 			The escape character is also used to automatically clear the callback.
 *
 * @param data				Pointer to the console data.
 * @param f					Callback function pointer
 * @param timeout_ms		Timeout for automatical clearing in milliseconds
 * @param escape_character	Escape character that automatically clears the callback.
 */
void console_set_byte_callback(console_data_t* data, console_cb_byte f, uint32_t timeout_ms, uint8_t escape_character);/**
 * @brief	Sets or clears an exclusive byte callback for the consoles uart.
 * 			When a callback function is set, the console just uses the callback for the received bytes and does not analyze the bytes itself.
 * 			The number of bytes is used if a fix number of bytes is expected.
 *
 * @param data				Pointer to the console data.
 * @param f					Callback function pointer
 * @param num				Number of bytes to receive directly. If 0, nothing happens.
 */
void console_set_byte_num_callback(console_data_t* data, console_cb_byte f, uint32_t num);
/**
 * @brief	Sets an activity function callback. If the function pointer is NULL, the console is always active.
 * 			If the function is set and returns true, the console stops receiving data.
 * @param data				Pointer to the console data.
 * @param f					Callback function pointer
 */
void console_set_hold_callback(console_data_t* data, console_cb_hold f);
/**
 * @brief Is meant to be called during a console callback to set the message that is sent in the body of the response.
 * Use return of this function at the end of the console callback.
 * 
 * @param data				Pointer to the console data.
 * @param ret				Value that this function returns when the message can be set.
 * @param max_len 			Maximum buffer len to allocate in heap for the message. 
 * 							If set to 0, you can send a dynamically allocated buffer in format, that already is completely built. 
 * 							It will then be sent without checking the format and afterwards the buffer will be freed. 
 * 							If the buffer is a static buffer, use console_set_response_static instead!
 * @param format 			Format buffer that is used internally with string_printf. @ref comm_vprintf for the different format variables.
 * @param ... 				Variable data that can be used for the format string.
 * @return FUNCTION_RETURN 	ret when response is set or an error code if response cannot be set.
 */
FUNCTION_RETURN console_set_response_dynamic(console_data_t* data, FUNCTION_RETURN ret, uint16_t max_len, const char* format, ...);
/**
 * @brief Is meant to be called during a console callback to set the message that is sent in the body of the response.
 * Use return of this function at the end of the console callback.
 * 
 * @param data				Pointer to the console data.
 * @param message 			Pointer to the message that should be sent in the response. 
 * 							Do not let this point to a local buffer in the stack, as this won't work, since content is not copied, but pointer is used.
 * @return FUNCTION_RETURN 	ret when response is set or an error code if response cannot be set.
 */
FUNCTION_RETURN console_set_response_static(console_data_t* data, FUNCTION_RETURN ret, const char* message);
#if CONSOLE_ENABLE_CRC
/**
 * Returns the last calculated CRC value of a received line excluding its last 4 byte.
 * @param data				Pointer to the console data.
 * @return					Calculated CRC value
 */
uint16_t console_get_last_crc_value(console_data_t* data);
/**
 * Indicates whether the last calculated CRC value of a received line excluding its last 4 byte is the same as the hex crc in the last 4 byte.
 * @param data				Pointer to the console data.
 * @return					Calculated CRC value
 */
bool console_has_valid_crc(console_data_t* data);
#endif
/**
 * @brief Indicates whether the console can currently receive data or is on hold.
 * 
 * @param data				Pointer to the console data.
 * @return true 			Console can receive data.
 * @return false 			Console is not receiving data.
 */
bool console_can_receive(console_data_t* data);

#endif

#endif /* CONSOLE_H_ */
