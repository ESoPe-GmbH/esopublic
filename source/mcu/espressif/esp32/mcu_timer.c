/**
 * 	@file 		mcu_timer.c
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_ESP32

#include "../esp32/mcu_internal.h"
#include <string.h>
#include <stdlib.h>

#include "esp_timer.h"

#if MCU_PERIPHERY_DEVICE_COUNT_TIMER > 0
				
struct mcu_timer_s *mcu_timer_handler_hash[MCU_TIMER_TOTAL_COUNT] = {0};
uint8_t mcu_current_timer_handler = 0;
struct mcu_timer_s mcu_timer_handler[MCU_PERIPHERY_DEVICE_COUNT_TIMER];

mcu_timer_t mcu_timer_init(MCU_INT_LVL lvl, uint32_t frq_hz, void (*f)(void*), void* obj, bool auto_start)
{
	mcu_timer_t handle;

	if(mcu_current_timer_handler >= MCU_PERIPHERY_DEVICE_COUNT_TIMER)
		MCU_RETURN_ERROR(MCU_ERROR_TMR_NOT_AVAILABLE, NULL);

	handle = &mcu_timer_handler[mcu_current_timer_handler];	

    const esp_timer_create_args_t periodic_timer_args = 
    {
        .name = "MCU_TIMER",
        .callback = f,
		.arg = obj
    };

    esp_timer_create(&periodic_timer_args, &handle->handle);

	if(MCU_OK != mcu_timer_set_frq(handle, frq_hz))
		return NULL;

	if(auto_start)	mcu_timer_start(handle);
	
	mcu_current_timer_handler++;

	return (mcu_timer_t)handle;
}

mcu_timer_t mcu_timer_create(const mcu_timer_config_t* config)
{
	mcu_timer_t handle;

	if(mcu_current_timer_handler >= MCU_PERIPHERY_DEVICE_COUNT_TIMER)
		MCU_RETURN_ERROR(MCU_ERROR_TMR_NOT_AVAILABLE, NULL);

	handle = &mcu_timer_handler[mcu_current_timer_handler];	

    const esp_timer_create_args_t periodic_timer_args = 
    {
        .name = "MCU_TIMER",
        .callback = config->f,
		.arg = config->obj
    };

    esp_timer_create(&periodic_timer_args, &handle->handle);

	if(MCU_OK != mcu_timer_set_frq(handle, config->frq_hz))
		return NULL;

	if(config->auto_start)	
		mcu_timer_start(handle);
	
	mcu_current_timer_handler++;

	return (mcu_timer_t)handle;
}

uint64_t mcu_timer_get_microseconds(void)
{
	return esp_timer_get_time();
}

void mcu_timer_start(mcu_timer_t h)
{	
	esp_timer_start_periodic(h->handle, 1000000 / h->frq);  // us from Hz
}

void mcu_timer_stop(mcu_timer_t h)
{	
	esp_timer_stop(h->handle);	
}

MCU_RESULT mcu_timer_set_frq(mcu_timer_t h, uint32_t frq_hz)
{
	if(h==NULL)
		return MCU_ERROR_TMR_NOT_AVAILABLE;

	if(h->frq == frq_hz)
	{
		return MCU_OK;
	}

	if(frq_hz == 0)
	{
		return MCU_ERROR_TMR_FRQ_INVALID;
	}

	h->frq = frq_hz;
	if(esp_timer_is_active(h->handle))
	{
		// Restart timer with new period
		mcu_timer_stop(h);
		mcu_timer_start(h);
	}

	return MCU_OK;
}

uint32_t mcu_timer_get_frq(mcu_timer_t h)
{
	if(h==NULL)
		return 0;
	else
		return h->frq;
}

#endif

#endif // #if MCU_TYPE == R5F563
