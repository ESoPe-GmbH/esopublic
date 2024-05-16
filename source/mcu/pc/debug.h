/**
 * @file debug.h
 * @copyright Urheberrecht 2013-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * 
 * @author Tim Koczwara
 * 
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include "mcu/mcu.h"

#if MCU_TYPE == PC_EMU

void debug_init(void);

#endif

#endif /* DEBUG_H_ */
