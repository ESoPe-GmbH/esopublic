/**
 * @file module_public.h
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * @author Tim Koczwara
 * 
 * @brief Translates KConfig configuration to internal defines
 * 
 * @version 1.00 (05.12.2024)
 * 	- Initial release
 * 
 * @par References
 * 
 **/
#ifndef __MODULE_MODULE_PUBLIC_KCONFIG_H_
#define __MODULE_MODULE_PUBLIC_KCONFIG_H_

#if CONFIG_ESOPUBLIC_ENABLE

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Modules that are initialized inside board_init -> Modifications of pins are necessary.
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Enables the led module
/// Init function should be in board_init.
#define MODULE_ENABLE_LED								CONFIG_MODULE_ENABLE_LED

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Modules that have no global initialization, but could have an object initialization.
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Enables the comm module. As if there is a case where this should be 0...
#define MODULE_ENABLE_COMM								CONFIG_MODULE_ENABLE_COMM

/// Enables the i2c in the comm module.
#define MODULE_ENABLE_COMM_I2C                          CONFIG_MODULE_ENABLE_COMM_I2C

/// Enables the spi in the comm module.
#define MODULE_ENABLE_COMM_SPI                          CONFIG_MODULE_ENABLE_COMM_SPI

/// Enables the virtual comm interface in the comm module.
#define MODULE_ENABLE_COMM_VCOMM                        CONFIG_MODULE_ENABLE_COMM_VCOMM

/// Enables the line_reader in the comm module.
#define MODULE_ENABLE_COMM_LINE_READER                  CONFIG_MODULE_ENABLE_COMM_LINE_READER

/// Enables the module for the uart tls adapter
#define MODULE_ENABLE_COMM_UART_TLS                     CONFIG_MODULE_ENABLE_COMM_UART_TLS

/// Enables the console module
#define MODULE_ENABLE_CONSOLE							CONFIG_MODULE_ENABLE_CONSOLE

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_BASE64					CONFIG_MODULE_ENABLE_CONVERT_BASE64

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_BCD                       CONFIG_MODULE_ENABLE_CONVERT_BCD

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_MATH						CONFIG_MODULE_ENABLE_CONVERT_MATH

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_SORT						CONFIG_MODULE_ENABLE_CONVERT_SORT

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_STRING					CONFIG_MODULE_ENABLE_CONVERT_STRING

/// Enables the convert module. Is necessary for multiple other modules.
#define MODULE_ENABLE_CONVERT_SWAP						CONFIG_MODULE_ENABLE_CONVERT_SWAP

/// Enables the crc module which is necessary for other modules like flash info.
#define MODULE_ENABLE_CRC								CONFIG_MODULE_ENABLE_CRC

/// Enables the debug console module
#define MODULE_ENABLE_DEBUG_CONSOLE						CONFIG_MODULE_ENABLE_DEBUG_CONSOLE

/// Enables the display module that might be used with slint, lvgl or other graphic frameworks.
#define MODULE_ENABLE_DISPLAY                           CONFIG_MODULE_ENABLE_DISPLAY

/// Enables the EEPROM module for small data storage EEPROMs, like identification data.
#define MODULE_ENABLE_EEPROM                            CONFIG_MODULE_ENABLE_EEPROM

/// Enables the enum module. Leave it on, there is no source in this.
#define MODULE_ENABLE_ENUM								CONFIG_MODULE_ENABLE_ENUM

/// Enables the fifo module. This is also needed in mcu like uart or can.
#define MODULE_ENABLE_FIFO								CONFIG_MODULE_ENABLE_FIFO

/// Enables gui module to enable the api for showing buttons, etc. on screens via EVE, FT810, etc.
/// When enabled, you need to have a gui_config.h in your config directory. A template can be found in the template directory.
#define MODULE_ENABLE_GUI								CONFIG_MODULE_ENABLE_GUI

/// Enable the touch abstraction for touch displays
#define MODULE_ENABLE_LCD_TOUCH                         CONFIG_MODULE_ENABLE_LCD_TOUCH

/// Enable the touch driver for Ilitek ILI2130
#define MODULE_ENABLE_LCD_TOUCH_DRIVER_ILI2130          CONFIG_MODULE_ENABLE_LCD_TOUCH_DRIVER_ILI2130

/// Enable the touch driver for Sitronix ST1633i
#define MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I          CONFIG_MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I

/// Enables the rtc module for calculation function on time.
#define MODULE_ENABLE_RTC                               CONFIG_MODULE_ENABLE_RTC

/// Enables an abstract light sensor so you do not have to use the concrete light sensor implementations.
#define MODULE_ENABLE_SENSOR_LIGHT                      CONFIG_MODULE_ENABLE_SENSOR_LIGHT

/// Enables the magnet sensor interface.
#define MODULE_ENABLE_SENSOR_MAGNET                     CONFIG_MODULE_ENABLE_SENSOR_MAGNET

/// Enables an abstract proximity sensor so you do not have to use the concrete proximity sensor implementations.
#define MODULE_ENABLE_SENSOR_PROXIMITY                  CONFIG_MODULE_ENABLE_SENSOR_PROXIMITY

/// Enables the module for an temperature sensor
#define MODULE_ENABLE_SENSOR_TEMPERATURE     			CONFIG_MODULE_ENABLE_SENSOR_TEMPERATURE

/// Enable the utility functions for the color.
#define MODULE_ENABLE_UTIL_COLOR                        CONFIG_MODULE_ENABLE_UTIL_COLOR

/// Enables the memory pool utility
#define MODULE_ENABLE_UTIL_MEM_POOL                     CONFIG_MODULE_ENABLE_UTIL_MEM_POOL

/// True:
/// When using MODULE_ARRAY_DECLARE,  MODULE_ARRAY_CREATE and MODULE_ARRAY_DESTROY arrays are used to see the full size in your RAM configuration.
/// False:
/// When using MODULE_ARRAY_DECLARE,  MODULE_ARRAY_CREATE and MODULE_ARRAY_DESTROY arrays are created using the heap and freed using the heap functions.
#define MODULE_PREFER_HEAP							CONFIG_MODULE_PREFER_HEAP

#if MODULE_ENABLE_COMM
//------------------------------------
// comm
//------------------------------------
/// Maximum number of bytes that can be used in format length.
/// For example if %80d is needed this define must be at least 80. Keep it small if you do not need big lines.
/// Every comm_t object contains a buffer with the size of COMM_MAX_FORMAT_LENGTH. So be careful.
#define COMM_MAX_FORMAT_LENGTH						CONFIG_COMM_MAX_FORMAT_LENGTH
/// If set to true, printf calls like %20s will cut off strings if they are longer than 20 bytes, so that only the
/// First 20 Bytes will be printed. If it is set to false, strings longer than 20 bytes will still be printed completely-
#define COMM_STRING_LENGTH_EXACT					CONFIG_COMM_STRING_LENGTH_EXACT

//------------------------------------
// comm/dbg
//------------------------------------
/// Is used to print all information
#define DEBUG_LEVEL_VERBOSE			                3
/// Is used to print only infos on info or error level
#define DEBUG_LEVEL_INFO			                2
/// Is used to print only infos on error level
#define DEBUG_LEVEL_ERROR			                1
/// Is used to prevent printing any debug information
#define DEBUG_LEVEL_NONE			                0
/// Setting of the selected debug level.
#define DEBUG_LEVEL					                CONFIG_DBG_LEVEL
/// Defines how many Letters of the Milliseconds are shown inside dbg_printf.
#define DBG_SYS_MS_COUNT_LETTERS			        CONFIG_DBG_SYS_MS_COUNT_LETTERS
/// If set to true, only the filename of the debug string is shown.
#define _DBG_STRING_HIDE_PATH				        CONFIG_DBG_STRING_HIDE_PATH
/// Can be set to append the debug string with spaces when it is shorter to assure the debug content all have the same start.
#define _DBG_STRING_MIN_LEN					        CONFIG_DBG_STRING_MIN_LEN
#endif

#if MODULE_ENABLE_COMM_LINE_READER
//------------------------------------
// comm/line_reader
//------------------------------------
/// Enable to use an internal task for line reader or to false if line_reader should be used as a sub protothread.
#define LINE_READER_USE_TASK		                CONFIG_LINE_READER_USE_TASK
#endif

#if MODULE_ENABLE_CONSOLE
//------------------------------------
// console
//------------------------------------
/// Size of the UART Receive Buffer
#define CONSOLE_UART_RECEIVE_BUFFER_SIZE			CONFIG_CONSOLE_UART_RECEIVE_BUFFER_SIZE
/// Size of the Line Buffer that buffers received commands
#define CONSOLE_LINE_BUFFER_SIZE					CONFIG_CONSOLE_LINE_BUFFER_SIZE
/// Maximum gap between received characters inside a command.
#define CONSOLE_DROP_LINE_MS						CONFIG_CONSOLE_DROP_LINE_MS
/// Maximum number of arguments for a single command.
#define CONSOLE_MAX_ARGUMENTS						CONFIG_CONSOLE_MAX_ARGUMENTS
/// Enables checking of crc for the console, the last 4 bytes are seen as a 4-character hex string containing the crc.
#define CONSOLE_ENABLE_CRC							CONFIG_CONSOLE_ENABLE_CRC
#endif

#if MODULE_ENABLE_DEBUG_CONSOLE
//------------------------------------
// console/dbg
//------------------------------------
/// Size of a buffer shared by the debug console applications 
#define DEBUG_BUFFER_SIZE				            CONFIG_DEBUG_BUFFER_SIZE
/// Enable / disable flag API
#define DEBUG_CONSOLE_ENABLE_FLAG				    CONFIG_DEBUG_CONSOLE_ENABLE_FLAG
/// Enable / disable identification API
#define DEBUG_CONSOLE_ENABLE_IDENTIFICATION			CONFIG_DEBUG_CONSOLE_ENABLE_IDENTIFICATION
/// Enable / disable console for I/O handling
#define DEBUG_CONSOLE_ENABLE_IO					    CONFIG_DEBUG_CONSOLE_ENABLE_IO
/// Enable / disable PCB testing API
#define DEBUG_CONSOLE_ENABLE_TEST				    CONFIG_DEBUG_CONSOLE_ENABLE_TEST
/// Enable / disable console for UART handling
#define DEBUG_CONSOLE_ENABLE_UART				    CONFIG_DEBUG_CONSOLE_ENABLE_UART
/// Password that has to be sent with test start for it to unlock testing.
#define DEBUG_CONSOLE_TEST_PASSWORD				    CONFIG_DEBUG_CONSOLE_TEST_PASSWORD
/// Enable / disable console for esp specific commands
#define DEBUG_CONSOLE_ENABLE_ESP					CONFIG_DEBUG_CONSOLE_ENABLE_ESP
#endif

#if MODULE_ENABLE_DISPLAY
//------------------------------------
// display
//------------------------------------
/// Enable the display driver for Smart line display
#define DISPLAY_ENABLE_SLD							    CONFIG_DISPLAY_ENABLE_SLD
#endif

#if MODULE_ENABLE_CONVERT_MATH
//------------------------------------
// convert/math
//------------------------------------
/// Set to true if 64-Bit operations are needed, otherwise set to false to deactivate the functions.
#define MATH_ENABLE_64BIT_OPERATIONS			    CONFIG_MATH_ENABLE_64BIT_OPERATIONS
#endif
#if MODULE_ENABLE_CONVERT_STRING
//------------------------------------
// convert/string
//------------------------------------
/// Defines if only the string functions that are used inside the comm module are built.
///     true: Only the functions for the comm module are built.
///     false: All functions will be built.
#define STRING_USE_COMM_MINIMUM                     CONFIG_STRING_USE_COMM_MINIMUM
/// Defines if the string_printf function will be included in the built.
///     true: string_printf function can be used.
///     false: string_printf function cannot be used.
#define STRING_BUILD_PRINTF                         CONFIG_STRING_BUILD_PRINTF
#endif

#if MODULE_ENABLE_CRC
//------------------------------------
// crc
//------------------------------------
/// If set, a constant lookup table will be used for 0x1021 when initialized with crc_init_handler.
#define CRC_USE_TABLE_X16_X12_X5_1					CONFIG_CRC_USE_TABLE_X16_X12_X5_1
/// If set, a constant lookup table will be used for 0x8005 when initialized with crc_init_handler.
#define CRC_USE_TABLE_X16_X15_X2_1					CONFIG_CRC_USE_TABLE_X16_X15_X2_1
/// If set, a constant lookup table will be used for 0x8408 when initialized with crc_init_handler_reversed.
#define CRC_USE_TABLE_X16_X12_X5_1_REVERSED			CONFIG_CRC_USE_TABLE_X16_X12_X5_1_REVERSED
#endif

#if MODULE_ENABLE_FIFO
//------------------------------------
// fifo
//------------------------------------
/// fifo_get_median functions are only available if define is set to true. If you do not need them, set the define to false.
#define FIFO_USE_MEDIAN					            CONFIG_FIFO_USE_MEDIAN
/// fifo_get_average functions are only available if define is set to true. If you do not need them, set the define to false.
#define FIFO_USE_AVERAGE				            CONFIG_FIFO_USE_AVERAGE
#endif

#if MODULE_ENABLE_GUI
//------------------------------------
// gui
//------------------------------------
/// Current eve generation that is used
/// 1 = FT800 / FT801
/// 2 = FT810 / FT811 / FT812 / FT813
/// 3 = BT815 / BT816
/// 4 = BT817 / BT818
#define EVE_GENERATION								CONFIG_EVE_GENERATION
/// Shows the SPI speed on the debug interface if set to true.
#define EVE_DEBUG_SPI_SPEED						CONFIG_EVE_DEBUG_SPI_SPEED
/// Shows the eror messages on the debug interface if set to true
#define EVE_DEBUG_ERROR							CONFIG_EVE_DEBUG_ERROR
/// Maximum SPI speed during power-on (0 - 11 MHz).
#define EVE_SPI_SPEED_INIT						CONFIG_EVE_SPI_SPEED_INIT
/// Maximum SPI speed for normal use (0 - 30 MHz).
#define EVE_SPI_SPEED								CONFIG_EVE_SPI_SPEED
/// Maximum number of tags used simultaneously on the visible screen.
#define EVE_COPRO_MAX_TAGS					CONFIG_EVE_COPRO_MAX_TAGS
/// Minimum number of time between touch and release
#define EVE_COPRO_TOUCH_MIN_MS				CONFIG_EVE_COPRO_TOUCH_MIN_MS
/// Number of bytes written in command buffer is shown if this is true.
#define EVE_COPRO_DEBUG_COMMAND_COUNT			CONFIG_EVE_COPRO_DEBUG_COMMAND_COUNT
/// Shows the number of bytes in the dllist after swapping a screen
#define EVE_COPRO_DEBUG_DL_LIST				CONFIG_EVE_COPRO_DEBUG_DL_LIST
/// If set to true the calibration command becomes available.
/// The calibration command should only be used for new displays and not for recalibration inside a product!
#define EVE_COPRO_USE_CALIBRATION				CONFIG_EVE_COPRO_USE_CALIBRATION
/// Enables function eve_copro_snapshot.
#define EVE_COPRO_ENABLE_SNAPSHOT				CONFIG_EVE_COPRO_ENABLE_SNAPSHOT
/// Buffer size for eve_copro_snapshot. Only valid when EVE_COPRO_ENABLE_SNAPSHOT is set.
#define EVE_COPRO_SNAPSHOT_BUFFER_SIZE		CONFIG_EVE_COPRO_SNAPSHOT_BUFFER_SIZE
/// Maximum number of memory files in eve
#define EVE_MEMORY_FILES_MAX						CONFIG_EVE_MEMORY_FILES_MAX
/// Buffer size to read files into eve ram.
#define EVE_MMC_READ_BUFFER_SIZE					CONFIG_EVE_MMC_READ_BUFFER_SIZE
/// If enabled, the DL List will be read into the internal RAM before a display swap.
/// 8092 Bytes of RAM are needed to enable this.
#define EVE_COPRO_ENABLE_DUMP				CONFIG_EVE_COPRO_ENABLE_DUMP
/// If enable the eve flash commands can be used via console.
#define EVE_COPRO_ENABLE_FLASH_CONSOLE		CONFIG_EVE_COPRO_ENABLE_FLASH_CONSOLE
/// Enable / Disable font number 16
#define EVE_ENABLE_FONT16				CONFIG_EVE_ENABLE_FONT16
/// Enable / Disable font number 17
#define EVE_ENABLE_FONT17				CONFIG_EVE_ENABLE_FONT17
/// Enable / Disable font number 18
#define EVE_ENABLE_FONT18				CONFIG_EVE_ENABLE_FONT18
/// Enable / Disable font number 19
#define EVE_ENABLE_FONT19				CONFIG_EVE_ENABLE_FONT19
/// Enable / Disable font number 20
#define EVE_ENABLE_FONT20				CONFIG_EVE_ENABLE_FONT20
/// Enable / Disable font number 21
#define EVE_ENABLE_FONT21				CONFIG_EVE_ENABLE_FONT21
#endif

#if MODULE_ENABLE_RTC
//------------------------------------
// rtc
//------------------------------------
/// Synchronization interval in seconds for the rtc, before a synchronized clock is seen as relative.
#define RTC_SYNCHRONIZE_DURATION    				CONFIG_RTC_SYNCHRONIZE_DURATION
#endif

#endif // CONFIG_ESOPUBLIC_ENABLE

#endif