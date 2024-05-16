// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file swap.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "module_public.h"
#if MODULE_ENABLE_CONVERT_SWAP

#include "swap.h"


uint16_t swap16(uint16_t x)
{
    return (((x)>>8) | ((x)<<8));
}

uint32_t swap32(uint32_t x)
{
    return (((x)>>24) |
        (((x)<<8) & 0x00FF0000) |
        (((x)>>8) & 0x0000FF00) |
        ((x)<<24));
}

uint64_t swap64(uint64_t x)
{
    return (uint64_t)swap32(x >> 32) | (uint64_t)swap32(x & 0xFFFFFFFF) << 32;
}

float swapf(float x)
{
    float f = x;
    swapf_ptr(&f);
    return f;
}

void swapf_ptr(float* ptr)
{
    uint8_t* v = (uint8_t*)ptr;
    *(uint32_t*)v = (((uint32_t)v[0]) << 24) | (((uint32_t)v[1]) << 16) | (((uint32_t)v[2]) << 8) | (uint32_t)v[3]; 
}

#endif
