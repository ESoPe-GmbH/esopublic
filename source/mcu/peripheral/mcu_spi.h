/**
 * 	@file 	mcu_spi.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_spi functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *	@subsection	mcu_htu_spi_master SPI Master Mode
 *
 *	SPI is initialized like the uart. There are just 2 more i/o pins in the initialization (Clock and Chip Select), because spi is a
 *	4-wire bus. There are definitions inside mcu_controller.h for the tx, rx and clock parameter, but you have to add the chip select
 *	pin yourself because any i/o pin can be used for it.
 *
@code
	mcu_spi_t my_spi = mcu_spi_init(0, P6_3, P6_2, P6_1, P6_0);
	// or
	mcu_spi_t my_spi = mcu_spi_init(MCU_SPI0_INIT_PARAM, P6_0);
@endcode
 *
 *	After the initialization you need to set parameters for spi like spi mode and the speed.
 * 	The clock you set cannot always be used but the controllers tries to set the next lower value in this case. So just call it with the
 * 	maximum value you can use and it will not be faster.
 *
@code
	if(MCU_OK == mcu_spi_set_param(my_spi, MCU_SPI_MODE_0, 5000000))
		...
@endcode
 *
 *	You can change the speed with mcu_spi_set_speed after it, but you need to use mcu_spi_set_param at least once because of the spi mode.
 *
 *	To send data you have 2 possibilites:
 *	 - Sending a single byte where the spi automatically sets the chip select pin.
 *	 - Sending a data block where you need to enable chip select.
 *
 * 	The example below shows the transmission of a single byte.
@code
	mcu_spi_send_with_cs(my_spi, 0x0F);	
@endcode 
 *				
 *	The example below shows how a data block can be send. You can see that you need to set the chip select pin manually.
@code
	mcu_spi_set_chip_select(my_spi, 0);
	mcu_spi_send(my_spi, 0x0F);	
	mcu_spi_send(my_spi, 0x23);	
	mcu_spi_set_chip_select(my_spi, 1);
@endcode 
 * 
 *	Reading works the same way because it is a synchronous interface:
@code
	uint8_t data;
	mcu_spi_set_chip_select(my_spi, 0);
	mcu_spi_send(my_spi, 0x0F);	// Only transmission
	data = mcu_spi_send(my_spi, 0xFF);	// Transmission and reception
	mcu_spi_set_chip_select(my_spi, 1);
@endcode
 *
 *  @version    2.00 (20.08.2022)
 *               - Extracted mcu_spi functions in this file. History from now on will only contain infos about mcu_spi.
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
 ******************************************************************************/

#ifndef __MCU_IO_SPI_HEADER_FIRST_INCLUDE__
#define __MCU_IO_SPI_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

//------------------------------------------------------------------------------------------------------------
// Enumerations
//------------------------------------------------------------------------------------------------------------

///@enum MCU_SPI_MODE
///		There are 4 existing spi modes. For more information on them see [1].
typedef enum mcu_spi_mode_e
{
	/// Clock Idle Low -> Take over at first edge	-> CKPOL 0 CKPH 0
	MCU_SPI_MODE_0 = 0,		
	/// Clock Idle Low -> Take over at second edge -> CKPOL 0 CKPH 1
	MCU_SPI_MODE_1 = 1,		
	/// Clock Idle High-> Take over at first edge	-> CKPOL 1 CKPH 0
	MCU_SPI_MODE_2 = 2,		
	/// Clock Idle High-> Take over at second edge -> CKPOL 1 CKPH 1
	MCU_SPI_MODE_3 = 3		
}
MCU_SPI_MODE_T;

typedef enum mcu_spi_trans_flags_e
{
	/// @brief No flag is set, this is the default value assumed.
	MCU_SPI_TRANS_FLAGS_NONE 		= 0,
	/// @brief Transmit and receive data in 2-bit mode.
	MCU_SPI_TRANS_FLAGS_DIO 		= 0x0001,
	/// @brief Transmit and receive data in 4-bit mode.
	MCU_SPI_TRANS_FLAGS_QIO 		= 0x0002,
	/// @brief If set in flag, received data will be stored in @c r_data and not in @c r_buf of the transaction.
	// MCU_SPI_TRANS_FLAGS_RXDATA		= 0x0004,
	/// @brief If set in flag, sent data will be sent from @c w_data and not @c w_buf of the transaction.
	MCU_SPI_TRANS_FLAGS_TXDATA		= 0x0008,
}MCU_SPI_TRANS_FLAGS_T;

//------------------------------------------------------------------------------------------------------------
// Structures
//------------------------------------------------------------------------------------------------------------

/**
 * @brief Structure for a spi transaction that should be sent.
 * 
 * The data for an spi transaction is tranmitted as following:
 * CS        `|_____________________________________|`
 * CLK       _|`|_~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|`
 * MOSI/MISO `| CMD | ADDR | DUMMY | W_BUF / R_BUF |
 * With the corrsponding length to @c cmd, @c addr, dummy and @c w_buf / @c r_buf you can control whether these bytes shall be sent.
 * You can also skip each section by keeping it's length on 0. This way you can for example only send data or only receive data.
 * This transaction is also applicable for DSPI and QSPI interfaces.
 */
typedef struct mcu_spi_transaction_s
{
	/// @brief Bitwise or of the values in @c MCU_SPI_TRANS_FLAGS_T which is used to set multiple flags. On 0, it uses no flag.
	MCU_SPI_TRANS_FLAGS_T flags;
	/// @brief Command instruction to send first in the transaction.
	/// It will be sent MSB first. If you enter 0x00BC with cmd_length 1, the instruction 0xBC is sent. If you use 0xBC00 with cmd_length 2, the instruction 0xBC and afterwards 0x00 are sent.
	uint16_t cmd;
	/// @brief Address that is sent first or after @c cmd in the transaction.
	/// It will be sent MSB first. If you enter 0x123456 with length 3, the bytes 0x12, 0x34 and 0x56 will be sent in this order.
	uint64_t addr;
	/// @brief Number of bytes from @c cmd that should be sent.
	uint8_t cmd_length;
	/// @brief Number of bytes from @c addr that should be sent.
	uint8_t addr_length;
	/// @brief Number of dummy bytes that should be sent. Dummy bytes are sent as 0xFF.
	uint8_t dummy_length;
	// Union for write buffer pointer or the content of the write buffer, if it only needs to be up to 4 bytes.
	union 
	{
		/// @brief Pointer to the buffer used for sending data. @c w_data is used instead, if @c MCU_SPI_TRANS_FLAGS_TXDATA is set in the flags.
		const void* w_buf;
		/// @brief Buffer used for sending data. @c w_buf is used by default. To use this @c w_data you need to set @c MCU_SPI_TRANS_FLAGS_TXDATA in the flags.
		uint8_t w_data[4];
	};
	/// @brief Number of bytes from @c w_buf that should be sent.
	uint32_t w_buf_length;
	// Union for read buffer pointer or the content of the write buffer, if it only needs to be up to 4 bytes.
	union
	{
		/// @brief Pointer to the buffer used for receiving data. @c r_data is used instead, if @c MCU_SPI_TRANS_FLAGS_RXDATA is set in the flags.
		void* r_buf;
		/// @brief Buffer used for receiving data. @c r_buf is used by default. To use this @c r_data you need to set @c MCU_SPI_TRANS_FLAGS_RXDATA in the flags.
		// uint8_t r_data[4];
	};
	/// @brief Number of bytes from @c r_buf that should be sent.
	uint32_t r_buf_length;

}mcu_spi_transaction_t;

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_DEVICE_COUNT_SPI>0
#ifdef _RENESAS_SYNERGY_
/**
 * Creates an SPI Handler for the corresponding I/O Port. The SPI interface must be created via configuration.xml.
 * @param h     SPI interface as created via configuration.xml.
 * @param cs    I/O for Chip select.
 */
MCU_RESULT mcu_spi_init(mcu_spi_t h, MCU_IO_PIN cs);
#else
/**
 * @brief 	Creates an SPI handler for the corresponding i/o ports. If the spi handler cannot be created NULL is returnd and the mcu_get_last_error function should be checked.
 *
 *			The following errors can be set:
 * 							MCU_ERROR_SPI_NOT_AVAILABLE: 	All SPI interfaces are in use.
 *															The MCU_PERIPHERY_DEVICE_COUNT_SPI define in mcu_config.h might be too low. Check it out.
 * 							MCU_ERROR_SPI_INVALID: 			There is no spi interface on the given i/o ports.
 *	
 * @pre		Check if MCU_PERIPHERY_DEVICE_COUNT_SPI is high enough.
 *
 * @param num				Number of the SPI interface.
 * @param tx      			I/O for MOSI / TX.
 * @param rx				I/O for MISO / RX.
 * @param clk				I/O for the generated Clock.
 * @param cs				I/O for Chip select.
 * @return					NULL: No SPI handler could be created.
 *							Else: Pointer to the SPI handler needed for other functions.
 */
mcu_spi_t mcu_spi_init(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx, MCU_IO_PIN clk, MCU_IO_PIN cs);
#endif

/**
 * @brief Frees the SPI Bus and changes the used pins back to being GPIO.
 * If there are multiple SPI handlers that use the same bus, freeing one will only free the chip select pin, but not miso, mosi and clock.
 * 
 * @param h					SPI handler, that was created with mcu_spi_init.
 */
void mcu_spi_free(mcu_spi_t h);

/**
 * @brief 	Get the device handle for the SPI that might be initialized internally for a mcu specific HAL.
 * Might be needed by driver calls of specific MCU functions that need the device handler of the SPI.
 * 
 * @param h					SPI handler, that was created with mcu_spi_init.
 * @return void* 			Device handler that was initialized internally.
 */
void* mcu_spi_get_device_handle(mcu_spi_t h);
/**
 * @brief 	Set the device handle for the SPI that might be initialized internally for a mcu specific HAL.
 * Might be needed by driver calls of specific MCU functions that need the device handler of the SPI.
 * 
 * Be careful if you use this function. Only call it if you know the consequences!
 * If used wrong, the mcu_spi functions for this handler might not work anymore.
 * 
 * @param h					SPI handler, that was created with mcu_spi_init.
 * @param dev 				Device handler that was initialized externally.
 */
void mcu_spi_set_device_handle(mcu_spi_t h, void* dev);
/**
 * @brief 	Return the SPI interface number that was set during @c mcu_spi_init.
 *  
 * @param h					SPI handler, that was created with @c mcu_spi_init.
 * @return uint8_t 			SPI interface number that was set during @c mcu_spi_init.
 */
uint8_t mcu_spi_get_num(mcu_spi_t h);
/**
 * @brief 	Return the Chip Select pin of the SPI interface.
 * 
 * @param h					SPI handler, that was created with @c mcu_spi_init.
 * @return MCU_IO_PIN 		Chip Select pin of the SPI interface.
 */
MCU_IO_PIN mcu_spi_get_cs(mcu_spi_t h);

/**
 * @brief 	Sets the parameters for the SPI handler.
 * 			Sets the SPI registers and sets the clock by calling mcu_spi_set_clock internally.
 *
 * @pre		The SPI handler must be created with mcu_spi_init.
 *
 * @param h					SPI handler, that was created with mcu_spi_init.
 * @param mode				SPI Mode 0-3. [1]
 * @param frq				SPI frequency in Hertz.
 * @return					MCU_OK: No error occured.\n
 * 							MCU_ERROR_SPI_INVALID : SPI handler is NULL.\n
 * 							MCU_ERROR_SPI_MODE_INVALID : SPI mode does not exist or is not supported.\n
 * 							MCU_ERROR_SPI_CLOCK_INVALID : The clock cannot be set (might be too high or too low).
 */
MCU_RESULT mcu_spi_set_param(mcu_spi_t h, MCU_SPI_MODE_T mode, uint32_t frq);

/**
 * @brief 	This function can be used to set the SPI clock.
 * 			Since the SPI clock cannot be set freely, because it depends on the mcu clock, the frq value is the maximum value that could be set.
 * 			If the frq value cannot be set exactly, the next possible lower value for frq is set for the clock. Check mcu_spi_get_frq to see the real
 * 			spi clock value.
 *
 * 			This function is called in mcu_spi_set_param. So it is only needed if only the clock should be changed.
 *
 * @pre		The SPI handler must be created with mcu_spi_init.
 *
 * @param h					SPI handler, that was created with mcu_spi_init.
 * @param frq				SPI frequency in Hertz.
 * @return					MCU_OK: No error occured.\n
 * 							MCU_ERROR_SPI_INVALID : SPI handler is null\n
 * 							MCU_ERROR_SPI_CLOCK_INVALID : The clock cannot be set (might be too high or too low).
 */
MCU_RESULT mcu_spi_set_clock(mcu_spi_t h, uint32_t frq);
/**
 * @brief	Returns the exact frequency, the spi interface uses.
 *
 * @pre		The SPI handler must be created with mcu_spi_init.
 *
 * @param h					SPI handler, that was created with mcu_spi_init.
 * @return					The exact frequency.
 */
uint32_t mcu_spi_get_frq(mcu_spi_t h);

FUNCTION_RETURN_T mcu_spi_transaction_start(mcu_spi_t h);

FUNCTION_RETURN_T mcu_spi_transaction_end(mcu_spi_t h);

FUNCTION_RETURN_T mcu_spi_transaction_add(mcu_spi_t h, mcu_spi_transaction_t t);

#define mcu_spi_transaction_add_(h, ...)	 	mcu_spi_transaction_add(h, (mcu_spi_transaction_t){__VA_ARGS__})

/**
 * @brief 	Writes a byte over spi. Sets the chip select pin, which was set in mcu_spi_init.
 *
 * @pre		The SPI handler must be created with mcu_spi_init.
 *
 * @param h					SPI handler, that was created with mcu_spi_init.
 * @param letter			Byte to send.
 * @return 					Returns the byte which is read while the letter was transmitted.
 */
uint8_t mcu_spi_send_with_cs(mcu_spi_t h, uint8_t letter);

/**
 * @brief 	Writes a byte over spi. Does not set chip select, so it must be set manually!
 * 			This function might be used if a block of spi data should be sent without changing chip select.
 * 			Use mcu_spi_set_chip_select to set the chip select pin set with mcu_spi_init.
 *
 * @pre		The SPI handler must be created with mcu_spi_init.
 *
 * @param h					SPI handler, that was created with mcu_spi_init.
 * @param letter			Byte to send.
 * @return 					Returns the byte which is read while the letter was transmitted.
 */
uint8_t mcu_spi_send(mcu_spi_t h, uint8_t letter);

/**
 *          Writes data over spi and writes the received data into the receive buffer.
 *          r_buf can be w_buf, if only a single buffer is used for sending and receiving.
 *
 * @param h                 SPI handler, that was created with mcu_spi_init.
 * @param w_buf             Pointer to the buffer of the data that should be written.
 * @param r_buf             Pointer to the buffer of the data where the response is stored.
 * @param len               Number of bytes in the buffer for writing and reading.
 */
void mcu_spi_send_buffer(mcu_spi_t h, uint8_t* w_buf, uint8_t* r_buf, uint32_t len);

/**
 * @brief 	Sets the spi chip select pin to 1 or 0.
 *
 * @pre		The SPI handler must be created with mcu_spi_init.
 *
 * @param h					SPI handler, that was created with mcu_spi_init.
 * @param state				0 or 1 to set chip select low or high.
 */
void mcu_spi_set_chip_select(mcu_spi_t h, uint8_t state);

#endif

#endif // __MCU_IO_SPI_HEADER_FIRST_INCLUDE__
