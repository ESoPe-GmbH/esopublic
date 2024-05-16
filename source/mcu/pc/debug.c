/**
 * @file debug.c
 * @copyright Urheberrecht 2013-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
*/

#include "debug.h"

#if MCU_TYPE == PC_EMU

#include <stdio.h>
#include <stdint.h>
#include "module/comm/dbg.h"
#include "module/comm/comm.h"

comm_interface_t comm_interface_debug;
comm_t comm_handler_debug;

static void debug_putc(void * v, int c);
static void debug_puts(void * v, uint8_t *b, uint16_t l);

void debug_init(void)
{
	comm_init_interface(&comm_interface_debug);
	comm_init_handler(&comm_handler_debug);

	comm_interface_debug.xputc = debug_putc;
	comm_interface_debug.xputs = debug_puts;
	comm_interface_debug.flush = (comm_flush_t)fflush;

	comm_handler_debug.interface = &comm_interface_debug;
	comm_handler_debug.device_handler = stdout;

	dbg_set_comm(&comm_handler_debug);
}

static void debug_putc(void * v, int c)
{
	putc(c, v);
}

static void debug_puts(void * v, uint8_t *b, uint16_t l)
{
	while(l--)
		debug_putc(v, (int)*b++);
}

#endif