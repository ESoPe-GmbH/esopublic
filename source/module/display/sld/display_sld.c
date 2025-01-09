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

#include "module_public.h"

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

    mcu_io_set_dir(config->display.rgb.disp_en, MCU_IO_DIR_OUT);
    mcu_io_set(config->display.rgb.disp_en, 1);

    string_nprintf(device->screen_diagonal, sizeof(device->screen_diagonal), "%x.%x", (uint32_t)eeid[6], (uint32_t)eeid[7]);

    display_sld_init(&config->display, eeid, sizeof(eeid), device);
    DBG_ASSERT(device->display, goto error, NULL, "Failed to initialize display\n");

    bool rotate = device->display->device_config.rgb.v_res > device->display->device_config.rgb.h_res;

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
                .flags = {.mirror_x = false, .mirror_y = false, .swap_xy = false},
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
    display_device_mirror(device->display, rotate, false);
    display_device_swap_xy(device->display, rotate);

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

display_handle_t display_sld_init(const display_common_hardware_t* config, const uint8_t *eeid, uint8_t eeid_length, display_sld_handle_t handle)
{
    DBG_ASSERT(config, NO_ACTION, NULL, "config cannot be NULL\n");
    DBG_ASSERT(eeid, NO_ACTION, NULL, "eeid cannot be NULL\n");
    DBG_ASSERT(eeid_length > 27, NO_ACTION, NULL, "eeid_length must be greater than 27\n");
    bool has_touch;
    uint16_t color_depth;
    display_handle_t device = mcu_heap_calloc(1, sizeof(struct display_data_s));
    DBG_ASSERT(device, NO_ACTION, NULL, "Cannot create display device pointer\n");

    DBG_INFO("EEPROM: %#A\n", eeid_length, eeid);

    DBG_VERBOSE("Create config\n");
#if MCU_TYPE == MCU_ESP32
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
    device->mcu_config.rgb.esp32.dma_burst_size = 64;
#else
    device->mcu_config.rgb.esp32.sram_trans_align = 4;
    device->mcu_config.rgb.esp32.psram_trans_align = 64;
#endif
    device->mcu_config.rgb.esp32.flags.fb_in_psram = true;
    device->mcu_config.rgb.esp32.flags.relax_on_idle = false;
#if KERNEL_USES_SLINT
    device->mcu_config.rgb.esp32.bounce_buffer_size_percent = 5;
#endif
#endif
    //[0] is 0
    ASSERT_RET(eeid[1] == 1, goto error, NULL, "Display with DPI needed\n");
    has_touch = eeid[2] > 0;
    ASSERT_RET(eeid[3] == 'S' && eeid[4] == 'W', goto error, NULL, "Invalid EEID content\n");
    // [5] = Manufacturer revision
    // [7:6] = Physical size in format xxh.xxh
    device->device_config.rgb.h_res = _uint16_from_eeid(eeid, 8);
    device->device_config.rgb.v_res = _uint16_from_eeid(eeid, 10);
    device->device_config.rgb.pclk_hz = _pclk_from_eeid(eeid, 12);
    color_depth = eeid[14]; // [14] = Color depth
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
    device->device_config.rgb.flags.pclk_idle_high = false;

    DBG_INFO("Initialize %x.%x\" Display (%d-Bit) with%s touch\n", (uint32_t)eeid[6], (uint32_t)eeid[7], (uint32_t)color_depth, has_touch ? "" : "out");

    device->mcu = NULL;

    if(config->interface == DISPLAY_INTERFACE_RGB)
    {
        // Displays can be 16-Bit and 24-Bit Interface, but HW on PCB is probably fixed to 16-Bit or 24-Bit RGB.
        // Ensure compatibility
        if(color_depth != config->rgb.data_width)
        {
            if(handle)
            {
                handle->data_width = MATH_MIN(color_depth, config->rgb.data_width);
            }
            if(color_depth == 16 && config->rgb.data_width == 24)
            {
                // Only use 16-Bit of 24-Bit interface
                display_common_hardware_t config2;
                memcpy(&config2, config, sizeof(display_common_hardware_t));

                config2.rgb.data_width = 16;

                config2.rgb.r[0] = config->rgb.r[3];
                config2.rgb.r[1] = config->rgb.r[4];
                config2.rgb.r[2] = config->rgb.r[5];
                config2.rgb.r[3] = config->rgb.r[6];
                config2.rgb.r[4] = config->rgb.r[7];
                config2.rgb.r[5] = PIN_NONE;
                config2.rgb.r[6] = PIN_NONE;
                config2.rgb.r[7] = PIN_NONE;

                config2.rgb.g[0] = config->rgb.g[2];
                config2.rgb.g[1] = config->rgb.g[3];
                config2.rgb.g[2] = config->rgb.g[4];
                config2.rgb.g[3] = config->rgb.g[5];
                config2.rgb.g[4] = config->rgb.g[6];
                config2.rgb.g[5] = config->rgb.g[7];
                config2.rgb.g[6] = PIN_NONE;
                config2.rgb.g[7] = PIN_NONE;

                config2.rgb.b[0] = config->rgb.b[3];
                config2.rgb.b[1] = config->rgb.b[4];
                config2.rgb.b[2] = config->rgb.b[5];
                config2.rgb.b[3] = config->rgb.b[6];
                config2.rgb.b[4] = config->rgb.b[7];
                config2.rgb.b[5] = PIN_NONE;
                config2.rgb.b[6] = PIN_NONE;
                config2.rgb.b[7] = PIN_NONE;
            }
        }

    }

    if(device->mcu == NULL)
    {
        // Initialize display with original config
        device->mcu = display_mcu_init(config, device);
    }
    ASSERT_RET(device->mcu, goto error, NULL, "Cannot create mcu device pointer\n");

    if(handle)
    {
        handle->data_width = color_depth;
        handle->supports_touch = has_touch;
        handle->display = device;
    }

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

static uint16_t _uint16_from_eeid(const uint8_t* eeid, uint8_t index)
{
    return ((uint16_t)eeid[index] * 256) + eeid[index + 1];
}

#endif // MODULE_ENABLE_DISPLAY
