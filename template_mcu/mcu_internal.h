// Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_internal.h
 * 	@copyright Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *  @date		13.05.2025
 *
 *  @brief		Declares the structures 
 *
 *  @version	1.00
 *  			 - First release
 *
 ******************************************************************************/

#include "../../mcu_config.h"
#if MCU_TYPE == MCU_NEW_NAME

#ifndef MCU_INTERNAL_HEADER_FIRST_INCLUDE_GUARD
#define MCU_INTERNAL_HEADER_FIRST_INCLUDE_GUARD

#include "mcu/common/mcu_internal.h"

// TODO: Includes

#if MCU_PERIPHERY_DEVICE_COUNT_UART > 0

/// Maximum number of UARTs on this mcu -> Indices 0 to (MCU_UART_MAX_NUM - 1)
#define MCU_UART_MAX_NUM			2 // TODO: Modify for mcu

struct mcu_uart_s
{
	/// Hardware configuration for the UART to declare pins, etc.
	mcu_uart_hw_config_t hw;
	/// Configuration for the UART.
	mcu_uart_config_t config;

	// TODO: Add other internal variables for the UART

	/// Callback function for uart alternate receive
	void (*alternate_receive)(void*, int);
	/// Callback function for notification when data is received
	void (*receive_notify)(void*);
	/// Parameter for the callback function
	void* obj;
	/// Parameter for the callback function
	void* obj_receive_notify;	
#if MCU_DEBUG_ENABLE
	/// Current error code
	MCU_RESULT	res;
#endif
};

extern uint8_t mcu_current_uart_handler;
extern struct mcu_uart_s mcu_uart_handler[MCU_PERIPHERY_DEVICE_COUNT_UART];

#endif

#if MCU_PERIPHERY_DEVICE_COUNT_TIMER > 0

/// Maximum number of timer the mcu supports.
#define MCU_TIMER_TOTAL_COUNT		4		 // TODO: Modify for mcu

struct mcu_timer_s
{
	/// @brief Index of the peripheral timer
	uint8_t num;
	/// @brief Frequency of the timer
	uint32_t frq;						
	/// Callback function for the timer	
	void (*callback)(void*);	
	/// Parameter for the callback function				
	void* obj;
#if MCU_DEBUG_ENABLE
	/// Current error code
	MCU_RESULT	res;
#endif				

	// TODO: Add other internal variables for the timer	
};

extern uint8_t mcu_current_timer_handler;	
extern struct mcu_timer_s mcu_timer_handler[MCU_PERIPHERY_DEVICE_COUNT_TIMER];	
#endif

#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0

/// Maximum number of SPI the mcu supports.
#define MCU_SPI_TOTAL_COUNT		3 // TODO: Modify for mcu

/// @brief Structure for the SPI bus interface, can be shared by multiple SPI with different chip select pins.
typedef struct mcu_spi_interface_s
{
	/// @brief Indicates whether the spi interface was initialized or not.
	bool initialized;
	/// @brief Index of the spi (0 to MCU_SPI_TOTAL_COUNT - 1)
	uint8_t num;
#if MCU_DEBUG_ENABLE
	/// Current error code
	MCU_RESULT	res;
#endif

	// TODO: Add other internal variables for the SPI	

}mcu_spi_interface_t;

/// @brief Structure for the SPI, that uses one SPI bus. Multiple SPI can share the same SPI bus with different chip select.
struct mcu_spi_s
{
    /// Indicates whether this structure is in use or not.
    bool initialized;
	/// Pointer to the interface used by this spi handler
	struct mcu_spi_interface_s* spi;
	/// Chip select pin of this spi handler
	MCU_IO_PIN cs;
	/// @brief Number of transactions in use.
	uint32_t transaction_count;

	// TODO: Add other internal variables for the SPI	
};

extern uint8_t mcu_current_spi_handler;
extern struct mcu_spi_interface_s mcu_spi_interface_handler[MCU_SPI_TOTAL_COUNT];
extern struct mcu_spi_s mcu_spi_handler[MCU_PERIPHERY_DEVICE_COUNT_SPI];

#endif

#if MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT>0

/// Maximum number of I/O interrupts the mcu supports.
#define MCU_IO_INTERRUPT_TOTAL_COUNT		16 // TODO: Modify for mcu

/// @brief Structure for a single I/O interrupt
struct mcu_io_int_s
{
	/// @brief Index of the I/O interrupt (0 to MCU_IO_INTERRUPT_TOTAL_COUNT-1)
	uint8_t num;
	/// @brief Pin of the I/O interrupt
	MCU_IO_PIN pin;
	/// @brief Priority level of the I/O interrupts
	uint8_t lvl;
	/// @brief Parameter for the callback
	void* callback_obj;
	/// @brief Callback that is triggered on the I/O interrupt
	void (*callback)(void*);
#if MCU_DEBUG_ENABLE
	/// @brief Current error code
	MCU_RESULT	res;
#endif

	// TODO: Add other internal variables for the I/O interrupt	
};

extern uint8_t mcu_current_io_int_handler;		
extern struct mcu_io_int_s mcu_io_int_handler[MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT];

#endif

#if MCU_PERIPHERY_DEVICE_COUNT_AD > 0

/// Maximum number of AD converter the mcu supports.
#define MCU_AD_TOTAL_COUNT		8 // TODO: Modify for mcu

/// @brief Structure for a single AD converter
struct mcu_ad_s
{
	/// @brief Channel of the ad converter
	uint8_t channel;
	/// @brief Pin of the ad converter
	MCU_IO_PIN port;
#if MCU_DEBUG_ENABLE
	/// @brief Current error code
	MCU_RESULT res;
#endif		

	// TODO: Add other internal variables for the AD	
};

extern uint8_t mcu_current_ad_handler;
extern struct mcu_ad_s mcu_ad_handler[MCU_PERIPHERY_DEVICE_COUNT_AD];

#endif

#if MCU_PERIPHERY_DEVICE_COUNT_CAN > 0

/// Maximum number of CAN interfaces the mcu supports.
#define MCU_CAN_TOTAL_COUNT		2 // TODO: Modify for mcu

/// @brief Structure for a CAN interface
struct mcu_can_s
{
	/// @brief Index of the CAN interface
	uint8_t num;
	/// @brief Configured baudrate of the CAN interface			
	uint32_t baud;
	
#if MCU_DEBUG_ENABLE
/// @brief Current error code
	MCU_RESULT	res;					
#endif

	// TODO: Add other internal variables for the CAN	

};	

extern uint8_t mcu_current_can_handler;	
extern struct mcu_can_s mcu_can_handler[MCU_PERIPHERY_DEVICE_COUNT_CAN];	

#endif

#define MCU_RETURN_ERROR(err, ret)													\
				{																	\
					mcu_last_error = err;											\
					return ret;														\
				}

#define MCU_GET_INT_LVL(lvl, register_value)										\
				switch(lvl)															\
				{																	\
					case MCU_INT_LVL_HI:	register_value = 15;	break;				\
					case MCU_INT_LVL_HI_LO:	register_value = 14;	break;				\
					case MCU_INT_LVL_MED:	register_value = 10;	break;				\
					case MCU_INT_LVL_LO:	register_value = 5;		break;				\
					default:				register_value = 0;		break;				\
				}

#endif

#endif // #if MCU_TYPE
