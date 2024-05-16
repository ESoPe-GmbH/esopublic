/**
 * 	@file 	mcu_timer.h
 * 	@copyright Urheberrecht 2011-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_timer functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *
 *  @version    2.00 (20.08.2022)
 *               - Extracted mcu_timer functions in this file. History from now on will only contain infos about mcu_timer.
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

#ifndef __MCU_TIMER_HEADER_FIRST_INCLUDE__
#define __MCU_TIMER_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

//------------------------------------------------------------------------------------------------------------
// Config structure
//------------------------------------------------------------------------------------------------------------

/**
 * @brief UART configuration variables
 */
typedef struct mcu_timer_config_s
{
    uint8_t unit;
    /// @brief Interrupt level of the timer.
    MCU_INT_LVL lvl;
    /// @brief The frequency for the timer in Hz (Example: 1ms = 1kHz so use 1000Hz).
    uint32_t frq_hz;
    /// @brief Callback function for the timer interrupt.
    /// The parameter of the callback is the \c obj parameter of this structure.
    void (*f)(void*);
    /// @brief Pointer that is used as a parameter in the timer callback \c f.
    void* obj;
    /// true: Start timer directly.
    /// false: Do not start the timer yet.
    bool auto_start;

}mcu_timer_config_t;

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_DEVICE_COUNT_TIMER>0
/**
 * @brief	Initializes a timer. The function tries to set the frequency, but in some cases it does not work 1000%. So check
 * 			mcu_timer_get_frq after this to see the frequency that was actually set.
 *
 *			The following errors might occur:
 * 			 - MCU_ERROR_TMR_NOT_AVAILABLE: 	All timer are in use. Adjust MCU_PERIPHERY_DEVICE_COUNT_TIMER in mcu_config.h
 * 			 - MCU_ERROR_TMR_FRQ_INVALID: 		Frequency is too high or too low and cannot be set.
 *	
 * @pre		Check MCU_PERIPHERY_DEVICE_COUNT_TIMER in the mcu_config.h before.
 *
 * @param lvl				Interrupt level of the timer.
 * @param frq_hz			The frequency for the timer in Hz (Example: 1ms = 1kHz so use 1000Hz).
 * 							Check mcu_timer_get_frq to see if it was possible to set the frequency.
 * @param f					Callback function for the timer interrupt.
 * 							The parameter of the callback is the \c obj parameter of this function.
 * @param obj				Pointer that is used as a parameter in the timer callback \c f.
 * @param auto_start		true: Start timer directly.\n
 * 							false: Do not start the timer yet.
 * @return					NULL: The timer could not be initialized.
 *							Other then NULL: The timer handler.
 */
mcu_timer_t mcu_timer_init(MCU_INT_LVL lvl, uint32_t frq_hz, void (*f)(void*), void* obj, bool auto_start);

/**
 * @brief	Initializes a timer. The function tries to set the frequency, but in some cases it does not work 1000%. So check
 * 			mcu_timer_get_frq after this to see the frequency that was actually set.
 *
 *			The following errors might occur:
 * 			 - MCU_ERROR_TMR_NOT_AVAILABLE: 	All timer are in use. Adjust MCU_PERIPHERY_DEVICE_COUNT_TIMER in mcu_config.h
 * 			 - MCU_ERROR_TMR_FRQ_INVALID: 		Frequency is too high or too low and cannot be set.
 * 
 * @param config            Pointer to the configuration 
 * @return					NULL: The timer could not be initialized.
 *							Other then NULL: The timer handler.
 */
mcu_timer_t mcu_timer_create(const mcu_timer_config_t* config);

/**
 * @brief   Returns microseconds since system start.
 * 
 * @return uint64_t         Elapsed microseconds since system start.
 */
uint64_t mcu_timer_get_microseconds(void);

/**
 * @brief	Starts the timer.
 *
 * @pre		handle must be created with mcu_timer_init.
 *
 * @param h					Timer handler.
 */
void mcu_timer_start(mcu_timer_t h);

/**
 * @brief	Stops the timer.
 *
 * @pre		handle must be created with mcu_timer_init.
 *
 * @param h					Timer handler.
 */
void mcu_timer_stop(mcu_timer_t h);
/**
 * Reset the timer internal counter to start counting by 0 until the time for the set frequency is reached.
 *
 * @param h					Timer handler.
 */
void mcu_timer_reset_counter(mcu_timer_t h);

/**
 * @brief	Sets the frequency of the timer.
 *
 * @pre		handle must be created with mcu_timer_init.
 *
 * @param h					Timer handler.
 * @param frq_hz			The frequency in the timer.
 */
MCU_RESULT mcu_timer_set_frq(mcu_timer_t h, uint32_t frq_hz);

/**
 * @brief	Returns the frequency of the timer.
 *
 * @pre		handle must be created with mcu_timer_init.
 *
 * @param h					Timer handler.
 * @return					The frequency set in the timer.
 */
uint32_t mcu_timer_get_frq(mcu_timer_t h);
#endif

#endif // __MCU_TIMER_HEADER_FIRST_INCLUDE__
