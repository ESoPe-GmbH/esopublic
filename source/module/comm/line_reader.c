// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file line_reader.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "module_public.h"
#if MODULE_ENABLE_COMM_LINE_READER

#include "line_reader.h"
#include "mcu/sys.h"
#include "module/convert/string.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define _USE_TASK		LINE_READER_USE_TASK

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if _USE_TASK
/**
 * Protothread for reading a line from an interface
 * @param pt		Protothread structure pointer
 * @return			Protothread return value
 */
static int _handle_read_line(struct pt* pt);
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN line_reader_init(line_reader_t* lr, line_reader_interface_t* interface, char* line_buffer, uint16_t sizeof_line_buffer)
{
	// The pointer shall not be NULL and the buffer length must be above 0!
	if(lr == NULL || interface == NULL || line_buffer == NULL || sizeof_line_buffer == 0)
		return FUNCTION_RETURN_PARAM_ERROR;

	// The interface must have both functions for the line reader to work!
	if(interface->available == NULL || interface->read_char == NULL)
		return FUNCTION_RETURN_PARAM_ERROR;

	lr->interface = interface;
	lr->line = line_buffer;
	lr->line_max = sizeof_line_buffer;
	lr->line_cnt = 0;
	lr->f_hold = NULL;

#if _USE_TASK
	system_remove_task(&lr->task);
	system_task_init_protothread(&lr->task, true, _handle_read_line, lr);
#endif

	line_reader_clear(lr);

	return FUNCTION_RETURN_OK;
}

bool line_reader_ready(line_reader_t* lr)
{
	if(lr == NULL)
		return false;

#if _USE_TASK
	return lr->line_read;
#else

	if(lr->line_read)
		return true;

	if(lr->f_hold && !lr->f_hold(lr))
		return lr->line_read;

	while(lr->interface->available(lr->interface->obj) > 0)
	{
		// Read a byte to the end of the buffer
		lr->line[lr->line_cnt] = lr->interface->read_char(lr->interface->obj);
		// If byte is a carriage return...
		if(lr->line[lr->line_cnt] == 0x0D)
		{
			// Ignore it!
		}
		else if(lr->line[lr->line_cnt] == 0x0A)// If byte is a line feed...
		{
			if(!lr->ignore_empty_lines || (lr->ignore_empty_lines && lr->line_cnt > 0))
			{
				// Set the line as ready for reading!
				lr->line[lr->line_cnt] = 0;
				lr->line_cnt = 0;
				lr->line_read = true;
				return true;
			}
		}
		else
		{
			// If read char is at least a space and the buffer fits at least the terminating zero -> increase line count
			if(lr->line[lr->line_cnt] >= 0x20 && lr->line_cnt < lr->line_max - 1)
				lr->line_cnt++;
		}
	}

	return lr->line_read;
#endif
}

void line_reader_clear(line_reader_t* lr)
{
	if(lr == NULL)
		return;

	lr->line_read = false;
}

int line_reader_receive(struct pt* pt, line_reader_t* lr, uint32_t max_timeout, bool (*f_cancel)(line_reader_t* lr), char** rsp)
{
	PT_BEGIN(pt);

	if(max_timeout > 0)
	{
		PT_YIELD_MS_OR_UNTIL(pt, max_timeout, line_reader_ready(lr) || (f_cancel && f_cancel(lr)));
	}
	else
	{
		PT_YIELD_UNTIL(pt, line_reader_ready(lr) || (f_cancel && f_cancel(lr)));
	}

	if(lr->line_read)
	{
		*rsp = lr->line;
		line_reader_clear(lr);
	}
	else
	{
		*rsp = NULL;
	}

	PT_END(pt);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if _USE_TASK
static int _handle_read_line(struct pt* pt)
{
	line_reader_t* lr = pt->obj;
	PT_BEGIN(pt);

	do
	{
		// Wait until the last line was read and at least one byte is available for reading
		PT_WAIT_UNTIL(pt, ((lr->f_hold && !lr->f_hold(lr)) || !lr->f_hold) && !lr->line_read && lr->interface->available(lr->interface->obj) > 0);
		// Read a byte to the end of the buffer
		lr->line[lr->line_cnt] = lr->interface->read_char(lr->interface->obj);
		// If byte is a carriage return...
		if(lr->line[lr->line_cnt] == 0x0D)
		{
			// Ignore it!
		}
		else if(lr->line[lr->line_cnt] == 0x0A)// If byte is a line feed...
		{
			if(!lr->ignore_empty_lines || (lr->ignore_empty_lines && lr->line_cnt > 0))
			{
				// Set the line as ready for reading!
				lr->line[lr->line_cnt] = 0;
				lr->line_cnt = 0;
				lr->line_read = true;
			}
		}
		else
		{
			// If read char is at least a space and the buffer fits at least the terminating zero -> increase line count
			if(lr->line[lr->line_cnt] >= 0x20 && lr->line_cnt < lr->line_max - 1)
				lr->line_cnt++;
		}

	}while(true);

	PT_END(pt);
}
#endif

#endif
