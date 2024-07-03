/**
 * 	@file sensor_common.h
 *  @copyright Urheberrecht 2022-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		Defines types that are relevant for all sensors.
 *			
 *  @version	1.00 (06.05.2022)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_SENSOR_COMMON_H_
#define MODULE_SENSOR_COMMON_H_

#include "module_public.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Value that is returned in @see sensor_get_lux when the lux could not be read from the light sensor.
#define LIGHT_SENSOR_LUX_INVALID				0xFFFFFFFF

/// Value that is returned when a proximity is gained but the sensor failed to read the proximity
#define PROXIMITY_SENSOR_INVALID                0xFFFFFFFF

/// Value that is returned when the temperature cannot be read by the sensor.
#define TEMPERATURE_SENSOR_INVALID_VALUE        -460.0f /* Below absolute zero in fahrenheit*/

typedef enum SENSOR_STATE_E
{
    /// Sensor is initalized and in use
    SENSOR_STATE_ACTIVE = 0,
    /// Sensor is inactive and in sleep mode
    SENSOR_STATE_SLEEPING,
    /// Sensor is defect. The sensor cannot be used.
    SENSOR_STATE_DEFECT
}SENSOR_STATE_T;

/// @brief Defines the units supported by the temperature sensor
typedef enum TEMPERATURE_UNIT_E
{
    /// Value that is returned when the temperature cannot be read by the sensor.
    TEMPERATURE_UNIT_INVALID = 0,
    /// Value will be in Celsius
    TEMPERATURE_UNIT_CELSIUS,
    /// Value will be in kelvin
    TEMPERATURE_UNIT_KELVIN,
    /// Value will be in fahrenheit
    TEMPERATURE_UNIT_FAHRENHEIT    
}TEMPERATURE_UNIT_T;

/// @brief Type for the value for the temperature.
typedef float temperature_value_t;

/// Type for temperature values in °C.
typedef struct temperature_s
{
    /// @brief Unit of the temperature
    TEMPERATURE_UNIT_T unit;
    /// @brief Value of the temperature
    temperature_value_t value;
} temperature_t;

/// Type for light values in lux.
typedef float lux_t;

/// Type for proximity values.
typedef uint32_t proximity_t;

/// @brief Structure for a 3D vector
typedef struct vector_3d_s
{
    /// @brief X-Coordinate
    float x;
    /// @brief Y-Coordinate
    float y;
    /// @brief Z-Coordinate
    float z;
}vector_3d_t;

/// Handle for the sensor device drivers. This is a common type for all sensors, because some sensors have multiple functions.
typedef void* sensor_device_handle_t;

/**
 * @brief Callback for freeing the ressources of the sensor device. 
 * The parameter is the device handler of the concrete sensor.
 */
typedef void (*sensor_free_t)(sensor_device_handle_t);
/**
 * @brief Callback to retrieve the state of a sensor.
 * The parameter is the device handler of the concrete sensor.
 */
typedef SENSOR_STATE_T (*sensor_get_state_t)(sensor_device_handle_t);

#endif /* MODULE_LIGHT_DEFINES_H_ */
