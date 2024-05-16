/**
 * 	@file 		mcu_controller.h
 *  @copyright Urheberrecht 2011-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *  		Das MCU Modul f�r den Renesas R32C 118 mit 100 Pins (R5F64189).
 *
 *  		Clock Einstellungen:
 *				-- todo --
 *
 *  			Enthaltene Peripherie:
 *  				- 6 IO Interrupts (INT0-INT5)
 *  				- 5 Timer (A0-A4)
 *  				- 9 UARTs 0-8
 *  				- 7 SPI Schnittstellen, die auf den UART Ports 0-6 liegen.
 *
 *  @version	1.00 (20.09.2011)
 *  			 - Erste Version
 *
 *	@todo Timer B0-B5 implementieren.
 ******************************************************************************/

#ifndef MCU_PC_HEADER_FIRST_INCLUDE_GUARD
#define MCU_PC_HEADER_FIRST_INCLUDE_GUARD

#define MCU_TYPE 	PC_EMU		/**< MCU Typ als PC definieren! */

#include "mcu_heap.h"

#define MCU_RTC_ALLOWS_FRACTIONS		1

/**
 * @enum MCU_IO_PIN
 * Eine Aufz�hlung aller IO Pinne der MCU.
 **/
typedef enum {
	P0_0 = 0x0001,	/**< Port 0 Pin 0 */
	P0_1 = 0x0002,	/**< Port 0 Pin 1 */
	P0_2 = 0x0004,	/**< Port 0 Pin 2 */
	P0_3 = 0x0008,	/**< Port 0 Pin 3 */
	P0_4 = 0x0010,	/**< Port 0 Pin 4 */
	P0_5 = 0x0020,	/**< Port 0 Pin 5 */
	P0_6 = 0x0040,	/**< Port 0 Pin 6 */
	P0_7 = 0x0080,	/**< Port 0 Pin 7 */

	P1_0 = 0x0101,	/**< Port 1 Pin 0 */
	P1_1 = 0x0102,	/**< Port 1 Pin 1 */
	P1_2 = 0x0104,	/**< Port 1 Pin 2 */
	P1_3 = 0x0108,	/**< Port 1 Pin 3 */
	P1_4 = 0x0110,	/**< Port 1 Pin 4 */
	P1_5 = 0x0120,	/**< Port 1 Pin 5 */
	P1_6 = 0x0140,	/**< Port 1 Pin 6 */
	P1_7 = 0x0180,	/**< Port 1 Pin 7 */

	P2_0 = 0x0201,	/**< Port 2 Pin 0 */
	P2_1 = 0x0202,	/**< Port 2 Pin 1 */

	PIN_NONE = 0xFF				/**< 	Dieser Pin dient als Pseudonym f�r IO Pins die nicht verwendet werden. Wenn ein Modul beispielsweise
										ein Signal auf einem Pin herausgeben kann, kann dies mit setzen dieses Pins verhindert werden. */
} MCU_IO_PIN;

							//	TXD	  RXD
#define MCU_UART0_INIT_PARAM	0, P0_0, P0_1			/**< UART0 TX und RX */ // DAS IST IMMER DAS PC TERMINAL!
#define MCU_UART1_INIT_PARAM	1, P0_2, P0_3			/**< UART1 TX und RX */
#define MCU_UART2_INIT_PARAM	2, P0_4, P0_5			/**< UART2 TX und RX */
#define MCU_UART3_INIT_PARAM	3, P0_6, P0_7			/**< UART3 TX und RX */
#define MCU_UART4_INIT_PARAM	4, P1_0, P1_1			/**< UART4 TX und RX */
#define MCU_UART5_INIT_PARAM	5, P1_2, P1_3			/**< UART5 TX und RX */
#define MCU_UART6_INIT_PARAM	6, P1_4, P1_5			/**< UART6 TX und RX */
#define MCU_UART7_INIT_PARAM	7, P1_6, P1_7			/**< UART7 TX und RX */
#define MCU_UART8_INIT_PARAM	8, P2_0, P2_1			/**< UART8 TX und RX */
#endif
