// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file 	mcu.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * @author 	Tim Koczwara
 *
 * @brief 	Contains debug functions that can be used for all controller.
 *
 * @version 1.01 (21.08.2013)
 * 			 - Added RX63N
 * 			 - Added RL78/G13
 * 			 - Added DA debug prints.
 * 			 - Removed channel from AD debug prints, because the RL78 has no channels, only pins.
 * @version	1.00 (21.09.2011)
 * 			 - Initial release
 **/

#include "mcu.h"
#include "mcu_config.h"
#if MCU_TYPE == RSYNERGY
#include "hal_data.h"
#endif
#include "module/list/list.h"
#if MCU_TYPE == PC_EMU
#if defined(_WIN32) || defined(__CYGWIN__)
    // Windows (x86 or x64)
	#include <windows.h>
#elif defined(__linux__)
    // Linux
    // ...
#else
    #error Unknown environment!
#endif

#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static list_t _list_io_expander;

static bool _list_initialized = false;

#if MCU_PERIPHERY_ENABLE_WAIT_TIMER
#if MCU_TYPE == MCU_ESP32
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

#define NOP() asm volatile ("nop")

unsigned long IRAM_ATTR micros()
{
    return (unsigned long) (esp_timer_get_time());
}

void IRAM_ATTR delayMicroseconds(uint32_t us)
{
    uint32_t m = micros();
    if(us){
        uint32_t e = (m + us);
        if(m > e){ //overflow
            while(micros() > e){
                NOP();
            }
        }
        while(micros() < e){
            NOP();
        }
    }
}

#else

volatile uint32_t mcu_controller_wait_time = 0;

#if MCU_TYPE != RSYNERGY && MCU_TYPE != PC_EMU
mcu_timer_t mcu_controller_h_timer = NULL;
#endif
#endif
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_ENABLE_WAIT_TIMER && MCU_TYPE != RSYNERGY && MCU_TYPE != MCU_ESP32 && MCU_TYPE != PC_EMU
static void _timer(void*);
#endif
/**
 * @brief Checks if there is an expander registered for the I/O.
 * 
 * @param p 		Pointer to the I/O for that an expander is checked. If an expander is found, the pin_base is substracted, making p usable as parameter for the expander functions.
 * @return			Pointer to the expander or NULL if no expander was found.
 */
static mcu_io_expander_t* _get_expander(MCU_IO_PIN* p);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void mcu_register_io_expander(mcu_io_expander_t* io_expander)
{
	if(!_list_initialized)
	{
		mcu_io_expander_t* exp = NULL;
		_list_initialized = true;
		list_init(&_list_io_expander, exp, &exp->next);
	}

	list_add(&_list_io_expander, io_expander);
}

void mcu_unregister_io_expander(mcu_io_expander_t* io_expander)
{	
	list_remove(&_list_io_expander, io_expander);
}

void mcu_io_set_dir_expander(MCU_IO_PIN p, MCU_IO_DIRECTION d)
{
	mcu_io_expander_t* ioex = _get_expander(&p);

	if(ioex && ioex->set_direction)
	{
		ioex->set_direction(ioex->obj, p, d);
	}
}

void mcu_io_set_pullup_expander(MCU_IO_PIN p, bool pullup_active)
{
	mcu_io_expander_t* ioex = _get_expander(&p);

	if(ioex && ioex->set_pullup)
	{
		ioex->set_pullup(ioex->obj, p, pullup_active);
	}
}

void mcu_io_set_expander(MCU_IO_PIN p, uint8_t d)
{
	mcu_io_expander_t* ioex = _get_expander(&p);

	if(ioex && ioex->set)
	{
		ioex->set(ioex->obj, p, d);
	}
}

void mcu_io_toggle_expander(MCU_IO_PIN p)
{
	mcu_io_expander_t* ioex = _get_expander(&p);

	if(ioex && ioex->toggle)
	{
		ioex->toggle(ioex->obj, p);
	}
}

uint8_t mcu_io_get_expander(MCU_IO_PIN p)
{
	mcu_io_expander_t* ioex = _get_expander(&p);

	if(ioex && ioex->get)
	{
		return ioex->get(ioex->obj, p);
	}
	return 0;
}

FUNCTION_RETURN mcu_io_interrupt_init_expander(MCU_IO_PIN p)
{
	mcu_io_expander_t* ioex = _get_expander(&p);

	if(ioex && ioex->interrupt_init)
	{
		return ioex->interrupt_init(ioex->obj, p);
	}

	return FUNCTION_RETURN_NOT_FOUND;
}

FUNCTION_RETURN mcu_io_interrupt_set_param_expander(MCU_IO_PIN p, void* obj, void (*f)(void*), MCU_IO_INT_EDGE edge)
{
	mcu_io_expander_t* ioex = _get_expander(&p);

	if(ioex && ioex->interrupt_set_param)
	{
		return ioex->interrupt_set_param(ioex->obj, p, obj, f, edge);
	}

	return FUNCTION_RETURN_NOT_FOUND;
}

void mcu_io_interrupt_enable_expander(MCU_IO_PIN p)
{
	mcu_io_expander_t* ioex = _get_expander(&p);

	if(ioex && ioex->interrupt_enable)
	{
		ioex->interrupt_enable(ioex->obj, p);
	}
}

void mcu_io_interrupt_disable_expander(MCU_IO_PIN p)
{
	mcu_io_expander_t* ioex = _get_expander(&p);

	if(ioex && ioex->interrupt_disable)
	{
		ioex->interrupt_disable(ioex->obj, p);
	}
}

#if defined(MCU_HEAP_DEBUG)

void* mcu_heap_calloc_debug(char* filename, char* line_str, size_t num, size_t size)
{
	void* ptr;
#if CONFIG_SPIRAM
	ptr = heap_caps_calloc(num, size, MALLOC_CAP_SPIRAM); 
#else
	ptr = calloc(num, size); 
#endif
	dbg_printf(filename, line_str, "calloc(%u, %u) -> %08x\n", num, size, ptr); 
	return ptr;
}

void* mcu_heap_malloc_debug(char* filename, char* line_str, size_t size)
{
	void* ptr;
#if CONFIG_SPIRAM
	ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM); 
#else
	ptr = malloc(size); 
#endif
	dbg_printf(filename, line_str, "malloc(%u) -> %08x\n", size, ptr); 
	return ptr;
}

void mcu_heap_free_debug(char* filename, char* line_str, void* ptr)
{
	free(ptr);
	dbg_printf(filename, line_str, "free(%08x)\n", ptr); 
}

#endif

#if MCU_PERIPHERY_ENABLE_WAIT_TIMER
void mcu_wait_us(uint16_t delay)
{
#if MCU_TYPE == PC_EMU
	uint32_t timestamp = system_get_tick_count();
	while(true)
	{
		if((system_get_tick_count() - timestamp) * 1000 >= delay)
		{
			return;
		}
	}	
#elif MCU_TYPE == MCU_ESP32
	delayMicroseconds(delay);
#else
	mcu_controller_wait_time = delay / 10;

#if MCU_TYPE != RSYNERGY
	if(mcu_controller_h_timer == NULL)
		mcu_controller_h_timer = mcu_timer_init(MCU_INT_LVL_LO, 1000, _timer, NULL, false);

	mcu_timer_set_frq(mcu_controller_h_timer, 100000); // 10us
	mcu_timer_start(mcu_controller_h_timer);
#else
    mcu_wait_timer.p_api->periodSet(mcu_wait_timer.p_ctrl, 10, TIMER_UNIT_PERIOD_USEC);
    mcu_wait_timer.p_api->start(mcu_wait_timer.p_ctrl);
#endif

	while(mcu_controller_wait_time > 0);
#endif
}

bool mcu_wait_us_until(uint16_t wait_max, bool(*f)(void*), void* obj)
{
#if MCU_TYPE == PC_EMU
	uint32_t timestamp = system_get_tick_count();
	while(true)
	{
		if(f && f(obj))
		{
			return true;
		}
		else if((system_get_tick_count() - timestamp) * 1000 >= wait_max)
		{
			return false;
		}
	}	
#elif MCU_TYPE == MCU_ESP32
    uint32_t m = micros();
    if(wait_max){
        uint32_t e = (m + wait_max);
        if(m > e){ //overflow
            while(micros() > e){
                if(f && f(obj))
                	return true;
            }
        }
        while(micros() < e){
            if(f && f(obj))
            	return true;
        }
    }
    return false;
//	delayMicroseconds(wait_max);
#else
	mcu_controller_wait_time = wait_max / 10;

#if MCU_TYPE != RSYNERGY
	if(mcu_controller_h_timer == NULL)
		mcu_controller_h_timer = mcu_timer_init(MCU_INT_LVL_LO, 1000, _timer, NULL, false);

	mcu_timer_set_frq(mcu_controller_h_timer, 100000); // 10us
	mcu_timer_start(mcu_controller_h_timer);
#else
    mcu_wait_timer.p_api->periodSet(mcu_wait_timer.p_ctrl, 10, TIMER_UNIT_PERIOD_USEC);
    mcu_wait_timer.p_api->start(mcu_wait_timer.p_ctrl);
#endif

	while(mcu_controller_wait_time > 0)
	{
		if(f && f(obj))
		{
#if MCU_TYPE != RSYNERGY
            mcu_timer_stop(mcu_controller_h_timer);
#else
            mcu_wait_timer.p_api->stop(mcu_wait_timer.p_ctrl);
#endif
			return true;
		}
	}
	return false;
#endif
}

void mcu_wait_ms(uint16_t delay)
{
#if MCU_TYPE == PC_EMU
	uint32_t timestamp = system_get_tick_count();
	while(true)
	{
		if((system_get_tick_count() - timestamp) >= delay)
		{
			return;
		}
	}		
#elif MCU_TYPE == MCU_ESP32
	delayMicroseconds((uint32_t)delay * 1000);
#else
	mcu_controller_wait_time = delay;

#if MCU_TYPE != RSYNERGY
	if(mcu_controller_h_timer == NULL)
		mcu_controller_h_timer = mcu_timer_init(MCU_INT_LVL_LO, 1000, _timer, NULL, false);

	mcu_timer_set_frq(mcu_controller_h_timer, 1000);
	mcu_timer_start(mcu_controller_h_timer);
#else
    mcu_wait_timer.p_api->periodSet(mcu_wait_timer.p_ctrl, 1, TIMER_UNIT_PERIOD_MSEC);
    mcu_wait_timer.p_api->start(mcu_wait_timer.p_ctrl);
#endif

	while(mcu_controller_wait_time > 0);
#endif
}

bool mcu_wait_ms_until(uint16_t wait_max, bool(*f)(void*), void* obj)
{
#if MCU_TYPE == PC_EMU
	uint32_t timestamp = system_get_tick_count();
	while(true)
	{
		if(f && f(obj))
		{
			return true;
		}
		else if((system_get_tick_count() - timestamp) >= wait_max)
		{
			return false;
		}
	}	
#elif MCU_TYPE == MCU_ESP32
    uint32_t m = micros();
    if(wait_max){
        uint32_t e = (m + ((uint32_t)wait_max * 1000));
        if(m > e){ //overflow
            while(micros() > e){
                if(f && f(obj))
                	return true;
            }
        }
        while(micros() < e){
            if(f && f(obj))
            	return true;
        }
    }
    return false;
//	delayMicroseconds(wait_max);
#else
	mcu_controller_wait_time = wait_max;

#if MCU_TYPE != RSYNERGY
	if(mcu_controller_h_timer == NULL)
		mcu_controller_h_timer = mcu_timer_init(MCU_INT_LVL_LO, 1000, _timer, NULL, false);

    mcu_timer_set_frq(mcu_controller_h_timer, 1000);
    mcu_timer_start(mcu_controller_h_timer);
#else
    mcu_wait_timer.p_api->periodSet(mcu_wait_timer.p_ctrl, 1, TIMER_UNIT_PERIOD_MSEC);
    mcu_wait_timer.p_api->start(mcu_wait_timer.p_ctrl);
#endif

	while(mcu_controller_wait_time > 0)
	{
		if(f && f(obj))
		{
#if MCU_TYPE != RSYNERGY
		    mcu_timer_stop(mcu_controller_h_timer);
#else
		    mcu_wait_timer.p_api->stop(mcu_wait_timer.p_ctrl);
#endif
			return true;
		}
	}
	return false;
#endif
}

#endif

#if MCU_DEBUG_ENABLE
#if MCU_TYPE==R5F6411F
	#include PATH_R5F6411F
#elif MCU_TYPE==R5F64111
	#include PATH_R5F64111
#elif MCU_TYPE==R5F64189
	#include PATH_R5F64189
#elif MCU_TYPE==R5F21238
	#include PATH_R5F21238
#elif MCU_TYPE==R5F211B4
	#include PATH_R5F211B4
#elif MCU_TYPE==R5F562
	#include PATH_R5F562
#elif MCU_TYPE==R5F563
	#include PATH_R5F563
#elif MCU_TYPE==R5F100L
	#include PATH_R5F100L
#endif

#include "module/comm/comm.h"

static uint8_t get_pin_num(uint16_t p);


void mcu_debug_print_usage(void)
{
	uint16_t i = 0;
	comm_printf(COMM_DEBUG, " ----------------------------- \n");
	comm_printf(COMM_DEBUG, " ------- MCU Benutzung ------- \n");
	comm_printf(COMM_DEBUG, " ----------------------------- \n");
	
	comm_printf(COMM_DEBUG, "Externe Clock: %uHz\n", (uint32_t)mcu_get_frq_external());
	comm_printf(COMM_DEBUG, "CPU Clock: %uHz\n", (uint32_t)mcu_get_frq_cpu());
	comm_printf(COMM_DEBUG, "Periphery Clock: %uHz\n", (uint32_t)mcu_get_frq_peripheral());
	
	comm_printf(COMM_DEBUG, " ----------------------------- \n");

	#if MCU_PERIPHERY_DEVICE_COUNT_UART
		comm_printf(COMM_DEBUG, "Reservierte UARTs: %u\n", (uint32_t)MCU_PERIPHERY_DEVICE_COUNT_UART);
		for(i = 0; i < mcu_current_uart_handler; i++)
		{
			comm_printf(COMM_DEBUG, "- UART %u: %u Baud\n", (uint32_t)mcu_uart_handler[i].num, (uint32_t)mcu_uart_handler[i].baud);
			comm_printf(COMM_DEBUG, "-- Status: %s\n", mcu_get_result_str(mcu_uart_handler[i].res));
		}
		comm_printf(COMM_DEBUG, "Freie UARTs: %u\n", (uint32_t)(MCU_PERIPHERY_DEVICE_COUNT_UART - mcu_current_uart_handler));
	#else
		comm_printf(COMM_DEBUG, "Keine UARTs erstellt.\n");
	#endif
	
	comm_printf(COMM_DEBUG, " ----------------------------- \n");
	
	#if MCU_PERIPHERY_DEVICE_COUNT_SPI>0
		comm_printf(COMM_DEBUG, "Reservierte SPI Schnittstellen: %u\n", (uint32_t)MCU_PERIPHERY_DEVICE_COUNT_SPI);
		for(i=0; i<mcu_current_spi_handler; i++)
		{
			comm_printf(COMM_DEBUG, "- SPI %u: %u Hz CS: %u_%u\n",
										(uint32_t)mcu_spi_handler[i].spi->num,
										(uint32_t)mcu_spi_handler[i].spi->clock,
										(uint32_t)(mcu_spi_handler[i].cs&0xFF00)>>8,
										(uint32_t)get_pin_num((uint16_t)mcu_spi_handler[i].cs & 0x00FF));
			comm_printf(COMM_DEBUG, "-- Status: %s\n", mcu_get_result_str(mcu_spi_handler[i].spi->res));
		}
		comm_printf(COMM_DEBUG, "Freie SPIs: %u\n", (uint32_t)(MCU_PERIPHERY_DEVICE_COUNT_SPI - mcu_current_spi_handler));
	#else
		comm_printf(COMM_DEBUG, "Keine SPI Schnittstellen erstellt.\n");
	#endif
	
	comm_printf(COMM_DEBUG, " ----------------------------- \n");
	
	#if MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT>0
		comm_printf(COMM_DEBUG, "Reservierte IO Interrupts: %u\n", (uint32_t)MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT);
		for(i=0; i<mcu_current_io_int_handler; i++)
		{
			comm_printf(COMM_DEBUG, "- IO Interrupt %u\n", (uint32_t)mcu_io_int_handler[i].num);
			comm_printf(COMM_DEBUG, "-- Status: %s\n", mcu_get_result_str(mcu_io_int_handler[i].res));
		}
		comm_printf(COMM_DEBUG, "Freie IO Interrupts: %u\n", (uint32_t)(MCU_PERIPHERY_DEVICE_COUNT_IO_INTERRUPT - mcu_current_io_int_handler));
	#else
		comm_printf(COMM_DEBUG, "Keine IO Interrupts erstellt.\n");
	#endif
	
	comm_printf(COMM_DEBUG, " ----------------------------- \n");
	
	#if MCU_PERIPHERY_DEVICE_COUNT_TIMER>0
		comm_printf(COMM_DEBUG, "Reservierte Timer: %u\n", (uint32_t)MCU_PERIPHERY_DEVICE_COUNT_TIMER);
		for(i=0; i<mcu_current_timer_handler; i++)
		{
			comm_printf(COMM_DEBUG, "- Timer %u: %u Hz\n", (uint32_t)i, (uint32_t)mcu_timer_handler[i].frq);
			comm_printf(COMM_DEBUG, "-- Status: %s\n", mcu_get_result_str(mcu_timer_handler[i].res));
		}
		comm_printf(COMM_DEBUG, "Freie Timer: %u\n", (uint32_t)(MCU_PERIPHERY_DEVICE_COUNT_TIMER - mcu_current_timer_handler));
	#else
		comm_printf(COMM_DEBUG, "Keine Timer erstellt.\n");
	#endif
	
	comm_printf(COMM_DEBUG, " ----------------------------- \n");
	
	#if MCU_PERIPHERY_DEVICE_COUNT_AD>0
		comm_printf(COMM_DEBUG, "Reservierte AD Wandler: %u\n", (uint32_t)MCU_PERIPHERY_DEVICE_COUNT_AD);
		for(i = 0; i < mcu_current_ad_handler; i++)
		{
			comm_printf(COMM_DEBUG, "- AD Wandler Pin %u_%u\n",
										(uint32_t)(mcu_ad_handler[i].port & 0xFF00) >> 8,
										(uint32_t)get_pin_num((uint16_t)mcu_ad_handler[i].port & 0x00FF)
									);
			comm_printf(COMM_DEBUG, "-- Status: %s\n", mcu_get_result_str(mcu_ad_handler[i].res));
		}
		comm_printf(COMM_DEBUG, "Freie AD Wandler: %u\n", (uint32_t)(MCU_PERIPHERY_DEVICE_COUNT_AD - mcu_current_ad_handler));
	#else
		comm_printf(COMM_DEBUG, "Keine AD Wandler erstellt.\n");
	#endif
	
	comm_printf(COMM_DEBUG, " ----------------------------- \n");
	
	#if MCU_PERIPHERY_DEVICE_COUNT_CAN>0
		comm_printf(COMM_DEBUG, "Reservierte CAN Schnittstellen: %u\n", (uint32_t)MCU_PERIPHERY_DEVICE_COUNT_CAN);
		for(i=0; i<mcu_current_can_handler; i++)
		{
			comm_printf(COMM_DEBUG, "- CAN %d: %u Baud\n", (uint32_t)mcu_can_handler[i].num, (uint32_t)mcu_can_handler[i].baud);
			comm_printf(COMM_DEBUG, "- Nachrichtenboxen registriert: %u\n", (uint32_t)mcu_can_handler[i].msg_boxes_in_use);
			comm_printf(COMM_DEBUG, "-- Status: %s\n", mcu_get_result_str(mcu_can_handler[i].res));
		}
		comm_printf(COMM_DEBUG, "Freie CAN Schnittstellen: %u\n", (uint32_t)(MCU_PERIPHERY_DEVICE_COUNT_CAN - mcu_current_can_handler));
	#else
		comm_printf(COMM_DEBUG, "Kein CAN erstellt.\n");
	#endif

	comm_printf(COMM_DEBUG, " ----------------------------- \n");

	#if MCU_PERIPHERY_DEVICE_COUNT_DA > 0
		comm_printf(COMM_DEBUG, "Reservierte DA Wandler: %u\n", (uint32_t)MCU_PERIPHERY_DEVICE_COUNT_DA);
		for(i = 0; i < mcu_current_ad_handler; i++)
		{
			comm_printf(COMM_DEBUG, "- DA Wandler Pin %u_%u\n",
										(uint32_t)(mcu_da_handler[i].port & 0xFF00) >> 8,
										(uint32_t)get_pin_num(mcu_da_handler[i].port&0x00FF)
									);
			comm_printf(COMM_DEBUG, "-- Status: %s\n", mcu_get_result_str(mcu_da_handler[i].res));
		}
		comm_printf(COMM_DEBUG, "Freie DA Wandler: %u\n", (uint32_t)(MCU_PERIPHERY_DEVICE_COUNT_DA - mcu_current_da_handler));
	#else
		comm_printf(COMM_DEBUG, "Keine DA Wandler erstellt.\n");
	#endif

	comm_printf(COMM_DEBUG, " ============================= \n");

	comm_printf(COMM_DEBUG, "MCU Last Error: %s\n", mcu_get_last_error_str());
	
	comm_printf(COMM_DEBUG, " ----------------------------- \n");
	comm_printf(COMM_DEBUG, " ---- MCU Benutzung Ende ----- \n");
	comm_printf(COMM_DEBUG, " ----------------------------- \n");
}

MCU_RESULT mcu_get_last_error(void)
{
	return mcu_last_error;
}

#define CASE_RESULT_RETURN(res) 	case res: return #res 
const char* mcu_get_result_str(MCU_RESULT res)
{
	switch(res)
	{
		CASE_RESULT_RETURN(MCU_OK);
		
		CASE_RESULT_RETURN(MCU_ERROR_FRQ_EXT_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_FRQ_MCU_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_FRQ_PERIPHERAL_INVALID);
		
		CASE_RESULT_RETURN(MCU_ERROR_IO_INT_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_IO_INT_NOT_AVAILABLE);
		CASE_RESULT_RETURN(MCU_ERROR_IO_INT_PIN_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_IO_INT_LVL_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_IO_INT_EDGE_INVALID);
		
		CASE_RESULT_RETURN(MCU_ERROR_TMR_NOT_AVAILABLE);
		CASE_RESULT_RETURN(MCU_ERROR_TMR_LVL_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_TMR_FRQ_INVALID);
		
		CASE_RESULT_RETURN(MCU_ERROR_UART_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_UART_NOT_AVAILABLE);
		CASE_RESULT_RETURN(MCU_ERROR_UART_DATABITS_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_UART_PARITY_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_UART_STOPBITS_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_UART_BAUDRATE_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_UART_RECEIVE_INVALID);
		
		CASE_RESULT_RETURN(MCU_ERROR_SPI_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_SPI_NOT_AVAILABLE);
		CASE_RESULT_RETURN(MCU_ERROR_SPI_MODE_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_SPI_CLOCK_INVALID);
		
		CASE_RESULT_RETURN(MCU_ERROR_CAN_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_CAN_NOT_AVAILABLE);
		CASE_RESULT_RETURN(MCU_ERROR_CAN_BAUDRATE_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_CAN_RECEIVE_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_CAN_MESSAGE_BOX_INVALID);

		CASE_RESULT_RETURN(MCU_ERROR_AD_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_AD_NOT_AVAILABLE);
		CASE_RESULT_RETURN(MCU_ERROR_AD_IO_PIN_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_AD_INTERRUPT_NOT_SUPPORTED);
		CASE_RESULT_RETURN(MCU_ERROR_AD_SIGNEDNESS_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_AD_RESOLUTION_INVALID);
		
		CASE_RESULT_RETURN(MCU_ERROR_DA_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_DA_NOT_AVAILABLE);
		CASE_RESULT_RETURN(MCU_ERROR_DA_IO_PIN_INVALID);
		CASE_RESULT_RETURN(MCU_ERROR_DA_RESOLUTION_INVALID);
		
		default: return "UNKNOWN RESULT";
	}
}

static uint8_t get_pin_num(uint16_t p)
{
	switch(p&0xFF)
	{
		case 0x01:	return 0;
		case 0x02:	return 1;
		case 0x04:	return 2;
		case 0x08:	return 3;
		case 0x10:	return 4;
		case 0x20:	return 5;
		case 0x40:	return 6;
		case 0x80:	return 7;
		default:	return 9;
	}
}

const char* mcu_get_last_error_str(void)
{
	return mcu_get_result_str(mcu_last_error);
}

void mcu_debug_can_print(can_frame_t p)
{
	uint8_t i;
	dbg_printf(NULL, "[%2u] %8h%s: [%d|", p.info.elements.message_box, p.id, p.info.elements.is_extended?"(E)":"", p.dlc);
	for(i=0; i<p.dlc && i<8; i++)	
	{	
		if(i>0)	comm_putc(COMM_DEBUG, ';');
		comm_printf(COMM_DEBUG, "%2X", p.data[i]);
	}
	comm_puts(COMM_DEBUG, "]\n");
}

#endif	// MCU_DEBUG_ENABLE

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_ENABLE_WAIT_TIMER && MCU_TYPE != MCU_ESP32 && MCU_TYPE != PC_EMU
#if MCU_TYPE != RSYNERGY
static void _timer(void* obj)
#else
void mcu_wait_timer_cb(timer_callback_args_t *args)
#endif
{
	if(mcu_controller_wait_time > 0)
		mcu_controller_wait_time--;
	else
	{
		mcu_controller_wait_time = 0;
#if MCU_TYPE != RSYNERGY
		mcu_timer_stop(mcu_controller_h_timer);
#else
		mcu_wait_timer.p_api->stop(mcu_wait_timer.p_ctrl);
#endif
	}
}
#endif

static mcu_io_expander_t* _get_expander(MCU_IO_PIN* p)
{
	mcu_io_expander_t* ioex = list_get_first(&_list_io_expander);

	while(ioex)
	{
		if(ioex->pin_base <= *p && (ioex->pin_base + ioex->pin_count) > *p)
		{
			*p -= ioex->pin_base;
			return ioex;
		}
		
		ioex = list_get_next(&_list_io_expander, ioex);
	}

	return NULL;
}
