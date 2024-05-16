/**
 * 	@file 		mcu_timer.c
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_ESP32

#include "../esp32/mcu_internal.h"
#include <string.h>
#include <stdlib.h>

#include "driver/timer.h"
#include "esp_timer.h"

#if MCU_PERIPHERY_DEVICE_COUNT_TIMER > 0

#define MCU_TIMER_INIT_HANDLER(h, n, mtu)										\
				{																\
						h->num = n;												\
						h->is_mtu = mtu;										\
						h->callback = f;										\
						h->obj = obj;											\
						h->frq = 0;												\
						mcu_timer_handler_hash[n] = h;							\
				}
				
struct mcu_timer_s *mcu_timer_handler_hash[MCU_TIMER_TOTAL_COUNT] = {0};
uint8_t mcu_current_timer_handler = 0;
struct mcu_timer_s mcu_timer_handler[MCU_PERIPHERY_DEVICE_COUNT_TIMER];

mcu_timer_t mcu_timer_init(MCU_INT_LVL lvl, uint32_t frq_hz, void (*f)(void*), void* obj, bool auto_start)
{
	mcu_timer_t handle;

	if(mcu_current_timer_handler >= MCU_PERIPHERY_DEVICE_COUNT_TIMER)
		MCU_RETURN_ERROR(MCU_ERROR_TMR_NOT_AVAILABLE, NULL);

	handle = &mcu_timer_handler[mcu_current_timer_handler];	

	// TODO: Implement

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

	// TODO: Implement

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

static void mcu_timer_set_start(mcu_timer_t h, uint8_t state)
{
	if(h==NULL)	
		return;

	// TODO: Implement
}

void mcu_timer_start(mcu_timer_t h)
{	
	mcu_timer_set_start(h, 1);	
}

void mcu_timer_stop(mcu_timer_t h)
{	
	mcu_timer_set_start(h, 0);	
}

MCU_RESULT mcu_timer_set_frq(mcu_timer_t h, uint32_t frq_hz)
{
	if(h==NULL)
		return MCU_ERROR_TMR_NOT_AVAILABLE;

	// TODO: Implement

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
