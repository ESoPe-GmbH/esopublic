/***
 * @file font.c
 * @copyright Urheberrecht 2017-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "font.h"

#include "../eve/eve.h"
#include "../eve/eve_spi.h"
#include "../eve/eve_copro.h"
#include "../eve/eve_memory.h"
#include "screen.h"
#include "module/comm/dbg.h"
#include "module/convert/string.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if EVE_ENABLE_FONT16
static const uint8_t _font_16[] = {
#include "fonts/font_16.h"
};
#endif
#if EVE_ENABLE_FONT17
static const uint8_t _font_17[] = {
#include "fonts/font_17.h"
};
#endif
#if EVE_ENABLE_FONT18
static const uint8_t _font_18[] = {
#include "fonts/font_18.h"
};
#endif
#if EVE_ENABLE_FONT19
static const uint8_t _font_19[] = {
#include "fonts/font_19.h"
};
#endif
#if EVE_ENABLE_FONT20
static const uint8_t _font_20[] = {
#include "fonts/font_20.h"
};
#endif
#if EVE_ENABLE_FONT21
static const uint8_t _font_21[] = {
#include "fonts/font_21.h"
};
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define _INIT_FONT(n)	font_init_from_flash(eve, n, _font_ ## n, sizeof(_font_ ## n), 0x18);

void font_initalize_default_fonts(eve_t* eve)
{
	DBG_INFO("Font initialize defaults\n");
	uint8_t i;

	for(i = 0; i < 16; i++)
	{
		eve->memory.font[i].height = 0;
	}

	eve->memory.metric_address = eve_spi_read_32(eve, EVE_ROM_FONT_ADDR);
//	dbg_printf(DBG_STRING, "Metric Address %08x\n", eve->memory.metric_address);
	for(i = 0; i < 16; i++)
		eve->memory.font[i].height = eve_spi_read_32(eve, eve->memory.metric_address + (148 * i + 140));
	// for(i = 16; i < 32; i++)
	// 	eve->memory.font[i].height = eve_spi_read_32(eve, eve->memory.metric_address + (148 * (i-16)) + 140);

#if EVE_ENABLE_FONT16
	_INIT_FONT(16);
#endif
#if EVE_ENABLE_FONT17
	_INIT_FONT(17);
#endif
#if EVE_ENABLE_FONT18
	_INIT_FONT(18);
#endif
#if EVE_ENABLE_FONT19
	_INIT_FONT(19);
#endif
#if EVE_ENABLE_FONT20
	_INIT_FONT(20);
#endif
#if EVE_ENABLE_FONT21
	_INIT_FONT(21);
#endif

//	dbg_printf(DBG_STRING, "%d bytes in use\n", eve->memory.ram_pointer);
//
//	for(i = 0; i < 32; i++)
//		dbg_printf(DBG_STRING, "Font %d Height %d\n",i, eve->memory.font[i].height);
}

bool font_init_from_flash(eve_t* eve, uint8_t num, const uint8_t* data, uint32_t data_len, uint8_t first_character)
{
//	int i;
	uint32_t address_metric, address_font;

	if(data_len == 0)
		return false;

	address_metric = eve_memory_get_address(eve, data_len);
	if(address_metric == 0xFFFFFFFF) // Not enough space!
		return false;

	address_font = address_metric + 148;//address_metric + 148 - ((*(uint32_t*)&metric[132]) * (*(uint32_t*)&metric[140])) * 1;

	eve_memory_write_to(eve, address_metric, (uint8_t*)data, 144);
	eve_memory_write_to(eve, address_metric + 144, (uint8_t*)&address_font, 4);
	eve_memory_write_to(eve, address_metric + 148, (uint8_t*)&data[148], data_len);

//	dbg_printf(DBG_STRING, "Font %d: F=%d S=%d W=%d H=%d Addr=%d\n", num, *(uint32_t*)&data[128], *(uint32_t*)&data[132],
//			*(uint32_t*)&data[136], *(uint32_t*)&data[140], *(uint32_t*)&data[144]);
//	dbg_printf(DBG_STRING, "Font %d:\n", num);
//	for(i = 0x1B; i <= 0x7E; i++)
//	{
//		if(i == '0' || i == 'a' || i == 'A')
//			comm_putc(COMM_DEBUG, '\n');
//		comm_printf(COMM_DEBUG, "'%c'=%d ", i, data[i]);
//	}
//	comm_putc(COMM_DEBUG, '\n');

//	eve_copro_dlstart(eve);
//	eve_copro_write_command(eve, EVE_CLEAR(1,1,1));
//	eve_copro_write_command(eve, EVE_CLEAR_COLOR_RGB(0xff, 0xff, 0xff));
//	eve_copro_write_command(eve, EVE_BITMAP_HANDLE(num)); // associate font to font handle
//	eve_copro_write_command(eve, EVE_BITMAP_SOURCE(address_font));
//	// Set Layout format, stride and height -> Take values from metric
//	eve_copro_write_command(eve, EVE_BITMAP_LAYOUT_H(0,0));
//	eve_copro_write_command(eve, EVE_BITMAP_LAYOUT(
//			*(uint32_t*)&data[128], // Format
//			*(uint32_t*)&data[132], // Stride
//			*(uint32_t*)&data[140]) // Height
//			);
//	// Set Layout width and height -> Take values from metric
//	eve_copro_write_command(eve, EVE_BITMAP_SIZE_H(0,0));
//	eve_copro_write_command(eve, EVE_BITMAP_SIZE(EVE_FILTER_NEAREST, EVE_WRAP_BORDER, EVE_WRAP_BORDER,
//			*(uint32_t*)&data[136], // Width
//			*(uint32_t*)&data[140]) // Height
//			);
//
//	eve_copro_set_font2(eve, num, address_metric, first_character);
//
//	eve_copro_write_command(eve, EVE_DISPLAY());
//	eve_copro_swap(eve);

	eve->memory.font[num].is_ram = true;
	eve->memory.font[num].address = address_metric;
	eve->memory.font[num].data = data;
	eve->memory.font[num].first_character = first_character;
	eve->memory.font[num].height = *(uint32_t*)&data[140];

	return true; // TODO: Check addresses
}

uint16_t font_get_height(eve_t* eve, uint8_t font)
{
	if(font < 32)
		return eve->memory.font[font].height;

	return 0;
}

uint16_t font_get_width(eve_t* eve, uint8_t font, char letter)
{
	if(letter > 127)
		return 0;

	if(font < 32)
	{
		switch(font)
		{
#if EVE_ENABLE_FONT16
			case 16:
				return _font_16[(int)letter];
#endif
#if EVE_ENABLE_FONT17
			case 17:
				return _font_17[(int)letter];
#endif
#if EVE_ENABLE_FONT18
			case 18:
				return _font_18[(int)letter];
#endif
#if EVE_ENABLE_FONT19
			case 19:
				return _font_19[(int)letter];
#endif
#if EVE_ENABLE_FONT20
			case 20:
				return _font_20[(int)letter];
#endif
#if EVE_ENABLE_FONT21
			case 21:
				return _font_21[(int)letter];
#endif
			default:
				if(font > 15)
					return eve_spi_read_8(eve, eve->memory.metric_address + (148 * (font-16)) + letter); // TODO: Place arrays with width in RAM.
		}
	}

	return 0;
}

uint16_t font_get_width_string(eve_t* eve, uint8_t font, const char* str)
{
	if(str == NULL)
		return 0;
		
	uint16_t w = 0;
	uint16_t w_max = 0;
	while(*str)
	{
		if(*str == '\n')
		{
			str++;
			if(w > w_max)
			{
				w_max = w;
				w = 0;
			}
		}
		else
		{
			w += font_get_width(eve, font, *str++);
		}
	}
	if(w > w_max)
		w_max = w;
		
	return w_max;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif
