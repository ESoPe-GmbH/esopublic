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
// Enables needed for ressources
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Enables file resources
#define MODULE_ENABLE_RESOURCE_FILE						1

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Modules that are initialized automatically when enabled.
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Enables the flash info module
#define MODULE_ENABLE_FLASH_INFO						0

/// Enables the swupdate Module
/// When enabled, the sw_update_routine can be automatically enabled during start-up. See defines in @see sw_update_mmc.h
#define MODULE_ENABLE_SWUPDATE							0

/// Enables the swupdate Module
/// When enabled, the sw_update_routine can be automatically enabled during start-up. See defines in @see sw_update_mmc.h
#define MODULE_ENABLE_SWUPDATE_ESO                      0

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Modules that are initialized inside board_init -> Modifications of pins are necessary.
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Enables the led module
/// Init function should be in board_init.
#define MODULE_ENABLE_LED								0

/// Enables the I/O Expander modul
/// Init function should be in board_init together with the mcu_register_io_expander.
#define MODULE_ENABLE_IO_EXPANDER                       1

/// Enables the mmc module
/// Init function in board_init available, only pins need to be corrected
#define MODULE_ENABLE_MMC								1

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Modules that have no global initialization, but could have an object initialization.
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Enables the bill dispenser module for the ICT P77
#define MODULE_ENABLE_BILL_ACCEPTOR_ICT_P77				0

/// Enables the bill dispenser module for the ICT ND300CM
#define MODULE_ENABLE_BILL_DISPENSER_ICT_ND300CM		0

/// Enables the bill dispenser module for the JCM F56.
#define MODULE_ENABLE_BILL_DISPENSER_JCM_F56			0

/// Enables the bill recycler module.
#define MODULE_ENABLE_BILL_RECYCLER_INNOVATIVE			0

/// Enables the bill recycler module.
#define MODULE_ENABLE_BILL_RECYCLER_JCM_UBA				0

/// Enables the bluetooth module for the BT900 chip.
#define MODULE_ENABLE_BLUETOOTH_LAIRDTECH_BT900			0

/// Enables the communication via casinonet interface.
#define MODULE_ENABLE_CASINONET							0

/// Enables the communication via cctalk. Is necessary for coin acceptor and hopper.
#define MODULE_ENABLE_CCTALK							0

/// Enables the cctalk coin acceptor NRI v2falcon. Needs MODULE_ENABLE_CCTALK.
#define MODULE_ENABLE_COIN_ACCEPTOR_NRI_V2FALCON		0

/// Enables the parallel coin acceptor WH EMP800
#define MODULE_ENABLE_COIN_ACCEPTOR_WH_EMP800			0

/// Enables the comm module. As if there is a case where this should be 0...
#define MODULE_ENABLE_COMM								1

/// Enables the i2c in the comm module.
#define MODULE_ENABLE_COMM_I2C                          0

/// Enables the spi in the comm module.
#define MODULE_ENABLE_COMM_SPI                          0

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
#define MODULE_ENABLE_DEBUG_CONSOLE						0

/// Enables the display module that might be used with lvgl and stuff.
#define MODULE_ENABLE_DISPLAY                           0

/// Enables the ec module which contains the handling for ec cash terminals.
#define MODULE_ENABLE_EC								0

/// Enables the ecan interface for communication of the ecan protocol over CAN.
#define MODULE_ENABLE_ECAN								0

/// Enables the EEPROM module for small data storage EEPROMs, like identification data.
#define MODULE_ENABLE_EEPROM                            0

/// Enables the enum module. Leave it on, there is no source in this.
#define MODULE_ENABLE_ENUM								1

/// Enables the AES module
#define MODULE_ENABLE_ENCRYPTION_AES					0

/// Enables the module to read eso files. Is needed for sw_update to work.
#define MODULE_ENABLE_ESO_FILE                          1

/// Enables the fifo module. This is also needed in mcu like uart or can.
#define MODULE_ENABLE_FIFO								1

/// Enables file module with helping operations on files, like linewise loading or saving/loading of structures.
#define MODULE_ENABLE_FILE								0

/// Enables filter modules like moving averaging.
#define MODULE_ENABLE_FILTER							0

/// Enables the module to access an external flash via QSPI
#define MODULE_ENABLE_FLASH_QSPI						0

/// Enables the module to access an external flash via SPI
#define MODULE_ENABLE_FLASH_SPI							0

/// Enables the module to access an external flash via ESP partitions
#define MODULE_ENABLE_FLASH_ESP							0

/// Enables the module to access the flash
#define MODULE_ENABLE_FLASH								1

/// Enables the module to emulate a flash device
#define MODULE_ENABLE_FLASH_EMULATION                   1

/// Enables flasher module for flashing software on other mcu via uart.
#define MODULE_ENABLE_FLASHER							0

/// Enabled the rflash submodule of the flasher to enable an esoprog to flash 
/// an externally connected mcu, using this application as a mediator.
#define MODULE_ENABLE_FLASHER_REMOTE					0

/// Enables the webclient module for downloading files via FTP
#define MODULE_ENABLE_FTP_CLIENT                        0

/// Enables the generic meter module to communicate with an electric meter over 485 bus
#define MODULE_ENABLE_GENERIC_METER                     0

/// Enables genius module to communicate with the genius on-board pcb.
#define MODULE_ENABLE_GENIUS							0

/// Enables gps module to communicate with ublox gps chips.
#define MODULE_ENABLE_GPS_UBLOX							0

/// Enables gsm module to communicate with gsm/lte modules.
#define MODULE_ENABLE_GSM								0

/// Enables gui module to enable the api for showing buttons, etc. on screens via EVE, FT810, etc.
/// When enabled, you need to have a gui_config.h in your config directory. A template can be found in the template directory.
#define MODULE_ENABLE_GUI								0

/// Enables the md5 module for hashing.
#define MODULE_ENABLE_HASH_MD5							1

/// Enables sha1 module for hashing.
#define MODULE_ENABLE_HASH_SHA1							1

/// Enables the ripemd160 module for hashing.
#define MODULE_ENABLE_HASH_RIPEMD160					0

/// Enables hopper module to communicate with hopper peripheral that have a cctalk interface.
#define MODULE_ENABLE_HOPPER_CCTALK						0

/// Enables hopper module to communicate with hopper peripheral that have a parallel interface.
#define MODULE_ENABLE_HOPPER_PARALLEL					0

/// Enables image module for bitmap files.
#define MODULE_ENABLE_IMAGE								0

/// Enables the SX1509 I/O Expander module
#define MODULE_ENABLE_IO_EXPANDER_SX1509                0

/// Enables ionwk module to communicate with an IONWK server via ionwk protocol.
#define MODULE_ENABLE_IONWK								0

/// Enables jpcb module to communicate with other device using the jpcb protocol.
#define MODULE_ENABLE_JPCB								0

/// Enables lcd module to show information on lcd 4x20, 2x16, etc.
#define MODULE_ENABLE_LCD								0

/// Enable the rgb led module using ws2812 RGB LEDs.
#define MODULE_ENABLE_LED_WS2812                        0

/// Enables lte module to communicate with lte modules.
#define MODULE_ENABLE_LTE                               0

/// Enables modbus tcp implementation
#define MODULE_ENABLE_MODBUS_TCP                        0

/// Enables the modbus slave module
#define MODULE_ENABLE_MODBUS_RTU_SLAVE                  0

/// Enables mqtt implementation
#define MODULE_ENABLE_MQTT                              0

/// Enable the internal DHCP protocol. Only needed when the used network interfaces do not use DHCP on their own.
#define MODULE_ENABLE_NETWORK_DHCP						0

/// Enable the internal DNS protocol. Only needed when the used network interfaces do not use DNS on their own.
#define MODULE_ENABLE_NETWORK_DNS						0

/// Enables the module for network interfaces
#define MODULE_ENABLE_NETWORK							0

/// Enables the lwip adapter for network interfaces using lwip internally.
#define MODULE_ENABLE_NETWORK_LWIP_ADAPTER				0

/// Enables the Ethernet interface for ST controller
#define MODULE_ENABLE_NETWORK_ETHERNET_ST				0

/// Enables the network interface using ESP32 as SPI interface.
#define MODULE_ENABLE_NETWORK_ESP_SPI					0

/// Enables the network interface using internal network on ESP32
#define MODULE_ENABLE_NETWORK_ESP_INTERNAL				0

/// Enables the printer module to communicate with nippon 2411D printer.
#define MODULE_ENABLE_PRINTER_NIPPON_2411D				0

/// Enables the printer module to communicate with innoprint printer.
#define MODULE_ENABLE_PRINTER_INNOPRINT					0

/// Enables the printer module to communicate with vdai printer.
#define MODULE_ENABLE_PRINTER_VDAI						0

/// Enables the progress module to visualize progress using multiple LEDs.
#define MODULE_ENABLE_PROGRESS							0

/// Enables the rfid module to communicate with rfid chips.
#define MODULE_ENABLE_RFID								0

/// Enables the ML90132 RFID chip to be used by rfid module.
#define MODULE_ENABLE_RFID_ML90132						0

/// Enables the ST25R391X RFID Chip to be used by rfid module.
#define MODULE_ENABLE_RFID_ST25R391X                    0

/// Enables the rfid module to communicate with rfid ACG chips.
#define MODULE_ENABLE_RFID_ACG							0

/// Enables the module for remote procedure call
#define MODULE_ENABLE_RPC                               0

/// Enables the rtc module for calculation function on time.
#define MODULE_ENABLE_RTC								1

/// Enables module for security
#define MODULE_ENABLE_SECURITY                          1

/// Enables the segment7 module for showing info on 7-Segment displays.
#define MODULE_ENABLE_SEGMENT7							0

/// Enables the module for an external acceleration sensor
#define MODULE_ENABLE_SENSOR_ACCEL_ADXL345				0

/// Enables the I2C A/D converter tla2528.
#define MODULE_ENABLE_AD_TLA2528                        0

/// Enables an abstract light sensor so you do not have to use the concrete light sensor implementations.
#define MODULE_ENABLE_SENSOR_LIGHT                      0

/// Enables the module for an external light sensor
#define MODULE_ENABLE_SENSOR_LIGHT_TSL2571				0

/// Enables the si115x light and proximity sensor
#define MODULE_ENABLE_SENSOR_LIGHT_SI115X               0

/// Enables an abstract proximity sensor so you do not have to use the concrete proximity sensor implementations.
#define MODULE_ENABLE_SENSOR_PROXIMITY                  0

/// Enables the module for an temperature sensor
#define MODULE_ENABLE_SENSOR_TEMPERATURE     			0

/// Enables the module for an external temperature sensor
#define MODULE_ENABLE_SENSOR_TEMPERATURE_MAX6675		0

/// Enables the module for an resistor based temperature sensor
#define MODULE_ENABLE_SENSOR_TEMPERATURE_AD 			0

/// Enables the module for an external temperature sensor
#define MODULE_ENABLE_SENSOR_TEMPERATURE_TMP102			0

/// Enables the synchronization for the binary buffer.
#define MODULE_ENABLE_SERIALIZATION_BINARY_BUFFER       1

/// Enables the module containing serialization functions for JSON
#define MODULE_ENABLE_SERIALIZATION_JSON				1

/// Enables the module containing serialization functions for XML
#define MODULE_ENABLE_SERIALIZATION_XML					0

/// Enables the redebus protocol for RS485 communication.
#define MODULE_ENABLE_SERIALPROTOCOL_REDEBUS            0

/// Enables the ssp module containing the ssp protool implementation used by innovative devices.
#define MODULE_ENABLE_SSP								0

/// Enables the module for an us2400 rf chip.
#define MODULE_ENABLE_US2400							0

/// Enable the utility functions for the color.
#define MODULE_ENABLE_UTIL_COLOR                        0

/// Enables the memory pool utility
#define MODULE_ENABLE_UTIL_MEM_POOL                     1

/// Enables the module for an w5100 TCP/IP chip, including socket api.
#define MODULE_ENABLE_W5100								0

/// Enables the module for an w5500 TCP/IP chip, including socket api.
#define MODULE_ENABLE_W5500								0

/// Enables the webclient module for downloading files via HTTP(s)
#define MODULE_ENABLE_WEBCLIENT                         0

/// Enables the webserver module.
#define MODULE_ENABLE_WEBSERVER							0

/// Enables the websocket module.
#define MODULE_ENABLE_WEBSOCKET                         0

/// Enables wifi for BX310X chip
#define MODULE_ENABLE_WIFI_BX310X						0

/// Enables the usage of the wildcards module.
#define MODULE_ENABLE_WILDCARDS							1

/// Enable the third party support for miniz
#define THIRD_PARTY_ENABLE_MINIZ                        1

#endif // __MODULE_ENABLE_H_GUARD__
