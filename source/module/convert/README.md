# Convert module

Contains modules that provide different modules to convert different values and collections. The respective modules can be (de-)activated by different precompiler definitions in `module_enable.h`

## Base64

Provides functions to encode binary data into base64 encoding or decode base64 encoded strings to binary data. For both of those applications you can either:

- en-/decode a single block by providing two arrays(the functions ending in `block`)
- en-/decode a complete buffer by providing a pointer to the input buffer, the output buffer and the length of the input(the functions ending in `buffer`)
- en-/decode a complete buffer in place by providing a pointer to the input buffer and the length of the input(the functions ending in `buffer_direct`)

## Math

Provides macros to calculate the maximum and minimum or the absolute difference between two numbers as well as a macro to constrain a number between a maximum and minimum value. Furthermore it offers
functions to calculate the number of digits of the decimal(`math_declen`) or hexadecimal representation(`math_hexlen`) of a 32bit unsigned integer. If `MATH_ENABLE_64BIT_OPERATIONS` in `module_config.h` is set to true you can also calculate a 64bit exponentiation with a modulo by using `math_pow_mod64`.

## Sort

Provides a very basic bubble sort implementation to sort an array of 32bit unsigned integers.

## String

Provides different string manipulation functions, most notably `string_printf` which provides a compiler agnostic and more lightweight implementation of the `sprintf` functionality from `string.h` expanded by some custom functionality. Other than that it provides a lot of functions to check and compare strings like finding the index of a substring or checking if a string contains only decimal or hexadecimal numbers, manipulating strings like trimming whitespace or changing a string to upper or lower case or converting numbers to strings or parse for example IPv4 addresses or a list of comma
separated numbers to an array of numbers.

## Swap

Provides functions to convert the endianness (byte order) of different number types like unsigned 16bit, 32bit and 64bit integers as well as floats.
