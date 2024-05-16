// Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file vcomm.c
 * 	@copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_COMM_VCOMM
#include "vcomm.h"
#include "module/fifo/fifo.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Data of the virtual comm interface 
 */
struct vcomm_s
{
    /// Pointer to the initialization data.
    const vcomm_init_t* init;
    /// Comm port to be used with this vcomm interface.
    comm_t comm;
    /// Buffer for the receive fifo.
    uint8_t* buffer_rx;
    /// Receive fifo that is filled by calling `vcomm_input`.
    fifo_t fifo_rx;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Put will write data to the output callback that was set in the initialization structure.
 * 
 * @param obj       Virtual comm handle that was created using `vcomm_create`.
 * @param c         Byte to output.
 */
static void _putc(void* obj, int c);
/**
 * @brief Put will write data to the output callback that was set in the initialization structure.
 * 
 * @param obj       Virtual comm handle that was created using `vcomm_create`.
 * @param buffer    Pointer to the data to output.
 * @param length    Number of bytes to output
 */
static void _put(void* obj, uint8_t* buffer, uint16_t length);
/**
 * @brief Getc will read a byte that was input using `vcomm_input` and now resides inside the fifo.
 * 
 * @param obj       Virtual comm handle that was created using `vcomm_create`.
 * @return          Byte read from fifo.
 */
static int _getc(void* obj);
/**
 * @brief Gets will read bytes that were input using `vcomm_input` and now resides inside the fifo.
 * 
 * @param obj       Virtual comm handle that was created using `vcomm_create`.
 * @param buffer    Buffer to fill with data from fifo.
 * @param length    Maximum number of bytes to read into buffer.
 * @return          Number of bytes written into buffer.
 */
static int _gets(void* obj, uint8_t *buffer, uint16_t length);
/**
 * @brief Get the number of bytes ready to be read.
 * 
 * @param obj       Virtual comm handle that was created using `vcomm_create`.
 * @return          Number of bytes ready to be read.
 */
static int _available(void* obj);
/**
 * @brief Indicates whether the output is ready to handle data.
 * 
 * @param obj       Virtual comm handle that was created using `vcomm_create`.
 * @return true     You can put data into the comm interface.
 * @return false    You should not put data into the comm interface.
 */
static bool _transmit_ready(void* obj);
/**
 * @brief Flushes the output.
 * 
 * @param obj       Virtual comm handle that was created using `vcomm_create`.
 */
static void _flush(void* obj);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Comm interface using the virtual comm.
static const comm_interface_t _comm_interface = {
    .xputc = _putc,
    .xputs = _put,
    .transmit_ready = _transmit_ready,
    .flush = _flush,
    .xgetc = _getc,
    .xgets = _gets,
    .data_present  = _available
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

vcomm_handle_t vcomm_create(const vcomm_init_t* init)
{
    vcomm_handle_t vcomm = mcu_heap_calloc(1, sizeof(struct vcomm_s));

    DBG_ASSERT(vcomm, NO_ACTION, NULL, "Error allocating virtual comm\n");

    vcomm->init = init;

    vcomm->buffer_rx = mcu_heap_calloc(1, init->rx_buffer_size);
    DBG_ASSERT(vcomm, goto error, NULL, "Error allocating rx buffer\n");

    fifo_init(&vcomm->fifo_rx, 1, vcomm->buffer_rx, vcomm->init->rx_buffer_size);
    comm_init_handler(&vcomm->comm);
    vcomm->comm.device_handler = vcomm;
    vcomm->comm.interface = &_comm_interface;

    return vcomm;
error:
    vcomm_free(vcomm);
    return NULL;
}

void vcomm_free(vcomm_handle_t vcomm)
{
    if(vcomm == NULL)
        return;

    if(vcomm->buffer_rx)
    {
        memset(vcomm->buffer_rx, 0, vcomm->init->rx_buffer_size);
        mcu_heap_free(vcomm->buffer_rx);
        vcomm->buffer_rx = NULL;
    }

    if(vcomm)
    {
        memset(vcomm, 0, sizeof(struct vcomm_s));
        mcu_heap_free(vcomm);
    }
}

FUNCTION_RETURN_T vcomm_input(vcomm_handle_t vcomm, uint8_t* buffer, size_t* length)
{
    if(vcomm == NULL || buffer == NULL || length == NULL || *length == 0)
    {
        if(length)
            *length = 0;
        return FUNCTION_RETURN_PARAM_ERROR;
    }

    size_t cnt = 0;

    while(!fifo_is_full(&vcomm->fifo_rx) && cnt < *length)
    {
        fifo_put8(&vcomm->fifo_rx, buffer[cnt++]);
    }
    *length = cnt;

    return FUNCTION_RETURN_OK;
}

comm_t* vcomm_get_comm(vcomm_handle_t vcomm)
{
    if(vcomm == NULL)
        return NULL;

    return &vcomm->comm;
}

void* vcomm_get_user(vcomm_handle_t vcomm)
{
    if(vcomm == NULL)
        return NULL;

    return vcomm->init->user;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _putc(void* obj, int c)
{
    vcomm_handle_t vcomm = obj;

    if(vcomm == NULL)
        return;

    if(vcomm->init->output_cb)
        vcomm->init->output_cb(vcomm, (uint8_t*)&c, 1);
}

static void _put(void* obj, uint8_t* buffer, uint16_t length)
{
    vcomm_handle_t vcomm = obj;

    if(vcomm == NULL)
        return;

    if(vcomm->init->output_cb)
        vcomm->init->output_cb(vcomm, buffer, length);
}

static int _getc(void* obj)
{
    vcomm_handle_t vcomm = obj;

    if(vcomm == NULL)
        return 0;

    return fifo_get8(&vcomm->fifo_rx);
}

static int _gets(void* obj, uint8_t *buffer, uint16_t length)
{
    vcomm_handle_t vcomm = obj;

    if(vcomm == NULL)
        return 0;

    int cnt = 0;

    while(cnt < length && fifo_data_available(&vcomm->fifo_rx))
    {
        buffer[cnt++] = fifo_get8(&vcomm->fifo_rx);
    }

    return cnt;
}

static int _available(void* obj)
{
    vcomm_handle_t vcomm = obj;

    if(vcomm == NULL)
        return 0;

    return fifo_data_available(&vcomm->fifo_rx);
}

static bool _transmit_ready(void* obj)
{
    vcomm_handle_t vcomm = obj;

    if(vcomm == NULL)
        return false;

    if(vcomm->init->output_ready_cb)
        return vcomm->init->output_ready_cb(vcomm);

    return vcomm->init->output_cb != NULL;
}

static void _flush(void* obj)
{
    vcomm_handle_t vcomm = obj;

    if(vcomm == NULL)
        return;

    if(vcomm->init->output_flush_cb)
        vcomm->init->output_flush_cb(vcomm);
}

#endif
