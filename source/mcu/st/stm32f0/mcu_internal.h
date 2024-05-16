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

#ifndef MCU_INTERNAL_HEADER_FIRST_INCLUDE_GUARD
#define MCU_INTERNAL_HEADER_FIRST_INCLUDE_GUARD

#include "../../mcu_config.h"

#if MCU_TYPE == MCU_STM32F0

#include "stm32f0xx_hal.h"
//#include "interrupt_handlers.h"
#include "../../sys.h"
#include "module/comm/comm.h"
#include "module/fifo/fifo.h"

#ifdef __GNUC_PATCHLEVEL__
	#define nop()	asm volatile("nop")
#endif

extern MCU_RESULT mcu_last_error;

extern uint32_t mcu_frq_ext_hz;
extern uint32_t mcu_frq_cpu_hz;
extern uint32_t mcu_frq_peripheral_hz;
extern uint32_t mcu_frq_peripheral_bus_hz;

#if MCU_PERIPHERY_DEVICE_COUNT_UART > 0

#if MCU_CONTROLLER_PIN_COUNT == 144
	#define MCU_UART_TOTAL_COUNT	13
#elif MCU_CONTROLLER_PIN_COUNT == 100
	#define MCU_UART_TOTAL_COUNT	9
#elif MCU_CONTROLLER_PIN_COUNT == 48
	#define MCU_UART_TOTAL_COUNT	4
#endif

#define MCU_UART_MAX_NUM			4

typedef struct
{
	UART_HandleTypeDef h_uart;

	uint8_t num;
	uint32_t baud;
	/// TX I/O
	MCU_IO_PIN io_tx;
	/// RX I/O
	MCU_IO_PIN io_rx;

	/// Callback function for uart alternate receive
	void (*alternate_receive)(void*, int);
	/// Parameter for the callback function
	void* obj;

	fifo_t fifo;

	DMA_HandleTypeDef h_dma_tx;
//	DMA_HandleTypeDef h_dma_rx;

	int16_t			dma_rx;
	int16_t			dma_tx;
	int16_t			irq;

	MCU_INT_LVL tx_lvl;

//	uint8_t dma_rx_buffer[100];

	bool use_transmit_buffer;
	uint8_t* transmit_buffer;
	uint16_t transmit_buffer_length;
	uint16_t transmit_buffer_cnt;

	MCU_RESULT	res;					///< Current error code

	system_task_t task;

}mcu_uart_handler_ctx;

extern uint8_t mcu_current_uart_handler;
extern mcu_uart_handler_ctx mcu_uart_handler[MCU_PERIPHERY_DEVICE_COUNT_UART];

#endif

#if MCU_PERIPHERY_DEVICE_COUNT_TIMER>0
#define MCU_TIMER_TOTAL_COUNT		4		/**< Es gibt 5 Timer insgesamt. */

typedef struct{
	uint8_t num;							/**< Die Timer Nummer. */
	bool is_mtu;
	uint32_t frq;							/**< Die eingestellte Timer Frequenz. */
	void (*callback)(void*);					/**< Die Timer Interruptfunktion. */
	/// Parameter for the callback function
	void* obj;
	MCU_RESULT	res;					/**< Aktueller Fehlercode für Debug Zwecke. */
}mcu_timer_handler_ctx;	/**< Enthält für einen Timer das Startbit für das Register, die Frequenz und die Interruptfunktion. */

extern uint8_t mcu_current_timer_handler;	/**< Zähler der die Anzahl der benutzen Timer enthält. */
extern mcu_timer_handler_ctx mcu_timer_handler[MCU_PERIPHERY_DEVICE_COUNT_TIMER];	/**< Kontext Objekte für jeden reservierte Timer. */
#endif



#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0

#if MCU_CONTROLLER_PIN_COUNT == 144
// SPI + UART (in clock synchronous mode)
	#define MCU_SPI_TOTAL_COUNT		12
#elif MCU_CONTROLLER_PIN_COUNT == 100
// SPI + UART (in clock synchronous mode)
	#define MCU_SPI_TOTAL_COUNT		13
#elif MCU_CONTROLLER_PIN_COUNT == 48
// SPI + UART (in clock synchronous mode)
	#define MCU_SPI_TOTAL_COUNT		2
#endif

typedef struct
{
	bool initialized;

	bool is_uart;

	MCU_IO_PIN tx;

	MCU_IO_PIN rx;

	MCU_IO_PIN clk;

	uint8_t num;

	uint32_t clock;

	MCU_RESULT	res;

	SPI_HandleTypeDef hspi;

}mcu_spi_interface_handler_ctx;

typedef struct
{
	/// Pointer to the interface used by this spi handler
	mcu_spi_interface_handler_ctx* spi;

	/// Chip select pin of this spi handler
	MCU_IO_PIN cs;

}mcu_spi_handler_ctx;

extern uint8_t mcu_current_spi_handler;
extern mcu_spi_interface_handler_ctx mcu_spi_interface_handler[MCU_SPI_TOTAL_COUNT];
extern mcu_spi_handler_ctx mcu_spi_handler[MCU_PERIPHERY_DEVICE_COUNT_SPI];


#endif

#if MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT>0
#define MCU_IO_INTERRUPT_TOTAL_COUNT		16	/**< Anzahl der IO Interrupts. */

typedef struct{
	uint8_t num;
	MCU_IO_PIN pin;
	uint8_t lvl;
	int8_t int_flag_num;
	GPIO_InitTypeDef gpio;
	void* callback_obj;
	void (*callback)(void*);
	MCU_RESULT	res;					/**< Aktueller Fehlercode für Debug Zwecke. */
}mcu_io_int_handler_ctx;

extern uint8_t mcu_current_io_int_handler;		/**< Zähler der die Anzahl der benutzen IO Interrupts enthält. */
extern mcu_io_int_handler_ctx mcu_io_int_handler[MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT];	/**< Kontext Objekte für jeden reservierte IO Interrupt. */

#endif

#if MCU_PERIPHERY_DEVICE_COUNT_AD > 0

typedef struct{
	uint8_t channel;
	MCU_IO_PIN port;
	MCU_RESULT res;					/**< Aktueller Fehlercode für Debug Zwecke. */
}mcu_ad_handler_ctx;


extern uint8_t mcu_current_ad_handler;
extern mcu_ad_handler_ctx mcu_ad_handler[MCU_PERIPHERY_DEVICE_COUNT_AD];

#endif

#if MCU_PERIPHERY_DEVICE_COUNT_CAN>0

typedef struct{
	uint8_t num;							/**< Die Nummer des CAN. */
	uint32_t baud;							/**< Die tatsächlich eingestellte Baudrate. */
	uint8_t msg_boxes_in_use;				/**< Enthält die Anzahl der Messageboxen, die in Benutzung sind. */
	fifo_t fifo;						/**< Die FIFO für den eigenen Empfangsbuffer. */
	#if MCU_DEBUG_ENABLE
		MCU_RESULT	res;					/**< Aktueller Fehlercode für Debug Zwecke. */
	#endif
	system_task_t task;

	/// Register of the can
	volatile struct st_can * 	reg;

//	/* Prozessorabhaengig */
//	can_mbox*	cNmb;						/**< Zeiger auf die CAN Message Boxen 0-31. */
//	can_baud*	cNbcr;						/**< Zeiger auf das CAN Bitrate Register. */
//	can_ctlr*	cNctlr;						/**< Zeiger auf das CAN Control Register. */
//	word_reg*	cNstr;						/**< Zeiger auf das CAN Status Register. */
//	can_mask*	cNmkr0;						/**< Zeiger auf das CAN Masken Register. */
//	uint32_t*	cNmkivlr;					/**< Zeiger auf das CAN Mask Invalidate Register */
//	uint32_t*	cNmier;						/**< Zeiger auf das CAN Mailbox Interrupt Enable Register */
//	can_mctl*	cNmctl;						/**< Zeiger auf das Can Mailbox Control Register. */
//	byte_reg*	cNric;						/**< Zeiger auf das CAN Receive Interrupt Control Register. */

}mcu_can_handler_ctx;	/**< Enthält für CAN Schnittstelle die Empfangsfifo, die eingestellte Baudrate und die benötigten Register. */

extern uint8_t mcu_current_can_handler;		/**< Enthält die Anzahl der in Benutzung befindlichen CAN Module. */
extern mcu_can_handler_ctx mcu_can_handler[MCU_PERIPHERY_DEVICE_COUNT_CAN];	/**< Handler Objekte für die reservierten CAN Module. */

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

/**
 * Returns the port pointer belonging to an MCU_IO_PIN
 */
GPIO_TypeDef* mcu_internal_get_port(MCU_IO_PIN port);

#endif

#endif // #if MCU_TYPE == R5F563
