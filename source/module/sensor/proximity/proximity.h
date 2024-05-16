/**
 * 	@file proximity.h
 * 	@copyright Urheberrecht 2019-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		Handles the proximity sensor.
 *			
 *  @version	1.00 (12.12.2019)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_SENSOR_PROXIMITY_H_
#define MODULE_SENSOR_PROXIMITY_H_

#include "module_public.h"
#if MODULE_ENABLE_SENSOR_PROXIMITY
#include "module/enum/function_return.h"
#include "../sensor_common.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Callback for the function to read the measured lux value from a proximity sensor.
 * The parameter is the device handler of the concrete sensor.
 * The return value is the measured proximity value in lux.
 */
typedef proximity_t (*proximity_get_value_t)(sensor_device_handle_t);

/**
 * Interface for the proximity sensor drivers.
 */
typedef struct
{
	/// Callback for the function to read the measured lux value from a proximity sensor.
	proximity_get_value_t get_value;
	/// Callback for freeing the ressources of the proximity sensor device. 
	sensor_free_t free;
	/// Callback to retrieve the state of a sensor.
	sensor_get_state_t get_state;
}proximity_interface_t;

/**
 * Handle for a single proximity sensor.
 */
typedef struct proximity_s* proximity_handle_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * Allocates the proximity sensor and returns a handle for it.
 * @note Use proximity_free if sensor is not used anymore. 
 * 
 * @param device		Device handle that was returned when the proximity sensor driver was initialized.
 * @param interface		Pointer to the interface for the proximity sensor driver.
 * @return 				Handle of the proximity sensor
 **/
proximity_handle_t proximity_init(sensor_device_handle_t device, const proximity_interface_t* interface);
/**
 * @brief Frees the ressources for a proximity sensor. If you need it again, you need to call proximity_init again to get a new handle.
 * 
 * @param l				Handle of the proximity sensor as created in proximity_init.
 * @param free_device	If true, the device handle will also be freed.
 */
void proximity_free(proximity_handle_t l, bool free_device);

/**
 * Returns the current proximity value.
 * @param l			Handle of the proximity sensor as created in proximity_init.
 * @return			Value of the proximity sensor in lux. If initialization failed the value is always PROXIMITY_SENSOR_INVALID.
 */
proximity_t proximity_get_value(proximity_handle_t l);

/**
 * @brief Retrieve the state of a sensor.
 * 
 * @param l			Handle of the proximity sensor as created in proximity_init.
 * @return 			State of the sensor. 
 */
SENSOR_STATE_T proximity_get_state(proximity_handle_t l);

#endif

#endif /* MODULE_SENSOR_PROXIMITY_H_ */
