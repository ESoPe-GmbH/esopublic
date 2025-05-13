# SLD Demo

## Hardware

This software demonstrate the usage of the [SMM display series from Schukat](https://shop.schukat.com/de/de/EUR/Optoelektronik/Anzeigen-Displays/LCD-Displays/LCD-Displays-Grafik/c/929?query=:relevance:allCategories:929:Serie_1699:SMM) using the [esopublic](https://github.com/ESoPe-GmbH/esopublic) library.

There are currently 3 supported boards:

Features | [SLD_C_W_S3](https://shop.schukat.com/de/de/EUR/ESoPe-GmbH/SLD-C-W-S3-1A/p/101762) | [SLD_C_W_S3_BT817](https://shop.schukat.com/de/de/EUR/ESoPe-GmbH/SLD-C-W-S3-BT817/p/103331) | SLD_C_W_P4
--- | --- | --- | ---
SMMx024240320H (2.4" 240 x 320) | ✓ | ❌ | ❌
SMMx035320240K (3.5" 320 x 240) | ✓ | ✓ | ✓
SMMx043480272G (4.3" 480 x 272) | ✓ | ✓ | ✓
SMMx050800480H (5.0" 800 x 480) | ❌ | ✓ | ✓
SMMx0701024600S (7.0" 1024 x 600) | ❌ | ✓ | ✓
Ext. UART | ✓ | ✓ | ✓
Ext. I2C | ❌ | ✓ | ❌
Ext. SPI | ❌ | ✓ | ❌
Ext. Audio | ❌ | ✓ | ❌
MIPI-CSI | ❌ | ❌ | ✓

The displays have can be connected to the board and can be used without any software change, regardless of the display size.

### [SLD_C_W_S3](https://esope.de/en/downloads-en?task=download.send&id=58&catid=2&m=0)

The board itself is driven by ESP32-S3-WROOM-1(U)-N16R8, which is a dual core Xtensa MCU with up to 240 MHz clock frequency. The board has a connector for power supply, a connector for the display and a connector for a piggy back PCB with 2 GPIOs. This enable the piggy back to make use of UART, RS485, CAN (TWAI) or I2C.

### [SLD_C_W_S3_BT817](https://esope.de/en/downloads-de?task=download.send&id=60&catid=2&m=0)

The board itself is driven by ESP32-S3-WROOM-1(U)-N16R8, which is a dual core Xtensa MCU with up to 240 MHz clock frequency. The display is driven by BT817 graphic chip, to control the bigger displays and have more free GPIO. The board has a connector for power supply, a connector for the display and a connector for a piggy back PCB with 19 GPIOs and the BT817 audio output.

### SLD_C_W_P4_C6

The board itself is driven by ESP32-P4, which is a dual core RISC-V MCU with up to 400 MHz clock frequency. The board has a connector for power supply, a connector for the display and a connector for a piggy back PCB with 2 GPIOs. This enable the piggy back to make use of UART, RS485, CAN (TWAI) or I2C. The pcb contains a MIPI-CSI interface to connect a camera and an ESP32-C6 for wifi communication.

To create the ESP32-C6 firmware, you can follow [this](https://github.com/espressif/esp-hosted-mcu/blob/main/docs/esp32_p4_function_ev_board.md#5-flashing-esp32-c6) instruction from Espressif. The SLD_C_W_P4_C6 uses the same pins for the ESP32-C6 as the function board.

## Setting up the Development Environment

For setup instructions, refer to the [English setup guide](docs/ide-setup-en.md) or the [German setup guide](docs/ide-setup-de.md).

## Build

The software is built using ESP-IDF 5.2 or 5.4. 5.4 is necessary to use with ESP32-P4. We recommend using VsCode with the ESP-IDF Extension for easy installation and usage of the espressif compiler. You can compile it either with the vscode function for compiling the esp-idf or by using the esp-idf prompt with the `idf.py build` command.

## UI generation

This demo shows how to use UI frameworks [Slint](https://slint.dev/), [LVGL](https://lvgl.io/) and the EVE interface of our esopublic, that utilizes the BT817 co-processor. Both have there advantages and you can decide for yourself which one you want to use. The default configuration of this project uses Slint.

### Slint

To use slint you have to make the following configuration:

/CMakeList.txt

```Cmake

# Make sure this is defined and set to true
set(KERNEL_ADD_DEPENDENCY_SLINT true)

```

main/idf_component.yml

```yml

dependencies:
  esope-gmbh/esopublic: "^25.0.0"
  slint/slint: "^1.8.0"
  idf:
    version: ">=5.2.0"

```

By enabling this configuration, the CMakeList.txt of esopublic sets a compiler define "KERNEL_USES_SLINT", which is checked internally and enables the main/app_ui_slint.cpp to use for the UI configuration. The main/ui directory contains the slint files, which are used to create the user interface.

### EVE

To use eve you have to make the following configuration:

/CMakeList.txt

```Cmake

# Make sure this is defined and set to false
set(KERNEL_ADD_DEPENDENCY_SLINT false)
set(KERNEL_ADD_DEPENDENCY_LVGL false)

```

main/idf_component.yml

```yml

dependencies:
  esope-gmbh/esopublic: "^25.0.0"
  slint/slint: "^1.8.0"
  idf:
    version: ">=5.2.0"

```

In the menuconfig, you must select the flag for SLD_C_W_S3_BT817 and make sure the /CMakeList.txt does not set slint or lvgl.

### LVGL

To use LVGL you have to make the following configuration:

/CMakeList.txt

```Cmake

# Make sure this line is commented or deleted
# set(KERNEL_ADD_DEPENDENCY_SLINT true)
# Is needed for generated image files to work directly
add_compile_definitions(LV_LVGL_H_INCLUDE_SIMPLE=1)
# This is needed for 5" display, to work propertly
add_compile_definitions(DISPLAY_NUM_FB=2)

```

main/idf_component.yml

```yml

dependencies:
  esope-gmbh/esopublic: "^25.0.0"
  lvgl/lvgl: "^9.1.0"
  idf:
    version: ">=5.2.0"

```

sdkconfig:

```bash

CONFIG_LCD_RGB_RESTART_IN_VSYNC=y
CONFIG_GDMA_CTRL_FUNC_IN_IRAM=y
CONFIG_COMPILER_OPTIMIZATION_PERF=y
CONFIG_LV_FONT_MONTSERRAT_10=y
CONFIG_LV_FONT_MONTSERRAT_14=y
CONFIG_LV_FONT_MONTSERRAT_24=y

```

By enabling this configuration, the CMakeList.txt of esopublic sets a compiler define "KERNEL_USES_LVGL", which is checked internally and enables the main/app_ui_lvgl.c to use for the UI configuration. The main/ui_lvgl directory contains assets to use for creating the lvgl user interface.

To prevent the display "drifting" (especially with 5" display), you should apply the sdkconfig changes above.
