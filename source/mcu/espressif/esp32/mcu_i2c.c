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
#if USE_I2C_MASTER_API
    /// Config of the ESP32 i2c master driver.
    i2c_master_bus_config_t conf;
    /// @brief Bus handle of i2c master driver
    i2c_master_bus_handle_t bus_handle;
    /// @brief Config for the device -> Contains address and frequency
    i2c_device_config_t dev_conf;
    /// @brief Handle for the device
    i2c_master_dev_handle_t dev_handle;
#else
    /// Config of the ESP32 i2c driver.
    i2c_config_t conf;
#endif
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
#if USE_I2C_MASTER_API
    i2c->conf.i2c_port = num;
    i2c->conf.sda_io_num = sda;
    i2c->conf.scl_io_num = scl;
    i2c->conf.flags.enable_internal_pullup = 1;
    i2c->conf.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c->dev_conf.dev_addr_length = I2C_ADDR_BIT_LEN_7;

    err = i2c_new_master_bus(&i2c->conf, &i2c->bus_handle);

    if(err != ESP_OK)
    {
        DBG_ERROR("Error creating master: %d\n", err);
        return NULL;
    }

#else
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
#endif

    DBG_INFO("Initialized MCU I2C interface %d on SDA=%d SCL=%d\n", num, sda, scl);

    _i2c_count++;
    return i2c;
}

void mcu_i2c_free(mcu_i2c_t h)
{    
#if USE_I2C_MASTER_API
    if(h->bus_handle)
    {
        i2c_del_master_bus(h->bus_handle);
        h->bus_handle = NULL;        
    }
#else
    // Clear the i2c driver
    i2c_driver_delete(h->num);
#endif
    // Reset the SDA and SCL pin
    gpio_reset_pin(h->conf.scl_io_num);
    gpio_reset_pin(h->conf.sda_io_num);
    // Clear the structure to have it usable again.
    memset(h, 0, sizeof(struct mcu_i2c_s));
    _i2c_count--;
}

void mcu_i2c_set_frq(mcu_i2c_t i2c, uint32_t frequency)
{    
    if(i2c == NULL)
        return;

#if USE_I2C_MASTER_API
    i2c->dev_conf.scl_speed_hz = frequency;
#else
    esp_err_t err;
    // If already correct, stop configuration
    if(i2c->conf.master.clk_speed == frequency)
        return;

    i2c->conf.master.clk_speed = frequency;

    err = i2c_param_config(i2c->num, &i2c->conf);

    if(err != ESP_OK)
    {
        DBG_ERROR("Error setting i2c parameter: %d\n", err);
    }
#endif
}

uint32_t mcu_i2c_get_frq(mcu_i2c_t i2c)
{
    if(i2c == NULL)
        return 0;

#if USE_I2C_MASTER_API
    return i2c->dev_conf.scl_speed_hz;
#else
    return i2c->conf.master.clk_speed;
#endif
}

void mcu_i2c_set_address(mcu_i2c_t i2c, uint8_t address)
{
    if(i2c == NULL)
        return;

#if USE_I2C_MASTER_API
    i2c->dev_conf.device_address = address;
#endif
    i2c->address = address << 1;
}

bool mcu_i2c_wr(mcu_i2c_t i2c, uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen)
{        
    esp_err_t err = ESP_OK;

    if((wbuf == NULL || wlen == 0) && (rbuf == NULL || rlen == 0))
    {
        // Nothing to write and nothing to read is invalid
        return false;
    }

#if USE_I2C_MASTER_API
    err = i2c_master_bus_add_device(i2c->bus_handle, &i2c->dev_conf, &i2c->dev_handle);

    if(rbuf == NULL || rlen == 0)
    {
        err = i2c_master_transmit(i2c->dev_handle, wbuf, wlen, 100);
        if(err != ESP_OK)
        {
            DBG_ERROR("I2C Transmit Error: %s\n", esp_err_to_name(err));
            return false;
        }
    }
    else if(wbuf == NULL || wlen == 0)
    {
        err = i2c_master_receive(i2c->dev_handle, rbuf, rlen, 100);
        if(err != ESP_OK)
        {
            DBG_ERROR("I2C Receive Error: %s\n", esp_err_to_name(err));
            return false;
        }
    }
    else
    {
        err = i2c_master_transmit_receive(i2c->dev_handle, wbuf, wlen, rbuf, rlen, 100);
        if(err != ESP_OK)
        {
            DBG_ERROR("I2C Transmit/Receive Error: %s\n", esp_err_to_name(err));
            return false;
        }
    }

    err = i2c_master_bus_rm_device(i2c->dev_handle);
    if(err != ESP_OK)
    {
        DBG_ERROR("I2C Remove Device Error: %s\n", esp_err_to_name(err));
    }
    i2c->dev_handle = NULL;
#else
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

#endif
    return err == ESP_OK;
}

bool mcu_i2c_wwr(mcu_i2c_t i2c, uint8_t* wbuf, size_t wlen, uint8_t* w2buf, size_t w2len, uint8_t* rbuf, size_t rlen)
{        
    esp_err_t err = ESP_OK;
#if USE_I2C_MASTER_API
    err = i2c_master_bus_add_device(i2c->bus_handle, &i2c->dev_conf, &i2c->dev_handle);

    if(wbuf && wlen > 0 && w2buf && w2len > 0)
    {
        i2c_master_transmit_multi_buffer_info_t buf[2] = {
            {.write_buffer = wbuf, .buffer_size = wlen},
            {.write_buffer = w2buf, .buffer_size = w2len}
        };
        err = i2c_master_multi_buffer_transmit(i2c->dev_handle, buf, 2, 100);
        if(err != ESP_OK)
        {
            DBG_ERROR("I2C Transmit Error: %s\n", esp_err_to_name(err));
            return false;
        }
    }
    else if(wbuf && wlen > 0)
    {
        err = i2c_master_transmit(i2c->dev_handle, wbuf, wlen, 100);
        if(err != ESP_OK)
        {
            DBG_ERROR("I2C Transmit Error: %s\n", esp_err_to_name(err));
            return false;
        }
    }
    else if(w2buf && w2len > 0)
    {
        err = i2c_master_transmit(i2c->dev_handle, w2buf, w2len, 100);
        if(err != ESP_OK)
        {
            DBG_ERROR("I2C Transmit Error: %s\n", esp_err_to_name(err));
            return false;
        }
    }

    if(rbuf && rlen > 0)
    {
        err = i2c_master_receive(i2c->dev_handle, rbuf, rlen, 100);
        if(err != ESP_OK)
        {
            DBG_ERROR("I2C Receive Error: %s\n", esp_err_to_name(err));
            return false;
        }
    }

    err = i2c_master_bus_rm_device(i2c->dev_handle);
    if(err != ESP_OK)
    {
        DBG_ERROR("I2C Remove Device Error: %s\n", esp_err_to_name(err));
    }
    i2c->dev_handle = NULL;
#else
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

#endif
    return err == ESP_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // MCU_PERIPHERY_ENABLE_UART

#endif // #if MCU_TYPE == R5F563
