// Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file 	fifo.h
 * 	@copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * 	@author Tim Koczwara
 *
 *  @brief
 *  		The fifo module offers a structure to handle storing and accessing data inside a buffer.
 *
 *  		The initialization function defines the used buffer, size of a single element and the number of elements
 *  		that can be stored.
 *
 *  		This offers the functionality to not only use it for single bytes but for whole package streams with 100 bytes or
 *  		more.
 *
 *  @version	1.11 (01.03.2023)
 * 		- Added fifo_get_maximum
 *	@version	1.10 (19.01.2022)
 * 	    - Modified to be used in esopekernel
 *  @version    1.09 (01.08.2019)
 *      -  Fixed a bug in fifo_freespace because maximum of fifo is max len - 1.
 *	@version	1.08 (30.01.2019)
 *		- Fixed a bug in fifo_data_available where the number of available bytes is wrong when the write pointer is starting at 0,
 *			while the read pointer is still above 0.
 *	@version	1.07 (06.08.2018)
 *		- Added fifo_freespace and fifo_contains
 *	@version	1.06 (07.06.2018)
 *		- Added module.h
 *	@version 	1.05 (24.05.2018)
 *		- Changed function to stop using the mcu_enable_interrupt and mcu_disable_interrupt
 *	@version 	1.04 (31.12.2014)
 *		- Added fifo_get_ptr
 * 		- Median functions for 8-bit and 32-bit added
 * 		- Added FIFO_USE_AVERAGE and FIFO_USE_MEDIAN
 *	@version 	1.03 (20.02.13)
 *		- Translated to english
 *		- Removed How-To-Use because it makes the whole file less readable and is only needed when someone is new to this.
 *			In this case main_fifo.c with the examples can be used for it.
 *	@version 	1.02 (16.02.13)
 *		- Inline functions moved to the fifo.c
 *	@version 	1.01 (26.08.2011)
 *		- Removed mytypedef.h and included stdbool and stdint types.
 *		- Renamed module vom ringbuffer to fifo.
 *  @version  	1.00
 *  	- Initial release
 *
 ******************************************************************************/

#ifndef FIFO_HEADER_FIRST_INCLUDE_GUARD
#define FIFO_HEADER_FIRST_INCLUDE_GUARD

#include "module_public.h"
#if MODULE_ENABLE_FIFO
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the crc module
#define FIFO_STR_VERSION "1.11"

//------------------------------------------------------------------------------------------------------------
// Structures
//------------------------------------------------------------------------------------------------------------
/**
 * @struct fifo_t
 * 		Structure for fifo objects used in all functions.
 */
typedef struct{
	uint8_t *data;				///< Pointer to the buffer used for storing data.
	uint16_t element_size;		///< Size of a single element inside the buffer
	uint16_t max_elements;		///< Maximum number of elements that can be put inside the buffer
	uint16_t max_len;			///< Maximum size of the buffer: element_size * max_elements
	uint16_t read_pos;			///< Current position for reading elements from the buffer.
	uint16_t write_pos;			///< Current position for the next element written into the buffer.
//	uint16_t entries;			///< Number of elements currently stored inside the fifo.
}fifo_t;

//------------------------------------------------------------------------------------------------------------
// Enumerations
//------------------------------------------------------------------------------------------------------------
/**
 * @enum FIFO_RESULT
 * 		Errorcode used, when initializing the fifo.
 */
typedef enum{
	FIFO_OK = 0,						///< No error occured.
	FIFO_ELEMENTSIZE_INVALID = 1,		///< Invalid element size (e.g. elementsize 0).
	FIFO_BUFFERSIZE_INVALID = 2			///< Buffer size exceeds 65536 Bytes.
}FIFO_RESULT;

//------------------------------------------------------------------------------------------------------------
// External functions
//------------------------------------------------------------------------------------------------------------
/**
 * @brief 		Initializes the fifo structure and includes the buffer.
 *
 * @param bs				Pointer to the fifo_t that needs to be initialized.
 * @param elementsize		Size of a single element inside the buffer.
 * @param buf				Pointer to the buffer with a size of elementsize * total_elements.
 * @param total_elements	Maximum number of elements inside the buffer.
 * @return					FIFO_OK:					Everything initialized without an error.\n
 * 							FIFO_ELEMENTSIZE_INVALID:	Elementsize is 0. That can not work.\n
 * 							FIFO_BUFFERSIZE_INVALID:	Buffer size (elementsize * total_elements) exceeds 65536 Bytes.
 */
FIFO_RESULT fifo_init(fifo_t* bs, uint8_t elementsize, void* buf, uint16_t total_elements);

/**
 * @brief 		Structure is reset, so that it does not contain any elements.
 *
 * @param bs				Pointer to the fifo_t to be used.
 */
void fifo_clear(fifo_t *bs);

/**
 * @brief 		Adds an element to the fifo. The pointer needs to point to an element with the defined element size.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @param c					Pointer to the element that should be added.
 * @return					true:	Element was added.\n
 * 							false:	Element was not added because the buffer is full.
 */
bool fifo_put(fifo_t* bs, uint8_t* c);

/**
 * @brief 		Adds a byte to the fifo. Can be used if the defined element size is 1.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @param c					8-Bit Element that should be added.
 * @return					true:	Element was added.\n
 * 							false:	Element was not added because the buffer is full.
 */
bool fifo_put8(fifo_t*bs, uint8_t c);

/**
 * @brief 		Adds 2 bytes to the fifo. Can be used if the defined element size is 2.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @param c					16-Bit Element that should be added.
 * @return					true:	Element was added.\n
 * 							false:	Element was not added because the buffer is full.
 */
bool fifo_put16(fifo_t* bs, uint16_t c);

/**
 * @brief 		Adds 4 bytes to the fifo. Can be used if the defined element size is 4.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @param c					32-Bit Element that should be added.
 * @return					true:	Element was added.\n
 * 							false:	Element was not added because the buffer is full.
 */
bool fifo_put32(fifo_t*bs, uint32_t c);

/**
 * @brief 		Reads a byte from the fifo. The pointer needs to point to an element that can store the defined element size.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @param c					Points to the buffer where the read element should be stored.
 * @return					true:	Element was copied into c.\n
 * 							false:	Buffer was empty, so nothing to read.
 */
bool fifo_get(fifo_t* bs, uint8_t* c);

/**
 * @brief		Returns a pointer to the element inside the fifo without copying the element to a new location.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					Pointer to the element that can be read or NULL if nothing is to read.
 */
uint8_t* fifo_get_ptr(fifo_t* bs);

/**
 * @brief 		Returns a single byte from the buffer. Can be used if element size is 1.
 *
 * @pre			Check with fifo_data_available if an element is available to be read! Otherwise it returns 0.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					The 8-Bit element read from the buffer.
 */
uint8_t fifo_get8(fifo_t* bs);

/**
 * @brief 		Returns a word from the buffer. Can be used if element size is 2.
 *
 * @pre			Check with fifo_data_available if an element is available to be read! Otherwise it returns 0.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					The 16-Bit element read from the buffer.
 */
uint16_t fifo_get16(fifo_t* bs);

/**
 * @brief 		Returns a dword from the buffer. Can be used if element size is 4.
 *
 * @pre			Check with fifo_data_available if an element is available to be read! Otherwise it returns 0.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					The 32-Bit element read from the buffer.
 */
uint32_t fifo_get32(fifo_t* bs);

/**
 * @brief 		Returns the number of stored elements inside the fifo.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					Number of stored elements inside the fifo.
 */
 uint16_t fifo_data_available(fifo_t *bs);
/**
 * @brief		Number of bytes that can be put into the fifo.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					Number of elements that fit inside the fifo.
 */
 uint16_t fifo_freespace(fifo_t* bs);
/**
 * Checks if element is contained in List. Len is the maximum length of the element. If the element size of the fifo is smaller, then only the element size
 * is compared
 * @param bs		Pointer to the fifo
 * @param element	Pointer to the element that could be in the list
 * @param len		Maximum length of the elements size.
 * @return			true if element is inside the list, false if not.
 */
 bool fifo_contains(fifo_t* bs, uint8_t* element, uint8_t len);

/**
 * @brief 		Indicates if the fifo is full.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					true: 	The buffer is full.\n
 * 							false:	There is still place for more elements inside the buffer.
 */
bool fifo_is_full(fifo_t *bs);

#if FIFO_USE_AVERAGE
/**
 * @brief 		Calculates the average value of all elements in the buffer and returns it.
 *
 * @attention	Only available for 1, 2 and 4 byte element size. It will not work for other element sizes.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					Average value of all fifo elements.
 */
uint32_t fifo_get_average(fifo_t *bs);
#endif

/**
 * @brief 		Returns the maximum value inside the fifo.
 * Uses fifo_get, so do not use it if you read the values manually.
 * 
 * @param bs				Pointer to the fifo_t to be used.
 * @return					Maximum value of all fifo elements.
 */
uint32_t fifo_get_maximum(fifo_t *bs);

#if FIFO_USE_MEDIAN
/**
 * @brief 		Calculates the median value of all elements in the buffer and returns it.
 *
 * @attention	Only available for 2 byte element size. It will not work for other element sizes.
 *
 * @param bs				Pointer to the fifo_t to be used.
 * @return					Median value of all fifo elements.
 */
uint32_t fifo_get_median(fifo_t *bs);
#endif

#endif

#endif
