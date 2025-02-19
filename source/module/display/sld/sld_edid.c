
/**
 * @file sld_edid.c
 **/

#include "sld_edid.h"
#if MODULE_ENABLE_DISPLAY && DISPLAY_ENABLE_SLD
#include "module/comm/dbg.h"
#include "module/eeprom/eeprom_i2c.h"
#include "module/util/assert.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Extract PCLK from eeid
 * 
 * @param eeid      Pointer to the EEID data
 * @param index     Offset of the PCLK in eeid
 * @return          PCLK in MHz 
 */
static uint32_t _pclk_from_edid(const uint8_t* eeid, uint8_t index);
/**
 * @brief Extract 16-Bit unsigned value from eeid
 * 
 * @param eeid      Pointer to the EEID data
 * @param index     Offset in eeid
 * @return          16-Bit value
 */
static uint16_t _uint16_from_edid(const uint8_t* eeid, uint8_t index);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN sld_edid_read(i2c_t* i2c, sld_edid_t* edid)
{
    ASSERT_RET_NOT_NULL(i2c, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(edid, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    uint8_t edid_data[0x1C] = {0};
    FUNCTION_RETURN ret;
    // Create handle to read the eeprom and free it after reading again
    const eeprom_i2c_config_t eeprom_config = EEPROM_I2C_M24C01(i2c, 0, 0, 0);
    eeprom_device_t eeprom = eeprom_i2c_init(&eeprom_config);
    ASSERT_RET_NOT_NULL(eeprom, NO_ACTION, FUNCTION_RETURN_INSUFFICIENT_MEMORY);
    ret = eeprom_i2c_read(eeprom, 0, edid_data, sizeof(edid_data));
    eeprom_i2c_free(eeprom);
    
    ASSERT_RET(ret == FUNCTION_RETURN_OK, NO_ACTION, FUNCTION_RETURN_READ_ERROR, "Error reading eeprom\n");
    ASSERT_RET(edid_data[0] == 0, NO_ACTION, FUNCTION_RETURN_READ_ERROR, "Invalid EDID data on eeprom\n");
    // [0] is 0
    // [1] = TFT
    edid->tft = edid_data[1];
    // [2] = Touch
    edid->touch = edid_data[2];
    // [5:3] = Manufacturer
    edid->manufacturer[0] = edid_data[3];
    edid->manufacturer[1] = edid_data[4];
    edid->manufacturer[2] = 0;
    // [7:6] = Physical size in format xxh.xxh
    string_nprintf(edid->screen_diagonal, sizeof(edid->screen_diagonal), "%x.%x", (uint32_t)edid_data[6], (uint32_t)edid_data[7]);
    // [9:8] = Horizontal resolution in pixel
    edid->rgb.h_res = _uint16_from_edid(edid_data, 8);
    // [11:10] = Vertical resolution in pixel
    edid->rgb.v_res = _uint16_from_edid(edid_data, 10);
    // [13:12] = Pixel clock in Hz
    edid->rgb.pclk_hz = _pclk_from_edid(edid_data, 12);
    // [14] = Color depth
    edid->rgb.color_depth = edid_data[14]; 
    // [16:15] = Horizontal back porch
    edid->rgb.hsync_back_porch = _uint16_from_edid(edid_data, 15);
    // [17] = Horizontal sync width
    edid->rgb.hsync_pulse_width = edid_data[17];
    // [19:18] = Horizontal front porch
    edid->rgb.hsync_front_porch = _uint16_from_edid(edid_data, 18);
    // [21:20] = Vertical back porch
    edid->rgb.vsync_back_porch = _uint16_from_edid(edid_data, 20);
    // [22] = Vertical sync width
    edid->rgb.vsync_pulse_width = edid_data[22];
    // [24:23] = Vertical front porch
    edid->rgb.vsync_front_porch = _uint16_from_edid(edid_data, 23);
    // [25] = Polarity mode
    edid->rgb.flags.hsync_polarity = (edid_data[25] & 0x01) == 0x01;
    edid->rgb.flags.vsync_polarity = (edid_data[25] & 0x02) == 0x02;
    edid->rgb.flags.de_polarity = (edid_data[25] & 0x04) == 0x04;
    edid->rgb.flags.hsync_phase = (edid_data[25] & 0x08) == 0x08;
    edid->rgb.flags.vsync_phase = (edid_data[25] & 0x10) == 0x10;
    edid->rgb.flags.de_phase = (edid_data[25] & 0x20) == 0x20;
    edid->rgb.flags.pixel_invert = (edid_data[25] & 0x40) == 0x40;
    edid->rgb.flags.de_mode = (edid_data[25] & 0x80) == 0x80;
    // [26] Rotation
    edid->rgb.rotation.display_mirror_x = (edid_data[26] & 0x01) == 0x01;
    edid->rgb.rotation.display_mirror_y = (edid_data[26] & 0x02) == 0x02;
    edid->rgb.rotation.swap_xy = (edid_data[26] & 0x04) == 0x04;
    // Bit 3: 0
    edid->rgb.rotation.touch_mirror_x = (edid_data[26] & 0x10) == 0x10;
    edid->rgb.rotation.touch_mirror_y = (edid_data[26] & 0x20) == 0x20;
    edid->rgb.rotation.touch_swap_xy = (edid_data[26] & 0x40) == 0x40;
    // Bit 7: 0
    return FUNCTION_RETURN_OK;
}

void sld_debug_print(sld_edid_t* edid)
{
    DBG_INFO("TFT: %u, Touch: %u\n", edid->tft, edid->touch);
    DBG_INFO("Manufacturer: %s\n", edid->manufacturer);
    DBG_INFO("Screen Diagonal: %s\"\n", edid->screen_diagonal);
    DBG_INFO("Resolution: %ux%u\n", edid->rgb.h_res, edid->rgb.v_res);
    DBG_INFO("PCLK: %u Hz\n", edid->rgb.pclk_hz);
    DBG_INFO("Color Depth: %u\n", edid->rgb.color_depth);
    DBG_INFO("HSync: %u, %u, %u\n", edid->rgb.hsync_back_porch, edid->rgb.hsync_pulse_width, edid->rgb.hsync_front_porch);
    DBG_INFO("VSync: %u, %u, %u\n", edid->rgb.vsync_back_porch, edid->rgb.vsync_pulse_width, edid->rgb.vsync_front_porch);
    DBG_INFO("Polarity: HSync: %u, VSync: %u, DE: %u\n", edid->rgb.flags.hsync_polarity, edid->rgb.flags.vsync_polarity, edid->rgb.flags.de_polarity);
    DBG_INFO("Phase: HSync: %u, VSync: %u, DE: %u\n", edid->rgb.flags.hsync_phase, edid->rgb.flags.vsync_phase, edid->rgb.flags.de_phase);
    DBG_INFO("Invert: %u, DE Mode: %u\n", edid->rgb.flags.pixel_invert, edid->rgb.flags.de_mode);
    DBG_INFO("Rotation: Display: %u, %u, %u, Touch: %u, %u, %u\n", edid->rgb.rotation.display_mirror_x, edid->rgb.rotation.display_mirror_y, edid->rgb.rotation.swap_xy, edid->rgb.rotation.touch_mirror_x, edid->rgb.rotation.touch_mirror_y, edid->rgb.rotation.touch_swap_xy);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t _pclk_from_edid(const uint8_t* eeid, uint8_t index)
{
    // uint32_t tmp = ((uint32_t)bcd_decode_uint8(eeid[index + 1]));
    // if(tmp > 10)
    // {
    //     tmp *= 10000;
    // }
    // else
    // {
    //     tmp *= 100000;
    // }
    // return ((uint32_t)bcd_decode_uint8(eeid[index]) * 1000000) + tmp;

    return (uint32_t)eeid[index] * 1000000 + eeid[index + 1] * 100000;
}

static uint16_t _uint16_from_edid(const uint8_t* eeid, uint8_t index)
{
    return ((uint16_t)eeid[index] * 256) + eeid[index + 1];
}

#endif // MODULE_ENABLE_SLD_EDID
