// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file version.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *		Stores functions to get the version of the application.
 *
 *	@version	1.01 (06.02.2019)
 *		- Added to module with version_set function.
 *  @version	1.00 (20.11.2015)
 *  	- Intial release
 *
 ******************************************************************************/

#ifndef APPLICATION_VERSION_H_
#define APPLICATION_VERSION_H_

#include "mcu/mcu.h"

/**
 * Sets the version of the application.
 * Example: version_set(10000, 1) -> "1.0000"
 * 			version_set(10000, 2) -> "10.000"
 * @param version			The version as 16-Bit number representing a 5 number string.
 * @param num_chars_main	Number of characters in string representation that build the major version number, while the other numbers represent the minor version.
 */
void version_set(uint16_t version, uint8_t num_chars_major);

/**
 * Returns the version of this software.
 *
 * @return		Version of this software as an integer. For example 10000 for version 1.0000
 */
uint16_t version_get_numeric(void);

/**
 * Returns the version string of this software.
 *
 * @return		String of the version of this software as an integer. For 1.0000
 */
char* version_get_string(void);


#endif /* APPLICATION_VERSION_H_ */
