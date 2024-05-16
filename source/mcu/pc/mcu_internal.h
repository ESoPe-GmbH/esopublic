/**
 * 	@file 		mcu_internal.h
 *  @copyright Urheberrecht 2011-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
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

#if MCU_TYPE == PC_EMU

extern MCU_RESULT mcu_last_error;			/**< Enth�lt den letzten Fehlercode den das MCU Modul hatte. */

extern uint32_t mcu_frq_ext_hz;				/**< Enth�lt die Frequenz des externen Quarzes. */
extern uint32_t mcu_frq_pll_hz;				/**< Enth�lt die Frequenz der PLL Clock. */
extern uint32_t mcu_frq_cpu_hz;				/**< Enth�lt die Frequenz der CPU Clock. */
extern uint32_t mcu_frq_base_hz;			/**< Enth�lt die Frequenz der Base Clock. */
extern uint32_t mcu_frq_peripheral_hz;		/**< Enth�lt die Frequenz der Periphery Clock. */
extern uint32_t mcu_frq_peripheral_bus_hz;	/**< Enth�lt die Frequenz der Peripheral Bus Clock. */

#define byte_reg	union byte_def			/**< Union f�r ein Byte aus dem sfr Register */
#define word_reg	union word_def			/**< Union f�r ein Word aus dem sfr Register */
#define lword_reg	union lword_def			/**< Union f�r ein LWord aus dem sfr Register */

typedef union can_ctlr_def				can_ctlr;	/**< Typedef f�r CAN Control Register. */
typedef union can_mail_box_def			can_mbox;	/**< Typedef f�r CAN Message Boxen Struktur. */
typedef union can_bit_config_reg_def	can_baud;	/**< Typedef f�r CAN Baudrate Register Struktur. */
typedef union can_acceptance_mask_def	can_mask;	/**< Typedef f�r CAN Masken Struktur. */
typedef union can_mctl_def				can_mctl;	/**< Typedef f�r CAN Mailbox Control Register Struktur. */


#if MCU_PERIPHERY_DEVICE_COUNT_UART>0
#include "module/fifo/fifo.h"
#include "module/util/buffer.h"
#include "module/network/protocol/webserver/websocket.h"
#include "module/network/protocol/http/webclient.h"

#define MCU_UART_TOTAL_COUNT	9			/**< Der R32C 118 hat 9 UARTs */

#define MCU_UART_MAX_BUFFER_SIZE		1024

#define MCU_UART_NUM_BUFFER				10

#if MODULE_ENABLE_WEBSOCKET && MODULE_ENABLE_WEBCLIENT
#define MCU_UART_ENABLE_ESOPROG			true
#else
#define MCU_UART_ENABLE_ESOPROG			false
#endif

struct mcu_uart_buffer_s
{
	uint8_t tx_buffer[MCU_UART_MAX_BUFFER_SIZE];

	uint32_t tx_length;
};

typedef struct mcu_uart_s
{
	/// Hardware configuration for the UART to declare pins, etc.
	mcu_uart_hw_config_t hw;
	/// Configuration for the UART.
	mcu_uart_config_t config;
	
	void (*alternate_receive)(void*, int);			/**< Die alternative Empfangsfunktion. */
	void* obj;
	fifo_t fifo;						/**< Die FIFO f�r den eigenen Empfangsbuffer. */
	#if MCU_DEBUG_ENABLE
		MCU_RESULT	res;					/**< Aktueller Fehlercode f�r Debug Zwecke. */
	#endif

	uint8_t rx_buffer[MCU_UART_MAX_BUFFER_SIZE];

#if MCU_UART_ENABLE_ESOPROG
	/// Task for receiving data from the websocket and parsing messages
	system_task_t task_esoprog;
	/// Pointer to the websocket to use for esoprog
	websocket_t ws;

	webclient_t wc;

	buffer_rxtx_t ws_buffer;

	struct mcu_uart_buffer_s tx[MCU_UART_NUM_BUFFER];

	struct mcu_uart_buffer_s settings;
#endif

	uint8_t tx_cnt;

	bool config_changed;

	uint32_t timestamp_sent;

}mcu_uart_handler_ctx;	/**< Enth�lt f�r eine bestimmte UART die notwendigen Register, Buffer, Baudrate und die alternative Empfangsfunktion. */

extern uint8_t mcu_current_uart_handler;	/**< Z�hler der die Anzahl der benutzen UARTs enth�lt. */
extern mcu_uart_handler_ctx mcu_uart_handler[MCU_PERIPHERY_DEVICE_COUNT_UART];	/**< Kontext Objekte f�r jede reservierte UART. */
#endif

#if MCU_PERIPHERY_DEVICE_COUNT_TIMER>0
#define MCU_TIMER_TOTAL_COUNT		5		/**< Es gibt 5 Timer insgesamt. */

typedef struct{
	uint8_t num;							/**< Die Timer Nummer. */
	uint32_t frq;							/**< Die eingestellte Timer Frequenz. */
	void (*callback)(void);					/**< Die Timer Interruptfunktion. */
	#if MCU_DEBUG_ENABLE
		MCU_RESULT	res;					/**< Aktueller Fehlercode f�r Debug Zwecke. */
	#endif
}mcu_timer_handler_ctx;	/**< Enth�lt f�r einen Timer das Startbit f�r das Register, die Frequenz und die Interruptfunktion. */

extern uint8_t mcu_current_timer_handler;	/**< Z�hler der die Anzahl der benutzen Timer enth�lt. */
extern mcu_timer_handler_ctx mcu_timer_handler[MCU_PERIPHERY_DEVICE_COUNT_TIMER];	/**< Kontext Objekte f�r jeden reservierte Timer. */
#endif


#define		te_(n)		n->bit.b0	/**< Das te Bit des uNc1 Registers. */
#define		ti_(n)		n->bit.b1	/**< Das ti Bit des uNc1 Registers. */
#define		re_(n)		n->bit.b2	/**< Das re Bit des uNc1 Registers. */
#define		ri_(n)		n->bit.b3	/**< Das ri Bit des uNc1 Registers. */

#define		txept_(n)	n->bit.b3	/**< Das txept Bit des uNc0 Registers. */

#if MCU_DEBUG_ENABLE
	#define MCU_SET_HANDLER_ERROR(handler, err)		(handler)->res = err;		/**< Tr�gt den Fehlercode in das Fehlerbyte des Handlers ein. */
#else
	#define MCU_SET_HANDLER_ERROR(handler, err)									/**< Verwirft den Fehlercode. */
#endif

/**
 * Internes Makro mit dem ein Fehlercode gesetzt wird und anschlie�end return aufgerufen wird.
 *
 * @param handler					Ein Handler, der ein res Byte besitzen muss, wenn Debug aktiv ist.
 * @param err						Der Fehlercode der in den last_error eingetragen wird.
 * @param ret						Der Return Wert der zur�ckgegeben wird.
 **/
#define MCU_RETURN_ERROR(handler, err, ret)											\
				{																	\
					MCU_SET_HANDLER_ERROR(handler, err);							\
					mcu_last_error = err;											\
					return ret;														\
				}

/**
 * Internes Makro mit dem ein Fehlercode nur global gesetzt wird und anschlie�end return aufgerufen wird.
 * Das wird ben�tigt, wenn der Handler NULL ist und dieser entsprechend nicht gesetzt werden kann.
 *
 * @param err						Der Fehlercode der in den last_error eingetragen wird.
 * @param ret						Der Return Wert der zur�ckgegeben wird.
 **/
#define MCU_RETURN_ERROR_GLB(err, ret)												\
				{																	\
					mcu_last_error = err;											\
					return ret;														\
				}

/**
 * Internes Makro, dass aus dem angegebenem MCU_INT_LVL einen Controllerspezifischen Interruptlevel generiert.
 *
 * @param lvl						Ein Level aus dem Enum MCU_INT_LVL.
 * @param register_value			Das Register oder die Variable, in die das Controller Interrupt Level eingetragen wird.
 **/
#define MCU_GET_INT_LVL(lvl, register_value)										\
				switch(lvl)															\
				{																	\
					case MCU_INT_LVL_HI:	register_value = 7;	break;				\
					case MCU_INT_LVL_MED:	register_value = 4;	break;				\
					case MCU_INT_LVL_LO:	register_value = 2;	break;				\
					default:				register_value = 0;	break;				\
				}

/**
 * Das Makro wird benutzt um f�r die UARTs 0-6 die entsprechenden Register zu setzen f�r UART und SPI.
 **/
#define INIT_UART_REG_0_6(n, Xmr, Xc0, Xc1, Xsmr, Xsmr2, Xsmr3, Xsmr4, Xric, Xtic)	\
				u ## n ## mr = Xmr;													\
				u ## n ## c0 = Xc0;													\
				u ## n ## c1 = Xc1;													\
				u ## n ## smr = Xsmr;												\
				u ## n ## smr2 = Xsmr2;												\
				u ## n ## smr3 = Xsmr3;												\
				u ## n ## smr4 = Xsmr4;												\
				s ## n ## ric = Xric;												\
				s ## n ## tic = Xtic

/**
 * Das Makro wird benutzt um f�r die UARTs 7 und 8 die entsprechenden Register zu setzen f�r UART und SPI.
 **/
#define INIT_UART_REG_7_8(n, Xmr, Xc0, Xc1, Xirs, Xrrm, Xric, Xtic)	\
				u ## n ## mr = Xmr;													\
				u ## n ## c0 = Xc0;													\
				u ## n ## c1 = Xc1;													\
				u ## n ## irs = Xirs;												\
				u ## n ## rrm = Xrrm;												\
				s ## n ## ric = Xric;												\
				s ## n ## tic = Xtic

/**
 * Setzt f�r die �bergebene UART den Ausgang, den Eingang, den Pull-Up am Eingang und das Special Pin Register am Ausgang.
 **/
#define MCU_UART_SET_IO(in, out)													\
				{																	\
				mcu_io_set_dir(P ## in ## , MCU_IO_DIR_IN);							\
				mcu_io_set_pullup(P ## in ## , 1);									\
				mcu_io_set_dir(P ## out ## , MCU_IO_DIR_OUT);						\
				p ## out ## s = 0x03;												\
				}

#endif

#endif
