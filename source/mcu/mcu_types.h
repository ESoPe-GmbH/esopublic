// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file mcu_types.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *		Defines the types used by mcu functions.
 *	@version 1.01 (20.08.2022)
 *  	- Changed pointer for handles like mcu_uart_t from void* to corresponding structure name like struct mcu_uart_s.
 *  @version 1.00 (10.10.2016)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MCU_MCU_TYPES_H_
#define MCU_MCU_TYPES_H_

#include "module/enum/function_return.h"

//------------------------------------------------------------------------------------------------------------
// Definitions
//------------------------------------------------------------------------------------------------------------

#ifndef NULL
	/// Defining NULL because it can be a parameter in some functions.
	#define NULL	0		
#endif

#ifndef bool	// #ifndef BUILD in Eclipse (not E2Studio) can help prevent error with bool type
	/// Define bool to prevent errors with the error parser in the editor. Compiler is fine without this because of stdbool.
	#define bool unsigned char	
#else
	#undef bool
	typedef char bool;
#endif

	///@defgroup mcu_group_handler      MCU periphery pointer
	/// The pointer are used for the internal structures of the mcu periphery modules. They are used for the purpose of keeping the structures
	/// private.
	/// @{
#ifndef _RENESAS_SYNERGY_
/// Handle for io Interrupts.
typedef struct mcu_io_int_s*	mcu_io_int_t;	
#else
#include "r_external_irq_api.h"
/// Handle for io Interrupts.
typedef external_irq_instance_t*   mcu_io_int_t;
#endif
#ifndef _RENESAS_SYNERGY_ // On synergy the mcu_timer_t is not supported!
/// Handle for timer.
typedef struct mcu_timer_s*	mcu_timer_t;
#endif
/// Handle for pwm.
typedef struct mcu_pwm_s*	mcu_pwm_t;
#ifndef _RENESAS_SYNERGY_
/// Handle for uarts.
typedef struct mcu_uart_s*	mcu_uart_t;
#else
#include "r_uart_api.h"
/// Handle for uarts.
typedef uart_instance_t*   mcu_uart_t;
#endif
#ifndef _RENESAS_SYNERGY_
/// Handle for spi.
typedef struct mcu_spi_s*	mcu_spi_t;
#else
/// Handle for spi.
typedef void*   mcu_spi_t;
#endif
/// Handle for i2c.
typedef struct mcu_i2c_s*	mcu_i2c_t;
/// Handle for can.
typedef struct mcu_can_s*	mcu_can_t;		
/// Handle for ad converter.
typedef struct mcu_ad_s*	mcu_ad_t;		
/// Handle for da converter.
typedef struct mcu_da_s*	mcu_da_t;		
/// Handle for ethernet
typedef struct mcu_eth_s*	mcu_eth_t;
///	@}

//------------------------------------------------------------------------------------------------------------
// Enumerations and structures
//------------------------------------------------------------------------------------------------------------
/// @enum MCU_RESULT
///	Used as return value of many mcu functions. The last error can be checked with mcu_get_last_error. Should always be MCU_OK.
typedef enum
{
	/// No error occured.
	MCU_OK,	

	/// Error when parameter are NULL unexpected.
	MCU_ERROR_NULL_PARAMETER,
	/// Error allocating space
	MCU_ERROR_INSUFFICIENT_MEMORY,
	/// Function was not implemented
	MCU_ERROR_NOT_IMPLEMENTED,
	/// Function is not supported for MCU.
	MCU_ERROR_NOT_SUPPORTED,

	// Frequenzen
	/// External frequency cannot be used.
	MCU_ERROR_FRQ_EXT_INVALID,	
	/// The cpu clock is invalid.
	MCU_ERROR_FRQ_MCU_INVALID,	
	/// the peripheral clock is invalid.
	MCU_ERROR_FRQ_PERIPHERAL_INVALID,	

	// IO Interrupt
	/// IO Interrupt: The mcu_io_int_t is invalid.
	MCU_ERROR_IO_INT_INVALID,	
	/// IO Interrupt: All i/o interrupts are in use (Adjust MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT).
	MCU_ERROR_IO_INT_NOT_AVAILABLE,
												
	/// IO Interrupt: The pin does not support i/o interrupt functionality.
	MCU_ERROR_IO_INT_PIN_INVALID,	
	/// IO Interrupt: The interrupt level is invalid.
	MCU_ERROR_IO_INT_LVL_INVALID,	
	/// IO Interrupt: The edge is invalid (Only rising, falling or both).
	MCU_ERROR_IO_INT_EDGE_INVALID,	

	// Timer
	/// Timer: All timers are in use (Adjust MCU_PERIPHERY_DEVICE_COUNT_TIMER).
	MCU_ERROR_TMR_NOT_AVAILABLE,	
	/// Timer: Interrupt Level is invalid.
	MCU_ERROR_TMR_LVL_INVALID,	
	/// Timer: The frequency is too high or too low.
	MCU_ERROR_TMR_FRQ_INVALID,	

	// UART
	/// UART: The uart parameters cannot be used for an uart.
	MCU_ERROR_UART_INVALID,	
	/// UART: All uarts are in use (Adjust MCU_PERIPHERY_DEVICE_COUNT_UART).
	MCU_ERROR_UART_NOT_AVAILABLE,	
	/// UART: The number of databits is not supported.
	MCU_ERROR_UART_DATABITS_INVALID,	
	/// UART: The parity is not supported.
	MCU_ERROR_UART_PARITY_INVALID,	
	/// UART: The number of stopbits is not supported.
	MCU_ERROR_UART_STOPBITS_INVALID,	
	/// UART: The baudrate is too high or too low.
	MCU_ERROR_UART_BAUDRATE_INVALID,	
	/// UART: Alternate receive or receive buffer are invalid (NULL).
	MCU_ERROR_UART_RECEIVE_INVALID,	

	// SPI
	/// SPI: The spi parameters cannot be used for a spi.
	MCU_ERROR_SPI_INVALID,	
	/// SPI: All spi are in use (Adjust MCU_PERIPHERY_DEVICE_COUNT_SPI).
	MCU_ERROR_SPI_NOT_AVAILABLE,	
	/// SPI: Mode is invalid.
	MCU_ERROR_SPI_MODE_INVALID,	
	/// SPI: Clock is too high or too low.
	MCU_ERROR_SPI_CLOCK_INVALID,	

	// CAN
	/// CAN: The can parameters cannot be used for a can.
	MCU_ERROR_CAN_INVALID, 	
	/// CAN: All can are in use (Adjust MCU_PERIPHERY_DEVICE_COUNT_CAN).
	MCU_ERROR_CAN_NOT_AVAILABLE,	
	/// CAN: Baudrate is too high or too low.
	MCU_ERROR_CAN_BAUDRATE_INVALID,	
	/// CAN: Receive buffer size is invalid.
	MCU_ERROR_CAN_RECEIVE_INVALID,	
	/// CAN: The messagebox is invalid or not initialized.
	MCU_ERROR_CAN_MESSAGE_BOX_INVALID,	

	// AD
	/// AD: The ad converter parameters cannot be used for an ad converter.
	MCU_ERROR_AD_INVALID,	
	/// AD: All ad converter are in use (Adjust MCU_PERIPHERY_DEVICE_COUNT_AD).
	MCU_ERROR_AD_NOT_AVAILABLE,	
	/// AD: The io pin cannot be used with the ad converter.
	MCU_ERROR_AD_IO_PIN_INVALID,	
	/// AD: Controller does not support ad interrupt.
	MCU_ERROR_AD_INTERRUPT_NOT_SUPPORTED,	
	/// AD: Controller does not support signed values.
	MCU_ERROR_AD_SIGNEDNESS_INVALID,	
	/// AD: Controller does not support the resolution.
	MCU_ERROR_AD_RESOLUTION_INVALID,	

	// DA
	/// DA: The da converter parameters cannot be used for an ad converter.
	MCU_ERROR_DA_INVALID,	
	/// DA: All da converter are in use (Adjust MCU_PERIPHERY_DEVICE_COUNT_DA).
	MCU_ERROR_DA_NOT_AVAILABLE,	
	/// DA: The io pin cannot be used with the da converter.
	MCU_ERROR_DA_IO_PIN_INVALID,	
	/// DA: Controller does not support the resolution.
	MCU_ERROR_DA_RESOLUTION_INVALID,	

	// I2C
	/// I2C: The i2c pins are invalid
	MCU_ERROR_I2C_INVALID,	
	/// I2C: All i2c converter are in use (Adjust MCU_PERIPHERY_DEVICE_COUNT_I2C).
	MCU_ERROR_I2C_NOT_AVAILABLE,	
	/// I2C: Clock is too high or too low.
	MCU_ERROR_I2C_CLOCK_INVALID,	

	// PWM
	/// PWM: The PWM pins/timer are invalid
	MCU_ERROR_PWM_INVALID,
	/// PWM: All PWM are in use.
	MCU_ERROR_PWM_NOT_AVAILABLE,
	/// PWM: Configuratrion is invalid.
	MCU_ERROR_PWM_CONFIGURATION_INVALID,

	/// Ethernet error happened
	MCU_ERROR_ETHERNET
}
MCU_RESULT;

///@enum MCU_IO_DIRECTION
///		Parameter for mcu_io_set_dir. Is used to define Input/Output direction of i/o pins.
typedef enum
{
	/// Output
	MCU_IO_DIR_OUT = 1,			
	/// Input
	MCU_IO_DIR_IN = 0			
}
MCU_IO_DIRECTION;

///@enum MCU_INT_LVL
///		4 predefined interrupt levels for the interrupt periphery of the mikrocontrollers.
typedef enum
{
	/// Interrupt disabled.
	MCU_INT_LVL_OFF 	= 0,		
	/// Interrupt Level Low.
	MCU_INT_LVL_LO 		= 1,		
	/// Interrupt Level Medium.
	MCU_INT_LVL_MED 	= 2,
	/// Interrupt Level higher than medium but lower than high.
	MCU_INT_LVL_HI_LO 	= 3,
	/// Interrupt Level High.
	MCU_INT_LVL_HI 		= 4
}
MCU_INT_LVL;

///@enum MCU_IO_INT_EDGE
///		Enumeration for the edges the i/o interrupt can use as a trigger. Not every controller might support all of them.
typedef enum
{
	/// IO Interrupt triggered at rising and falling edge.
	MCU_IO_INT_EDGE_BOTH = 0,		
	/// IO Interrupt at rising edge.
	MCU_IO_INT_EDGE_HIGH = 1,		
	/// IO Interrupt at falling edge.
	MCU_IO_INT_EDGE_LOW  = 2		
}
MCU_IO_INT_EDGE;

///@struct can_frame_t
///		Frame for can packages that are used for reception and transmission. May need to be packed for some compilers.
typedef struct
{
	/// The id used inside the can package (extended 29-bit or standard 10-bit).
	uint32_t id;
	/// Union for info byte and its bit structure.
	union{
		/// Info byte contains message number and extended bit.
		uint8_t info_byte;	
		/// Bit structure for the info byte.
		struct{
			/// true if id is extended (29-bit), false if id is standard (10-bit).
			uint8_t is_extended: 1;		
			/// Number of the message box the controller uses.
			uint8_t message_box: 7;		
		}elements;			
	}info;					
	/// Length of the data (0 - 8) inside the package.
	uint8_t dlc;			
	/// Data bytes of the package.
	uint8_t data[8];		
}
can_frame_t;

///@enum MCU_AD_SIGNEDNESS
///		Not supported by all microcontrollers. Declares the signedness of the ad values.
typedef enum
{
	/// Signed
	MCU_AD_SIGNED 		= 0,	
	/// Unsigned
	MCU_AD_UNSIGNED 	= 1		
}
MCU_AD_SIGNEDNESS;

#if !defined(ESP_PLATFORM) && !defined(_WIN32) && !defined(__linux__)
#if !defined(_RENESAS_SYNERGY_)

#if  !defined(_TIME_H_) && !defined(USE_HAL_DRIVER)
//typedef uint64_t time_t;

struct tm {
   uint8_t tm_sec;         /* seconds,  range 0 to 59          */
   uint8_t tm_min;         /* minutes, range 0 to 59           */
   uint8_t tm_hour;        /* hours, range 0 to 23             */
   uint8_t tm_mday;        /* day of the month, range 1 to 31  */
   uint8_t tm_mon;         /* month, range 0 to 11             */
   uint16_t tm_year;        /* The number of years since 1900   */
   uint8_t tm_wday;        /* day of the week, range 0 to 6    */
   uint16_t tm_yday;        /* day in the year, range 0 to 365  */
   bool tm_isdst;       /* daylight saving time             */
};
#endif

#else
#include "r_rtc_api.h"
#endif
#else
#include <time.h>
#endif
/// @brief Structure for time, that is compatible with struct tm, but has additional fractions in milliseconds.
struct rtc_time_s 
{
   /// seconds,  range 0 to 59
   uint8_t tm_sec;        
   /// minutes, range 0 to 59
   uint8_t tm_min;      
   /// hours, range 0 to 23
   uint8_t tm_hour;        
   /// day of the month, range 1 to 31
   uint8_t tm_mday;    
   /// month, range 0 to 11
   uint8_t tm_mon;     
   /// The number of years since 1900
   uint16_t tm_year;    
   /// day of the week, range 0 to 6 
   uint8_t tm_wday;        
   /// day in the year, range 0 to 365
   uint16_t tm_yday;       
   /// daylight saving time 
   bool tm_isdst;  
   /// Fractions in milliseconds
   uint16_t tm_msec;
   /// Timezone of the timestamp
   int8_t tm_timezone;
};
///@struct rtc_time_t
///		Structure for storing the rtc time. This is used in the internal rtc of some microcontrollers.
typedef struct rtc_time_s rtc_time_t;

/**
 * @struct mcu_clock_change_listener_t
 * Structure used to register a clock change listener. Is needed to register for a notification when the clock of the mcu is changed.
 * This might be needed to recalculate values.
 */
typedef struct
{
	/// Pointer to the object used in the callback function
	void* obj;
	/// Callback function that is called when the clock changed.
	void (*f)(void*);
	/// Pointer to the next mcu_clock_change_listener_t in a list.
	void* next;
}mcu_clock_change_listener_t;

//------------------------------------------------------------------------------------------------------------
// I/O definitions and structures
//------------------------------------------------------------------------------------------------------------
#include "mcu_config.h"

/**
 * @struct mcu_io_handler_t
 * 		The handler can be used in modules to set i/o pins to high with 1 and to low with 0. Therefore the handler uses an inverted byte
 * 		that shows if setting a pin to 0 really sets a 0 at the end device. Can also be used for leds to enlighten them with 1 if they only
 * 		enlighten with a 0.
 * 		To use the inversion efficiently some macros (MCU_IO_HANDLER_ ...) are defined.
 */
typedef struct{
	/// Pin at the microcontroller
	MCU_IO_PIN pin;		
	/// true:	1 -> pin = 0 and 0 -> pin = 1\n
	/// false:	1 -> pin = 1 and 0 -> pin = 0
	bool is_inverted;	
}mcu_io_handler_t;

/// Define for a high level with MCU_IO_HANDLER_SET and MCU_IO_HANDLER_GET.
#define MCU_IO_HIGH		1	
/// Define for a low level with MCU_IO_HANDLER_SET and MCU_IO_HANDLER_GET.
#define MCU_IO_LOW		0	

#if MCU_PERIPHERY_ENABLE_ETHERNET

typedef enum
{
	MCU_ETHERNET_AUTONEGOTIATION_DISABLE = 0,
	MCU_ETHERNET_AUTONEGOTIATION_ENABLE = 1
}MCU_ETHERNET_AUTONEGOTIATION_T;

typedef enum
{
	MCU_ETHERNET_SPEED_10M = 0,
	MCU_ETHERNET_SPEED_100M = 1
}MCU_ETHERNET_SPEED_T;

typedef enum
{
	MCU_ETHERNET_DUPLEXMODE_HALFDUPLEX = 0,
	MCU_ETHERNET_DUPLEXMODE_FULLDUPLEX = 1
}MCU_ETHERNET_DUPLEXMODE_T;


typedef struct mcu_ethernet_rmii_s
{
	MCU_IO_PIN tx_en;
	MCU_IO_PIN txd0;
	MCU_IO_PIN txd1;

	MCU_IO_PIN crs_dv;
	MCU_IO_PIN rxd0;
	MCU_IO_PIN rxd1;

	MCU_IO_PIN mdc;
	MCU_IO_PIN mdio;
	MCU_IO_PIN ref_clk;

	MCU_ETHERNET_AUTONEGOTIATION_T autonegotiation;

	MCU_ETHERNET_SPEED_T speed;

	MCU_ETHERNET_DUPLEXMODE_T duplexmode;

	uint16_t phy_address;

	uint8_t mac[6];

}mcu_ethernet_rmii_t;

#endif

/// Method type for the set direction method of an I/O expander.
/// First parameter is the object of the I/O expander as set in mcu_io_expander_t.
/// Second parameter is an I/O number of the expander itself.
/// Third parameter is the direction based on the mcu direction enum.
typedef void (*mcu_io_expander_set_pin_direction_t)(void*, int, MCU_IO_DIRECTION);

/// Method type for the set pullup method of an I/O expander.
/// First parameter is the object of the I/O expander as set in mcu_io_expander_t.
/// Second parameter is an I/O number of the expander itself.
/// Third parameter is true for activating a pull-up or false for deactivating a pull-up.
typedef void (*mcu_io_expander_set_pullup_t)(void*, int, bool);

/// Method type for the set pin method of an I/O expander.
/// First parameter is the object of the I/O expander as set in mcu_io_expander_t.
/// Second parameter is an I/O number of the expander itself.
/// Third parameter is the level on the I/O to set.
typedef void (*mcu_io_expander_set_pin_t)(void*, int, bool);

/// Method type for the pin toggling method of an I/O expander.
/// First parameter is the object of the I/O expander as set in mcu_io_expander_t.
/// Second parameter is an I/O number of the expander itself.
/// Third parameter is the level on the I/O to set.
typedef void (*mcu_io_expander_toggle_pin_t)(void*, int);

/// Method type for the get pin method of an I/O expander.
/// First parameter is the object of the I/O expander as set in mcu_io_expander_t.
/// Second parameter is an I/O number of the expander itself.
/// Return value is the read level on the I/O.
typedef bool (*mcu_io_expander_get_pin_t)(void*, int);

/**
 * @brief	Initializes a mcu_io_int_t pointer for the given io interrupt pin. If it is not possible to set the i/o interrupt
 * 			the function returns NULL and an error will be set into mcu_last_error.
 *
 *			The following errors might occur:
 * 			 - MCU_ERROR_IO_INT_PIN_INVALID: 	There is no i/o interrupt with the pin and number.\n
 *			 - MCU_ERROR_IO_INT_NOT_AVAILABLE: 	All i/o interrupts are in use. Adjust MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT in mcu_config.h
 *
 * @pre		Check MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT in the mcu_config.h before.
 *
 * @param drv    	Pointer to the i/o expander driver.
 * @param pin_num   I/O number of the expander itself.
 * @return			FUNCTION_RETURN_OK if interrupt could be initialized or false if not.
 */
typedef FUNCTION_RETURN_T (*mcu_io_expander_interrupt_init_t)(void* drv, int pin_num);

/**
 * @brief	Sets the parameters for an I/O interrupt of an I/O expander.
 *
 * @pre		handle must be created with mcu_io_expander_io_interrupt_init before.
 *
 * @param drv    	Pointer to the i/o expander driver.
 * @param pin_num   I/O number of the expander itself.
 * @param obj		Pointer to an object that will be used as a parameter in the callback function.\n
 * 					Can be NULL if no object pointer is needed.
 * @param f			Callback function for the io interrupt.\n
 * 					The callback function has one parameter which is always the same as the obj parameter in this function.
 * @param edge		Interrupt edge trigger: rising, falling or both edges.
 * @return			FUNCTION_RETURN_OK: No error occured.\n
 *			 		FUNCTION_RETURN_PARAM_ERROR: The io interrupt handler is NULL.\n
 * 			 		FUNCTION_RETURN_PARAM_ERROR: The edge does not exist.\n
 * 			 		FUNCTION_RETURN_PARAM_ERROR: The interrupt level does not exist.
 */
typedef FUNCTION_RETURN_T (*mcu_io_expander_interrupt_set_param_t)(void* drv, int pin_num, void* obj, void (*f)(void*), MCU_IO_INT_EDGE edge);

/**
 * @brief	Disables the i/o interrupt.
 *
 * @pre		handle must be created with mcu_io_interrupt_init.
 * @pre		handle must be configured with mcu_io_interrupt_set_param.
 *
 * @param drv    	Pointer to the i/o expander driver.
 * @param pin_num   I/O number of the expander itself.
 */
typedef void (*mcu_io_expander_interrupt_disable_t)(void* drv, int pin_num);

/**
 * @brief	Enables the i/o interrupt. Must only be called if MCU_INT_LVL_OFF was set in mcu_io_interrupt_set_param or mcu_io_interrupt_disable
 * 			was called before.
 *
 * @pre		handle must be created with mcu_io_interrupt_init.
 * @pre		handle must be configured with mcu_io_interrupt_set_param.
 *
 * @param drv    	Pointer to the i/o expander driver.
 * @param pin_num   I/O number of the expander itself.
 */
typedef void (*mcu_io_expander_interrupt_enable_t)(void* drv, int pin_num);

/// Interface an I/O expander should use. You need to declare all variables except next.
typedef struct mcu_io_expander_s mcu_io_expander_t;

/// Interface an I/O expander should use. You need to declare all variables except next.
struct mcu_io_expander_s
{
	/// Pointer to an object of the I/O expander that is used as first parameter in the I/O functions.
	void* obj;
	/// The base pin is the first pin. When calling mcu_io_set the pin number given to it, is subtracted by the pin_base and the result is sent to the I/O expander.
	/// Sample: If pin_base is 0x1000 and mcu_io_set(0x1002, 0); is called, the set function of this I/O expander is called with the pin parameter 2.
	int pin_base;
	/// Number of pins on the I/O expander to use.
	int pin_count;
	/// Set direction function of the I/O expander.
	mcu_io_expander_set_pin_direction_t set_direction;
	/// Set pullup function of the I/O expander.
	mcu_io_expander_set_pullup_t set_pullup;
	/// Set function of the I/O expander.
	mcu_io_expander_set_pin_t set;
	/// Toggle function of the I/O expander.
	mcu_io_expander_toggle_pin_t toggle;
	/// Get function of the I/O expander.
	mcu_io_expander_get_pin_t get;
	/// Initialization function of the I/O expander interrupt.
	mcu_io_expander_interrupt_init_t interrupt_init;
	/// Interrupt parameter settings for the I/O expander
	mcu_io_expander_interrupt_set_param_t interrupt_set_param;
	/// Disable function for the I/O expander interrupt.
	mcu_io_expander_interrupt_disable_t interrupt_disable;
	/// Enable function for the I/O expander interrupt.
	mcu_io_expander_interrupt_disable_t interrupt_enable;
	/// Is used to create a list of expanders. Do not modify manually.
	mcu_io_expander_t* next;
};

#endif /* MCU_MCU_TYPES_H_ */
