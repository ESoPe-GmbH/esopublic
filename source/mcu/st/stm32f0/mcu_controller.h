// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_controller.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *  @date		05.09.2011
 *
 *  @brief
 *
 *  @version	1.00
 *  			 - Erste Version
 *
 ******************************************************************************/

#ifndef MCU_STM32F0_MCU_CONTROLLER_HEADER_FIRST_INCLUDE_GUARD
#define MCU_STM32F0_MCU_CONTROLLER_HEADER_FIRST_INCLUDE_GUARD

#define MCU_TYPE 					MCU_STM32F0

#if !(MCU_CONTROLLER_PIN_COUNT==48)
	#error "Invalid MCU Pin Size or Pin Size not implemented"
#endif

#if !(MCU_CONTROLLER_ROM_SIZE_KBYTE==128)
	#error "Invalid MCU ROM Size or ROM Size nor implemented"
#endif

// CPU IO Pinne
typedef enum
{
	PA_0 = 0x0A00,	///< Port A Pin 0
	PA_1 = 0x0A01,	///< Port A Pin 1
	PA_2 = 0x0A02,	///< Port A Pin 2
	PA_3 = 0x0A03,	///< Port A Pin 3
    PA_4 = 0x0A04,  ///< Port A Pin 4
	PA_5 = 0x0A05,	///< Port A Pin 5
	PA_6 = 0x0A06,	///< Port A Pin 6
    PA_7 = 0x0A07,  ///< Port A Pin 7
    PA_8 = 0x0A08,  ///< Port A Pin 8
    PA_9 = 0x0A09,  ///< Port A Pin 9
    PA_10 = 0x0A0A,  ///< Port A Pin 10
    PA_11 = 0x0A0B,  ///< Port A Pin 11
    PA_12 = 0x0A0C,  ///< Port A Pin 12
    PA_13 = 0x0A0D,  ///< Port A Pin 13
    PA_14 = 0x0A0E,  ///< Port A Pin 14
    PA_15 = 0x0A0F,  ///< Port A Pin 15
	PA   = 0x0AFF,	///< Port A

	PB_0 = 0x0B00,	///< Port B Pin 0
	PB_1 = 0x0B01,	///< Port B Pin 1
	PB_2 = 0x0B02,	///< Port B Pin 2
	PB_3 = 0x0B03,	///< Port B Pin 3
    PB_4 = 0x0B04,  ///< Port B Pin 4
	PB_5 = 0x0B05,	///< Port B Pin 5
	PB_6 = 0x0B06,	///< Port B Pin 6
    PB_7 = 0x0B07,  ///< Port B Pin 7
    PB_8 = 0x0B08,  ///< Port B Pin 8
    PB_9 = 0x0B09,  ///< Port B Pin 9
    PB_10 = 0x0B0A,  ///< Port B Pin 10
    PB_11 = 0x0B0B,  ///< Port B Pin 11
    PB_12 = 0x0B0C,  ///< Port B Pin 12
    PB_13 = 0x0B0D,  ///< Port B Pin 13
    PB_14 = 0x0B0E,  ///< Port B Pin 14
    PB_15 = 0x0B0F,  ///< Port B Pin 15
	PB   = 0x0BFF,	///< Port B

	PC_0 = 0x0C00,	///< Port C Pin 0
	PC_1 = 0x0C01,	///< Port C Pin 1
	PC_2 = 0x0C02,	///< Port C Pin 2
	PC_3 = 0x0C03,	///< Port C Pin 3
    PC_4 = 0x0C04,  ///< Port C Pin 4
	PC_5 = 0x0C05,	///< Port C Pin 5
	PC_6 = 0x0C06,	///< Port C Pin 6
    PC_7 = 0x0C07,  ///< Port C Pin 7
    PC_8 = 0x0C08,  ///< Port C Pin 8
    PC_9 = 0x0C09,  ///< Port C Pin 9
    PC_10 = 0x0C0A,  ///< Port C Pin 10
    PC_11 = 0x0C0B,  ///< Port C Pin 11
    PC_12 = 0x0C0C,  ///< Port C Pin 12
    PC_13 = 0x0C0D,  ///< Port C Pin 13
    PC_14 = 0x0C0E,  ///< Port C Pin 14
    PC_15 = 0x0C0F,  ///< Port C Pin 15
	PC   = 0x0CFF,	///< Port C

	PD_0 = 0x0D00,	///< Port D Pin 0
	PD_1 = 0x0D01,	///< Port D Pin 1
	PD_2 = 0x0D02,	///< Port D Pin 2
	PD_3 = 0x0D03,	///< Port D Pin 3
    PD_4 = 0x0D04,  ///< Port D Pin 4
	PD_5 = 0x0D05,	///< Port D Pin 5
	PD_6 = 0x0D06,	///< Port D Pin 6
    PD_7 = 0x0D07,  ///< Port D Pin 7
    PD_8 = 0x0D08,  ///< Port D Pin 8
    PD_9 = 0x0D09,  ///< Port D Pin 9
    PD_10 = 0x0D0A,  ///< Port D Pin 10
    PD_11 = 0x0D0B,  ///< Port D Pin 11
    PD_12 = 0x0D0C,  ///< Port D Pin 12
    PD_13 = 0x0D0D,  ///< Port D Pin 13
    PD_14 = 0x0D0E,  ///< Port D Pin 14
    PD_15 = 0x0D0F,  ///< Port D Pin 15
	PD   = 0x0DFF,	///< Port D

	PE_0 = 0x0E00,	///< Port E Pin 0
	PE_1 = 0x0E01,	///< Port E Pin 1
	PE_2 = 0x0E02,	///< Port E Pin 2
	PE_3 = 0x0E03,	///< Port E Pin 3
    PE_4 = 0x0E04,  ///< Port E Pin 4
	PE_5 = 0x0E05,	///< Port E Pin 5
	PE_6 = 0x0E06,	///< Port E Pin 6
    PE_7 = 0x0E07,  ///< Port E Pin 7
    PE_8 = 0x0E08,  ///< Port E Pin 8
    PE_9 = 0x0E09,  ///< Port E Pin 9
    PE_10 = 0x0E0A,  ///< Port E Pin 10
    PE_11 = 0x0E0B,  ///< Port E Pin 11
    PE_12 = 0x0E0C,  ///< Port E Pin 12
    PE_13 = 0x0E0D,  ///< Port E Pin 13
    PE_14 = 0x0E0E,  ///< Port E Pin 14
    PE_15 = 0x0E0F,  ///< Port E Pin 15
	PE   = 0x0EFF,	///< Port E

	PIN_NONE = 0xFFFF
	
} MCU_IO_PIN;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// UART Interfaces
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// SPI Interfaces
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Flash Type defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define FLASH_PTR_TYPE	uint32_t
#define BUF_PTR_TYPE   	uint32_t
#define ERASE_PTR_TYPE	uint32_t

#define MCU_CONTROLLER_FLASH_MIN_STEPPING	128

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Flash Block Addresses
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/* Code Flash Block Area: 256 2kB Blocks */
#define BLOCK(n)	(0x08000000 + ((n) * 0x800))

/* No special dataflash on controller: Use normal Code Flash */
#define BLOCK_DB(n)	BLOCK(n)

#endif
