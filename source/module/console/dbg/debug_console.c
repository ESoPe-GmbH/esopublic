// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file debug_console.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "debug_console.h"
#include "module_public.h"
#if MODULE_ENABLE_DEBUG_CONSOLE
#include "module/console/console.h"

#define DECLARE_DEBUG_BUFFER
#include "debug_buffer.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define DEBUG_APP_COMMAND			"app"
#define DEBUG_APP_HELP				"Subcommands: version, reset\n"	\
									"\tversion: Prints the software version.\n" \
									"\treset: Resets the application."

#if MODULE_ENABLE_FLASH_INFO
#define DEBUG_FLASH_COMMAND			"flash"
#define DEBUG_FLASH_HELP			"Subcommands: hwid, mac, save\n" \
									"\thwid (set <id> | get): Sets or returns the hardware ID.\n" \
									"\tmac (set <id> | get): Sets or returns the hardware address.\n" \
									"\tsave: Saves changes to the flash. Is automatically called when hwid set or mac set is used\n"
#endif

#if DEBUG_CONSOLE_ENABLE_IO
#define DEBUG_IO_COMMAND			"io"
#define DEBUG_IO_HELP				"Subcommands: dir, set, get, toggle\n"	\
									"\tdir <pin> (in|out): Sets the direction of an I/O.\n" \
									"\tset <pin> (h|l): Sets the output value of the pin to High (h) or Low (l).\n" \
									"\tget <pin>: Returns the value of the pin High or Low.\n" \
									"\ttoggle <pin>: Toggles the output pin.\n" \
									"\t<pin> is used as a string form of the MCU_IO_PIN enumeration for the controller."
#endif

#if MODULE_ENABLE_MMC
#define DEBUG_MMC_COMMAND			"mmc"
#define DEBUG_MMC_HELP				"Subcommands: show, list, delete, write\n" \
									"\tshow <filename>: Prints the content of a file. Use the absolute filename as an argument.\n" \
									"\tlist [<path>]: Can be used to print the filenames of a directory. Use the path of the directory as an argument.\n" \
									"\tdelete <filename>: Deletes the specified file from mmc. Use the absolute filename as an argument.\n" \
									"\twrite <filename> <content>\\0: Writes the specified file on mmc. Use the absolute filename as an argument.The content is written " \
									"1:1 into the file, use a terminating zero when the content is finished. Therefore no content with a binary 0 can be written.\n"
#endif

#if MODULE_ENABLE_NETWORK
#define DEBUG_NETWORK_COMMAND		"network"
#define DEBUG_NETWORK_HELP			"Subcommands: all, mac, link, ip, sockets, socket\n"	\
									"\tall: Shows all registered network interfaces and their states.\n"	\
									"\tmac <name>: Shows the hardware address of the network interface.\n"	\
									"\tip <name>: Shows the IP address of the network interface.\n"	\
									"\tlink <name>: Shows the link state of the network interface.\n"\
									"\tsockets: Shows all open sockets and their configuration, state and port.\n"	\
									"\tsocket <number>: Shows the socket's current configuration, state and port."
#endif

#if MODULE_ENABLE_RTC
#define DEBUG_TIME_COMMAND			"time"
#define DEBUG_TIME_HELP				"Subcommands: get, set\n" \
									"\tget: Prints the current date and time.\n" \
									"\tset <datetime>: Sets the current date and time in format DD.MM.YYYY hh:mm:ss."
#endif
#if DEBUG_CONSOLE_ENABLE_UART
#define DEBUG_UART_COMMAND			"uart"
#define DEBUG_UART_HELP				"Subcommands: init, put, get, clear, list\n" \
									"\tinit <num/name> <baudrate>: Initializes the UART by setting the baudrate and parameters as well as a receive buffer. Responds with +uartrsp init <baudrate>. Always uses 8N1.\n" \
									"\tput <num/name> \"<base64>\": Writes the data from the decoded base64 string to the UART. Responds with +uartrsp put <num>, where <num> is the number of sent bytes.\n" \
									"\tget <num/name> <num>: Returns the read bytes with a maximum of <num> from the uart in a +uartrsp get \"<base64>\" response, where the string in the quotes might be empty.\n" \
									"\tclear <num/name> rx/tx: Clears the rx or tx buffer. At the moment only rx is supported. Response is +uartrsp clear rx/tx.\n" \
									"\tlist: Print a list of all UARTs."
#endif

#if DEBUG_CONSOLE_ENABLE_TEST
#define DEBUG_TEST_COMMAND			"test"
#define DEBUG_TEST_HELP				"Subcommands: start\n" \
									"\tstart <password>: Enters the pcb testing mode, further arguments depend on application. Responds with +testrsp start when authorization is OK."
#endif
#if MODULE_ENABLE_NETWORK && DEBUG_CONSOLE_ENABLE_SOCKET
#define DEBUG_SOCKET_COMMAND		"socket"
#define DEBUG_SOCKET_HELP			"Subcommands: openserver, state, send, receive, close\n" \
									"\topenserver <port>: Starts a task for a server socket that should wait for an incoming connection.\n" \
									"\tsend \"<base64>\": Writes the data from the decoded base64 string to the socket. Responds with +socketrsp put <num>, where <num> is the number of sent bytes.\n" \
									"\treceive <num>: Returns the read bytes with a maximum of <num> from the socket in a +socketrsp get \"<base64>\" response, where the string in the quotes might be empty.\n" \
									"\tclose: Closes the socket, stops the socket tasks and clears all buffers with pending data."
#endif

#if DEBUG_CONSOLE_ENABLE_FLAG
#define DEBUG_FLAG_COMMAND			"flag"
#define DEBUG_FLAG_HELP				"Parameter is the name of a registered flag, return value is the return value of the registered flag function."
#endif

#if MODULE_ENABLE_NETWORK && DEBUG_CONSOLE_ENABLE_WIFI
#define DEBUG_WIFI_COMMAND			"wifi"
#define DEBUG_WIFI_HELP				"Subcommands: connect, disconnect, scan\n" \
									"\tconnect <ssid> <password>: Connects the ssid with the given password.\n" \
									"\tdisconnect: Disconnect the wifi.\n" \
									"\tscan: Scans for wifi networks and prints them.\n" \

#endif

#if DEBUG_CONSOLE_ENABLE_IDENTIFICATION
#define DEBUG_IDENTIFICATION_COMMAND			"id"
#define DEBUG_IDENTIFICATION_HELP				"Can be used to check if the firmware is running on the correct hardware"
#endif

#if DEBUG_CONSOLE_ENABLE_ESP
#define DEBUG_ESP_COMMAND			"esp"
#define DEBUG_ESP_HELP				"Subcommands: hash\n" \
									"\thash: Get the SHA256 hash of the data partition\n"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief	Handles the command line for the APP command.
 *
 * @param data		Pointer to the console that received the command line.
 * @param args		Pointer array for each part of the command. The whitespaces were used as dividers for each part.
 * @param args_len	Number of parts used in args.
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 **/
extern FUNCTION_RETURN debug_app_execute(console_data_t* data, char** args, uint8_t args_len);
#if MODULE_ENABLE_FLASH_INFO
/**
 * @brief	Handles the command line for the flash command.
 *
 * @param data		Pointer to the console that received the command line.
 * @param line		Pointer to the line string without the "flash" command
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 **/
extern FUNCTION_RETURN debug_flash_execute(console_data_t* data, char* line);
#endif
#if DEBUG_CONSOLE_ENABLE_IO
/**
 * @brief	Handles the command line for the I/O command.
 *
 * @param data		Pointer to the console that received the command line.
 * @param args		Pointer array for each part of the command. The whitespaces were used as dividers for each part.
 * @param args_len	Number of parts used in args.
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 **/
extern FUNCTION_RETURN debug_io_execute(console_data_t* data, char** args, uint8_t args_len);
#endif
#if MODULE_ENABLE_MMC
/**
 * @brief	Handles the command line for the mmc command.
 *
 * @param data		Pointer to the console that received the command line.
 * @param args		Pointer array for each part of the command. The whitespaces were used as dividers for each part.
 * @param args_len	Number of parts used in args.
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 **/
extern FUNCTION_RETURN debug_mmc_execute(console_data_t* data, char** args, uint8_t args_len);
#endif
#if MODULE_ENABLE_NETWORK
/**
 * @brief	Handles the command line for the network command.
 * @param data		Pointer to the console that received the command line.
 * @param args		Pointer array for each part of the command. The whitespaces were used as dividers for each part.
 * @param args_len	Number of parts used in args.
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 */
extern FUNCTION_RETURN debug_network_execute(console_data_t* data, char** args, uint8_t args_len);
#endif
#if MODULE_ENABLE_RTC
/**
 * @brief	Handles the command line for the time command.
 *
 * @param data		Pointer to the console that received the command line.
 * @param line		Pointer to the line string without the "time" command
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 **/
extern FUNCTION_RETURN debug_time_execute(console_data_t* data, char* line);
#endif
#if DEBUG_CONSOLE_ENABLE_UART
/**
 * Initialize the variables of uart console.
 */
extern void debug_uart_init(void);
/**
 * @brief Handles commands for UART testing.
 * @param data		Pointer to the console that received the command line.
 * @param args		Pointer array for each argument of the command. The whitespaces were used as dividers for each part.
 * @param args_len	Number of arguments used in args.
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 */
extern FUNCTION_RETURN debug_uart_execute(console_data_t* data, char** args, uint8_t args_len);
#endif
#if DEBUG_CONSOLE_ENABLE_TEST
/**
 * Initialize the variables of test console.
 */
extern void debug_test_init(void);
/**
 * @brief Handles commands for enabling PCB testing.
 * @param data		Pointer to the console that received the command line.
 * @param args		Pointer array for each argument of the command. The whitespaces were used as dividers for each part.
 * @param args_len	Number of arguments used in args.
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 */
extern FUNCTION_RETURN debug_test_execute(console_data_t* data, char** args, uint8_t args_len);
#endif
#if MODULE_ENABLE_DEBUG_CONSOLE && MODULE_ENABLE_MMC
/**
 * Initialize mmc variables. 
 */
extern void debug_mmc_init(void);
#endif
#if MODULE_ENABLE_NETWORK && DEBUG_CONSOLE_ENABLE_SOCKET
/**
 * Initialize the socket and add a task to test start.
 */
extern void debug_socket_init(void);
/**
 * @brief Handles commands for enabling socket for testing.
 * @param data		Pointer to the console that received the command line.
 * @param args		Pointer array for each argument of the command. The whitespaces were used as dividers for each part.
 * @param args_len	Number of arguments used in args.
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 */
extern FUNCTION_RETURN debug_socket_execute(console_data_t* data, char** args, uint8_t args_len);
#endif
#if DEBUG_CONSOLE_ENABLE_FLAG
/**
 * Initialize the variables of flag console.
 */
extern void debug_flag_init(void);
/**
 * @brief Handles commands for registered flags
 * @param data		Pointer to the console that received the command line.
 * @param args		Pointer array for each argument of the command. The whitespaces were used as dividers for each part.
 * @param args_len	Number of arguments used in args.
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 */
extern FUNCTION_RETURN debug_flag_execute(console_data_t* data, char** args, uint8_t args_len);
#endif
#if MODULE_ENABLE_NETWORK && DEBUG_CONSOLE_ENABLE_WIFI
/**
 * @brief Initializes the task for wifi scans. 
 */
extern void debug_wifi_init(void);
/**
 * @brief Handles commands for wifi operations. For connecting, the network api is used.
 * @param data		Pointer to the console that received the command line.
 * @param args		Pointer array for each argument of the command. The whitespaces were used as dividers for each part.
 * @param args_len	Number of arguments used in args.
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 */
extern FUNCTION_RETURN debug_wifi_execute(console_data_t* data, char** args, uint8_t args_len);
#endif
#if DEBUG_CONSOLE_ENABLE_IDENTIFICATION
/**
 * @brief Handles the identification command
 * @param data		Pointer to the console that received the command line.
 * @param args		Pointer array for each argument of the command. The whitespaces were used as dividers for each part.
 * @param args_len	Number of arguments used in args.
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 */
extern FUNCTION_RETURN debug_identification_execute(console_data_t* data, char** args, uint8_t args_len);
#endif
#if DEBUG_CONSOLE_ENABLE_ESP
/**
 * @brief Handles esp specific commands
 * @param data		Pointer to the console that received the command line.
 * @param args		Pointer array for each argument of the command. The whitespaces were used as dividers for each part.
 * @param args_len	Number of arguments used in args.
 * @return			FUNCTION_RETURN value to show if execution succeeded.
 */
extern FUNCTION_RETURN debug_esp_execute(console_data_t* data, char** args, uint8_t args_len);
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

console_command_t debug_console_commands[] =
{
		{DEBUG_APP_COMMAND,			 		debug_app_execute, 			true, 	DEBUG_APP_HELP, 			NULL},
#if MODULE_ENABLE_MMC
		{DEBUG_MMC_COMMAND,			 		debug_mmc_execute, 			true, 	DEBUG_MMC_HELP, 			NULL},
#endif
#if MODULE_ENABLE_FLASH_INFO
		{DEBUG_FLASH_COMMAND,				debug_flash_execute, 		false, 	DEBUG_FLASH_HELP, 			NULL},
#endif
#if DEBUG_CONSOLE_ENABLE_IO
		{DEBUG_IO_COMMAND,					debug_io_execute, 			true, 	DEBUG_IO_HELP, 				NULL},
#endif
#if MODULE_ENABLE_NETWORK
		{DEBUG_NETWORK_COMMAND,				debug_network_execute, 		true, 	DEBUG_NETWORK_HELP, 		NULL},
#endif
#if MODULE_ENABLE_RTC
		{DEBUG_TIME_COMMAND,				debug_time_execute, 		false, 	DEBUG_TIME_HELP, 			NULL},
#endif
#if DEBUG_CONSOLE_ENABLE_UART
		{DEBUG_UART_COMMAND,				debug_uart_execute, 		true, 	DEBUG_UART_HELP, 			NULL},
#endif
#if DEBUG_CONSOLE_ENABLE_TEST
		{DEBUG_TEST_COMMAND,				debug_test_execute, 		true, 	DEBUG_TEST_HELP, 			NULL},
#endif
#if DEBUG_CONSOLE_ENABLE_FLAG
		{DEBUG_FLAG_COMMAND,				debug_flag_execute, 		true, 	DEBUG_FLAG_HELP, 			NULL},
#endif
#if MODULE_ENABLE_NETWORK && DEBUG_CONSOLE_ENABLE_SOCKET
		{DEBUG_SOCKET_COMMAND,				debug_socket_execute, 		true, 	DEBUG_SOCKET_HELP, 			NULL},
#endif
#if MODULE_ENABLE_DEBUG_CONSOLE && MODULE_ENABLE_NETWORK && DEBUG_CONSOLE_ENABLE_WIFI && MCU_TYPE == MCU_ESP32 && MCU_PERIPHERY_ENABLE_WIFI
		{DEBUG_WIFI_COMMAND,				debug_wifi_execute, 		true, 	DEBUG_WIFI_HELP, 			NULL},
#endif
#if DEBUG_CONSOLE_ENABLE_IDENTIFICATION
		{DEBUG_IDENTIFICATION_COMMAND, 		debug_identification_execute, true, DEBUG_IDENTIFICATION_HELP, NULL},
#endif
#if DEBUG_CONSOLE_ENABLE_ESP
		{DEBUG_ESP_COMMAND, 				debug_esp_execute, 			true, 	DEBUG_ESP_HELP, 			NULL},
#endif
};

const uint16_t debug_console_command_cnt = sizeof(debug_console_commands) / sizeof(console_command_t);
#if MCU_TYPE != MCU_ESP32
static uint8_t _uart_rx[CONSOLE_UART_RECEIVE_BUFFER_SIZE];
#endif
static console_data_t _console;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void debug_console_init(comm_t* comm)
{
	uint16_t i;
#if MCU_TYPE != MCU_ESP32
	if(comm->interface->xputc == mcu_uart_putc)
	{
		while(!mcu_uart_transmit_ready(comm->device_handler));
		mcu_uart_set_buffer(comm->device_handler, MCU_INT_LVL_MED, _uart_rx, sizeof(_uart_rx));
	}
#endif

	_console.task.name = "Debug";
	console_init(&_console, comm);

	// Add all console commands
	for(i = 0; i < debug_console_command_cnt; i++)
		console_add_command(&debug_console_commands[i]);
		
#if MODULE_ENABLE_DEBUG_CONSOLE && MODULE_ENABLE_MMC
	debug_mmc_init();
#endif
#if DEBUG_CONSOLE_ENABLE_TEST
	debug_test_init();
#endif
#if DEBUG_CONSOLE_ENABLE_UART
	debug_uart_init();
#endif
#if MODULE_ENABLE_NETWORK && DEBUG_CONSOLE_ENABLE_SOCKET
	debug_socket_init();
#endif
#if DEBUG_CONSOLE_ENABLE_FLAG
	debug_flag_init();
#endif
#if MODULE_ENABLE_DEBUG_CONSOLE && MODULE_ENABLE_NETWORK && DEBUG_CONSOLE_ENABLE_WIFI && MCU_TYPE == MCU_ESP32 && MCU_PERIPHERY_ENABLE_WIFI
	debug_wifi_init();
#endif
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif
