/**
 * @file crc8.c
 * @copyright Urheberrecht 2018-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "crc8.h"

#include "module_public.h"
#if MODULE_ENABLE_CRC

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void crc8_init(crc8_t *c, uint8_t polynom, uint8_t initial, uint8_t final_xor, bool reversed)
{
	c->polynom = polynom;
	c->initial = initial;
	c->final_xor = final_xor;
	c->reverse = reversed;
}

uint8_t crc8_calc(crc8_t *c, const uint8_t *data, size_t data_len)
{
	int i;
	uint8_t tmp_crc = c->initial;

	for(i = 0; i < data_len; i++)
		tmp_crc = crc8_calc_byte(c, tmp_crc, data[i]);

	return tmp_crc ^ c->final_xor;
}

uint8_t crc8_calc_byte(crc8_t *c, uint8_t crc, uint8_t b)
{
	if(c->reverse)
	{
		switch(c->polynom)
			{
				// Lookup tables?

				default:
					for (uint8_t i = 8; i; i--) 
					{
						uint8_t c0 = (crc ^ b) & 0x01;
						crc >>= 1;
						if (c0)
							crc ^= c->polynom;
						b >>= 1;
					}
				break;
			}
	}
	else
	{
		switch(c->polynom)
			{
				// Lookup tables?

				default:
					for (uint8_t i = 8; i; i--) 
					{
						uint8_t c8 = (crc ^ b) & 0x80;
						crc <<= 1;
						if (c8)
							crc ^= c->polynom;
						b <<= 1;
					}
				break;
			}
	}

    return crc;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif
