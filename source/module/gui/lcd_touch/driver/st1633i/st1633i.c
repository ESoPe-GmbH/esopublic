/**
 * @file st1633i.c
 **/

#include "st1633i.h"
#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH

#include "module/comm/dbg.h"
#include "module/convert/convert.h"
#include "module/util/assert.h"
#include "mcu/sys.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define ST1633I_I2C_ADDRESS 0x55  // I2C address of the ST1633I

#define MAX_POINTS			5

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct lcd_touch_device_s
{
	const st1633i_hw_config_t* hw;

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

const struct lcd_touch_interface_s st1633i_lcd_touch_interface = 
{
	.del = st1633i_free,
	.get_xy = st1633i_get_xy,
	.read_data = st1633i_read_data
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

lcd_touch_device_handle_t st1633i_create(const st1633i_hw_config_t* hw)
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

		device->timestamp_init = system_get_tick_count();

		if(hw->io_int)
		{
			mcu_io_interrupt_set_param(device->hw->io_int, device, (void(*)(void*))_int_touch, MCU_INT_LVL_HI, MCU_IO_INT_EDGE_LOW);
		}

		system_task_init_handle(&device->task, hw->use_protothread, (system_task_cb_handle_t)_handle, device);
	}

	return device;    
}

FUNCTION_RETURN_T st1633i_free(lcd_touch_device_handle_t device)
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

// FUNCTION_RETURN_T st1633i_enter_sleep(lcd_touch_device_handle_t device)
// {
// 	ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

// 	return FUNCTION_RETURN_OK;
// }

// FUNCTION_RETURN_T st1633i_exit_sleep(lcd_touch_device_handle_t device)
// {
// 	ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

// 	return FUNCTION_RETURN_OK;
// }

FUNCTION_RETURN_T st1633i_read_data(lcd_touch_device_handle_t device)
{
	ASSERT_RET_NOT_NULL(device, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

	// Chip needs 500ms after reset until it responds to I2C frames. Return not ready to indicate that it is not ready yet.
	if( (system_get_tick_count() - device->timestamp_init) < 400)
	{
		return FUNCTION_RETURN_NOT_READY;
	}
	
    uint8_t reg = 0x12;
	uint8_t data[19]; // (3 Byte X/Y + 1 Byte Reserved) x 5 finger -> last reserved not read

	i2c_set_address(device->hw->i2c, ST1633I_I2C_ADDRESS);

	memset(device->x, 0, sizeof(device->x));
	memset(device->y, 0, sizeof(device->y));
	device->num_touched = 0;

	if(i2c_wr(device->hw->i2c, &reg, 1, data, sizeof(data)))
	{        
        int num = 0;
        for(int i = 0, j = 0; i < MAX_POINTS; i++, j += 4)
        {
            // [j]
            //      [7] 1 if finger detected, 0 if not
            //      [6:4] Upper bits of X
            //      [3] Reserved
            //      [2:0] Upper bits of Y
            // [j+1] Lower Bits of X
            // [j+2] Lower Bits of Y
            // [j+3] Reserved 
            
            if((data[j] & 0x80) == 0x80)
            {
                device->x[num] = ((uint16_t)(data[j] & 0x70) << 4) + data[j + 1];
                device->y[num] = ((uint16_t)(data[j] & 0x07) << 8) + data[j + 2];
                
                // DBG_INFO("Finger %d: %d / %d\n", (uint32_t)point_id, (uint32_t)device->x[i], (uint32_t)device->y[i]);
                num++;
            }
        }

        device->num_touched = num;
	}
	else
	{
		// DBG_ERROR("Error reading touch data\n");
		return FUNCTION_RETURN_READ_ERROR;
	}

	// DBG_INFO("Num: %d\n", device->num_touched);

	return FUNCTION_RETURN_OK;
}

bool st1633i_get_xy(lcd_touch_device_handle_t device, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
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

		st1633i_read_data(device);
	}
}

static void _int_touch(lcd_touch_device_handle_t device)
{
	device->int_triggered = true;
}

#endif // MODULE_ENABLE_ST1633I
