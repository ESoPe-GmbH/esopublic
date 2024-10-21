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

#ifndef __DISPLAY__SLD__DISPLAY_SLD_H__
#define __DISPLAY__SLD__DISPLAY_SLD_H__

#include "module.h"

#if MODULE_ENABLE_DISPLAY && DISPLAY_ENABLE_SLD

#include "display_sld.h"
#include "../internal/display_internal.h"

#include "module/comm/dbg.h"
#include "module/convert/convert.h"
#include "module/util/assert.h"

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

#endif // __DISPLAY__SLD__DISPLAY_SLD_H__