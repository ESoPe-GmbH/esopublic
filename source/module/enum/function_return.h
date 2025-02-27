// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file function_return.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief	Defines an enumeration that can be used as generic return values for functions.
 *			
 *  @version	1.06 (13.07.2022)
 * 		- Always active, without module enable for enum.
 *	@version	1.05 (07.05.2022)
 *		- Added FUNCTION_RETURN_NO_RESPONSE
 *		- Added FUNCTION_RETURN_WRONG_DEVICE
 *		- Added FUNCTION_RETURN_EXECUTION_ERROR
 *	@version	1.04 (19.01.2022)
 * 	    - Modified to be used in esopekernel
 *  @version    1.03 (30.04.2019)
 *      - Added FUNCTION_RETURN_NOT_FOUND
 *	@version	1.02 (30.01.2019)
 *		- Added FUNCTION_RETURN_UNSUPPORTED
 *	@version	1.01 (07.06.2018)
 *		- Added module.h support
 *  @version	1.00 (12.06.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef FUNCTION_RETURN_H_
#define FUNCTION_RETURN_H_

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the function return module
#define FUNCTION_RETURN_STR_VERSION "1.06"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Default return value for functions. Indicates if the function can be executed or not.
 */
typedef enum FUNCTION_RETURN_E
{
	/// Function was executed
	FUNCTION_RETURN_OK = 0,
	/// Function could not be executed because of an parameter error.
	FUNCTION_RETURN_PARAM_ERROR,
	/// Function could not be executed because the state of the module is incorrect.
	FUNCTION_RETURN_NOT_READY,
	/// Function could not be executed because the function is not implemented.
	/// E.g. abstraction layer offers functionality, but underlying layer does not offer it.
	FUNCTION_RETURN_UNSUPPORTED,
	/// Function could not be executed because a searched value could not be found.
	FUNCTION_RETURN_NOT_FOUND,
	/// Function could not be executed because there is not enough memory (e.g. buffer in parameter has not enough space)
	FUNCTION_RETURN_INSUFFICIENT_MEMORY,
	/// Function could not be executed because the current security level is not authorize to access the given resource
	FUNCTION_RETURN_UNAUTHORIZED,
	/// Function tried to communicate with a device, but it got no response from the device.
	FUNCTION_RETURN_NO_RESPONSE,
	/// Function tried to communicate with a device, but the device was not what we expected.
	FUNCTION_RETURN_WRONG_DEVICE,
	/// Function tried to do stuff but something stopped working during it, so we had to stop.
	FUNCTION_RETURN_EXECUTION_ERROR,
	/// Function tried to write something but the writing was not possible (e.g. writing to flash).
	FUNCTION_RETURN_WRITE_ERROR,
	/// Function tried to read something, but reading was not possible (e.g. reading from flash).
	FUNCTION_RETURN_READ_ERROR,
	/// Function tried to execute something with an external device, but there was an error of the other device, which needs to be recovered.
	FUNCTION_RETURN_DEVICE_ERROR,
	/// Function tried to execute something with an external device, but the other device was restarted.
	FUNCTION_RETURN_DEVICE_RESET,
	/// Function timed out.
	FUNCTION_RETURN_TIMEOUT,
	/// Is returned when function checked the integrity of data and this check failed. For example if a checksum is checked.
	FUNCTION_RETURN_INTEGRITYCHECK_FAILED,
	/// Is returned when a value is invalid. For example if a value read from a remote device is outside of the possible range.
	FUNCTION_RETURN_INVALID_VALUE,	
	/// Is returned when an authenticity check failed. For example if a signature is broken.
	FUNCTION_RETURN_AUTHENTICITY_FAILED,

}FUNCTION_RETURN_T;

/// Definition for legacy compatibility @see FUNCTION_RETURN_T.
/// @deprecated Use FUNCTION_RETURN_T instead.
#define FUNCTION_RETURN		FUNCTION_RETURN_T

#endif /* FUNCTION_RETURN_H_ */
