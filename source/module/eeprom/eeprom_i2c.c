// Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file eeprom_i2c.c
 * @copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "module_public.h"
#if MODULE_ENABLE_EEPROM

#include "eeprom_i2c.h"
#include "eeprom_internal.h"
#include "module/comm/dbg.h"
#include "module/convert/string.h"
#include "mcu/sys.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Datastructure used for handling the eeprom.
struct eeprom_i2c_data_s
{
    /// @brief Timestamp of the last access
    uint32_t timestamp_last_write;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Gets the EEPROM address based on configuration and the targetted address.
 * 
 * @param config    Pointer to the configuration of the eeprom.
 * @param address   Address that should be read or written.
 * @return uint8_t  I2C Address of the eeprom.
 */
static uint8_t _get_address(const eeprom_i2c_config_t* config, uint16_t address);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

eeprom_device_t eeprom_i2c_init(const eeprom_i2c_config_t* config)
{
    DBG_ASSERT(config, NO_ACTION, NULL, "Config should not be NULL\n");
    DBG_ASSERT(config->a0 < 2, NO_ACTION, NULL, "A0 must be 0 or 1\n");
    DBG_ASSERT(config->a1 < 2, NO_ACTION, NULL, "A1 must be 0 or 1\n");
    DBG_ASSERT(config->a2 < 2, NO_ACTION, NULL, "A2 must be 0 or 1\n");
    DBG_ASSERT(_get_address(config, 0) > 0, NO_ACTION, NULL, "The size is invalid\n");
    DBG_ASSERT(config->page_size > 0, NO_ACTION, NULL, "The page size must be set\n");

    eeprom_device_t device = mcu_heap_calloc(1, sizeof(struct eeprom_device_s));
    if(device == NULL)
        return NULL;

    device->context = mcu_heap_calloc(1, sizeof(struct eeprom_i2c_data_s));
    if(device->context == NULL)
    {
        mcu_heap_free(device);
        return NULL;
    }

    device->config = config;
    device->interface.f_write = eeprom_i2c_write;
    device->interface.f_read = eeprom_i2c_read;

    return device;
}

void eeprom_i2c_free(eeprom_device_t device)
{
    DBG_ASSERT(device, NO_ACTION, NO_RETURN, "Invalid device\n");
    mcu_heap_free(device->context);
    mcu_heap_free(device);
}

FUNCTION_RETURN eeprom_i2c_read(eeprom_device_t device, uint16_t address, void* buffer, uint16_t length)
{
    DBG_ASSERT(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid device\n");
    DBG_ASSERT(buffer, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Buffer shall not be NULL\n");
    DBG_ASSERT(length > 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Length cannot be 0\n");
    const eeprom_i2c_config_t* config = device->config;
    DBG_ASSERT(address < config->max_size, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid address\n");
    DBG_ASSERT(address + length <= config->max_size, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Data does not fit in eeprom\n");

    uint8_t i2c_address = _get_address(config, address);

    i2c_set_address(config->i2c, i2c_address);

    struct eeprom_i2c_data_s* data = device->context;
    
    bool success;
    uint16_t count = 0;
    uint16_t offset = address;
    do
    {    
        uint16_t l = length - count;
        // DBG_INFO("l=%d count=%d length=%d page=%d offset=%d\n", l, count, length, config->page_size, offset);

        // Ensure to only read a page size in one operation
        if(l > config->page_size)
            l = config->page_size;
        // DBG_INFO("l=%d count=%d length=%d page=%d offset=%d\n", l, count, length, config->page_size, offset);

        // Ensure to stay inside the current pages boundaries
        uint16_t address_offset = offset & (config->page_size - 1);
        if(address_offset > 0)
        {
            uint16_t l2 = config->page_size - address_offset;
            if(l2 < l)
                l = l2;
            // DBG_INFO("l=%d count=%d length=%d page=%d offset=%d address_offset=%d l2=%d\n", l, count, length, config->page_size, offset, address_offset, l2);
        }

        // DBG_INFO("l=%d count=%d length=%d page=%d offset=%d address_offset=%d\n", l, count, length, config->page_size, offset, address_offset);
        
        if(system_get_tick_count() - data->timestamp_last_write < 5)
            mcu_wait_ms(5);

        success = i2c_wr(config->i2c, (uint8_t*)&offset, 1, &((uint8_t*)buffer)[count], l);

        data->timestamp_last_write = system_get_tick_count();

        if(!success)
            return FUNCTION_RETURN_NOT_READY;

        count += l;
        offset += l;

    } while (success && count < length);

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN eeprom_i2c_write(eeprom_device_t device, uint16_t address, const void* buffer, uint16_t length)
{
    DBG_ASSERT(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid device\n");
    DBG_ASSERT(buffer, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Buffer shall not be NULL\n");
    DBG_ASSERT(length > 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Length cannot be 0\n");
    const eeprom_i2c_config_t* config = device->config;
    DBG_ASSERT(address < config->max_size, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid address\n");
    DBG_ASSERT(address + length <= config->max_size, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Data does not fit in eeprom\n");

    uint8_t i2c_address = _get_address(config, address);

    i2c_set_address(config->i2c, i2c_address);

    struct eeprom_i2c_data_s* data = device->context;

    bool success;
    uint16_t count = 0;
    uint16_t offset = address;
    do
    {    
        uint16_t ptr_address = offset;
        uint8_t* ptr = &((uint8_t*)buffer)[count];
        uint16_t l = length - count;
        // DBG_INFO("l=%d count=%d length=%d page=%d\n", l, count, length, config->page_size);

        // Ensure to only write a page size in one operation
        if(l > config->page_size)
            l = config->page_size;

        // DBG_INFO("l=%d count=%d length=%d page=%d\n", l, count, length, config->page_size);

        // Ensure to stay inside the current pages boundaries
        uint16_t address_offset = offset & (config->page_size - 1);
        if(address_offset > 0 || l < config->page_size)
        {
            uint16_t l2 = config->page_size - address_offset;
            if(l2 < l)
                l = l2;
            else
                l2 = l;

            ptr = mcu_heap_calloc(1, config->page_size);

            if(ptr == NULL)
                return FUNCTION_RETURN_INSUFFICIENT_MEMORY;

            if(system_get_tick_count() - data->timestamp_last_write < 5)
                mcu_wait_ms(5);

            ptr_address = offset & ~(config->page_size - 1);

            success = i2c_wr(config->i2c, (uint8_t*)&ptr_address, 1, ptr, config->page_size);

            data->timestamp_last_write = system_get_tick_count();

            // DBG_INFO("address_offset=%d l2=%d ptr_address=%d ptr=%16Q buf=%#Q\n", address_offset, l2, ptr_address, ptr, l2, &buffer[count]);

            memcpy(ptr + (address_offset), &((uint8_t*)buffer)[count], l2);

            if(!success)
            {
                mcu_heap_free(ptr);
                return FUNCTION_RETURN_NOT_READY;
            }

            // DBG_INFO("l=%d count=%d length=%d page=%d offset=%d address_offset=%d l2=%d\n", l, count, length, config->page_size, offset, address_offset, l2);
        }

        // DBG_INFO("l=%d count=%d length=%d page=%d offset=%d address_offset=%d\n", l, count, length, config->page_size, offset, address_offset);

        if(system_get_tick_count() - data->timestamp_last_write < 5)
            mcu_wait_ms(5);

        if(address_offset > 0 || l < config->page_size)
        {
            success = i2c_wwr(config->i2c, (uint8_t*)&ptr_address, 1, ptr, config->page_size, NULL, 0);
            mcu_heap_free(ptr);
        }
        else
            success = i2c_wwr(config->i2c, (uint8_t*)&ptr_address, 1, ptr, l, NULL, 0);

        data->timestamp_last_write = system_get_tick_count();

        if(!success)
            return FUNCTION_RETURN_NOT_READY;

        count += l;
        offset += l;

    } while (success && count < length);

    return FUNCTION_RETURN_OK;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static uint8_t _get_address(const eeprom_i2c_config_t* config, uint16_t address)
{
    switch(config->max_size)
    {
        case 1024:
        case 2048:
            return 0x50 | (config->a2 << 2) | (config->a1 << 1) | (config->a0);
            
        case 4096:
            return 0x50 | (config->a2 << 2) | (config->a1 << 1) | ((address >> 8) & 1);
            
        case 8192:
            return 0x50 | (config->a2 << 2) | ((address >> 8) & 3);
            
        case 16384:
            return 0x50 | ((address >> 8) & 7);

        default:
            return 0;
    }
}

#endif


