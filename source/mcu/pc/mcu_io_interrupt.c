/**
 * 	@file 		mcu_io_interrupt.c
 *  @copyright Urheberrecht 2018-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../mcu.h"
#include "mcu_internal.h"


#if MCU_TYPE == PC_EMU

#if MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT > 0

mcu_io_int_t mcu_io_interrupt_init(uint8_t num, MCU_IO_PIN pin)
{
	return NULL;
}

MCU_RESULT mcu_io_interrupt_set_param(mcu_io_int_t handle, void* obj, void (*f)(void*), MCU_INT_LVL lvl, MCU_IO_INT_EDGE edge)
{
	return MCU_ERROR_IO_INT_INVALID;
}

void mcu_io_interrupt_disable(mcu_io_int_t handle)
{
	
}

void mcu_io_interrupt_enable(mcu_io_int_t handle)
{
	
}

MCU_IO_PIN mcu_io_interrupt_get_pin(mcu_io_int_t handle)
{
	return PIN_NONE;
}

#endif

#endif
