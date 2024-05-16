// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file buffer.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Defines buffer structure.
 *
 *  @version	1.00 (29.10.2019)
 *  	- Intial release
 *
 *	@par 	References
 *
 **/
#ifndef MODULE_UTIL_BUFFER_H_
#define MODULE_UTIL_BUFFER_H_

#include <stdint.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Defines a structure for a single buffer. Containing a pointer to the buffer and the size of the buffer.
 */
typedef struct buffer_s
{
	/// Pointer to the buffer
	uint8_t* data;
	/// Size of the buffer
	int32_t len;
    /// Set this if data was allocated dynamically and needs to be freed afterwards. If macro BUFFER_FREE is used, this flag is checked.
	/// Is false by default to use static data buffer. Set to true manually when using dynamic buffer.
    bool free_data;
}buffer_t;

/**
 * Defines a structure for a transmission and a reception buffer. Containing a pointer to the buffers and the size of the buffers.
 */
typedef struct buffer_rxtx_s
{
	/// Receive buffer
	buffer_t rx;
	/// Transmit buffer
	buffer_t tx;
}buffer_rxtx_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Macro for initializing a buffer_t by using an array as a parameter.
#define BUFFER_INIT(buf)			{.data = buf, .len = sizeof(buf)}

/// Macro for initializing a buffer_rxtx_t by using arrays for rx (brx) and tx (btx) as parameters.
#define BUFFER_RXTX_INIT(brx, btx)	{.rx = BUFFER_INIT(brx), .tx = BUFFER_INIT(btx)}

/// Creates a buffer structure and two buffer and inserts the buffer into the structure.
#define DECLARE_BUFFER_RXTX(name_buf, name_rx, size_rx, name_tx, size_tx) \
		static uint8_t name_rx[size_rx] = {0}; \
		static uint8_t name_tx[size_tx] = {0}; \
		static buffer_rxtx_t name_buf = BUFFER_RXTX_INIT(name_rx, name_tx)

/**
 * Frees the buffer if the corresponding flag is set.
 * Clears the data pointer to NULL and clears the free flag.
 *
 * @param b			Pointer to the buffer_t object.
 */
#define BUFFER_FREE(b)		\
	do{ \
		if((b)->data && (b)->free_data)\
		{\
			mcu_heap_free((b)->data);\
			(b)->data = NULL;\
			(b)->free_data = false;\
		}\
		else{\
		}\
	}while(0)

#endif /* MODULE_UTIL_BUFFER_H_ */
