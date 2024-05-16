// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 	led.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			The led module is used for life led handling, etc.
 *
 *	@version	1.05 (19.01.2022)
 * 	    - Modified to be used in esopekernel
 *  @version    1.04 (10.04.2018)
 *      - Added pulsing
 *	@version 	1.03 (07.06.2018)
 *		- Added module.h support
 *		- Changed led_init
 *		- renamed led_t to led_t
 *	@version 	1.02 (24.05.2018)
 *		- Using task handler
 *		- Added led_set_blinking_ms and led_set_blinking_hz
 *  @version	1.01 (04.05.2014)
 *  	- led_set and led_toggle added
 *  @version	1.00 (19.08.2013)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef LED_H_
#define LED_H_

#include "module_public.h"
#if MODULE_ENABLE_LED
#include "mcu/sys.h"
#include "module/enum/function_return.h"

//------------------------------------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------------------------------------
/// Version of the led module
#define LED_STR_VERSION "1.02"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Typedef for led structure.
typedef struct led_s led_t;

/// @struct led_t
/// Structure for handling a led.
struct led_s
{
	/// Pointer to the mcu_io_handler_t used for the led.
	mcu_io_handler_t pin;
	/// Timestamp used for calculating the time for toggling the led.
	uint32_t timestamp;
	/// If the LED shall blink, the interval is set in ms.
	uint32_t blinking_interval_ms;
    /// If the LED shall pulse, the interval is set in ms
    uint32_t pulsing_interval_ms;
    /// Is used to check the state when pulsing
    bool is_on;
	/// Task for blinking led.
	system_task_t task;
	/// Pointer for next LED
	led_t* next;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief 		Initializes the led structure, sets the pin as output pin and switches the led off.
 *
 * @param s								Pointer to the led_structure used for the led pin.
 * @param pin							MCU Pin for the LED.
 * @param is_inverted					Set to true if the LED is on when the pin is set to 0.
 */
void led_init(led_t *s, MCU_IO_PIN pin, bool is_inverted);

/**
 * @brief 		Registers an LED for the console. Do not call in board_init, since the console is not started there.
 * @param s								Pointer to the led_structure used for the led pin.
 * @param name							Name of LED that is used for the task name and for @see led_get_handle.
 * @return								FUNCTION_RETURN value as used in @see list_add.
 */
FUNCTION_RETURN led_register(led_t* s, const char* name);

/**
 * Returns the pointer to the LED structure for the corresponding name.
 * @param name							Name of the LED as used in @see led_init.
 * @return								NULL if no LED was found with this name or a pointer to the handle of the LED.
 */
led_t* led_get_pointer(const char* name);

/**
 * @brief 		After calling this function the LED will toggle its state after the given interval.
 *
 * @param s								Pointer to the led_structure used for the led pin.
 * @param interval						Number of milliseconds after which the led output toggles.
 */
void led_set_blinking_ms(led_t *s, uint32_t interval);

/**
 * @brief 		After calling this function the LED will toggle its state by the given frequency in Hz.
 *
 * @param s								Pointer to the led_structure used for the led pin.
 * @param interval						Frequency with which the LED toggles (Maximum is 1000Hz).
 */
void led_set_blinking_hz(led_t *s, uint32_t interval);

/**
 * @brief       After calling this function the LED will pulse after the given interval.
 *
 * @param s             Pointer to the led_structure used for the led pin.
 * @param on_time       Number of milliseconds the LED is ON
 * @param off_time      Number of milliseconds the LED is OFF
 */
void led_set_pulsing_ms(led_t *s, uint32_t on_time, uint32_t off_time);

/**
 * @brief 		Switches the led on/off.
 *
 * @param s								Pointer to the led_structure used for the led pin.
 * @param switch_on						true switches led on, false switches led off.
 */
void led_set(led_t *s, bool switch_on);

/**
 * @brief 		Toggles the led.
 *
 * @param s								Pointer to the led_structure used for the led pin.
 */
void led_toggle(led_t *s);

/**
 * @brief 		Switches the led on.
 *
 * @param s								Pointer to the led_structure used for the led pin.
 */
void led_on(led_t *s);

/**
 * @brief 		Switches the led off.
 *
 * @param s								Pointer to the led_structure used for the led pin.
 */
void led_off(led_t *s);

#endif

#endif /* LED_H_ */
