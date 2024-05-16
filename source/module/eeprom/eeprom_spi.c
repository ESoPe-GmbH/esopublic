// Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file eeprom_spi.c
 * @copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "module_public.h"
#if MODULE_ENABLE_EEPROM

#include "eeprom_spi.h"
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
struct eeprom_spi_data_s
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

static FUNCTION_RETURN_T _set_write_enable(const eeprom_spi_config_t* config, bool write_enable);

static FUNCTION_RETURN_T _write_memory(const eeprom_spi_config_t* config, uint16_t address, const uint8_t* data, size_t data_length);

static FUNCTION_RETURN_T _read_memory(const eeprom_spi_config_t* config, uint16_t address, uint8_t* data, size_t data_length);
/**
 * @brief Read the status byte of the EEPROM.
 * 
 * @param config    Pointer to the eeprom config, containing the SPI interface.
 * @param status    Pointer to the byte where the read status should be written to.
 * @return FUNCTION_RETURN_OK on success, other on failure.
 */
static FUNCTION_RETURN_T _read_status(const eeprom_spi_config_t* config, uint8_t* status);
/**
 * @brief Write the status byte of the EEPROM.
 * 
 * @param config    Pointer to the eeprom config, containing the SPI interface.
 * @param status    Status that should be written.
 * @return FUNCTION_RETURN_OK on success, other on failure.
 */
static FUNCTION_RETURN_T _write_status(const eeprom_spi_config_t* config, uint8_t status);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

eeprom_device_t eeprom_spi_init(const eeprom_spi_config_t* config)
{
    DBG_ASSERT(config, NO_ACTION, NULL, "Config should not be NULL\n");
    DBG_ASSERT(config->page_size > 0, NO_ACTION, NULL, "The page size must be set\n");

    eeprom_device_t device = mcu_heap_calloc(1, sizeof(struct eeprom_device_s));
    if(device == NULL)
        return NULL;

    device->context = mcu_heap_calloc(1, sizeof(struct eeprom_spi_data_s));
    if(device->context == NULL)
    {
        mcu_heap_free(device);
        return NULL;
    }

    mcu_spi_set_param(config->spi, MCU_SPI_MODE_0, config->frequency);

    device->config = config;
    device->interface.f_write = eeprom_spi_write;
    device->interface.f_read = eeprom_spi_read;

    return device;
}

void eeprom_spi_free(eeprom_device_t device)
{
    DBG_ASSERT(device, NO_ACTION, NO_RETURN, "Invalid device\n");
    mcu_heap_free(device->context);
    mcu_heap_free(device);
}

FUNCTION_RETURN eeprom_spi_read(eeprom_device_t device, uint16_t address, void* buffer, uint16_t length)
{
    DBG_ASSERT(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid device\n");
    DBG_ASSERT(buffer, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Buffer shall not be NULL\n");
    DBG_ASSERT(length > 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Length cannot be 0\n");
    const eeprom_spi_config_t* config = device->config;
    DBG_ASSERT(address < config->max_size, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid address\n");
    DBG_ASSERT(address + length <= config->max_size, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Data does not fit in eeprom\n");

    struct eeprom_spi_data_s* data = device->context;
    
    FUNCTION_RETURN_T ret;
    uint8_t status;
    uint16_t count = 0;
    uint16_t offset = address;
    uint32_t timestamp;
    do
    {    
        uint16_t l = length - count;

        // Ensure to only read a page size in one operation
        if(l > config->page_size)
            l = config->page_size;
        
        // Wait until EEPROM is ready
        timestamp = system_get_tick_count();
        do
        {
            ret = _read_status(config, &status);

        }while( (status & 0x01) && (system_get_tick_count() - timestamp) < 100);
        ASSERT_RET((status & 0x01) == 0, NO_ACTION, FUNCTION_RETURN_TIMEOUT, "Timeout on EEPROM ready\n");

        ret = _read_memory(config, offset, &((uint8_t*)buffer)[count], l);
        // DBG_INFO("rd offset=%d l=%d buf=%$Q\n", offset, l, &((uint8_t*)buffer)[count]);

        data->timestamp_last_write = system_get_tick_count();

        count += l;
        offset += l;

    } while (ret == FUNCTION_RETURN_OK && count < length);

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN eeprom_spi_write(eeprom_device_t device, uint16_t address, const void* buffer, uint16_t length)
{
    DBG_ASSERT(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid device\n");
    DBG_ASSERT(buffer, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Buffer shall not be NULL\n");
    DBG_ASSERT(length > 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Length cannot be 0\n");
    const eeprom_spi_config_t* config = device->config;
    DBG_ASSERT(address < config->max_size, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid address\n");
    DBG_ASSERT((address & (config->page_size - 1)) == 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid address alignment.\n");
    DBG_ASSERT(address + length <= config->max_size, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Data does not fit in eeprom\n");

    struct eeprom_spi_data_s* data = device->context;

    FUNCTION_RETURN_T ret;

    uint8_t* page_buffer = mcu_heap_calloc(1, config->page_size);
    ASSERT_RET(page_buffer, NO_ACTION, FUNCTION_RETURN_INSUFFICIENT_MEMORY, "Cannot create page buffer\n");

    uint8_t status;
    uint16_t count = 0;
    uint16_t offset = address;
    uint32_t timestamp;
    do
    {    
        uint8_t* ptr = &((uint8_t*)buffer)[count];
        uint16_t l = length - count;

        // Ensure to only write a page size in one operation
        if(l > config->page_size)
            l = config->page_size;
   
        // Wait until EEPROM is ready
        timestamp = system_get_tick_count();
        do
        {
            ret = _read_status(config, &status);

        }while( (status & 0x01) && (system_get_tick_count() - timestamp) < 100);
        ret = FUNCTION_RETURN_TIMEOUT;
        ASSERT_RET((status & 0x01) == 0, goto exit, FUNCTION_RETURN_TIMEOUT, "Timeout on EEPROM ready\n");

        // Enable the writing to the EEPROM
        ret = _set_write_enable(config, true);
        ASSERT_RET(ret == FUNCTION_RETURN_OK, goto exit, FUNCTION_RETURN_WRITE_ERROR, "Error enabling the writing\n");
   
        // Wait until EEPROM is ready
        timestamp = system_get_tick_count();
        do
        {
            ret = _read_status(config, &status);

        }while( (status & 0x01) && (system_get_tick_count() - timestamp) < 100);
        ret = FUNCTION_RETURN_TIMEOUT;
        ASSERT_RET((status & 0x01) == 0, goto exit, FUNCTION_RETURN_TIMEOUT, "Timeout on EEPROM ready\n");
        
        // Write data to the EEPROM
        // DBG_INFO("wr offset=%d l=%d buf=%$Q status=%02x\n", offset, l, ptr, status);
        ret = _write_memory(config, offset, ptr, l);
        ASSERT_RET(ret == FUNCTION_RETURN_OK, goto exit, FUNCTION_RETURN_WRITE_ERROR, "Error writing the EEPROM\n");

        data->timestamp_last_write = system_get_tick_count();

        count += l;
        offset += l;

    } while (ret == FUNCTION_RETURN_OK && count < length);

    ret = FUNCTION_RETURN_OK;
exit:
    // Free the page buffer
    mcu_heap_free(page_buffer);
    // Disable the writing
    ret = _set_write_enable(config, false);
    return ret;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static FUNCTION_RETURN_T _set_write_enable(const eeprom_spi_config_t* config, bool write_enable)
{
    // Write RDSR and read the status byte
    mcu_spi_transaction_t t = 
    {
        .cmd = 0x06,
        .cmd_length = 1
    };

    if(!write_enable)
    {
        t.cmd = 0x04;
    }

    mcu_spi_transaction_start(config->spi);
    mcu_spi_transaction_add(config->spi, t);
    mcu_spi_transaction_end(config->spi);

    return FUNCTION_RETURN_OK;
}

static FUNCTION_RETURN_T _write_memory(const eeprom_spi_config_t* config, uint16_t address, const uint8_t* data, size_t data_length)
{
    // Write RDSR and read the status byte
    mcu_spi_transaction_t t = 
    {
        .cmd = 0x02,
        .cmd_length = 1,
        .addr = address & 0xFF,
        .addr_length = 1,
        .w_buf = data,
        .w_buf_length = data_length
    };

    if(address > 0xFF)
    {
        t.cmd |= 0x08;
    }

    mcu_spi_transaction_start(config->spi);
    mcu_spi_transaction_add(config->spi, t);
    mcu_spi_transaction_end(config->spi);

    return FUNCTION_RETURN_OK;
}

static FUNCTION_RETURN_T _read_memory(const eeprom_spi_config_t* config, uint16_t address, uint8_t* data, size_t data_length)
{
    // Write RDSR and read the status byte
    mcu_spi_transaction_t t = 
    {
        .cmd = 0x03,
        .cmd_length = 1,
        .addr = address & 0xFF,
        .addr_length = 1,
        .r_buf = data,
        .r_buf_length = data_length
    };

    if(address > 0xFF)
    {
        t.cmd |= 0x08;
    }

    mcu_spi_transaction_start(config->spi);
    mcu_spi_transaction_add(config->spi, t);
    mcu_spi_transaction_end(config->spi);

    return FUNCTION_RETURN_OK;
}

static FUNCTION_RETURN_T _read_status(const eeprom_spi_config_t* config, uint8_t* status)
{
    // Write RDSR and read the status byte
    mcu_spi_transaction_t t = 
    {
        .cmd = 0x05,
        .cmd_length = 1,
        .r_buf = status,
        .r_buf_length = 1
    };

    mcu_spi_transaction_start(config->spi);
    mcu_spi_transaction_add(config->spi, t);
    mcu_spi_transaction_end(config->spi);

    return FUNCTION_RETURN_OK;
}

static __unused FUNCTION_RETURN_T _write_status(const eeprom_spi_config_t* config, uint8_t status)
{
    // Write RDSR and read the status byte
    mcu_spi_transaction_t t = 
    {
        .cmd = 0x01,
        .cmd_length = 1,
        .w_data = {status, 0},
        .w_buf_length = 1,
        .flags = MCU_SPI_TRANS_FLAGS_TXDATA
    };

    mcu_spi_transaction_start(config->spi);
    mcu_spi_transaction_add(config->spi, t);
    mcu_spi_transaction_end(config->spi);

    return FUNCTION_RETURN_OK;
}

#endif


