// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 	mcu.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the MCU Functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *	@subsection	mcu_htu_init MCU initialization
 *		This header offers three functions, which can be used for initializing the MCU clock. The first
 *		function @link mcu_init mcu_init@endlink has three parameters for the external clock frequency,
 *		the CPU frequency and the peripheral frequency. The first one is fixed on each board. The others are
 *		calculated with the MCU dividers, so that the assigned frequency is nearly matched.
 *		The second function for initializing @link mcu_init_max_external mcu_init_max_external@endlink
 *		only needs the external clock frequency and sets the other frequencies to the maximum the MCU offers.
 *		At third the header offers a function @link mcu_init_max_internal mcu_init_max_internal@endlink
 *		which can be used for MCU which do not need an external clock. It sets the internal clocks to a maximum
 *		without using an external clock.
 *
 *		@warning Be careful using the maximum functions because there might be a cooler necessary.
 *
 *		All three functions give back an error code which can be looked at with a debugger.
 *		There are also functions, which contain the real calculated frequencies for the external, CPU and peripheral
 *		clock. These functions might be useful if there are some calculations necessary which need the real
 *		frequencies. These functions are @link mcu_get_frq_external mcu_get_frq_external@endlink for the
 *		external, @link mcu_get_frq_cpu mcu_get_frq_cpu@endlink for the CPU and @link mcu_get_frq_peripheral
 *		mcu_get_frq_peripheral@endlink for the peripheral frequency.
 *
 *		The example below shows how the initialization function might be used with a 10MHz external frequency.
 *		The CPU clock is set to 60 MHz and the peripheral clock is set to 30 MHz.
@code
	mcu_init(10000000, 60000000, 30000000);
@endcode
 *
 * 		In the second example below the MCU is initialized with a 10 MHz external clock, but the CPU and peripheral
 * 		clock are maximized by the MCU.
@code
	mcu_init_max_external(10000000);
@endcode
 *
 * 		The third example below shows the initialization without an external clock and with other frequencies
 * 		maximized.
@code
	mcu_init_max_internal();
@endcode
 *
 *		@attention The initialization functions should be called once and only at the beginning of the program.
 *
 *		With the functions above the MCU is initialized and can be used. But there exist 3 more functions which should
 *		be mentioned. The first two are used for enabling and disabling the global interrupts, which might be necessary
 *		in some times. These functions are @link mcu_enable_interrupt mcu_enable_interrupt@endlink and
 *		@link mcu_disable_interrupt mcu_disable_interrupt@endlink. The third function can be used for restarting the
 *		MCU with a software reset. This function is called @link mcu_soft_reset mcu_soft_reset@endlink.
 *
 *		@attention The initialization functions should be called once and only at the beginning of the program.
 *
 *		With the functions above the MCU is initialized and can be used. But there exist 3 more functions which should
 *		be mentioned. The first two are used for enabling and disabling the global interrupts, which might be necessary
 *		in some times. These functions are @link mcu_enable_interrupt mcu_enable_interrupt@endlink and
 *		@link mcu_disable_interrupt mcu_disable_interrupt@endlink. The third function can be used for restarting the
 *		MCU with a software reset. This function is called @link mcu_soft_reset mcu_soft_reset@endlink.
 *
 * 	@subsection mcu_htu_io MCU IO Ports
 * 		There exist 5 functions for the digital IO ports.\n
 * 		The first ones, which should be called before using the IO Ports are @link mcu_io_set_port_dir
 * 		mcu_io_set_port_dir@endlink and @link mcu_io_set_dir mcu_io_set_dir@endlink which set the digital IO to
 * 		Input or Output. The functions can be used to set the whole Port with 8 pins at once or for each pin.
 * 		See the example below for an usage example.
@code
	mcu_io_set_port_dir(PA, 0xF0); // Port A[7:4] Output [3:0] Input
	mcu_io_set_dir(PB_0, MCU_IO_DIR_OUT); // Port B 0 Output
	mcu_io_set_dir(PB_1, MCU_IO_DIR_IN); // Port B 1 Input
@endcode
 *
 *		To set high or low levels to Output pins or ports the @link mcu_io_set mcu_io_set@endlink functions can be
 *		used. Continuing the above example the example below shows the usage of setting output pins.
 *
 *		@attention Some Output ports are N-Channel Open Drain outputs and need an external resistor.
 *
@code
	mcu_io_set(PA, 0xC0);	// Port A[7:6] High, [5:4] Low and [3:0] are not set because they are Input
	mcu_io_set(PB_0, 1);	// Port B 0 High
@endcode
 * 		Many MCU contain internal pull-up resistors for input ports. These can be set with the @link mcu_io_set_pullup
 * 		mcu_io_set_pullup@endlink function.
 *
 * 		@attention Some MCU have no pull-up for every Input!
 * 		@attention Some MCU have shared pull-up registers, so that activating one pull-up also activates pull-ups for
 * 				other inputs.
 *
@code
	mcu_io_set_pullup(PA_2, true);	// Port A 2 Pulled Up
@endcode
 *
 *		The last function for using the IO pins is the @link mcu_io_get mcu_io_get@endlink functions, which is used
 *		for reading the Input states of the Pins or the whole Port.
@code
	uint8_t port_a_value = mcu_io_get(PA);	// Read Port A [7:0]
	uint8_t pin_b_1_value = mcu_io_get(PB_1);	// Read Port B 1
@endcode
 *
 * 	@subsection mcu_htu_io_handle Using IO Pins with a handler
 * 		The motivation for using the IO Pins is simple. Sometimes you have a transistor which inverts an input or
 * 		output signal and do not want to care inside your software module. The Handler Object contains the IO Pin and
 * 		a boolean variable for inverting. It is used for reading and writing the output pin corresponding to the
 * 		inverted value. Since it is a structure you can also access the pin variable inside the structure if you need
 * 		to use any of the IO functions from the section above.
 *
 * 		The example below shows how the handler can be used from initialization to reading and writing.
@code
	mcu_io_handler_t mih1 = MCU_IO_HANDLER_INIT(P1_7, false);	// Pin 1_7 Handler not inverted
	mcu_io_handler_t mih2 = MCU_IO_HANDLER_INIT(P1_6, true);	// Pin 1_6 Handler inverted
	
	MCU_IO_SET_HANDLER(mih1, 1);	// P1_7 sets a High level
	MCU_IO_SET_HANDLER(mih2, 1);	// P1_6 sets a Low level
	
	value = MCU_IO_GET_HANDLER(mih1);	// P1_7 = High -> value = 1
	value = MCU_IO_GET_HANDLER(mih2);	// P1_6 = High -> value = 0
@endcode 
 *
 * 	@subsection mcu_htu_handler MCU Peripheral Handler
 * 		The following sections explain the MCU periphery. All of them are using handler types, which is created in the
 * 		initialization function and is always the first parameter in each function which is used by the periphery.
 *
 * 		Since the handler are statically allocated you need to configure in the mcu_config.h how many handler are
 * 		reserved for each module. This is needed to save space and for completely disabling a peripheral module if it
 * 		is not needed for a project.
 *
 * 		There are 2 possible define names for the configuration of a peripheral module.
 * 		- MCU_PERIPHERY_ENABLE_ is used if a peripheral module only has one instance like the flash module. With
 * 			this define it can be enabled or disabled
 * 		- MCU_PERIPHERY_DEVICE_COUNT_ tells the compiler how many handler might be reserved for the peripheral module.
 * 			If set to zero the peripheral module will be disabled.
 *
 * 		The initialization functions return a handler for the internal structure of the peripheral module. If the
 * 		handler is NULL the initialization went wrong and the peripheral module can not be used with this handler.
 *
 *		The motivation behind the handler is to use UART, SPI, etc without knowing the number of them. So you can
 *		initialize a handler in the board module and then use the periphery in a module without knowing the internal
 *		number of the periphery or its port pins (in case of a UART).
 *
 *
 *
 *
 *
 *
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

#ifndef MCU_INIT_HEADER_FIRST_INCLUDE_GUARD
#define MCU_INIT_HEADER_FIRST_INCLUDE_GUARD

#ifdef __RENESAS__
#ifndef __RX
#define __RX
#endif
#endif

#include <stdint.h>
#include <stdbool.h>

#include "mcu_types.h"

// Peripherals
#include "peripheral/mcu_ad.h"
#include "peripheral/mcu_can.h"
#include "peripheral/mcu_da.h"
#include "peripheral/mcu_flash.h"
#include "peripheral/mcu_i2c.h"
#include "peripheral/mcu_io_interrupt.h"
#include "peripheral/mcu_io.h"
#include "peripheral/mcu_pwm.h"
#include "peripheral/mcu_rtc.h"
#include "peripheral/mcu_spi.h"
#include "peripheral/mcu_timer.h"
#include "peripheral/mcu_uart.h"
#include "peripheral/mcu_watchdog.h"

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

// Initialisierung
/**
 * @brief	Initializes the internal clocks of the mcu with the parameters.
 *
 * @param frq_ext			Frequency of the external clock.
 * @param frq_cpu			Frequency that should be set for the system clock.
 * @param frq_peripheral	Frequency that should be set for the peripheral clock.
 */
void mcu_init(uint32_t frq_ext, uint32_t frq_cpu, uint32_t frq_peripheral);
/**
 * @brief	Initializes the internal clocks to the maximum without using an external clock. This is not possible with some mcus.
 */
void mcu_init_max_internal();
/**
 * @brief	Initializes the internal clocks to the maximum using an external clock.
 *
 * @param frq_ext			Frequency of the external clock.
 */
void mcu_init_max_external(uint32_t frq_ext);
/**
 * Registers a listener that is used to be notified when the clock frequency is changed.
 * The listener is used inside a list, the next variable of @ref mcu_clock_change_listener_t should not be changed externally.
 * Set the function and object of the listener before calling the register function.
 *
 * @param listener			Pointer of the listener that is used to be notified when the clock frequency is changed.
 */
void mcu_register_clock_change(mcu_clock_change_listener_t* listener);

// Clock
/**
 * @return Frequency of the external clock in Hertz.
 */
uint32_t mcu_get_frq_external(void);
/**
 * @return Frequency of the system clock in Hertz.
 */
uint32_t mcu_get_frq_cpu(void);
/**
 * @return Frequency of the peripheral clock in Hertz.
 */
uint32_t mcu_get_frq_peripheral(void);

/**
 * Enters the sleep mode with the time period the mcu should sleep.
 * @param sleep_time		Time to sleep in milliseconds
 * @return					Real time that was set to sleep in milliseconds.
 */
uint32_t mcu_enter_sleep_mode(uint32_t sleep_time);

// Interrupt
/**
 * @brief	Enables the usage of interrupts. If this is not called at least once no interrupt is going to work!
 */
void mcu_enable_interrupt(void);	
/**
 * @brief	Disable the usage of interrupts.
 */
void mcu_disable_interrupt(void);		

// Reset

/**
 * @brief	Resets the mcu.
 */
void mcu_soft_reset(void);		

// Delay
/**
 * @brief 	Waits actively for the given time in microseconds.
 *
 *  @param delay			Time to wait in microseconds.
 */
void mcu_wait_us(uint16_t delay);
/**
 * Waits until the number of microseconds are reached or until the function in the parameter returns true. If function pointer is NULL, this
 * function has the same effect as mcu_wait_us.
 * @param wait_max			Maximum waiting time in microseconds
 * @param f					Pointer to a function indicating that there is no need to wait anymore. If function returns true, the wait stops.
 * @param obj				Pointer to an object that is used as the function parameter. Might be NULL.
 * @retval true				Function ended because f returned true
 * @retval false			Function ended because wait_max was reached
 */
bool mcu_wait_us_until(uint16_t wait_max, bool(*f)(void*), void* obj);
/**
 * @brief 	Waits actively for the given time in milliseconds.
 * 			Uses a timer on most mcus, so make sure, that 2 timers are enabled in mcu config.
 *
 *  @param delay			Time to wait in milliseconds.
 */
void mcu_wait_ms(uint16_t delay);
/**
 * Waits until the number of milliseconds are reached or until the function in the parameter returns true. If function pointer is NULL, this
 * function has the same effect as mcu_wait_ms.
 * @param wait_max			Maximum waiting time in milliseconds
 * @param f					Pointer to a function indicating that there is no need to wait anymore. If function returns true, the wait stops.
 * @param obj				Pointer to an object that is used as the function parameter. Might be NULL.
 * @retval true				Function ended because f returned true
 * @retval false			Function ended because wait_max was reached
 */
bool mcu_wait_ms_until(uint16_t wait_max, bool(*f)(void*), void* obj);

// Debug functions
#if MCU_DEBUG_ENABLE
/**
 * @brief 	Prints all clocks, uarts, spi, etc. to the debug interface which is set inside the comm module.
 *
 * @pre		You should have set a debug interface with comm_set_debug before calling this function.
 **/
void mcu_debug_print_usage(void);

/**
 * @brief 	Prints the name of a MCU_RESULT element as a string. If the element is not yet included "UNKNOWN RESULT" will be returned.
 *
 * @param res		Result code from which the name should be returned.
 * @return			Pointer to the string with the name of the code.
 **/
const char* mcu_get_result_str(MCU_RESULT res);

/**
 * @brief 	Prints the name of the last error MCU_RESULT element as a string.
 * 			If the element is not yet included "UNKNOWN RESULT" will be returned.
 *			Same as mcu_get_result_str(mcu_get_last_error());
 *
 * @return			Pointer to the string with the name of the code.
 **/
const char* mcu_get_last_error_str(void);

/**
 * @brief 	Returns the last MCU_RESULT error code or MCU_OK.
 *
 * @return			Last set MCU_RESULT.
 **/
MCU_RESULT mcu_get_last_error(void);
#endif

#endif
