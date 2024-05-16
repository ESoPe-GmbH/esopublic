/**
 * @file pmod.c
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "pmod.h"
#include "assert.h"
#include "module/comm/dbg.h"

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


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN pmod_init(pmod_t* pmod)
{
    ASSERT_RET_NOT_NULL(pmod, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    switch(pmod->interface)
    {
        case PMOD_INTERFACE_UNDEFINED:
            DBG_ERROR("PMOD interface was not defined!\n");
            return FUNCTION_RETURN_PARAM_ERROR;

#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0
        case PMOD_INTERFACE_SPI:
            pmod->device = mcu_spi_init(pmod->interface_num, pmod->spi_mosi, pmod->spi_miso, pmod->spi_sck, pmod->spi_cs);
            pmod->spi = pmod->device;
            
            if(pmod->is_extended)
            {
                if(pmod->spi_cs2 != PIN_NONE)
                {
                    pmod->spi2 = mcu_spi_init(pmod->interface_num, pmod->spi_mosi, pmod->spi_miso, pmod->spi_sck, pmod->spi_cs2);
                }
                if(pmod->spi_cs3 != PIN_NONE)
                {
                    pmod->spi3 = mcu_spi_init(pmod->interface_num, pmod->spi_mosi, pmod->spi_miso, pmod->spi_sck, pmod->spi_cs3);
                }
                
                // TODO: INT and Reset
            }
            break;
#endif

        case PMOD_INTERFACE_UART:
        {
#if MCU_PERIPHERY_DEVICE_COUNT_UART
            mcu_uart_hw_config_t hw_config = 
            {
                .unit = pmod->interface_num,
                .io_tx = pmod->uart_txd,
                .io_rx = pmod->uart_rxd,
                .io_rts = pmod->uart_rts,
                .io_cts = pmod->uart_cts,
                .receive_buffer_size = 8092,
                .receive_interrupt_level = MCU_INT_LVL_MED,
                .transmit_buffer_size = 8092,
                .transmit_interrupt_level = MCU_INT_LVL_MED
            };
            mcu_uart_config_t config = 
            {
                .mode = MCU_UART_MODE_UART_NO_FLOW_CONTROL,
                .baudrate = 115200,
                .databits = 8,
                .parity = 'N',
                .stopbits = 1,
            };
            pmod->uart = mcu_uart_create(&hw_config, &config);
            pmod->device = pmod->uart;
            if(pmod->is_extended)
            {
                if(pmod->uart_int != PIN_NONE)
                {
                    mcu_io_set_dir(pmod->uart_int, MCU_IO_DIR_IN);
                    // TODO: I/O Interrupt?
                }
                if(pmod->uart_reset != PIN_NONE)
                {
                    mcu_io_set(pmod->uart_reset, 1);
                    mcu_io_set_dir(pmod->uart_reset, MCU_IO_DIR_OUT);
                }
            }
#endif
            break;
        }

#if MODULE_ENABLE_COMM_I2C
        case PMOD_INTERFACE_I2C:
            i2c_init(&pmod->i2c, pmod->interface_num, pmod->i2c_sda, pmod->i2c_scl);
	        i2c_set_frq(&pmod->i2c, 100000);
            pmod->device = &pmod->i2c;
            // TODO: INT and Reset
            break;
#endif

        case PMOD_INTERFACE_I2S:
            // TODO: Implement I2S
            DBG_ERROR("PMOD I2S not implemented yet\n");
            return FUNCTION_RETURN_UNSUPPORTED;

        default:
            DBG_ERROR("Invalid PMOD interface\n");
            return FUNCTION_RETURN_PARAM_ERROR;
    }
    
    return pmod->device ? FUNCTION_RETURN_OK : FUNCTION_RETURN_EXECUTION_ERROR;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void pmod_free(pmod_t* pmod)
{
    ASSERT_RET_NOT_NULL(pmod, NO_ACTION, NO_RETURN);

    switch(pmod->interface)
    {
        case PMOD_INTERFACE_UNDEFINED:
            DBG_ERROR("PMOD interface was not defined!\n");
            return;

#if MCU_PERIPHERY_DEVICE_COUNT_SPI > 0
        case PMOD_INTERFACE_SPI:

            if(pmod->is_extended)
            {
                if(pmod->spi_cs2 != PIN_NONE && pmod->spi2)
                {
                    // Resets GPIO internally
                    mcu_spi_free(pmod->spi2);
                    pmod->spi2 = NULL;
                }
                if(pmod->spi_cs3 != PIN_NONE && pmod->spi3)
                {
                    // Resets GPIO internally
                    mcu_spi_free(pmod->spi3);
                    pmod->spi3 = NULL;
                }
            }
            // Resets GPIO internally
            mcu_spi_free(pmod->spi);
            // Clear device pointer
            pmod->spi = NULL;
            pmod->device = NULL;

            // TODO: INT and Reset
            break;
#endif

        case PMOD_INTERFACE_UART:
        {
#if MCU_PERIPHERY_DEVICE_COUNT_UART > 0
            mcu_uart_free(pmod->uart);
            pmod->uart = NULL;
            pmod->device = NULL;
            // TODO: INT and Reset
#endif
            break;
        }

#if MODULE_ENABLE_COMM_I2C
        case PMOD_INTERFACE_I2C:
            i2c_free(&pmod->i2c);
            pmod->device = NULL;
            // TODO: INT and Reset
            break;
#endif

        case PMOD_INTERFACE_I2S:
            // TODO: Implement I2S
            DBG_ERROR("PMOD I2S not implemented yet\n");
            return;

        default:
            DBG_ERROR("Invalid PMOD interface\n");
            return;
    }
}
