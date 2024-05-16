# Sensor module

The sensor module is a collection of different sensors like temperature, light or proximity sensors.
Each sensor type has its own directory containing an abstract sensor module for it. The idea is to use the abstract sensor module inside the application to gather the sensor values while registering the physical sensor drivers to the abstract sensors inside the board. This way you can switch the physical sensors without affecting the application that uses them. Some sensors have the functionality for multiple sensor types. In this case look in the table below which sensors exist in the different directories and the functionality these provide.

Directory | Sensor | Functionality
--- | --- | ---
ad | tla2528 | AD
light | si115x | Light, Proximity
light | tsl2571 | Light

The sensor abstractions work all the same way as instructed below. Sample shows si115x, but the same works for different sensors.

```c

// Internal variables:

/// Handle for the sensor device is gathered in initialization of the sensor.
static sensor_device_handle_t _si115x = NULL;
/// Config for the sensor needs to be done once. The configuration differs between different sensors.
static si115x_config_t _si115x_config = {  };

// External variables:

/// Declare the light sensor
light_handle_t board_light = NULL;
/// Declare the proximity sensor
proximity_handle_t board_proximity = NULL;

void board_init(void)
{
    // MCU initalization and early stuff

    // Initialize the concrete sensor by giving the config and getting a pointer to a handle.
    _si115x = si115x_init(&_si115x_config);

    if(_si115x)
    {
        // Sensor was initialized successfully

        // Initialize the sensor by providing the handle for the sensor device and the interface to the sensor.
        board_light = light_init(_si115x, &si115x_light_interface); // The interfaces are declared in sensor header
        board_proximity = proximity_init(_si115x, &si115x_proximity_interface); // The interfaces are declared in sensor header

        /// Now you can use sensor functions
    }
}

void app_main_handle(void)
{
    // The sensors have a status function to check if sensor is working. Check this in case it stops working.
    if(light_get_state(board_light) == SENSOR_STATE_ACTIVE)
    {
        // Get the value from the sensor.
        lux_t result = light_get_lux(board_light);

        // Do somthing with it.
    }
    if(proximity_get_state(board_proximity) == SENSOR_STATE_ACTIVE)
    {
        // Get the value from the sensor.
        proximity_t result = proximity_get_value(board_proximity);

        // Do somthing with it.
    }
}

```

---

## Limitations

The ad sensor currently has no abstraction.

---
