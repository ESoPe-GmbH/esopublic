// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file comm.c
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_COMM
#include "comm.h"
#include "mcu/sys.h"
#include "module/convert/string.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Is set when strings should be printed from the left and are padded with spaces on the right side.
/// Is cleared when strings should be printed to the right and are padded with spaces on the left side.
static bool _set_left_aligned = false;
/// Is used to restrict @see COMM_STRING_LENGTH_EXACT to %s. So %02x etc. are not restricted by this.
static bool _is_printing_string = false;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/// Stores Numbers for conversion. Not static because it can be used in dbg.c
char comm_num_str[COMM_MAX_FORMAT_LENGTH];

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void comm_init_handler(comm_t *h)
{
	if(h == NULL)
		return;

	h->device_handler = NULL;
	h->interface = NULL;
	h->format_len = 0;
	h->len_ascii_str_len = 0;
}

void comm_init_interface(comm_interface_t *h)
{
	if(h == NULL)
		return;

	h->xputc = NULL;
	h->xputs = NULL;
	h->xgetc = NULL;
	h->xgets = NULL;
	h->data_present = NULL;
	h->flush = NULL;
}

void comm_putc(comm_t *h, int letter)
{	
	if(h && h->interface && h->interface->xputc)
		h->interface->xputc(h->device_handler, letter);
}

void comm_put(comm_t *h, uint8_t *buf, uint16_t len)
{
	if(h!=NULL && h->interface && h->interface->xputs && buf!=NULL)
		h->interface->xputs(h->device_handler, buf, len);
}

void comm_puts(comm_t *h, char* str)
{
	if(str == NULL)
		return;

	uint16_t str_len = strlen(str);
	uint16_t space_count = 0;

	if(h->format_len>0)
	{
		if(h->format_len > str_len)
			space_count = h->format_len - str_len;	// Fill with letters
#if COMM_STRING_LENGTH_EXACT
		else if(_is_printing_string)
			str_len = h->format_len;
#endif
	}	

	if(!_set_left_aligned)
	{
		while(space_count>0)
		{
			comm_putc(h, ' ');
			space_count--;
		}
	}

	comm_put(h, (uint8_t*)str, str_len);

	if(_set_left_aligned)
	{
		while(space_count>0)
		{
			comm_putc(h, ' ');
			space_count--;
		}
	}

	_set_left_aligned = false;
}

void comm_printf(comm_t *h, char *str, ...)
{
	va_list vl;	
	va_start(vl, str);
	comm_vprintf(h, str, vl);
	va_end(vl);
}

void comm_vprintf(comm_t *h, char *str, va_list vl)
{
	uint8_t	letter;						// Stores a character. If the character is %, the following format will be checked.
	uint8_t	letter2;					// Stores a character inside the format checking routine.
	bool is_in_fromatted_data = false;	// Set to true while the format is checked.
	bool use_var_len = false;			// Is set when two parameter for one wildcard are used, first one is the number of letters to print, second is the value.
	bool use_prev_len = false;			// Is set when the previously printed parameter is the length value for the current value.
	bool string_left_aligned = true;	// Is cleared with wildcard '.'. Strings will then be right aligned.
	char* tmp_ptr = NULL;				// Is used for storing string pointers temporarily
	int32_t tmp_int32 = 0;				// Is used for storing integers temporarily
	int64_t tmp_int64 = 0;
	void* ptr_param;					// Temporary pointer where the type is not needed

	if(h==NULL || h->interface==NULL || h->interface->xputc==NULL)	
		return;	// Cancel if it cannot be used.

	h->format_len = 0;
	h->len_ascii_str_len = 0;
	h->len_ascii_str[0] = 0;

	while(*str)
	{
		 letter = *str++;
		 if(letter=='%')
		 {
			 is_in_fromatted_data = true;
			 use_var_len = false;
			 use_prev_len = false;
			 do
			 {
				 letter2 = *str++;
				 if(letter2 >= '0' && letter2 <= '9') // If value is an ascii number
				 {
					 // Add it to the number buffer
					 if(h->len_ascii_str_len < (sizeof(h->len_ascii_str) - 1) )
						 h->len_ascii_str[h->len_ascii_str_len++] = letter2;
					 continue;
				 }
				 else if(letter2 == '#')
				 {
					 use_var_len = true;
					 continue;
				 }
				 else if(letter2 == '$')
				 {
					 use_prev_len = true;
					 continue;
				 }
				 else if(letter2 == '.')
				 {
					 string_left_aligned = false;
					 continue;
				 }
				 else if(letter2 == 'l')
				 {
					 // used for lu (long unsigned integer) -> 64 bit -> Not implemented yet
					 continue;
				 }
				 else if(h->len_ascii_str_len > 0)
				 {
					 // Can be replaces by an ascii to int function in the future -> Do not use standard libraries.
					switch(h->len_ascii_str_len)
					{
						case 3:
							h->format_len = (h->len_ascii_str[0]-'0')*100 + (h->len_ascii_str[1]-'0')*10 + h->len_ascii_str[2]-'0';
						break;

						case 2:
							h->format_len = (h->len_ascii_str[0]-'0')*10 + h->len_ascii_str[1]-'0';
						break;

						case 1:
							h->format_len = h->len_ascii_str[0]-'0';
						break;
					}
					h->len_ascii_str_len = 0;
				 }
				 if(use_var_len)
				 {
					h->format_len = va_arg(vl, uint32_t);
					use_var_len = false;
				 }
				 else if(use_prev_len)
				 {
					 h->format_len = tmp_int32;
					 use_prev_len = false;
				 }
				 if(letter2 != 'D' && letter2 != 'T' && letter2 != 'A' && letter2 != 'a' && letter2 != 'Q' && letter2 != 'q')
				 {
					if(h->format_len > COMM_MAX_FORMAT_LENGTH - 1) // Limit!
						h->format_len = COMM_MAX_FORMAT_LENGTH - 1;
				 }
				 switch(letter2)
				 {
					case '%':
						comm_putc(h, '%');
					break;

					case 'c':
						comm_putc(h, va_arg(vl, int));
						is_in_fromatted_data = false;
					break;

					case 'u':
						tmp_int32 = va_arg(vl, uint32_t);
						string_create_uint_string(comm_num_str, (uint32_t)tmp_int32, 10, h->format_len, (h->len_ascii_str[0] == '0'));
						comm_puts(h, comm_num_str);
						is_in_fromatted_data = false;
					break;

                    case 'i':
					case 'd':
						tmp_int32 = va_arg(vl, int32_t);
						string_create_int_string(comm_num_str, tmp_int32, 10, h->format_len, (h->len_ascii_str[0] == '0'));
						comm_puts(h, comm_num_str);
						is_in_fromatted_data = false;
					break;

                    case 'U':
                        tmp_int64 = va_arg(vl, uint64_t);
                        string_create_uint64_string(comm_num_str, (uint64_t)tmp_int64, 10, h->format_len, (h->len_ascii_str[0] == '0'));
                        comm_puts(h, comm_num_str);
                        is_in_fromatted_data = false;
                    break;

                    case 'I':
                        tmp_int64 = va_arg(vl, int64_t);
                        string_create_int64_string(comm_num_str, tmp_int64, 10, h->format_len, (h->len_ascii_str[0] == '0'));
                        comm_puts(h, comm_num_str);
                        is_in_fromatted_data = false;
                    break;

					case 'm':
						tmp_int32 = va_arg(vl, int32_t);
						tmp_ptr = string_create_num_string(comm_num_str, tmp_int32 / 100, true);
						*tmp_ptr++ = string_get_decimal_point_character();
						if(tmp_int32 < 0)
							tmp_int32 *= -1;
						string_create_int_string(tmp_ptr, tmp_int32 % 100, 10, 2, true);
						comm_puts(h, comm_num_str);
						is_in_fromatted_data = false;
					break;
					
					case 'M':
						tmp_int32 = va_arg(vl, int32_t);
						tmp_ptr = string_create_num_string(comm_num_str, tmp_int32 / 100, false);
						*tmp_ptr++ = string_get_decimal_point_character();
						if(tmp_int32 < 0)
							tmp_int32 *= -1;
						string_create_int_string(tmp_ptr, tmp_int32 % 100, 10, 2, true);
						comm_puts(h, comm_num_str);
						is_in_fromatted_data = false;
					break;

					case 'X':
					case 'x':
					case 'h':
						string_set_hex_letter_size(letter2 == 'X');
						tmp_int32 = va_arg(vl, int32_t);
						string_create_uint_string(comm_num_str, tmp_int32, 16, h->format_len, (h->len_ascii_str[0] == '0'));
						comm_puts(h, comm_num_str);
						is_in_fromatted_data = false;
					break;

					case 'a':
					case 'A':
						string_set_hex_letter_size(letter2 == 'A');
						{
							uint8_t* arr = va_arg(vl, uint8_t*);
							uint16_t i;
							for(i = 0; i < h->format_len; i++)
							{
								comm_putc(h, string_uint8_to_ascii((arr[i] >> 4) & 0x0F));
								comm_putc(h, string_uint8_to_ascii((arr[i]     ) & 0x0F));
								if(i < h->format_len - 1)
									comm_putc(h, ' ');
							}
						}
						is_in_fromatted_data = false;
					break;

					case 'q':
					case 'Q':
						string_set_hex_letter_size(letter2 == 'Q');
						{
							uint8_t* arr = va_arg(vl, uint8_t*);
							uint16_t i;
							for(i = 0; i < h->format_len; i++)
							{
								comm_putc(h, string_uint8_to_ascii((arr[i] >> 4) & 0x0F));
								comm_putc(h, string_uint8_to_ascii((arr[i]     ) & 0x0F));
							}
						}
						is_in_fromatted_data = false;
					break;

					case 'b':
						tmp_int32 = va_arg(vl, uint32_t);
						string_create_uint_string(comm_num_str, tmp_int32, 2, h->format_len, (h->len_ascii_str[0] == '0'));
						comm_puts(h, comm_num_str);
						is_in_fromatted_data = false;
					break;

					case 'B':
						tmp_int32 = va_arg(vl, int);
						if(tmp_int32)
							comm_puts(h, "true");
						else
							comm_puts(h, "false");
						is_in_fromatted_data = false;
						break;

					case 's':
						_set_left_aligned = string_left_aligned;
						_is_printing_string = true;
						comm_puts(h, va_arg(vl, char*));
						_is_printing_string = false;
						is_in_fromatted_data = false;
						string_left_aligned = true;
					break;
#if MODULE_ENABLE_RTC
					case 'D':	// Date
						ptr_param = va_arg(vl, rtc_time_t*);
						string_create_date(comm_num_str, ptr_param, h->format_len);
						h->format_len = 0;
						comm_puts(h, comm_num_str);
						is_in_fromatted_data = false;
					break;

					case 'T':	// Time
						ptr_param = va_arg(vl, rtc_time_t*);
						string_create_time(comm_num_str, ptr_param, h->format_len);
						h->format_len = 0;
						comm_puts(h, comm_num_str);
						is_in_fromatted_data = false;
					break;
#endif
					default:
						comm_putc(h, letter);
						comm_putc(h, letter2);
						is_in_fromatted_data = false;
						break;
				 }
				 h->format_len = 0;
			 }while(*str && is_in_fromatted_data);
		 }
		 else
			 comm_putc(h, letter);
	}
	va_end(vl);
}

bool comm_transmit_ready(comm_t* h)
{
    // Invalid pointer -> Not ready
    if(h==NULL || h->interface==NULL)
        return false;

    // The interface has no specific transmit ready function, so every transmission should work.
    if(h->interface->transmit_ready == NULL)
        return true;

    // Call the transmit ready function of the interface to determine if transmission is possible.
    return h->interface->transmit_ready(h->device_handler);
}

void comm_flush(comm_t *h)
{
	if(h==NULL || h->interface==NULL || h->interface->flush==NULL)
		return;

	h->interface->flush(h->device_handler);
}

int	comm_getc(comm_t *h)
{				
	if(h==NULL || h->interface==NULL || h->interface->xgetc==NULL)
		return 0;

	return h->interface->xgetc(h->device_handler);				
}

int	comm_gets(comm_t *h, uint8_t *buf, uint16_t element_cnt)
{
	if(h==NULL || h->interface==NULL || h->interface->xgets==NULL || buf==NULL || element_cnt==0)
		return 0;

	return h->interface->xgets(h->device_handler, buf, element_cnt);
}

int	comm_data_available(comm_t *h)
{			
	if(h==NULL || h->interface==NULL || h->interface->data_present==NULL)
		return 0;

	return h->interface->data_present(h->device_handler);		
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#endif
