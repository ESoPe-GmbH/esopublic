// Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_controller.c
 * 	@copyright Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"
#include "module_public.h"

#if MCU_TYPE == MCU_NEW_NAME

#include "mcu_internal.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Last set mcu error by any peripheral
MCU_RESULT mcu_last_error = MCU_OK;

/// @brief Frequency of the external crystal
uint32_t mcu_frq_ext_hz = 0;
/// @brief Frequency of the CPU
uint32_t mcu_frq_cpu_hz = 0;
/// @brief Frequency for the peripherals
uint32_t mcu_frq_peripheral_hz = 0;
/// @brief Frequency for the flash
uint32_t mcu_frq_flash_hz = 0;

#if MCU_PERIPHERY_ENABLE_WATCHDOG
/// Task for watchdog
static system_task_t _task = {.name = "Watchdog"};
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_ENABLE_WATCHDOG
/**
 * @brief Handle for a task that retriggers the watchdog timer.
 * 
 * @param obj 	Parameter is unused
 */
static void _watchdog_handle(void* obj);
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
uint32_t mcu_get_frq_external(void){ 			return mcu_frq_ext_hz; }
uint32_t mcu_get_frq_cpu(void){ 				return mcu_frq_cpu_hz; }
uint32_t mcu_get_frq_peripheral(void){ 			return mcu_frq_peripheral_hz; }

void mcu_init_max_internal()
{
	// TODO: Set the maximum frequency when no external crystal is used
	mcu_init(0, 64000000, 32000000);
}

void mcu_init_max_external(uint32_t frq_ext)
{
	// TODO: Set the maximum frequency when an external crystal is used
	mcu_init(frq_ext, 72000000UL, 36000000UL);	
}

void mcu_init(uint32_t frq_ext, uint32_t frq_cpu, uint32_t frq_peripheral)
{
	// TODO: Initialize the clock registers to achieve the frequencies of the parameters.

	// TODO: Initialize I/Os

#if MCU_PERIPHERY_ENABLE_WATCHDOG
	system_task_init_handle(&_task, true, _watchdog_handle, NULL);	
#endif
}

void mcu_enable_interrupt(void)
{
	// TODO: Implement
}

void mcu_disable_interrupt(void)
{
	// TODO: Implement
}

void mcu_soft_reset(void)
{
	// TODO: Implement
}

uint32_t mcu_enter_sleep_mode(uint32_t sleep_time)
{
	// TODO: Implement
	return 0;
}

void mcu_io_reset(MCU_IO_PIN p)
{
	// TODO: Implement
}

void mcu_io_set_port_dir(MCU_IO_PIN p, uint8_t d)
{
	// TODO: Implement
}

void mcu_io_set_dir(MCU_IO_PIN p, MCU_IO_DIRECTION d)
{
	if(p == PIN_NONE)
	{
		return;
	}

	if(p >= GPIO_EXTERNAL)
	{
		mcu_io_set_dir_expander(p, d);
	}
	else
	{
		// TODO: Implement
	}
}

void mcu_io_set_pullup(MCU_IO_PIN p, bool pullup_active)
{
	if(p == PIN_NONE)
	{
		return;
	}
	if(p >= GPIO_EXTERNAL)
	{
		mcu_io_set_pullup_expander(p, pullup_active);
	}
	else
	{
		// TODO: Implement
	}
}

void mcu_io_set(MCU_IO_PIN p, uint8_t d)
{
	if(p == PIN_NONE)
	{
		return;
	}
	if(p >= GPIO_EXTERNAL)
	{
		mcu_io_set_expander(p, d);
	}
	else
	{
		// TODO: Implement
	}
}

void mcu_io_toggle(MCU_IO_PIN p)
{
	if(p == PIN_NONE)
	{
		return;
	}
	if(p >= GPIO_EXTERNAL)
	{
		mcu_io_toggle_expander(p);
	}
	else
	{
		// TODO: Implement
	}
}

uint8_t mcu_io_get(MCU_IO_PIN p)
{
	if(p == PIN_NONE)
	{
		return 0;
	}
	if(p >= GPIO_EXTERNAL)
	{
		return mcu_io_get_expander(p);
	}
	else
	{
		// TODO: Implement
		return 0;
	}
}

#if MCU_PERIPHERY_ENABLE_WATCHDOG
void mcu_watchdog_init(void (*f)(void))
{	
	// TODO: Implement watchdog
	
	system_task_add(&_task);
	mcu_watchdog_trigger();	// Start with first Trigger
}

void mcu_watchdog_trigger(void)
{
	// TODO: Retrigger watchdog
}
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_ENABLE_WATCHDOG
static void _watchdog_handle(void* obj)
{
	mcu_watchdog_trigger();
}
#endif

#endif // #if MCU_TYPE == MCU_NEW_NAME
