// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file mcu_flash.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/
#include <math.h>
#include "../../mcu.h"
#include <string.h>
#include "../esp32/mcu_internal.h"

#if MCU_TYPE == MCU_STM32F3 && MCU_PERIPHERY_ENABLE_FLASH

#define MCU_FLASH_BUFFER_RAM_SIZE			256					///< Size of the buffer

extern uint32_t mcu_frq_flash_hz;

uint8_t mcu_flash_ram_buf[MCU_FLASH_BUFFER_RAM_SIZE];
uint32_t mcu_flash_ram_cnt = 0;

uint32_t mcu_flash_error_code = 0;

// Addresses
#define MCU_FLASH_ADR_DF			0x00100000
#if MCU_PERIPHERY_ENABLE_CODE_FLASH
#if MCU_CONTROLLER_ROM_SIZE_KBYTE==4096
	#define MCU_FLASH_ADR_ROM_WRITE		0xFFC00000
	#define MCU_FLASH_ADR_ROM_READ		0xFFC00000
#else
	#error "Flash implemented for size defined in MCU_CONTROLLER_ROM_SIZE_KBYTE"
#endif
#endif
#define MCU_FLASH_ADR_RAM			0x007F8000
#define FCU_FLASH_ADR_FCU			0xFEFFF000

// Timings
//#define MCU_FLASH_US_FACTOR						((60.0/(mcu_frq_flash_hz/1000000))*(mcu_frq_cpu_hz/1000000))
//#define MCU_FLASH_US_FACTOR						(((double)mcu_frq_cpu_hz/(double)mcu_frq_flash_hz)/5)
//#define MCU_FLASH_US_FACTOR						(mcu_frq_cpu_hz/1000000)
#define MCU_FLASH_US_FACTOR							360

#define MCU_FLASH_MAX_WAIT_DBFULL				((uint32_t)(90 * MCU_FLASH_US_FACTOR))
#define MCU_FLASH_MAX_WAIT_ERASE				((uint32_t)(8000 * MCU_FLASH_US_FACTOR))
#define MCU_FLASH_MAX_WAIT_NOTIFY_PERIPHERAL	((uint32_t)(60 * MCU_FLASH_US_FACTOR))
#define MCU_FLASH_MAX_WAIT_ROM_WRITE			((uint32_t)(6000 * MCU_FLASH_US_FACTOR))
#define MCU_FLASH_MAX_WAIT_DF_WRITE				((uint32_t)(1700 * MCU_FLASH_US_FACTOR))
#define MCU_FLASH_MAX_WAIT_BLANK_CHECK			((uint32_t)(30 * MCU_FLASH_US_FACTOR))
#define MCU_FLASH_MAX_WAIT_FCU_RESET			((uint32_t)(35 * MCU_FLASH_US_FACTOR))
#define MCU_FLASH_MAX_WAIT_NORMAL_TRANSITION	((uint32_t)(120 * MCU_FLASH_US_FACTOR))

#ifndef __GNUC_PATCHLEVEL__
typedef __evenaccess volatile uint8_t  * FCU_BYTE_PTR;
#else
typedef volatile uint8_t  * FCU_BYTE_PTR;
#endif

FCU_BYTE_PTR mcu_flash_cmd_ptr = (uint8_t*)0x007E0000;

//#define FACI_CMD(cmd)				*((volatile uint8_t*)0x007E0000) = (uint8_t)(cmd);
//#define FACI_WORD(w)				*((volatile uint16_t*)0x007E0000) = (uint16_t)(w);
#define FACI_CMD(cmd)				*mcu_flash_cmd_ptr = (uint8_t)(cmd);
#define FACI_WORD(w)				*((uint16_t*)mcu_flash_cmd_ptr) = (uint16_t)(w);

// Macros
#define MCU_FLASH_INITIALIZATION()\
{\
	FLASH.FENTRYR.WORD = 0xAA00;\
	while(0x0000 != FLASH.FENTRYR.WORD);\
}

#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_enter_pe_mode(bool in_dataflash);
#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_leave_pe_mode(void);
#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_check_status(void);
#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_stop(void);
#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_reset(void);
#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_wait_and_check_error(uint32_t wait_max);


void mcu_flash_init(void)
{
	uint8_t fclk_mhz = (mcu_frq_flash_hz / 1000000);

	FLASH.FPCKAR.WORD = 0x1E00 | fclk_mhz;

	MCU_FLASH_INITIALIZATION();

	FLASH.FCURAME.WORD = 0xC403;
	{
		uint8_t * p_src = (uint8_t*)FCU_FLASH_ADR_FCU;
		uint8_t * p_dest = (uint8_t*)MCU_FLASH_ADR_RAM;
		memcpy(p_dest, p_src, 4096);
	}
	FLASH.FCURAME.WORD = 0xC400;

	mcu_flash_wait_and_check_error(MCU_FLASH_MAX_WAIT_ERASE);
	FLASH.FSUINITR.WORD = 0x2D01;
	mcu_flash_wait_and_check_error(MCU_FLASH_MAX_WAIT_ERASE);
	FLASH.FSUINITR.WORD = 0x2D00;
	mcu_flash_wait_and_check_error(MCU_FLASH_MAX_WAIT_ERASE);
	FLASH.FAEINT.BYTE = 0; // Disable Flash interrupts
	FLASH.FRDYIE.BYTE = 0;
	FLASH.FWEPROR.BYTE = 0x01; // Enable programming/erasure


	// Clear ECC error flag in FCURAM
	if(mcu_flash_enter_pe_mode(false))
	{
		mcu_flash_stop();

		mcu_flash_leave_pe_mode();
	}
}

#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
bool mcu_flash_write(FLASH_PTR_TYPE flash_addr, BUF_PTR_TYPE buffer_addr, uint32_t bytes)
{
	bool ret_value = true;
	bool addr_is_dataflash = false;
	bool buffer_is_flash_data = false;
	volatile uint8_t* src_buffer_addr = mcu_flash_ram_buf;
	uint32_t src_buffer_cnt = 0;
	uint32_t i = 0;
	uint16_t padding = 0;

	mcu_flash_error_code = 0;

	if(bytes==0)					return true;	// 0 Bytes successfully flashed and it is not even a lie!

	if(FLASH.FSTATR.BIT.FRDY==0)
	{
		mcu_flash_error_code |= 0x00000001;
		return false;	// Flash is not ready
	}

	if((((uint32_t)flash_addr)%8) > 0)	// Address must be in a 8 Byte Stepping
	{
		mcu_flash_error_code |= 0x00000002;
		return false;
	}

//	flash_addr &= 0x00FFFFFF;	// Write Addresses must have upper two bytes zero!

	// Check if target is data or program flash
	if((uint32_t)flash_addr>=0x00100000 && (uint32_t)flash_addr<=0x0010FFFF)
		addr_is_dataflash = true;
#if MCU_PERIPHERY_ENABLE_CODE_FLASH
	else if((uint32_t)flash_addr>=MCU_FLASH_ADR_ROM_WRITE && (uint32_t)flash_addr<=0xFFFFFFFF)
		addr_is_dataflash = false;
	else if(flash_addr < 134)
	{
		addr_is_dataflash = false;
		flash_addr = BLOCK(flash_addr);
	}
#endif
	else
	{
		mcu_flash_error_code |= 0x00000004;
		return false;	// Address not even in ROM!
	}

	if(!addr_is_dataflash && (flash_addr & 0xFF) > 0) // In case of code flash stepping must be 256 byte
	{
		padding = 256 - (flash_addr & 0xFF);
//		dbg_printf(DBG_STRING, "Padding %d\n", padding);
//		mcu_flash_error_code |= 0x00000002;
//		return false;
	}

	// Check if source is flash data
	// Data from flash must be copied to the internal ram buffer first
	if((uint32_t)buffer_addr>=0x00100000 && (uint32_t)buffer_addr<=0x0010FFFF)
		buffer_is_flash_data = true;
#if MCU_PERIPHERY_ENABLE_CODE_FLASH
	else if((uint32_t)buffer_addr>=MCU_FLASH_ADR_ROM_READ)
		buffer_is_flash_data = true;
#endif
	else
	{
		src_buffer_addr = (uint8_t*)buffer_addr;
		buffer_is_flash_data = false;	// Data not from a flash source so it must not be buffered in RAM
	}

//	dbg_printf(DBG_STRING, "mcu_flash_write(%08x)\n", flash_addr);

	if(addr_is_dataflash)
	{
#ifndef __GNUC_PATCHLEVEL__
		clrpsw_i();
#else
		__asm volatile( "CLRPSW I" );
#endif
	}

	while(bytes>0 && ret_value)	// Loop till all bytes are written or an error ocurred
	{
		if(src_buffer_cnt==0)
		{
			if(buffer_is_flash_data)	// Copy Data to Buffer first!
			{
				mcu_flash_ram_cnt = 0;
				if(padding > 0)
				{
					while(mcu_flash_ram_cnt < padding)
						mcu_flash_ram_buf[mcu_flash_ram_cnt++] = 0;
					mcu_flash_ram_cnt = padding;
					padding = 0;
				}
				while(mcu_flash_ram_cnt<MCU_FLASH_BUFFER_RAM_SIZE)
				{
					mcu_flash_ram_buf[mcu_flash_ram_cnt] = *((uint8_t*)(buffer_addr+mcu_flash_ram_cnt));
					mcu_flash_ram_cnt++;
					bytes--;
					if(bytes==0)
						break;
				}
//				dbg_printf(DBG_STRING, "WR[%d] %$A\n", mcu_flash_ram_cnt, mcu_flash_ram_buf);
				src_buffer_addr = mcu_flash_ram_buf;
				src_buffer_cnt = mcu_flash_ram_cnt;
			}
			else
			{
				src_buffer_addr = (uint8_t*)buffer_addr;	// Buffer address renews at the end of every loop!
				src_buffer_cnt = bytes;
				bytes = 0;
			}
		}

		FLASH.FENTRYR.WORD = 0xAA00;
		while(0x0000 != FLASH.FENTRYR.WORD);

		ret_value &= mcu_flash_enter_pe_mode(addr_is_dataflash);

		while(src_buffer_cnt>0 && ret_value)
		{
			uint8_t flash_write_cnt = addr_is_dataflash ? 2 : 128;
			if(addr_is_dataflash)
				FLASH.FSADDR.LONG = flash_addr & 0xFFFFFFFC;
			else
				FLASH.FSADDR.LONG = flash_addr & 0xFFFFFF00;
			FACI_CMD(0xE8);
			FACI_CMD(flash_write_cnt);

			// There needs to be 128 words written at once. So there are always 256 byte buffer needed
			for(i = 0; i < flash_write_cnt; i++)
			{
				uint32_t wait_cnt = MCU_FLASH_MAX_WAIT_DBFULL;
				if(padding > 0)
				{
					if(padding > 1)
					{
						padding -= 2;
						FACI_WORD(0xFFFF); // Pad 0xFFFF at the beginning
					}
					else
					{
						// 1 padding byte is left, take the padding together with first data byte
						FACI_WORD((uint16_t)(((*(uint16_t *)src_buffer_addr) << 8) | 0xFF));
						src_buffer_cnt -= 1;
						src_buffer_addr += 1;
					}
				}
				else
				{
					switch(src_buffer_cnt)
					{
						case 0:	// There are no bytes left in the Buffer but the 256 / 8 Bytes must be filled!
							FACI_WORD(0xFFFF);
						break;

						case 1:	// Only one byte left so write 1 Byte (LSB) and set the higher Byte to 0xFF (Little Endian)
							FACI_WORD((uint16_t)((*(uint16_t *)src_buffer_addr) | 0xFF00));
							src_buffer_cnt = 0;
						break;

						default: // Write 2 Bytes into the flash
							FACI_WORD(*(uint16_t *)src_buffer_addr);
							src_buffer_cnt -= 2;
							src_buffer_addr += 2;
						break;
					}
				}
				flash_addr += 2;
				while (FLASH.FSTATR.BIT.DBFULL == 1)
				{
					if (wait_cnt-- <= 0)
					{
						ret_value = mcu_flash_reset();
					}
				}
			}
			FACI_CMD(0xD0);

			ret_value &= mcu_flash_wait_and_check_error(addr_is_dataflash ? MCU_FLASH_MAX_WAIT_DF_WRITE : MCU_FLASH_MAX_WAIT_ROM_WRITE);
		}

		ret_value &= mcu_flash_leave_pe_mode();

		buffer_addr+=mcu_flash_ram_cnt;
	}

	if(addr_is_dataflash)
	{
#ifndef __GNUC_PATCHLEVEL__
		setpsw_i();
#else
		__asm volatile( "SETPSW I" );
#endif
	}

	return ret_value;
}

#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
bool mcu_flash_erase(ERASE_PTR_TYPE flash_addr)
{
	bool ret_value = true;
	bool addr_is_dataflash = false;
	uint32_t flash_addr_max = 0;

	mcu_flash_error_code = 0;

	if((uint32_t)flash_addr>=0x00100000 && (uint32_t)flash_addr<=0x0010FFFF)
	{
		addr_is_dataflash = true;
		if((((uint32_t)flash_addr) % 0x800) > 0)	// Address
			return false;
	}
#if MCU_PERIPHERY_ENABLE_CODE_FLASH
	else if((uint32_t)flash_addr >= MCU_FLASH_ADR_ROM_READ)
	{
		addr_is_dataflash = false;
		if((((uint32_t)flash_addr) % 8) > 0)	// Address must be in a 8 Byte Stepping
			return false;
	}
	else if(flash_addr < 134)
	{
		addr_is_dataflash = false;
		flash_addr = BLOCK(flash_addr);
	}
#endif
	else
		return false;	// Address not in ROM!

	flash_addr &= 0x00FFFFFF;
//	dbg_printf(DBG_STRING, "mcu_flash_erase(%08x)\n", flash_addr);

	if(addr_is_dataflash)
	{
#ifndef __GNUC_PATCHLEVEL__
		clrpsw_i();
#else
		__asm volatile( "CLRPSW I" );
#endif
	}

	ret_value &= mcu_flash_enter_pe_mode(addr_is_dataflash);

	if(ret_value)	// If there was an error the erasure should not proceed
	{
		// Erasure priority mode
		FLASH.FCPSR.WORD = 0x0001;
		// Set address
		FLASH.FSADDR.LONG = (uint32_t)flash_addr;

		if(FLASH.FASTAT.BIT.CMDLK)
		{
			mcu_flash_error_code |= 0x40000000;
			mcu_flash_leave_pe_mode();
			return false;
		}

//		if(FLASH.FPROTR.BIT.FPROTCN == 0)
//		{
//			mcu_flash_error_code |= 0x80000000;
//			ret_value = false;
//		}
//		FACI_CMD(0x71);
//		FACI_CMD(0xD0);
//		ret_value &= mcu_flash_wait_and_check_error(MCU_FLASH_MAX_WAIT_BLANK_CHECK);
//		if(FLASH.FLKSTAT.BIT.FLOCKST)
//		{
//			mcu_flash_error_code |= 0x40000000;
//			mcu_flash_leave_pe_mode();
//			return false;
//		}
//		FACI_CMD(0x50);
//		ret_value &= mcu_flash_wait_and_check_error(MCU_FLASH_MAX_WAIT_NORMAL_TRANSITION);

		if(ret_value)
		{
			FACI_CMD(0x20);
			FACI_CMD(0xD0);
			ret_value &= mcu_flash_wait_and_check_error(MCU_FLASH_MAX_WAIT_ERASE);
		}
	}

	ret_value &= mcu_flash_leave_pe_mode();

	if(addr_is_dataflash)
	{
#ifndef __GNUC_PATCHLEVEL__
		setpsw_i();
#else
		__asm volatile( "SETPSW I" );
#endif
	}

//	dbg_printf(DBG_STRING, "mcu_flash_erase(%08x) -> %d\n", flash_addr, ret_value);

	return ret_value;
}

bool mcu_flash_read(FLASH_PTR_TYPE flash_addr, BUF_PTR_TYPE buffer_addr, uint32_t bytes)
{
	if(bytes == 0)
		return false;

	while(bytes--)
	{
		*(uint8_t*)buffer_addr = *(uint8_t*)flash_addr;
		buffer_addr++;
		flash_addr++;
	}
	return true;
}

#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_enter_pe_mode(bool in_dataflash)
{
	bool ret_val = true;
	uint8_t fclk_mhz = (mcu_frq_flash_hz / 1000000) + ((mcu_frq_flash_hz % 1000000) > 0 ? 1 : 0);
//	dbg_printf(DBG_STRING, "mcu_flash_enter_pe_mode(%08x, %B)\n", flash_addr, in_dataflash);

	FLASH.FWEPROR.BYTE = 0x01;

	if(FLASH.FPCKAR.BIT.PCKA != fclk_mhz)
	{
		FLASH.FPCKAR.WORD = 0x1E00 | fclk_mhz;
		ret_val &= mcu_flash_wait_and_check_error(MCU_FLASH_MAX_WAIT_NOTIFY_PERIPHERAL);
	}

	if(in_dataflash)
	{
		FLASH.FENTRYR.WORD = 0xAA80;
		ret_val &= (FLASH.FENTRYR.WORD == 0x0080);
	}
	else
	{
		FLASH.FENTRYR.WORD = 0xAA01;
//		while((FLASH.FENTRYR.WORD != 0x0001));
		ret_val &= (FLASH.FENTRYR.WORD == 0x0001);
	}
	// Override Lock bit
	FLASH.FPROTR.WORD = 0x5501;

	ret_val &= mcu_flash_check_status();

	return ret_val;
}

#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_leave_pe_mode()
{
	bool ret_value = true;
	ret_value &= mcu_flash_wait_and_check_error(MCU_FLASH_MAX_WAIT_ERASE);
	if (0 != FLASH.FASTAT.BIT.CMDLK)
	{
		FACI_CMD(0x50);
	}
	FLASH.FENTRYR.WORD = 0xAA00;
//	FLASH.FWEPROR.BYTE = 0x02;
//	dbg_printf(DBG_STRING, "mcu_flash_leave_pe_mode(%08x)\n", flash_addr);
//
//	dbg_printf(DBG_STRING, "FSTATR:%04x FASTAT:%02x FENTRYR:%04x FPESTAT:%02x\n",
//			FLASH.FSTATR.LONG, FLASH.FASTAT.BYTE, FLASH.FENTRYR.WORD, FLASH.FPESTAT.BIT.PEERRST);
	return ret_value;
}

#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_check_status(void)
{
	bool ret_value = true;
	if( FLASH.FSTATR.BIT.FCUERR == 1)
	{
		mcu_flash_error_code |= 0x00000010;
		ret_value = mcu_flash_reset();
	}
	if( FLASH.FSTATR.BIT.FLWEERR == 1)
	{
		mcu_flash_error_code |= 0x00000200;
		ret_value = mcu_flash_reset();
	}
	if((FLASH.FSTATR.LONG & 0x00007000) > 0)
	{
		if(FLASH.FSTATR.BIT.PRGERR)
			mcu_flash_error_code |= 0x00000040;
		if(FLASH.FSTATR.BIT.ERSERR)
			mcu_flash_error_code |= 0x00000080;
		if(FLASH.FSTATR.BIT.ILGLERR)
		{
			mcu_flash_error_code |= 0x00000020;
			if((FLASH.FASTAT.BYTE & 0x10) == 0x10)
				FLASH.FASTAT.BYTE = 0x10;
		}
		FACI_CMD(0x50);
		ret_value = false;
	}

	return ret_value;
}

#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_stop(void)
{
	FACI_CMD(0xB3); // Flash Stop

	while (1 != FLASH.FSTATR.BIT.FRDY)
	{
		;
	}

	if (0 != FLASH.FASTAT.BIT.CMDLK)
		return false;

	return true;
}

#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_reset(void)
{
	uint32_t wait_cnt_reset = MCU_FLASH_MAX_WAIT_FCU_RESET;
	FACI_CMD(0xB3);
	while(FLASH.FSTATR.BIT.FRDY == 0)
	{
		if(wait_cnt_reset == 0)
		{
			break;
		}
		wait_cnt_reset--;
	}
	if (FLASH.FASTAT.BIT.CFAE == 1)
	{
		FLASH.FASTAT.BIT.CFAE = 0;
	}
	if (FLASH.FASTAT.BIT.DFAE == 1)
	{
		FLASH.FASTAT.BIT.DFAE = 0;
	}
	wait_cnt_reset = MCU_FLASH_MAX_WAIT_FCU_RESET;
	FACI_CMD(0xB3);
	while(FLASH.FSTATR.BIT.FRDY == 0)
	{
		if(wait_cnt_reset == 0)
		{
			break;
		}
		wait_cnt_reset--;
	}
	MCU_FLASH_INITIALIZATION();

	return false;
}

#ifndef __GNUC_PATCHLEVEL__
#pragma section FRAM
#else
__attribute__((section(".fdata")))
#endif
static bool mcu_flash_wait_and_check_error(uint32_t wait_max)
{
	bool retval = true;
	bool has_timeout = false;

	while(FLASH.FSTATR.BIT.FRDY == 0)
	{
		if(wait_max == 0)
		{
			retval = mcu_flash_reset();
			has_timeout = true;
			break;
		}
		else
			wait_max--;
	}

	if(!has_timeout)
	{
		mcu_flash_error_code |= 0x00000100;
		retval = mcu_flash_check_status();
	}

	return retval;
}

#endif
