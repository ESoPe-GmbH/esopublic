/**
 * @file mem_pool
 * @copyright Urheberrecht 2022-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "module_public.h"
#if MODULE_ENABLE_UTIL_MEM_POOL
#include "mem_pool.h"
#include "module/comm/comm.h"

#if MCU_ENABLE_FREERTOS
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Data for the memory pool
struct mem_pool_s
{
    /// Number of chunks in the pool
    size_t chunks_num;
    /// Buffer size of a single chunk
    size_t chunk_size;
    /// Pointer an array of chunks that is allocated when the pool is created. 
    mem_pool_chunk_t* chunks;

#if MCU_ENABLE_FREERTOS
    /// Semaphore used to synchronize allocations and deallocations.
    SemaphoreHandle_t x_semaphore;
#endif
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
FUNCTION_RETURN mem_pool_init(mem_pool_handle_t* pool, size_t num, size_t chunk_size)
{
    DBG_ASSERT(pool != NULL, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid pool\n");
    DBG_ASSERT(num > 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid num\n");
    DBG_ASSERT(chunk_size > 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid size\n");

    // Try to allocate the complete memory buffer as a single buffer.
    void* buffer = mcu_heap_calloc(num, chunk_size);
    mem_pool_handle_t p = NULL;
    *pool = NULL;

    if(buffer)
    {
        p = mcu_heap_calloc(1, sizeof(struct mem_pool_s));

        if(p)
        {
            // Now pool structure and buffers are allocated -> Allocate chunks
            p->chunks = mcu_heap_calloc(num, sizeof(struct mem_pool_chunk_s));
            p->chunk_size = chunk_size;
            p->chunks_num = num;

            if(p->chunks)
            {
                uintptr_t ptr = (uintptr_t)buffer;
                for(size_t i = 0; i < num; i++)
                {
                    p->chunks[i].buffer = (void*)ptr;
                    p->chunks[i].pool = p;
                    ptr += chunk_size;
                }
                *pool = p;
                
#if MCU_ENABLE_FREERTOS
                // Create a semaphore so mem_pool is task save!
		        p->x_semaphore = xSemaphoreCreateMutex();
                if(p->x_semaphore)
                {
                    // Ready to use!
                    return FUNCTION_RETURN_OK;
                }
                // Failed with the semaphore, free the chunks
                mcu_heap_free(p->chunks);
#else
                // Ready to use!
                return FUNCTION_RETURN_OK;
#endif
            }
            // Failed with the chunks, free the pool
            mcu_heap_free(pool);
        }
        // Failed with pool or chunks, free the buffer
        mcu_heap_free(buffer);
    }

    return FUNCTION_RETURN_INSUFFICIENT_MEMORY;
}

FUNCTION_RETURN mem_pool_free(mem_pool_handle_t pool, bool force)
{
    DBG_ASSERT(pool != NULL, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid pool\n");

#if MCU_ENABLE_FREERTOS
    // Take the semaphore for myself, so when i delete it i can be sure no other task has taken it!
	if(!xSemaphoreTake(pool->x_semaphore, portMAX_DELAY)) // Semaphore is blocked too long...
		return FUNCTION_RETURN_NOT_READY;
#endif
    if(force == false)
    {
        size_t num_used = 0;
        for(size_t i = 0; i < pool->chunks_num; i++)
        {
            switch(pool->chunks[i].usage)
            {
                case MEM_POOL_FREE:
                    // This chunk is not in use, lock it!
                    pool->chunks[i].usage = MEM_POOL_LOCKED;
                    break;
                case MEM_POOL_USED:
                    // Woops, we cannot free the pool! There are chunks in use!
                    num_used++;
                    break;
                case MEM_POOL_LOCKED:
                    // This is fine
                    break;
            }
        }

        if(num_used > 0)
        {
            // As said above: Woops
            return FUNCTION_RETURN_NOT_READY;
        }
    }
    
#if MCU_ENABLE_FREERTOS
    vSemaphoreDelete(pool->x_semaphore);
    pool->x_semaphore = NULL;
#endif

    // Free the buffer of the first chunk, since this is also the start of the buffer over all chunks.
    mcu_heap_free(pool->chunks[0].buffer);
    // Free the chunks
    mcu_heap_free(pool->chunks);
    // Free the pool
    mcu_heap_free(pool);

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN mem_pool_alloc_chunk(mem_pool_handle_t pool, mem_pool_chunk_handle_t* chunk, size_t size)
{
    DBG_ASSERT(chunk != NULL, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid chunk\n");
    *chunk = NULL;
    DBG_ASSERT(pool != NULL, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid pool\n");
    DBG_ASSERT(size > 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid size\n");

#if MCU_ENABLE_FREERTOS
	if(!xSemaphoreTake(pool->x_semaphore, portMAX_DELAY)) // Semaphore is blocked too long...
		return FUNCTION_RETURN_NOT_READY;
#endif

    size_t buffersize_collected = 0;
    // Loop all chunks or until enough buffersize was collected from the chunks
    for(size_t i = 0; i < pool->chunks_num && buffersize_collected < size; i++)
    {
        if(pool->chunks[i].usage == MEM_POOL_FREE)
            buffersize_collected += pool->chunk_size;
    }

    if(buffersize_collected < size)
    {
#if MCU_ENABLE_FREERTOS
	    xSemaphoreGive(pool->x_semaphore);
#endif
        DBG_ERROR("Not enough free chunks\n");
        return FUNCTION_RETURN_INSUFFICIENT_MEMORY;
    }

    buffersize_collected = 0;
    mem_pool_chunk_handle_t chunk_current = NULL;
    // Loop all chunks or until enough buffersize was collected from the chunks
    for(size_t i = 0; i < pool->chunks_num && buffersize_collected < size; i++)
    {
        if(pool->chunks[i].usage == MEM_POOL_FREE)
        {
            // Set chunk to used and clear the chunk from previous stuff
            pool->chunks[i].usage = MEM_POOL_USED;
            pool->chunks[i].next = NULL;
            pool->chunks[i].used_size = 0;
            // Increment collected buffersize
            buffersize_collected += pool->chunk_size;

            // No chunk allocated yet
            if(*chunk == NULL)
            {
                // Set chunk i as first chunk.
                *chunk = &pool->chunks[i];
                chunk_current = *chunk;
                chunk_current->previous = NULL;
            }
            else if(chunk_current)
            {
                // A previous chunk was set -> Set chunk i as the next chunk after previous known chunk.
                chunk_current->next = &pool->chunks[i];
                pool->chunks[i].previous = chunk_current;
                // Set this chunk as the previous known chunk for next iteration
                chunk_current = chunk_current->next;
            }
        }
    }

#if MCU_ENABLE_FREERTOS
    xSemaphoreGive(pool->x_semaphore);
#endif

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN mem_pool_realloc_chunk(mem_pool_chunk_handle_t chunk, size_t size)
{
    DBG_ASSERT(chunk != NULL, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid chunk\n");
    DBG_ASSERT(size > 0, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid size\n");
    mem_pool_handle_t pool = chunk->pool;

#if MCU_ENABLE_FREERTOS
	if(!xSemaphoreTake(pool->x_semaphore, portMAX_DELAY)) // Semaphore is blocked too long...
		return FUNCTION_RETURN_NOT_READY;
#endif

    size_t total_size = mem_pool_get_chunk_total_size(chunk);
    
    if(total_size > size)
    {
        // We can free chunks
        size_t size_not_needed = total_size - size;

        mem_pool_chunk_t* last_chunk = chunk;
        // Skip to last chunk
        while(last_chunk->next)
        {
            last_chunk = last_chunk->next;
        }

        while(size_not_needed >= pool->chunk_size)
        {
            size_not_needed -= pool->chunk_size;
            mem_pool_chunk_t* chunk = last_chunk->previous;
            chunk->next = NULL;
            last_chunk->previous = NULL;
            last_chunk->usage = MEM_POOL_FREE;
            last_chunk->used_size = 0;

            last_chunk = chunk;
        }        
    }
    else if(total_size < size)
    {
        // We need to allocate more chunks! Check fist how much is available
        size_t size_needed = size - total_size;
        size_t buffersize_collected = 0;
        // Loop all chunks or until enough buffersize was collected from the chunks
        for(size_t i = 0; i < pool->chunks_num && buffersize_collected < size_needed; i++)
        {
            if(pool->chunks[i].usage == MEM_POOL_FREE)
                buffersize_collected += pool->chunk_size;
        }

        if(buffersize_collected < size_needed)
        {
#if MCU_ENABLE_FREERTOS
            xSemaphoreGive(pool->x_semaphore);
#endif  
            DBG_ERROR("Not enough space\n");
            return FUNCTION_RETURN_INSUFFICIENT_MEMORY;
        }

        mem_pool_chunk_t* last_chunk = chunk;
        // Skip to last chunk
        while(last_chunk->next)
        {
            last_chunk = last_chunk->next;
        }

        buffersize_collected = 0;
        // Loop all chunks or until enough buffersize was collected from the chunks
        for(size_t i = 0; i < pool->chunks_num && buffersize_collected < size_needed; i++)
        {
            if(pool->chunks[i].usage == MEM_POOL_FREE)
            {
                // Set chunk to used and clear the chunk from previous stuff
                pool->chunks[i].usage = MEM_POOL_USED;
                pool->chunks[i].next = NULL;
                pool->chunks[i].used_size = 0;
                // Increment collected buffersize
                buffersize_collected += pool->chunk_size;

                last_chunk->next = &pool->chunks[i];
                pool->chunks[i].previous = last_chunk;
                last_chunk = &pool->chunks[i];
            }
        }
    }

#if MCU_ENABLE_FREERTOS
    xSemaphoreGive(pool->x_semaphore);
#endif

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN mem_pool_free_chunk(mem_pool_chunk_handle_t chunk)
{
    DBG_ASSERT(chunk != NULL, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Invalid chunk\n");
    mem_pool_handle_t pool = chunk->pool;

#if MCU_ENABLE_FREERTOS
	if(!xSemaphoreTake(pool->x_semaphore, portMAX_DELAY)) // Semaphore is blocked too long...
		return FUNCTION_RETURN_NOT_READY;
#endif
    mem_pool_chunk_t* last_chunk;

    do
    {
        last_chunk = chunk->next;

        chunk->previous = NULL;
        chunk->next = NULL;
        chunk->usage = MEM_POOL_FREE;
        chunk->used_size = 0;

        chunk = last_chunk;

    }while(last_chunk);

#if MCU_ENABLE_FREERTOS
    xSemaphoreGive(pool->x_semaphore);
#endif

    return FUNCTION_RETURN_OK;
}

size_t mem_pool_get_chunk_total_size(mem_pool_chunk_handle_t chunk)
{
    DBG_ASSERT(chunk != NULL, NO_ACTION, 0, "Invalid chunk\n");

    size_t size = 0;
    size_t chunk_size = chunk->pool->chunk_size;
    do
    {
        size += chunk_size;
        chunk = chunk->next;
    }while(chunk);

    return size;
}

size_t mem_pool_get_chunk_free_size(mem_pool_chunk_handle_t chunk)
{
    DBG_ASSERT(chunk != NULL, NO_ACTION, 0, "Invalid chunk\n");

    return mem_pool_get_chunk_total_size(chunk) - mem_pool_get_chunk_used_size(chunk);
}

size_t mem_pool_get_chunk_used_size(mem_pool_chunk_handle_t chunk)
{
    DBG_ASSERT(chunk != NULL, NO_ACTION, 0, "Invalid chunk\n");

    size_t size = 0;
    do
    {
        size += chunk->used_size;
        chunk = chunk->next;
    }while(chunk);

    return size;
}

size_t mem_pool_append_chunk(mem_pool_chunk_handle_t chunk, void* data, size_t size, bool realloc)
{
    DBG_ASSERT(chunk != NULL, NO_ACTION, 0, "Invalid chunk\n");

    size_t chunk_size = chunk->pool->chunk_size;
    size_t written = 0;
    mem_pool_chunk_handle_t first_chunk = chunk;

    // Skip the full chunks
    while(chunk && chunk->used_size == chunk_size)
    {
        if(chunk->next == NULL && realloc)
        {
            // Try to reallocate the size -> If it fails the next chunk will remain NULL and the loop ends.
            mem_pool_realloc_chunk(first_chunk, mem_pool_get_chunk_total_size(first_chunk) + (size - written));
        }

        chunk = chunk->next;
    }

    // fill the chunk
    while(chunk && chunk->used_size < chunk_size && written < size)
    {
        ((uint8_t*)chunk->buffer)[chunk->used_size] = *(uint8_t*)data;
        chunk->used_size++;
        data++;
        written++;
        if(chunk->used_size == chunk_size)
        {
            if(chunk->next == NULL)
            {
                if(realloc)
                {
                    // Try to reallocate the size -> If it fails the next chunk will remain NULL and the loop ends.
                    mem_pool_realloc_chunk(first_chunk, mem_pool_get_chunk_total_size(first_chunk) + (size - written));
                }
            }
            chunk = chunk->next;
        }
    }

    return written;
}

size_t mem_pool_write_chunk(mem_pool_chunk_handle_t chunk, size_t chunk_offset, void* data, size_t size, bool realloc)
{
    DBG_ASSERT(chunk != NULL, NO_ACTION, 0, "Invalid chunk\n");

    size_t chunk_size = chunk->pool->chunk_size;
    size_t written = 0;
    mem_pool_chunk_handle_t first_chunk = chunk;
    size_t offset = 0;
    
    while(chunk && size > 0)
    {
        if(chunk_offset >= chunk_size)
        {
            // Skip this chunk completely
            chunk_offset -= chunk_size;
            // Since data will be put behind this chunk, set the used to max, even if no previous write in this chunk occured
            // Example: First write option is offset 1076 with 3 byte and we have 512 byte chunks. If this would not happen the total use would be 3 and these 3 were somewhere in a later chunk.
            // To prevent above we set this to have a usage of 1079 afterwards.
            chunk->used_size = chunk_size;
        }
        else
        {
            offset = chunk_offset;
            chunk_offset = 0;

            // Well, start in this chunk
            while(size > 0 && offset < chunk_size)
            {                
                // Fill remaining
                ((uint8_t*)chunk->buffer)[offset] = *(uint8_t*)data;
                offset++;
                data++;
                written++;
                size--;
            }
            if(chunk->used_size < offset)
            {
                chunk->used_size = offset;
            }
        }
    
        if(chunk->next == NULL)
        {
            if(realloc)
            {
                // Try to reallocate the size -> If it fails the next chunk will remain NULL and the loop ends.
                mem_pool_realloc_chunk(first_chunk, mem_pool_get_chunk_total_size(first_chunk) + (size - written));
            }
        }

        chunk = chunk->next;
    }

    return written;
}

size_t mem_pool_read_chunk(mem_pool_chunk_handle_t chunk, size_t chunk_offset, void* data, size_t size)
{
    DBG_ASSERT(chunk != NULL, NO_ACTION, 0, "Invalid chunk\n");

    size_t chunk_size = chunk->pool->chunk_size;
    size_t read = 0;
    size_t offset = 0;
    
    while(chunk && size > 0)
    {
        if(chunk_offset >= chunk_size)
        {
            // Skip this chunk completely
            chunk_offset -= chunk_size;
        }
        else
        {
            offset = chunk_offset;
            chunk_offset = 0;

            // Well, start in this chunk
            while(size > 0 && offset < chunk_size)
            {                
                // Fill remaining
                *(uint8_t*)data = ((uint8_t*)chunk->buffer)[offset];
                offset++;
                data++;
                read++;
                size--;
            }
        }

        chunk = chunk->next;
    }

    return read;
}

void mem_pool_print_usage(comm_t* comm, mem_pool_handle_t pool)
{
    if(comm == NULL)
        return;

    if(pool)
    {
        comm_printf(comm, "Number of chunks: %d\n", pool->chunks_num);
        comm_printf(comm, "Size of chunks: %d\n", pool->chunk_size);
        // size_t count = 0;
        // while(chunk)
        for(size_t i = 0; i < pool->chunks_num; i++)
        {
            mem_pool_chunk_handle_t chunk = &pool->chunks[i];
            comm_printf(comm, " %3d [%08x] - usage=%s used=%04d next=%08x previous=%08x\n", i, chunk, chunk->usage == MEM_POOL_FREE ? "free" : chunk->usage == MEM_POOL_LOCKED ? "lock" : "used", chunk->used_size, chunk->next, chunk->previous);
            // chunk = chunk->next;
            // count++;
        }
    }
    else
    {
        comm_puts(comm, "There is no pool\n");
    }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // MODULE_ENABLE_UTIL_MEM_POOL
