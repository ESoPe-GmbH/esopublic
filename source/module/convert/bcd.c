// Urheberrecht 2018-2024 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file swap.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "module_public.h"
#if MODULE_ENABLE_CONVERT_BCD

#include "bcd.h"


static uint8_t bcd_encode_uint8(uint8_t value)
{
	return((value / 10 * 0x10) + (value % 10));
}

static uint8_t bcd_decode_uint8(uint8_t value)
{
	return ((value / 0x10 * 10) + (value & 0x0F));
}

#endif
