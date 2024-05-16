/**
 * 	@file 		mcu_controller.c
 *	@copyright Urheberrecht 2011-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../mcu.h"
#include "mcu_internal.h"

#if MCU_TYPE == PC_EMU

/**********************
 * Interne Variablen
 *********************/

/**********************
 * Externe Variablen
 *********************/
MCU_RESULT mcu_last_error = MCU_OK;

uint32_t mcu_frq_ext_hz = 0;
uint32_t mcu_frq_pll_hz = 0;
uint32_t mcu_frq_cpu_hz = 0;
uint32_t mcu_frq_base_hz = 0;
uint32_t mcu_frq_peripheral_hz = 0;
uint32_t mcu_frq_peripheral_bus_hz = 0;

/**********************
 * Interne Prototypen
 *********************/

/**********************
 * Funktionen
 *********************/
uint32_t mcu_get_frq_external(void){ 			return mcu_frq_ext_hz; }
uint32_t mcu_get_frq_cpu(void){ 				return mcu_frq_cpu_hz; }
uint32_t mcu_get_frq_peripheral(void){ 		return mcu_frq_peripheral_hz; }

void mcu_init(uint32_t frq_ext, uint32_t frq_cpu, uint32_t frq_peripheral)
{
	/// @todo Aktuell wird immer Fix 64MHz eingestellt. Der Parameter der externen Clock ist der einzige, der benutzt wird.
}

void mcu_init_max_internal()
{
	mcu_last_error = MCU_ERROR_FRQ_EXT_INVALID;	// R32C ben�tigt externe Clock!
}

void mcu_init_max_external(uint32_t frq_ext)
{
}

MCU_RESULT mcu_init_r5f6411f(uint32_t frq_ext, uint32_t frq_pll, uint32_t div_base, uint32_t div_mcu_from_base,
								uint32_t div_peripheral_bus, uint32_t div_peripheral)
{
	return MCU_OK;
}

void mcu_enable_interrupt(void)
{
	//_asm(" FSET I");
}

void mcu_disable_interrupt(void)
{
	//_asm(" FCLR I");
}

void mcu_soft_reset(void)
{

}

/** Makro, dass ein Statement f�r alle Pins 0-7 in Abh�ngigkeit der �bergebenen Port Nummer anwendet. */
#define CASE_ALL_PIN(CASE_STATEMENT, n)	CASE_STATEMENT(n, 0);	\
										CASE_STATEMENT(n, 1);	\
										CASE_STATEMENT(n, 2);	\
										CASE_STATEMENT(n, 3);	\
										CASE_STATEMENT(n, 4);	\
										CASE_STATEMENT(n, 5);	\
										CASE_STATEMENT(n, 6);	\
										CASE_STATEMENT(n, 7)

/** Makro, dass ein Statement f�r die Pins 0-4, 6 und 7 anwendet. */
#define CASE_ALL_PIN_8(CASE_STATEMENT)	CASE_STATEMENT(8, 0);	\
										CASE_STATEMENT(8, 1);	\
										CASE_STATEMENT(8, 2);	\
										CASE_STATEMENT(8, 3);	\
										CASE_STATEMENT(8, 4);	\
										CASE_STATEMENT(8, 6);	\
										CASE_STATEMENT(8, 7)

/** Makro, dass ein Statement f�r alle Pins 1, 3-7 anwendet. */
#define CASE_ALL_PIN_9(CASE_STATEMENT)	CASE_STATEMENT(9, 1);	\
										CASE_STATEMENT(9, 3);	\
										CASE_STATEMENT(9, 4);	\
										CASE_STATEMENT(9, 5);	\
										CASE_STATEMENT(9, 6);	\
										CASE_STATEMENT(9, 7)


void mcu_io_set_port_dir(MCU_IO_PIN p, uint8_t d)
{
}

/** Direction f�r einen Port und Pin setzen. */
#define CASE_PORT_SET_DIR(n, p)		case P ## n ## _ ## p:				pd ## n ## _ ## p = d;					break
/** Direction f�r Pin auf Port 9 setzen. */
#define CASE_PORT_SET_DIR9(n, p)	case P9_ ## p: 			prc2 = 1;	pd9_ ## p = d;				prc2 = 0;	break
void mcu_io_set_dir(MCU_IO_PIN p, MCU_IO_DIRECTION d)
{
}

void mcu_io_set_pullup(MCU_IO_PIN p, bool pullup_active)
{
}

/** Einen bestimmten Portpin setzen. */
#define CASE_PORT_SET_IO(n, i)		case P ## n ## _ ## i:	p ## n ## _ ## i = d;	break
void mcu_io_set(MCU_IO_PIN p, uint8_t d)
{
}

/** Einen bestimmten Portpin lesen. */
#define CASE_PORT_GET_IO(n, i)		case P ## n ## _ ## i:	return p ## n ## _ ## i
uint8_t mcu_io_get(MCU_IO_PIN p)
{
	return 0;
}

void mcu_io_toggle(MCU_IO_PIN p)
{

}

#if MCU_PERIPHERY_ENABLE_WATCHDOG
void mcu_watchdog_init(void (*f)(void))
{
}

void mcu_watchdog_trigger(void)
{
}
#endif

#endif