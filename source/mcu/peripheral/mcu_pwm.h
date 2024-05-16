/**
 * 	@file 	mcu_pwm.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author Tim Koczwara
 *
 *  @brief
 *  		Contains Definitions, Enumerations and Prototypes needed for the mcu_pwm functions, which can be used by
 *  		any MCU supporting this architecture.
 *
 *  @section	How-to-use
 *
 *
 *  @version    1.00 (20.08.2022)
 *  			 - Initial Release
 *
 ******************************************************************************/

#ifndef __MCU_PWM_HEADER_FIRST_INCLUDE__
#define __MCU_PWM_HEADER_FIRST_INCLUDE__

#include <stdint.h>
#include <stdbool.h>

#include "../mcu_types.h"

#include "module/enum/function_return.h"

#if MCU_PERIPHERY_DEVICE_COUNT_PWM > 0

/**
 * @brief Defines which timer to use and where to output the PWM signal.
 */
typedef struct
{
    /// The selected timer unit as it is often described in the datasheets as timer 0, 1, 2, etc.
    uint32_t timer_unit;
    /// The selected channel of the timer to use for this PWM. Often one timer has multiple channels for multiple outputs of the same pwm.
    /// This could be important because if you use multiple pwm on the same timer_unit and change the frequency, the frequency will be applied to all of them.
    uint32_t timer_channel;
    /// Output pin of the timer.
    MCU_IO_PIN output_pin;
    /// Indicate that the PWM should be inverted, so setting 0% will lead to the output of 100% and 20% will become 80%.
    bool invert;
}mcu_pwm_config_hw_t;
/**
 * @brief Contains information about timings and such. 
 */
typedef struct
{
    /// Resolution of the timer value in bits. For example when you set 13 the PWM will use a 13-Bit resolution, which means that you can set the values 0 to 2^13-1 inside duty_cycle.
    /// Default value is 13. Minimum value is 8. Maximum value is 16.
    uint16_t resolution;
    /// Duty cycle for the PWM based on the resolution.
    /// Default value is 0. Minimum value is 0. Maximum value is 2^resolution - 1.
    uint32_t duty_cycle;
    /// Frequency for the PWM in Hertz.
    /// Default value is 1000. Minimum value is 1. Maximum value is 44000.
    /// Keep in mind that different MCU might also limit the frequency range. This peripheral will try to set the closest possible value.
    uint32_t frequency_hz;
}mcu_pwm_config_t;

//------------------------------------------------------------------------------------------------------------
// MCU functions
//------------------------------------------------------------------------------------------------------------

/**
 * @brief	Creates a timer for PWM output functionality.
 *          The mcu_pwm_config_hw_t defines which timer to use and where to output the PWM signal.
 *          The mcu_pwm_config_t contains information about timings and such. 
 *
 *			The following errors might occur:
 * 			 - MCU_ERROR_TMR_NOT_AVAILABLE: 	The timer you wanted cannot be used.
 * 			 - MCU_ERROR_TMR_FRQ_INVALID: 		Frequency is too high or too low and cannot be set.
 *	
 * @pre		Check MCU_PERIPHERY_DEVICE_COUNT_PWM in the mcu_config.h before.
 *
 * @param hw                Configuration struct for the hardware settings of the pwm.
 * @param config            Configuration struct for the PWM to set its values. Can be NULL for default 0% output and applies later via mcu_pwm_update_config.
 * @return					NULL: The PWM could not be created.
 *							Other then NULL: The pwm handler.
 */
mcu_pwm_t mcu_pwm_create(const mcu_pwm_config_hw_t* hw, const mcu_pwm_config_t* config);
/**
 * @brief   Frees a PWM that was created using `mcu_pwm_create`. Do not use its handle anymore afterwards.
 * 
 * @param h                 Handle of the PWM as created in `mcu_pwm_create`. 
 */
void mcu_pwm_free(mcu_pwm_t h);
/**
 * @brief   Changes the internal configuration to the given values.
 * 
 * @note    If you only want to change the duty cycle, you can use `mcu_pwm_set_duty_cycle`. 
 *          Keep in mind that in this function, the configured resolution is range for the duty cycle, while `mcu_pwm_set_duty_cycle` uses 10000 as a fix range.
 * 
 * @param h                 Handle of the PWM as created in `mcu_pwm_create`. 
 * @param config            Configuration struct for the PWM to set its values.
 * 
 * @retval FUNCTION_RETURN_OK on success.
 * @retval FUNCTION_RETURN_PARAM_ERROR when the parameter are invalid/NULL, the duty cycle is out of range.
 * @retval FUNCTION_RETURN_UNSUPPORTED when the resolution is not applicable.
 */
FUNCTION_RETURN_T mcu_pwm_update_config(mcu_pwm_t h, const mcu_pwm_config_t* config);
/**
 * @brief   Changes the duty cycle. Does also stop the timer when duty cycle is 0 or 10000 by keeping the output on the desired level.
 * 
 * @note    You can alternatively update the duty cycle by calling `mcu_pwm_update_config` with a new duty cycle value.
 *          Keep in mind that in this function, 10000 is range for the duty cycle, while `mcu_pwm_update_config` uses resolution as a range.
 * 
 * @param h                 Handle of the PWM as created in `mcu_pwm_create`. 
 * @param duty_cycle        Duty cycle in 1/100 percent to achieve finer resolution.
 * 
 * @retval FUNCTION_RETURN_OK on success.
 * @retval FUNCTION_RETURN_PARAM_ERROR when the parameter/NULL are invalid or the duty cycle is > 10000.
 */
FUNCTION_RETURN_T mcu_pwm_set_duty_cycle(mcu_pwm_t h, uint32_t duty_cycle);

#endif

#endif // __MCU_PWM_HEADER_FIRST_INCLUDE__
