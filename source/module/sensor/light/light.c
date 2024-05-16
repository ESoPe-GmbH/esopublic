/**
 * @file light.c
 * @copyright Urheberrecht 2019-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "module_public.h"
#if MODULE_ENABLE_SENSOR_LIGHT

#include "light.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct light_s
{
	/// Pointer to the device handle of the concrete sensor
	sensor_device_handle_t device;
	/// Interface of the concrete light sensor
	const light_interface_t* interface;
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

light_handle_t light_init(sensor_device_handle_t device, const light_interface_t* interface)
{
	// Do not allocate sensor if device or interface are invalid
	if(device == NULL || interface == NULL)
		return NULL;

	// Allocate a new light sensor
	light_handle_t l = mcu_heap_calloc(1, sizeof(struct light_s));

	// Allocation failed, return NULL
	if(l == NULL)
		return NULL;

	l->device = device;
	l->interface = interface;

	return l;
}

void light_free(light_handle_t l, bool free_device)
{
	if(l == NULL)
		return;

	if(free_device && l->interface->free)	
	{
		l->interface->free(l->device);
	}

	mcu_heap_free(l);
}

lux_t light_get_lux(light_handle_t l)
{
	// Invalid light sensor -> invalid lux value
	// Assume interface pointer is not NULL, because init would have failed
	if(l == NULL || l->interface->get_lux == NULL)
		return LIGHT_SENSOR_LUX_INVALID;

	return l->interface->get_lux(l->device);
}

SENSOR_STATE_T light_get_state(light_handle_t l)
{
	if(l == NULL || l->interface->get_state == NULL)
		return SENSOR_STATE_DEFECT;

	return l->interface->get_state(l->device);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif


