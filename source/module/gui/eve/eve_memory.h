/**
 * 	@file eve_memory.h
 *  @copyright Urheberrecht 2017-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		The memory manager of the eve manages the available RAM of the EVE to dynamically load images that are shown
 *  			on the screen, the fonts needed always and the audio that can be played.
 *
 *  @version	1.00 (13.09.2017)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_GUI_EVE_EVE_MEMORY_H_
#define MODULE_GUI_EVE_EVE_MEMORY_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "eve.h"

/**
 * Clears all stored memory pointer. Is needed if eve is restarted.
 *
 * @param eve				Pointer to the EVE device
 */
void eve_memory_clear(eve_t* eve);

/**
 * Registers a filename as a memory object. Returns the memory object for this file object.
 *
 * @param eve				Pointer to the EVE device
 * @param filename			Filename of the object
 * @param data 				Optional: Pointer to the buffer where the complete object information is stored.
 * 							Is used when the information is stored in the flash. If the information is provided on mmc,
 * 							this parameter should be set to NULL.
 * @param length			Number of bytes to write into memory for this object.
 * @return					NULL if no memory object can be registered or pointer to the memory object.
 */
eve_memory_file_t* eve_memory_register(eve_t* eve, const char* filename, const uint8_t* data, uint32_t length);

/**
 * Registers a filename as a memory object. Returns the memory object for this file object.
 *
 * @param eve				Pointer to the EVE device
 * @param filename			Filename of the object
 * @param flash_address 	Address of the object in the flash memory of the eve chip.
 * @param length			Number of bytes to write into memory for this object.
 * @return					NULL if no memory object can be registered or pointer to the memory object.
 */
eve_memory_file_t* eve_memory_register_from_external_flash(eve_t* eve, const char* filename, uint32_t flash_address, uint32_t length);

/**
 * Returns address where data according to the needed space can be stored.
 *
 * @param eve				Pointer to the EVE device
 * @param space_needed		Number of bytes that need to be written into the RAM.
 * @return					Address of the RAM that can be used or 0xFFFFFFFF, when there is no free space.
 */
uint32_t eve_memory_get_address(eve_t* eve, uint32_t space_needed);

/**
 * Writes the data to the next free address in the eve RAM and returns the address where it was written.
 * Calls eve_memory_get_address and eve_memory_register_address internally.
 *
 * @param eve				Pointer to the EVE device
 * @param data				Pointer to the data that should be written into the RAM.
 * @param size				Number of bytes that need to be written into the RAM.
 */
uint32_t eve_memory_write(eve_t* eve, const uint8_t* data, uint32_t size);

/**
 * Writes the data to the address in the eve RAM.
 * Calls eve_memory_register_address internally.
 *
 * @param eve				Pointer to the EVE device
 * @param address			Address where the data should be written to.
 * @param data				Pointer to the data that should be written into the RAM.
 * @param size				Number of bytes that need to be written into the RAM.
 */
void eve_memory_write_to(eve_t* eve, uint32_t address, const uint8_t* data, uint32_t size);

/**
 * Stores the size of RAM for an address.
 *
 * @param eve				Pointer to the EVE device
 * @param address			Address of the RAM that is now in use
 * @param size				Number of bytes written at the address.
 */
void eve_memory_register_address(eve_t* eve, uint32_t address, uint32_t size);

/**
 * Writes the content of a memory file into the eve ram. Handles the address registration internally.
 *
 * @param eve				Pointer to the EVE device
 * @param mfo				Pointer to the memory file object as gotten via eve_memory_register.
 * @return					true when the file was stored in the ram.
 */
bool eve_memory_write_file_to(eve_t* eve, eve_memory_file_t* mfo);

#endif

#endif /* MODULE_GUI_EVE_EVE_MEMORY_H_ */
