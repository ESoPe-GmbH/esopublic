// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_uart.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "mcu/mcu.h"

#if MCU_TYPE == MCU_ESP32

#if MCU_PERIPHERY_DEVICE_COUNT_I2C > 0

#include "../esp32/mcu_internal.h"
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
    /// Config of the ESP32 i2c driver.
    i2c_config_t conf;
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
    esp_err_t err;

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
    i2c->conf.mode = I2C_MODE_MASTER;
    i2c->conf.sda_io_num = sda;
    i2c->conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c->conf.scl_io_num = scl;
    i2c->conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c->conf.master.clk_speed = 100000;

    err = i2c_param_config(num, &i2c->conf);

    if(err != ESP_OK)
    {
        DBG_ERROR("Error setting i2c parameter: %d\n", err);
        return NULL;
    }

    err = i2c_driver_install(num, I2C_MODE_MASTER, 0, 0, 0);

    if(err != ESP_OK)
    {
        DBG_ERROR("Error initializing i2c: %d\n", err);
        return NULL;
    }

    DBG_INFO("Initialized MCU I2C interface %d on SDA=%d SCL=%d\n", num, sda, scl);

    _i2c_count++;
    return i2c;
}

void mcu_i2c_free(mcu_i2c_t h)
{
    // Clear the i2c driver
    i2c_driver_delete(h->num);
    // Reset the SDA and SCL pin
    gpio_reset_pin(h->conf.scl_io_num);
    gpio_reset_pin(h->conf.sda_io_num);
    // Clear the structure to have it usable again.
    memset(h, 0, sizeof(struct mcu_i2c_s));
    _i2c_count--;
}

void mcu_i2c_set_frq(mcu_i2c_t i2c, uint32_t frequency)
{    
    esp_err_t err;

    if(i2c == NULL)
        return;

    // If already correct, stop configuration
    if(i2c->conf.master.clk_speed == frequency)
        return;

    i2c->conf.master.clk_speed = frequency;

    err = i2c_param_config(i2c->num, &i2c->conf);

    if(err != ESP_OK)
    {
        DBG_ERROR("Error setting i2c parameter: %d\n", err);
    }
}

uint32_t mcu_i2c_get_frq(mcu_i2c_t i2c)
{
    if(i2c == NULL)
        return 0;

    return i2c->conf.master.clk_speed;
}

void mcu_i2c_set_address(mcu_i2c_t i2c, uint8_t address)
{
    if(i2c == NULL)
        return;

    i2c->address = address << 1;
}

bool mcu_i2c_wr(mcu_i2c_t i2c, uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen)
{        
    esp_err_t err = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if(wlen > 0)
    {
        err = i2c_master_start(cmd);
        if (err != ESP_OK) 
            goto end;

        err = i2c_master_write_byte(cmd, i2c->address | I2C_MASTER_WRITE, true);
        if (err != ESP_OK) 
            goto end;

        err = i2c_master_write(cmd, wbuf, wlen, true);
        if (err != ESP_OK)
            goto end;
    }

    if(rlen > 0)
    {
        err = i2c_master_start(cmd);
        if (err != ESP_OK)
            goto end;

        err = i2c_master_write_byte(cmd, i2c->address | I2C_MASTER_READ, true);
        if (err != ESP_OK) 
            goto end;
        
        err = i2c_master_read(cmd, rbuf, rlen, I2C_MASTER_LAST_NACK);
        if (err != ESP_OK) 
            goto end;
    }
    
    err = i2c_master_stop(cmd);
    if (err != ESP_OK) 
        goto end;

    // DBG_INFO("I2C WR: A=%02x W[%#Q] R[%#Q]\n", i2c->address >> 1, wlen, wbuf, rlen, rbuf);
    err = i2c_master_cmd_begin(i2c->num, cmd, 100);

end:
    if(err != ESP_OK)
    {
        DBG_ERROR("I2C Error: %s\n", esp_err_to_name(err));
    }

    i2c_cmd_link_delete(cmd);

    return err == ESP_OK;
}

bool mcu_i2c_wwr(mcu_i2c_t i2c, uint8_t* wbuf, size_t wlen, uint8_t* w2buf, size_t w2len, uint8_t* rbuf, size_t rlen)
{        
    esp_err_t err = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if(wlen > 0)
    {
        err = i2c_master_start(cmd);
        if (err != ESP_OK) 
            goto end;

        err = i2c_master_write_byte(cmd, i2c->address | I2C_MASTER_WRITE, true);
        if (err != ESP_OK) 
            goto end;

        err = i2c_master_write(cmd, wbuf, wlen, true);
        if (err != ESP_OK)
            goto end;
    }

    if(w2len > 0)
    {
        err = i2c_master_write(cmd, w2buf, w2len, true);
        if (err != ESP_OK)
            goto end;
    }

    if(rlen > 0)
    {
        err = i2c_master_start(cmd);
        if (err != ESP_OK)
            goto end;

        err = i2c_master_write_byte(cmd, i2c->address | I2C_MASTER_READ, true);
        if (err != ESP_OK) 
            goto end;
        
        err = i2c_master_read(cmd, rbuf, rlen, I2C_MASTER_LAST_NACK);
        if (err != ESP_OK) 
            goto end;
    }
    
    err = i2c_master_stop(cmd);
    if (err != ESP_OK) 
        goto end;

    // DBG_INFO("I2C WWR: A=%02x W[%#Q%#Q] R[%#Q]\n", i2c->address >> 1, wlen, wbuf, w2len, w2buf, rlen, rbuf);
    err = i2c_master_cmd_begin(i2c->num, cmd, 100);

end:
    if(err != ESP_OK)
        DBG_ERROR("I2C Error: %s\n", esp_err_to_name(err));

    i2c_cmd_link_delete(cmd);

    return err == ESP_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // MCU_PERIPHERY_ENABLE_UART

#endif // #if MCU_TYPE == R5F563
