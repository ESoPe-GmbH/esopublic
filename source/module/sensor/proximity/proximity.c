/**
 * @file proximity.c
 * @copyright Urheberrecht 2019-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "module_public.h"
#if MODULE_ENABLE_SENSOR_PROXIMITY

#include "proximity.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct proximity_s
{
	/// Pointer to the device handle of the concrete sensor
	sensor_device_handle_t device;
	/// Interface of the concrete proximity sensor
	const proximity_interface_t* interface;
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

proximity_handle_t proximity_init(sensor_device_handle_t device, const proximity_interface_t* interface)
{
	// Do not allocate sensor if device or interface are invalid
	if(device == NULL || interface == NULL)
		return NULL;

	// Allocate a new proximity sensor
	proximity_handle_t l = mcu_heap_calloc(1, sizeof(struct proximity_s));

	// Allocation failed, return NULL
	if(l == NULL)
		return NULL;

	l->device = device;
	l->interface = interface;

	return l;
}

void proximity_free(proximity_handle_t l, bool free_device)
{
	if(l == NULL)
		return;

	if(free_device && l->interface->free)	
	{
		l->interface->free(l->device);
	}

	mcu_heap_free(l);
}

proximity_t proximity_get_value(proximity_handle_t l)
{
	// Invalid proximity sensor -> invalid value
	// Assume interface pointer is not NULL, because init would have failed
	if(l == NULL || l->interface->get_value == NULL)
		return PROXIMITY_SENSOR_INVALID;

	return l->interface->get_value(l->device);
}

SENSOR_STATE_T proximity_get_state(proximity_handle_t l)
{
	if(l == NULL || l->interface->get_state == NULL)
		return SENSOR_STATE_DEFECT;

	return l->interface->get_state(l->device);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif


