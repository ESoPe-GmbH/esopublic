// Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_ad.c
 * 	@copyright Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "mcu/mcu.h"

#if MCU_TYPE == MCU_NEW_NAME

#include "mcu_internal.h"

#if MCU_PERIPHERY_DEVICE_COUNT_AD > 0

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

				
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t mcu_current_ad_handler = 0;
struct mcu_ad_s mcu_ad_handler[MCU_PERIPHERY_DEVICE_COUNT_AD];

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

mcu_ad_t mcu_ad_init(MCU_IO_PIN pin)
{
	mcu_ad_t handle;

	if(mcu_current_ad_handler>=MCU_PERIPHERY_DEVICE_COUNT_AD)
		MCU_RETURN_ERROR(MCU_ERROR_AD_NOT_AVAILABLE, NULL);

	handle = &mcu_ad_handler[mcu_current_ad_handler];

	// TODO: Implement
	
	mcu_current_ad_handler++;

	return (mcu_ad_t)handle;
}

MCU_RESULT mcu_ad_set_param(mcu_ad_t h, void (*f)(int), MCU_INT_LVL lvl, MCU_AD_SIGNEDNESS sign, uint8_t bit_resolution, bool auto_read)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_AD_INVALID, MCU_ERROR_AD_INVALID);	// Handler is invalid
		
	// TODO: Implement
	
	// if(bit_resolution != 12)
	// 	MCU_RETURN_ERROR(MCU_ERROR_AD_RESOLUTION_INVALID, MCU_ERROR_AD_RESOLUTION_INVALID);

	// if(sign == MCU_AD_SIGNED)
	// 	MCU_RETURN_ERROR(MCU_ERROR_AD_SIGNEDNESS_INVALID, MCU_ERROR_AD_SIGNEDNESS_INVALID);

	// if(f != NULL || lvl > 0)
	// 	MCU_RETURN_ERROR(MCU_ERROR_AD_INTERRUPT_NOT_SUPPORTED, MCU_ERROR_AD_INTERRUPT_NOT_SUPPORTED);

	mcu_ad_set_channel_active(h);

	if(auto_read)
		return mcu_ad_start(h);

	return MCU_OK;
}

MCU_RESULT mcu_ad_set_channel_active(mcu_ad_t h)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_AD_INVALID, MCU_ERROR_AD_INVALID);	// Handler is invalid

	// TODO: Implement

	return MCU_OK;
}

MCU_RESULT mcu_ad_start(mcu_ad_t h)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_AD_INVALID, MCU_ERROR_AD_INVALID);	// Handler is invalid

	// TODO: Implement

	return MCU_OK;
}

bool mcu_ad_ready(mcu_ad_t h)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_AD_INVALID, false);					// Handler is invalid

	// TODO: Implement
	return false;
}

int mcu_ad_read(mcu_ad_t h)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_AD_INVALID, 0);						// Handler is invalid

	// TODO: Implement

	return 0;	
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


#endif

#endif // #if MCU_TYPE == MCU_NEW_NAME
