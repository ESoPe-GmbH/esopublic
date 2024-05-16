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
/// Defines if the mmc can be used for log. If set to true dbg_init_logfile must be called.
#define DBG_USE_MMC_LOG							    false
/// If enabled, debugging can be enabled via TCP, when dbg_init_tcp is called.
#define DBG_USE_TCP								    true
/// If enabled, a console will be added to the tcp debug interface when DBG_USE_TCP is enabled.
#define DBG_USE_TCP_CONSOLE						    true
#if DBG_USE_MMC_LOG
	/// Name of the directory on the sd card where the log files of this module will be stored.
	#define DBG_LOG_DIRECTORY						"DevLog"
	/// Defines whether the logfile is the only debug target.
	/// 	true: 	Debug prints will be only written to the logfile.
	///		false: 	Debug prints will be written into the logfile and to the comm handler that was set with
	///				dbg_set_comm before dbg_init_logfile was called or when dbg_set_comm is called after dbg_init_logfile.
	#define DBG_LOG_IS_ADDITION						true
	/// Maximum inactivity time after which the logfile is closed.
	#define DBG_LOG_CLOSE_INTERVAL_MS				200
	/// Maximum interval before closing a file. The file is closed regardless of the activity.
	#define DBG_LOG_CLOSE_MAX_INTERVAL_MS			2000
#endif
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
/// Enable / disable console for I/O handling
#define DEBUG_CONSOLE_ENABLE_IO					    true
/// Enable / disable socket API
#define DEBUG_CONSOLE_ENABLE_SOCKET				    true
/// Enable / disable PCB testing API
#define DEBUG_CONSOLE_ENABLE_TEST				    true
/// Enable / disable console for UART handling
#define DEBUG_CONSOLE_ENABLE_UART				    true
/// Enable / disable wifi API
#define DEBUG_CONSOLE_ENABLE_WIFI				    true
/// Password that has to be sent with test start for it to unlock testing.
#define DEBUG_CONSOLE_TEST_PASSWORD				    "KameSennin@ESoPe-1337"
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

#if MODULE_ENABLE_DISPLAY
//------------------------------------
// display
//------------------------------------
/// Enable the display driver for KD021WVFPD003
#define DISPLAY_ENABLE_KD021WVFPD003				true
#endif

#if MODULE_ENABLE_ESO_FILE
//------------------------------------
// eso_file
//------------------------------------
/// If set, the eso file reader will work from mmc.
#define ESO_FILE_ENABLE_MMC							true
/// If set, the rso file reader will work from an ongoing input source.
#define ESO_FILE_ON_THE_FLY							true
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

#if MODULE_ENABLE_FLASH
//------------------------------------
// flash
//------------------------------------
/// Timeout for writing/erasing the flash in ms.
#define FLASH_DATA_TIMEOUT_MS				        100
#endif

#if MODULE_ENABLE_FLASH_INFO
//------------------------------------
// flash_info
//------------------------------------
/// Enables/Disables debug prints
#define FLASH_INFO_DEBUG						    false					
/// Number of tries for saving and verifying data before flash_info_save returns false.
#define FLASH_SAVE_TRIES						    3								
/// Size of the flash info data for the device. Contains hardware id and testing date.
#define FLASH_DATA_SIZE							    32						
/// Defines if custom data is used. If it is false all custom data will be lost when flash_data_save is triggered.
#define FLASH_USE_CUSTOM_DATA					    false					
#if FLASH_USE_CUSTOM_DATA
/// Size of the flash info data for the user. Should not be changed after the first use inside a project, because old data will not be read
/// if this size changes. On some microcontrollers it needs to be a multiple of some value (e.g. 8 byte for R32C)
#define FLASH_CUSTOM_DATA_SIZE					    480						
/// If true: 	Custom data is read at the beginning without checking the CRC or content.
/// 			This mode is used in the pcb test software so, that the custom data is not changed
/// 			When a new hardware id or test date is saved inside the flash.
/// If false:	Custom data is read at the beginning with checking the CRC and its content.
#define FLASH_CUSTOM_DATA_READ_ONLY				    false					
#else
/// Custom Data size is 0 for calculation.
#define FLASH_CUSTOM_DATA_SIZE				        0						
#endif
#endif

#if MODULE_ENABLE_FLASHER
//------------------------------------
// flasher
//------------------------------------
/// Set to true if RL78 (Renesas) can be used as a target
#define FLASHER_ENABLE_RL78			                false
/// Set to true if RX (RX63N / RX231 / RX210 / RX111 / RX62N) (Renesas) can be used as a target
#define FLASHER_ENABLE_RX			                false
/// Set to true if RX71M (Renesas) can be used as a target
#define FLASHER_ENABLE_RX7			                false
/// Set to true if R32C (Renesas) can be used as a target
#define FLASHER_ENABLE_R32C			                false
/// Set to true if Synergy (Renesas) can be used as a target
#define FLASHER_ENABLE_SYNERGY		                true
/// Set to true if STM32 (ST Microelectronics) can be used as a target
#define FLASHER_ENABLE_STM32		                false
/// Set to true if ESP32 (Espressif) can be used as a target
#define FLASHER_ENABLE_ESP32		                false
/// Set to true if EFM32 (Silicon Labs) can be used as a target
#define FLASHER_ENABLE_EFM32		                false
/// Set to true when the r32c flasher should use an internal uart buffer or false if an external buffer is used
#define FLASHER_USE_INTERNAL_UART_BUFFER_R32C		false
/// Set to true when the rx flasher should use an internal uart buffer or false if an external buffer is used
#define FLASHER_USE_INTERNAL_UART_BUFFER_RX			false
/// Size for the buffer inside the flasher used to read data from a file and send it to the device.
#define FLASHER_BUFFER_SIZE			                1050
#endif

#if MODULE_ENABLE_GUI
//------------------------------------
// gui
//------------------------------------
/// True: FT810 register are used, false: EVE register are used
#define EVE_USE_FT81X								true
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
#endif

#if MODULE_ENABLE_IO_EXPANDER_SX1509
//------------------------------------
// io_expander
//------------------------------------
/// Set this to true to enable the PWM LED driver functionality of the SX1509.
#define SX1509_ENABLE_LED_DRIVER                    true
/// When enabled, the register of the SX1509 are stored locally in RAM, so when writing a register bit, the local register are used to see the "is" status.
/// When disabled, the registers are read before writing to get the "is" status.
#define SX1509_ENABLE_SHADOW_REGISTER               true
/// Enables/disables the interrupt functionality of the sx1509.
#define SX1509_ENABLE_INTERRUPT						true
#endif

#if MODULE_ENABLE_MMC
//------------------------------------
// mmc
//------------------------------------
/// Maximum number of partitions to handle.
#define DISKIO_PARTITION_MAX                        2
/// Number of physical drives
#define MMC_MAXIMUM_PHY_DRIVES		                2		
/// Maximum number of files that can be opened simultaneously
#define MMC_NUMBER_OF_OPEN_FILES	                6		
/// Enables/Disables debug output for the speed that is set for the spi clock.
#define MMC_DEBUG_SPI_CLOCK			                false	
/// Enables/Disables debug test functions for reading and printing a file.
#define MMC_DEBUG_FILE_READ			                false	
/// Sets the size of an internal buffer used for the putc functions.
/// If set to zero the data will be written to the mmc for every putc function invocation (Slows everything down).
/// Set to 0 if you want to write multiple files at a time! Otherwise the module will not work correctly.
#define MMC_PUTC_BUFFER_SIZE		                256		
/// Maximum filename length for storing filenames.
#define MMC_DEBUG_FILENAME_BUFFER_SIZE	            50	
/// Enable/disable the abstraction for the lvgl filesystem to use the mmc module.
#define MMC_ENABLE_LVGL_FS							0
#endif

#if MODULE_ENABLE_NETWORK
//------------------------------------
// network
//------------------------------------
/// Enable initialization of IP101 phy for ESP32
#define PHY_ENABLE_ESP_IP101						false
/// Enable initialization of DM9162 phy for ESP32
#define PHY_ENABLE_ESP_DM9162						false
/// Enables/disables debug prints for functions like socket_open, etc.
#define SOCKET_DEBUG_FUNCTIONS			            false
/// Enables/disables debug prints for changes in the state and internal state register
#define SOCKET_DEBUG_STATE_CHANGED		            false
/// Maximum number of sockets that can be handled over all network interfaces.
/// Be careful, for each socket, resources are assigned internally.
#define SOCKET_MAX_NUMBER_OF_HANDLES	            10
/// Enables/Disables the mesh network interface of the ESP32.
#define NETWORK_ESP_ENABLE_MESH						false
/// If set to true the network driver will automatically connect to the last connected wifi if using ESP32.
#define NETWORK_ESP_ENABLE_AUTO_WIFI_CONNECT		true
/// If set to true PCB will be reset if ethernet error occurs which would lead to not sending ethernet frames anymore.
#define NETWORK_ESP_ETHERNET_FORCE_RESET_ON_UNRECOVERABLE	true
#endif

#if MODULE_ENABLE_WEBCLIENT
//------------------------------------
// network/protocols/http
//------------------------------------
/// Enable the usage of http basic authentication
#define WEBCLIENT_ENABLE_AUTHENTICATION			    true
/// If set to true, the download progress of a GET is printed.
#define WEBCLIENT_PRINT_PROGRESS				    false
#endif

#if MODULE_ENABLE_WEBSERVER
//------------------------------------
// network/protocols/http
//------------------------------------
/// Size of buffer used for sending a response
#define WEBSERVER_RESPONSE_BUFFER_SIZE		        1024
/// Maximum number of characters in a URL (+1 for terminating zero is added in buffer).
#define WEBSERVER_MAX_URL_LEN				        250
/// Size of the custom buffer in each webserver
#define WEBSERVER_CUSTOM_BUFFER_SIZE		        2000
/// Set to true if websocket can be established via webserver.
#define WEBSERVER_ENABLE_WEBSOCKET			        true
/// Set to true if the files.html can be used for showing the complete content of the mmc and enabling / disabling control of mmc via browser.
#define WEBSERVER_ENABLE_FILE_DOWNLOAD		        false
/// Maximum number of connections allowed to the webserver
#define WEBSERVER_MAX_CONNECTIONS			        4
/// Set to true for webserver debug prints
#define WEBSERVER_ENABLE_DEBUG						false
#endif

#if MODULE_ENABLE_SERIALPROTOCOL_REDEBUS
//------------------------------------
// serialprotocols/redebus
//------------------------------------
/// Defines the mode in which the library will be used.
/// 0 = Control - The application controls all other peripherals, aggregates data from them and distributes it back to the peripherals.
/// 1+ = Peripheral - The application is a peripheral which will be controlled by the control. It only delivers data to the control and gets new instructions from it.
#ifndef REDEBUS_LIB_MODE // Use the ifdef because you could also set it as a compiler flag.
#define REDEBUS_LIB_MODE           0
#endif

/// Defines the medium of layer 1 of the bus protocol.
/// 0 = UDP - The application will use UDP for sending and receiving data that is delivered to layer 2. This is mainly used for local testing on pc.
/// 1 = 485 - The application will use 485 for sending and receiving data. This is the correct mode for most pcbs.
#define REDEBUS_LIB_L1_MODE        0

/// Enable/disable the below debug prints for ebus in general.
#define REDEBUS_DEBUG              0

#if REDEBUS_DEBUG
/// Enable/disable the printing of sent and received layer 3 frames.
#define REDEBUS_DEBUG_L3_FRAMES       0
/// Enable/disable the printing of layer 4 processes like key exchange or resource put/get.
#define REDEBUS_DEBUG_L4_PROCESS      0
#else
/// Enable/disable the printing of sent and received layer 3 frames.
#define REDEBUS_DEBUG_L3_FRAMES       0
/// Enable/disable the printing of layer 4 processes like key exchange or resource put/get.
#define REDEBUS_DEBUG_L4_PROCESS      0
#endif
/// Duration in milliseconds for the initialization phase of the protocol.
#define REDEBUS_CONTROL_INIT_DURATION_MS	5000
/// Interval in milliseconds in which the control loop is triggered
#define REDEBUS_CONTROL_LOOP_INTERVAL_MS	200
/// Duration in milliseconds in which the ntp connector should stay connected, before it is closed.
/// Set to 0 for immediately closing the ntp connector.
#define REDEBUS_CONTROL_NTP_KEEPOPEN_MS		30000
/// Interval in which the time synchronization with the control is triggered again by a peripheral.
#define REDEBUS_PERIPHERAL_TIMESYNC_INTERVAL_MS        1800000
#endif

#if MODULE_ENABLE_RFID
//------------------------------------
// rfid
//------------------------------------
// Interval in which the rfid will be checked for new detected cards
#define RFID_CHECK_INTERVAL_MS		                200
/// Select the ST25R3916 device driver to be used.
#define RFID_ST25R91X_SELECT_ST25R3916				0
/// Select the ST25R3915 device driver to be used.
#define RFID_ST25R91X_SELECT_ST25R3915				1
#endif

#if MODULE_ENABLE_RTC
//------------------------------------
// rtc
//------------------------------------
/// Set to true if an ic is used over i2c connection. Make sure to call rtc_i2c_init if it is used.
#define RTC_ENABLE_RV3028   			            false
/// Synchronization interval in seconds for the rtc, before a synchronized clock is seen as relative.
#define RTC_SYNCHRONIZE_DURATION    				(60 * 60)
#endif

#if MODULE_ENABLE_SECURITY
//------------------------------------
// security
//------------------------------------
/// If set, the mbedtls platform is initialized to enable the use of hash/encryption functions
#define SECURITY_ENABLE_MBEDTLS                     false
/// If set, the mbedtls stack is initialized completely with SSL/TLS
#define SECURITY_ENABLE_MBEDTLS_NETWORK             false
#endif

#if MODULE_ENABLE_SENSOR_LIGHT_TSL2571
//------------------------------------
// sensor/light/tsl2571
//------------------------------------
/// The interval for which the last read values of the light sensor are handles as "current" data.
/// If the light values are accessed when the interval is expired, the values are read from the chip.
#define TSL2571_SENSOR_REFRESH_MS					10
/// Number of measurements in light sensor filter. Set to 0 to disable filter
#define TSL2571_SENSOR_FILTER_BUFFER_SIZE			50
#endif

#if MODULE_ENABLE_SERIALIZATION_BINARY_BUFFER
//------------------------------------
// serialization/binary_buffer
//------------------------------------
/// Align used in the binary buffer that can be used to create compatibility between systems if buffer is shared. 
/// If only used on one system or systems with same MCU, you can set it to 1.
#define BINARY_BUFFER_ALIGN                         1
#endif

#if MODULE_ENABLE_SERIALIZATION_JSON
//------------------------------------
// serialization/json
//------------------------------------
/// Maximum number of objects/arrays that can be put into another.
#define JSON_MAX_NESTING_DEPTH                      5
#endif

#if MODULE_ENABLE_SWUPDATE
//------------------------------------
// swupdate
//------------------------------------
/// Set to the string of an appname that should be checked when updating the firmware. If set to NULL, the app name will not be checked.
#define SW_UPDATE_APPNAME_TO_CHECK					NULL
/// Set to true if the main in sys should automatically trigger an sw update.
/// NOTE: Not relevant for SW_UPDATE_ENABLE_READER == true -> Decryption happens in eso_reader.
#define SW_UPDATE_ESO_SUPPORT_ENCRYPTION	        false
/// Enables / Disables debug prints.
#define SW_UPDATE_MMC_DEBUG             	        false					
/// Current supported version of the .eso file.
/// NOTE: Not relevant for SW_UPDATE_ENABLE_READER == true
#define SW_UPDATE_MMC_HEADER_VERSION	            3							
/// Filename of the .eso file containing the new firmware version.
#define SW_UPDATE_MMC_FILE					        "0:firmware.eso"			
/// Set to true if the main in sys should automatically trigger an sw update.
#define SW_UPDATE_MMC_ON_STARTUP			        false					
/// Set to true if the main in sys should automatically trigger an sw update.
/// NOTE: Not relevant for SW_UPDATE_ENABLE_READER == true -> Decryption happens in eso_reader.
#define SW_UPDATE_MMC_SUPPORT_ENCRYPTION	        false	
/// Enable / Disable debug output			
#define SW_UPDATE_DEBUG                             false			
/// Enable / Disable debug output
#define SW_UPDATE_DEBUG_ERROR                       true			
/// Number of bytes stored inside the internal buffer.
/// Needs to be a multiple of MCU_CONTROLLER_FLASH_MIN_STEPPING inside mcu_controller.h
/// Note: For SW_UPDATE_ENABLE_READER == true, the eso_reader will use up to 3 buffer (input, encryption, output) with this size additionally to the buffer used by sw_update_mmc buffer.
#define SW_UPDATE_BUFFER_SIZE		                2048				
/// Set to true to update from v3 eso, false if v1 or v2
/// NOTE: Set to false when setting SW_UPDATE_ENABLE_READER.
#define SW_UPDATE_ENABLE_V3			                true			
/// Set to true to update using eso_reader. 
/// To use it, you also need MODULE_ENABLE_ESO_FILE to be active 
/// To use sw_update_eso you also need ESO_FILE_ON_THE_FLY to be active. 
/// For sw_update_mms you also need MODULE_ENABLE_MMC and ESO_FILE_ENABLE_MMC to be active.
/// NOTE: Set to false when setting SW_UPDATE_ENABLE_READER.
#define SW_UPDATE_ENABLE_READER		                true					
#endif

#endif // __MODULE_CONFIG_H_GUARD__
