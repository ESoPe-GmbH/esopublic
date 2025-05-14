/**
 * @file mcu_can.c
 * @copyright Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_PERIPHERY_DEVICE_COUNT_CAN > 0 && MCU_TYPE == MCU_NEW_NAME

#include "../../sys.h"
#include "module/fifo/fifo.h"
#include "mcu_internal.h"
#include <string.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 *	Casts a pointer to an mcu_can_handler_ctx pointer.
 *
 * @param h			Pointer to cast
 **/
#define CAST_CAN(h)	((mcu_can_handler_ctx*)h)
				
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


uint8_t mcu_current_can_handler = 0;

struct mcu_can_s mcu_can_handler[MCU_PERIPHERY_DEVICE_COUNT_CAN] = {0};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

mcu_can_t mcu_can_init(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx)
{
	mcu_can_t h;

	if(mcu_current_can_handler >= MCU_PERIPHERY_DEVICE_COUNT_CAN)
		MCU_RETURN_ERROR(MCU_ERROR_CAN_NOT_AVAILABLE, NULL);

	h = &mcu_can_handler[mcu_current_can_handler];	// For easier handling

	// TODO: Initialize CAN

	mcu_current_can_handler++;

	return (mcu_can_t)h;
}

MCU_RESULT mcu_can_set_baudrate(mcu_can_t h, uint32_t baudrate)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_CAN_INVALID, MCU_ERROR_CAN_INVALID);

	// TODO: Check valid baud or return MCU_RETURN_ERROR(MCU_ERROR_CAN_BAUDRATE_INVALID, MCU_ERROR_CAN_BAUDRATE_INVALID); on error
	// TODO: Calculate baudrate

	h->baud = baudrate;

	return MCU_OK;
}

uint32_t mcu_can_get_baudrate(mcu_can_t h)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_CAN_INVALID, 0);

	return h->baud;
}

MCU_RESULT mcu_can_receive_enable(mcu_can_t h, MCU_INT_LVL lvl, uint8_t *buf, uint16_t buf_elements)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_CAN_INVALID, MCU_ERROR_CAN_INVALID);

	// TODO: Implement

	return MCU_OK;
}

bool mcu_can_receive_package(mcu_can_t h, can_frame_t *cf)
{
	twai_message_t msg = {0};

	if(h == NULL || cf == NULL)
		return false;

	// TODO: Receive frame and copy it into @c cf

	return false;
}

void mcu_can_transmit_package(mcu_can_t h, can_frame_t cf)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_CAN_INVALID, );

	// TODO: Transmit frame
}

MCU_RESULT mcu_can_register_receive_messagebox(mcu_can_t h, uint32_t addr_mask, uint32_t addr, bool is_extended)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_CAN_INVALID, MCU_ERROR_CAN_INVALID);
	
	// TODO: Register a receive messagebox

	return MCU_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


#endif // MCU_PERIPHERY_DEVICE_COUNT_CAN>0
