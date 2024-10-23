/**
 * 	@file display_sld.c
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten
 *  @author 	Tim Koczwara
 *
 *  @brief	Display driver implementation for SLD.
 *			
 *  @version	1.00 (19.08.2022)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#include "module.h"

#if MODULE_ENABLE_DISPLAY && DISPLAY_ENABLE_SLD

#include "display_sld.h"
#include "../internal/display_internal.h"

#include "module/comm/dbg.h"
#include "module/convert/convert.h"
#include "module/util/assert.h"

#if MODULE_ENABLE_EEPROM
#include "module/eeprom/eeprom_i2c.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structures
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
static uint32_t _pclk_from_eeid(const uint8_t* eeid, uint8_t index);
/**
 * @brief Extract 16-Bit unsigned value from eeid
 * 
 * @param eeid      Pointer to the EEID data
 * @param index     Offset in eeid
 * @return          16-Bit value
 */
static uint16_t _uint16_from_eeid(const uint8_t* eeid, uint8_t index);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MODULE_ENABLE_EEPROM
display_sld_handle_t display_sld_init_hardware(const display_sld_hardware_t* config)
{
    DBG_ASSERT(config, NO_ACTION, NULL, "config cannot be NULL\n");
    display_sld_handle_t device = mcu_heap_calloc(1, sizeof(display_sld_t));
    DBG_ASSERT(device, NO_ACTION, NULL, "Cannot create display device pointer\n");

    uint8_t eeid[0x1C] = {0};
    FUNCTION_RETURN ret;
    // Create handle to read the eeprom and free it after reading again
    const eeprom_i2c_config_t eeprom_config = EEPROM_I2C_M24C01(config->touch.i2c, 0, 0, 0);
    eeprom_device_t eeprom = eeprom_i2c_init(&eeprom_config);
    ret = eeprom_i2c_read(eeprom, 0, eeid, sizeof(eeid));
    eeprom_i2c_free(eeprom);

    DBG_ASSERT(ret == FUNCTION_RETURN_OK, goto error, NULL, "Failed to read eeprom\n");

    device->backlight = mcu_pwm_create(&config->backlight, NULL);

    device->display = display_sld_init(&config->display, eeid, sizeof(eeid));


    if(eeid[2] == 1)
    {
#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
        // Capacitive touch is used
        lcd_touch_device_handle_t touch_device = st1633i_create(&config->touch);
        if(touch_device)
        {
            // TODO: Set flags based on display
            struct lcd_touch_config_s touch_config = 
            {
                .flags = {.mirror_x = true, .mirror_y = true, .swap_xy = false},
                .x_max = device->display->device_config.rgb.h_res,
                .y_max = device->display->device_config.rgb.v_res
            };
            lcd_touch_create(touch_device, &st1633i_lcd_touch_interface, &touch_config, &device->touch);
        }
#endif
    }
    // Else: No touch

    display_device_reset(device->display);
    display_device_init(device->display);
    // TODO: Set display orientation based on display in use.
    display_device_mirror(device->display, true, false);
    display_device_swap_xy(device->display, true);


    return device;
error:
    if(device->display)
    {
        // TODO: Uninitialize display
        mcu_heap_free(device->display);
        device->display = NULL;
    }

    if(device->touch)
    {
        lcd_touch_del(device->touch);
        lcd_touch_free(&device->touch);
    }

    mcu_heap_free(device);
    return NULL;
}
#endif

display_handle_t display_sld_init(const display_common_hardware_t* config, const uint8_t *eeid, uint8_t eeid_length)
{
    DBG_ASSERT(config, NO_ACTION, NULL, "config cannot be NULL\n");
    DBG_ASSERT(eeid, NO_ACTION, NULL, "eeid cannot be NULL\n");
    DBG_ASSERT(eeid_length > 27, NO_ACTION, NULL, "eeid_length must be greater than 27\n");
    bool has_touch;
    display_handle_t device = mcu_heap_calloc(1, sizeof(struct display_data_s));
    DBG_ASSERT(device, NO_ACTION, NULL, "Cannot create display device pointer\n");

    DBG_VERBOSE("Create config\n");
#if MCU_TYPE == MCU_ESP32
    device->mcu_config.rgb.esp32s3.sram_trans_align = 4;
    device->mcu_config.rgb.esp32s3.psram_trans_align = 64;
    device->mcu_config.rgb.esp32s3.flags.fb_in_psram = true;
    device->mcu_config.rgb.esp32s3.flags.relax_on_idle = false;
#endif
    //[0] is 0
    ASSERT_RET(eeid[1] == 1, goto error, NULL, "Display with DPI needed\n");
    has_touch = eeid[2] > 0;
    ASSERT_RET(eeid[3] == 'S' && eeid[4] == 'M', goto error, NULL, "Invalid EEID content\n");
    // [5] = Manufacturer revision
    // [7:6] = Physical size in format xxh.xxh
    DBG_INFO("Initialize %x.%x\" Display with%s touch\n", (uint32_t)eeid[6], (uint32_t)eeid[7], has_touch ? "" : "out");
    device->device_config.rgb.h_res = _uint16_from_eeid(eeid, 8);
    device->device_config.rgb.v_res = _uint16_from_eeid(eeid, 10);
    device->device_config.rgb.pclk_hz = _pclk_from_eeid(eeid, 12);
    // [14] = Color depth
    device->device_config.rgb.hsync_back_porch = _uint16_from_eeid(eeid, 15);
    device->device_config.rgb.hsync_pulse_width = eeid[17];
    device->device_config.rgb.hsync_front_porch = _uint16_from_eeid(eeid, 18);
    device->device_config.rgb.vsync_back_porch = _uint16_from_eeid(eeid, 20);
    device->device_config.rgb.vsync_pulse_width = eeid[22];
    device->device_config.rgb.vsync_front_porch = _uint16_from_eeid(eeid, 23);
    device->device_config.rgb.flags.hsync_idle_low = (eeid[25] & 0x01) == 0x01;
    device->device_config.rgb.flags.vsync_idle_low = (eeid[25] & 0x02) == 0x02;
    device->device_config.rgb.flags.de_idle_high = (eeid[25] & 0x04) == 0;
    device->device_config.rgb.flags.pclk_active_neg = (eeid[25] & 0x20) == 0x20;

    device->mcu = display_mcu_init(config, device);
    ASSERT_RET(device->mcu, goto error, NULL, "Cannot create mcu device pointer\n");

    return device;

error:
    if(device->mcu)
    {
        mcu_heap_free(device->mcu);
        device->mcu = NULL;
    }
    if(device)
    {
        mcu_heap_free(device);
        device = NULL;
    }
    return NULL;
}

void display_sld_set_backlight(display_sld_handle_t device, float pwm)
{
    mcu_pwm_set_duty_cycle(device->backlight, (uint32_t)(pwm * 100.0));
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static uint32_t _pclk_from_eeid(const uint8_t* eeid, uint8_t index)
{
    uint32_t tmp = ((uint32_t)bcd_decode_uint8(eeid[index + 1]));
    if(tmp > 10)
    {
        tmp *= 10000;
    }
    else
    {
        tmp *= 100000;
    }
    return ((uint32_t)bcd_decode_uint8(eeid[index]) * 1000000) + tmp;
}

static uint16_t _uint16_from_eeid(const uint8_t* eeid, uint8_t index)
{
    return ((uint16_t)eeid[index] * 256) + eeid[index + 1];
}

#endif // MODULE_ENABLE_DISPLAY
