/***
 * @file eve_spi.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "eve.h"
#include "eve_spi.h"
#include "eve_copro.h"
#include "module/comm/dbg.h"
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

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void eve_spi_send_host_command(eve_t* obj, EVE_HOST_CMD cmd)
{
// #if EVE_USE_FT81X
// 	mcu_spi_transaction_start(obj->hw.spi);
// 	mcu_spi_transaction_add(obj->hw.spi, (uint8_t[3]){cmd, 0, 0}, NULL, 3);
// 	mcu_spi_transaction_end(obj->hw.spi);
// #else
	mcu_spi_set_chip_select(obj->hw.spi, MCU_IO_LOW);
	mcu_spi_send(obj->hw.spi, (uint8_t)cmd);
	mcu_spi_send(obj->hw.spi, 0x00);
	mcu_spi_send(obj->hw.spi, 0x00);
	mcu_spi_set_chip_select(obj->hw.spi, MCU_IO_HIGH);
// #endif
}

uint16_t eve_spi_write_str_data(eve_t* obj, uint32_t address, char *data, bool add_padding)
{
	uint16_t cnt = 0;
	uint16_t len = (data == NULL ? 0 : strlen(data));

	if(len == 0)
		return 0;

	address &= 0x003FFFFF;	// Shrink to 22-Bit
	address |= 0x00800000;	// Set Bit 23 to 1 for write address

#if EVE_USE_FT81X
	mcu_spi_transaction_start(obj->hw.spi);
	// mcu_spi_transaction_add(obj->hw.spi, (uint8_t[3]){((uint8_t*)&address)[2], ((uint8_t*)&address)[1], ((uint8_t*)&address)[0]}, NULL, 3);
	if(data[0])
	{		
		cnt = len + 1;
		mcu_spi_transaction_add_(obj->hw.spi, .addr = address, .addr_length = 3, .w_buf = (uint8_t*)data, .w_buf_length = cnt);
		
		if(add_padding && cnt & 3)
		{
			// Add padding bytes 
			// tx_data will be initialized with 0.
			mcu_spi_transaction_add_(obj->hw.spi, .flags = MCU_SPI_TRANS_FLAGS_TXDATA, .w_buf_length = 4 - (cnt & 3));
		}
	}
	else if(add_padding)
	{
		mcu_spi_transaction_add_(obj->hw.spi, .flags = MCU_SPI_TRANS_FLAGS_TXDATA, .w_buf_length = 4);
		cnt = 4;
	}
	else
	{
		mcu_spi_transaction_add_(obj->hw.spi, .flags = MCU_SPI_TRANS_FLAGS_TXDATA, .w_buf_length = 1);
		cnt = 1;
	}
	mcu_spi_transaction_end(obj->hw.spi);
#else
	uint32_t i = 0;
	mcu_spi_set_chip_select(obj->hw.spi, MCU_IO_LOW);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[2]);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[1]);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[0]);
	for(i = 0; i < len; i++)
	{
		if(data[i] > 1)
		{
			mcu_spi_send(obj->hw.spi, data[i]);
			cnt++;
		}
	}
	mcu_spi_send(obj->hw.spi, 0);
	cnt++;
	
	mcu_spi_set_chip_select(obj->hw.spi, MCU_IO_HIGH);
#endif
	return cnt;
}

void eve_spi_write_data(eve_t* obj, uint32_t address, const uint8_t *data, uint32_t len, bool add_padding)
{
	if(len == 0 || data == NULL)
		return;

	address &= 0x003FFFFF;	// Shrink to 22-Bit
	address |= 0x00800000;	// Set Bit 23 to 1 for write address
	
	// if(len > 4)
	// {
	// 	DBG_INFO("eve_spi_write_data %06x %u %B\n", address, len, add_padding);
	// }
#if EVE_USE_FT81X
	uint32_t offset = 0;
	mcu_spi_transaction_start(obj->hw.spi);
	// mcu_spi_transaction_add(obj->hw.spi, (uint8_t[3]){((uint8_t*)&address)[2], ((uint8_t*)&address)[1], ((uint8_t*)&address)[0]}, NULL, 3);
	mcu_spi_transaction_add_(obj->hw.spi, .addr = address, .addr_length = 3);
	do
	{
		uint32_t len_a = len - offset;
		if(len_a > 4000)
			len_a = 4000;
			
		// mcu_spi_transaction_add(obj->hw.spi, (uint8_t*)&data[offset], NULL, len_a);	
		mcu_spi_transaction_add_(obj->hw.spi, .w_buf = (uint8_t*)&data[offset], .w_buf_length = len_a);
		offset += len_a;
	}while(offset < len);
	if(add_padding && len & 3)
	{
		// Add padding bytes 
		mcu_spi_transaction_add_(obj->hw.spi, .flags = MCU_SPI_TRANS_FLAGS_TXDATA, .w_buf_length = 4 - (len & 3));
	}
	mcu_spi_transaction_end(obj->hw.spi);
#else		
	uint32_t i = 0;
	mcu_spi_set_chip_select(obj->hw.spi, MCU_IO_LOW);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[2]);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[1]);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[0]);
	for(i = 0; i < len; i++)
		mcu_spi_send(obj->hw.spi, data[i]);
	mcu_spi_set_chip_select(obj->hw.spi, MCU_IO_HIGH);
#endif
	// if(len > 4)
	// {
	// 	DBG_INFO("FIN eve_spi_write_data %06x %u %B\n", address, len, add_padding);
	// }
}

void eve_spi_write_multi_data(eve_t* obj, uint32_t address, const uint8_t **data, uint32_t *len, uint32_t num_dataset)
{
	if(len == 0 || data == NULL)
		return;

	address &= 0x003FFFFF;	// Shrink to 22-Bit
	address |= 0x00800000;	// Set Bit 23 to 1 for write address
#if EVE_USE_FT81X
	mcu_spi_transaction_start(obj->hw.spi);
	// mcu_spi_transaction_add(obj->hw.spi, (uint8_t[3]){((uint8_t*)&address)[2], ((uint8_t*)&address)[1], ((uint8_t*)&address)[0]}, NULL, 3);
	for(uint32_t k = 0; k < num_dataset; k++)
	{
		uint32_t offset = 0;
		do
		{
			uint32_t len_a = len[k] - offset;
			if(len_a > 4000)
				len_a = 4000;
				
			// mcu_spi_transaction_add(obj->hw.spi, (uint8_t*)&data[offset], NULL, len_a);	
			mcu_spi_transaction_t t = {
				.w_buf = (uint8_t*)&data[k][offset], 
				.w_buf_length = len_a
			};
			if(k == 0 && offset == 0)
			{
				t.addr = address;
				t.addr_length = 3;
			}
			mcu_spi_transaction_add(obj->hw.spi, t);

			offset += len_a;
		}while(offset < len[k]);
	}
	mcu_spi_transaction_end(obj->hw.spi);
#else		
	uint32_t i = 0;
	mcu_spi_set_chip_select(obj->hw.spi, MCU_IO_LOW);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[2]);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[1]);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[0]);
	// DBG_INFO("Write Multi to %06x\n", address);
	for(uint32_t k = 0; k < num_dataset; k++)
	{
		// DBG_INFO("[%d] = %d | %08x\n", k, len[k], data[k]);
		// Maybe a data buffer inside the array is optional and marked as NULL or length 0. Skip it then.
		if(data[k] && len[k] > 0)
		{
			// DBG_INFO("[%d] = %#A\n", k, len[k], data[k]);
			for(i = 0; i < len[k]; i++)
				mcu_spi_send(obj->hw.spi, data[k][i]);
		}
	}
	mcu_spi_set_chip_select(obj->hw.spi, MCU_IO_HIGH);
#endif
}

void eve_spi_write_8(eve_t* obj, uint32_t address, uint8_t value)
{
#if EVE_USE_FT81X
	address &= 0x003FFFFF;	// Shrink to 22-Bit
	address |= 0x00800000;	// Set Bit 23 to 1 for write address
	mcu_spi_transaction_start(obj->hw.spi);

	mcu_spi_transaction_t t = {
		.flags = MCU_SPI_TRANS_FLAGS_TXDATA,
		.addr = address,
		.addr_length = 3,
		.w_data = {value, 0, 0, 0},
		.w_buf_length = 1
	};
	mcu_spi_transaction_add(obj->hw.spi, t);	
	mcu_spi_transaction_end(obj->hw.spi);
#else
	eve_spi_write_data(obj, address, (uint8_t*)&value, 1, false);
#endif
}

void eve_spi_write_16(eve_t* obj, uint32_t address, uint16_t value)
{
#if EVE_USE_FT81X
	address &= 0x003FFFFF;	// Shrink to 22-Bit
	address |= 0x00800000;	// Set Bit 23 to 1 for write address
	mcu_spi_transaction_start(obj->hw.spi);

	mcu_spi_transaction_t t = {
		.flags = MCU_SPI_TRANS_FLAGS_TXDATA,
		.addr = address,
		.addr_length = 3,
		.w_data = {value & 0xFF, (value >> 8) & 0xFF, 0, 0},
		.w_buf_length = 2
	};
	mcu_spi_transaction_add(obj->hw.spi, t);	
	mcu_spi_transaction_end(obj->hw.spi);
#else
	eve_spi_write_data(obj, address, (uint8_t*)&value, 2, false);
#endif
}

void eve_spi_write_32(eve_t* obj, uint32_t address, uint32_t value)
{
#if EVE_USE_FT81X
	address &= 0x003FFFFF;	// Shrink to 22-Bit
	address |= 0x00800000;	// Set Bit 23 to 1 for write address
	mcu_spi_transaction_start(obj->hw.spi);

	mcu_spi_transaction_t t = {
		.flags = MCU_SPI_TRANS_FLAGS_TXDATA,
		.addr = address,
		.addr_length = 3,
		.w_buf_length = 4,
		// .w_buf = &value
	};
	memcpy(t.w_data, &value, 4);
	mcu_spi_transaction_add(obj->hw.spi, t);	
	mcu_spi_transaction_end(obj->hw.spi);
#else
	eve_spi_write_data(obj, address, (uint8_t*)&value, 4, false);
#endif
}

void eve_spi_read_data(eve_t* obj, uint32_t address, uint8_t *data, uint16_t len)
{
	address &= 0x003FFFFF;	// Shrink to 22-Bit
#if EVE_USE_FT81X
	// DBG_INFO("Read %06x: %u\n", address, len);
	uint32_t offset = 0;
	mcu_spi_transaction_start(obj->hw.spi);
	mcu_spi_transaction_add_(obj->hw.spi, .addr = address, .addr_length = 3, .dummy_length = 1);
	// mcu_spi_transaction_add(obj->hw.spi, (uint8_t[3]){((uint8_t*)&address)[2], ((uint8_t*)&address)[1], ((uint8_t*)&address)[0]}, NULL, 3);
	do
	{
		uint32_t len_a = len - offset;
		if(len_a > 4000)
			len_a = 4000;
			
		mcu_spi_transaction_add_(obj->hw.spi, .r_buf = (uint8_t*)&data[offset], .r_buf_length = len_a);
		offset += len_a;
	}while(offset < len);
	mcu_spi_transaction_end(obj->hw.spi);
#else		
	uint32_t i = 0;
	mcu_spi_set_chip_select(obj->hw.spi, MCU_IO_LOW);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[2]);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[1]);
	mcu_spi_send(obj->hw.spi, ((uint8_t*)&address)[0]);
	mcu_spi_send(obj->hw.spi, 0xFF);
	for(i = 0; i < len; i++)
		data[i] = mcu_spi_send(obj->hw.spi, 0xFF);
	mcu_spi_set_chip_select(obj->hw.spi, MCU_IO_HIGH);
#endif
}

uint8_t eve_spi_read_8(eve_t* obj, uint32_t address)
{
	uint8_t value = 0;
#if EVE_USE_FT81X
	address &= 0x003FFFFF;	// Shrink to 22-Bit
	mcu_spi_transaction_start(obj->hw.spi);

	mcu_spi_transaction_t t = {
		.addr = address,
		.addr_length = 3,
		.dummy_length = 1,
		.r_buf = &value,
		.r_buf_length = 1
	};
	mcu_spi_transaction_add(obj->hw.spi, t);	
	mcu_spi_transaction_end(obj->hw.spi);
	
	// DBG_INFO("Read 8-Bit from %06x: %02x\n", address, value);
#else
	eve_spi_read_data(obj, address, (uint8_t*)&value, 1);
#endif
	return value;
}

uint16_t eve_spi_read_16(eve_t* obj, uint32_t address)
{
	uint16_t value = 0;
#if EVE_USE_FT81X
	address &= 0x003FFFFF;	// Shrink to 22-Bit
	mcu_spi_transaction_start(obj->hw.spi);

	mcu_spi_transaction_t t = {
		.addr = address,
		.addr_length = 3,
		.dummy_length = 1,
		.r_buf_length = 2,
		.r_buf = &value
	};
	mcu_spi_transaction_add(obj->hw.spi, t);	
	mcu_spi_transaction_end(obj->hw.spi);
#else
	eve_spi_read_data(obj, address, (uint8_t*)&value, 2);
#endif
	return value;
}

uint16_t eve_spi_read_16_twice(eve_t* obj, uint32_t address)
{
	uint16_t value = 0;
	uint16_t value2 = 0;
	value = eve_spi_read_16(obj, address);
	do
	{
		value2 = eve_spi_read_16(obj, address);
		if(value == value2)
			return value;
		value = value2;
	}while(true);
}

uint32_t eve_spi_read_32(eve_t* obj, uint32_t address)
{
	uint32_t value = 0;
#if EVE_USE_FT81X
	address &= 0x003FFFFF;	// Shrink to 22-Bit
	mcu_spi_transaction_start(obj->hw.spi);

	mcu_spi_transaction_t t = {
		.addr = address,
		.addr_length = 3,
		.dummy_length = 1,
		.r_buf_length = 4,
		.r_buf = &value
	};
	mcu_spi_transaction_add(obj->hw.spi, t);	
	mcu_spi_transaction_end(obj->hw.spi);
#else
	eve_spi_read_data(obj, address, (uint8_t*)&value, 4);
#endif
	return value;
}

#endif
