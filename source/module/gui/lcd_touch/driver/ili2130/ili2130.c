/**
 * @file ili2130.c
 **/

#include "ili2130.h"

#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ILI2130 && MODULE_ENABLE_LCD_TOUCH

#include "module/comm/dbg.h"
#include "module/convert/convert.h"
#include "module/util/assert.h"
#include "mcu/sys.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define ILI2130_I2C_ADDRESS 0x41  // I2C address of the ILI2130

#define MAX_POINTS			10

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct lcd_touch_device_s
{
	const ili2130_hw_config_t* hw;

	bool int_triggered;

	uint32_t timestamp_poll;

	system_task_t task;

	uint16_t x[MAX_POINTS];

	uint16_t y[MAX_POINTS];

	int num_touched;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _handle(lcd_touch_device_handle_t device);

static void _int_touch(lcd_touch_device_handle_t device);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

const struct lcd_touch_interface_s ili2130_lcd_touch_interface = 
{
	.del = ili2130_free,
	.get_xy = ili2130_get_xy,
	.read_data = ili2130_read_data
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

lcd_touch_device_handle_t ili2130_create(const ili2130_hw_config_t* hw)
{
	lcd_touch_device_handle_t device = mcu_heap_calloc(1, sizeof(struct lcd_touch_device_s));

	if(device)
	{
		device->hw = hw;

		if(hw->io_reset != PIN_NONE)
		{
			mcu_io_set_dir(hw->io_reset, MCU_IO_DIR_OUT);
			mcu_io_set(hw->io_reset, 0);
			mcu_wait_ms(50);
			mcu_io_set(hw->io_reset, 1);
		}

		if(hw->io_int)
		{
			mcu_io_interrupt_set_param(device->hw->io_int, device, (void(*)(void*))_int_touch, MCU_INT_LVL_HI, MCU_IO_INT_EDGE_LOW);
		}

		system_task_init_handle(&device->task, hw->use_protothread, (system_task_cb_handle_t)_handle, device);
	}

	return device;    
}

FUNCTION_RETURN_T ili2130_free(lcd_touch_device_handle_t device)
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

// FUNCTION_RETURN_T ili2130_enter_sleep(lcd_touch_device_handle_t device)
// {
// 	ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

// 	return FUNCTION_RETURN_OK;
// }

// FUNCTION_RETURN_T ili2130_exit_sleep(lcd_touch_device_handle_t device)
// {
// 	ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

// 	return FUNCTION_RETURN_OK;
// }

FUNCTION_RETURN_T ili2130_read_data(lcd_touch_device_handle_t device)
{
	ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
	
	uint8_t data[64];

	i2c_set_address(device->hw->i2c, ILI2130_I2C_ADDRESS);

	memset(device->x, 0, sizeof(device->x));
	memset(device->y, 0, sizeof(device->y));
	device->num_touched = 0;

	if(i2c_wr(device->hw->i2c, NULL, 0, data, sizeof(data)))
	{
		uint8_t sum = 0;
		for(int i = 0; i < sizeof(data); i++)
		{
			sum += data[i];
		}

		if(data[0] == 0x48 && sum == 0)
		{
			device->num_touched = data[61];
			if(device->num_touched > MAX_POINTS)
			{
				device->num_touched = MAX_POINTS;
			}
			int num = 0;
			for(int i = 0; i < device->num_touched; i++)
			{
				if((data[1 + (i * 5)] & 0x40) == 0x40)
				{
					// uint8_t point_id = data[1 + (i * 5)] & 0x3F;
					device->x[i] = data[2 + (i * 5)] + (uint16_t)data[3 + (i * 5)] * 256;
					device->y[i] = data[4 + (i * 5)] + (uint16_t)data[5 + (i * 5)] * 256;
					
					// DBG_INFO("Finger %d: %d / %d\n", (uint32_t)point_id, (uint32_t)device->x[i], (uint32_t)device->y[i]);
					num++;
				}
			}
			if(num < device->num_touched)
			{
				device->num_touched = num;
			}
		}
		else
		{
			// DBG_ERROR("Invalid Checksum %02x\n", (uint32_t)sum);
			return FUNCTION_RETURN_INTEGRITYCHECK_FAILED;
		}
	}
	else
	{
		// DBG_ERROR("Error reading touch data\n");
		return FUNCTION_RETURN_READ_ERROR;
	}

	// DBG_INFO("Num: %d\n", device->num_touched);

	return FUNCTION_RETURN_OK;
}

bool ili2130_get_xy(lcd_touch_device_handle_t device, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
	ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

	if(device->num_touched == 0)
		return false;

	*point_num = MATH_MIN(max_point_num, device->num_touched);

	for(int i = 0; i < *point_num; i++)
	{
		x[i] = device->x[i];
		y[i] = device->y[i];

		// DBG_INFO("Finger 2 %d: %d / %d\n", (uint32_t)i, (uint32_t)device->x[i], (uint32_t)device->y[i]);
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

		ili2130_read_data(device);
	}
}

static void _int_touch(lcd_touch_device_handle_t device)
{
	device->int_triggered = true;
}

#endif
