/**
 * @file mcu_can.c
 * @copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_PERIPHERY_DEVICE_COUNT_CAN > 0 && MCU_TYPE == MCU_ESP32

#include "../../sys.h"
#include "module/fifo/fifo.h"
#include "driver/twai.h"
#include "mcu_internal.h"
#include <string.h>

uint8_t mcu_current_can_handler = 0;

struct mcu_can_s mcu_can_handler[MCU_PERIPHERY_DEVICE_COUNT_CAN] = {0};

static twai_timing_config_t _t_config = TWAI_TIMING_CONFIG_25KBITS();
static twai_filter_config_t _f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static twai_general_config_t _g_config = TWAI_GENERAL_CONFIG_DEFAULT(-1, -1, TWAI_MODE_NORMAL);

/**
 *	Casts a pointer to an mcu_can_handler_ctx pointer.
 *
 * @param h			Pointer to cast
 **/
#define CAST_CAN(h)	((mcu_can_handler_ctx*)h)

mcu_can_t mcu_can_init(uint8_t num, MCU_IO_PIN tx, MCU_IO_PIN rx)
{
	mcu_can_t h;

	if(mcu_current_can_handler >= MCU_PERIPHERY_DEVICE_COUNT_CAN)
		MCU_RETURN_ERROR(MCU_ERROR_CAN_NOT_AVAILABLE, NULL);

	h = &mcu_can_handler[mcu_current_can_handler];	// For easier handling

	_g_config.tx_io = tx;
	_g_config.rx_io = rx;

	mcu_current_can_handler++;

	return (mcu_can_t)h;
}

MCU_RESULT mcu_can_set_baudrate(mcu_can_t h, uint32_t baudrate)
{
	twai_stop();
	twai_driver_uninstall();

	switch(baudrate)
	{
#if (SOC_TWAI_BRP_MAX > 256)
		case 10000: 	
		{
			twai_timing_config_t t_config = TWAI_TIMING_CONFIG_10KBITS(); 
			memcpy(&_t_config, &t_config, sizeof(twai_timing_config_t));
			break;
		}
#endif
#if (SOC_TWAI_BRP_MAX > 128) || (CONFIG_ESP32_REV_MIN >= 2)
		case 20000: 
		{
			twai_timing_config_t t_config = TWAI_TIMING_CONFIG_20KBITS(); 
			memcpy(&_t_config, &t_config, sizeof(twai_timing_config_t));
			break;
		}
#endif
		case 50000:
		{
			twai_timing_config_t t_config = TWAI_TIMING_CONFIG_50KBITS(); 
			memcpy(&_t_config, &t_config, sizeof(twai_timing_config_t));
			break;
		}
		case 100000: 
		{
			twai_timing_config_t t_config = TWAI_TIMING_CONFIG_100KBITS(); 
			memcpy(&_t_config, &t_config, sizeof(twai_timing_config_t));
			break;
		}
		case 125000: 
		{
			twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS(); 
			memcpy(&_t_config, &t_config, sizeof(twai_timing_config_t));
			break;
		}
		case 250000: 
		{
			twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS(); 
			memcpy(&_t_config, &t_config, sizeof(twai_timing_config_t));
			break;
		}
		case 500000: 	
		{
			twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS(); 
			memcpy(&_t_config, &t_config, sizeof(twai_timing_config_t));
			break;
		}
		case 1000000: 
		{
			twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS(); 
			memcpy(&_t_config, &t_config, sizeof(twai_timing_config_t));
			break;
		}
		default:		
			MCU_RETURN_ERROR(MCU_ERROR_CAN_BAUDRATE_INVALID, MCU_ERROR_CAN_BAUDRATE_INVALID);
	}

	h->baud = baudrate;

	twai_driver_install(&_g_config, &_t_config, &_f_config);

	twai_start();
	return MCU_OK;
}

uint32_t mcu_can_get_baudrate(mcu_can_t h)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_CAN_INVALID, 0);

	return h->baud;
}

MCU_RESULT mcu_can_receive_enable(mcu_can_t h, MCU_INT_LVL lvl, uint8_t *buf, uint16_t buf_elements)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_CAN_INVALID, MCU_ERROR_CAN_INVALID);

	// Receive is already available...

	return MCU_OK;
}

bool mcu_can_receive_package(mcu_can_t h, can_frame_t *cf)
{
	twai_message_t msg = {0};

	if(h == NULL || cf == NULL)
		return false;

	if(ESP_OK == twai_receive(&msg, 0))
	{
		// The hardware filter has no option to select between Standard and Single ID.
		// Therefore we filter manually here if filter was set and the extended setting does not match.
		if(h->has_filter_set && msg.extd != h->filter_extended)
			return false;

		// if(msg.extd)
		// 	DBG_VERBOSE("RCV %08x %d %$Q\n", msg.identifier, msg.data_length_code, msg.data);
		// else
		// 	DBG_VERBOSE("RCV %03x %d %$Q\n", msg.identifier, msg.data_length_code, msg.data);

		cf->id = msg.identifier;
		cf->info.elements.is_extended = msg.extd;
		cf->dlc = msg.data_length_code;
		for(uint8_t i = 0; i < msg.data_length_code; i++)
			cf->data[i] = msg.data[i];

		return true;
	}

	return false;
}

void mcu_can_transmit_package(mcu_can_t h, can_frame_t cf)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_CAN_INVALID, );

	esp_err_t err;
	twai_message_t msg = {0};

	msg.extd = cf.info.elements.is_extended;
	msg.identifier = cf.id;

	if(cf.dlc > TWAI_FRAME_MAX_DLC)
		cf.dlc = TWAI_FRAME_MAX_DLC;

	msg.data_length_code = cf.dlc;
	for(uint8_t i = 0; i < cf.dlc; i++)
	{
		msg.data[i] = cf.data[i];
	}

	// if(msg.extd)
	// 	DBG_VERBOSE("TRM %08x %d %$Q\n", msg.identifier, msg.data_length_code, msg.data);
	// else
	// 	DBG_VERBOSE("TRM %03x %d %$Q\n", msg.identifier, msg.data_length_code, msg.data);

	err = twai_transmit(&msg, pdMS_TO_TICKS(((1310L * 1000L / h->baud) + 1) * 10));

	if(err != ESP_OK)
	{
		DBG_ERROR("Failed to transmit CAN\n");
	}
}

MCU_RESULT mcu_can_register_receive_messagebox(mcu_can_t h, uint32_t addr_mask, uint32_t addr, bool is_extended)
{
	if(h==NULL)
		MCU_RETURN_ERROR(MCU_ERROR_CAN_INVALID, MCU_ERROR_CAN_INVALID);
	
	twai_stop();
	twai_driver_uninstall();

	DBG_INFO("Apply %s filter %08x for address %08x\n", is_extended ? "extended" : "standard", addr_mask, addr);

	if(is_extended)
	{
		_f_config.acceptance_code = addr << 3;
		_f_config.acceptance_mask = ~(addr_mask << 3);
	}
	else
	{
		_f_config.acceptance_code = addr << 21;
		_f_config.acceptance_mask = ~(addr_mask << 21);
	}

	_f_config.single_filter = true;

	h->filter_extended = is_extended;

	h->has_filter_set = true;

	twai_driver_install(&_g_config, &_t_config, &_f_config);

	twai_start();

	return MCU_OK;
}

#endif // MCU_PERIPHERY_DEVICE_COUNT_CAN>0
