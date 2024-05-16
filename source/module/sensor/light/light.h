/**
 * 	@file light.h
 *  @copyright Urheberrecht 2019-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		Handles the light sensor.
 *			
 *  @version	1.00 (12.12.2019)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_SENSOR_LIGHT_H_
#define MODULE_SENSOR_LIGHT_H_

#include "module_public.h"
#if MODULE_ENABLE_SENSOR_LIGHT
#include "module/enum/function_return.h"
#include "../sensor_common.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Callback for the function to read the measured lux value from a light sensor.
 * The parameter is the device handler of the concrete sensor.
 * The return value is the measured light value in lux.
 */
typedef lux_t (*light_get_lux_t)(sensor_device_handle_t);

/**
 * Interface for the light sensor drivers.
 */
typedef struct
{
	/// Callback for the function to read the measured lux value from a light sensor.
	light_get_lux_t get_lux;
	/// Callback for freeing the ressources of the light sensor device. 
	sensor_free_t free;
	/// Callback to retrieve the state of a sensor.
	sensor_get_state_t get_state;
}light_interface_t;

/**
 * Handle for a single light sensor.
 */
typedef struct light_s* light_handle_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * Allocates the light sensor and returns a handle for it.
 * @note Use light_free if sensor is not used anymore. 
 * 
 * @param device		Device handle that was returned when the light sensor driver was initialized.
 * @param interface		Pointer to the interface for the light sensor driver.
 * @return 				Handle of the light sensor
 **/
light_handle_t light_init(sensor_device_handle_t device, const light_interface_t* interface);
/**
 * @brief Frees the ressources for a light sensor. If you need it again, you need to call light_init again to get a new handle.
 * 
 * @param l				Handle of the light sensor as created in light_init.
 * @param free_device	If true, the device handle will also be freed.
 */
void light_free(light_handle_t l, bool free_device);

/**
 * Returns the current light value in lux.
 * @param l			Handle of the light sensor as created in light_init.
 * @return			Value of the light sensor in lux. If initialization failed the value is always LIGHT_SENSOR_LUX_INVALID.
 */
lux_t light_get_lux(light_handle_t l);

/**
 * @brief Retrieve the state of a sensor.
 * 
 * @param l			Handle of the light sensor as created in light_init.
 * @return 			State of the sensor. 
 */
SENSOR_STATE_T light_get_state(light_handle_t l);

#endif

#endif /* MODULE_SENSOR_LIGHT_H_ */
