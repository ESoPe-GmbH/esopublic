# Util

Contains various utilities and data structures.

## Assert

Provides a macro called `ASSERT_RET` to check if a boolean condition is satisfied and print a message, execute some action and return from the current function if it's not. The message uses a variable argument list for the error printing.  
So the code:

```c
ASSERT_RET(a > 0, free(some_pointer), FUNCTION_RETURN_PARAM_ERROR, "Invalid value for a specified");
```

is synonymous with the code:

```c
if(!(a > 0))
{
    DBG_ERROR("Invalid value for a specified");
    free(some_pointer);
    return FUNCTION_RETURN_PARAM_ERROR;
}
```

If you want to use the macro in a function returning void you can use the define `NO_RETURN` in place of the return code. You could just leave it empty but using the define will improve readability. For the action a similar empty define is provided: `NO_ACTION`.  
By setting `ASSERT_PRINT_ERROR` in the header file to `1` or `0` you can globally activate or deactivate the error messages in the ASSERTs.  
To check variables for `NULL` a shorthand called `ASSERT_RET_NOT_NULL` is provided where you only need to supply the variable, the optional action and the return value. The error message that will be printed is `<variable_name> cannot be NULL`

## Bit Array

Can be used to create bit fields of arbitrary size(in theory, it is bound to sizeof(size_t) bits in practice). Calling `bit_array_create` with the number of bits you need will return a `bit_array_handle_t` or `NULL` if the memory allocation failed. After that you can set specific bits by calling `bit_array_set`, clear them with `bit_array_clear` or check if a bit is set with `bit_array_is_set`. There are also functions to clone a bit array, compare a bit array, clear all bits or check if any bits are set.  
If you no longer need the bit array you can free the allocated memory by calling `bit_array_free`

```c
void bit_array_example(void)
{
    bit_array_handle_t bit_array = bit_array_create(100);//Creates a bit array with 100 bits. Allocates space for the bit_array_t structure and 13 bytes to store the 100 bits

    if(bit_array == NULL)
        return;

    bit_array_set(bit_array, 50);//Set the 51st bit to 1

    if(bit_array_is_set(bit_array, 50))//Checks if the 51st bit is set
    {
        DBG_VERBOSE("The 51st bit is set");
    }

    bit_array_clear(bit_array, 50);//Now it is cleared
    bit_array_free(bit_array);
}
```

## Buffer

Provides a structure `buffer_t` containing a buffer, the size of the buffer and a flag indicating wether the buffer was dynamically allocated. It also contains the definition of a structure containing two `buffer_t` entries, that can be used as receive and transmit buffers for a communication interface. It also provides macros to concisely declare and initialize these buffers.

## Color

Contains a structure to store color information in RGB format and another structure for HSV format. For the `color_rgb_t` you can set the red, green and blue components separately or use the 32bit `value` to set all three values at the same time with the blue value as the lowest byte. It contains functions to convert from RGB to HSV and vice versa and a function to change the perceived brightness of an RGB color value by a percent value.

```c
color_rgb_t color = {.r = 0xAB, .g = 0xCD, .b = 0xEF };//This will result in the same color
color.value = 0xABCDEF;//As this
```

Using the `value` of the `` structure will allow you also to use "%06x" inside a printf to print them as color hex codes that are commonly used in the web.

## MemPool

The memory pool is used to create and store buffers in smaller chunks that can be used and freed without always allocating and de-allocating memory from the heap.  
The advantage using the memory pool is to reduce the fragmentation of the heap by directly allocating a bigger pool and then using these buffers without allocating/de-allocating them all the time.  


You initialize the mempool by calling `mem_pool_init` and specifying the number and size of chunks the pool shall contain. After that you can allocate contiguous regions in the pool `mem_pool_alloc_chunk` with the desired size of the region. You can write and read to and from the chunk by using `mem_pool_write_chunk`, `mem_pool_append_chunk` and `mem_pool_read_chunk`. After you no longer need the chunk free it with `mem_pool_free_chunk`. To grow the size of a chunk use `mem_pool_realloc_chunk` with the new desired size.  
`mem_pool_get_chunk_total_size` tells you the total size(free + used) of the chunk, while `mem_pool_get_chunk_free_size` and `mem_pool_get_chunk_used_size` tell you the free size in bytes or used size respectively.

## Network util

Provides macros to print an IPv4 address using printf as well as a function to check if a MAC address is a unicast address.

```c
uint8_t my_ip[4] = { 192, 168, 168, 1 };
DBG_VERBOSE("My IP: " IPSTR "\n", IPPTR2STR(my_ip));
```

will result in something similar to this being printed

```bash
My IP: 192.168.168.1
```

## Pair

Provides a structure for a key/value string pair(`pair_str_t`) and macros to initialize the structure(`PAIR_STR_INIT`) and to check the number of entries in an array of `pair_str_t`(`PAIR_STR_SIZE`).

## PMOD interface

Pmod interface (peripheral module interface) is an open standard defined by Digilent in the PMOD Interface Specification for connecting peripheral modules to microcontrollers.  
This module can be used to quickly initialize the communication interface for these peripherals by configuring the interface type(SPI, I2C, UART, I2S or just GPIO) and the needed I/O pins for that interface and calling `pmod_init`, which internally calls SPI / I2C / UART initializations based on the declared interface of the PMOD. When you no longer need the peripheral you can free the underlying device handlers by calling `pmod_free`