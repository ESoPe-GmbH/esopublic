/**
 * @file module_enable.h
 * @author Tim Koczwara (tim.koczwara@esope.de)
 * @brief Selection of modules to use inside this application.
 * @version 1.0
 * @date 19.01.2022
 * 
 * @copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * 
 */

#ifndef __MODULE_ENABLE_H_GUARD__
#define __MODULE_ENABLE_H_GUARD__

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Modules that are initialized inside board_init -> Modifications of pins are necessary.
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Enables the led module
/// Init function should be in board_init.
#define MODULE_ENABLE_LED								1

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Modules that have no global initialization, but could have an object initialization.
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Enables the comm module. As if there is a case where this should be 0...
#define MODULE_ENABLE_COMM								1

/// Enables the i2c in the comm module.
#define MODULE_ENABLE_COMM_I2C                          1

/// Enables the spi in the comm module.
#define MODULE_ENABLE_COMM_SPI                          1

/// Enables the virtual comm interface in the comm module.
#define MODULE_ENABLE_COMM_VCOMM                        0

/// Enables the line_reader in the comm module.
#define MODULE_ENABLE_COMM_LINE_READER                  0

/// Enables the module for the uart tls adapter
#define MODULE_ENABLE_COMM_UART_TLS                     0

/// Enables the console module
#define MODULE_ENABLE_CONSOLE							1

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_BASE64					1

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_BCD                       1

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_MATH						1

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_SORT						1

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_STRING					1

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_SWAP						1

/// Enables the crc module which is necessary for other modules like flash info.
#define MODULE_ENABLE_CRC								1

/// Enables the debug console module
#define MODULE_ENABLE_DEBUG_CONSOLE						1

/// Enables the EEPROM module for small data storage EEPROMs, like identification data.
#define MODULE_ENABLE_EEPROM                            0

/// Enables the enum module. Leave it on, there is no source in this.
#define MODULE_ENABLE_ENUM								1

/// Enables the fifo module. This is also needed in mcu like uart or can.
#define MODULE_ENABLE_FIFO								1

/// Enables gui module to enable the api for showing buttons, etc. on screens via EVE, FT810, etc.
/// When enabled, you need to have a gui_config.h in your config directory. A template can be found in the template directory.
#define MODULE_ENABLE_GUI								0

/// Enables an abstract light sensor so you do not have to use the concrete light sensor implementations.
#define MODULE_ENABLE_SENSOR_LIGHT                      0

/// Enables the magnet sensor interface.
#define MODULE_ENABLE_SENSOR_MAGNET                     0

/// Enables an abstract proximity sensor so you do not have to use the concrete proximity sensor implementations.
#define MODULE_ENABLE_SENSOR_PROXIMITY                  0

/// Enable the touch abstraction for touch displays
#define MODULE_ENABLE_LCD_TOUCH                         0

/// Enable the touch driver for Ilitek ILI2130
#define MODULE_ENABLE_LCD_TOUCH_DRIVER_ILI2130          0

/// Enable the touch driver for Sitronix ST1633i
#define MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I          0

/// Enables the rtc module for calculation function on time.
#define MODULE_ENABLE_RTC								1

/// Enables the module for an temperature sensor
#define MODULE_ENABLE_SENSOR_TEMPERATURE     			0

/// Enable the display driver for Smart line display
#define DISPLAY_ENABLE_SLD							    0

/// Enable the utility functions for the color.
#define MODULE_ENABLE_UTIL_COLOR                        0

/// Enables the memory pool utility
#define MODULE_ENABLE_UTIL_MEM_POOL                     1

#endif // __MODULE_ENABLE_H_GUARD__