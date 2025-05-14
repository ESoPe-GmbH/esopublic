// Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_pwm.c
 * 	@copyright Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "mcu/mcu.h"

#if MCU_TYPE == MCU_NEW_NAME && MCU_PERIPHERY_DEVICE_COUNT_PWM > 0

#include "mcu_internal.h"
#include "../../peripheral/mcu_pwm.h"
#include "module/fifo/fifo.h"
#include <string.h>
#include "module/convert/math.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct mcu_pwm_s
{
    /// Hardware configuration for the PWM.
    mcu_pwm_config_hw_t hw;
    /// Configuration for the PWM.
    mcu_pwm_config_t config;
    /// Maximum pwm value calculated 2^resolution
    uint32_t pwm_max_value;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

mcu_pwm_t mcu_pwm_create(const mcu_pwm_config_hw_t* hw, const mcu_pwm_config_t* config)
{
    if(hw == NULL)
        MCU_RETURN_ERROR(MCU_ERROR_NULL_PARAMETER, NULL);

    mcu_pwm_t h = mcu_heap_calloc(1, sizeof(struct mcu_pwm_s));
    if(h == NULL)
        MCU_RETURN_ERROR(MCU_ERROR_INSUFFICIENT_MEMORY, NULL);

    memcpy(&h->hw, hw, sizeof(mcu_pwm_config_hw_t));
    if(config == NULL)
    {
        // TODO: Set default config
        h->config.duty_cycle = 0;
        h->config.resolution = 13;
        h->config.frequency_hz = 1000;
    }
    else
    {
        memcpy(&h->config, config, sizeof(mcu_pwm_config_t));
    }

    // TODO: Initialize pwm timer or pin

    FUNCTION_RETURN_T ret = mcu_pwm_update_config(h, &h->config);
    if(ret != FUNCTION_RETURN_OK)
    {
        mcu_pwm_free(h);
        return NULL;
    }

    return h;
}

void mcu_pwm_free(mcu_pwm_t h)
{
    if(h == NULL)
        return;

    // TODO: Free or stop inner settings
    mcu_heap_free(h);
}

FUNCTION_RETURN_T mcu_pwm_update_config(mcu_pwm_t h, const mcu_pwm_config_t* config)
{
    DBG_ASSERT(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid PWM handle\n");
    DBG_ASSERT(config, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Config cannot be NULL\n");
    DBG_ASSERT(config->resolution >= 8 && config->resolution <= 16, NO_ACTION, FUNCTION_RETURN_UNSUPPORTED, "Resolution invalid\n");

    if(config != &h->config)
        memcpy(&h->config, config, sizeof(mcu_pwm_config_t));

    // TODO: Set config

    h->pwm_max_value = (uint32_t)pow(2.0, (double)h->config.resolution);

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T mcu_pwm_set_duty_cycle(mcu_pwm_t h, uint32_t duty_cycle)
{
    DBG_ASSERT(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid PWM handle\n");
    DBG_ASSERT(duty_cycle <= 10000, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid PWM duty cycle %u\n", duty_cycle);

    if(h->hw.invert)
        duty_cycle = 10000 - duty_cycle;

    // TODO: Set duty cycle

    return FUNCTION_RETURN_OK;
}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------



#endif // #if MCU_TYPE == MCU_NEW_NAME && MCU_PERIPHERY_DEVICE_COUNT_PWM > 0
