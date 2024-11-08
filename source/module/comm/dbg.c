// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/***
 * @file dbg.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/


#include "module_public.h"
#if MODULE_ENABLE_COMM

#include "dbg.h"
#include "mcu/sys.h"
#include "module/fifo/fifo.h"
#include <string.h>

#if DBG_USE_MMC_LOG
	#include "module/mmc/mmc.h"
	#include "module/rtc/rtc.h"

	#if !STRING_BUILD_PRINTF
		#error "The debug logfile needs the string_printf function! Set STRING_BUILD_PRINTF to true"
	#endif

	#if !FF_USE_LFN
		#error "The debug logfile needs long filenames!"
	#endif
#endif

#if MCU_ENABLE_FREERTOS
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#endif

#if DBG_USE_TCP
#include "module/network/network_interface.h"
#include "module/network/socket.h"
#if DBG_USE_TCP_CONSOLE
#include "module/console/console.h"
#endif
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if !MODULE_ENABLE_NETWORK
#undef DBG_USE_TCP
#define DBG_USE_TCP									0
#endif

/// Set to true if more info should be printed
#define _DEBUG_SOCKETS    false

#if _DEBUG_SOCKETS
    /// Debug macro to print all received bytes
    #define _DBG_SOCKET         DBG_VERBOSE
#else
    /// Debug macro that prints nothing
    #define _DBG_SOCKET         DBG_NONE
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if DBG_USE_TCP
/**
 * Handles incoming TCP connections and responds to requests.
 * 
 * @param pt 		Protothread pointer
 * @return int 		Protothread return value
 */
static int _pt_tcp_server(struct pt* pt);
/**
 * Handles the connection with a connected client and is responsible for sending and receiving.
 * 
 * @param pt 		Protothread pointer
 * @return int 		Protothread return value
 */
static int _pt_tcp_client(struct pt* pt);
/**
 * @brief Put a single character into the send buffer.
 * 
 * @param obj		Unused pointer
 * @param c			Character to send.
 */
static void _tcp_putc(void* obj, int c);
/**
 * @brief Puts data in the buffer to send.
 * @param obj		Unused pointer
 * @param buf		Pointer to the buffer from which to read from.
 * @param len		Number of bytes in the buffer.
 */
static void _tcp_puts(void* obj, uint8_t* buf, uint16_t len);
/**
 * @brief Reads a single character from the received tcp data.
 * 
 * @param obj		Unused pointer
 * @return int 		Character that was received.
 */
static int _tcp_getc(void* obj);
/**
 * @brief Read data from the received tcp data into a buffer.
 * 
 * @param obj		Unused pointer
 * @param buf		Buffer where the received data should be written to.
 * @param len		Maximum number of bytes that can be read.
 * @return int 		Number of bytes that were written into the buffer.
 */
static int _tcp_gets(void* obj, uint8_t* buf, uint16_t len);
/**
 * @brief Return the number of bytes that were received.
 * 
 * @param obj		Unused pointer
 * @return int 		Number of bytes that were received.
 */
static int _tcp_data_present(void* obj);
/**
 * @brief Indicates whether data can be sent or not.
 * 
 * @param obj		Unused pointer
 * @return true 	Data can be sent.
 * @return false 	Buffer is full and data cannot be sent.
 */
static bool _tcp_transmit_ready(void* obj);

#endif

#if DBG_USE_MMC_LOG
/**
 * Closes the log file after some time of inactivity.
 */
static void dbg_handle(void);
/**
 * Writes a byte inside the log file and to the sio
 * @param v		Pointer to the comm_t pointer
 * @param b		Value to write.
 */
static void dbg_log_putc(void *v, int b);
/**
 * Writes a buffer inside the log file and to the sio
 * @param v		Pointer to the comm_t pointer
 * @param c		Buffer to write
 * @param len	Length of the buffer
 */
static void dbg_log_put(void *v, uint8_t *c, uint16_t len);
/**
 * Flushes data to the mmc that are temporarily buffered.
 * @param v		Pointer to the comm_t pointer
 */
static void dbg_log_flush(void *v);
/**
 * Generates the log filename and opens the file.
 */
static void dbg_log_open_file(void);
/**
 * Closes the currently open log file.
 */
static void dbg_log_close_file(void);

#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Pointer to the debug device. Not static because it can be used externally via COMM_DEBUG.
comm_t* comm_debug = NULL;

#if DBG_SYS_MS_COUNT_LETTERS > 0
/// Buffer for storing the milliseconds that are printed before a debug string
static char _str_milliseconds[DBG_SYS_MS_COUNT_LETTERS + 2];
#endif

#if MCU_ENABLE_FREERTOS
/// Semaphore used to synchronize debug calls.
static SemaphoreHandle_t _xSemaphore = NULL;

#if DBG_USE_TCP
/// Is used to check in lower functions to see if the dbgprint is entered. If it is not send, lower functions need to synchronize.
static bool _in_dbgprint = false;
#endif
#endif

#if DBG_USE_TCP
/// Configuration of the tcp configuration interface.
static dbg_tcp_config_t _tcp_config = DBG_DEFAULT_TCP_CONFIG();
/// Default debug port that is set with dbg_set_comm. Is used again when a client is disconnected.
static comm_t* _default_dbg_comm = NULL;
/// Task for handling the tcp server socket.
static system_task_t _task_tcp_server;
/// Task for handling the tcp client socket.
static system_task_t _task_tcp_client;
/// Socket used for communicating with the client.
static socket_t _socket = SOCKET_ERROR;
/// Server socket used for listening.
static socket_t _socket_server = SOCKET_ERROR;
/// Pointer to a dynamically allocated buffer for data to send on the tcp interface through the socket
static uint8_t* _buffer_tcp_send;
/// Contains the number of bytes of the send buffer that were already sent.
static size_t _buffer_tcp_send_cnt = 0;
/// Contains the number of bytes in the send buffer that can be sent.
static size_t _buffer_tcp_send_len = 0;
/// Pointer to a dynamically allocated buffer for receiving data from the fifo before putting it into the fifo.
static uint8_t* _buffer_tcp_receive_socket;
/// Pointer to a dynamically allocated buffer for data received via tcp interface.
static uint8_t* _buffer_tcp_receive_fifo;
/// Fifo used for receiving data.
static fifo_t _fifo_tcp_receive;
/// Is set to true in init and only set to false on stop function
static bool _shall_run = false;
/// Is set to true when a new client connects so the server closes the current client
static bool _stop_client = false;
/// Comm interface for the tcp debugging.
static comm_interface_t _comm_interface_tcp = 
{
	.xputc = _tcp_putc,
	.xputs = _tcp_puts,
	.xgetc = _tcp_getc,
	.xgets = _tcp_gets,
	.data_present = _tcp_data_present,
	.transmit_ready = _tcp_transmit_ready
};
/// Comm handler for the tcp debugging.
static comm_t _comm_tcp = 
{
	.interface = &_comm_interface_tcp
};
#if DBG_USE_TCP_CONSOLE
/// Console for debugging via tcp.
static console_data_t _console;
#endif

#endif

#if DBG_USE_MMC_LOG

	/// File object used for writing and reading a file in one-time read/write operations. For example used for writing structs into a file.
	static FIL dbg_log_file_obj;

	/// Comm interface object used for the log file.
	static comm_interface_t dbg_log_comm_interface;

	/// Comm Handler object used for the log file.
	static comm_t dbg_log_comm_handler;

	/// Indicates if the printer file is opened for reading.
	static bool dbg_log_file_opened = false;

	/// Timestamp for closing the dev log file. Is set when file is opened and written to determine minimum opening time.
	static uint32_t dbg_log_timestamp = 0;

	/// Timestamp for closing the dev log file. Is set when file is opened to determine maximum opening time.
	static uint32_t dbg_log_timestamp_opened = 0;

	/// Buffer for the filename.
	static char dbg_log_filename[50];

	/// Timestamp of the starting time. Is used to determine if the date changes.
	static rtc_time_t dbg_log_startup_time;

	/// Timestamp used to check the current date.
	static rtc_time_t dbg_log_time;

	/// Pointer to the comm handler of the serial interface that is in use.
	static comm_t* dbg_log_comm_handler_sio;

	/// Task Object for handling of the debug log.
	static system_task_t dbg_log_task;

	/// Is set to true when the filename needs to be generated.
	static bool dbg_log_generate_filename = true;

	/// Is set to true when a filename was changed. Is used to mark all files that not start with the system are markes
	/// with a C.
	static bool dbg_log_filename_cont = false;

#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if DBG_USE_MMC_LOG
void dbg_init_logfile(void)
{
	comm_init_interface(&dbg_log_comm_interface);
	dbg_log_comm_interface.xputc = dbg_log_putc;
	dbg_log_comm_interface.xputs = dbg_log_put;
	dbg_log_comm_interface.flush = dbg_log_flush;
	dbg_log_comm_handler.interface = &dbg_log_comm_interface;
	dbg_log_comm_handler.device_handler = NULL;

	mmc_mkdir(DBG_LOG_DIRECTORY, false); 	// Create not hidden log directory
	rtc_get_time(&dbg_log_startup_time);
	dbg_log_comm_handler_sio = comm_debug;
	comm_debug = &dbg_log_comm_handler;

	system_init_void_task(&dbg_log_task, true, dbg_handle);
}
#endif

#if DBG_USE_TCP
FUNCTION_RETURN dbg_init_tcp(const dbg_tcp_config_t* config)
{
	if(config == NULL)
		return FUNCTION_RETURN_PARAM_ERROR;

	memcpy(&_tcp_config, config, sizeof(dbg_tcp_config_t));

	_buffer_tcp_send = mcu_heap_calloc(1, config->buffer_tx);
	_buffer_tcp_receive_fifo = mcu_heap_calloc(1, config->buffer_rx_fifo);
	_buffer_tcp_receive_socket = mcu_heap_calloc(1, config->buffer_rx_socket);

	if(_buffer_tcp_send == NULL || _buffer_tcp_receive_fifo == NULL || _buffer_tcp_receive_socket == NULL)
	{
		DBG_ERROR("Cannot enable tcp debugging\n");

		if(_buffer_tcp_send)
			mcu_heap_free(_buffer_tcp_send);

		if(_buffer_tcp_receive_fifo)
			mcu_heap_free(_buffer_tcp_receive_fifo);

		if(_buffer_tcp_receive_socket)
			mcu_heap_free(_buffer_tcp_receive_socket);

		return FUNCTION_RETURN_INSUFFICIENT_MEMORY;
	}

	fifo_init(&_fifo_tcp_receive, 1, _buffer_tcp_receive_fifo, config->buffer_rx_fifo);

#if DBG_USE_TCP_CONSOLE
	_console.task.name = "Debug TCP";
	console_init(&_console, &_comm_tcp);
#endif

	_shall_run = true;
	system_task_init_protothread(&_task_tcp_server, true, _pt_tcp_server, NULL);
	system_task_init_protothread(&_task_tcp_client, false, _pt_tcp_client, NULL);
	return FUNCTION_RETURN_OK;
}
#endif

void dbg_set_comm(comm_t *h)
{
#if MCU_ENABLE_FREERTOS
	if(_xSemaphore == NULL)
		_xSemaphore = xSemaphoreCreateMutex();
#endif
#if DBG_USE_MMC_LOG
	if(h != &dbg_log_comm_handler)
	{
		if(comm_debug == NULL)
			comm_debug = h;
		else
			dbg_log_comm_handler_sio = h;
	}
#elif DBG_USE_TCP
	// Since comm_debug is under the tcp control, store the comm into a different variable.
	_default_dbg_comm = h;
	// Only set the new comm port if comm_debug is not currently the tcp connection.
	if(comm_debug != &_comm_tcp)
		comm_debug = h;
#else
	comm_debug = h;
#endif
}

void dbg_printf(const char *dbg_string1, const char *dbg_string2, const char *str, ...)
{
	va_list vl;
	va_start(vl, str);
	dbg_vprintf(dbg_string1, dbg_string2, str, vl);
	va_end(vl);
}

void dbg_vprintf(const char *dbg_string1, const char *dbg_string2, const char *str, va_list vl)
{
#if _DBG_STRING_HIDE_PATH || _DBG_STRING_MIN_LEN
	uint16_t i = 0, len = 0;
#endif

#if MCU_ENABLE_FREERTOS
	if(_xSemaphore == NULL)
		return;
#endif

	if(comm_debug==NULL)
		return;

#if MCU_ENABLE_FREERTOS
	if(!xSemaphoreTake(_xSemaphore, portMAX_DELAY)) // Semaphore is blocked too long...
		return;
#if DBG_USE_TCP
	_in_dbgprint = true;
#endif
#endif

#if DBG_SYS_MS_COUNT_LETTERS > 0
	string_create_uint_string(_str_milliseconds, system_get_tick_count(), 10, DBG_SYS_MS_COUNT_LETTERS, true);
	comm_puts(comm_debug, _str_milliseconds);
	comm_puts(comm_debug, ": ");
#endif

#if _DBG_STRING_HIDE_PATH || _DBG_STRING_MIN_LEN
	len = strlen(dbg_string1) + strlen(dbg_string2);
	i = len;

#if _DBG_STRING_HIDE_PATH

	if(len > 0)
	{
		for(i = len - 1; i > 0; i--)
		{
			if(dbg_string1[i] == '/')
			{
				comm_puts(comm_debug, (char*)&dbg_string1[i + 1]);
				break;
			}
		}
	}
	if(i == 0)
		comm_puts(comm_debug, (char*)dbg_string1);

#else // _DBG_STRING_HIDE_PATH
	comm_puts(comm_debug, (char*)dbg_string1);
#endif // else _DBG_STRING_HIDE_PATH
	comm_puts(comm_debug, ", ");
	comm_puts(comm_debug, (char*)dbg_string2);
	comm_puts(comm_debug, ": ");

#if _DBG_STRING_MIN_LEN
	if(len > i)
		len = (len - i);

	if(len < _DBG_STRING_MIN_LEN)
	{
		i = _DBG_STRING_MIN_LEN - len;
		while(i > 0)
		{
			comm_putc(comm_debug, ' ');
			i--;
		}
	}
#endif // _DBG_STRING_MIN_LEN

#endif // _DBG_STRING_HIDE_PATH || _DBG_STRING_MIN_LEN

	comm_vprintf(comm_debug, str, vl);
	va_end(vl);
	comm_flush(comm_debug);
#if MCU_TYPE == PC_EMU
	fflush(stdout);
#endif

#if MCU_ENABLE_FREERTOS
#if DBG_USE_TCP
	_in_dbgprint = false;
#endif
	xSemaphoreGive(_xSemaphore);
#endif
}

#if DBG_USE_MMC_LOG
char* dbg_get_curr_filename(void)
{
	return dbg_log_filename;
}

void dbg_new_file(void)
{
	if(dbg_log_file_opened)
	{
		mmc_close_file(&dbg_log_file_obj);
		dbg_log_file_opened = false;
	}
	rtc_get_time(&dbg_log_startup_time);
	dbg_log_generate_filename = true;
	dbg_log_filename_cont = true;
}
#endif
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if DBG_USE_TCP
static int _pt_tcp_server(struct pt* pt)
{
	SOCKET_STATE ss = socket_get_state(_socket_server);
	
	PT_BEGIN(pt);

	do
	{
		if(_tcp_config.nwk == NULL)
			_tcp_config.nwk = network_interface_get_default();

		_socket_server = socket_open(_tcp_config.nwk, SOCKET_PROTOCOL_TCP, _tcp_config.port);
		if(_socket_server)
		{
			_DBG_SOCKET("Socket opened on %d\n", _tcp_config.port);
			PT_YIELD_UNTIL(pt, ss != SOCKET_STATE_BUSY);
			_DBG_SOCKET("Socket ready %d\n", ss);
			if(ss == SOCKET_STATE_INIT)
			{
				if(FUNCTION_RETURN_OK == socket_listen(_socket_server))
				{
					_DBG_SOCKET("Socket listen\n");
					do
					{
						PT_YIELD_UNTIL(pt, ss != SOCKET_STATE_BUSY);

						if(ss == SOCKET_STATE_ESTABLISHED)
						{
							_DBG_SOCKET("Socket connected\n");

							if(system_task_is_active(&_task_tcp_client))
							{
								_DBG_SOCKET("Stop previous client!\n");
								_stop_client = true;
								PT_YIELD_UNTIL(pt, !system_task_is_active(&_task_tcp_client));
								_stop_client = false;
								_DBG_SOCKET("Previous client stopped!\n");
							}

							_socket = socket_accept(_socket_server);

							if(_socket != SOCKET_ERROR)
							{
								system_add_task(&_task_tcp_client);
							}
							_DBG_SOCKET("Wait for next client... %d\n", socket_get_state(_socket_server));
						}
					}while(ss == SOCKET_STATE_ESTABLISHED
							|| ss == SOCKET_STATE_LISTEN
							|| ss == SOCKET_STATE_BUSY);
				}
				else
				{
					DBG_ERROR("Socket not listening\n");
					PT_YIELD_MS(pt, 100);
				}
			}
			DBG_ERROR("Socket closed? %d\n", ss);
			socket_close(_socket_server);
		}
		PT_YIELD_MS(pt, 100);
	}while(_shall_run);

	PT_END(pt);
}

static int _pt_tcp_client(struct pt* pt)
{
	SOCKET_STATE s = socket_get_state(_socket);
	PT_BEGIN(pt);

	PT_YIELD_UNTIL(pt, s != SOCKET_STATE_BUSY);

	pt->timestamp = system_get_tick_count();

	comm_debug = &_comm_tcp;

	while((s == SOCKET_STATE_ESTABLISHED || s == SOCKET_STATE_BUSY) && _shall_run && !_stop_client)
	{
		if(!fifo_is_full(&_fifo_tcp_receive))
		{
			int len = socket_recv(_socket, _buffer_tcp_receive_socket, _tcp_config.buffer_rx_fifo);

			if(len > 0)
			{
				for(int i = 0; i < len && !fifo_is_full(&_fifo_tcp_receive); i++)
					fifo_put8(&_fifo_tcp_receive, _buffer_tcp_receive_socket[i]);
			}
		}

		if(_buffer_tcp_send_len > 0)
		{
			int len = socket_send(_socket, _buffer_tcp_send, _buffer_tcp_send_len - _buffer_tcp_send_cnt);

			if(len > 0)
			{
#if MCU_ENABLE_FREERTOS
				xSemaphoreTake(_xSemaphore, portMAX_DELAY);
#endif
				// When all data of the buffer were sent, clear the counter so buffer is free again.
				_buffer_tcp_send_cnt += len;
				if(_buffer_tcp_send_cnt >= _buffer_tcp_send_len)
				{
					_buffer_tcp_send_cnt = 0;
					_buffer_tcp_send_len = 0;
				}
#if MCU_ENABLE_FREERTOS
				xSemaphoreGive(_xSemaphore);
#endif
			}
		}

		PT_YIELD(pt);
	}

	comm_debug = _default_dbg_comm;

	DBG_ERROR("Socket disconnected %d %d %d\n", s, _shall_run, _stop_client);
	socket_close(_socket);

	PT_END(pt);
}

static void _tcp_putc(void* obj, int c)
{
	_tcp_puts(obj, (uint8_t*)&c, 1);
}

static void _tcp_puts(void* obj, uint8_t* buf, uint16_t len)
{
#if MCU_ENABLE_FREERTOS
	if(!_in_dbgprint)
		xSemaphoreTake(_xSemaphore, portMAX_DELAY);
#endif
	int available = _tcp_config.buffer_tx - _buffer_tcp_send_len;
	if(available < len)
		len = available;

	if(len > 0)
	{
		memcpy(&_buffer_tcp_send[_buffer_tcp_send_len], buf, len);
		_buffer_tcp_send_len += len;
	}
#if MCU_ENABLE_FREERTOS
	if(!_in_dbgprint)
		xSemaphoreGive(_xSemaphore);
#endif
}

static int _tcp_getc(void* obj)
{
	return fifo_get8(&_fifo_tcp_receive);
}

static int _tcp_gets(void* obj, uint8_t* buf, uint16_t len)
{
	uint16_t r = 0;
	while(fifo_data_available(&_fifo_tcp_receive) && r < len)
	{
		buf[r++] = fifo_get8(&_fifo_tcp_receive);
	}
	return r;
}

static int _tcp_data_present(void* obj)
{
	return fifo_data_available(&_fifo_tcp_receive);
}

static bool _tcp_transmit_ready(void* obj)
{
	return _buffer_tcp_send_len < _tcp_config.buffer_tx;
}

#endif

#if DBG_USE_MMC_LOG
static void dbg_handle(void)
{
	if(!dbg_log_file_opened) // After this, handle log file, so do nothing if it is not opened.
		return;

	// Check for inactivity to close the log file
	if( (system_get_tick_count() - dbg_log_timestamp) >= DBG_LOG_CLOSE_INTERVAL_MS
			|| (system_get_tick_count() - dbg_log_timestamp_opened) >= DBG_LOG_CLOSE_MAX_INTERVAL_MS)
	{
		dbg_log_close_file();
	}
}


static void dbg_log_putc(void *v, int b)
{
	if(!dbg_log_file_opened)
		dbg_log_open_file();

	if(dbg_log_file_opened)
		mmc_putc(&dbg_log_file_obj, (uint8_t)b);

#if DBG_LOG_IS_ADDITION
	comm_putc(dbg_log_comm_handler_sio, b);
#endif
}

static void dbg_log_put(void *v, uint8_t *c, uint16_t len)
{
	if(!dbg_log_file_opened)
		dbg_log_open_file();

	if(dbg_log_file_opened)
		mmc_puts(&dbg_log_file_obj, c, len);

#if DBG_LOG_IS_ADDITION
	comm_put(dbg_log_comm_handler_sio, c, len);
#endif
}

static void dbg_log_flush(void *v)
{
	mmc_flush(&dbg_log_file_obj);
}

static void dbg_log_open_file(void)
{
	if(dbg_log_file_opened)
		dbg_log_close_file();

	rtc_get_time(&dbg_log_time);
	if(dbg_log_time.date != dbg_log_startup_time.date)
	{
		dbg_log_startup_time.hour = 0;
		dbg_log_startup_time.min = 0;
		dbg_log_startup_time.sec = 0;
		dbg_log_filename_cont = true;
		dbg_log_generate_filename = true; // New filename needs to be generated!
	}

	if(dbg_log_generate_filename)
	{
		// Create year-month directory in log folder
		string_printf(dbg_log_filename, DBG_LOG_DIRECTORY "/%04d-%02d", dbg_log_time.year, dbg_log_time.month);
		mmc_mkdir(dbg_log_filename, false);

		// Create filename for day in month folder
		string_printf(dbg_log_filename + strlen(dbg_log_filename), "/%02d[%02d-%02d-%02d]%s.log",
															dbg_log_time.date,
															dbg_log_startup_time.hour,
															dbg_log_startup_time.min,
															dbg_log_startup_time.sec,
															dbg_log_filename_cont ? " C" : "");
	}
	dbg_log_generate_filename = false; // Filename was generated, clear flag

	if(FR_OK == mmc_open_file(dbg_log_filename, &dbg_log_file_obj, OPEN_CONTINUE))
	{
		dbg_log_timestamp = system_get_tick_count();
		dbg_log_timestamp_opened = system_get_tick_count();
		dbg_log_file_opened = true;
	}
}

static void dbg_log_close_file(void)
{
	if(dbg_log_file_opened)
	{
		mmc_close_file(&dbg_log_file_obj);
		dbg_log_file_opened = false;
	}
}
#endif

#endif
