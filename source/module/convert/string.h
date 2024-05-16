// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 *  @file   string.h
 *  @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author     Tim Koczwara
 *
 *  @brief
 *          Contains helping functions to work with Strings.
 *          Extracted from the old ESoPe convert.c module.
 *
 *	@version	1.12 (19.01.2022)
 * 	    - Modified to be used in esopekernel
 *  @version    1.11 (02.09.2021)
 *    - Added string_nprintf and string_vnprintf
 *	@version 	  1.10 (14.12.2019)
 *		- Added uint32_t string_count_char(char* str, char c)
 *  @version    1.09 (13.11.2019)
 *      - Added string_parse_bool
 *  @version    1.08 (11.11.2019)
 *      - Added string_replace
 *      - Added string_parse_datetime_json
 *  @version    1.07 (15.05.2019)
 *      - Added string_is_valid_num_array
 *      - Added string_read_next_num_array_value
 *      - Added string_read_num_array
 *      - Added string_parse_ipv4
 *  @version    1.06 (06.08.2018)
 *      - Added string_strstr_end
 *      - Added stdlib include
 *  @version    1.05 (07.06.2018)
 *      - Added module.h support
 *  @version    1.04 (24.05.2018)
 *      - Added string_set_thousand_separator_character, string_get_thousand_separator_character, string_set_decimal_point_character,
 *          string_get_decimal_point_character, string_set_time_format, string_set_date_format, string_set_date_seperator,
 *          string_vprintf, string_create_time, string_create_date, string_parse_datetime, string_create_num_string, string_to_lower,
 *          string_to_upper, string_ends_with, string_strcasecmp and string_strcasestr
 *  @version    1.03 (04.01.2015)
 *      - Added string_printf
 *      - Added string_create_int_string
 *      - Added string_create_uint_string
 *      - Changed string_create_num_string -> Does not use sprintf anymore
 *  @version    1.02 (16.07.2014)
 *      - Added string_create_num_string
 *      - Includes changed
 *  @version    1.01 (17.02.2013)
 *      - Removed Paramater length parameter from string_find_first_int, string_index_of_substring,
 *          string_extract_between and string_trim
 *      - Bugfix string_index_of_substring
 *      - Bugfix in string_ascii_to_uint8
 *      - Bugfix in string_trim
 *  @version    1.00 (28.09.2012)
 *      - Intial release
 *
 *  @par    References
 *
 ******************************************************************************/
 
#ifndef _CONVERT_STRING_HEADER_FIRST__INCL__
#define _CONVERT_STRING_HEADER_FIRST__INCL__


#include "module_public.h"
#if MODULE_ENABLE_CONVERT_STRING
#include <stdio.h>      // for sprintf -> Try to use string_printf on small microcontrollers!
#include <string.h>     // for other string operations
#include <stdarg.h>     // for string_printf
#include <stdlib.h>
#include "module/enum/function_return.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the string module
#define STRING_STR_VERSION "1.12"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief   Sets the thousand point character used in string_create_num_string and string_printf.
 *          Default is the german '.' as thousand point. For example 1.000,00 for one thousand.
 *
 * @param c             Character for the thousand point.
 */
void string_set_thousand_separator_character(char c);

/**
 * @brief   Returns the character that is used as a thousand point.
 *
 * @return              Character for the thousand point.
 */
char string_get_thousand_separator_character(void);

/**
 * @brief   Sets the decimal point character used in string_create_num_string and string_printf.
 *          Default is the german ',' as decimal point. For example 1.000,00 for one thousand.
 *
 * @param c             Character for the decimal point.
 */
void string_set_decimal_point_character(char c);

/**
 * @brief   Returns the character that is used as a decimal point.
 *
 * @return              Character for the decimal point.
 */
char string_get_decimal_point_character(void);

#if MODULE_ENABLE_RTC
/**
 *  Sets the format and order that is used to create a time string in string_create_time.
 *  The format is a printf formatted string. Default is %02d:%02d:%02d.
 *  The order is used to set the order of hour, minute and seconds. The string must have 3 letters. Default is "hms"
 *  - h for hour
 *  - m for minute
 *  - s for seconds
 *
 * @param format        UNUSED AT THE MOMENT. sprintf formatted string used to format the time.
 * @param order         Order of hour, minute and second as a 3 letter string.
 */
void string_set_time_format(char *format, char* order);

/**
 *  Sets the format and order that is used to create a date string in string_create_date.
 *  The format is a printf formatted string. Default is %02d.%02d.%04d.
 *  The order is used to set the order of date, month and year. The string must have 3 letters. Default is "dmy"
 *  - d for date
 *  - m for month
 *  - y for year
 *
 * @param format        UNUSED AT THE MOMENT. sprintf formatted string used to format the date.
 * @param order         Order of date, month and year as a 3 letter string.
 */
void string_set_date_format(char *format, char* order);
void string_set_date_seperator(char* c);
#endif

#if STRING_BUILD_PRINTF

/**
 *  Composes a string with the same text that would be printed if format was used on xprintf,
 *  but instead of being printed, the content is stored as a string in the buffer pointed by str.
 *  The size of the buffer should be large enough to contain the entire resulting string.
 *
 *  A terminating null character is automatically appended after the content.
 *
 *  See vxprintf in comm.h for the possible format strings.
 *  See COMM_MAX_FORMAT_LENGTH for maximum number of formating lengths.
 *
 * @param str           Pointer a buffer where the resulting string is stored.
 * @param format        String that contains a format string that follows the same specification as str in vxprintf.
 * @param vl            Variable argument list corresponding to the format.
 *
 * @return              On success, the total number of characters written is returned.
 *                      This count does not include the additional null-character automatically appended at the end of the string.
 *                      On failure, a negative number is returned.
 */
int16_t string_vprintf(char* str, const char* format, va_list vl);

/**
 *  Composes a string with the same text that would be printed if format was used on xprintf,
 *  but instead of being printed, the content is stored as a string in the buffer pointed by str.
 *  The size of the buffer should be large enough to contain the entire resulting string.
 *
 *  A terminating null character is automatically appended after the content. If the buffer is full before the end is reached, the 0 will not be added.
 *
 *  See vxprintf in comm.h for the possible format strings.
 *  See COMM_MAX_FORMAT_LENGTH for maximum number of formating lengths.
 *
 * @param str           Pointer a buffer where the resulting string is stored.
 * @param n             Maximum size of the buffer that can be filled.
 * @param format        String that contains a format string that follows the same specification as str in vxprintf.
 * @param vl            Variable argument list corresponding to the format.
 *
 * @return              On success, the total number of characters written is returned.
 *                      This count does not include the additional null-character automatically appended at the end of the string.
 *                      On failure, a negative number is returned.
 */
int16_t string_vnprintf(char* str, int16_t n, const char* format, va_list vl);

/**
 *  Composes a string with the same text that would be printed if format was used on xprintf,
 *  but instead of being printed, the content is stored as a string in the buffer pointed by str.
 *  The size of the buffer should be large enough to contain the entire resulting string.
 *
 *  A terminating null character is automatically appended after the content.
 *
 *  See vxprintf in comm.h for the possible format strings.
 *  See COMM_MAX_FORMAT_LENGTH for maximum number of formating lengths.
 *
 * @param str           Pointer a buffer where the resulting string is stored.
 * @param format        String that contains a format string that follows the same specification as str in vxprintf.
 * @param ...           Different parameters according to the format.
 *
 * @return              On success, the total number of characters written is returned.
 *                      This count does not include the additional null-character automatically appended at the end of the string.
 *                      On failure, a negative number is returned.
 */
int16_t string_printf(char* str, const char* format, ...);

/**
 *  Composes a string with the same text that would be printed if format was used on xprintf,
 *  but instead of being printed, the content is stored as a string in the buffer pointed by str.
 *  The size of the buffer should be large enough to contain the entire resulting string.
 *
 *  A terminating null character is automatically appended after the content. If the buffer is full before the end is reached, the 0 will not be added.
 *
 *  See vxprintf in comm.h for the possible format strings.
 *  See COMM_MAX_FORMAT_LENGTH for maximum number of formating lengths.
 *
 * @param str           Pointer a buffer where the resulting string is stored.
 * @param n             Maximum size of the buffer that can be filled.
 * @param format        String that contains a format string that follows the same specification as str in vxprintf.
 * @param ...           Different parameters according to the format.
 *
 * @return              On success, the total number of characters written is returned.
 *                      This count does not include the additional null-character automatically appended at the end of the string.
 *                      On failure, a negative number is returned.
 */
int16_t string_nprintf(char* str, int16_t n, const char* format, ...);

#endif

#if MODULE_ENABLE_RTC
/**
 *  Creates a string based on the set format and order for the time.
 *      Default format is %02d:%02d:%02d
 *      Default order is hour minute second "hms".
 *
 * @param str           Pointer to the string where the format should be written into. Make sure there is enough space.
 * @param time          Pointer to the rtc time that should be converted to the string.
 */
void string_create_time(char* str, rtc_time_t *time, uint16_t len);


/**
 *  Creates a string based on the set format and order for the time.
 *      Default format is %02d.%02d.%04d
 *      Default order is date month year "dmy".
 *
 * @param str           Pointer to the string where the format should be written into. Make sure there is enough space.
 * @param time          Pointer to the rtc date that should be converted to the string.
 */
void string_create_date(char* str, rtc_time_t *time, uint16_t len);

/**
 * Parses the date inside str into the time structure. The format of the date inside str can be defined with the format parameter.
 *      The characters Y, M, D represent the year, month and date inside the format string.
*       The characters h, m, s represent the hour, minute and second inside the format string.
 *      The format must be written with alternating date character and separator (e.g. '-', '.', etc.) and must start with a date character.
 *      Example: "D.M.Y h:m:s", "Y-M-D", ""h:m:s", "h:m", "h:m.s"
 *      Example invalid: "-m-d", "dsgf", ":m:s", "h:m:"
 *
 * @param str           Pointer to the strint where the date is stored.
 * @param format        Format of the string that should be parsed.
 * @param time          Pointer to the time structure where the parsed date is written into.
 * @return              NULL: There was an error in the string or format.
 *                      Otherwise: Points to the first character behind the last parsed date character.
 */
char* string_parse_datetime(const char* str, char* format, rtc_time_t* time);
#endif

#if !STRING_USE_COMM_MINIMUM
/**
 *  Returns the index of the first numeric character inside the given string.
 *
 * @param str                   Pointer to the string which is searched.
 * @return                      Returns -1 if no numeric character is found or the index of the first numeric character.
 **/
int16_t string_find_first_int(char *str);

/**
 *  Returns the index of the first position of the substring.
 *
 * @param str                   Pointer to the string which should include the substring.
 * @param str_to_find           Pointer to the substring which is searched inside str.
 * @return                      Returns -1 if the substring is not found. Otherwise the index of the first character is returned.
 **/
int16_t string_index_of_substring(const char *str, char *str_to_find);

/**
 *  Extractes a substring from a String which is enclosed between to special signs.
 *
 * @param str                   Pointer to the string.
 * @param target_str            Pointer to the String where the target string is copied to.
 * @param target_str_len        Maximum Length of the target string.
 * @param extract_tag_begin     Sign that marks the beginning of the substring which should be copied.
 * @param extract_tag_end       Sign that marks the end of the substring which should be copied.
 * @return                      Length of the String without the spaces.
 **/
uint16_t string_extract_between(char *str, char *target_str, uint16_t target_str_len,
                                char extract_tag_begin, char extract_tag_end);
/**
 *  Deletes the spaces and tabs at the beginning and at the end of a given String.
 *
 * @param str                   Pointer to the string.
 * @return                      Length of the string without the spaces.
 **/
uint16_t string_trim(char *str);

/**
 *  Checks if a given string contains only decimal or hexadecimal numbers.
 *
 * @param str                   Pointer to the string.
 * @param str_len               Length of the string.
 * @param is_hexa               true: Numbers 0-9 and Letters A-F and a-f are allowed inside the string.
 *                              false: Only numbers 0-9 are allowed.
 * @return                      true if it is a valid decimal or hecadecimal number according to is_hexa.
 **/
bool string_is_decimal(const char *str, uint16_t str_len, bool is_hexa);
#endif

/**
 * Counts the occurences of a char inside a string
 * @param str		Pointer to the string
 * @param c			Character to find
 * @return			Number of occurences of c in str
 */
uint32_t string_count_char(const char* str, char c);

/**
 *  Sets the hex characters to upper or lower for the functions of this modules that convert integers to hex strings.
 *
 * @param use_upper             true: Following hex strings will contain upper letters.
 *                              false: Following hex strings will contain lower letters.
 */
void string_set_hex_letter_size(bool use_upper);

/**
 *  Returns ASCII letter corresponding to the given integer value.
 *
 * @param value                 Integer Value 0 - 15.
 * @return                      Returns 0 if the integer value is above 15 so that no valid hexadecimal character can be returned.
 *                              Otherwise an upper character 0 - F is returned.
 **/
char string_uint8_to_ascii(uint8_t value);

/**
 *  Returns an integer value to the given ASCII letter.
 *
 * @param letter                ASCII character 0 - F.
 * @return                      Returns 0 if the ASCII value is no numeric character and no character A-F(a-f).
 *                              Otherwise it returns the corresponding integer value 0 - 15.
 **/
uint8_t string_ascii_to_uint8(char letter);

/**
 * Parses an hex string into a byte array.
 * @param str					ASCII character 0 - F.
 * @param v						Pointer to the array where the value shall be stored.
 * @param v_len					Maximum length of buffer v.
 * @return						true if str has the correct len.
 */
bool string_parse_hex_array(const char* str, uint8_t* v, uint16_t v_len);

/**
 * Returns the lower case character of a letter
 * @param letter        Letter
 * @return              Lower case of the letter
 */
char string_to_lower(char letter);

/**
 * Returns the upper case character of a letter
 * @param letter        Letter
 * @return              Upper case of the letter
 */
char string_to_upper(char letter);

/**
 *  Creates an integer string from an an unsigned integer 32-bit value.
 *
 * @param str                   Pointer to a buffer where the string will be written to.
 * @param val                   32-Bit integer that needs to be converted to a string.
 * @param base                  Base of the number. Currently supported are 10 for decimal and 16 for hex.
 * @param min_letters           Minimum number of characters that must be shown. If min_letters is 0, the full number will be shown.
 * @param add_leading_zero      true: Leading 0 will be added if the number has less characters then min_letters.
 *                              false: Spaces will be added if the number has less characters then min_letters.
 * @return                      Pointer to the address of the terminating zero.
 **/
char* string_create_uint_string(char* str, uint32_t val, uint8_t base, uint8_t min_letters, bool add_leading_zero);

/**
 *  Creates an integer string from an an integer 32-bit value.
 *
 * @param str                   Pointer to a buffer where the string will be written to.
 * @param val                   32-Bit integer that needs to be converted to a string.
 * @param base                  Base of the number. Currently supported are 10 for decimal and 16 for hex.
 * @param min_letters           Minimum number of characters that must be shown. If min_letters is 0, the full number will be shown.
 * @param add_leading_zero      true: Leading 0 will be added if the number has less characters then min_letters.
 *                              false: Spaces will be added if the number has less characters then min_letters.
 * @return                      Pointer to the address of the terminating zero.
 **/
char* string_create_int_string(char* str, int32_t val, uint8_t base, uint8_t min_letters, bool add_leading_zero);

/**
 *  Creates an integer string from an an unsigned integer 64-bit value.
 *
 * @param str                   Pointer to a buffer where the string will be written to.
 * @param val                   64-Bit integer that needs to be converted to a string.
 * @param base                  Base of the number. Currently supported are 10 for decimal and 16 for hex.
 * @param min_letters           Minimum number of characters that must be shown. If min_letters is 0, the full number will be shown.
 * @param add_leading_zero      true: Leading 0 will be added if the number has less characters then min_letters.
 *                              false: Spaces will be added if the number has less characters then min_letters.
 * @return                      Pointer to the address of the terminating zero.
 **/
char* string_create_uint64_string(char* str, uint64_t val, uint8_t base, uint8_t min_letters, bool add_leading_zero);

/**
 *  Creates an integer string from an an integer 64-bit value.
 *
 * @param str                   Pointer to a buffer where the string will be written to.
 * @param val                   64-Bit integer that needs to be converted to a string.
 * @param base                  Base of the number. Currently supported are 10 for decimal and 16 for hex.
 * @param min_letters           Minimum number of characters that must be shown. If min_letters is 0, the full number will be shown.
 * @param add_leading_zero      true: Leading 0 will be added if the number has less characters then min_letters.
 *                              false: Spaces will be added if the number has less characters then min_letters.
 * @return                      Pointer to the address of the terminating zero.
 **/
char* string_create_int64_string(char* str, int64_t val, uint8_t base, uint8_t min_letters, bool add_leading_zero);

#if !STRING_USE_COMM_MINIMUM

/**
 * Converts a given number to a number string including a separator for thousand, million and billion.
 * The separator is a dot, not a comma. Make sure that the target string has enough space for the number.
 * Puts a terminating zero behind the last digit.
 *
 * @param str_buf               Pointer to the string where the number should be inserted.
 * @param num                   Number that should be converted to the string.
 * @param with_thousand_seperator Indicates wether a thousand seperator should be inserted
 * @return                      Pointer to the address of the terminating zero.
 */
char* string_create_num_string(char *str_buf, int32_t num, bool with_thousand_seperator);
#endif

/**
 * Checks if str ends with end_str.
 *
 * @param str       Pointer to the 0-terminated string that needs to be checked.
 * @param end_str   Pointer to the 0-terminated string that is checked to be at the end of str.
 * @return          true: end_str is the end of str.
 *                  false: end_str is not the end of str or one of the pointers is NULL.
 */
bool string_ends_with(char* str, char* end_str);

/**
 * Compare S1 and S2, ignoring case, returning less than, equal to or
   greater than zero if S1 is lexicographically less than,
   equal to or greater than S2.

 * @param s1        Pointer to S1
 * @param s2        Pointer to S2
 * @return          Negative value (S1 < S2), 0 (S1 = S2) or Positive value (S1 > S2)
 */
int string_strcasecmp(const char *s1, const char* s2);
/**
 * Compare S1 and S2, ignoring case, returning less than, equal to or
   greater than zero if S1 is lexicographically less than,
   equal to or greater than S2.

 * @param s1        Pointer to S1
 * @param s2        Pointer to S2
 * @param len       Maximum number of bytes that are compared between S1 and S2
 * @return          Negative value (S1 < S2), 0 (S1 = S2) or Positive value (S1 > S2)
 */
int string_strncasecmp(const char *s1, const char* s2, uint16_t len);
/**
 * Find the first occurrence of find in s, ignore case.
 *
 * @param s         Pointer to the string to search in
 * @param find      String that is searched in s
 * @return          Pointer to first occurrence of find in s or NULL if find is not part of s.
 */
char* string_strcasestr(const char *s, const char *find);
/**
 * Searches for needle in the haystack.
 *
 * @param haystack  String where needle should be found.
 * @param needle    String that is searched for in haystack.
 * @return          NULL if needle is not finde inside haystack. If needle is found in haystack, the return value points to the character behind needle.
 */
char* string_strstr_end(const char* haystack, const char* needle);
/**
 * Checks whether the string is a valid array consisting of numbers.
 *  The array is valid if there are only whitespaces, linefeeds, carriage returns, commas and numbers (hex/dex).
 *  Empty entries of an element (e.g. ", , ") are seen as invalid.
 *  Spaces during a number "1, 3 4, 5" are seen as invalid.
 *  - is only allowed at the beginning of a number, otherwise it is seen as invalid.
 *  If minimum/maximum are set, the number value is compared to minimum/maximum to check whether the value is in range.
 *  The values for minimum and maximum are inclusive and are compared as strings.
 *
 * @param[in] str           Pointer to the start of the array. The function expects a 0-terminated string.
 * @param[in] min           String containing the minimum number value. Must be a valid string or NULL. If NULL the minimum is not checked.
 * @param[in] max           String containing the maximum number value. Must be a valid string or NULL. If NULL the maximum is not checked.
 * @param[in] max_entries   Maximum number of elements allowed for the array.
 * @param[in] is_hex        Set to true if hex values are used inside the string array.
 * @return              true if the array is valid, false if not.
 */
bool string_is_valid_num_array(const char* str, char* min, char* max, uint16_t max_entries, bool is_hex);
/**
 * Reads a value from a number array.
 * Sample:
 * @code
const char* str = "1, 2, 3, 4";
if(string_is_valid_num_array(str, "-100", "9999", 6, false))
{
    int32_t v;
    uint16_t j = 0;
    char* ptr = str;
    printf("\"%s\" -> OK\n", str);
    while(string_read_next_num_array_value(ptr, &ptr, &v, false))
    {
        printf("[%d] = %d\n", j, v);
        j++;
    }
}
 * @endcode
 *
 * @pre Use @see string_is_valid_num_array to check if the string is a valid array that can be used in this function!
 *
 * @param[in]  str           Pointer to the start of an array.
 * @param[out] ptr           Pointer where the end of the current value is written into. Can be used as str in next call.
 * @param[out] v             Pointer to the value that is filled, when this function returns true.
 * @param[in] is_hex         Set to true if hex values are used inside the string array.
 * @param[in] is_unsigned    Set to true if unsigned values are used inside the string array.
 * @return              True if a number value was converted and written into the value. False if no value was found (e.g. end of array).
 */
bool string_read_next_num_array_value(const char* str, char** ptr, int32_t* v, bool is_hex, bool is_unsigned);
/**
 * Converts a string array containing numbers into an integer array.
 *
 * @param[in]  str          Pointer to the start of an array.
 * @param[out] arr          Pointer to the array where the converted number shall be stored.
 * @param[in] max_entries   Maximum number of elements inside the array.
 * @param[in] is_hex        Set to true if hex values are used inside the string array.
 * @param[out] entries      Pointer to an integer where the number of converted elements is stored.
 * @param[in] is_unsigned   Set to true if the values shall be interpreted as unsigned integers
 * @return                  True if an array was converted, false if str is NULL, arr is NULL or no elements were found.
 */
bool string_read_num_array(const char* str, int32_t* arr, uint16_t max_entries, bool is_hex, uint16_t* entries, bool is_unsigned);

/**
 * Converts an array of quoted strings into an array of strings.
 * @param str				Pointer to the start of a string array. The strings have to be in quotes and comma separated. E.g. "ABC","DEF","GHI". Whitespaces and linefeed between
 * 							the Strings are tolerated.
 * @param entries			Pointer to the array where the converted strings shall be stored. Be careful: The target array should have space for storing the strings.
 * @param entry_len			Maximum number of bytes for a single string inside the array.
 * @param entry_num			Maximum number of elements for the array.
 * @return                  If array was converted or empty the number of entries put into the target are returned. If the input is invalid, a negative value is returned.
 */
int string_read_string_array(const char* str, char** entries, int entry_len, int entry_num);
/**
 * Tries to parse a string into an array for an IPv4 address.
 *
 * @param[in] str           Pointer to the start of the IP address.
 * @param[out] ip           Pointer to the buffer (4 byte), where the converted IP is stored. If NULL, the function only verifies the IP string.
 * @return                  True if parsed successfully. False if parsing failed -> ip is not changed.
 */
bool string_parse_ipv4(const char* str, uint8_t* ip);
/**
 * Replaces all instances of orig in str by rep.
 * @attention The string must be null-terminated
 * @param str String where chars are to be replaced
 * @param orig The character that shall be removed
 * @param rep The character that shall be substituted
 * @return The number of replacements that were made
 */
int string_replace(char *str, char orig, char rep) ;
#if MODULE_ENABLE_RTC
/**
 * Tries to parse a string as iso8601 time format (e.g. 2019-11-11T15:55:03.697Z) into the rtc time structure.
 * @param[in] str           String containing time in javascript format.
 * @param[out] t            Pointer to the time structure where the parsed time is saved
 * @param[in] timezone      If this is not NULL, the timezone value will be set as the offset of UTC. On UTC this will be 0.
 * @retval true             Time was parsed successfully and can be used.
 * @retval false            Time cannot be parsed, do not use values stored in t.
 */
bool string_parse_datetime_js(const char* str, rtc_time_t* t, int8_t* timezone);
/**
 * Creates a string of the time based on the rtc time structure in a javascript format.
 * @param[out] str          String containing time in javascript format.
 * @param max_len           Maximum length of the string buffer
 * @param t                 Pointer to the structure containing the time.
 * @param timezone          Timezone offset, 0 means UTC time
 * @param add_fractals      If true, millisecond fractals will be added after the seconds.
 * @retval true             String was created and can be used.
 * @retval false            Failed in creating the string (e.g. not enough space).
 */
bool string_create_datetime_js(char* str, uint16_t max_len, rtc_time_t* t, int8_t timezone, bool add_fractals);
/**
 * Creates a string of the time based on the rtc time structure in the format used in OCMF.
 * @param[out] str          String containing time in OCMF format.
 * @param max_len           Maximum length of the string buffer
 * @param t                 Pointer to the structure containing the time.
 * @param timezone          Timezone offset, 0 means UTC time
 * @retval true             String was created and can be used.
 * @retval false            Failed in creating the string (e.g. not enough space).
 */
bool string_create_datetime_ocmf(char* str, uint16_t max_len, rtc_time_t* t, int8_t timezone);
#endif
/**
 * Checks if the given string represents a valid boolean value. Is case insensitive
 * @param[in] str The string to check
 * @param[out] bool_value Pointer to a bool variable where the result is written to if it is valid
 * @return FUNCTION_RETURN_OK if a valid boolean value was found, FUNCTION_RETURN_PARAM_ERROR if there was no valid boolean value found
 */
FUNCTION_RETURN string_parse_bool(const char* str, bool* bool_value);

/**
 * @brief Copies the string from a given source to the destination removing escape sequences
 * 
 * @param[in] src                               The original string containing escape sequences
 * @param src_len                               Length of the original
 * @param[out] dest                             A copy of the original string with the escape sequences replaced
 * @param dest_len                              Maximum length of the destination buffer
 * @retval FUNCTION_RETURN_PARAM_ERROR          A null pointer was given for src or dest
 * @retval FUNCTION_RETURN_INSUFFICIENT_MEMORY  The source string was truncated, because it didn't fit the destination buffer
 * @retval FUNCTION_RETURN_OK                   All of the source string was copied to the destination with the escape sequences replaced
 */
FUNCTION_RETURN string_unescape(const char* src, size_t src_len, char* dest, size_t dest_len);
#endif

#endif
