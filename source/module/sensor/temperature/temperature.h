/**
 * 	@file temperature.h
 *  @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		Handles a single temperature sensor or a temperature sensor that delivers temperature arrays.
 *			
 *  @version	1.01 (29.02.2024)
 * 		- Added temperature array capabilities.
 *  @version	1.00 (12.12.2019)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_SENSOR_TEMPERATURE_H_
#define MODULE_SENSOR_TEMPERATURE_H_

#include "module_public.h"
#if MODULE_ENABLE_SENSOR_TEMPERATURE
#include "module/enum/function_return.h"
#include "../sensor_common.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Options for sensors using a temperature array instead of just one temperature sensor.
 */
typedef struct temperature_array_options_s
{
	/// @brief Number of columns in the array.
	uint16_t columns;
	/// @brief Number of rows in the array
	uint16_t rows;
}temperature_array_options_t;

/**
 * @brief Callback for the function to read the measured value from a temperature sensor in its default unit.
 * @param dev       The parameter is the device handler of the concrete sensor.
 * @return 			Measured temperature value and its unit.
 */
typedef temperature_t (*temperature_get_value_t)(sensor_device_handle_t dev);
/**
 * @brief Callback for the function to retrieve the default unit of a temperature sensor.
 * @param dev       The parameter is the device handler of the concrete sensor.
 * @return			Unit used by default of the sensor
 */
typedef TEMPERATURE_UNIT_T (*temperature_get_default_unit_t)(sensor_device_handle_t dev);
/**
 * @brief Callback for the function to retrieve the options for a temperature array.
 * @param dev		The parameter is the device handler of the concrete sensor.
 * @return			Options for the array.
 */
typedef temperature_array_options_t (*temperature_get_array_options_t)(sensor_device_handle_t dev);
/**
 * @brief Callback for the function to retrieve the array data from the sensor.
 * @param dev		The parameter is the device handler of the concrete sensor.
 * @param values	Array where the read sensor values will be written into. Must have the size of the columns * rows from the array options to fit all values.
 * @return 			FUNCTION_RETURN_OK on success, others on failure.
 */
typedef FUNCTION_RETURN_T (*temperature_read_array_t)(sensor_device_handle_t dev, temperature_value_t* values);

/**
 * Interface for the temperature sensor drivers.
 */
typedef struct
{
	/// Callback for the function to read the measured value from a temperature sensor by its default unit.
	temperature_get_value_t get_value;
    /// Callback for the function to retrieve the default unit of a temperature sensor.
    temperature_get_default_unit_t get_default_unit;
	/// Callback for freeing the resources of the temperature sensor device. 
	sensor_free_t free;
	/// Callback to retrieve the state of a sensor.
	sensor_get_state_t get_state;
	/// Callback for the function to retrieve the options for a temperature array.
	temperature_get_array_options_t get_array_options;
	/// Callback for the function to retrieve the array data from the sensor.
	temperature_read_array_t read_array;
}temperature_interface_t;

/**
 * Handle for a single temperature sensor.
 */
typedef struct temperature_dev_s* temperature_handle_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Variable
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Constant for an invalid temperature value.
extern const temperature_t temperature_invalid_value;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * Allocates the temperature sensor and returns a handle for it.
 * @note Use temperature_free if sensor is not used anymore. 
 * 
 * @param device		Device handle that was returned when the temperature sensor driver was initialized.
 * @param interface		Pointer to the interface for the temperature sensor driver.
 * @return 				Handle of the temperature sensor
 **/
temperature_handle_t temperature_init(sensor_device_handle_t device, const temperature_interface_t* interface);
/**
 * @brief Frees the ressources for a temperature sensor. If you need it again, you need to call temperature_init again to get a new handle.
 * 
 * @param l				Handle of the temperature sensor as created in temperature_init.
  */
void temperature_free(temperature_handle_t l);

/**
 * Returns the current temperature value.
 * @param l			Handle of the temperature sensor as created in temperature_init.
 * @param unit      Unit of the temperature that is prefered.
 * @return			Value of the temperature sensor in lux. If initialization failed the value is always TEMPERATURE_SENSOR_INVALID.
 */
temperature_value_t temperature_get_value_by_unit(temperature_handle_t l, TEMPERATURE_UNIT_T unit);

/**
 * Returns the current temperature value in the sensor's default unit.
 * @param l			Handle of the temperature sensor as created in temperature_init.
 * @return			Value of the temperature sensor in lux. If initialization failed the value is always TEMPERATURE_SENSOR_INVALID.
 */
temperature_t temperature_get_value(temperature_handle_t l);

/**
 * Returns the unit that is used as default by the temperature sensor.
 * 
 * @param t         Handle of the temperature sensor as created in temperature_init.
 * @return          Unit that is used as default by the temperature sensor. 
 */
TEMPERATURE_UNIT_T temperature_get_default_unit(temperature_handle_t t);

/**
 * @brief Retrieve the state of a sensor.
 * 
 * @param l			Handle of the temperature sensor as created in temperature_init.
 * @return 			State of the sensor. 
 */
SENSOR_STATE_T temperature_get_state(temperature_handle_t l);
/**
 * @brief Read the options of the temperature array.
 * 
 * @param t 		Handle of the temperature sensor as created in temperature_init.
 * @return			Options for the array.
 */
temperature_array_options_t temperature_get_array_options(temperature_handle_t t);
/**
 * @brief Allocate an array for the temperature sensor that can be used for @c temperature_read_array. Make sure to free it if not used.
 * 
 * @param t 		Handle of the temperature sensor as created in temperature_init.
 * @return 			Pointer to an array that can be used for @c temperature_read_array. Make sure to free it using @c temperature_free_array if not needed anymore.
 */
temperature_value_t* temperature_alloc_array(temperature_handle_t t);
/**
 * @brief Frees the space for an array allocated by @c temperature_alloc_array.
 * 
 * @param values 	Pointer to the allocated array.
 */
void temperature_free_array(temperature_value_t* values);
/**
 * @brief Read the temperature array values of the sensor.
 * 
 * @param t 		Handle of the temperature sensor as created in temperature_init.
 * @param values	Array where the read sensor values will be written into. Must have the size of the columns * rows from the array options to fit all values.
 * @return 			FUNCTION_RETURN_OK on success, others on failure.
 */
FUNCTION_RETURN_T temperature_read_array(temperature_handle_t t, temperature_value_t* values);
/**
 * @brief Convert the temperature value of a unit to a desired unit.
 * 
 * @param desired_unit          Unit that the temperature is needed in.
 * @param current               Temperature structure containing the current unit and value in this unit.
 * @return                      Value in the desired unit or TEMPERATURE_SENSOR_INVALID_VALUE on invalid unit.
 */
temperature_value_t temperature_convert_value(TEMPERATURE_UNIT_T desired_unit, temperature_t current);

#endif

#endif /* MODULE_SENSOR_TEMPERATURE_H_ */
