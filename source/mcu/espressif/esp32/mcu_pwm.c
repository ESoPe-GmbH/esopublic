// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_pwm.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "mcu/mcu.h"

#if MCU_TYPE == MCU_ESP32 && MCU_PERIPHERY_DEVICE_COUNT_PWM > 0

#include "mcu_internal.h"
#include "../../peripheral/mcu_pwm.h"
#include "module/fifo/fifo.h"
#include <string.h>
#include "driver/ledc.h"
#include "module/convert/math.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Default led speed mode.
#define LEDC_MODE           LEDC_LOW_SPEED_MODE

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
        h->config.duty_cycle = 0;
        h->config.resolution = 13;
        h->config.frequency_hz = 1000;
    }
    else
        memcpy(&h->config, config, sizeof(mcu_pwm_config_t));

    FUNCTION_RETURN_T ret = mcu_pwm_update_config(h, &h->config);
    if(ret != FUNCTION_RETURN_OK)
    {
        ledc_stop(LEDC_MODE, h->hw.timer_channel, 0);
        mcu_heap_free(h);
        return NULL;
    }

    return h;
}

void mcu_pwm_free(mcu_pwm_t h)
{
    if(h == NULL)
        return;

    ledc_stop(LEDC_MODE, h->hw.timer_channel, 0);
    mcu_heap_free(h);
}

FUNCTION_RETURN_T mcu_pwm_update_config(mcu_pwm_t h, const mcu_pwm_config_t* config)
{
    esp_err_t ret;

    DBG_ASSERT(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid PWM handle\n");
    DBG_ASSERT(config, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Config cannot be NULL\n");
    DBG_ASSERT(config->resolution >= 8 && config->resolution <= 16, NO_ACTION, FUNCTION_RETURN_UNSUPPORTED, "Resolution invalid\n");

    if(config != &h->config)
        memcpy(&h->config, config, sizeof(mcu_pwm_config_t));

     // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = h->hw.timer_unit,
        .duty_resolution  = h->config.resolution,
        .freq_hz          = h->config.frequency_hz,  // Set output frequency at 1 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    }; 
    ret = ledc_timer_config(&ledc_timer);
    if(ret != ESP_OK)
        return FUNCTION_RETURN_PARAM_ERROR;

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = h->hw.timer_channel,
        .timer_sel      = h->hw.timer_unit,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = h->hw.output_pin,
        .duty           = h->config.duty_cycle, // Set duty to 0%
        .hpoint         = 0
    };
    ret = ledc_channel_config(&ledc_channel);
    if(ret != ESP_OK)
        return FUNCTION_RETURN_PARAM_ERROR;

    h->pwm_max_value = (uint32_t)pow(2.0, (double)h->config.resolution);

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T mcu_pwm_set_duty_cycle(mcu_pwm_t h, uint32_t duty_cycle)
{
    DBG_ASSERT(h, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid PWM handle\n");
    DBG_ASSERT(duty_cycle <= 10000, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid PWM duty cycle %u\n", duty_cycle);

    if(h->hw.invert)
        duty_cycle = 10000 - duty_cycle;

    if(duty_cycle == 0 || duty_cycle == 10000)
        ledc_stop(LEDC_MODE, h->hw.timer_channel, 10000 == duty_cycle);
    else
    {
		ledc_set_duty(LEDC_MODE, h->hw.timer_channel, duty_cycle * h->pwm_max_value / 10000);
        ledc_update_duty(LEDC_MODE, h->hw.timer_channel);
    }

    return FUNCTION_RETURN_OK;
}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------



#endif // #if MCU_TYPE
