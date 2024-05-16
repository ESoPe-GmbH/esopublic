# EEPROM

This module is used for simple EEPROMs that are used to store short information. Usually those EEPROMs have a size from 128 Byte to 65536 Byte.

The eeprom.c/.h contains the general access functions to access the EEPROM from the outside.

At the moment, we have drivers for I2C and SPI EEPROMs, but there are also serial EEPROMs, that might be supported in the future.

## Usage

With the samples below you can use the EEPROM.

### 1. Create a config for the driver and initialize the device's driver with it

```c
// Board.c
static const EEPROM_i2c_config_t _EEPROM_config = EEPROM_I2C_M24C01(&board_i2c_external, 0, 0, 0);

// Board.c/.h
EEPROM_device_t board_EEPROM_external;

// In board_init
board_EEPROM_external = EEPROM_i2c_init(&_EEPROM_config);
```

Now you have initialized the driver. For now the driver did not communicate with the device.

### 2. Write or read data

The code below shows a sample of how to read and write data.

```c
FUNCTION_RETURN ret = FUNCTION_RETURN_OK;
uint16_t address = 0;
uint8_t data[6] = {0};

ret = EEPROM_read(board_EEPROM_external, address, data, 6);
if(ret != FUNCTION_RETURN_OK)
    goto exit;

DBG_INFO("DATA R: %6Q\n", data);

strcpy((char*)data, "ESoPe");
ret = EEPROM_write(board_EEPROM_external, address, data, 6);
if(ret != FUNCTION_RETURN_OK)
    goto exit;

DBG_INFO("DATA W: %6Q\n", data);

ret = EEPROM_read(board_EEPROM_external, address, data, 6);
if(ret != FUNCTION_RETURN_OK)
    goto exit;

DBG_INFO("DATA R: %6Q\n", data);
```

The result output for an empty EEPROM can look like this:

```bash
18:23:49.009 tiko-t 000000349: app_main.c, 100       : DATA R: FFFFFFFFFF00
18:23:49.010 tiko-t 000000350: app_main.c, 107       : DATA W: 45536F506500
18:23:49.016 tiko-t 000000356: app_main.c, 113       : DATA R: 45536F506500
```
