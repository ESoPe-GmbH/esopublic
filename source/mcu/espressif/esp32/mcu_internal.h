// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_internal.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *  @date		13.09.2011
 *
 *  @brief
 *  		Der Header deklariert Variablen die innerhalb der unterschiedlichen MCU Peripherie Sourcen benutzt werden.
 *			Auf die Variablen in dieser Datei darf nur innerhalb des MCU Moduls zugegriffen werden!
 *
 *  @version	1.00
 *  			 - Erste Version
 *
 ******************************************************************************/

#include "../../mcu_config.h"
#if MCU_TYPE == MCU_ESP32

#ifndef MCU_INTERNAL_HEADER_FIRST_INCLUDE_GUARD
#define MCU_INTERNAL_HEADER_FIRST_INCLUDE_GUARD

#include "mcu/common/mcu_internal.h"

#include "driver/uart.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#if MCU_PERIPHERY_DEVICE_COUNT_UART > 0

#define MCU_UART_MAX_NUM			2

struct mcu_uart_s
{
	/// Hardware configuration for the UART to declare pins, etc.
	mcu_uart_hw_config_t hw;
	/// Configuration for the UART.
	mcu_uart_config_t config;

	/// Callback function for uart alternate receive
	void (*alternate_receive)(void*, int);
	/// Callback function for notification when data is received
	void (*receive_notify)(void*);
	/// Parameter for the callback function
	void* obj;
	/// Parameter for the callback function
	void* obj_receive_notify;
	
	QueueHandle_t uart_queue;

	MCU_RESULT	res;					///< Current error code

	system_task_t task;

};

extern uint8_t mcu_current_uart_handler;
extern struct mcu_uart_s mcu_uart_handler[MCU_PERIPHERY_DEVICE_COUNT_UART];

#endif

#if MCU_PERIPHERY_DEVICE_COUNT_TIMER > 0
#define MCU_TIMER_TOTAL_COUNT		4		

struct mcu_timer_s
{
	uint8_t num;
	uint32_t frq;							
	void (*callback)(void*);					
	void* obj;
	MCU_RESULT	res;					
};

extern uint8_t mcu_current_timer_handler;	
extern struct mcu_timer_s mcu_timer_handler[MCU_PERIPHERY_DEVICE_COUNT_TIMER];	
#endif

#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0

#define MCU_SPI_TOTAL_COUNT		3

typedef struct mcu_spi_interface_s
{
	bool initialized;

	/// Semaphore used to synchronize debug calls.
	SemaphoreHandle_t semaphore;

	spi_bus_config_t bus;

	uint8_t num;

	MCU_RESULT	res;

}mcu_spi_interface_t;

struct mcu_spi_s
{
    /// Indicates whether this structure is in use or not.
    bool initialized;
	/// Pointer to the interface used by this spi handler
	struct mcu_spi_interface_s* spi;
	/// Chip select pin of this spi handler
	MCU_IO_PIN cs;
	/// Bus select pin of this spi handler
	spi_device_interface_config_t businterface;

	spi_device_handle_t dev;

	uint32_t transaction_count;
};

extern uint8_t mcu_current_spi_handler;
extern struct mcu_spi_interface_s mcu_spi_interface_handler[MCU_SPI_TOTAL_COUNT];
extern struct mcu_spi_s mcu_spi_handler[MCU_PERIPHERY_DEVICE_COUNT_SPI];

#endif

#if MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT>0
#define MCU_IO_INTERRUPT_TOTAL_COUNT		16

struct mcu_io_int_s
{
	uint8_t num;
	MCU_IO_PIN pin;
	uint8_t lvl;
	void* callback_obj;
	void (*callback)(void*);
	MCU_RESULT	res;					
};

extern uint8_t mcu_current_io_int_handler;		
extern struct mcu_io_int_s mcu_io_int_handler[MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT];

#endif

#if MCU_PERIPHERY_DEVICE_COUNT_AD > 0

struct mcu_ad_s
{
	uint8_t channel;
	MCU_IO_PIN port;
	MCU_RESULT res;					
};


extern uint8_t mcu_current_ad_handler;
extern struct mcu_ad_s mcu_ad_handler[MCU_PERIPHERY_DEVICE_COUNT_AD];

#endif

#if MCU_PERIPHERY_DEVICE_COUNT_CAN>0

struct mcu_can_s
{
	uint8_t num;							
	uint32_t baud;							
	bool has_filter_set;
	bool filter_extended;
	#if MCU_DEBUG_ENABLE
		MCU_RESULT	res;					
	#endif

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
