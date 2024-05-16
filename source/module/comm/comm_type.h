/**
 * 	@file 		comm_type.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  Defines the types used in comm.h.
 */
#ifndef MODULE_COMM_COMM_TYPE_H_
#define MODULE_COMM_COMM_TYPE_H_

#include <stdint.h>

/**
 * Pointer to the putc function of the interface.
 * First parameter is the device_handler of the comm_t structure.
 * Second value is an integer. Might be needed if 9-Bit values or more are send in one go.
 */
typedef void(*comm_putc_t)(void*, int);
/**
 * Pointer to the puts function of the interface.
 * First parameter is the device_handler of the comm_t structure.
 * Second parameter is a pointer to the data (8-Bit) that needs to be sent.
 * Third parameter is the length of the data that needs to be sent.
 */
typedef void(*comm_puts_t)(void*, uint8_t *, uint16_t);
/**
 * Pointer to the getc function of the interface.
 * Parameter is the device_handler of the comm_t structure.
 * Return value is an integer. Might be needed if 9-Bit values or more are received in one go.
 */
typedef int(*comm_getc_t)(void*);
/**
 * Pointer to the gets function of the interface.
 * First parameter is the device_handler of the comm_t structure.
 * Second parameter is a pointer to the data (8-Bit) that will store the received data.
 * Third parameter is the maximum length of the data that needs to be received.
 * Return value is the number of data written inside the data array.
 */
typedef int(*comm_gets_t)(void*, uint8_t *, uint16_t);
/**
 * Pointer to the gets function of the interface.
 * First parameter is the device_handler of the comm_t structure.
 * Return value is the number of data available for reading
 */
typedef int(*comm_available_t)(void*);
/**
 * Pointer to the transmit ready function of the interface.
 * @param     Pointer to the device handler of the comm_t structure.
 * @return    True if transmit is ready.
 */
typedef bool (*comm_transmit_ready_t)(void*);
/**
 * Pointer to the flush function of the interface, which should send stored data that is not yet sent.
 * Parameter is the device_handler of the comm_t structure.
 */
typedef void(*comm_flush_t)(void*);

/**
 *	@struct	comm_interface_t
 *		Similiar to the FILE structure of stdio. Contains put/get functions, that will get the device_handler of the
 *		comm_t as the first parameter.
 **/
typedef struct
{
	/**
	 * Pointer to the putc function of the interface.
	 * First parameter is the device_handler of the comm_t structure.
	 * Second value is an integer. Might be needed if 9-Bit values or more are send in one go.
	 */
	comm_putc_t xputc;
	/**
	 * Pointer to the getc function of the interface.
	 * Parameter is the device_handler of the comm_t structure.
	 * Return value is an integer. Might be needed if 9-Bit values or more are received in one go.
	 */
	comm_getc_t xgetc;
	/**
	 * Pointer to the puts function of the interface.
	 * First parameter is the device_handler of the comm_t structure.
	 * Second parameter is a pointer to the data (8-Bit) that needs to be sent.
	 * Third parameter is the length of the data that needs to be sent.
	 */
	comm_puts_t xputs;
	/**
	 * Pointer to the gets function of the interface.
	 * First parameter is the device_handler of the comm_t structure.
	 * Second parameter is a pointer to the data (8-Bit) that will store the received data.
	 * Third parameter is the maximum length of the data that needs to be received.
	 * Return value is the number of data written inside the data array.
	 */
	comm_gets_t xgets;
	/**
	 * Pointer to the gets function of the interface.
	 * First parameter is the device_handler of the comm_t structure.
	 * Return value is the number of data available for reading
	 */
	comm_available_t data_present;
	/**
	 * Pointer to the transmit ready function of the interface.
	 * @param     Pointer to the device handler of the comm_t structure.
	 * @return    True if transmit is ready.
	 */
	comm_transmit_ready_t transmit_ready;
	/**
	 * Pointer to the flush function of the interface, which should send stored data that is not yet sent.
	 * Parameter is the device_handler of the comm_t structure.
	 */
	comm_flush_t flush;
}comm_interface_t;

/**
 *	@struct	comm_t
 *		The comm handler contains a pointer to a device handler like mcu_uart_t for uarts or a FIL object for the
 *		mmc module and a pointer to the interface the device handler uses.
 **/
typedef struct
{
	/// Pointer to the device handler of the interface
	void *device_handler;

	/// Pointer to the interface structure the device uses.
	const comm_interface_t* interface;

	/// Used for buffering the width of a format inside a xprintf function.
	/// Stores 3 letters + 1 Byte zero-termination.
	char len_ascii_str[4];

	/// Contains the number of bytes of comm_len_ascii_str.
	uint16_t len_ascii_str_len;

	/// Converted integer value of comm_len_ascii_str.
	uint16_t format_len;
}comm_t;

#endif /* MODULE_COMM_COMM_TYPE_H_ */
