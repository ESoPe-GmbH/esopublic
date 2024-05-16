/**
 * 	@file 	mcu_can.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_can functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *	@subsection	mcu_htu_can_init CAN Bus
 *
 *	The can bus periphery is not offered in all controllers. To use it you need to initialize it first (like uarts).
 *	The initialization function needs the can number, tx and rx pin.
 *
 *	You need a can_handle for the can functions the example below shows.
@code
	mcu_can_t can_handle;
	...
	can_handle = mcu_can_init(1, P8_2, P8_3);	// CAN 1 with pin P8_2 and P8_3
@endcode
 *
 *	After the initialization you might want to receive data. This is a bit more complex then the uart receive.
 *	First you need to set the baudrate. Then you need to advice a receive buffer to the can module. These 2 steps are like the uart as shown
 *	in the example below.
@code
	uint8_t can_buffer[10*sizeof(can_frame_t)];	// Buffer f�r 10 Pakete
	...
	mcu_can_set_baudrate(can_handle, 1000000);	// 1 MBit/s
	mcu_can_receive_enable(can_handle, MCU_INT_LVL_MED, can_buffer, 10); // 10 Pakete
@endcode
 *
 * 	To receive packages you need to tell the can module which packages you want to receive. For this you register receive messageboxes with
 * 	an address and an addressmask. The addressmask is used to listen for multiple addresses. If you set a bit in the addressmask to 1
 * 	the can module only receives packages where the corresponding bit in the received address is the same es the address you set in the
 * 	register function.
@code
	mcu_can_register_receive_messagebox(can_handle, 0x1FFFFFF0, 0x05, true);	// Extended IDs	(29-Bit) 0x00 - 0x0F
	mcu_can_register_receive_messagebox(can_handle, 0, 0, false);	// All Standard IDs (10-Bit)
@endcode
 *
 *	Now you can send and receive data. Below is an example that simply sends a received package back to the bus.
@code
	can_frame_t can_rx;
	...
	if(mcu_can_receive_package(can_handle, &can_rx))	// Returns true when a package was received.
		mcu_can_transmit_package(can_handle, can_rx);	// Sends a can package.
@endcode
 *
 *  @version    2.00 (20.08.2022)
 *               - Extracted mcu_can functions in this file. History from now on will only contain infos about mcu_can
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

#ifndef __MCU_CAN_HEADER_FIRST_INCLUDE__
#define __MCU_CAN_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

#if MCU_PERIPHERY_ENABLE_COMM_MODE_CAN
	#include "module/comm/comm_type.h"
#endif

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_DEVICE_COUNT_CAN>0 && MCU_DEBUG_ENABLE
/**
 * @brief	Prints the content of the can frame on the debug comm.
 * @param p					Can Frame that should be printed.
 */
void mcu_debug_can_print(can_frame_t p);
#endif

#if MCU_PERIPHERY_DEVICE_COUNT_CAN>0
/**
 * @brief 	Creates a CAN handler on the given i/o ports. In case of an error you should check mcu_get_last_error.
 *
 *			The following errors might occur:
 * 							MCU_ERROR_CAN_NOT_AVAILABLE: 	All CAN interfaces are used.
 *															Check if MCU_PERIPHERY_DEVICE_COUNT_CAN in mcu_config.h is too low.
 * 							MCU_ERROR_CAN_INVALID: 			The i/o pins have no can functionality.
 *	
 * @pre		Check if MCU_PERIPHERY_DEVICE_COUNT_CAN in mcu_config.h is high enough.
 *
 * @param num				Number of the CAN port.
 * @param tx      			I/O pin of tx.
 * @param rx				I/O pin of rx.
 * @return					NULL: The CAN handler could not be created.
 *							Else: Pointer to the CAN handler.
 */
mcu_can_t mcu_can_init(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx);

/**
 * @brief 	Sets the baudrate for the can interface.
 *
 * @pre		The CAN handler must be created with mcu_can_init before.
 *
 * @param h					CAN handler, created with mcu_can_init.
 * @param baudrate			CAN Baudrate in Bit/s.
 * @return					MCU_OK: No error occurred.\n
 * 							MCU_ERROR_CAN_INVALID : The CAN handler is invalid.\n
 * 							MCU_ERROR_CAN_BAUDRATE_INVALID : The baudrate cannot be set. Might be too low or too high.
 */
MCU_RESULT mcu_can_set_baudrate(mcu_can_t h, uint32_t baudrate);

/**
 * @brief 	Returns the exact baudrate that is used for the can.
 *
 * @pre		The CAN handler must be created with mcu_can_init before.
 *
 * @param h					CAN handler, created with mcu_can_init.
 * @return					CAN Baudrate in Bit/s.
 */
uint32_t mcu_can_get_baudrate(mcu_can_t h);

/**
 * @brief 	This function is used to set a receive buffer for the can interface. Without this, the receive function will not work.
@code
	mcu_can_t can_handle;
	uint8_t can_buffer[10 * sizeof(can_frame_t)];	// Buffer for 10 Packages
	...
	mcu_can_receive_enable(can_handle, MCU_INT_LVL_MED, can_buffer, 10); // 10 Packages
@endcode
 *
 * @pre		The CAN handler must be created with mcu_can_init before.
 *
 * @param h					CAN handler, created with mcu_can_init.
 * @param lvl				Interrupt Level of the CAN Receive Interrupt.
 * @param buf				Pointer to the buffer that can be used to store received can packages.
 * @param buf_elements		Number of can frames that fit into the buffer.
 * @return					MCU_OK: No error occurred.\n
 * 							MCU_ERROR_CAN_INVALID : The CAN handler is invalid.\n
 * 							MCU_ERROR_CAN_RECEIVE_INVALID : Buffer is NULL or number of buffer elements is 0.
 */
MCU_RESULT mcu_can_receive_enable(mcu_can_t h, MCU_INT_LVL lvl, uint8_t *buf, uint16_t buf_elements);

/**
 * @brief 	Creates a message box for receiving can packages.
@code
	mcu_can_t can_handle;
	...
	mcu_can_register_receive_messagebox(can_handle, 0, 0, false); // Receives ALL Standard IDs
	mcu_can_register_receive_messagebox(can_handle, 0, 0, true); // Receives ALL Extended IDs
@endcode
 *
 * @pre		The CAN handler must be created with mcu_can_init before.
 *
 * @param h					CAN handler, created with mcu_can_init.
 * @param addr_mask			Mask for the addressfilter. This can be used like a subnetmask in tcp/ip. If a bit inside the mask is set to 1, then
 * 							the corresponfing bit inside the address must be the same as in the received package. If a bit in the mask is set to 0 the
 * 							corresponding bit inside the address can be different then in the received package.\n
 *							If the mask is 0, then all addresses can be received (Extended or standard address must be differentiated! See example above).\n
 *							If the mask is 0x1FFFFFFF (extended) or 0x7FF (standard), the received address must be the same as the address set with this function.
 * @param addr				Address of this message box. If the mask is 0 this address does not matter.
 * @param is_extended		true: This message box receives packages with extended address.\n
 *							false: This message box receives packages with standard address.\n
 * @return					MCU_OK: No error occurred.\n
 * 							MCU_ERROR_CAN_INVALID : The CAN handler is invalid.\n
 * 							MCU_ERROR_CAN_MESSAGE_BOX_INVALID : All message boxes are in use.
 */
MCU_RESULT mcu_can_register_receive_messagebox(mcu_can_t h, uint32_t addr_mask, uint32_t addr, bool is_extended);

/**
 * @brief 	Checks if a package was received. If a package was received it is copied into the pointed can_frame_t.
@code
	mcu_can_t can_handle;
	can_frame_t can_rx;
	...
	if(mcu_can_receive_package(can_handle, &can_rx))
		// Handle package
@endcode
 *
 * @pre		The CAN handler must be created with mcu_can_init before.
 * @pre		The baudrate must be set with mcu_can_set_baudrate.
 * @pre		A receive buffer must be set with mcu_can_receive_enable.
 * @pre		A receive message box must be set with mcu_can_register_receive_messagebox.
 *
 * @param h					CAN handler, created with mcu_can_init.
 * @param cf				Pointer to a can frame, the received package should be copied into.
 * @return					true: A package was received and it was copied into cf.\n
 *							false: Nothing received.
 */
bool mcu_can_receive_package(mcu_can_t h, can_frame_t *cf);

/**
 * @brief 	Sends the can package.
@code
	mcu_can_t can_handle;
	can_frame_t can_tx;
	...
	mcu_can_transmit_package(can_handle, &can_tx);
@endcode
 *
 * @pre		The CAN handler must be created with mcu_can_init before.
 * @pre		The baudrate must be set with mcu_can_set_baudrate.
 *
 * @param h					CAN handler, created with mcu_can_init.
 * @param cf				The can package that should be sent.
 */
void mcu_can_transmit_package(mcu_can_t h, can_frame_t cf);
#endif

#endif // __MCU_CAN_HEADER_FIRST_INCLUDE__
