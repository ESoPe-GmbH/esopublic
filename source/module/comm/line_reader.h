// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file line_reader.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief	Reads a line from an interface.
 *			
 *	@version	1.03 (19.01.2022)
 * 				 - Modified to be used in esopekernel
 *	@version 	1.02 (12.03.2021)
 *		- Added a mode without extra internal task
 *		- Added ignore for empty lines
 *		- Types for the interface functions created
 *  @version	1.01 (05.02.2021)
 *  	- Added user data
 *  	- Added line_reader_receive
 *  @version	1.00 (12.12.2019)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_COMM_LINE_READER_H_
#define MODULE_COMM_LINE_READER_H_

#include "module_public.h"
#if MODULE_ENABLE_COMM_LINE_READER
#include "module/enum/function_return.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the line_reader module
#define LINE_READER_STR_VERSION "1.03"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Type for the line reader interface structure.
 */
typedef struct line_reader_interface_s line_reader_interface_t;
/**
 * Type for the line reader structure.
 */
typedef struct line_reader_s line_reader_t;
/**
 * Callback function that can be used to stop receiving on the line reader.
 * @param lr				Pointer to the line reader.
 */
typedef bool (*line_reader_cb_hold)(line_reader_t* lr);
/**
 * Callback to check if a character can be read from the interface.
 *
 * @param 			Pointer to the interface that is set for the line reader interface
 * @return			0 if nothing is to read or > 0 if there are characters available to read.
 */
typedef int (*line_reader_cb_available)(void*);
/**
 * Callback to read a single line from the interface.
 *
 * @param 			Pointer to the interface that is set for the line reader interface
 * @return			Read character from the interface.
 */
typedef char (*line_reader_cb_read_char)(void*);
/**
 * Structure for line reader interface
 */
struct line_reader_interface_s
{
	/// Pointer that is used as argument in the function pointers.
	void* obj;
	/// Pointer to a function that returns a value greater 0 if getc can be called to read a byte.
	line_reader_cb_available available;
	/// Pointer to a function that returns a read byte once available returns a value greater 0.
	line_reader_cb_read_char read_char;
};
/**
 * Structure for line reader data
 */
struct line_reader_s
{
	/// Interface for reading data from.
	line_reader_interface_t* interface;
	/// Pointer to user data. Can be assigned by user freely, is not used by line reader internally.
	void* user_data;
#if LINE_READER_USE_TASK
	/// Task for reading the line
	system_task_t task;
#endif
	/// Set this to true so line_reader_ready will not return true if the received line is empty.
	bool ignore_empty_lines;
	/// Pointer to a buffer for a received line
	char* line;
	/// Maximum number of byte that fit into the line buffer
	uint16_t line_max;
	/// Number of currently read bytes from the line buffer
	uint16_t line_cnt;
	/// Is set to true once a complete line is read into line buffer.
	bool line_read;
	/// Pointer to a hold function that can stop the line reader. Is cleared in init, set it afterwards.
	line_reader_cb_hold f_hold;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * Initialize a line reader for a specific interface.
 * A received line does not contain a linefeed or carriage return.
 *
 * @param lr					Pointer to the context of a specific line reader.
 * @param interface				Pointer to the interface from which the line is read. Set the interface structure manually before calling init.
 * @param line_buffer			Pointer to the buffer where the read line will be stored.
 * @param sizeof_line_buffer	Size of the buffer where the read line will be stored.
 *
 * @return FUNCTION_RETURN_OK if line reader is initialized. FUNCTION_RETURN_PARAM_ERROR if a parameter is NULL or 0.
 **/
FUNCTION_RETURN line_reader_init(line_reader_t* lr, line_reader_interface_t* interface, char* line_buffer, uint16_t sizeof_line_buffer);

/**
 * Returns true when a complete line was read.
 *
 * @param lr					Pointer to the context of a specific line reader.
 * @return true if a line was read, false if not.
 */
bool line_reader_ready(line_reader_t* lr);

/**
 * Has to be called after @see line_reader_ready returns true to enable the reading of the next line.
 * @param lr					Pointer to the context of a specific line reader.
 */
void line_reader_clear(line_reader_t* lr);
/**
 * Protothread function to receive a single line and return.
 * Check *rsp afterwards to see if something was received or a timeout was triggered.
 * If task is cancelled or timed out *rsp is set to NULL. Otherwise it will point to the line buffer of the line reader.
 *
 * @param pt			Protothread pointer
 * @param lr			Pointer to the line reader to use. Ensure it was initialized before.
 * @param max_timeout	Optional Timeout in milliseconds for receiving. If not needed, set to 0. If timeout is triggered, *rsp will be set to NULL.
 * @param f_cancel		Optional Cancel callback function.
 * 						Points to a function that returns true when the reception shall be cancelled. The function gets called with a pointer to the line reader as the parameter.
 * 						Set to NULL if no cancel function shall be used.
 * @param rsp			Pointer to a Pointer that shall point to the response buffer. You can use char* rsp; and then use &rsp in the paramter.
 * 						After receiving check rsp == NULL to see if task was cancelled/timed out.
 * 						If rsp != NULL, rsp will point to the received line.
 * @return				Protothread return value
 */
int line_reader_receive(struct pt* pt, line_reader_t* lr, uint32_t max_timeout, bool (*f_cancel)(line_reader_t* lr), char** rsp);

#endif // MODULE_ENABLE_COMM_LINE_READER
#endif /* MODULE_COMM_LINE_READER_H_ */
