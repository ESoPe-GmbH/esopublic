// Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_uart.c
 * 	@copyright Urheberrecht 2018-2025 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "mcu/mcu.h"

#if MCU_TYPE == MCU_NEW_NAME && MCU_PERIPHERY_DEVICE_COUNT_I2C > 0

#include "mcu_internal.h"
#include "module/fifo/fifo.h"
#include <string.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Structure containing all data for managing an i2c interface.
struct mcu_i2c_s
{
    /// Indicates whether this structure is in use or not.
    bool initialized;
    /// Port of the i2c interface
    uint8_t num;
    /// Address of the slave device
    uint8_t address;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Contains the data for each i2c interface.
struct mcu_i2c_s _i2c[MCU_PERIPHERY_DEVICE_COUNT_I2C] = {0};
/// Number of i2c interfaces registered
uint8_t _i2c_count = 0;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

mcu_i2c_t mcu_i2c_init(uint8_t num, MCU_IO_PIN sda, MCU_IO_PIN scl)
{
    if(_i2c_count >= MCU_PERIPHERY_DEVICE_COUNT_I2C)
        return NULL;

    mcu_i2c_t i2c = NULL;
	// Find a free i2c handler
	for(int i = 0; i < MCU_PERIPHERY_DEVICE_COUNT_I2C && i2c == NULL; i++)
	{
		if(!_i2c[i].initialized)
		{
			i2c = &_i2c[i];
		}
	}

	// No free spi handler found. Return error
	if(i2c == NULL)
	{
		return NULL;
	}

    i2c->initialized = true;
    i2c->num = num;

    // TODO: Initialize i2c

    DBG_INFO("Initialized MCU I2C interface %d on SDA=%d SCL=%d\n", num, sda, scl);

    _i2c_count++;
    return i2c;
}

void mcu_i2c_free(mcu_i2c_t h)
{    
    // TODO: Free internal components if necessary

    // Clear the structure to have it usable again.
    memset(h, 0, sizeof(struct mcu_i2c_s));
    _i2c_count--;
}

void mcu_i2c_set_frq(mcu_i2c_t i2c, uint32_t frequency)
{    
    if(i2c == NULL)
        return;

    // TODO: Implement setting the frequency
}

uint32_t mcu_i2c_get_frq(mcu_i2c_t i2c)
{
    if(i2c == NULL)
        return 0;

    return 0; // TODO: Implement getting the frequency
}

void mcu_i2c_set_address(mcu_i2c_t i2c, uint8_t address)
{
    if(i2c == NULL)
        return;

    i2c->address = address << 1;

    // TODO: Set address to register if necessary, may be safer to set it to the registers inside the write/read functions
}

bool mcu_i2c_wr(mcu_i2c_t i2c, uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen)
{        
    if(i2c == NULL)
    {
        return false;
    }
    
    // TODO: Implement

    return false;
}

bool mcu_i2c_wwr(mcu_i2c_t i2c, uint8_t* wbuf, size_t wlen, uint8_t* w2buf, size_t w2len, uint8_t* rbuf, size_t rlen)
{        
    if(i2c == NULL)
    {
        return false;
    }

    // TODO: Implement

    return false;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // MCU_TYPE == MCU_NEW_NAME && MCU_PERIPHERY_ENABLE_I2C > 0
