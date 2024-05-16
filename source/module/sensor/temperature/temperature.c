/**
 * @file temperature.c
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "module_public.h"
#if MODULE_ENABLE_SENSOR_TEMPERATURE

#include "temperature.h"
#include "module/util/assert.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct temperature_dev_s
{
	/// Pointer to the device handle of the concrete sensor
	sensor_device_handle_t device;
	/// Interface of the concrete temperature sensor
	const temperature_interface_t* interface;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

const temperature_t temperature_invalid_value = {.unit = TEMPERATURE_UNIT_INVALID, .value = TEMPERATURE_SENSOR_INVALID_VALUE};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

temperature_handle_t temperature_init(sensor_device_handle_t device, const temperature_interface_t* interface)
{
	// Do not allocate sensor if device or interface are invalid
	if(interface == NULL)
		return NULL;

	// Allocate a new temperature sensor
	temperature_handle_t l = mcu_heap_calloc(1, sizeof(struct temperature_dev_s));

	// Allocation failed, return NULL
	if(l == NULL)
		return NULL;

	l->device = device;
	l->interface = interface;

	return l;
}

void temperature_free(temperature_handle_t l)
{
	if(l == NULL)
		return;

	if(l->interface->free)	
	{
		l->interface->free(l->device);
	}

	mcu_heap_free(l);
}

temperature_value_t temperature_get_value_by_unit(temperature_handle_t l, TEMPERATURE_UNIT_T unit)
{
	ASSERT_RET_NOT_NULL(l, NO_ACTION, TEMPERATURE_SENSOR_INVALID_VALUE);

	temperature_t t = temperature_get_value(l);

	return temperature_convert_value(unit, t);
}

temperature_t temperature_get_value(temperature_handle_t l)
{
	// Invalid temperature sensor -> invalid value
	// Assume interface pointer is not NULL, because init would have failed
	if(l == NULL || l->interface == NULL || l->interface->get_value == NULL)
		return temperature_invalid_value;

	return l->interface->get_value(l->device);
}

TEMPERATURE_UNIT_T temperature_get_default_unit(temperature_handle_t t)
{
	// Invalid temperature sensor -> invalid value
	// Assume interface pointer is not NULL, because init would have failed
	if(t == NULL || t->interface == NULL || t->interface->get_default_unit == NULL)
		return TEMPERATURE_UNIT_INVALID;

	return t->interface->get_default_unit(t->device);
}

SENSOR_STATE_T temperature_get_state(temperature_handle_t l)
{
	if(l == NULL || l->interface == NULL || l->interface->get_state == NULL)
		return SENSOR_STATE_DEFECT;

	return l->interface->get_state(l->device);
}

temperature_array_options_t temperature_get_array_options(temperature_handle_t t)
{
    if(t == NULL || t->interface == NULL || t->interface->get_array_options == NULL)
        return (temperature_array_options_t){0};

    return t->interface->get_array_options(t->device);
}

temperature_value_t* temperature_alloc_array(temperature_handle_t t)
{
    if(t == NULL || t->interface == NULL || t->interface->get_array_options == NULL || t->interface->read_array == NULL)
        return NULL;

    temperature_array_options_t o = t->interface->get_array_options(t->device);
    if((o.columns + o.rows) == 0)
        return NULL;
    
    return mcu_heap_calloc(sizeof(temperature_value_t), o.columns * o.rows);
}

void temperature_free_array(temperature_value_t* values)
{
    if(values)
    {
        mcu_heap_free(values);
    }
}

FUNCTION_RETURN_T temperature_read_array(temperature_handle_t t, temperature_value_t* values)
{
    if(t == NULL || t->interface == NULL || t->interface->read_array == NULL)
        return FUNCTION_RETURN_PARAM_ERROR;

    return t->interface->read_array(t->device, values);
}

temperature_value_t temperature_convert_value(TEMPERATURE_UNIT_T desired_unit, temperature_t current)
{
    switch(desired_unit)
    {
        case TEMPERATURE_UNIT_CELSIUS:

            switch(current.unit)
            {
                case TEMPERATURE_UNIT_CELSIUS:
                    // Already Celsius
                    return current.value;

                case TEMPERATURE_UNIT_FAHRENHEIT:
                    // Convert Fahrenheit to Celsius
                    return (current.value - 32.0f) * 5 / 9;

                case TEMPERATURE_UNIT_KELVIN:
                    // Convert Kelvin to Celsius
                    return current.value - 273.15;

                default:
                    break;
            }

            break;

        case TEMPERATURE_UNIT_FAHRENHEIT:

            switch(current.unit)
            {
                case TEMPERATURE_UNIT_CELSIUS:
                    // Convert Celsius to Fahrenheit
                    return (current.value * 9 / 5) + 32.0f;

                case TEMPERATURE_UNIT_FAHRENHEIT:
                    // Already Fahrenheit
                    return current.value;

                case TEMPERATURE_UNIT_KELVIN:
                    // Convert Kelvin to Fahrenheit
                    return ((current.value - 273.15) * 9 / 5) + 32.0f;

                default:
                    break;
            }

            break;

        case TEMPERATURE_UNIT_KELVIN:

            switch(current.unit)
            {
                case TEMPERATURE_UNIT_CELSIUS:
                    // Convert Celsius to Kelvin
                    return current.value + 273.15;

                case TEMPERATURE_UNIT_FAHRENHEIT:
                    // Convert Fahrenheit to Kelvin
                    return ((current.value - 32.0f) * 5 / 9) + 273.15;

                case TEMPERATURE_UNIT_KELVIN:
                    // Already Kelvin
                    return current.value;

                default:
                    break;
            }

            break;

        default:
            break;
    }

    return TEMPERATURE_SENSOR_INVALID_VALUE;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif


