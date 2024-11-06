/**
 * @file module_config.h
 * @author Tim Koczwara (tim.koczwara@esope.de)
 * @brief Configurations for all modules
 * @version 0.1
 * @date 19.01.2022
 * 
 * @copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * 
 */

#ifndef __MODULE_CONFIG_H_GUARD__
#define __MODULE_CONFIG_H_GUARD__

#include "module_enable.h"

/// True:
/// When using MODULE_ARRAY_DECLARE,  MODULE_ARRAY_CREATE and MODULE_ARRAY_DESTROY arrays are used to see the full size in your RAM configuration.
/// False:
/// When using MODULE_ARRAY_DECLARE,  MODULE_ARRAY_CREATE and MODULE_ARRAY_DESTROY arrays are created using the heap and freed using the heap functions.
#define MODULE_PREFER_HEAP							1

#if MODULE_ENABLE_COMM
//------------------------------------
// comm
//------------------------------------
/// Maximum number of bytes that can be used in format length.
/// For example if %80d is needed this define must be at least 80. Keep it small if you do not need big lines.
/// Every comm_t object contains a buffer with the size of COMM_MAX_FORMAT_LENGTH. So be careful.
#define COMM_MAX_FORMAT_LENGTH						600
/// If set to true, printf calls like %20s will cut off strings if they are longer than 20 bytes, so that only the
/// First 20 Bytes will be printed. If it is set to false, strings longer than 20 bytes will still be printed completely-
#define COMM_STRING_LENGTH_EXACT					true

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
#define DEBUG_LEVEL					                DEBUG_LEVEL_VERBOSE
/// Defines how many Letters of the Milliseconds are shown inside dbg_printf.
#define DBG_SYS_MS_COUNT_LETTERS			        9
/// If set to true, only the filename of the debug string is shown.
#define _DBG_STRING_HIDE_PATH				        false
/// Can be set to append the debug string with spaces when it is shorter to assure the debug content all have the same start.
#define _DBG_STRING_MIN_LEN					        40
#endif

#if MODULE_ENABLE_COMM_LINE_READER
//------------------------------------
// comm/line_reader
//------------------------------------
/// Enable to use an internal task for line reader or to false if line_reader should be used as a sub protothread.
#define LINE_READER_USE_TASK		                false
#endif

#if MODULE_ENABLE_CONSOLE
//------------------------------------
// console
//------------------------------------
/// Size of the UART Receive Buffer
#define CONSOLE_UART_RECEIVE_BUFFER_SIZE			10000
/// Size of the Line Buffer that buffers received commands
#define CONSOLE_LINE_BUFFER_SIZE					1536
/// Maximum gap between received characters inside a command.
#define CONSOLE_DROP_LINE_MS						1000
/// Maximum number of arguments for a single command.
#define CONSOLE_MAX_ARGUMENTS						40
/// Enables checking of crc for the console, the last 4 bytes are seen as a 4-character hex string containing the crc.
#define CONSOLE_ENABLE_CRC							true
#endif

#if MODULE_ENABLE_DEBUG_CONSOLE
//------------------------------------
// console/dbg
//------------------------------------
/// Size of a buffer shared by the debug console applications 
#define DEBUG_BUFFER_SIZE				            256
/// Enable / disable flag API
#define DEBUG_CONSOLE_ENABLE_FLAG				    true
/// Enable / disable identification API
#define DEBUG_CONSOLE_ENABLE_IDENTIFICATION			false
/// Enable / disable console for I/O handling
#define DEBUG_CONSOLE_ENABLE_IO					    true
/// Enable / disable PCB testing API
#define DEBUG_CONSOLE_ENABLE_TEST				    true
/// Enable / disable console for UART handling
#define DEBUG_CONSOLE_ENABLE_UART				    true
/// Password that has to be sent with test start for it to unlock testing.
#define DEBUG_CONSOLE_TEST_PASSWORD				    "YourPasswordHere"
/// Enable / disable console for esp specific commands
#define DEBUG_CONSOLE_ENABLE_ESP					true
#endif

#if MODULE_ENABLE_CONVERT_MATH
//------------------------------------
// convert/math
//------------------------------------
/// Set to true if 64-Bit operations are needed, otherwise set to false to deactivate the functions.
#define MATH_ENABLE_64BIT_OPERATIONS			    true
#endif
#if MODULE_ENABLE_CONVERT_STRING
//------------------------------------
// convert/string
//------------------------------------
/// Defines if only the string functions that are used inside the comm module are built.
///     true: Only the functions for the comm module are built.
///     false: All functions will be built.
#define STRING_USE_COMM_MINIMUM                     false
/// Defines if the string_printf function will be included in the built.
///     true: string_printf function can be used.
///     false: string_printf function cannot be used.
#define STRING_BUILD_PRINTF                         true
#endif

#if MODULE_ENABLE_CRC
//------------------------------------
// crc
//------------------------------------
/// If set, a constant lookup table will be used for 0x1021 when initialized with crc_init_handler.
#define CRC_USE_TABLE_X16_X12_X5_1					false		
/// If set, a constant lookup table will be used for 0x8005 when initialized with crc_init_handler.
#define CRC_USE_TABLE_X16_X15_X2_1					true		
/// If set, a constant lookup table will be used for 0x8408 when initialized with crc_init_handler_reversed.
#define CRC_USE_TABLE_X16_X12_X5_1_REVERSED			false		
#endif

#if MODULE_ENABLE_FIFO
//------------------------------------
// fifo
//------------------------------------
/// fifo_get_median functions are only available if define is set to true. If you do not need them, set the define to false.
#define FIFO_USE_MEDIAN					            false
/// fifo_get_average functions are only available if define is set to true. If you do not need them, set the define to false.
#define FIFO_USE_AVERAGE				            false
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
#define EVE_GENERATION								3
/// Shows the SPI speed on the debug interface if set to true.
#define EVE_DEBUG_SPI_SPEED						true		
/// Shows the eror messages on the debug interface if set to true
#define EVE_DEBUG_ERROR							true		
/// Maximum SPI speed during power-on (0 - 11 MHz).
#define EVE_SPI_SPEED_INIT						1000000		
/// Maximum SPI speed for normal use (0 - 30 MHz).
#define EVE_SPI_SPEED								30000000		
/// Maximum number of tags used simultaneously on the visible screen.
#define EVE_COPRO_MAX_TAGS					20		
/// Minimum number of time between touch and release
#define EVE_COPRO_TOUCH_MIN_MS				50		
/// Number of bytes written in command buffer is shown if this is true.
#define EVE_COPRO_DEBUG_COMMAND_COUNT			false 	
/// Shows the number of bytes in the dllist after swapping a screen
#define EVE_COPRO_DEBUG_DL_LIST				false	
/// If set to true the calibration command becomes available.
/// The calibration command should only be used for new displays and not for recalibration inside a product!
#define EVE_COPRO_USE_CALIBRATION				false	
/// Enables function eve_copro_snapshot.
#define EVE_COPRO_ENABLE_SNAPSHOT				false	
/// Buffer size for eve_copro_snapshot. Only valid when EVE_COPRO_ENABLE_SNAPSHOT is set.
#define EVE_COPRO_SNAPSHOT_BUFFER_SIZE		500		
/// Maximum number of memory files in eve
#define EVE_MEMORY_FILES_MAX						10		
/// Buffer size to read files into eve ram.
#define EVE_MMC_READ_BUFFER_SIZE					200		
/// If enabled, the DL List will be read into the internal RAM before a display swap.
/// 8092 Bytes of RAM are needed to enable this.
#define EVE_COPRO_ENABLE_DUMP				false
/// If enable the eve flash commands can be used via console.
#define EVE_COPRO_ENABLE_FLASH_CONSOLE		true
/// Enable / Disable font number 16
#define EVE_ENABLE_FONT16				false
/// Enable / Disable font number 17
#define EVE_ENABLE_FONT17				false
/// Enable / Disable font number 18
#define EVE_ENABLE_FONT18				false
/// Enable / Disable font number 19
#define EVE_ENABLE_FONT19				false
/// Enable / Disable font number 20
#define EVE_ENABLE_FONT20				false
/// Enable / Disable font number 21
#define EVE_ENABLE_FONT21				false
#endif



#endif // __MODULE_CONFIG_H_GUARD__