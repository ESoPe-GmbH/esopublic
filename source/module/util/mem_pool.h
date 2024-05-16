/**
 * 	@file mem_pool.h
 *  @copyright Urheberrecht 2022-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		The memory pool is used to create and store buffers in smaller chunks that can be used and freed without always allocating and de-allocating memory from the heap.
 *              The advantage using the memory pool is to reduce the fragmentation of the heap by directly allocating a bigger pool and then using these buffers without allocating/deallocating them all the time.
 *              The handling is a bit more complex, because when reading data from the chunks of the pool you have to consider that you may not have everything in a single buffer, but it can be splitted on multiple chunks.
 *			
 *  @version	1.00 (10.05.2022)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_UTIL_MEM_POOL_H_
#define MODULE_UTIL_MEM_POOL_H_

#include "module_public.h"
#if MODULE_ENABLE_UTIL_MEM_POOL
#include "module/enum/function_return.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Memory pool usage enumeration indicates what a frame in the pool is used for.
typedef enum mem_pool_e
{
    /// This chunk is currently not in used and can be allocated.
    MEM_POOL_FREE = 0,
    /// This chunk is currently allocated.
    MEM_POOL_USED,
    /// Status set when mem_pool_free was called and this chunk was unused. This means it should not be used anymore.
    MEM_POOL_LOCKED
}MEM_POOL_T;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Type for a single pool chunk
typedef struct mem_pool_chunk_s mem_pool_chunk_t;
/// Type for a single pool chunk handle
typedef struct mem_pool_chunk_s* mem_pool_chunk_handle_t;
/// Handle for the memory pool that is gotten when the memory pool is created.
typedef struct mem_pool_s* mem_pool_handle_t;

/// A pool of multiple pool chunks can be allocated at once.
/// Using these, the heap will not be fragmented, because the different chunks remain ready to use.
struct mem_pool_chunk_s
{
    /// Indicates the current usage of this chunk
    MEM_POOL_T usage;
    /// Buffer of this chunk
    void* buffer;
    /// Used of data in this chunk.
    size_t used_size;
    /// Handle of the pool it belongs to.
    mem_pool_handle_t pool;
    /// If total buffer size needs more than one chunk, `next` can be set to point to the next buffer. Is NULL if this is the last allocated chunk.
    mem_pool_chunk_handle_t next;
    /// Points to the chunk before this chunk. Is NULL on first chunk.
    mem_pool_chunk_handle_t previous;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes a pool of memory by allocating space for the pool based on the given chunk size and number.
 * 
 * @param pool          Pointer to the handle of a pool that will be set on success or to NULL when the initialization fails.
 * @param num           Number of chunks to use in the pool. Must not be 0.
 * @param chunk_size    Maximum size of a single chunk. Must not be 0.
 * @return              Handle for the pool or NULL if there was a problem allocating the pool. 
 */
FUNCTION_RETURN mem_pool_init(mem_pool_handle_t* pool, size_t num, size_t chunk_size);
/**
 * @brief Frees a pool of memory by deallocating all of it chunks.
 *          Set the force flag to deallocate all chunks, even when they are still in use. Only use it if you are sure noone would access them.
 * 
 * @param pool          Handle for the pool as initialized with mem_pool_init.
 * @param force         If set: Deallocates all chunks, regardless of their usage.
 *                      If cleared: Only deallocate all chunks that are currently unused.
 * @retval FUNCTION_RETURN_OK           The pool is completely deallocated, so you have to call mem_pool_init again.
 * @retval FUNCTION_RETURN_PARAM_ERROR  The pool is invalid.
 * @retval FUNCTION_RETURN_NOT_READY    All unused chunks were deallocated, but some chunks are still allocated. Does not happen on force = true
 */
FUNCTION_RETURN mem_pool_free(mem_pool_handle_t pool, bool force);
/**
 * @brief Allocate space based on the given size.
 * Tries to allocate space based on the given size, but fails if there are not enough free chunks for this.
 * 
 * @param pool          Handle for the pool.
 * @param chunk         Pointer to the handle of a chunk that will be set on success. Will be set to NULL if failed.
 * @param size          Total space needed. Must not be 0.
 * @retval FUNCTION_RETURN_OK                   The chunk(s) were successfully allocated and handle now contains the handle for the chunk.
 * @retval FUNCTION_RETURN_PARAM_ERROR          Invalid parameters
 * @retval FUNCTION_RETURN_INSUFFICIENT_MEMORY  There is not enough free chunks to satisfy the requested size.
 */
FUNCTION_RETURN mem_pool_alloc_chunk(mem_pool_handle_t pool, mem_pool_chunk_handle_t* chunk, size_t size);
/**
 * @brief Re-allocate space based on the new given size.
 * This can be used to make an existing chunk bigger or to free data if the total buffer size does not need to be as big as before. Cannot be used to free a chunk.
 * 
 * @param chunk         Handle for the chunk that was allocated by mem_pool_alloc_chunk.
 * @param size          Total space needed. Must not be 0.
 * @retval FUNCTION_RETURN_OK                   The chunk(s) were successfully allocated or size of the buffer was reduced successfully.
 * @retval FUNCTION_RETURN_PARAM_ERROR          Invalid parameters
 * @retval FUNCTION_RETURN_INSUFFICIENT_MEMORY  There is not enough free chunks to satisfy the requested size.
 */
FUNCTION_RETURN mem_pool_realloc_chunk(mem_pool_chunk_handle_t chunk, size_t size);
/**
 * @brief Frees a complete chunk and sets them all to unused. 
 * 
 * @param chunk         Handle for the chunk that was allocated by mem_pool_alloc_chunk.
 * @return FUNCTION_RETURN_OK                   The chunk(s) were successfully freed.
 */
FUNCTION_RETURN mem_pool_free_chunk(mem_pool_chunk_handle_t chunk);
/**
 * @brief Returns the total buffer size of all chunks belonging to the allocated handle.
 * 
 * @param chunk         Handle for the chunk that was allocated by mem_pool_alloc_chunk.
 * @return size_t       Number of bytes totally usable in this chunks belonging to the handle.
 */
size_t mem_pool_get_chunk_total_size(mem_pool_chunk_handle_t chunk);
/**
 * @brief Returns the number of unused bytes of all chunks belonging to the allocated handle.
 * 
 * @param chunk         Handle for the chunk that was allocated by mem_pool_alloc_chunk.
 * @return size_t       Number of unused bytes in the chunks belonging to the handle.
 */
size_t mem_pool_get_chunk_free_size(mem_pool_chunk_handle_t chunk);
/**
 * @brief Returns the number of used bytes of all chunks belonging to the allocated handle.
 * 
 * @param chunk         Handle for the chunk that was allocated by mem_pool_alloc_chunk.
 * @return size_t       Number of used bytes in the chunks belonging to the handle.
 */
size_t mem_pool_get_chunk_used_size(mem_pool_chunk_handle_t chunk);
/**
 * @brief Append data to this chunk.
 * Puts data at the end of the current chunk.
 * 
 * @param chunk         Handle for the chunk that was allocated by mem_pool_alloc_chunk.
 * @param data          Pointer to the data that should be written into the chunk.
 * @param size          Number of bytes to write into the chunk.
 * @param realloc       If true: Automatically reallocates the pool to a bigger size, if not enough space is available.
 *                      If false: Stops writing the data at the end of the last chunk.
 * @return size_t       Number of bytes written into the chunk.
 */
size_t mem_pool_append_chunk(mem_pool_chunk_handle_t chunk, void* data, size_t size, bool realloc);
/**
 * @brief Write data to this chunk. 
 * Puts data into the buffer the chunks belonging to this pool.
 * 
 * @param chunk         Handle for the chunk that was allocated by mem_pool_alloc_chunk.
 * @param chunk_offset  Offset inside the chunk as to where to write the data.
 * @param data          Pointer to the data that should be written into the chunk.
 * @param size          Number of bytes to write into the chunk.
 * @param realloc       If true: Automatically reallocates the pool to a bigger size, if not enough space is available.
 *                      If false: Stops writing the data at the end of the last chunk.
 * @return size_t       Number of bytes written into the chunk.
 */
size_t mem_pool_write_chunk(mem_pool_chunk_handle_t chunk, size_t chunk_offset, void* data, size_t size, bool realloc);
/**
 * @brief Read data from this chunk.
 * Reads data from the chunk and puts them into the data buffer.
 * 
 * @param chunk         Handle for the chunk that was allocated by mem_pool_alloc_chunk.
 * @param chunk_offset  Offset inside the chunk as to where to start reading the data.
 * @param data          Pointer to the buffer the read data should be written into.
 * @param size          Number of bytes to read.
 * @return size_t       Number of bytes that were read from the chunk.
 */
size_t mem_pool_read_chunk(mem_pool_chunk_handle_t chunk, size_t chunk_offset, void* data, size_t size);
/**
 * @brief Diagnostic function to print the usage of the memory pool on a comm interface.
 * 
 * @param comm          Pointer of the comm interface to print on.
 * @param pool          Handle for the pool.
 */
void mem_pool_print_usage(comm_t* comm, mem_pool_handle_t pool);

#endif // MODULE_ENABLE_UTIL_MEM_POOL
#endif // MODULE_UTIL_MEM_POOL_H_
