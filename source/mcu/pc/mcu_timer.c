/**
 * 	@file 		mcu_timer.c
 *  @copyright Urheberrecht 2018-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../mcu.h"
#include "mcu_internal.h"

#if MCU_TYPE == PC_EMU
#if  MCU_PERIPHERY_DEVICE_COUNT_TIMER>0

//uint8_t mcu_timer_divider[2] = {1, 8};
//
//
//mcu_timer_handler_ctx *mcu_timer_handler_hash[MCU_TIMER_TOTAL_COUNT] = {NULL, NULL, NULL, NULL, NULL};	// 5 Timer A0 - A4
//uint8_t mcu_current_timer_handler = 0;
//mcu_timer_handler_ctx mcu_timer_handler[MCU_PERIPHERY_DEVICE_COUNT_TIMER];
//
//#define MCU_TIMER_SET(n)	MCU_TIMER_INIT_HANDLER(handle, n)

mcu_timer_t mcu_timer_init(MCU_INT_LVL lvl, uint32_t frq_hz, void (*f)(void*), void* obj, bool auto_start)
{
//	uint8_t i, divider;
//	mcu_timer_handler_ctx* handle;
//	uint32_t timer_value = (mcu_frq_peripheral_hz/frq_hz);						// Berechnung des Z�hlers f�r den Timer
//	if(mcu_current_timer_handler>=MCU_PERIPHERY_DEVICE_COUNT_TIMER)		MCU_RETURN_ERROR_GLB(MCU_ERROR_TMR_NOT_AVAILABLE, NULL);
//	handle = &mcu_timer_handler[mcu_current_timer_handler];	// F�r einfachere Handhabung
//
//	for(i=0; i<sizeof(mcu_timer_divider); i++)
//	{
//		divider = i;	// Teiler einstellen
//		timer_value = mcu_frq_peripheral_hz / (frq_hz * ((uint32_t)mcu_timer_divider[divider]));	// Mit Teiler berechnen
//		if(timer_value<0xFFFF)	// Pr�fern obs passt...
//			break;	// Wert passt in das Register, also abbrechen!
//	}
//
//	if(timer_value>0x10000)	MCU_RETURN_ERROR(handle, MCU_ERROR_TMR_FRQ_INVALID, NULL);	// Da der Timer nur 16 Bit ist, darf der Wert nicht h�her sein! Beim einsetzen wird noch -1 gerechnet.
//
///*	switch(mcu_current_timer_handler)
//	{
//		case 0:		MCU_TIMER_SET(0);	break;
//		case 1:		MCU_TIMER_SET(1);	break;
//		case 2:		MCU_TIMER_SET(2);	break;
//		case 3:		MCU_TIMER_SET(3);	break;
//		case 4:		MCU_TIMER_SET(4);	break;
//		default: 				MCU_RETURN_ERROR(handle, MCU_ERROR_TMR_NOT_AVAILABLE, NULL);
//	}*/
//
//	handle->frq = mcu_frq_peripheral_hz/(timer_value*((uint32_t)mcu_timer_divider[divider]));
//
//	if(auto_start)	mcu_timer_start(handle);
//
//	mcu_current_timer_handler++;
//
//	return (mcu_timer_t)handle;
	return NULL;
}

static void mcu_timer_set_start(mcu_timer_t h, uint8_t state)
{
	if(h==NULL)	return;
	//if(state)	tabsr |= ((mcu_timer_handler_ctx*)h)->tabsr_bit;	// Bit setzen
	//else		tabsr &= ~((mcu_timer_handler_ctx*)h)->tabsr_bit;	// Bit l�schen
}

void mcu_timer_start(mcu_timer_t h){	mcu_timer_set_start(h, 1);	}
void mcu_timer_stop(mcu_timer_t h){	mcu_timer_set_start(h, 0);	}

uint32_t mcu_timer_get_frq(mcu_timer_t h)
{
//	if(h==NULL)	return 0;
//	return ((mcu_timer_handler_ctx*)h)->frq;
	return 0;
}
//void mcu_timer_interrupt_a0(void){	MCU_TIMER_INTERRUPT_N(0);	}
//void mcu_timer_interrupt_a1(void){	MCU_TIMER_INTERRUPT_N(1);	}
//void mcu_timer_interrupt_a2(void){	MCU_TIMER_INTERRUPT_N(2);	}
//void mcu_timer_interrupt_a3(void){	MCU_TIMER_INTERRUPT_N(3);	}
//void mcu_timer_interrupt_a4(void){	MCU_TIMER_INTERRUPT_N(4);	}

#endif

#endif
