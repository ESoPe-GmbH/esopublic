# comm module

## Comm interface

Offers functions to read and write to and from different communication interfaces as an abstraction for other modules to use independent of the underlying implementation.  
Many functions in this module work similar to functions in stdio.h, but do not need as much RAM/ROM.
Also the stdio.h functions differ when using different compiler, so this module is a solution that works with all.

### vcomm(Virtual Comm)

Implementation of the comm interface that allocates a buffer for received data and a function to function to input data into it. Also it provides a callback function that will be called
each time data is "send" via the `put` function. Can be used to simulate a communication interface.

## Debug interface

Can be used to send formatted debug information over a communication interface.  
It provides three different macros representing different levels of verbosity:

1. Verbose
1. Info
1. Error
1. None

By using the corresponding macros `DBG_VERBOSE`, `DBG_INFO` and `DBG_ERROR` and setting the desired `DEBUG_LEVEL` in the `module_config.h` you can control which debug statements will be
printed to the communication interface.
These macros take a format string with optional arguments and add to it the system clock, filename and line number where the macro is used. For example this invocation on line 123 in a file called `app_main.c`

```c
uint8_t test = 80;
DBG_VERBOSE("The value of test is %d\n", test);
```

will result in a line similar to this being send over the comm interface:

```
000000619: app_main.c, 123          : The value of test is 80
│          └─file name └─line number  └─message
└─system clock            
```

As you can see the message is not automatically appended by a newline.

Furthermore this module offers a macro called `DBG_ASSERT` consisting of a boolean condition, an optional action, an optional return value and the message to be send when the condition is not met. That is
the following code:

```c
FUNCTION_RETURN_T test_function(void)
{
    uint32_t* pointer = mcu_heap_calloc(1, sizeof(uint32_t));
    DBG_ASSERT(pointer, NO_ACTION, FUNCTION_RETURN_INSUFFICIENT_MEMORY, "Pointer could not be allocated\n");
    // [...]
    return FUNCTION_RETURN_OK;
}
```

is synonymous with the following code:

```c
FUNCTION_RETURN_T test_function(void)
{
    uint32_t* pointer = mcu_heap_calloc(1, sizeof(uint32_t));
    if(!pointer)
    {
        DBG_ERROR("Pointer could not be allocated\n");
        return FUNCTION_RETURN_INSUFFICIENT_MEMORY;
    }
    // [...]
    return FUNCTION_RETURN_OK;
}
```

## Line reader

Can be used to implement line based protocols on a communication interface. To access to the underlying comm interface you need to configure two callback functions, one to check if there is data available to be read and the other to read a single
character from the interface.  
You can either use the internal by setting `LINE_READER_USE_TASK` to true in `module_config.h` and wait until `line_reader_ready` returns true or actively
poll it until it returns `true`.  
After you handled the received line you have to call `line_reader_clear` before the next line can be received.  
Alternatively you can spawn `line_reader_receive` as a subtask from a protothread which will wait until a line is received and clear it afterwards with an optional timeout and an optional function pointer to cancel the receive from outside the module.

## UART TLS

Provides an encrypted communication interface over UART. The TLS encryption is done using MbedTLS.

## Bit-banging SPI and I2C

Provides a wrapper over the MCU SPI and I2C functionality that also provides manual(bit-banging) implementations of those protocols for applications where native drivers are not available.