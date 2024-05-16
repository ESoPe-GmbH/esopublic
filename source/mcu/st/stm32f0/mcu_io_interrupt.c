// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_io_interrupt.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"

#if MCU_TYPE == MCU_STM32F0

#include "../stm32f0/mcu_internal.h"

#if MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT > 0

/**
 * Enables or disables a single interrupt.
 */
static void _io_interrupt_set_enable(mcu_io_int_handler_ctx* h, bool enable);

#define MCU_IO_INT_INIT_HANDLER(h, n)											\
		{																\
			h->num = n;												\
			h->lvl = MCU_INT_LVL_OFF;								\
			h->callback = NULL;										\
			mcu_io_int_handler_hash[n] = h;							\
		}

#define IRICUIRQ(h)	ICU.IR[ ((mcu_io_int_handler_ctx*)h)->int_flag_num ].BIT.IR

mcu_io_int_handler_ctx *mcu_io_int_handler_hash[16] = {	0 };
uint8_t mcu_current_io_int_handler = 0;
mcu_io_int_handler_ctx mcu_io_int_handler[MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT];

mcu_io_int_t mcu_io_interrupt_init(uint8_t num, MCU_IO_PIN pin)
{
	uint8_t pin_num = pin & 0xFF;
	mcu_io_int_handler_ctx* handle;

	if(mcu_current_io_int_handler>=MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT)
		MCU_RETURN_ERROR(MCU_ERROR_IO_INT_NOT_AVAILABLE, NULL)

	handle = &mcu_io_int_handler[mcu_current_io_int_handler];

	// There are 16 I/O interrupts in total. All ports share a line for each pin (e.g. PA_0 and PB_0 have the same line).
	// Therefore only one pin can use a line. But different lines can be used by multiple ports (e.g. PA_0 and PB_1 can be active together).

	// Maximum of 16 I/O interrupts
	if(pin_num > 15)
		MCU_RETURN_ERROR(MCU_ERROR_IO_INT_PIN_INVALID, NULL);

	// Check if I/O Interrupt line already in use!
	if(mcu_io_int_handler_hash[num])
		MCU_RETURN_ERROR(MCU_ERROR_IO_INT_NOT_AVAILABLE, NULL);

	// Verify that the I/O Interrupt number is the same as the I/O pin
	if(pin_num != num)
		MCU_RETURN_ERROR(MCU_ERROR_IO_INT_PIN_INVALID, NULL);

	// Initialize the structure
	MCU_IO_INT_INIT_HANDLER(handle, num);

	// Pre-initialize the GPIO as input
	handle->pin = pin;
	memset(&handle->gpio, 0, sizeof(GPIO_InitTypeDef));
	handle->gpio.Pin = (1 << pin_num);
	handle->gpio.Mode = GPIO_MODE_INPUT;
	handle->gpio.Pull = GPIO_PULLUP;
	handle->gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(mcu_internal_get_port(pin), &handle->gpio);

	// Determine the interrupt flag number by the line number
	if(num < 2)
		handle->int_flag_num = EXTI0_1_IRQn;
	else if(num < 4)
		handle->int_flag_num = EXTI2_3_IRQn;
	else
		handle->int_flag_num = EXTI4_15_IRQn;

	mcu_current_io_int_handler++;
	return (mcu_io_int_t)handle;
}

MCU_RESULT mcu_io_interrupt_set_param(mcu_io_int_t handle, void* obj, void (*f)(void*), MCU_INT_LVL lvl, MCU_IO_INT_EDGE edge)
{
	uint8_t priority;
	if(handle==NULL)	return MCU_ERROR_IO_INT_INVALID;
	if(lvl>3)			return MCU_ERROR_IO_INT_LVL_INVALID;
	if(edge>2) 			return MCU_ERROR_IO_INT_EDGE_INVALID;

	_io_interrupt_set_enable((mcu_io_int_handler_ctx*)handle, 0);

	switch(edge)
	{
	case MCU_IO_INT_EDGE_LOW:	((mcu_io_int_handler_ctx*)handle)->gpio.Mode = GPIO_MODE_IT_FALLING;			break;
	case MCU_IO_INT_EDGE_HIGH:	((mcu_io_int_handler_ctx*)handle)->gpio.Mode = GPIO_MODE_IT_RISING;			break;
	case MCU_IO_INT_EDGE_BOTH:	((mcu_io_int_handler_ctx*)handle)->gpio.Mode = GPIO_MODE_IT_RISING_FALLING;	break;
	}

	HAL_GPIO_Init(mcu_internal_get_port(((mcu_io_int_handler_ctx*)handle)->pin), &((mcu_io_int_handler_ctx*)handle)->gpio);

	MCU_GET_INT_LVL(lvl, priority);
	HAL_NVIC_SetPriority(((mcu_io_int_handler_ctx*)handle)->int_flag_num, priority, priority);
	((mcu_io_int_handler_ctx*)handle)->callback_obj = obj;
	((mcu_io_int_handler_ctx*)handle)->callback = f;

	_io_interrupt_set_enable((mcu_io_int_handler_ctx*)handle, 1);
	return MCU_OK;
}

void mcu_io_interrupt_disable(mcu_io_int_t handle)
{
	if(handle==NULL)	return;
	_io_interrupt_set_enable((mcu_io_int_handler_ctx*)handle, 0);
}

void mcu_io_interrupt_enable(mcu_io_int_t handle)
{
	if(handle==NULL)	return;
	_io_interrupt_set_enable((mcu_io_int_handler_ctx*)handle, 1);
}

MCU_IO_PIN mcu_io_interrupt_get_pin(mcu_io_int_t handle)
{
	if(handle==NULL)
		return PIN_NONE;

	return ((mcu_io_int_handler_ctx*)handle)->pin;
}

#define MCU_IO_INTERRUPT_INTN_DIRECT(n)											\
		__HAL_GPIO_EXTI_CLEAR_IT(1 << n);								\
		if(mcu_io_int_handler_hash[n] && mcu_io_int_handler_hash[n]->callback)	\
			mcu_io_int_handler_hash[n]->callback(mcu_io_int_handler_hash[n]->callback_obj); \

#define MCU_IO_INTERRUPT_INTN(n)											\
		if(__HAL_GPIO_EXTI_GET_IT(1 << n) != RESET) \
		{\
			MCU_IO_INTERRUPT_INTN_DIRECT(n); \
		}

void  EXTI0_1_IRQHandler(void){		MCU_IO_INTERRUPT_INTN(0); MCU_IO_INTERRUPT_INTN(1); }
void  EXTI2_3_IRQHandler(void){		MCU_IO_INTERRUPT_INTN(2); MCU_IO_INTERRUPT_INTN(3);}
void  EXTI4_15_IRQHandler(void)
{
	MCU_IO_INTERRUPT_INTN(4);
	MCU_IO_INTERRUPT_INTN(5);
	MCU_IO_INTERRUPT_INTN(6);
	MCU_IO_INTERRUPT_INTN(7);
	MCU_IO_INTERRUPT_INTN(8);
	MCU_IO_INTERRUPT_INTN(9);
	MCU_IO_INTERRUPT_INTN(10);
	MCU_IO_INTERRUPT_INTN(11);
	MCU_IO_INTERRUPT_INTN(12);
	MCU_IO_INTERRUPT_INTN(13);
	MCU_IO_INTERRUPT_INTN(14);
	MCU_IO_INTERRUPT_INTN(15);
}

static void _io_interrupt_set_enable(mcu_io_int_handler_ctx* h, bool enable)
{
	if(enable)
		HAL_NVIC_EnableIRQ(h->int_flag_num);
	else
		HAL_NVIC_DisableIRQ(h->int_flag_num);
}

#endif

#endif // #if MCU_TYPE
