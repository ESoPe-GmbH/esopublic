// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/*
 * @file version.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "version.h"
#include <stdlib.h>
#include <string.h>

/// Current Software version number. Should be incremented for each release.
static uint16_t 	version_numeric = 0;

/// Stores the software version as a string
static char			version_string[7] = "0.0000";

void version_set(uint16_t version, uint8_t num_chars_major)
{
	uint8_t i;
	uint16_t d = 10000;
	version_numeric = version;
	for(i = 0; i < 6; i++)
	{
		if(i == num_chars_major)
			version_string[i] = '.';
		else
		{
			version_string[i] = '0' + ((version / d) % 10);
			d /= 10;
		}
	}
	version_string[6] = '\0';
}

uint16_t version_get_numeric(void)
{
	return version_numeric;
}

char* version_get_string(void)
{
	return version_string;
}

