/**
 * 	@file 	bit_array.h
 * 	@copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Function to store large bitmask in an array with functions to set and clear the bit as well as checking if it is set.
 *
 *  @version	1.00 (31.01.2023)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/
#ifndef __UTIL_BIT_ARRAY__FIRST_INCL
#define __UTIL_BIT_ARRAY__FIRST_INCL

#include "module_public.h"
#include "module/enum/function_return.h"

//------------------------------------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------
// Type Definition
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// Config
//------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure and Enum
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Type to represent a bit array that is used as parameter in the functions.
 * 
 */
typedef struct bit_array_s
{
    uint8_t* array;
    /// Size of the array.
    size_t size;
}bit_array_t;

/// Handle for bit array structure that is allocated dynamically inside bit_array_create.
typedef struct bit_array_s* bit_array_handle_t;

//------------------------------------------------------------------------------------------------------------
// External functions
//------------------------------------------------------------------------------------------------------------

/**
 * @brief Create a handle for a bit array.
 * You need to call `bit_array_free` if you do not need it anymore.
 * 
 * @param num_bits              Maximum number of bits that need to be stored inside the bit array.
 * @return                      Pointer to the handle or NULL if creation failed.
 */
bit_array_handle_t bit_array_create(size_t num_bits);
/**
 * @brief Creates a bit array from a given byte array and its size in bytes
 * 
 * @param bytes                 The bytes to use as the content of the bitarray
 * @param num_bytes             The number of bytes to reserve/copy
 * @return bit_array_handle_t   Pointer to the handle or NULL if creation failed.
 */
bit_array_handle_t bit_array_create_from_bytes(uint8_t* bytes, size_t num_bytes);
/**
 * @brief Frees tha handle for a bit array.
 * 
 * @param ba                    Handle for the bit array as created using `bit_array_create`.
 */
void bit_array_free(bit_array_handle_t ba);
/**
 * @brief Clone a handle for a bit array.
 * You need to call `bit_array_free` if you do not need it anymore.
 * 
 * @param ba                    Handle for the bit array as created using `bit_array_create`.
 * @return                      New handle for a bit array with same values as previous handle.
 */
bit_array_handle_t bit_array_clone(bit_array_handle_t ba);
/**
 * @brief Sets a bit inside the bit array.
 * 
 * @param ba                    Handle for the bit array as created using `bit_array_create`.
 * @param index                 Index of the bit that should be set to 1.
 * @return FUNCTION_RETURN_T    FUNCTION_RETURN_OK on success or parameter error on error.
 */
FUNCTION_RETURN_T bit_array_set(bit_array_handle_t ba, size_t index);
/**
 * @brief Clears a bit inside the bit array.
 * 
 * @param ba                    Handle for the bit array as created using `bit_array_create`.
 * @param index                 Index of the bit that should be cleared to 0.
 * @return FUNCTION_RETURN_T    FUNCTION_RETURN_OK on success or parameter error on error.
 */
FUNCTION_RETURN_T bit_array_clear(bit_array_handle_t ba, size_t index);
/**
 * @brief Clears all bits inside the bit array.
 * 
 * @param ba                    Handle for the bit array as created using `bit_array_create`.
 * @return FUNCTION_RETURN_T    FUNCTION_RETURN_OK on success or parameter error on error.
 */
FUNCTION_RETURN_T bit_array_clear_all(bit_array_handle_t ba);
/**
 * @brief Sets or clears a bit inside the bit array.
 * 
 * @param ba                    Handle for the bit array as created using `bit_array_create`.
 * @param index                 Index of the bit that should be set or clered.
 * @param value                 true to set the bit or false to clear the bit.
 * @return FUNCTION_RETURN_T    FUNCTION_RETURN_OK on success or parameter error on error.
 */
FUNCTION_RETURN_T bit_array_set_value(bit_array_handle_t ba, size_t index, bool value);
/**
 * @brief Check whether a bit is set inside the bit array.
 * 
 * @param ba                    Handle for the bit array as created using `bit_array_create`.
 * @param index                 Index of the bit that should be checked.
 * @return true                 The bit was set.
 * @return false                The bit was not set.
 */
bool bit_array_is_set(bit_array_handle_t ba, size_t index);
/**
 * @brief Check if any bit inside the bit array is set.
 * 
 * @param ba                    Handle for the bit array as created using `bit_array_create`.
 * @return true                 One or more bits are set to 1.
 * @return false                All bits are 0.
 */
bool bit_array_has_any_set(bit_array_handle_t ba);

/**
 * @brief Compares two bit arrays
 * Considers whether they are NULL, their size and their content
 *
 * @param ba1				Handle for the first bit array to be compared
 * @param ba2				Handle for the second bit array to be compared
 * @return 					0: Both bit arrays contain the same bits\n
 * 							Otherwise they differ in some way
 */
int bit_array_compare(bit_array_handle_t ba1, bit_array_handle_t ba2);

#endif // __UTIL_BIT_ARRAY__FIRST_INCL
