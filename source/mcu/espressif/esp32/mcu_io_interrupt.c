/**
 * 	@file 		mcu_io_interrupt.c
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_ESP32

#include "mcu_internal.h"
#include "driver/gpio.h"

#if MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT > 0

#define ESP_INTR_FLAG_DEFAULT 0

uint8_t mcu_current_io_int_handler = 0;
struct mcu_io_int_s mcu_io_int_handler[MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT];

mcu_io_int_t mcu_io_interrupt_init(uint8_t num, MCU_IO_PIN pin)
{
	mcu_io_int_t handle;

	if(mcu_current_io_int_handler>=MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT)
		MCU_RETURN_ERROR(MCU_ERROR_IO_INT_NOT_AVAILABLE, NULL)

	handle = &mcu_io_int_handler[mcu_current_io_int_handler];

	if(mcu_current_io_int_handler == 0)
	{
		//install gpio isr service
		gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	}

	handle->pin = pin;
	if(pin < GPIO_EXTERNAL)
	{
		// Set pin directly
		mcu_io_set_dir(pin, MCU_IO_DIR_IN);
	}
	else
	{
		FUNCTION_RETURN ret = mcu_io_interrupt_init_expander(pin);

		if(ret != FUNCTION_RETURN_OK)
			return NULL;
	}
	
	mcu_current_io_int_handler++;
	return (mcu_io_int_t)handle;
}

MCU_RESULT mcu_io_interrupt_set_param(mcu_io_int_t h, void* obj, void (*f)(void*), MCU_INT_LVL lvl, MCU_IO_INT_EDGE edge)
{
	if(h == NULL)	
		return MCU_ERROR_IO_INT_INVALID;

	if(lvl > MCU_INT_LVL_HI)			
		return MCU_ERROR_IO_INT_LVL_INVALID;

	if(edge > 2) 			
		return MCU_ERROR_IO_INT_EDGE_INVALID;

	if(h->pin < GPIO_EXTERNAL)
	{
		switch(edge)
		{
			case MCU_IO_INT_EDGE_LOW:	
				gpio_set_intr_type(h->pin, GPIO_INTR_NEGEDGE);	
				break;
			case MCU_IO_INT_EDGE_HIGH:	
				gpio_set_intr_type(h->pin, GPIO_INTR_POSEDGE);	
				break;
			case MCU_IO_INT_EDGE_BOTH:	
				gpio_set_intr_type(h->pin, GPIO_INTR_ANYEDGE);	
				break;
		}

		h->callback_obj = obj;
		h->callback = f;

		//hook isr handler for specific gpio pin
		gpio_isr_handler_add(h->pin, h->callback, h->callback_obj);
	}
	else
	{
		FUNCTION_RETURN ret = mcu_io_interrupt_set_param_expander(h->pin, obj, f, edge);

		if(ret != FUNCTION_RETURN_OK)
			return MCU_ERROR_IO_INT_INVALID;
		else
		{
			if(lvl == MCU_INT_LVL_OFF)
				mcu_io_interrupt_disable_expander(h->pin);
			else
				mcu_io_interrupt_enable_expander(h->pin);
		}
	}

	return MCU_OK;
}

void mcu_io_interrupt_disable(mcu_io_int_t h)
{
	if(h == NULL)	
		return;

	if(h->pin < GPIO_EXTERNAL)
	    gpio_isr_handler_remove(h->pin);
	else
		mcu_io_interrupt_disable_expander(h->pin);
}

void mcu_io_interrupt_enable(mcu_io_int_t h)
{
	if(h == NULL)	
		return;

	if(h->pin < GPIO_EXTERNAL)
	{
		//hook isr handler for specific gpio pin
		gpio_isr_handler_add(h->pin, h->callback, h->callback_obj);
	}
	else
		mcu_io_interrupt_enable_expander(h->pin);
}

MCU_IO_PIN mcu_io_interrupt_get_pin(mcu_io_int_t h)
{
	if(h==NULL)
		return PIN_NONE;

	return h->pin;
}

#endif

#endif // #if MCU_TYPE == MCU_ESP32
