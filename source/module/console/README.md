# Console

Can be used to create a line based human machine or machine machine interface. You can configure this module using the directives in `module_config.h`, for example changing the size of the
receive buffer or the time after which an incomplete line will be dropped.  
You create a structure to contain the state of the console and initialize it with `console_init` specifying the comm interface you want to associate the console interface with.  
After that you can add as many commands to the console as you need. A console command consists of a string representing the command, a pointer to the function that will be called upon reception of the keyword and a string containing a description of the command. This will be printed if the `help` command is entered.  
The input to the callback function will either be the rest of the received line after the command(minus the space) if use_array_param is `false` or a string array of the parameters, which is the line split by space not escaped in quotes.  
In the callback function you can send a formatted response by using the `console_set_response_dynamic` function or a static response by using `console_set_response_static`.  
If you return a value other that `FUNCTION_RETURN_OK` the return value will be sent to the communication interface with the prefix "!err".

```c
static FUNCTION_RETURN debug_app_execute(console_data_t* data, char** args, uint8_t args_len);

void debug_console_init(comm_t* comm)
{
    console_data_t console;
    console_init(&console, comm);
    console_add_command((console_command_t){ .command = "app", .explanation = "subcommand version: Prints the software version", .fnc_exec = debug_app_execute, .use_array_param = true });
}

static FUNCTION_RETURN debug_app_execute(console_data_t* data, char** args, uint8_t args_len)
{
    if(args_len > 0)
	{
		if(strcmp(args[0], "version") == 0)
		{
			return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 20, "version %s", version_get_string());
		}
        else
			return FUNCTION_RETURN_NOT_FOUND;
    }
	else
		return FUNCTION_RETURN_NOT_FOUND;
}
```

With the code above if you send the following string over the interface:

```bash
app version\n
```

you will get the response:

```bash
res 0 "OK" "version 24.001"
    │  │    └─message
    │  └─string representation of the FUNCTION_RETURN_T
    └─numeric representation of the FUNCTION_RETURN_T
```

## Debug Console

The debug console modules provide a number of predefined commands. Some are active automatically when `MODULE_ENABLE_DEBUG_CONSOLE` is set to true in `module_enable.h` and some can be (de-)activated by
setting the corresponding defines in `module_config.h`.

### Debug app

Command: "app"  
Subcommands: version, reset  
|Subcommand|Explanation|
|----------|-----------|
|version|Prints the software version.|
|reset| Resets the controller.|

### Debug flag

Command: "flag"  
No subcommands

You can register a flag with a name and a pointer to a function by calling `debug_console_register_flag`. When the command is received with the name of your flag the function will be called.  
You can use this module to check the state of a variable at runtime. To do that you need to call `console_set_response_dynamic` with a return value of FUNCTION_RETURN_OK in your callback function
and provide the state of the variable in the formatted message.

### Debug identification

Command: "id"
No subcommands

Basically a special case of the flag command without a name parameter. You can use it to check if the firmware is running on the expected hardware.  
You register a callback function by calling `debug_console_register_identification`. In it you can run code to determine if the hardware is as expected, for example checking the state of a GPIO pin
with a known state or checking if the communication with a certain peripheral was successfully initialized.  
If it is return `true` from the callback function. If `false` is returned the command will
print the result "WRONG_DEVICE"(8) to the console output

### Debug I/O

Command: "io"  
Subcommands: dir, set, get, toggle  
|Subcommand|Parameters      |Explanation|
|----------|----------------|-----------|
|dir       |\<pin> (in\|out)|Sets the direction of an I/O|
|set       |\<pin> (h\|l)   |Sets the output value of the pin to High (h) or Low (l)|
|get       |\<pin>          |Returns the value of the pin H(igh) or L(ow)|
|toggle    |\<pin>          |Toggles the output pin|

\<pin> is used as a string form of the `MCU_IO_PIN` enumeration for the controller

### Debug Test

Command: "test"  
Subcommands: start

`test start <password>`: Enters the pcb testing mode, further arguments depend on application. Responds with `+testrsp start` when authorization is OK.
You can set the password via the `DEBUG_CONSOLE_TEST_PASSWORD` define in `module_config.h`. By calling `debug_console_register_test_callback` you can register a callback function that will be called
when `test start` is received with a valid password. There you can execute commands to enter testing mode, like stopping the regular business logic or resetting device handlers, etc.

### Debug UART

Command: "uart"  
Subcommands: init, put, get, clear, list
|Subcommand|Parameters            |Explanation|
|----------|----------------------|-----------|
|init      |<num/name> \<baudrate>|Initializes the UART by setting the baudrate and parameters as well as a receive buffer. Responds with `+uartrsp init <baudrate>`. Always uses 8N1.|
|put       |<num/name> "\<base64>"|Writes the data from the decoded base64 string to the UART. Responds with `+uartrsp put <num>`, where \<num> is the number of sent bytes.|
|get       |<num/name> \<num>     |Returns the read bytes with a maximum of \<num> from the uart in a `+uartrsp get "<base64>"` response, where the string in the quotes might be empty.|
|clear     |<num/name> rx/tx      |Clears the rx or tx buffer. At the moment only rx is supported. Response is `+uartrsp clear rx/tx`|
|list      |                      |Print a list of all UARTs.|

To use the module you need to register the UART by calling `debug_console_register_uart` with a pointer to a `debug_console_uart_t` structure to store the configuration and state of the debug UART handler, the handler of the actual UART itself, a shorthand name to reference it, a number to reference it, an optional pointer to a buffer to store received bytes and the size of the buffer.
