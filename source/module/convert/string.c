// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * @file string.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */
 

#include "module_public.h"
#if MODULE_ENABLE_CONVERT_STRING
#include "module/comm/comm.h"
#include "string.h"

#if MODULE_ENABLE_RTC
#include "module/rtc/rtc.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Set to 'A' if an hex upper letter is used for conversion.
/// Set to 'a' if an hex lower letter is used for conversion.
static char string_hex_char = 'A';

static char string_thousand_separator = '.';

static char string_decimal_point = ',';

#if MODULE_ENABLE_RTC
static const char* string_format_time_order = "hms";

static const char* string_format_date_order = "dmy";

static const char* string_format_date_seperator = ".";
#endif

#if STRING_BUILD_PRINTF

static bool string_printf_initialized = false;

static comm_interface_t string_printf_comm_interface;

static comm_t string_printf_comm_handler;

static int16_t string_printf_char_count = 0;

static int16_t string_printf_char_max = -1;

#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if STRING_BUILD_PRINTF

static void string_printf_putc(char* str, int ch);

static void string_printf_puts(char* str, char* str2, uint16_t len);

#endif

/**
 *  Creates an integer string from an an unsigned integer 32-bit value.
 *
 * @param str                   Pointer to a buffer where the string will be written to.
 * @param val                   32-Bit integer that needs to be converted to a string.
 * @param base                  Base of the number. Currently supported are 10 for decimal and 16 for hex.
 * @param min_letters           Minimum number of characters that must be shown. If min_letters is 0, the full number will be shown.
 * @param add_leading_zero      true: Leading 0 will be added if the number has less characters then min_letters.
 *                              false: Spaces will be added if the number has less characters then min_letters.
 * @param add_minus             true: A minus will be added right before the num string.
 *                              false: No minus will be added.
 * @return                      Pointer to the address of the terminating zero.
 **/
static char* string_internal_create_int_string(char* str, uint32_t uval, uint8_t base, uint8_t min_letters, bool add_leading_zero, bool add_minus);

/**
 *  Creates an integer string from an an unsigned integer 64-bit value.
 *
 * @param str                   Pointer to a buffer where the string will be written to.
 * @param val                   32-Bit integer that needs to be converted to a string.
 * @param base                  Base of the number. Currently supported are 10 for decimal and 16 for hex.
 * @param min_letters           Minimum number of characters that must be shown. If min_letters is 0, the full number will be shown.
 * @param add_leading_zero      true: Leading 0 will be added if the number has less characters then min_letters.
 *                              false: Spaces will be added if the number has less characters then min_letters.
 * @param add_minus             true: A minus will be added right before the num string.
 *                              false: No minus will be added.
 * @return                      Pointer to the address of the terminating zero.
 **/
static char* string_internal_create_int64_string(char* str, uint64_t uval, uint8_t base, uint8_t min_letters, bool add_leading_zero, bool add_minus);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void string_set_thousand_separator_character(char c)
{
    string_thousand_separator = c;
}

char string_get_thousand_separator_character(void)
{
    return string_thousand_separator;
}

void string_set_decimal_point_character(char c)
{
    string_decimal_point = c;
}

char string_get_decimal_point_character(void)
{
    return string_decimal_point;
}

#if MODULE_ENABLE_RTC
void string_set_time_format(char *format, char* order)
{
    string_format_time_order = order;
}

void string_set_date_format(char *format, char* order)
{
    string_format_date_order = order;
}
void string_set_date_seperator(char* c)
{
    string_format_date_seperator = c;
}
#endif

#if STRING_BUILD_PRINTF

int16_t string_vprintf(char* str, const char* format, va_list vl)
{
    return string_vnprintf(str, -1, format, vl);
}

int16_t string_printf(char* str, const char* format, ...)
{
    va_list vl;

    va_start(vl, format);
    string_printf_char_count = string_vnprintf(str, -1, format, vl);
    va_end(vl);

    return string_printf_char_count;
}

int16_t string_nprintf(char* str, int16_t n, const char* format, ...)
{
    va_list vl;

    va_start(vl, format);
    string_printf_char_count = string_vnprintf(str, n, format, vl);
    va_end(vl);

    return string_printf_char_count;
}

int16_t string_vnprintf(char* str, int16_t n, const char* format, va_list vl)
{
    if(str == NULL || format == NULL)
        return -1;

    if(!string_printf_initialized)
    {
        string_printf_initialized = true;

        comm_init_interface(&string_printf_comm_interface);
        string_printf_comm_interface.xputc = (void(*)(void*, int))string_printf_putc;
        string_printf_comm_interface.xputs = (void (*)(void *, uint8_t *, uint16_t))string_printf_puts;
        string_printf_comm_handler.interface = &string_printf_comm_interface;
    }

    string_printf_comm_handler.device_handler = str;
    string_printf_char_max = n;
    string_printf_char_count = 0;

    comm_vprintf(&string_printf_comm_handler, (char*)format, vl);
    if(string_printf_char_max == -1 || string_printf_char_count < string_printf_char_max)
        str[string_printf_char_count] = 0;

    return string_printf_char_count;
}

#endif

#if MODULE_ENABLE_RTC
void string_create_time(char* str, rtc_time_t *time, uint16_t len)
{
    uint8_t i = 0;
    for(i = 0; i < 3; i++)
    {
        switch(string_format_time_order[i])
        {
            case 'h':
                *str++ = string_uint8_to_ascii(time->tm_hour / 10);
                *str++ = string_uint8_to_ascii(time->tm_hour % 10);
            break;

            case 'm':
                *str++ = string_uint8_to_ascii(time->tm_min / 10);
                *str++ = string_uint8_to_ascii(time->tm_min % 10);
            break;

            case 's':
                *str++ = string_uint8_to_ascii(time->tm_sec / 10);
                *str++ = string_uint8_to_ascii(time->tm_sec % 10);
            break;
        }
        if(i < 2)
            *str++ = ':';
    }
    *str++ = 0;
}

void string_create_date(char* str, rtc_time_t *time, uint16_t len)
{
    uint8_t i = 0;
    for(i = 0; i < 3; i++)
    {
        switch(string_format_date_order[i])
        {
            case 'd':
                *str++ = string_uint8_to_ascii(time->tm_mday / 10);
                *str++ = string_uint8_to_ascii(time->tm_mday % 10);
            break;

            case 'm':
            {
                uint8_t month = time->tm_mon + 1;
                *str++ = string_uint8_to_ascii(month / 10);
                *str++ = string_uint8_to_ascii(month % 10);
            }
            break;

            case 'y':
            {
//              if(time->year > 1000)
//              {
                uint16_t year = time->tm_year + RTC_EPOCH_YR;
                    *str++ = string_uint8_to_ascii(year / 1000);
                    *str++ = string_uint8_to_ascii((year / 100) % 10);
//              }
                *str++ = string_uint8_to_ascii((year / 10) % 10);
                *str++ = string_uint8_to_ascii(year % 10);
            }
            break;
        }
        if(i < 2)
            *str++ = string_format_date_seperator[0];
    }
    *str = 0;
}

char* string_parse_datetime(const char* str, char* format, rtc_time_t* time)
{
    char* ptr = NULL;
    uint16_t i = 0;
    uint32_t tmp = 0;
    uint16_t len = 0;

    time->tm_mday = 0;
    time->tm_mon = 0;
    time->tm_year = 0;
    time->tm_hour = 0;
    time->tm_min = 0;
    time->tm_sec = 0;

//  dbg_printf(DBG_STRING, "string_parse_date(\"%s\", \"%s\", <%D %T>)\n", str, format, time, time);

    if(str == NULL || format == NULL || time == NULL)
        return NULL;

    len = strlen(format);

    if((len & 1) == 0 || len > 11)
        return NULL;

    for(i = 0; i < (len + 1) / 2; i++)
    {
        tmp = strtol(str, &ptr, 10);

        if(ptr == NULL)
            return NULL;

        switch(format[i * 2])
        {
            case 'D':
                if(tmp == 0 || tmp > 31)
                    return NULL;
                time->tm_mday = tmp;
            break;

            case 'M':
                if(tmp == 0 || tmp > 12)
                    return NULL;
                time->tm_mon = tmp - 1;
            break;

            case 'Y':
                if(tmp < 100)
                    tmp += 2000;
                time->tm_year = tmp - RTC_EPOCH_YR;
            break;

            case 'h':
                if(tmp > 23)
                    return NULL;
                time->tm_hour = tmp;
            break;

            case 'm':
                if(tmp > 59)
                    return NULL;
                time->tm_min = tmp;
            break;

            case 's':
                if(tmp > 59)
                    return NULL;
                time->tm_sec = tmp;
            break;

            default:
                return NULL;
        }

        if(len - 1 > (i * 2))
        {
            if(*ptr != format[i * 2 + 1])
                return NULL;
        }
        else
            return ptr;

        str = ptr + 1;
    }

    return NULL;
}
#endif

#if !STRING_USE_COMM_MINIMUM
int16_t string_find_first_int(char *str)
{
    uint16_t i;
    uint16_t str_len = strlen(str);

    for(i = 0; i < str_len; i++)
    {
        if(str[i] >=0x30 && str[i] <=0x39)
            return i;
    }

    return -1;
}

int16_t string_index_of_substring(const char *str, char *str_to_find)
{
    uint16_t i;
    uint16_t inside_index = 0;
    uint16_t str_len = strlen(str);
    uint16_t str_to_find_len = strlen(str_to_find);

    for(i=0; i < str_len; i++)
    {
        if(str[i] == str_to_find[inside_index])
        {
            inside_index++;

            if(str_to_find_len == inside_index) // Last character is the same
                return (i - inside_index + 1);
        }
        else
            inside_index = 0;
    }

    return -1;
}

uint16_t string_extract_between(char *str,
                                char *target_str, uint16_t target_str_len,
                                char extract_tag_begin, char extract_tag_end)
{
    uint8_t step = 0;
    uint16_t str_len = strlen(str);
    uint16_t real_target_str_len = 0;
    uint16_t tmp_str_len = 0;

    while(tmp_str_len < str_len && real_target_str_len < target_str_len)
    {
        switch(step)
        {
        case 0:
            if(str[tmp_str_len] == extract_tag_begin)
                step++;
        break;

        case 1:
            if(str[tmp_str_len] == extract_tag_end)
            {
                target_str[real_target_str_len] = 0;
                return real_target_str_len;
            }
            target_str[real_target_str_len] = str[tmp_str_len];
            real_target_str_len++;
        break;
        }
        tmp_str_len++;
    }

    if(real_target_str_len < target_str_len)
        target_str[real_target_str_len] = 0;

    return real_target_str_len;
}

uint16_t string_trim(char *str)
{
    uint16_t i;
    uint16_t str_len = strlen(str);
    uint16_t str_len_old = str_len;
    for(i=0; i<str_len_old; i++)
    {
        if(str[0] != ' ' && str[0] != '\t')
            break;
        memcpy(str, str+1, str_len_old-1);
        if(str_len) str_len--;

    }
    if(!str_len)    return 0;

    for(i=str_len - 1; i>0; i--)
    {
        if(str[i]!=' ' && str[0]!='\t')
            break;
        str[i] = 0;
        str_len--;
    }

    return str_len;
}

bool string_is_decimal(const char *str, uint16_t str_len, bool is_hexa)
{
    if(str_len == 0 || str[0] == 0)
        return false;

    for(uint16_t i = 0; i < str_len; i++)
    {
        switch(str[i])
        {
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':

            break;

            case '-':
                if(i > 0)
                    return false;
            break;

            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                if(!is_hexa)
                    return false;
            break;

            case '\0':
                return true;

            default:
                return false;
        }
    }
    return true;
}
#endif // #if !STRING_USE_COMM_MINIMUM

uint32_t string_count_char(const char* str, char c)
{
	uint32_t cnt = 0;
	if(str == NULL)
		return 0;

	for(;*str; str++)
		cnt += (*str == c);

	return cnt;
}

void string_set_hex_letter_size(bool use_upper)
{
    if(use_upper)
        string_hex_char = 'A';
    else
        string_hex_char = 'a';
}

char string_uint8_to_ascii(uint8_t value)
{
    if(value < 10)  return (value + '0');
    if(value < 16)  return (value + string_hex_char - 10);

    return 0;
}

uint8_t string_ascii_to_uint8(char letter)
{
    if(letter >= 'a' && letter <= 'z')  return letter - 'a' + 10;
    if(letter >= 'A' && letter <= 'Z')  return letter - 'A' + 10;
    if(letter >= '0' && letter <= '9')  return letter - '0';

    return 0;
}

bool string_parse_hex_array(const char* str, uint8_t* v, uint16_t v_len)
{
	if(str == NULL || v == NULL || v_len == 0)
		return false;

	for(int i = 0; i < v_len; i++, str+=2)
	{
		if(str[0] == 0 || str[1] == 0)
			return false;

		v[i] = (string_ascii_to_uint8(str[0]) << 4) + string_ascii_to_uint8(str[1]);
	}
	return true;
}

char string_to_lower(char letter)
{
    if(letter >= 'A' && letter <= 'Z')
        return letter - 'A' + 'a';
    return letter;
}

char string_to_upper(char letter)
{
    if(letter >= 'a' && letter <= 'z')
        return letter - 'a' + 'A';
    return letter;
}

char* string_create_uint_string(char* str, uint32_t val, uint8_t base, uint8_t min_letters, bool add_leading_zero)
{
    return string_internal_create_int_string(str, val, base, min_letters, add_leading_zero, false);
}

char* string_create_int_string(char* str, int32_t val, uint8_t base, uint8_t min_letters, bool add_leading_zero)
{
    bool add_minus = false;

    if(val < 0)
    {
        add_minus = true;
        val *= -1;
    }

    return string_internal_create_int_string(str, (uint32_t)val, base, min_letters, add_leading_zero, add_minus);
}

char* string_create_uint64_string(char* str, uint64_t val, uint8_t base, uint8_t min_letters, bool add_leading_zero)
{
    return string_internal_create_int64_string(str, val, base, min_letters, add_leading_zero, false);
}

char* string_create_int64_string(char* str, int64_t val, uint8_t base, uint8_t min_letters, bool add_leading_zero)
{
    bool add_minus = false;

    if(val < 0)
    {
        add_minus = true;
        val *= -1;
    }

    return string_internal_create_int64_string(str, (uint64_t)val, base, min_letters, add_leading_zero, add_minus);
}

#if !STRING_USE_COMM_MINIMUM

char* string_create_num_string(char *str_buf, int32_t num, bool with_thousand_seperator)
{
    uint32_t max_num = 1000000000;
    bool in_num = false;

    if(str_buf == NULL)
        return NULL;

    if(num < 0)
    {
        num *= -1;
        *str_buf++ = '-';
    }
    else if(num == 0)
    {
        *str_buf++ = '0';
        *str_buf = 0;
        return str_buf;
    }

    while(max_num > 0)
    {
        if(num >= max_num)
        {
            if(in_num)
                str_buf = string_create_uint_string(str_buf, (num / max_num) % 1000, 10, 3, true);
            else
                str_buf = string_create_uint_string(str_buf, (num / max_num) % 1000, 10, 0, true);

            if(with_thousand_seperator && max_num > 1)
                *str_buf++ = string_thousand_separator;

            in_num = true;
        }
        max_num /= 1000;
    }

    *str_buf = 0;

    return str_buf;
}

#endif //#if !STRING_USE_COMM_MINIMUM

bool string_ends_with(char* str, char* end_str)
{
    uint16_t len1, len2;

    if(str == NULL || end_str == NULL)
        return false;

    len1 = strlen(str);
    len2 = strlen(end_str);

    if(len1 == 0 || len2 == 0 || len2 > len1)
        return false;

    if(strstr(str + len1 - len2, end_str) == NULL)
        return false;

    return true;
}

int string_strcasecmp(const char *s1, const char* s2)
{
    if(s1 == NULL && s2 == NULL)
        return 0;

    if(s1 == NULL)
        return -1;
    if(s2 == NULL)
        return 1;

    if(s1 == s2) // Same pointer, Same string
        return 0;

    while(*s1 && *s2)
    {
        char c1 = string_to_lower(*s1);
        char c2 = string_to_lower(*s2);
        int res = c1 - c2;

        if(res != 0)
            return res;

        s1++;
        s2++;
    }

    if(*s1)
        return 1;
    if(*s2)
        return -1;

    return 0;
}

int string_strncasecmp(const char *s1, const char* s2, uint16_t len)
{
    if(len == 0)
        return 0;

    if(s1 == NULL && s2 == NULL)
        return 0;

    if(s1 == NULL)
        return -1;

    if(s2 == NULL)
        return 1;

    if(s1 == s2) // Same pointer, Same string
        return 0;

    while(*s1 && *s2 && len > 0)
    {
        char c1 = string_to_lower(*s1);
        char c2 = string_to_lower(*s2);
        int res = c1 - c2;

        if(res != 0)
            return res;

        s1++;
        s2++;
        len--;
    }

    if(len > 0)
    {
        if(*s1)
            return 1;
        if(*s2)
            return -1;
    }

    return 0;
}

char* string_strcasestr(const char *s, const char *find)
{
    char sc;
    uint16_t len;
    char c;

    if(s == NULL || find == NULL || find[0] == 0)
        return NULL;

    if ((c = *find++) != 0) {
        c = (char)string_to_lower((unsigned char)c);
        len = strlen(find);
        do {
            do {
                if ((sc = *s++) == 0)
                    return (NULL);
            } while ((char)string_to_lower((unsigned char)sc) != c);
        } while (string_strncasecmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}

char* string_strstr_end(const char* haystack, const char* needle)
{
    if(haystack == NULL || needle == NULL || needle[0] == 0)
        return NULL;

    char* ptr = strstr(haystack, needle);

//  DBG_VERBOSE("string_strstr_end(%s, %s) -> %s\n", haystack, needle, ptr ? ptr : NULL);

    if(ptr)
        return ptr + strlen(needle);

    return NULL;
}

bool string_is_valid_num_array(const char* str, char* min, char* max, uint16_t max_entries, bool is_hex)
{
    if(str == NULL)
        return false;

    const char* ptr1 = str;
    uint16_t len_min = min == NULL ? 0 : strlen(min);
    uint16_t len_max = max == NULL ? 0 : strlen(max);
    uint16_t len = 0;
    uint16_t num_entries = 0;
    bool is_negative = false;
    bool min_is_negative = min && min[0] == '-';
    bool max_is_negative = max && max[0] == '-';
    bool in_number = false;
    bool has_separator = true;

    if(min)
    {
        // Skip leading -
        if(min_is_negative)
        {
            min++;
            len_min--;
        }

        // Skip leading 0
        while(len_min > 1 && *min == '0')
        {
            min++;
            len_min--;
        }
    }

    if(max)
    {
        // Skip leading -
        if(max_is_negative) // If negative, just check the number
        {
            max++;
            len_max--;
        }

        // Skip leading 0
        while(len_max > 1 && *max == '0')
        {
            max++;
            len_max--;
        }
    }

    while(true)
    {
        if(*str == ' ' || *str == ',' || *str == '\t' || *str == '\n' || *str == '\r' || *str == 0) // Skipping whitespaces, comma and new lines
        {
            if(in_number) // end of a number!
            {
                // Number searched! -> No found number means error!
                if(len == 0)
                {
//                  DBG_ERROR("Missing number!\n");
                    return false;
                }

                // Skip leading zeroes
                while(len > 1 && *ptr1 == '0')
                {
                    len--;
                    ptr1++;
                }

                if(is_negative)
                {
                    if(min)
                    {
                        if(min_is_negative)
                        {
                            if(len > len_min) // value string is longer than minimum string -> value is smaller than minimum!
                            {
    //                          DBG_ERROR("below minimum A [%d]!\n", num_entries);
                                return false;
                            }
                            else if(len == len_min) // Same length -> compare string value
                            {
                                if(string_strncasecmp(ptr1, min, len) > 0) // value is smaller than minimum!
                                {
    //                              DBG_ERROR("below minimum B [%d]!\n", num_entries);
                                    return false;
                                }
                            }
                            // else len of value is smaller than minimum length -> value is above minimum -> everything is fine
                        }
                        else // Minimum is positive -> Value is below minimum!
                        {
    //                      DBG_ERROR("below minimum C [%d]!\n", num_entries);
                            return false;
                        }
                    }

                    if(max)
                    {
                        if(max_is_negative)
                        {
                            if(len < len_max) // Value string is smaller than maximum string -> Value is above maximum!
                            {
    //                          DBG_ERROR("above maximum A [%d]!\n", num_entries);
                                return false;
                            }
                            else if(len == len_max)
                            {
                                if(string_strncasecmp(ptr1, max, len) < 0) // Value is above maximum!
                                {
    //                              DBG_ERROR("above maximum B [%d]!\n", num_entries);
                                    return false;
                                }
                            }
                            // else len of value is higher than maximum length -> value is below maximum -> everything is fine
                        }
                        // else max > 0 and value < 0 -> value is below maximum -> everything is fine
                    }
                }
                else
                {
                    if(min)
                    {
                        if(!min_is_negative) // Min is positive -> compare minimum with value
                        {
                            if(len < len_min) // Value shorter -> Value below minimum
                            {
    //                          DBG_ERROR("below minimum D [%d]!\n", num_entries);
                                return false;
                            }
                            else if(len == len_min) // Same length -> Compare string
                            {
                                if(string_strncasecmp(ptr1, min, len) < 0) // value is smaller than minimum!
                                {
    //                              DBG_ERROR("below minimum E [%d]!\n", num_entries);
                                    return false;
                                }
                            }
                            // else value longer -> Value above minimum
                        }
                        //else: negative min and positive value means value is greater -> everything is fine
                    }

                    if(max)
                    {
                        if(max_is_negative) // Value positive, max negative -> Value above maximum!
                        {
    //                      DBG_ERROR("above maximum C [%d]!\n", num_entries);
                            return false;
                        }
                        else // Both positive
                        {
                            if(len > len_max) // Value longer -> Value above maximum!
                            {
    //                          DBG_ERROR("above maximum D [%d]!\n", num_entries);
                                return false;
                            }
                            else if(len == len_max) // Same length -> Compare string
                            {
                                if(string_strncasecmp(ptr1, max, len) > 0) // Value is above maximum!
                                {
    //                              DBG_ERROR("above maximum E [%d]!\n", num_entries);
                                    return false;
                                }
                            }
                            // else Value shorter -> Value below maximum -> everything is fine
                        }
                    }
                }

                // No Number check because len is > 0 only for valid numbers!
                in_number = false;
                has_separator = false;
                is_negative = false;

                num_entries++;
            }
            else if(is_negative) // not a number, but negative -> Wrong!
            {
                return false;
            }

            if(*str == ',')
            {
                if(has_separator) // Two separators behind each other without a number -> wrong!
                {
//                  DBG_ERROR("double separator [%d]!\n", num_entries);
                    return false;
                }

                has_separator = true;
            }
            len = 0;

            if(*str == 0)
            {
                if(num_entries > 0 && num_entries <= max_entries)
                    return true;
                else
                {
//                  DBG_ERROR("0 < %d <= %d\n", num_entries, max_entries);
                    return false;
                }
            }
        }
        else if((*str >= '0' && *str <= '9') || (is_hex && ((*str >= 'A' && *str <= 'F') || (*str >= 'a' && *str <= 'f'))))
        {
            // Is a number!
            in_number = true;
            if(len == 0)
            {
                ptr1 = (char*)str; // Set pointer to first character of the number!
                if(!has_separator)
                {
//                  DBG_ERROR("missing separator [%d]!\n", num_entries);
                    return false;
                }
            }
            len++;
        }
        else if(*str == '-')
        {
            if(!in_number && len == 0 && !is_negative) // Valid at the begin of a number!
                is_negative = true;
            else
            {
//              DBG_ERROR("- in the middle [%d]!\n", num_entries);
                return false;
            }
        }
        else
        {
            // Invalid character!
//          DBG_ERROR("Invalid character [%d]!\n", num_entries);
            return false;
        }
        str++;
    }

    // Never reached! -> return true is inside while on *str == 0
    return true;
}

bool string_read_next_num_array_value(const char* str, char** ptr, int32_t* v, bool is_hex, bool is_unsigned)
{
    if(str == NULL || v == NULL)
        return false;

    if(ptr)
        *ptr = NULL;

    while(true)
    {
        // If the number or - is found -> convert it!
        if(*str == '-' || (*str >= '0' && *str <= '9') || (is_hex && ((*str >= 'A' && *str <= 'F') || (*str >= 'a' && *str <= 'f'))))
        {
            if(is_unsigned)
                *v = strtoul(str, ptr, is_hex ? 16 : 10);
            else
                *v = strtol(str, ptr, is_hex ? 16 : 10);
            return true;
        }
        else if(*str == ' ' || *str == ',' || *str == '\t' || *str == '\n' || *str == '\r')
        {
            // Skip whitespaces
            str++;
        }
        else // Invalid characters...
            return false;
    }
    return false;
}

bool string_read_num_array(const char* str, int32_t* arr, uint16_t max_entries, bool is_hex, uint16_t* entries, bool is_unsigned)
{
    if(str == NULL || arr == NULL)
        return false;

    char* ptr = (char*)str;
    uint16_t cnt = 0;
    while(string_read_next_num_array_value(ptr, &ptr, &arr[cnt], is_hex, is_unsigned) && cnt < max_entries)
    {
        cnt++;
    }

    if(entries)
        *entries = cnt;

    return cnt > 0;
}

int string_read_string_array(const char* str, char** entries, int entry_len, int entry_num)
{
    if(str == NULL || entries == NULL)
    {
        return 0;
    }

	DBG_INFO("Str=%s out=%08x len=%d num=%d\n", str, entries, entry_len, entry_num);

	int state = 0;
	char* ptr = (char*)str;
	char* dst = (char*)entries;
	int k = 0;
	int i = 0;

	while(*ptr)
	{
		if(state == 0) // State = 0: First quote
		{
			if(k >= entry_num)
			{
				DBG_ERROR("Too many entries\n");
				return -1;
			}

			if(*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')
			{
				// Ignore whitespace
			}
			else if(*ptr != '"')
			{
				DBG_ERROR("Expecting quote\n");
				return -1;
			}
			else
			{
				state++; // State = 1 -> Extract content
				i = 0;
			}
		}
		else if(state == 1) // State = 1 -> Extract content
		{
			if(*ptr != '"')
			{
				if(i < entry_len - 1)
				{
//					DBG_INFO("[%d]=%c(%02x)\n", i, *ptr, (uint32_t)*ptr);
					dst[i++] = *ptr;
//					DBG_INFO("[%d]=%c(%02x)\n", i, *ptr, (uint32_t)*ptr);
				}
				else
				{
					DBG_ERROR("Too long\n");
					return -1;
				}
			}
			else
			{
				dst[i] = 0;
				DBG_INFO("Entry %d [%d]: \"%s\"[%08x]\n", k, i, dst, dst);
				state++; // State = 2 Comma for next
			}
		}
		else if(state == 2) // State = 2 Comma for next
		{
			if(*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')
			{
				// Ignore whitespace
			}
			else if(*ptr != ',')
			{
				DBG_ERROR("Expecting comma!\n");
				return -1;
			}
			else
			{
				state = 0; // State = 0: First quote
				dst += entry_len;
				k++;
			}
		}

		ptr++;
	}

	if(state == 1)
		return -1;

	if(state == 2)
		k++;

	DBG_INFO("State = %d Entries = %d\n", state, k);

	return k;
}

bool string_parse_ipv4(const char* str, uint8_t* ip)
{
    int32_t v;
    uint8_t cnt = 0;
    uint8_t ip1[4];

    if(str == NULL)
        return false;

    char* ptr = (char*)str;

    do
    {
        // Has to start with a number...
        if(ptr[0] < '0' || ptr[0] > '9')
            return false;
        v = strtol(ptr, &ptr, 10);

        if(v < 0 || v > 255)
            return false;

        if(cnt < 3 && *ptr != '.') // first 3 must have a '.' behind them
            return false;
        else if(cnt == 3 && *ptr) // has to be 0-terminated
            return false;

        ptr++; // is a dot after first 3 elements and irrelevant after the fourth

        ip1[cnt] = (uint8_t)v;
        cnt++;
    }while(cnt < 4);

    if(ip)
        memcpy(ip, ip1, 4);

    return true;
}

int string_replace(char *str, char orig, char rep)
{
    if(str == NULL)
        return 0;
    
    char *ix = str;
    int n = 0;

    while((ix = strchr(ix, orig)) != NULL)
    {
        *ix++ = rep;
        n++;
    }
    return n;
}

#if MODULE_ENABLE_RTC
bool string_parse_datetime_js(const char* str, rtc_time_t* t, int8_t* timezone)
{
    char* ptr = (char*)str;
    int32_t v;
    if(str == NULL || t == NULL)
        return false;

    ptr++;
    v = strtol(str, &ptr, 10);
    if(ptr == NULL || ptr == str || *ptr != '-' || v < 1900)
        return false;
    str = ptr;
    t->tm_year = v - RTC_EPOCH_YR;
    str = ptr + 1;
    v = strtol(str, &ptr, 10);
    if(ptr == NULL || ptr == str || *ptr != '-' || v <= 0 || v > 12)
        return false;
    t->tm_mon = v - 1;
    str = ptr + 1;
    v = strtol(str, &ptr, 10);
    if(ptr == NULL || ptr == str || *ptr != 'T' || v <= 0 || v > 31)
        return false;
    t->tm_mday = v;
    str = ptr + 1;
    v = strtol(str, &ptr, 10);
    if(ptr == NULL || ptr == str || *ptr != ':' || v < 0 || v > 23)
        return false;
    t->tm_hour = v;
    str = ptr + 1;
    v = strtol(str, &ptr, 10);
    if(ptr == NULL || ptr == str || *ptr != ':' || v < 0 || v > 59)
        return false;
    t->tm_min = v;
    str = ptr + 1;
    v = strtol(str, &ptr, 10);
    if(ptr == NULL || ptr == str || v < 0  || v > 59)
        return false;
    t->tm_sec = v;
    str = ptr + 1;
    if(*ptr == '.')
    {
        v = strtol(str, &ptr, 10);
        if(ptr == NULL || ptr == str || v < 0)
            return false;
        // Do something with the milliseconds?
        str = ptr + 1;
    }

    if(*ptr == 'Z' || *ptr == '+' || *ptr == '-')
    {
        v = 0;
        if(*ptr != 'Z')
        {
            if(strlen(str) < 2)
                return false;

            if(ptr[1] < '0' || ptr[1] > '1' 
            || ptr[2] < '0' || ptr[2] > '9')
                return false;

            if(*ptr == '+')
            {
                v = strtol(str, &ptr, 10);
                if(ptr == NULL || ptr == str)
                    return false;
                if(*ptr == ':' && strcmp(":00", ptr) != 0)
                    return false;
            }
            else
            {
                v = strtol(str, &ptr, 10) *-1;
                if(ptr == NULL || ptr == str)
                    return false;
                if(*ptr == ':' && strcmp(":00", ptr) != 0)
                    return false;
            }
        }        

        if(timezone)
        {
            *timezone = v;
        }
    }
        
    return true;
}

bool string_create_datetime_js(char* str, uint16_t max_len, rtc_time_t* t, int8_t timezone, bool add_fractals)
{
    int res;
    rtc_time_t ts;

    char str_zone[8] = {0};
    char str_fractals[8] = {0};

    if(str == NULL || t == NULL)
        return false;

    memcpy(&ts, t, sizeof(rtc_time_t));

    // --- Quickfix Nextcharge?
    if(timezone != 0)
    {
    	time_t x = rtc_mktime(t);
    	if(timezone < 0)
    	{
    		timezone = -timezone;
    		x += (time_t)((int32_t)timezone * 3600);
    	}
    	else
    		x -= (time_t)((int32_t)timezone * 3600);
    	ts = rtc_time(x);

    	timezone = 0;
    }
    // --- End Quickfix Nextcharge

    if(timezone == 0)
    {
        str_zone[0] = 'Z';
    }
    else
    {
        string_printf(str_zone, "%c%02d:00", timezone > 0 ? '+' : '-', timezone >= 0 ? timezone : -timezone);
    }

    if(add_fractals)
    {
        string_printf(str_fractals, ".%03d", system_get_tick_count() % 1000);
    }

    res = snprintf(str, max_len, "%04d-%02d-%02dT%02d:%02d:%02d%s%s",
                    (int)(ts.tm_year + RTC_EPOCH_YR), (int)(ts.tm_mon + 1), (int)(ts.tm_mday),
                    (int)(ts.tm_hour), (int)(ts.tm_min), (int)(ts.tm_sec), str_fractals, str_zone);


    if(res < 0 || res >= max_len)
        return false;

    return true;
}

bool string_create_datetime_ocmf(char* str, uint16_t max_len, rtc_time_t* t, int8_t timezone)
{
    int res;
    rtc_time_t ts;

    char str_zone[8] = {0};
    char str_fractals[8] = {0};

    if(str == NULL || t == NULL)
        return false;

    memcpy(&ts, t, sizeof(rtc_time_t));

    // --- Quickfix Nextcharge?
    if(timezone != 0)
    {
    	time_t x = rtc_mktime(t);
    	if(timezone < 0)
    	{
    		timezone = -timezone;
    		x += (time_t)((int32_t)timezone * 3600);
    	}
    	else
    		x -= (time_t)((int32_t)timezone * 3600);
    	ts = rtc_time(x);

    	timezone = 0;
    }
    // --- End Quickfix Nextcharge

    string_printf(str_zone, "%c%02d00", timezone >= 0 ? '+' : '-', timezone > 0 ? timezone : -timezone);
    string_printf(str_fractals, ",%03d", system_get_tick_count() % 1000);    

    res = snprintf(str, max_len, "%04d-%02d-%02dT%02d:%02d:%02d%s%s",
                    (int)(ts.tm_year + RTC_EPOCH_YR), (int)(ts.tm_mon + 1), (int)(ts.tm_mday),
                    (int)(ts.tm_hour), (int)(ts.tm_min), (int)(ts.tm_sec), str_fractals, str_zone);

    if(res < 0 || res >= max_len)
        return false;

    return true;
}
#endif // #if MODULE_ENABLE_RTC

FUNCTION_RETURN string_parse_bool(const char* str, bool* bool_value)
{
    if(str == NULL)
        return FUNCTION_RETURN_PARAM_ERROR;

    bool tmp_bool;

    if(string_strcasecmp(str, "true") == 0)
        tmp_bool = true;
    else if(string_strcasecmp(str, "false") == 0)
        tmp_bool = false;
    else
        return FUNCTION_RETURN_PARAM_ERROR;

    if(bool_value != NULL)
        *bool_value = tmp_bool;
    
    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN string_unescape(const char *src, size_t src_len, char *dest, size_t dest_len)
{
    DBG_ASSERT(src != NULL, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Source buffer null");
    DBG_ASSERT(dest != NULL, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR, "Destination buffer null");

    size_t src_pos = 0;
    size_t dest_pos = 0;

    while(src_pos < src_len && dest_pos < dest_len)
    {
        if(src[src_pos] == '\\' && src_pos < src_len - 1)
        {
            src_pos++;
            switch(src[src_pos])
            {
                case 'a':
                    dest[dest_pos] = '\a';
                    break;
                case 'b':
                    dest[dest_pos] = '\b';
                    break;
                case 'f':
                    dest[dest_pos] = '\f';
                    break;
                case 'v':
                    dest[dest_pos] = '\v';
                    break;
                case 'n':
                    dest[dest_pos] = '\n';
                    break;
                case 'r':
                    dest[dest_pos] = '\r';
                    break;
                case 't':
                    dest[dest_pos] = '\t';
                    break;
                default:
                    dest[dest_pos] = src[src_pos];
            }
        }
        else
            dest[dest_pos] = src[src_pos];
        
        dest_pos++;
        src_pos++;
    }  
    return src_pos == src_len ? FUNCTION_RETURN_OK : FUNCTION_RETURN_INSUFFICIENT_MEMORY;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if STRING_BUILD_PRINTF

static void string_printf_putc(char* str, int ch)
{
    // With -1, the check is not used, but if 0 or higher, the count uses the meax value
    if(string_printf_char_max >= 0 && (string_printf_char_count >= string_printf_char_max))
        return;

    str[string_printf_char_count] = ch;
    string_printf_char_count++;
}

static void string_printf_puts(char* str, char* str2, uint16_t len)
{
    while(len > 0)
    {
        // With -1, the check is not used, but if 0 or higher, the count uses the meax value
        if(string_printf_char_max >= 0 && (string_printf_char_count >= string_printf_char_max))
            return;

        str[string_printf_char_count] = *str2++;
        string_printf_char_count++;
        len--;
    }
}

#endif

static char* string_internal_create_int_string(char* str, uint32_t uval, uint8_t base, uint8_t min_letters, bool add_leading_zero, bool add_minus)
{
    uint8_t len = 10;
    uint32_t max_val = 1000000000;

    switch(base)
    {
        case 2:
            len = 32;
            max_val = 0x80000000;
        break;

        case 10:
            len = 10;
            max_val = 1000000000;
        break;

        case 16:
            len = 8;
            max_val = 0x10000000;
        break;
    }

    for(; len > 0; len--)
    {
        if(uval >= max_val)
            break;

        max_val /= base;
    }

    if(len == 0)
    {
        len = 1;
        max_val = 1;
    }

    if(min_letters > 0 && min_letters > (len + add_minus))
    {
        min_letters -= (len + add_minus);

        if(add_leading_zero && add_minus)
        {
            *str++ = '-';
            add_minus = false;
        }

        while(min_letters > 0)
        {
            if(add_leading_zero)
                *str++ = '0';       // Add leading 0
            else
                *str++ = ' ';       // Add Space
            min_letters--;
        }
    }

    if(add_minus)
        *str++ = '-';

    while(len > 0)
    {
        *str++ = string_uint8_to_ascii((uval / max_val) % base);
        max_val /= base;
        len--;
    }

    *str = 0;

    return str;
}

static char* string_internal_create_int64_string(char* str, uint64_t uval, uint8_t base, uint8_t min_letters, bool add_leading_zero, bool add_minus)
{
    uint8_t len = 20;
    uint64_t max_val = 10000000000000000000ULL;

    switch(base)
    {
        case 2:
            len = 64;
            max_val = 0x8000000000000000;
        break;

        case 10:
            len = 20;
            max_val = 10000000000000000000ULL;
        break;

        case 16:
            len = 16;
            max_val = 0x1000000000000000;
        break;
    }

    for(; len > 0; len--)
    {
        if(uval >= max_val)
            break;

        max_val /= base;
    }

    if(len == 0)
    {
        len = 1;
        max_val = 1;
    }

    if(min_letters > 0 && min_letters > (len + add_minus))
    {
        min_letters -= (len + add_minus);

        if(add_leading_zero && add_minus)
        {
            *str++ = '-';
            add_minus = false;
        }

        while(min_letters > 0)
        {
            if(add_leading_zero)
                *str++ = '0';       // Add leading 0
            else
                *str++ = ' ';       // Add Space
            min_letters--;
        }
    }

    if(add_minus)
        *str++ = '-';

    while(len > 0)
    {
        *str++ = string_uint8_to_ascii((uval / max_val) % base);
        max_val /= base;
        len--;
    }

    *str = 0;

    return str;
}

#endif
