/**
 * 	@file 		mcu_internal.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		Contains functionality common for all mcu.
 *
 *  @version	1.00 (20.08.2022)
 *  			 - Initial release
 *
 */

#ifndef MCU_COMMON_HEADER_FIRST_INCLUDE_GUARD
#define MCU_COMMON_HEADER_FIRST_INCLUDE_GUARD

#include "../mcu_config.h"

#include "../sys.h"
#include "module/comm/comm.h"
#include "module/enum/function_return.h"
#include "module/fifo/fifo.h"

extern MCU_RESULT mcu_last_error;

extern uint32_t mcu_frq_ext_hz;
extern uint32_t mcu_frq_cpu_hz;
extern uint32_t mcu_frq_peripheral_hz;
extern uint32_t mcu_frq_peripheral_bus_hz;

/**
 * @brief Sets the I/O direction of a registered I/O expander.
 * 
 * @param p 	Pin to set.
 * @param d 	Direction to set.
 */
void mcu_io_set_dir_expander(MCU_IO_PIN p, MCU_IO_DIRECTION d);
/**
 * @brief Sets the I/O pull-up of a registered I/O expander.
 * 
 * @param p 	Pin to set.
 * @param pullup_active 	true to enable, false to disable
 */
void mcu_io_set_pullup_expander(MCU_IO_PIN p, bool pullup_active);
/**
 * @brief Set the I/O level output of a registered I/O expander.
 * 
 * @param p 	Pin to set.
 * @param d 	I/O level to set (1 or 0).
 */
void mcu_io_set_expander(MCU_IO_PIN p, uint8_t d);
/**
 * @brief Toggles the I/O level output of a registered I/O expander.
 * 
 * @param p 	Pin to set.
 */
void mcu_io_toggle_expander(MCU_IO_PIN p);
/**
 * @brief Reads the I/O level output of a registered I/O expander.
 * 
 * @param p 	Pin to get.
 * @return 		I/O level (1 or 0). 
 */
extern uint8_t mcu_io_get_expander(MCU_IO_PIN p);
/**
 * @brief Initialize the I/O interrupt for the pin on a registered I/O expander.
 * 
 * @param p 	Pin that needs the I/O interrupt initialized.
 * @return 		Indicates whether initalization was successful
 */
extern FUNCTION_RETURN mcu_io_interrupt_init_expander(MCU_IO_PIN p);
/**
 * @brief Set the configuration for an I/O interrupt of a registered expander.
 * 
 * @param p 	Pin that needs the I/O interrupt configuation.
 * @param obj	Pointer to an object that will be used as a parameter in the callback function.\n
 * 				Can be NULL if no object pointer is needed.
 * @param f		Callback function for the io interrupt.\n
 * 				The callback function has one parameter which is always the same as the obj parameter in this function.
 * @param edge	Interrupt edge trigger: rising, falling or both edges.
 * @return 		Indicates whether setting the parameter was successful
 */
extern FUNCTION_RETURN mcu_io_interrupt_set_param_expander(MCU_IO_PIN p, void* obj, void (*f)(void*), MCU_IO_INT_EDGE edge);
/**
 * @brief Enables the I/O interrupt for the pin on a registered I/O expander.
 * 
 * @param p 	Pin that needs the I/O interrupt enable.
 */
extern void mcu_io_interrupt_enable_expander(MCU_IO_PIN p);
/**
 * @brief Disables the I/O interrupt for the pin on a registered I/O expander.
 * 
 * @param p 	Pin that needs the I/O interrupt disable.
 */
extern void mcu_io_interrupt_disable_expander(MCU_IO_PIN p);

#define MCU_RETURN_ERROR(err, ret)													\
				{																	\
					mcu_last_error = err;											\
					return ret;														\
				}

#endif // MCU_COMMON_HEADER_FIRST_INCLUDE_GUARD
