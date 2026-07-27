/**
 * @file hy4614.c
 **/

#include "hy4614.h"

#if MODULE_ENABLE_LCD_TOUCH_DRIVER_HY4614 && MODULE_ENABLE_LCD_TOUCH

#include "module/comm/dbg.h"
#include "module/convert/convert.h"
#include "module/util/assert.h"
#include "mcu/sys.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define HY4614_I2C_ADDRESS 0x38  // I2C address of the HY4614

#define MAX_POINTS			5

#define HY4614_REG_TD_STATUS		0x02
#define HY4614_REG_TOUCH1_XH		0x03
#define HY4614_TOUCH_REG_STRIDE		6

#define HY4614_TD_STATUS_TOUCH_COUNT_MASK	0x0F
#define HY4614_POS_H_NIBBLE_MASK		0x0F
#define HY4614_TOUCH_EVENT_MASK			0xC0
#define HY4614_TOUCH_EVENT_SHIFT		6
#define HY4614_TOUCH_EVENT_DOWN			0

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct lcd_touch_device_s
{
	const hy4614_hw_config_t* hw;

	bool int_triggered;

	uint32_t timestamp_poll;

	system_task_t task;

	uint16_t x[MAX_POINTS];

	uint16_t y[MAX_POINTS];

	int num_touched;

	uint32_t timestamp_init;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _handle(lcd_touch_device_handle_t device);

static void _int_touch(lcd_touch_device_handle_t device);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

const struct lcd_touch_interface_s hy4614_lcd_touch_interface = 
{
	.del = hy4614_free,
	.get_xy = hy4614_get_xy,
	.read_data = hy4614_read_data
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

lcd_touch_device_handle_t hy4614_create(const hy4614_hw_config_t* hw)
{
	lcd_touch_device_handle_t device = mcu_heap_calloc(1, sizeof(struct lcd_touch_device_s));

	if(device)
	{
		device->hw = hw;

		if(hw->io_reset != PIN_NONE)
		{
			mcu_io_set_dir(hw->io_reset, MCU_IO_DIR_OUT);
			mcu_io_set(hw->io_reset, 0);
			mcu_wait_ms(10);
			mcu_io_set(hw->io_reset, 1);
		}

		device->timestamp_init = system_get_tick_count();

		if(hw->io_int)
		{
			mcu_io_interrupt_set_param(device->hw->io_int, device, (void(*)(void*))_int_touch, MCU_INT_LVL_HI, MCU_IO_INT_EDGE_LOW);
		}

		system_task_init_handle(&device->task, hw->use_protothread, (system_task_cb_handle_t)_handle, device);
	}

	return device;    
}

FUNCTION_RETURN_T hy4614_free(lcd_touch_device_handle_t device)
{
	ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

	system_remove_task(&device->task);
	if(device->hw->io_int)
	{
		mcu_io_interrupt_disable(device->hw->io_int);
	}
	mcu_heap_free(device);

	return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T hy4614_read_data(lcd_touch_device_handle_t device)
{
	ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

	// Chip needs some time after reset until it responds to I2C frames.
	if( (system_get_tick_count() - device->timestamp_init) < 200)
	{
		return FUNCTION_RETURN_NOT_READY;
	}
	
	i2c_set_address(device->hw->i2c, HY4614_I2C_ADDRESS);

	memset(device->x, 0, sizeof(device->x));
	memset(device->y, 0, sizeof(device->y));
	device->num_touched = 0;

	uint8_t td_status = 0;
	uint8_t reg = HY4614_REG_TD_STATUS;
	bool b = i2c_wr(device->hw->i2c, &reg, 1, &td_status, 1);
	ASSERT_RET(b, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Failed to read number of touched points from HY4614\n");

	uint8_t reported_touches = (td_status & HY4614_TD_STATUS_TOUCH_COUNT_MASK);
	if(reported_touches > MAX_POINTS)
	{
		reported_touches = MAX_POINTS;
	}

	if(reported_touches == 0)
	{
		return FUNCTION_RETURN_OK;
	}

	for(int i = 0; i < reported_touches; i++)
	{
		uint8_t point_reg = HY4614_REG_TOUCH1_XH + (uint8_t)(i * HY4614_TOUCH_REG_STRIDE);
		uint8_t point_data[4] = {0};

		b = i2c_wr(device->hw->i2c, &point_reg, 1, point_data, sizeof(point_data));
		ASSERT_RET(b, NO_ACTION, FUNCTION_RETURN_EXECUTION_ERROR, "Failed to read touch coordinates from HY4614");
		// DBG_VERBOSE("Read reg 0x%02X: %04A\n", point_reg, point_data);
		// DBG_VERBOSE("Finger %d: xh=%02X xl=%02X yh=%02X yl=%02X\n", i, point_data[0], point_data[1], point_data[2], point_data[3]);

		uint8_t event = (uint8_t)((point_data[0] & HY4614_TOUCH_EVENT_MASK) >> HY4614_TOUCH_EVENT_SHIFT);
		if(event != HY4614_TOUCH_EVENT_DOWN)
		{
			continue;
		}

		device->x[device->num_touched] = (uint16_t)(((uint16_t)(point_data[0] & HY4614_POS_H_NIBBLE_MASK) << 8) | point_data[1]);
		device->y[device->num_touched] = (uint16_t)(((uint16_t)(point_data[2] & HY4614_POS_H_NIBBLE_MASK) << 8) | point_data[3]);

		DBG_VERBOSE("Finger %d: x=%d y=%d\n", i, device->x[device->num_touched], device->y[device->num_touched]);
		device->num_touched++;
	}

	return FUNCTION_RETURN_OK;
}

bool hy4614_get_xy(lcd_touch_device_handle_t device, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
	ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
	(void)strength;

	if(device->num_touched == 0)
		return false;

	*point_num = (uint8_t)MATH_MIN(max_point_num, device->num_touched);

	for(int i = 0; i < *point_num; i++)
	{
		x[i] = device->x[i];
		y[i] = device->y[i];
	}

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _handle(lcd_touch_device_handle_t device)
{    
	if(device->hw->io_int == NULL)
	{
		if( (system_get_tick_count() - device->timestamp_poll) >= 10)
		{
			device->timestamp_poll = system_get_tick_count();
			device->int_triggered = true;
		}
	}
	else
	{	
		// Interrupt not triggered for 50ms -> finger removed, clear coordinates
		if(!device->int_triggered && (system_get_tick_count() - device->timestamp_poll) >= 50)
		{
			memset(device->x, 0, sizeof(device->x));
			memset(device->y, 0, sizeof(device->y));
			device->num_touched = 0;
		}
	}

	if(device->int_triggered)
	{
		device->int_triggered = false;

		hy4614_read_data(device);
	}
}

static void _int_touch(lcd_touch_device_handle_t device)
{
	device->int_triggered = true;
}

#endif
