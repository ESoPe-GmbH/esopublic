/**
 * @file pmod.h
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * @author Tim Koczwara
 * 
 * @brief 
 * 
 * @version 1.00 (19.02.2024)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_PMOD_H_
#define __MODULE_PMOD_H_

#include "module_public.h"
#include "module/enum/function_return.h"
#include "module/comm/i2c/i2c.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief 
 * 
 */
typedef enum pmod_interface_e
{
    /// @brief Default undefined value for the PMOD interfaces.
    PMOD_INTERFACE_UNDEFINED = 0,
    /// @brief PMOD interface is for GPIO.
    PMOD_INTERFACE_GPIO,
    /// @brief PMOD interface is SPI.
    PMOD_INTERFACE_SPI,
    /// @brief PMOD interface is I2C.
    PMOD_INTERFACE_I2C,
    /// @brief PMOD interface is for UART.
    PMOD_INTERFACE_UART,
    /// @brief PMOD interface is for I2S.
    PMOD_INTERFACE_I2S,
    // TODO: H-Bridge
    // TODO: Dual H-Bridge
    /// @brief Limiter for the PMOD interfaces
    PMOD_INTERFACE_MAX
}PMOD_INTERFACE_T;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief 
 * 
 */
typedef struct pmod_s
{
    /// @brief indicates the interface defined in this structure. Needs to be set expicitly, since @c PMOD_INTERFACE_UNDEFINED is the default.
    PMOD_INTERFACE_T interface;
    /// @brief By default PMOD uses 6-pins, if extended, 12 pins are used.
    bool is_extended;
    /// @brief Interface index needed for mcu initialization for SPI, I2C and UART.
    uint8_t interface_num;

    union
    {
        /// @brief Pin 1 of PMOD interface. IO1 function in case of @c interface = @c PMOD_INTERFACE_GPIO.
        MCU_IO_PIN gpio_io1;
        /// @brief Pin 1 of PMOD interface. CS function in case of @c interface = @c PMOD_INTERFACE_SPI.
        MCU_IO_PIN spi_cs;
        /// @brief Pin 1 of PMOD interface. Interrupt function in case of @c interface = @c PMOD_INTERFACE_I2C.
        MCU_IO_PIN i2c_int;
        /// @brief Pin 1 of PMOD interface. CTS function in case of @c interface = @c PMOD_INTERFACE_UART.
        MCU_IO_PIN uart_cts;
        /// @brief Pin 1 of PMOD interface. Left channel / Right channel select function in case of @c interface = @c PMOD_INTERFACE_I2S.
        MCU_IO_PIN i2s_lrclock;
    };

    union
    {
        /// @brief Pin 2 of PMOD interface. IO2 function in case of @c interface = @c PMOD_INTERFACE_GPIO.
        MCU_IO_PIN gpio_io2;
        /// @brief Pin 2 of PMOD interface. Master-Out-Slave-In function in case of @c interface = @c PMOD_INTERFACE_SPI.
        MCU_IO_PIN spi_mosi;
        /// @brief Pin 2 of PMOD interface. Reset function in case of @c interface = @c PMOD_INTERFACE_I2C.
        MCU_IO_PIN i2c_reset;
        /// @brief Pin 2 of PMOD interface. TX (Host -> Peripheral) function in case of @c interface = @c PMOD_INTERFACE_UART.
        MCU_IO_PIN uart_txd;
        /// @brief Pin 2 of PMOD interface. Output data to DAC function in case of @c interface = @c PMOD_INTERFACE_I2S.
        MCU_IO_PIN i2s_dac;
    };

    union
    {
        /// @brief Pin 3 of PMOD interface. IO3 function in case of @c interface = @c PMOD_INTERFACE_GPIO.
        MCU_IO_PIN gpio_io3;
        /// @brief Pin 3 of PMOD interface. Master-In-Slave-Out function in case of @c interface = @c PMOD_INTERFACE_SPI.
        MCU_IO_PIN spi_miso;
        /// @brief Pin 3 of PMOD interface. SCL (clock) function in case of @c interface = @c PMOD_INTERFACE_I2C.
        MCU_IO_PIN i2c_scl;
        /// @brief Pin 3 of PMOD interface. RX (Peripheral -> Host) function in case of @c interface = @c PMOD_INTERFACE_UART.
        MCU_IO_PIN uart_rxd;
        /// @brief Pin 3 of PMOD interface. Input data from ADC function in case of @c interface = @c PMOD_INTERFACE_I2S.
        MCU_IO_PIN i2s_adc;
    };

    union
    {
        /// @brief Pin 4 of PMOD interface. IO4 function in case of @c interface = @c PMOD_INTERFACE_GPIO.
        MCU_IO_PIN gpio_io4;
        /// @brief Pin 4 of PMOD interface. SCK (clock) function in case of @c interface = @c PMOD_INTERFACE_SPI.
        MCU_IO_PIN spi_sck;
        /// @brief Pin 4 of PMOD interface. SDA (data) function in case of @c interface = @c PMOD_INTERFACE_I2C.
        MCU_IO_PIN i2c_sda;
        /// @brief Pin 4 of PMOD interface. RTS function in case of @c interface = @c PMOD_INTERFACE_UART.
        MCU_IO_PIN uart_rts;
        /// @brief Pin 4 of PMOD interface. Serial bit clock function in case of @c interface = @c PMOD_INTERFACE_I2S.
        MCU_IO_PIN i2s_bclk;
    };

    union
    {
        /// @brief Pin 7 of extended PMOD interface. IO5 function in case of @c interface = @c PMOD_INTERFACE_GPIO.
        MCU_IO_PIN gpio_io5;
        /// @brief Pin 7 of extended PMOD interface. Interrupt function in case of @c interface = @c PMOD_INTERFACE_SPI.
        MCU_IO_PIN spi_int;
        /// @brief Pin 7 of extended PMOD interface. GPIO function in case of @c interface = @c PMOD_INTERFACE_I2C.
        MCU_IO_PIN i2c_io5;
        /// @brief Pin 7 of extended PMOD interface. Interrupt function in case of @c interface = @c PMOD_INTERFACE_UART.
        MCU_IO_PIN uart_int;
        /// @brief Pin 7 of extended PMOD interface. GPIO function in case of @c interface = @c PMOD_INTERFACE_I2S.
        MCU_IO_PIN i2s_io5;
    };

    union
    {
        /// @brief Pin 8 of extended PMOD interface. IO6 function in case of @c interface = @c PMOD_INTERFACE_GPIO.
        MCU_IO_PIN gpio_io6;
        /// @brief Pin 8 of extended PMOD interface. Reset function in case of @c interface = @c PMOD_INTERFACE_SPI.
        MCU_IO_PIN spi_reset;
        /// @brief Pin 8 of extended PMOD interface. GPIO function in case of @c interface = @c PMOD_INTERFACE_I2C.
        MCU_IO_PIN i2c_io6;
        /// @brief Pin 8 of extended PMOD interface. Reset function in case of @c interface = @c PMOD_INTERFACE_UART.
        MCU_IO_PIN uart_reset;
        /// @brief Pin 8 of extended PMOD interface. GPIO function in case of @c interface = @c PMOD_INTERFACE_I2S.
        MCU_IO_PIN i2s_io6;
    };

    union
    {
        /// @brief Pin 9 of extended PMOD interface. IO7 function in case of @c interface = @c PMOD_INTERFACE_GPIO.
        MCU_IO_PIN gpio_io7;
        /// @brief Pin 9 of extended PMOD interface. Chip Select 2 function in case of @c interface = @c PMOD_INTERFACE_SPI.
        MCU_IO_PIN spi_cs2;
        /// @brief Pin 9 of extended PMOD interface. GPIO function in case of @c interface = @c PMOD_INTERFACE_I2C.
        MCU_IO_PIN i2c_io7;
        /// @brief Pin 9 of extended PMOD interface. GPIO function in case of @c interface = @c PMOD_INTERFACE_UART.
        MCU_IO_PIN uart_io7;
        /// @brief Pin 9 of extended PMOD interface. Master clock function in case of @c interface = @c PMOD_INTERFACE_I2S.
        MCU_IO_PIN i2s_mclk;
    };

    union
    {
        /// @brief Pin 10 of extended PMOD interface. IO8 function in case of @c interface = @c PMOD_INTERFACE_GPIO.
        MCU_IO_PIN gpio_io8;
        /// @brief Pin 10 of extended PMOD interface. Chip Select 3 function in case of @c interface = @c PMOD_INTERFACE_SPI.
        MCU_IO_PIN spi_cs3;
        /// @brief Pin 10 of extended PMOD interface. GPIO function in case of @c interface = @c PMOD_INTERFACE_I2C.
        MCU_IO_PIN i2c_io8;
        /// @brief Pin 10 of extended PMOD interface. GPIO function in case of @c interface = @c PMOD_INTERFACE_UART.
        MCU_IO_PIN uart_io8;
        /// @brief Pin 10 of extended PMOD interface. GPIO function in case of @c interface = @c PMOD_INTERFACE_I2S.
        MCU_IO_PIN i2s_io8;
    };

    union
    {
        struct
        {
            /// @brief Handler for SPI (CS) interface in case of @c interface = @c PMOD_INTERFACE_SPI.
            mcu_spi_t spi;
            /// @brief Handler for SPI (CS2) interface in case of @c interface = @c PMOD_INTERFACE_SPI.
            mcu_spi_t spi2;
            /// @brief Handler for SPI (CS3) interface in case of @c interface = @c PMOD_INTERFACE_SPI.
            mcu_spi_t spi3;
        };
#if MODULE_ENABLE_COMM_I2C
        /// @brief Handler for I2C interface in case of @c interface = @c PMOD_INTERFACE_I2C.
        i2c_t i2c;
#endif
        /// @brief Handler for I2C interface in case of @c interface = @c PMOD_INTERFACE_UART.
        mcu_uart_t uart;

        // TODO: I2S
    };
    /// @brief Device handler after initialization, can be either @c mcu_spi_t, @c mcu_i2c_t or @c mcu_uart_t.
    void* device;
}pmod_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External function
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the PMOD interface based on its configuration.
 * Internally calls spi / i2c / uart initializations based on the declared interface of the pmod.
 * The interface enumeration and I/O need to be set before calling this function.
 * 
 * @param pmod              Pointer to the PMOD to initialize. The interface enumeration and I/O need to be set before calling this function.
 * @return FUNCTION_RETURN  FUNCTION_RETURN_OK on success, other on error.
 */
FUNCTION_RETURN pmod_init(pmod_t* pmod);
/**
 * @brief De-initialized the PMOD interface. Does not de-allocate the pmod pointer.
 * 
 * @param pmod              Pointer to the PMOD interface that was initialized.
 */
void pmod_free(pmod_t* pmod);

#endif /* __MODULE_PMOD_H_ */