// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 		vcomm.h
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		Virtual comm interface that allocates a buffer for received data and has an input function that will put data into the receive buffer.
 *              There is also an output callback to send data that was written into by the put functions.
 *
 *  @version	1.00 (11.09.2022)
 *  			 - Initial release
 *
 ******************************************************************************/

#ifndef COMM_VCOMM_HEADER_FIRST_INCLUDE_GUARD
#define COMM_VCOMM_HEADER_FIRST_INCLUDE_GUARD
 
#include "module_public.h"
#if MODULE_ENABLE_COMM_VCOMM

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the comm module
#define VCOMM_STR_VERSION		"1.00"

#include "module/comm.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Handle for the virtual comm that is created using `vcomm_create`.
typedef struct vcomm_s* vcomm_handle_t;
/// Output function that is called when data is written into the comm interface.
/// @param comm         Virtual comm handle that was created using `vcomm_create`.
/// @param buffer       Pointer to the data that is outputted.
/// @param length       Number of bytes in buffer for output.
typedef void (*vcomm_output_cb_t)(vcomm_handle_t vcomm, uint8_t* buffer, size_t length);
/// Function that is called when transmit ready of comm interface is checked.
/// If NULL, only the presence of output_cb will determine if true is returned.
/// @param comm         Virtual comm handle that was created using `vcomm_create`.
/// @return     True if output can be called. If NULL, output will be called whenever data is written into virtual comm.
typedef bool (*vcomm_output_ready_cb_t)(vcomm_handle_t vcomm);
/// Flush function that is called when flush of comm interface is called.
/// Can be NULL if not needed.
/// @param comm         Virtual comm handle that was created using `vcomm_create`.
typedef void (*vcomm_output_flush_cb_t)(vcomm_handle_t vcomm);
/**
 * @brief Initialization structure for the virtual comm.
 */
typedef struct vcomm_init_s
{
    /// User-Pointer that can be gotten by vcomm_get_user
    void* user;
    /// Size of the receive buffer that should be allocated internally for storing the data written into the vcomm by using vcomm_input.
    size_t rx_buffer_size;
    /// Pointer to output function that is called when data is written into the comm interface.
    vcomm_output_cb_t output_cb;
    /// Pointer to output function that is called when transmit ready of comm interface is checked.
    /// If NULL, only the presence of output_cb will determine if true is returned.
    vcomm_output_ready_cb_t output_ready_cb;
    /// Pointer to the flush function that is called when flush of comm interface is called.
    /// Can be NULL if not needed.
    vcomm_output_flush_cb_t output_flush_cb;
}vcomm_init_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Create a virtual comm handler.
 * 
 * @param init      Pointer to the initialization structure.
 * @return          Pointer to the created virtual comm handler or NULL if it failed.
 */
vcomm_handle_t vcomm_create(const vcomm_init_t* init);
/**
 * @brief Free a virtual comm handler.
 * 
 * @param vcomm     Virtual comm handle that was created using `vcomm_create`.
 */
void vcomm_free(vcomm_handle_t vcomm);
/**
 * @brief 
 * 
 * @param vcomm     Virtual comm handle that was created using `vcomm_create`.
 * @param buffer    Pointer to the buffer containing data to put into receive buffer of virtual comm.
 * @param length    Pointer to the number of bytes inside buffer to put into receive buffer of virtual comm. 
 *                  After calling it contains the number of bytes that were stored inside the buffer.
 * @return          FUNCTION_RETURN_OK on success or other value on failure. 
 */
FUNCTION_RETURN_T vcomm_input(vcomm_handle_t vcomm, uint8_t* buffer, size_t* length);
/**
 * @brief 
 * 
 * @param vcomm     Virtual comm handle that was created using `vcomm_create`.
 * @return          Comm interface that can be used for this virtual comm.
 */
comm_t* vcomm_get_comm(vcomm_handle_t vcomm);
/**
 * @brief Get the user pointer from the initialization structure of the virtual comm.
 * 
 * @param vcomm     Virtual comm handle that was created using `vcomm_create`.
 * @return          User pointer that was set in the initialization structure.
 */
void* vcomm_get_user(vcomm_handle_t vcomm);

#endif // MODULE_ENABLE_COMM_VCOMM

#endif // COMM_VCOMM_HEADER_FIRST_INCLUDE_GUARD
