/**
 * 	@file 	mcu_uart.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_uart functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *	@subsection	mcu_htu_uart_init UART
 *
 *		Like the IO Interrupt the first step using the UART is creating the handler. For this you should use the
 *		@link mcu_uart_init mcu_uart_init@endlink function in the board initialization. The parameter for the function
 *		are the uart numner, TX and the RX pin. Some mcu_controller.h files have defines for their UARTs, so that you must not
 *		check the pin directly. You can see an example for both versions below. Both function calls are for the UART1
 *		of the RX621/RX62N controller of Renesas.
@code
	mcu_uart_t my_uart = mcu_uart_init(1, P2_6, P3_0);
	mcu_uart_t my_uart = mcu_uart_init(MCU_UART1_INIT_PARAM);
@endcode
 *	
 *		After creation of the handler the module can set the parameters for the UART. These parameters are baudrate,
 *		databits, stopbits and parity. If the function returns MCU_OK the UART works with the parameters. You need to
 *		look at this, because not all controller support for example 9 databits.
@code
	if(MCU_OK == mcu_uart_set_param(my_uart, 9600, 8, 'N', 1))
		... // Parameter were set successfull
@endcode
 *
 *		The uart can now be used to send some data. For a debug interface you do not need a receive buffer, but if you need to receive data
 *		you need to tell the uart how you want the data. There are 2 ways for the uart to receive data:
 *		 - Declaring a receive buffer.
 *		 - Declaring an own receive interrupt function (Alternate receive).
 *
 *		The example below shows how you can add a receive buffer for an uart with maximum of 8 databits.
@code
	uint8_t my_uart_buffer[800];

	mcu_uart_set_buffer(my_uart, MCU_INT_LVL_HI, my_uart_buffer, sizeof(my_uart_buffer));	
	...
	if(mcu_uart_available(my_uart) > 0)	// If data were received
	{
		uint8_t b = mcu_uart_getc(my_uart);	// read 1 byte
		// ...
	}
@endcode		
 *	@attention	If you use your uart with 9 databits you need a 16-Bit buffer! The example below shows how you add the receive buffer to the
 *				uart. Keep in mind that you cannot use sizeof() for the buffer size because you need to tell the number of elements to the
 *				uart!
@code
	uint16_t my_uart_buffer[800];	// 16-bit Buffer!
	
	mcu_uart_set_param(my_uart, 9600, 9, 'N', 1);	// A 9-Bit UART!
	mcu_uart_set_buffer(my_uart, MCU_INT_LVL_HI, my_uart_buffer, 800);	// Important: Last parameter is number of elements.
	...
	if(mcu_uart_available(my_uart))	// If data were received
	{
		uint16_t b = mcu_uart_getc(my_uart);	// ... read 1 word
		// ...
	}
@endcode
 *
 *	If you have an own receive function you can just add it to the uart. If you use an own function the uart does not use its buffer for
 *	the receive. So functions like mcu_uart_available cannot be used.
@code
	void my_uart_receive(int b);	// Receive function with int as parameter (Because of the 9-Bit possibility)
	...
	mcu_uart_set_alternate_receive(my_uart, MCU_INT_LVL_HI, my_uart_receive);	
@endcode
 *
 *	@version	2.01 (04.09.2022)
 * 				 - Added `mcu_uart_create`, `mcu_uart_free`, `mcu_uart_set_config` and `mcu_uart_get_config`. 
 *  @version    2.00 (20.08.2022)
 *               - Extracted mcu_uart functions in this file. History from now on will only contain infos about mcu_uart
 *	@version	1.12 (10.10.2016)
 *				 - Extracted types into new mcu_types.h
 *				 - Added I2C
 *	@version	1.11 (11.01.2015)
 *				 - Added mcu_flash_read -> Is needed for RL78
 *	@version	1.10 (13.11.2014)
 *				 - Changes mcu_io_interrupt_set_param -> There is now an object pointer as a parameter
 *				   inside the function and as a pointer parameter inside the callback function.
 *	@version	1.09 (25.05.2014)
 *				 - Added mcu_uart_set_direct_transmit_interrupt
 *				 - Added mcu_uart_transmit_ready
 *				 - Added mcu_io_interrupt_get_pin function
 *				 - Added rtc functions (init, get and set)
 *	@version	1.08 (28.02.2013)
 *				 - Translated the comments in english
 *				 - Added DA converter and flash to the How-to-use
 *				 - Added parameter to mcu_uart_init, mcu_spi_init and mcu_io_interrupt_init
 *	@version	1.07 (04.06.2012)
 *				 - Added DA converter
 *	@version	1.06 (02.01.2012)
 *				 - Added Flash Init function
 *				 - Changed defines for mcu_config.h 1.04
 *	@version	1.05 (24.09.2011)
 *				 - Added CAN
 *				 - Added Flash
 *	@version	1.04 (22.09.2011)
 *				 - MCU_PERIPHERY_ENABLE_ check removed for functions using MCU_PERIPHERY_DEVICE_COUNT_
 *				 - How-to-use rewritten for handler objects
 *	@version	1.03 (21.09.2011)
 *				 - Added mcu_io_handler_t with the macros MCU_IO_HANDLER_INIT, MCU_IO_SET_HANDLE, MCU_IO_SET_HANDLE_PTR,
 *				 	MCU_IO_GET_HANDLER, MCU_IO_GET_HANDLE_PTR, MCU_IO_TOGGLE_HANDLE and MCU_IO_TOGGLE_HANDLE_PTR
 *	@version	1.02
 *				 - Added debug functions, which needs to be implemented for each controller in the mcu.c
 *	@version	1.01
 *				 - Global Interrupt Levels for all MCU
 *  @version	1.00
 *  			 - Initial Release
 *
 *  @see	[1] http://de.wikipedia.org/wiki/Serial_Peripheral_Interface am 06.08.2011
 *
 ******************************************************************************/

#ifndef __MCU_UART_HEADER_FIRST_INCLUDE__
#define __MCU_UART_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

#if MCU_PERIPHERY_ENABLE_COMM_MODE_UART
	#include "module/comm/comm_type.h"
#endif

//------------------------------------------------------------------------------------------------------------
// Enumeration
//------------------------------------------------------------------------------------------------------------

/**
 * @brief Operation modes of the UART. 
 */
typedef enum mcu_uart_mode_e
{
	/// Normal UART mode using TX for sending and RX for receiving.
	/// Default operation mode
	MCU_UART_MODE_UART_NO_FLOW_CONTROL = 0,
	/// Normal UART mode with the addition of RTS/CTS Flow control.
	MCU_UART_MODE_UART_RTS_CTS_FLOW_CONTROL,
	/// RS485 mode uses a DE line to assert to high when data is sent and cleared to 0, when sending of last byte is finished.
	/// Uses RTS pin for DE and /RE.
	MCU_UART_MODE_UART_485_HALF_DUPLEX,
	/// RS485 mode uses a DE line to assert to high when data is sent and cleared to 0, when sending of last byte is finished.
	/// Uses RTS pin for DE.
	MCU_UART_MODE_UART_485_FULL_DUPLEX
}MCU_UART_MODE_T;

//------------------------------------------------------------------------------------------------------------
// Structures
//------------------------------------------------------------------------------------------------------------

/**
 * @brief Hardware configuration for the UART that should be created in board init.
 */
typedef struct mcu_uart_hw_config_s
{
	/// Index of the UART in case the UARTs have a specific unit number.
	int unit;
	/// Output pin for the TX signal used to sent data.
	/// If unused, set to PIN_NONE.
	MCU_IO_PIN io_tx;
	/// Input pin for the RX signal used to receive data.
	/// If unused, set to PIN_NONE.
	MCU_IO_PIN io_rx;
	/// Input pin for the CTS signal used to check whether data can be sent. Only relevant in mode MCU_UART_MODE_UART_RTS_CTS_FLOW_CONTROL.
	/// If unused, set to PIN_NONE.
	MCU_IO_PIN io_cts;
	/// Output pin for the RTS signal in MCU_UART_MODE_UART_RTS_CTS_FLOW_CONTROL. This will be cleared to low if receive buffer is full to prevent the sender from sending data.
	/// In mode MCU_UART_MODE_UART_485 this is the output pin connected to DE of 485. For half duplex mode, this can also be tied to /RE pin. If not connected to /RE, echoes of sent data will be received.
	/// If unused, set to PIN_NONE.
	MCU_IO_PIN io_rts;
	/// Size of the transmit buffer to allocate internally. Is not necessary to send data.
	/// If 0, sent data will block until everything is sent.
	/// If 0 and direct transmit interrupt is activated, you can check mcu_uart_transmit_ready to check if data can be sent and then use mcu_uart_puts to send your buffer. This routine uses the interrupt or dma and might be even better than setting this to > 0.
	size_t transmit_buffer_size;
	/// Only relevant when transmit_buffer_size is > 0 or use_direct_transmit_interrupt is set to true. Declares the interrupt level used by the UART transmit.
	MCU_INT_LVL transmit_interrupt_level;
	/// Size of the receive buffer to allocate internally. Is necessary to receive data.
	size_t receive_buffer_size;
	/// Only relevant when receive_buffer_size > 0. Declares the interrupt level used by the UART receive.
	MCU_INT_LVL receive_interrupt_level;
#if MCU_TYPE == PC_EMU
	/// Optional config parameter on pc only that can be set to use an esoprog.
	/// Note: At the moment, the name must be set to the IP that can be seen in the Launcher for the esoprog, since we have no zeroconf implementation in c on pc.
	const char* name;
#endif
}mcu_uart_hw_config_t;

/**
 * @brief UART configuration variables
 */
typedef struct mcu_uart_config_s
{
	/// If set while transmit_buffer_size is 0, the mcu_uart_puts function will place a buffer to be send with interrupt/dma and return immediately. You need to check mcu_uart_transmit_ready to check when your data was completely sent.
	bool use_direct_transmit_interrupt;
	/// Baudrate in bit/s
	/// If 0, 500000 will be set.
	uint32_t baudrate;
	/// Parity of the the UART. Valid values are 'N'=None, 'E'=Even and 'O'=Odd.
	/// If 0, 'N' will be set.
	char parity;
	/// Number of databits for the UART.
	/// If 0, is set to 8.
	uint8_t databits;
	/// Number of stopbits. Valid values are 1 and 2.
	/// If 0, is set to 1.
	uint8_t stopbits;
	/// UART mode to use.
	/// Default is MCU_UART_MODE_UART_NO_FLOW_CONTROL.
	MCU_UART_MODE_T mode;
	/// Percentage of the buffer fullness to set rts.
	/// Only relevant for MCU_UART_MODE_UART_RTS_CTS_FLOW_CONTROL.
	/// Default is 90.
	uint8_t percent_rts_set; 
	/// Percentage of the buffer fullness to clear rts.
	/// Only relevant for MCU_UART_MODE_UART_RTS_CTS_FLOW_CONTROL.
	/// Default is 10.
	uint8_t percent_rts_clear;
}mcu_uart_config_t;

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_DEVICE_COUNT_UART > 0

#ifdef _RENESAS_SYNERGY_
/**
 * Initializes a uart. The UART must be created inside configuration.xml.
 * @param h     The pointer to the UART as created with configuration.xml.
 */
MCU_RESULT mcu_uart_init(mcu_uart_t h);
#else
/**
 * @brief 	Initializes a uart with the given pins. If the function returns NULL, the uart could not be intialized. Check mcu_get_last_error
 * 			to see why it could not be initialized.
 * Can be used as an alternative to `mcu_uart_create`.
 * 
 *			The following errors might occur:
 * 			 - MCU_ERROR_UART_NOT_AVAILABLE: 	All uarts are in use. Adjust MCU_PERIPHERY_DEVICE_COUNT_UART in mcu_config.h
 * 			 - MCU_ERROR_UART_INVALID: 		The i/o pins or the number are wrong.
 *
 * @pre		Check MCU_PERIPHERY_DEVICE_COUNT_UART in the mcu_config.h before. in the mcu_config.h before.
 *
 * @param num      			Number of the uart.
 * @param tx      			I/o of the tx pin.
 * @param rx				I/o of the rx pin.
 * @return					NULL: The uart could not be initialized.
 *							Other then NULL: The uart handler.
 */
mcu_uart_t mcu_uart_init(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx);
#endif
/**
 * @brief Creates a UART by applying the hardware and UART configuration at once.
 * Can be used as an alternative to `mcu_uart_init`.
 * 
 * @param hw 				Hardware configuration for the UART to declare pins, etc.
 * @param config 			Configuration for the UART.
 * @return					NULL: The uart could not be initialized.
 *							Other then NULL: The uart handler.
 */
mcu_uart_t mcu_uart_create(mcu_uart_hw_config_t* hw, mcu_uart_config_t* config);
/**
 * @brief Frees a previously created UART and deactivates it functionality.
 * Keep in mind to set your handler to NULL after calling this, since calling other functions with the old freed handler might lead to unexpected behavior.
 * 
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 */
void mcu_uart_free(mcu_uart_t h);
/**
 * @brief Retrieves the configuration of the UART. Do not modify the data from the pointer. Use `mcu_uart_set_config` to set a new configuration.
 * 
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @return 					Configuration for the UART.
 */
const mcu_uart_config_t* mcu_uart_get_config(mcu_uart_t h);
/**
 * @brief 
 * 
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param config 			Configuration for the UART.
 * @return					MCU_OK: No error occured.\n
 * 							MCU_ERROR_UART_INVALID : Uart handler is NULL.\n
 * 							MCU_ERROR_UART_BAUDRATE_INVALID : Baudrate is too high or too low and cannot be set.\n
 * 							MCU_ERROR_UART_DATABITS_INVALID : Number of databits is invalid.\n
 * 							MCU_ERROR_UART_PARITY_INVALID : Parity is invalid.\n
 * 							MCU_ERROR_UART_STOPBITS_INVALID : Number of Stopbits is invalid.\n
 */
MCU_RESULT mcu_uart_set_config(mcu_uart_t h, mcu_uart_config_t* config);
/**
 * @brief	Enables CTS functionality
 * 
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param io_cts			CTS Pin
 * @return					MCU_OK: No error occured.\n
 * 							MCU_ERROR_UART_INVALID : Uart handler is NULL.\n
 */
MCU_RESULT mcu_uart_enable_cts(mcu_uart_t h, MCU_IO_PIN io_cts);

/**
 * @brief	Enables RTS functionality
 * 
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param io_rts			RTS Pin
 * @return					MCU_OK: No error occured.\n
 * 							MCU_ERROR_UART_INVALID : Uart handler is NULL.\n
 */
MCU_RESULT mcu_uart_enable_rts(mcu_uart_t h, MCU_IO_PIN io_rts);
/**
 * @brief   Manually sets RTS if it is needed from an upper level.
 *          RTS is cleared in handle if buffer has enough free space!
 *          RTS pin is set directly.
 * 
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param set               true to set rts, false to clear it.
 * @return                  MCU_OK: No error occured.\n
 *                          MCU_ERROR_UART_INVALID : Uart handler is NULL.\n
 */
MCU_RESULT mcu_uart_set_rts(mcu_uart_t h, bool set);
/**
 * Sets the percentage values that are used to control RTS internally. By default RTS is set when 90% of the buffer
 * is full and cleared if only 10% of the buffer is full.
 * 
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param percent_rts_set   Percentage of the buffer fullness to set rts (default: 90).
 * @param percent_rts_clear Percentage of the buffer fullness to clear rts (default: 10).
 * @return                  MCU_OK: No error occured.\n
 *                          MCU_ERROR_UART_INVALID : Uart handler is NULL.\n
 */
MCU_RESULT mcu_uart_set_rts_param(mcu_uart_t h, uint8_t percent_rts_set, uint8_t percent_rts_clear);
/**
 * @brief 	Sets baudrate, number of databits, parity and the number of stopbits for the uart.
 * 			Because of the clocks and dividers the baudrate cannot be set precisely. To see the baudrate that was set,
 * 			see mcu_uart_get_baudrate.
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param baud				Baudrate in bit/s
 * @param databits			Number of databits (5 - 9). Not all mcus support all this databits.
 * @param parity			Parity ('N' = None, 'O' = Odd, 'E' = Even)
 * @param stopbits			Number of Stopbits (1, 2)
 * @return					MCU_OK: No error occured.\n
 * 							MCU_ERROR_UART_INVALID : Uart handler is NULL.\n
 * 							MCU_ERROR_UART_BAUDRATE_INVALID : Baudrate is too high or too low and cannot be set.\n
 * 							MCU_ERROR_UART_DATABITS_INVALID : Number of databits is invalid.\n
 * 							MCU_ERROR_UART_PARITY_INVALID : Parity is invalid.\n
 * 							MCU_ERROR_UART_STOPBITS_INVALID : Number of Stopbits is invalid.\n
 */
MCU_RESULT mcu_uart_set_param(mcu_uart_t h, uint32_t baud, uint8_t databits, uint8_t parity, uint8_t stopbits);

/**
 * @brief 	Sets a buffer to the uart for storing received bytes / words. Since this function activates the receive interrupt an interrupt
 * 			level is set in this function.
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 * @pre		The baudrate, databits, etc. should be set with mcu_uart_set_param at least once.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param lvl				Level of the uart receive interrupt.
 * @param data				Pointer to the buffer that can be used for uart receive.
 * @param len				Number of elements that can be stored inside the buffer.
 * @return					MCU_OK: No error occured.\n
 * 							MCU_ERROR_UART_INVALID : Uart handler is NULL.\n
 * 							MCU_ERROR_UART_RECEIVE_INVALID : The buffer length is 0 or the pointer to the buffer is NULL.
 */
MCU_RESULT mcu_uart_set_buffer(mcu_uart_t h, MCU_INT_LVL lvl, uint8_t *data, uint16_t len);

/**
 * @brief 	Sets an alternate receive function to use instead of an uart internal receive buffer.
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 * @pre		The baudrate, databits, etc. should be set with mcu_uart_set_param at least once.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param lvl				Level of the uart receive interrupt.
 * @param f					Callback function for the receive function with two parameters.\n
 * 								- Parameter 1: Pointer that is the same as the obj parameter of this function.
 * 								- Parameter 2: Received byte
 * @param obj				Object pointer that is used as first parameter in the callback function.
 * @return					MCU_OK: No error occured.\n
 * 							MCU_ERROR_UART_INVALID : Uart handler is NULL.\n
 * 							MCU_ERROR_UART_RECEIVE_INVALID : The callback function is NULL.
 */
MCU_RESULT mcu_uart_set_alternate_receive(mcu_uart_t h, MCU_INT_LVL lvl, void (*f)(void*, int), void* obj);

/**
 * @brief 	Sets a receive event function that will be triggered when a data was received.
 * This function does not activate the receive interrupt, it only sets the function pointer to be triggered on receive interrupt.
 * 
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param f 				Callback function for the receive function with \c obj as parameter.
 * @param obj 				Pointer that will be used inside the callback.
 * @return					MCU_OK: No error occured.\n
 * 							MCU_ERROR_UART_INVALID : Uart handler is NULL.\n
 * 							MCU_ERROR_UART_RECEIVE_INVALID : The callback function is NULL.
 */
MCU_RESULT mcu_uart_set_receive_event(mcu_uart_t h, void (*f)(void*), void* obj);

/**
 * @brief	Enables the use of a direct transmit interrupt. Which means, that in mcu_uart_puts or xput with a uart in
 * 			the comm handler, the buffer in the parameter is set for sending data inside the interrupt.
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @return					MCU_OK: No error occured.\n
 * 							MCU_ERROR_UART_INVALID : Uart handler is NULL.
 */
MCU_RESULT mcu_uart_set_direct_transmit_interrupt(mcu_uart_t h);

/**
 * @brief	Indicates if the uart is ready to send a byte. This can be used for asynchronous sending when using a slow
 * 			baudrate. If mcu_uart_set_direct_transmit_interrupt was used and a buffer is sent with mcu_uart_puts
 * 			it returns true when all data from the transmit buffer are sent.
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @return					true: UART is free for sending data
 * 							false: UART is busy with sending data.
 */
bool mcu_uart_transmit_ready(mcu_uart_t h);

/**
 * @brief 	Sets the baudrate for the uart. Because of the clocks and dividers the baudrate cannot be set precisely. To see
 * 			the baudrate that was set, see mcu_uart_get_baudrate.
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 * @pre		The baudrate, databits, etc. should be set with mcu_uart_set_param at least once.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param baudrate			Baudrate in bit/s
 * @return					MCU_OK: No error occured.\n
 * 							MCU_ERROR_UART_INVALID : Uart handler is NULL.\n
 * 							MCU_ERROR_UART_BAUDRATE_INVALID : Baudrate is too high or too low and cannot be set.
 */
MCU_RESULT mcu_uart_set_baudrate(mcu_uart_t h, uint32_t baudrate);

/**
 * @brief	Returns the real baudrate the uart uses.
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @return					The baudrate or 0 if handler is NULL.
 */
uint32_t mcu_uart_get_baudrate(mcu_uart_t h);
/**
 * Sets the pins to gpio function or peripheral function.
 * Once the uart is initialized and running, the pins are set to peripheral function for the uart
 * to use. Sometimes it is necessary to manually take control over the pins. This function can be used
 * to set the pins into gpio mode, which effectively disables the uart function.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param b					true: Pins can be used as GPIO -> UART unusable
 * 							false: Pins are controlled by UART -> GPIO not possible (default after mcu_uart_init)
 */
void mcu_uart_set_pin_gpio(mcu_uart_t h, bool b);

/**
 * @brief 	Sends a single byte (databits <=8) / word (databits >8) over the uart.
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param data     			Byte/word to be sent or 0 if handler is NULL.
 */ 
void mcu_uart_putc(mcu_uart_t h, int data);

/**
 * @brief 	Sends a datablock over the uart. If mcu_uart_set_direct_transmit_interrupt is used the data parameter is
 * 			set as the transmit buffer from where the data is sent within the transmit interrupts. So make sure that
 * 			the transmit buffer is not changed while mcu_uart_transmit_ready returns false. If there is data still sending
 * 			this function blocks until mcu_uart_transmit_ready returns true.
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param data				Pointer to the buffer containing the data that needs to be sent.
 * @param len				Number of bytes (not elements!) that needs to be sent or 0 if handler is NULL.
 */
void mcu_uart_puts(mcu_uart_t h, uint8_t *data, uint16_t len);

/**
 * @brief 	Returns the number of elements inside the uart buffer.
 * 			Does not work when alternate receive is used!
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 * @pre		A receive buffer for the uart must have been set with mcu_uart_set_buffer.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @return 					Number of elements inside the uart buffer or 0 if handler is NULL.
 */
int mcu_uart_available(mcu_uart_t h);

/**
 * @brief 	Reads a single byte (databits <=8) / word (databits >8) from the uart buffer.
 * 			Does not work when alternate receive is used!
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 * @pre		A receive buffer for the uart must have been set with mcu_uart_set_buffer.
 * @pre		The function mcu_uart_available must return a value greater than 0.
 *
 * @param h					Uart handler
 * @return 					Read byte/word or 0 if handler is NULL.
 */
int mcu_uart_getc(mcu_uart_t h);

/**
 * @brief 	Reads a data block from the uart and stores it inside the pointed buffer.
 * 			Does not work when alternate receive is used!
 *
 * @attention	element_count needs to be the number of elements, not the size of the buffer! sizeof might cause problems with 16-bit buffers.
 *
@code
	#define STR_ELEMENTS	500
	uint16_t str_16[STR_ELEMENTS];		// Buffer for 9-bit uart
	uint8_t str_8[STR_ELEMENTS];		// Buffer for 8-bit uart
	
	mcu_uart_gets(h, (uint8_t*)str_8, sizeof(str_8));	// Right
	mcu_uart_gets(h, (uint8_t*)str_8, STR_ELEMENTS);	// Better, because it would still work if str_8 becomes 16-bit.
	
	mcu_uart_gets(h, (uint8_t*)str_16, sizeof(str_16));	// Wrong, because sizeof returns 1000, but the number of elements is 500.
	mcu_uart_gets(h, (uint8_t*)str_16, STR_ELEMENTS);	// Right -> Works the same with 8-Bit
@endcode
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 * @pre		A receive buffer for the uart must have been set with mcu_uart_set_buffer.
 * @pre		The function mcu_uart_available must return a value greater than 0.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param buf				Buffer to store the received data.
 * @param element_count		Number of elements that can be stored inside buf.
 * @return 					Number of elements that are stored inside buf or 0 if handler is NULL.
 */
int mcu_uart_gets(mcu_uart_t h, uint8_t* buf, uint16_t element_count);

/**
 * @brief 	Clears the receive buffer of the uart handler.
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 * @pre		A receive buffer for the uart must have been set with mcu_uart_set_buffer.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 */
void mcu_uart_clear_rx(mcu_uart_t h);

#if MCU_PERIPHERY_ENABLE_COMM_MODE_UART

/**
 * @brief 	Inserts the comm_interface_t of the uart and the uart handler into the comm_t.
 *
 * @pre		handle must be created using `mcu_uart_init` or `mcu_uart_create`.
 *
 * @param h                 Uart handler as created using `mcu_uart_init` or `mcu_uart_create`.
 * @param ch				Pointer to the comm_t the uart comm_interface_t should be inserted.
 */
void mcu_uart_create_comm_handler(mcu_uart_t h, comm_t *ch);

#endif // #if MCU_PERIPHERY_ENABLE_COMM_MODE_UART

#endif

#endif // __MCU_UART_HEADER_FIRST_INCLUDE__
