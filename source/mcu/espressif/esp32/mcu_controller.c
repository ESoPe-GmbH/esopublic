// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		mcu_controller.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "../../mcu.h"
#include "module_public.h"

#if MCU_TYPE == MCU_ESP32

#include "../esp32/mcu_internal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_private/system_internal.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

MCU_RESULT mcu_last_error = MCU_OK;

uint32_t mcu_frq_ext_hz = 0;
uint32_t mcu_frq_cpu_hz = 0;
uint32_t mcu_frq_peripheral_hz = 0;
uint32_t mcu_frq_flash_hz = 0;

#if MCU_PERIPHERY_ENABLE_WATCHDOG
/// Task for watchdog
static system_task_t _task = {.name = "Watchdog"};
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_ENABLE_WATCHDOG
static void _watchdog_handle(void* obj);
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
uint32_t mcu_get_frq_external(void){ 			return mcu_frq_ext_hz; }
uint32_t mcu_get_frq_cpu(void){ 				return mcu_frq_cpu_hz; }
uint32_t mcu_get_frq_peripheral(void){ 			return mcu_frq_peripheral_hz; }

void mcu_init_max_internal()
{
	mcu_init(0, 64000000, 32000000);
}

void mcu_init_max_external(uint32_t frq_ext)
{
	mcu_init(frq_ext, 72000000UL, 36000000UL);	// ICLK Max 120MHz and PCLK Max 60 MHz
}

void mcu_init(uint32_t frq_ext, uint32_t frq_cpu, uint32_t frq_peripheral)
{
#if CONFIG_IDF_TARGET_ESP32S3
	gpio_reset_pin(GPIO19);
	gpio_reset_pin(GPIO20);
	gpio_reset_pin(GPIO39);
	gpio_reset_pin(GPIO40);
	gpio_reset_pin(GPIO41);
	gpio_reset_pin(GPIO42);
#endif

#if MCU_PERIPHERY_ENABLE_WATCHDOG
	system_task_init_handle(&_task, true, _watchdog_handle, NULL);
	
#endif
}

void mcu_enable_interrupt(void)
{
	// TODO: Implement
}

void mcu_disable_interrupt(void)
{
	// TODO: Implement
}

void mcu_soft_reset(void)
{
	esp_restart();
}

uint32_t mcu_enter_sleep_mode(uint32_t sleep_time)
{
	// TODO: Implement
	return 0;
}

void mcu_io_reset(MCU_IO_PIN p)
{
#if CONFIG_IDF_TARGET_ESP32S3
	if(p <= GPIO48)
#else
	if(p <= GPIO39)
#endif
	{
		gpio_reset_pin(p);
	}
}

void mcu_io_set_port_dir(MCU_IO_PIN p, uint8_t d)
{
	// TODO: Implement
}

void mcu_io_set_dir(MCU_IO_PIN p, MCU_IO_DIRECTION d)
{
#if CONFIG_IDF_TARGET_ESP32S3
	if(p <= GPIO48)
#elif CONFIG_IDF_TARGET_ESP32P4
	if(p <= GPIO56)
#else
	if(p <= GPIO39)
#endif
	{
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
		esp_rom_gpio_pad_select_gpio(p);
#else
		gpio_pad_select_gpio(p);
#endif
		gpio_set_direction(p, d == MCU_IO_DIR_OUT ? GPIO_MODE_INPUT_OUTPUT : GPIO_MODE_INPUT);
	}
	else if(p >= GPIO_EXTERNAL && p < PIN_NONE)
	{
		mcu_io_set_dir_expander(p, d);
	}
}

void mcu_io_set_pullup(MCU_IO_PIN p, bool pullup_active)
{
#if CONFIG_IDF_TARGET_ESP32S3
	if(p <= GPIO48)
#elif CONFIG_IDF_TARGET_ESP32P4
	if(p <= GPIO56)
#else
	if(p <= GPIO39)
#endif
	{
		if(pullup_active)
			gpio_pullup_en(p);
		else
			gpio_pullup_dis(p);
	}
	else if(p >= GPIO_EXTERNAL && p < PIN_NONE)
	{
		mcu_io_set_pullup_expander(p, pullup_active);
	}
}

void mcu_io_set(MCU_IO_PIN p, uint8_t d)
{
#if CONFIG_IDF_TARGET_ESP32S3
	if(p <= GPIO48)
#elif CONFIG_IDF_TARGET_ESP32P4
	if(p <= GPIO56)
#else
	if(p <= GPIO39)
#endif
	{
		gpio_set_level(p, d);
	}
	else if(p >= GPIO_EXTERNAL && p < PIN_NONE)
	{
		mcu_io_set_expander(p, d);
	}
}

void mcu_io_toggle(MCU_IO_PIN p)
{
#if CONFIG_IDF_TARGET_ESP32S3
	if(p <= GPIO48)
#elif CONFIG_IDF_TARGET_ESP32P4
	if(p <= GPIO56)
#else
	if(p <= GPIO39)
#endif
	{
		gpio_set_level(p, !gpio_get_level(p));
	}
	else if(p >= GPIO_EXTERNAL && p < PIN_NONE)
	{
		mcu_io_toggle_expander(p);
	}
}

uint8_t mcu_io_get(MCU_IO_PIN p)
{
#if CONFIG_IDF_TARGET_ESP32S3
	if(p <= GPIO48)
#elif CONFIG_IDF_TARGET_ESP32P4
	if(p <= GPIO56)
#else
	if(p <= GPIO39)
#endif
	{
		return gpio_get_level(p);
	}
	else if(p >= GPIO_EXTERNAL && p < PIN_NONE)
	{
		return mcu_io_get_expander(p);
	}
	return 0;
}

#if MCU_PERIPHERY_ENABLE_WATCHDOG
void mcu_watchdog_init(void (*f)(void))
{	
	// Initialize task watchdog
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
	esp_task_wdt_config_t esp_task_wdt_config = { .timeout_ms = 30000, .trigger_panic = false};
	esp_task_wdt_init(&esp_task_wdt_config);
#else
	esp_task_wdt_init(30, false);
#endif
	// Add current task to task watchdog
	esp_task_wdt_add(NULL);
	
	system_task_add(&_task);
	mcu_watchdog_trigger();	// Start with first Trigger
}

void mcu_watchdog_trigger(void)
{
	esp_task_wdt_reset();
}
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if MCU_PERIPHERY_ENABLE_WATCHDOG
static void _watchdog_handle(void* obj)
{
	esp_task_wdt_reset();
}
#endif

void esp_task_wdt_isr_user_handler(void)
{
	esp_reset_reason_set_hint(ESP_RST_PANIC);
    abort();
}

#endif // #if MCU_TYPE
