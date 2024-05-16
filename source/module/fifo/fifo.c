// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 	fifo.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */


#include "module_public.h"
#if MODULE_ENABLE_FIFO

#include "fifo.h"
#include <string.h>

//------------------------------------------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------------------------------------------
#if FIFO_USE_MEDIAN
/**
 * @brief 		Calculates the median for a fifo with an element size of 1.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					Median value of all fifo elements.
 */
static uint8_t fifo_get_median8(fifo_t *bs);

/**
 * @brief 		Calculates the median for a fifo with an element size of 2.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					Median value of all fifo elements.
 */
static uint16_t fifo_get_median16(fifo_t *bs);

/**
 * @brief 		Calculates the median for a fifo with an element size of 4.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					Median value of all fifo elements.
 */
static uint32_t fifo_get_median32(fifo_t *bs);
#endif

//------------------------------------------------------------------------------------------------------------
// External functions
//------------------------------------------------------------------------------------------------------------

FIFO_RESULT fifo_init(fifo_t *bs, uint8_t elementsize, void *buf, uint16_t total_elements)
{
	uint32_t tmp = elementsize * total_elements;
	if(elementsize<1)	return FIFO_ELEMENTSIZE_INVALID;
	if(tmp>=65536)		return FIFO_BUFFERSIZE_INVALID;
	bs->data = (uint8_t*)buf;
	bs->max_elements = total_elements;
	bs->element_size = elementsize;
	bs->max_len = total_elements * elementsize;
	fifo_clear(bs);
	return FIFO_OK;
}

bool fifo_put(fifo_t *bs, uint8_t* c)
{
//	mcu_disable_interrupt();
//	if(bs->entries<bs->max_elements)
	if((bs->write_pos+bs->element_size)%bs->max_len != bs->read_pos)
	{
		memcpy(bs->data+bs->write_pos, c, bs->element_size);
		bs->write_pos = (bs->write_pos+bs->element_size)%bs->max_len;
//		(bs->entries)++;
//		mcu_enable_interrupt();
		return true;
	}
//	mcu_enable_interrupt();
	return false;
}

bool fifo_put8(fifo_t*bs, uint8_t c)
{
	return fifo_put(bs, (uint8_t*)&c);
}

bool fifo_put16(fifo_t* bs, uint16_t c)
{
	return fifo_put(bs, (uint8_t*)&c);
}

bool fifo_put32(fifo_t*bs, uint32_t c)
{
	return fifo_put(bs, (uint8_t*)&c);
}

uint8_t fifo_get8(fifo_t* bs)
{
	uint8_t c = 0;
	fifo_get(bs, (uint8_t*)&c);
	return c;
}

uint16_t fifo_get16(fifo_t* bs)
{
	uint16_t c = 0;
	fifo_get(bs, (uint8_t*)&c);
	return c;
}

uint32_t fifo_get32(fifo_t* bs)
{
	uint32_t c = 0;
	fifo_get(bs, (uint8_t*)&c);
	return c;
}

uint16_t fifo_data_available(fifo_t *bs)
{
	uint16_t w = bs->write_pos;
	uint16_t r = bs->read_pos;
//	return bs->entries;
	if(w == r)
		return 0;
	else if(w > r)
		return (w - r) / bs->element_size;
	else
		return (bs->max_elements + w -  r) / bs->element_size;
}

uint16_t fifo_freespace(fifo_t* bs)
{
	return (bs->max_len / bs->element_size) - fifo_data_available(bs) - 1;
}

bool fifo_is_full(fifo_t *bs)
{
//	return (bs->entries>=bs->max_elements);
	return ((bs->write_pos+bs->element_size)%bs->max_len == bs->read_pos);
}

bool fifo_get(fifo_t *bs, uint8_t* c)
{
//	mcu_disable_interrupt();
//	if(bs->entries>0)
	if(bs->write_pos != bs->read_pos)
	{
		memcpy(c, bs->data+bs->read_pos, bs->element_size);
		bs->read_pos = (bs->read_pos+bs->element_size)%bs->max_len;
//		(bs->entries)--;
//		mcu_enable_interrupt();
		return true;
	}
//	mcu_enable_interrupt();
	return false;
}

uint8_t* fifo_get_ptr(fifo_t* bs)
{
	uint8_t *ptr;

//	if(bs->entries == 0)
	if(bs->write_pos == bs->read_pos)
		return NULL;

//	mcu_disable_interrupt();

	ptr = bs->data+bs->read_pos;
	bs->read_pos = (bs->read_pos+bs->element_size)%bs->max_len;
//	(bs->entries)--;

//	mcu_enable_interrupt();

	return ptr;
}

bool fifo_contains(fifo_t* bs, uint8_t* element, uint8_t len)
{
	uint16_t r_pos = bs->read_pos;
	if(bs == NULL || bs->write_pos == bs->read_pos) // Null or empty -> Not in list
		return false;

	if(len > bs->element_size)
		len = bs->element_size;

	while(bs->write_pos != r_pos)
	{
		if(memcmp(bs->data+r_pos, element, len) == 0)
			return true;
		r_pos = (r_pos+bs->element_size) % bs->max_len;
	}

	return false;
}

void fifo_clear(fifo_t *bs)
{
	bs->read_pos = 0;
	bs->write_pos = 0;
//	bs->entries = 0;
}

#if FIFO_USE_AVERAGE
uint32_t fifo_get_average(fifo_t *bs)
{
	uint32_t c = 0;
	uint32_t average = 0;
	uint16_t len = bs->entries;
	while(fifo_get(bs, (uint8_t*)&c))
		average+=c;
	average/=len;
	return average;
}
#endif

uint32_t fifo_get_maximum(fifo_t *bs)
{
	uint32_t c = 0;
	uint32_t max = 0;
	while(fifo_get(bs, (uint8_t*)&c))
	{
		if(c > max)
			max = c;
	}
	return max;
}

#if FIFO_USE_MEDIAN
uint32_t fifo_get_median(fifo_t *bs)
{
	switch(bs->element_size)
	{
		case 1:		return fifo_get_median8(bs);
		case 2:		return fifo_get_median16(bs);
		case 4:		return fifo_get_median32(bs);
		default: 	return 0;
	}
}
#endif

//------------------------------------------------------------------------------------------------------------
// Internal functions
//------------------------------------------------------------------------------------------------------------

#if FIFO_USE_MEDIAN
static uint8_t fifo_get_median8(fifo_t *bs)
{
	uint8_t c = 0;
	uint8_t median = 0;
	uint8_t len = bs->max_elements;
	uint8_t i, j;
	uint8_t* b = (uint8_t*)bs->data;
	if(len==0)	return 0;
	if(len<=2)	return b[0];
	for(i=0; i<len-1; i++)
	{
		for(j=i+1; j<len; j++)
		{
			if(b[i]<b[j])
			{
				c = b[i];
				b[i] = b[j];
				b[j] = c;
			}
		}
	}
	median = b[len/2];
	fifo_clear(bs);
	return median;
}

static uint16_t fifo_get_median16(fifo_t *bs)
{
	uint16_t c = 0;
	uint16_t median = 0;
	uint16_t len = bs->max_elements;
	uint16_t i, j;
	uint16_t* b = (uint16_t*)bs->data;
	if(len==0)	return 0;
	if(len<=2)	return b[0];
	for(i=0; i<len-1; i++)
	{
		for(j=i+1; j<len; j++)
		{
			if(b[i]<b[j])
			{
				c = b[i];
				b[i] = b[j];
				b[j] = c;
			}
		}
	}
	median = b[len/2];
	fifo_clear(bs);
	return median;
}

static uint32_t fifo_get_median32(fifo_t *bs)
{
	uint32_t c = 0;
	uint32_t median = 0;
	uint32_t len = bs->max_elements;
	uint32_t i, j;
	uint32_t* b = (uint32_t*)bs->data;
	if(len==0)	return 0;
	if(len<=2)	return b[0];
	for(i=0; i<len-1; i++)
	{
		for(j=i+1; j<len; j++)
		{
			if(b[i]<b[j])
			{
				c = b[i];
				b[i] = b[j];
				b[j] = c;
			}
		}
	}
	median = b[len/2];
	fifo_clear(bs);
	return median;
}
#endif

#endif
