// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		comm.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		Module for handling comm infrastructure for different devices with a single module.
 *
 *  			This module contains functions for sending and reading data over different communication interfaces.
 *  			By using this module when writing a protocol it does not matter if the communication interface is a
 *  			serial interface, can, ethernet or some other interface.
 *
 *				Many functions in this module work similar to functions in stdio.h, but do not need as many RAM/ROM.
 *				Also the stdio.h functions differ when using different compiler, so this module is a solution that works
 *				with all.
 *
 *	@version	2.09 (19.01.2022)
 * 				 - Modified to be used in esopekernel
 *	@version	2.08 (07.06.2018)
 *			     - Added module.h support
 *			     - Added docu in comm.c
 *			     - Shifted static variables to stack of comm_vprintf
 *			     - Renamed comm_t to comm_t and comm_interface_t to comm_interface_t
 *			     - Removed num_str from comm_t and put it into comm.c and externally in dbg.c
 *	@version	2.07 (24.05.2018)
 *				 - Added COMM_STRING_LENGTH_EXACT
 *				 - Added %#, %$ and %.
 *				 - Added %M, %m, %a, %A, %Q, %q and %B
 *	@version	2.06 (04.01.2015)
 *				 - Removed wait_for_data
 *				 - Uses convert/string.h functions internally. All internal functions were removed in the process.
 *				 - Added COMM_MAX_FORMAT_LENGTH to determine the internal buffer length for conversion data.
 *				 - Excluded dbg_ functions into dbg.c/dbg.h
 *				 - Added internal variables to the comm_t. Is needed if a printf is called in another printf or comm function.
 *	@version 	2.05 (16.02.2013)
 *				 - Translated the description to english
 *	@version 	2.04 (17.11.2012)
 *				 - dbg_vprintf added
 *  @version	2.03 (25.09.2011)
 *				 - include of mcu.h removed and included stdint.h and stdbool.h instead.
 *				 - dbg_printf added.
 *  @version	2.02 (22.09.2011)
 *				 - flush function added.
 *				 - vxprintf added.
 *  @version	2.01 (21.09.2011)
 *  			 - comm_interface_t structure added and functions from the comm_t included in the comm_interface_t.
 *  			 - functions edited for usage of the comm_interface_t
 *  @version	2.00 (19.09.2011)
 *  			 - Removed the old comm module and written everything from scratch.
 *
 ******************************************************************************/

#ifndef COMM_INIT_HEADER_FIRST_INCLUDE_GUARD
#define COMM_INIT_HEADER_FIRST_INCLUDE_GUARD
 
#include "module_public.h"
#if MODULE_ENABLE_COMM
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#ifndef BUILD
#define _far
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the comm module
#define COMM_STR_VERSION		"2.09"

#ifndef NULL
	#define NULL 0	///< NULL is needed inside, so it must be defined if it does not exist.
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#include "comm_type.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief	Initializes a comm_interface_t structure.
 *
 *	This function should be called before functions are assigned to the comm_interface_t structure.
 *	All functions in the comm_interface_t structure will be set to NULL.
 *
 * @param h			Pointer to the comm_interface_t
 **/
void 	comm_init_interface(comm_interface_t *h);

/**
 * @brief	Initializes a comm_t structure.
 *
 *	This function should be called before assigning anything to the comm_t structure.
 *	The device_handler and the comm_interface_t of the comm_t will be set to NULL.
 *
 * @param h			Pointer to the comm_t
 **/
void 	comm_init_handler(comm_t *h);

/**
 * @brief	Sends the data to the device the comm_t is assigned to.
 *
 *	This function sends an integer to the xputc function inside the comm_interface_t of the comm_t.
 *
 * @pre		The comm_interface_t inside the comm_t needs to be initialized
 * @pre		The comm_t needs to be initialized
 *
 * @param h			Pointer to the comm_t. Does nothing if h is NULL.
 * @param letter	Value that is send via the device of the comm_t.
 **/
void 	comm_putc(comm_t *h, int letter);

/**
 * @brief	Sends the data to the device the comm_t is assigned to.
 *
 *	This function sends an integer to the xputs function inside the comm_interface_t of the comm_t.
 *
 * @pre		The comm_interface_t inside the comm_t needs to be initialized
 * @pre		The comm_t needs to be initialized
 *
 * @param h					Pointer to the comm_t. Does nothing if h is NULL.
 * @param buf				Buffer containing the data that should be sent. Function does nothing if the buf is NULL.
 * @param element_cnt		Number of bytes that should be sent.
 **/
void 	comm_put(comm_t *h, uint8_t *buf, uint16_t element_cnt);

/**
 * @brief	Sends a string to the device the comm_t is assigned to.
 *
 *	This function writes an 0-terminated ascii string to the device_handler of the comm_t using the xputs function
 *	of the comm_interface_t.
 *
 * @pre		The comm_interface_t inside the comm_t needs to be initialized
 * @pre		The comm_t needs to be initialized
 *
 * @param h			Pointer to the comm_t. Does nothing if h is NULL.
 * @param str		A 0-terminated ascii string.
 **/
void 	comm_puts(comm_t *h, const char* str);

/**
 * @brief	Sends a formatted string to the device the comm_t is assigned to.
 *
 * 	This function is used like the fprintf function in the stdio. It does not support all formattings.
 * 	See @link vxprintf vxprintf@endlink for the supported format.
 *
 * @pre		The comm_interface_t inside the comm_t needs to be initialized
 * @pre		The comm_t needs to be initialized
 *
 * @param h			Pointer to the comm_t. Does nothing if h is NULL.
 * @param str		A string with the format defined in @link vxprintf vxprintf@endlink.
 * @param ...		Different parameters according to the format of str.
 **/
void 	comm_printf(comm_t *h, const char *str, ...);

/**
 * @brief	Sends a formatted string to the device the comm_t is assigned to.
 *
 * 	This function is used like the fprintf function in the stdio. It does not support all formattings.
 *\verbatim
 *		The formatted string can have the following format: %[width]specifier
 *		width:	A length with maximum of 2 digits.
 *		specifier:
 *			h:	An unsigned long value will be printed as a hex string with lower case. The hex string will have
 *				leading zeroes for a whole byte if no width is specified.
 *				Example: 0xABC will be printed as 0ABC.
 *			x:	Same as h.
 *			X:	Same as x and h, just with upper case letters.
 *			d:	A long value will be printed as signed decimal string.
 *			u:	Same as d just unsigned.
 *			s:	A zero-terminated state will be printed.
 *			b:	Prints an unsigned long value as a binary string. It contains leading zeroes for a whole byte
 *				if no with is specified.
 *				Example: 0x0A will be printed as 00001010.
 *\endverbatim
 * @pre		The comm_interface_t inside the comm_t needs to be initialized
 * @pre		The comm_t needs to be initialized
 *
 * @param h			Pointer to the comm_t. Does nothing if h is NULL.
 * @param str		A string with the format defined above.
 * @param vl		List with variable arguments according to the format of str.
 **/
void 	comm_vprintf(comm_t *h, const char *str, va_list vl);

/**
 * Indicates whether data can be sent on an interface.
 * @param     Pointer to the device handler of the comm_t structure.
 * @return    True if transmit is ready.
 */
bool    comm_transmit_ready(comm_t* h);

/**
 * @brief	Sends data the device has not send yet.
 *
 * 	Function is needed if the device stores the data and sends it only if the send is triggered explicitly.
 * 	Should be used when protocols use xputc for each byte, to make sure everything is sent.
 *
 * @pre		The comm_interface_t inside the comm_t needs to be initialized
 * @pre		The comm_t needs to be initialized
 *
 * @param h			Pointer to the comm_t. Does nothing if h is NULL.
 **/
void	comm_flush(comm_t *h);

/**
 * @brief	Returns a received byte of the device.
 *
 * @pre		The comm_interface_t inside the comm_t needs to be initialized
 * @pre		The comm_t needs to be initialized
 * @pre		xdata_available(h) should return a value greater then 0.
 *
 * @param h			Pointer to the comm_t. Does nothing if h is NULL.
 * @return			integer received by the interface.
 **/
int		comm_getc(comm_t *h);

/**
 * @brief	Reads more than one byte from the device at once.
 *
 * 	Reads the specified number of bytes from the device and stores it in buf.
 *
 * @pre		The comm_interface_t inside the comm_t needs to be initialized
 * @pre		The comm_t needs to be initialized
 *
 * @param h				Pointer to the comm_t. Does nothing if h is NULL.
 * @param buf			Buffer where the received bytes should be stored. Does nothing if h is NULL.
 * @param element_cnt	Maximum number of bytes that can be stored in the buffer.
 * @return				Number of bytes stored in buf or 0 if there is nothing to receive.
 **/
int		comm_gets(comm_t *h, uint8_t *buf, uint16_t element_cnt);

/**
 * @brief	Returns the number of bytes that are available to read from the comm_interface_t.
 *
 * @pre		The comm_interface_t inside the comm_t needs to be initialized
 * @pre		The comm_t needs to be initialized
 *
 * @param h			Pointer to the comm_t. Does nothing if h is NULL.
 * @return			Number of bytes available to read
 **/
int		comm_data_available(comm_t *h);

#include "dbg.h"

#endif

#endif
