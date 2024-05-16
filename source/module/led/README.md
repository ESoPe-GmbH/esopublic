# LED module

Offers handling of an LED connected to a GPIO pin.

Initialize it by calling `led_init` with a pointer to an `led_t` struct to store the configuration and state of the LED together with the pin to which it is connected and a boolean flag indicating wether the signal is inverted, meaning that the LED lights up when the pin is set to zero. The function initializes the struct, configures the output pin and switches the LED off according to the configuration.  
After that you can either set the LED to automatically blink or pulse in different rhythms or set the desired state manually.

```c
led_t led_example;

led_init(&led_example, GPIO4, false);

led_on(&led_example);//The LED is now on
led_toggle(&led_example);//The LED is now off
led_set(&led_example, true);//The LED is now on

led_set_pulsing_ms(&led_example, 50, 1950);//The LED will keep on turning on for 50ms after which it will be turned off for 1950ms
```

To stop the automatic blinking call any of the manual functions(`led_on`, `led_toggle`, etc).

## LED console

If `MODULE_ENABLE_DEBUG_CONSOLE` is set to true in `module_enable.h` you can manipulate an LED via the [debug console](../console/README.md) by calling `led_register` with a pointer to the `led_t` structure and a name to reference the given LED. Make sure to call `led_register` only after the debug console has been initialized, that is don't call it in `board_init`.

Command: "led"  
Subcommands: list, on, off, pulse  
|Subcommand|Parameters                |Explanation|
|----------|--------------------------|-----------|
|list      |                          |Lists all LEDs that were registered to the console|
|on        |\<name>                   |Turns the LED on|
|off       |\<name>                   |Turns the LED off|
|pulse     |\<name> \<on_ms> \<off_ms>|Configures the LED to automatically turn on for `on_ms` milliseconds and off for `off_ms`|

## RGB LED

### WS2812

WS2812 or NeoPixel LEDs are sets of RGB LEDs which can be individually addressed. Since the protocol is extremely time sensitive, we use source files for the implementations on different processors. At the moment only the ESP32 is supported.  
The LEDs are daisy chained, so each LED consumes one color value and passes the rest on to the next LED.

To use it create a handle to the module by calling `ws2812_create` with the hardware configuration and initialize it by calling `ws2812_init`. After that add as many pixels(that is RGB color values) as there are LEDs in your set with `ws2812_add_pixel` and call `ws2812_show` to send the color information to the LEDs.  
If the handle is no longer needed you can free the reserved buffers and stop the internal handling by calling `ws2812_free`.

```c
void ws2812_example(void)
{
    ws2812_hw_config_t config = {.num_led = 10, .dout = GPIO4 };
    ws2812_handle_t ws2812_handle = ws2812_create(&config);
    FUNCTION_RETURN_T ret = ws2812_init(ws2812_handle);

    ASSERT_RET(ret == FUNCTION_RETURN_OK, ws2812_free(ws2812_handle), NO_RETURN, "ws2812_init failed with return code %d\n", ret);
    
    color_rgb_t color_red = {.r = 255 };

    ws2812_add_pixels(&ws2812_handle, color_red, config.num_led);
    ws2812_show(&ws2812_handle);
    ws2812_free(ws2812_handle);
}
```
