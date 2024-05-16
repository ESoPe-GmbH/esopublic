// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/*
 * @file base64.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "module_public.h"
#if MODULE_ENABLE_CONVERT_BASE64

#include "base64.h"
#include <string.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";                    // Encode
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";   // Decode

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void base64_encodeblock(uint8_t* in, uint8_t out[4], uint8_t len)
{
	uint8_t in_tmp[3] = {0};
	if(len == 0)
		return;

	if(len < 3)
	{
		memcpy(in_tmp, in, len);
		in = in_tmp;
	}

    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | (len > 1 ? ((in[1] & 0xf0) >> 4) : 0) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ( len == 3 ? ((in[2] & 0xc0) >> 6) : 0) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

uint32_t base64_encodebuffer(uint8_t* buf, uint8_t* out, uint32_t len)
{
	uint32_t ret;
	int32_t o1, o2;
	uint8_t bl;

	if(len == 0)
		return 0;

	if(buf == out)
		return 0;

	// Calculate total length of base64 buffer
	o2 = len / 3 * 4;

	bl = len % 3;
	if( bl == 0) // Length is multiple of three -> Start address of last block is 3 before the length
	{
		bl = 3;
		o1 = len - 3;
	}
	else // Length is not multiple of three -> Encoded buffer size has to be incremented by 4
	{
		o1 = len - bl;
		o2 += 4;
	}

	// Create a 0-termination at the end of the buffer!
	ret = o2;

	// Offset of last block to encode is length -4
	o2 -= 4;

	// Now o1 and o2 are the offsets of the last blocks
	while(o1 >= 0)
	{
		base64_encodeblock(&buf[o1], &out[o2], bl);
		bl = 3;
		o1 -= 3;
		o2 -= 4;
	}
	return ret;
}

uint32_t base64_encodebuffer_direct(uint8_t* buf, uint32_t len)
{
	uint32_t ret;
	int32_t o1, o2;
	uint8_t bl;

	if(len == 0)
		return 0;

	// Calculate total length of base64 buffer
	o2 = len / 3 * 4;

	bl = len % 3;
	if( bl == 0) // Length is multiple of three -> Start address of last block is 3 before the length
	{
		bl = 3;
		o1 = len - 3;
	}
	else // Length is not multiple of three -> Encoded buffer size has to be incremented by 4
	{
		o1 = len - bl;
		o2 += 4;
	}

	// Create a 0-termination at the end of the buffer!
//	buf[o2] = 0;
	ret = o2;

	// Offset of last block to encode is length -4
	o2 -= 4;

	// Now o1 and o2 are the offsets of the last blocks
	while(o1 >= 0)
	{
		if(o1 < 9) // o1 and o2 are overlapping! Copy data into different buffer
		{
			uint8_t dec[4];
			base64_encodeblock(&buf[o1], dec, bl);
			memcpy(&buf[o2], dec, 4);
		}
		else // o1 and o2 do not overlap, encode inside the same buffer
			base64_encodeblock(&buf[o1], &buf[o2], bl);
		bl = 3;
		o1 -= 3;
		o2 -= 4;
	}
	return ret;
}

void base64_encodestring(char* str, uint8_t* out)
{
	uint16_t i;
	uint16_t len = strlen(str);

	for(i = 0; i < len; i += 3)
	{
		uint16_t j = len - i;
		if(j > 3)
			j = 3;
		base64_encodeblock((uint8_t*)str, out, j);
		str += 3;
		out += 4;
	}
}

void base64_decodeblock(uint8_t in[4], uint8_t out[3])
{
	uint8_t v;
	uint8_t in2[4];
	uint8_t i;
	for(i=0; i<4; i++)
	{
		v = in[i];

        v = ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
        if( v )
            v = ((v == '$') ? 0 : v - 61);

        in2[i] = v?v-1:0;
	}

    out[ 0 ] = (unsigned char ) (in2[0] << 2 | in2[1] >> 4);
    out[ 1 ] = (unsigned char ) (in2[1] << 4 | in2[2] >> 2);
    out[ 2 ] = (unsigned char ) (((in2[2] << 6) & 0xc0) | in2[3]);
}

uint32_t base64_decodebuffer(uint8_t* buf, uint8_t* out, uint32_t len)
{
	uint8_t* ptr;
	uint8_t* ptr2;
	uint32_t ret = 0;

	// Len must be greater than 0 and has to be a multiple of 4!
	if(len == 0 || (len & 3) > 0)
		return 0;

	if(buf == out)
		return 0;

	uint16_t i;

	ptr = buf;
	ptr2 = out;

	ret = len / 4 * 3;
	if(buf[len - 1] == '=')
		ret--;
	if(buf[len - 2] == '=')
		ret--;

	for(i = 0; i < len; i += 4)
	{
		base64_decodeblock(ptr, ptr2);
		ptr += 4;
		ptr2 += 3;
	}

	return ret;
}

uint32_t base64_decodebuffer_direct(uint8_t* buf, uint32_t len)
{
	uint8_t* ptr;
	uint8_t* ptr2;
	uint32_t ret = 0;

	// Len must be greater than 0 and has to be a multiple of 4!
	if(len == 0 || (len & 3) > 0)
		return 0;

	uint16_t i;

	ptr = buf;
	ptr2 = buf;

	ret = len / 4 * 3;
	if(buf[len - 1] == '=')
		ret--;
	if(buf[len - 2] == '=')
		ret--;

	for(i = 0; i < len; i += 4)
	{
		if(i == 0) // Buffer are overlapping!
		{
			uint8_t out[3];
			base64_decodeblock(ptr, out);
			memcpy(ptr2, out, 3);
		}
		else // Buffer are not overlapping
			base64_decodeblock(ptr, ptr2);
		ptr += 4;
		ptr2 += 3;
	}

	return ret;
}

void base64_decodestring(char* str, uint8_t* out)
{
	uint16_t i;
	uint16_t len = strlen(str);

	for(i = 0; i < len; i += 4)
	{
		base64_decodeblock((uint8_t*)str, out);
		str += 4;
		out += 3;
	}
}

void base64_decodestring_direct(char* str)
{
	uint8_t out[3];
	uint16_t i, j;
	uint16_t len = strlen(str);

	if(len == 0)
		return;

	for(i = 0, j = 0; i < len; i += 4, j += 3)
	{
		base64_decodeblock((uint8_t*)&str[i], out);
		str[j] = out[0];
		str[j + 1] = out[1];
		str[j + 2] = out[2];
	}
	str[j + 1] = 0;
}

uint32_t base64_decode_to_uint32_t(char* b64_buf)
{
	uint32_t value = 0;
	uint8_t out[3];

	base64_decodeblock((uint8_t*)b64_buf, out);
	((uint8_t*)&value)[3] = out[0];
	((uint8_t*)&value)[2] = out[1];
	((uint8_t*)&value)[1] = out[2];
	base64_decodeblock((uint8_t*)b64_buf + 4, out);
	((uint8_t*)&value)[0] = out[0];

	return value;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif
