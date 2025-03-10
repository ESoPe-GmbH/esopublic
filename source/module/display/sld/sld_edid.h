/**
 * @file sld_edid.h
 * @author Tim Koczwara
 * 
 * @brief 
 * 
 * @version 1.00 (17.02.2025)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_SLD_EDID_H_
#define __MODULE_SLD_EDID_H_

#include "module_public.h"
#if MODULE_ENABLE_DISPLAY && DISPLAY_ENABLE_SLD
#include "module/comm/i2c/i2c.h"
#include "module/enum/function_return.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief TFT interface
typedef enum sld_tft_interface_e
{
    /// @brief Invalid value
    SLD_TFT_INTERFACE_INVALID = 0,
    /// @brief Data parallel interface (RGB)
    SLD_TFT_INTERFACE_DPI = 1,
    /// @brief Display serial interface (DSI)
    SLD_TFT_INTERFACE_DSI = 2
}SLD_TFT_INTERFACE_T;

/// @brief Touch interface
typedef enum sld_touch_interface_e
{
    /// @brief No touch
    SLD_TOUCH_INTERFACE_NONE = 0,
    /// @brief Capacitive touch interface (CTP)
    SLD_TOUCH_INTERFACE_CAPACITIVE = 1,
}SLD_TOUCH_INTERFACE_T;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct sld_edid_s
{
    /// @brief TFT interface
    SLD_TFT_INTERFACE_T tft;
    /// @brief Touch interface
    SLD_TOUCH_INTERFACE_T touch;
    /// @brief Manufacturer of the display
    char manufacturer[3];
    /// Screen diagonal of the display in inch as read from EEPROM e.g. "2.4"
    char screen_diagonal[6];

    /**
     * @brief For displays using the RGB interface.
     * @verbatim
     *                                                 Total Width
     *                             <--------------------------------------------------->
     *                       HSYNC width HBP             Active Width                HFP
     *                             <---><--><--------------------------------------><--->
     *                         ____    ____|_______________________________________|____|
     *                             |___|   |                                       |    |
     *                                     |                                       |    |
     *                         __|         |                                       |    |
     *            /|\    /|\  |            |                                       |    |
     *             | VSYNC|   |            |                                       |    |
     *             |Width\|/  |__          |                                       |    |
     *             |     /|\     |         |                                       |    |
     *             |  VBP |      |         |                                       |    |
     *             |     \|/_____|_________|_______________________________________|    |
     *             |     /|\     |         | / / / / / / / / / / / / / / / / / / / |    |
     *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
     *    Total    |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
     *    Height   |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
     *             |Active|      |         |/ / / / / / / / / / / / / / / / / / / /|    |
     *             |Heigh |      |         |/ / / / / / Active Display Area / / / /|    |
     *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
     *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
     *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
     *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
     *             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
     *             |     \|/_____|_________|_______________________________________|    |
     *             |     /|\     |                                                      |
     *             |  VFP |      |                                                      |
     *            \|/    \|/_____|______________________________________________________|
     * @endverbatim
     */
    struct
    {
        /// Horizontal resolution, i.e. the number of pixels in a line
        uint16_t h_res;             
        /// Vertical resolution, i.e. the number of lines in the frame
        uint16_t v_res;
        /// Frequency of pixel clock in Hz
        uint32_t pclk_hz;       
        /// @brief Color depth in bit (16-Bit, 24-Bit)
        uint8_t color_depth;
        /// Horizontal back porch, number of PCLK between hsync and start of line active data
        uint16_t hsync_back_porch;
        /// Horizontal sync width, unit: PCLK period
        uint8_t hsync_pulse_width;
        /// Horizontal front porch, number of PCLK between the end of active data and the next hsync
        uint16_t hsync_front_porch;
        /// Vertical back porch, number of invalid lines between vsync and start of frame
        uint16_t vsync_back_porch;
        /// Vertical sync width, unit: number of lines
        uint8_t vsync_pulse_width;
        /// Vertical front porch, number of invalid lines between the end of frame and the next vsync
        uint16_t vsync_front_porch;
        struct 
        {
            /// The hsync polarity (0: Negative, 1: Positive)
            uint8_t hsync_polarity: 1;  
            /// The vsync polarity (0: Negative, 1: Positive)
            uint8_t vsync_polarity: 1;
            /// The de polarity (0: Negative, 1: Positive)
            uint8_t de_polarity: 1;
            /// The hsync phase (0: Rising edge, 1: Falling edge)
            uint8_t hsync_phase: 1;
            /// The vsync phase (0: Rising edge, 1: Falling edge)
            uint8_t vsync_phase: 1;
            /// The de phase (0: Rising edge, 1: Falling edge)
            uint8_t de_phase: 1;
            /// Is 1 if all pixels are inverted
            uint8_t pixel_invert: 1;
            /// DE_mode (0: DE enabled,1: Combined Sync)
            uint8_t de_mode: 1;
        } flags;
        struct 
        {
            /// X-axis mirror of the display
            uint8_t display_mirror_x: 1;
            /// Y-axis mirror of the display
            uint8_t display_mirror_y: 1;
            /// X/Y are swapped of the display
            uint8_t swap_xy: 1;
            /// X-axis mirror of the touch
            uint8_t touch_mirror_x: 1;
            /// Y-axis mirror of the touch
            uint8_t touch_mirror_y: 1;
            /// X/Y are swapped of the touch
            uint8_t touch_swap_xy: 1;
        } rotation;
    } rgb;
} sld_edid_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Read the EDID data from the eeprom.
 * 
 * @param i2c       Pointer to the I2C device
 * @param edid      Pointer to the EDID data
**/
FUNCTION_RETURN sld_edid_read(i2c_t* i2c, sld_edid_t* edid);
/**
 * @brief Print the EDID data to the debug console.
 * 
 * @param edid    Pointer to the EDID data
 */
void sld_debug_print(sld_edid_t* edid);

#endif // MODULE_ENABLE_SLD_EDID

#endif /* __MODULE_SLD_EDID_H_ */