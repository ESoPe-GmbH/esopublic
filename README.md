# esopublic

## Usage

To use this library with an ESP32 you need to use the ESP-IDF. It is tested to work with version 4.4.6 and 5.2.1.

Create a folder called `components` in the root directory of your ESP-IDF project if it is not already present and copy this folder into it or link the repository as a git submodule by running the following command in the root directory of the project

```bash
git submodule add git@<repository-url>.git components/esopublic
```

Create a directory called `config` in the `components` folder and copy the contents of the `template` folder into it.

Modify the `CMakeLists.txt` in the `main` directory to require the `esopublic` component. For example:

```cmake
idf_component_register(SRCS "main.c" INCLUDE_DIRS "." REQUIRES esopublic)
```

The resulting project structure should look like this:

```bash
.
├── components
│   ├── config
│   └── esopublic
└── main
```

## Description

For a description what the different modules of this library do check the readme files in the respective subfolders in the `source/modules` directory.

## Configuration

The files in the configuration folder can be used to adjust the library to your specific needs.

### mcu_selection.h

Here you can enable/disable certain peripherals like the realtime clock or watchdog and specify how many instances of certain peripherals like I/O interrupts, UARTs or SPI interfaces your project needs.

### sys_config.h

Contains options for the task handling module like wether to enable/disable statistical analysis of task or the option to get a notification when a task runs for more than a given number of milliseconds

### module_enable.h

With the pre-compiler defines in this module you can enable/disable the individual modules in the `source/modules` directory. Consider that some modules depend on other modules. To use these modules the other modules need also to be enabled.

### module_config.h

Here you can change settings related to the modules enabled in `module_config`

### gui_config.h

Contains settings related to the `gui` module like the default text color, background color for the screens or default font sizes to use for the gui components

## Building

If you're using Visual Studio Code with the ESP-IDF extension you can use the shortcut `Ctrl+B E` to build or use `Ctrl+Shift+P` to open the command palette, search for `build` and select the entry
`ESP-IDF: Build your project`.  
Alternatively you can use the esp-idf command prompt. Navigate to the root directory of your project and enter the command `idy.py build`.
