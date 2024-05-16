#include <gtest/gtest.h>
#include "gmock/gmock.h"

extern "C"
{
    #include "module/convert/string.h"
    #include <limits.h>
    void app_main_init(void)
    {

    }

    void board_init(void)
    {
        
    }
}

char convert_string_test_result_string[20];
char zero_length_string[2] = "\0";

TEST(convert_string, thousand_separator)
{
    char sep = string_get_thousand_separator_character();
    EXPECT_EQ(sep, '.');

    string_set_thousand_separator_character(',');
    sep = string_get_thousand_separator_character();
    EXPECT_EQ(sep, ',');

    string_create_num_string(convert_string_test_result_string, 10000, true);
    EXPECT_STREQ(convert_string_test_result_string, "10,000");

    string_set_thousand_separator_character('.');
    sep = string_get_thousand_separator_character();
    EXPECT_EQ(sep, '.');
}

TEST(convert_string, decimal_point)
{
    char dec_point = string_get_decimal_point_character();
    EXPECT_EQ(dec_point, ',');

    string_set_decimal_point_character('.');
    dec_point = string_get_decimal_point_character();
    EXPECT_EQ(dec_point, '.');

    string_printf(convert_string_test_result_string, "%m", 150);
    EXPECT_STREQ(convert_string_test_result_string, "1.50");

    string_set_decimal_point_character(',');
    dec_point = string_get_decimal_point_character();
    EXPECT_EQ(dec_point, ',');
}

TEST(convert_string, string_printf)
{
    char result_string[65] = { 0 };
    uint8_t test_array[10] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF1, 0x23, 0x45};
    char test_string[] = "ESoPeTest";

    EXPECT_EQ(string_printf(result_string, "%c %u%3u %03u", 'A', 10, 10, 10), 11);
    EXPECT_STREQ(result_string, "A 10 10 010");

    EXPECT_EQ(string_printf(result_string, "%i %d%3i %03i", -1, -1, -1, -1), 12);
    EXPECT_STREQ(result_string, "-1 -1 -1 -01");

    EXPECT_EQ(string_printf(result_string, "%U %I", LLONG_MIN, LLONG_MIN), 40);
    EXPECT_STREQ(result_string, "9223372036854775808 -9223372036854775808");

    EXPECT_EQ(string_printf(result_string, "%M %m", 1000*100, 1000*100), 16);
    EXPECT_STREQ(result_string, "1000,00 1.000,00");

    EXPECT_EQ(string_printf(result_string, "%h %02x %3X", 0xA, 0xA, 0xA), 8);
    EXPECT_STREQ(result_string, "a 0a   A");

    EXPECT_EQ(string_printf(result_string, "%b %08b %B", true, true, true), 15);
    EXPECT_STREQ(result_string, "1 00000001 true");

    EXPECT_EQ(string_printf(result_string, "%d %$q %#Q", sizeof(test_array), test_array, sizeof(test_array), test_array), 44);
    EXPECT_STREQ(result_string, "10 123456789abcdef12345 123456789ABCDEF12345");

    EXPECT_EQ(string_printf(result_string, "%d %$a %#A", sizeof(test_array), test_array, sizeof(test_array), test_array), 62);
    EXPECT_STREQ(result_string, "10 12 34 56 78 9a bc de f1 23 45 12 34 56 78 9A BC DE F1 23 45");

    EXPECT_EQ(string_printf(result_string, "%s", test_string), 9);
    EXPECT_STREQ(result_string, "ESoPeTest");

    EXPECT_EQ(string_printf(result_string, "%15s", test_string), 15);
    EXPECT_STREQ(result_string, "ESoPeTest      ");

    EXPECT_EQ(string_printf(result_string, "%.15s", test_string), 15);
    EXPECT_STREQ(result_string, "      ESoPeTest");

    EXPECT_EQ(string_printf(result_string, "%.4s", test_string), 4);
    EXPECT_STREQ(result_string, "ESoP");
}

TEST(convert_string, find_first_int)
{
    char test[] = "test1test2";
    EXPECT_EQ(string_find_first_int(test), 4);
    test[4] = 't';
    EXPECT_EQ(string_find_first_int(test), 9);
    test[9] = 't';
    EXPECT_EQ(string_find_first_int(test), -1);
}

TEST(convert_string, index_of_substring)
{
    char test_string[] = "testABCrestDEF";
    char find[] = "ABC";
    EXPECT_EQ(string_index_of_substring(test_string, find), 4);
    EXPECT_EQ(string_index_of_substring(find, test_string), -1);
}

TEST(convert_string, extract_between)
{
    char test_string[] = "abcdefg<test>hijklmnop[test2]";
    char target[5];
    
    EXPECT_EQ(string_extract_between(test_string, target, 5, '<', '>'), 4);
    EXPECT_STREQ(target, "test");
    EXPECT_EQ(string_extract_between(test_string, target, 5, '[', ']'), 5);
    EXPECT_EQ(target[4], '2');
    EXPECT_EQ(string_extract_between(test_string, target, 5, '(', ']'), 0);
}

TEST(convert_string, trim)
{
    char test[] = "\t  Test String\tTest      ";
    EXPECT_EQ(strlen(test), 25);
    EXPECT_EQ(string_trim(test), 16);
    EXPECT_EQ(strlen(test), 16);
}

TEST(convert_string, is_decimal)
{
    char test_string[] = "123F";
    EXPECT_TRUE(string_is_decimal(test_string, 3, false));
    EXPECT_FALSE(string_is_decimal(test_string, 4, false));
    EXPECT_TRUE(string_is_decimal(test_string, 4, true));
}

TEST(convert_string, count_char)
{
    char test[] = "test1test2test3";
    EXPECT_EQ(string_count_char(test, '1'), 1);
    EXPECT_EQ(string_count_char(test, 't'), 6);
    EXPECT_EQ(string_count_char(test, 'Q'), 0);
}

TEST(convert_string, convert_to_and_from_byte)
{
    EXPECT_EQ(string_uint8_to_ascii(9), '9');
    EXPECT_EQ(string_uint8_to_ascii(11), 'B');
    string_set_hex_letter_size(false);
    EXPECT_EQ(string_uint8_to_ascii(11), 'b');
    string_set_hex_letter_size(true);
    EXPECT_EQ(string_ascii_to_uint8('F'), 15);
    EXPECT_EQ(string_ascii_to_uint8('f'), 15);
    EXPECT_EQ(string_ascii_to_uint8('7'), 7);    
    EXPECT_EQ(string_ascii_to_uint8('0'), 0);    
    EXPECT_EQ(string_ascii_to_uint8('X'), '!');    
    EXPECT_EQ(string_ascii_to_uint8('\n'), 0);
}

TEST(convert_string, parse_hex_array)
{
    uint8_t target_array[5];
    char test_string[] = "\0\0""1ABCDEF2";
    uint8_t expected_result[] = { 0x00, 0x1A, 0xBC, 0xDE, 0xF2 };

    EXPECT_FALSE(string_parse_hex_array(NULL, target_array, 5));
    EXPECT_FALSE(string_parse_hex_array(test_string, NULL, 5));
    EXPECT_FALSE(string_parse_hex_array(test_string, target_array, 0));
    EXPECT_FALSE(string_parse_hex_array(test_string, target_array, 5));
    test_string[0] = '0';
    EXPECT_FALSE(string_parse_hex_array(test_string, target_array, 5));
    test_string[1] = '0';
    EXPECT_TRUE(string_parse_hex_array(test_string, target_array, 5));
    EXPECT_EQ(memcmp(target_array, expected_result, sizeof(target_array)), 0);
}

TEST(convert_string, case_conversions)
{
    EXPECT_EQ(string_to_lower('A'), 'a');
    EXPECT_EQ(string_to_lower('('), '(');
    EXPECT_EQ(string_to_upper('a'), 'A');
    EXPECT_EQ(string_to_upper('2'), '2');
}

TEST(convert_string, num_strings)
{
    char result_string[65];

    EXPECT_EQ(string_create_num_string(NULL, 0, false), nullptr);

    string_create_num_string(result_string, INT_MIN, false);
    EXPECT_STREQ(result_string, "-2147483648");

    string_create_num_string(result_string, INT_MIN, true);
    EXPECT_STREQ(result_string, "-2.147.483.648");

    string_create_num_string(result_string, INT_MAX, false);
    EXPECT_STREQ(result_string, "2147483647");

    string_create_num_string(result_string, INT_MAX, true);
    EXPECT_STREQ(result_string, "2.147.483.647");

    string_create_uint_string(result_string, 10, 10, 0, false);
    EXPECT_STREQ(result_string, "10");
    string_create_uint_string(result_string, 10, 10, 10, false);
    EXPECT_STREQ(result_string, "        10");
    string_create_uint_string(result_string, 10, 10, 10, true);
    EXPECT_STREQ(result_string, "0000000010");

    string_create_int_string(result_string, INT_MIN, 10, 0, false);
    EXPECT_STREQ(result_string, "-2147483648");
    string_create_uint_string(result_string, INT_MIN, 10, 0, false);
    EXPECT_STREQ(result_string, "2147483648");
    string_create_uint_string(result_string, 2147483647, 16, 0, false);
    EXPECT_STREQ(result_string, "7FFFFFFF");
    string_create_uint_string(result_string, 2147483647, 2, 32, true);
    EXPECT_STREQ(result_string, "01111111111111111111111111111111");

    string_create_uint64_string(result_string, 10, 10, 0, false);
    EXPECT_STREQ(result_string, "10");
    string_create_uint64_string(result_string, 10, 10, 10, false);
    EXPECT_STREQ(result_string, "        10");
    string_create_uint64_string(result_string, 10, 10, 10, true);
    EXPECT_STREQ(result_string, "0000000010");
    
    string_create_int64_string(result_string, LLONG_MIN, 10, 0, false);
    EXPECT_STREQ(result_string, "-9223372036854775808");
    string_create_uint64_string(result_string, LLONG_MIN, 10, 0, false);
    EXPECT_STREQ(result_string, "9223372036854775808");
    string_create_uint64_string(result_string, 9223372036854775807, 16, 0, false);
    EXPECT_STREQ(result_string, "7FFFFFFFFFFFFFFF");
    string_create_uint64_string(result_string, 9223372036854775807, 2, 64, true);
    EXPECT_STREQ(result_string, "0111111111111111111111111111111111111111111111111111111111111111");
}

TEST(convert_string, ends_with)
{
    char str[] = "TeststringESoPe";
    char end_str[] = "ESoPu";

    EXPECT_FALSE(string_ends_with(NULL, NULL));
    EXPECT_FALSE(string_ends_with(NULL, end_str));
    EXPECT_FALSE(string_ends_with(str, NULL));
    EXPECT_FALSE(string_ends_with(zero_length_string, end_str));
    EXPECT_FALSE(string_ends_with(str, zero_length_string));
    EXPECT_FALSE(string_ends_with(end_str, str));
    EXPECT_FALSE(string_ends_with(str, end_str));
    end_str[4] = 'e';
    EXPECT_TRUE(string_ends_with(str, end_str));
}

TEST(convert_string, casecmp)
{
    char string_one[10] = "aBcDeFgHi";
    char string_two[11] = "AbCdEfGhJj";

    EXPECT_EQ(string_strcasecmp(NULL, NULL), 0);
    EXPECT_EQ(string_strcasecmp(NULL, string_two), -1);
    EXPECT_EQ(string_strcasecmp(string_one, NULL), 1);
    EXPECT_EQ(string_strcasecmp(string_one, string_two), -1);
    EXPECT_EQ(string_strcasecmp(string_two, string_one), 1);

    string_two[9] = 0;
    EXPECT_EQ(string_strcasecmp(string_one, string_two), -1);
    string_two[8] = 'h';
    EXPECT_EQ(string_strcasecmp(string_one, string_two), 1);
    string_two[8] = 'I';
    EXPECT_EQ(string_strcasecmp(string_one, string_two), 0);
}

TEST(convert_string, ncasecmp)
{
    char string_one[10] = "aBcDeFgHi";
    char string_two[11] = "AbCdEfGhJj";

    EXPECT_EQ(string_strncasecmp(string_one, string_two, 0), 0);

    EXPECT_EQ(string_strncasecmp(NULL, NULL, strlen(string_two)), 0);
    EXPECT_EQ(string_strncasecmp(NULL, string_two, strlen(string_two)), -1);
    EXPECT_EQ(string_strncasecmp(string_one, NULL, strlen(string_one)), 1);
    EXPECT_EQ(string_strncasecmp(string_one, string_two, strlen(string_one)), -1);
    EXPECT_EQ(string_strncasecmp(string_two, string_one, strlen(string_two)), 1);

    string_two[8] = 'h';
    EXPECT_EQ(string_strncasecmp(string_one, string_two, strlen(string_one)), 1);
    string_two[8] = 'I';
    EXPECT_EQ(string_strncasecmp(string_one, string_two, strlen(string_two)), -1);
    EXPECT_EQ(string_strncasecmp(string_one, string_two, strlen(string_one)), 0);
}

TEST(convert_string, casestr)
{
    char test_string[] = "000aBcD111EfGh222";
    char find_valid[] = "AbCd";
    char find_valid_two[] = "eFgH";
    char find_invalid[] = "jKlM";

    EXPECT_EQ(string_strcasestr(NULL, find_valid), nullptr);
    EXPECT_EQ(string_strcasestr(test_string, NULL), nullptr);
    EXPECT_EQ(string_strcasestr(test_string, zero_length_string), nullptr);

    EXPECT_EQ(string_strcasestr(test_string, find_valid), &test_string[3]);
    EXPECT_EQ(string_strcasestr(test_string, find_valid_two), &test_string[10]);
    EXPECT_EQ(string_strcasestr(test_string, find_invalid), nullptr);
}

TEST(convert_string, strstr_end)
{
    char test_string[] = "000abcd111efgh222";
    char find_valid[] = "abcd";
    char find_valid_two[] = "efgh";
    char find_invalid[] = "jklm";

    EXPECT_EQ(string_strstr_end(NULL, find_valid), nullptr);
    EXPECT_EQ(string_strstr_end(test_string, NULL), nullptr);
    EXPECT_EQ(string_strstr_end(test_string, zero_length_string), nullptr);

    EXPECT_EQ(string_strstr_end(test_string, find_valid), &test_string[7]);
    EXPECT_EQ(string_strstr_end(test_string, find_valid_two), &test_string[14]);
    EXPECT_EQ(string_strstr_end(test_string, "222"), &test_string[17]);
    EXPECT_EQ(string_strstr_end(test_string, find_invalid), nullptr);
}

TEST(convert_string, num_array)
{
    int32_t result_array[5] = { 0 };
    int32_t expected_signed[5] = { -1, -2, -3, -4, -5};

    uint32_t unsigned_result_array[5] = { 0 };
    uint32_t expected_unsigned[5] = { 0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFFD, 0xFFFFFFFC, 0xFFFFFFFB};
    uint16_t entries;

    /// Is valid num array
    EXPECT_FALSE(string_is_valid_num_array("", (char*)"", (char*)"", 5, false));
    EXPECT_FALSE(string_is_valid_num_array(",", (char*)"0", (char*)"100", 5, false));
    EXPECT_FALSE(string_is_valid_num_array(", 1", (char*)"0", (char*)"100", 5, false));
    EXPECT_FALSE(string_is_valid_num_array("1 1", (char*)"0", (char*)"100", 5, false));
    EXPECT_FALSE(string_is_valid_num_array("1, 2 3", (char*)"0", (char*)"100", 5, false));
    
    EXPECT_FALSE(string_is_valid_num_array("1", (char*)"2", (char*)"100", 5, false));
    EXPECT_FALSE(string_is_valid_num_array("-150", (char*)"-100", (char*)"100", 5, false));
    EXPECT_FALSE(string_is_valid_num_array("150", (char*)"-150", (char*)"100", 5, false));

    EXPECT_TRUE(string_is_valid_num_array("1", (char*)"0", (char*)"100", 5, false));
    EXPECT_TRUE(string_is_valid_num_array("1,", (char*)"0", (char*)"100", 5, false));
    EXPECT_TRUE(string_is_valid_num_array("1,\n2", (char*)"0", (char*)"100", 5, false));
    EXPECT_TRUE(string_is_valid_num_array("1,\n2,\t3", (char*)"0", (char*)"100", 5, false));
    EXPECT_TRUE(string_is_valid_num_array("1,\n2,\t3,  4", (char*)"0", (char*)"100", 5, false));
    EXPECT_TRUE(string_is_valid_num_array("1,\n2,\t3,  4,5", (char*)"0", (char*)"100", 5, false));

    EXPECT_FALSE(string_is_valid_num_array("1,\n2,\t3,  4,5,6", (char*)"0", (char*)"100", 5, false));
    EXPECT_FALSE(string_is_valid_num_array("1A,2B,3C,4D,5E,6F", (char*)"0", (char*)"100", 6, false));
    
    EXPECT_TRUE(string_is_valid_num_array("1A,2B,3C,4D,5E,6F", (char*)"0", (char*)"100", 6, true));
    
    // read num array

    EXPECT_FALSE(string_read_num_array(NULL, NULL, 5, false, NULL, false));
    EXPECT_FALSE(string_read_num_array("-1", NULL, 5, false, NULL, false));
    EXPECT_FALSE(string_read_num_array(NULL, result_array, 5, false, NULL, false));

    EXPECT_TRUE(string_read_num_array("-1", result_array, 5, false, NULL, false));
    EXPECT_TRUE(string_read_num_array("-1", result_array, 5, false, &entries, false));
    EXPECT_EQ(entries, 1);
    EXPECT_EQ(result_array[0], -1);

    EXPECT_TRUE(string_read_num_array("-1", (int32_t*)unsigned_result_array, 5, false, &entries, true));
    EXPECT_EQ(entries, 1);
    EXPECT_EQ(unsigned_result_array[0], 4294967295);

    EXPECT_TRUE(string_read_num_array("-1, -2, -3, -4, -5, -6", result_array, 5, false, &entries, false));
    EXPECT_EQ(entries, 5);
    for(int i = 0; i < entries; i++)
    {
        EXPECT_EQ(result_array[i], expected_signed[i]) << "Difference at index " << i << " expected " << expected_signed[i] << ", got " << result_array[i];
    }

    EXPECT_TRUE(string_read_num_array("-1, -2, -3, -4, -5, -6", (int32_t*) unsigned_result_array, 3, false, &entries, true));
    EXPECT_EQ(entries, 3);
    for(int i = 0; i < entries; i++)
    {
        EXPECT_EQ(unsigned_result_array[i], expected_unsigned[i]) << "Difference at index " << i << " expected " << expected_signed[i] << ", got " << result_array[i];
    }

    EXPECT_TRUE(string_read_num_array("-1, -2, -3, -4, -5, -6", (int32_t*) unsigned_result_array, 5, false, &entries, true));
    EXPECT_EQ(entries, 5);
    for(int i = 0; i < entries; i++)
    {
        EXPECT_EQ(unsigned_result_array[i], expected_unsigned[i]) << "Difference at index " << i << " expected " << expected_signed[i] << ", got " << result_array[i];
    }
}

TEST(convert_string, string_array)
{
    char test_array[3][10];
    //Invalid Pointers -> Empty result
    EXPECT_EQ(string_read_string_array(NULL, (char**)test_array, 10, 3), 0);
    EXPECT_EQ(string_read_string_array("Test", NULL, 10, 3), 0);
    //Expecting 0 strings -> empty string okay
    EXPECT_EQ(string_read_string_array("", (char**)test_array, 10, 0), 0);
    //TODO: Is this intended? Should zero elements even be allowed?
    EXPECT_EQ(string_read_string_array(" ", (char**)test_array, 10, 0), -1);
    
    //Expecting quote -> Invalid
    EXPECT_EQ(string_read_string_array("Test", (char**)test_array, 0, 3), -1);
    // Valid -> empty
    EXPECT_EQ(string_read_string_array(" ", (char**)test_array, 10, 3), 0);
    //Expecting quote
    EXPECT_EQ(string_read_string_array("\"", (char**)test_array, 10, 3), -1);
    // Too long
    EXPECT_EQ(string_read_string_array("\"1234567890\"", (char**)test_array, 10, 3), -1);
    // Valid
    EXPECT_EQ(string_read_string_array("\"123456789\"", (char**)test_array, 10, 3), 1);
    EXPECT_STREQ(test_array[0], "123456789");

    // Expecting comma
    EXPECT_EQ(string_read_string_array("\"123456789\"\"ABCDEFGHJ\"", (char**)test_array, 10, 3), -1);
    //Two elements
    EXPECT_EQ(string_read_string_array("\"123456789\",\"ABCDEFGHJ\"", (char**)test_array, 10, 3), 2);
    EXPECT_STREQ(test_array[0], "123456789");
    EXPECT_STREQ(test_array[1], "ABCDEFGHJ");
    //Three elements
    EXPECT_EQ(string_read_string_array("\"123456789\",\"ABCDEFGHJ\",\"KLMNOPQRS\"", (char**)test_array, 10, 3), 3);
    EXPECT_STREQ(test_array[0], "123456789");
    EXPECT_STREQ(test_array[1], "ABCDEFGHJ");
    EXPECT_STREQ(test_array[2], "KLMNOPQRS");
    // Too many elements
    EXPECT_EQ(string_read_string_array("\"123456789\",\"ABCDEFGHJ\",\"KLMNOPQRS\",\"TUVWXYZ\"", (char**)test_array, 10, 3), -1);    
}

TEST(convert_string, parse_ipv4)
{
    uint8_t ip[4] = { 0 };
    uint8_t expected[4] = { 192, 168, 178, 1 };

    EXPECT_FALSE(string_parse_ipv4(NULL, ip));

    EXPECT_FALSE(string_parse_ipv4("a92.168.168.1", ip));
    EXPECT_FALSE(string_parse_ipv4("192.1c8.168.1", ip));
    EXPECT_FALSE(string_parse_ipv4("1.1.1.1.", ip));
    EXPECT_FALSE(string_parse_ipv4("1.-1.1.1", ip));
    EXPECT_FALSE(string_parse_ipv4("1.1.1.1000", ip));
    EXPECT_FALSE(string_parse_ipv4("1.1.1.100 ", ip));
    
    EXPECT_TRUE(string_parse_ipv4("192.168.178.1", ip));
    EXPECT_EQ(memcmp(ip, expected, sizeof(ip)), 0);
    EXPECT_TRUE(string_parse_ipv4("127.0.0.1", NULL));
    EXPECT_EQ(memcmp(ip, expected, sizeof(ip)), 0);
}

TEST(convert_string, replace)
{
    char test_string[] = "qabcabcabcabd";

    EXPECT_EQ(string_replace(NULL, 'x', 'y'), 0);
    EXPECT_EQ(string_replace(test_string, 'x', 'y'), 0);

    EXPECT_EQ(string_replace(test_string, 'd', 'c'), 1);
    EXPECT_STREQ(test_string, "qabcabcabcabc");

    EXPECT_EQ(string_replace(test_string, 'a', 'b'), 4);
    EXPECT_STREQ(test_string, "qbbcbbcbbcbbc");

    EXPECT_EQ(string_replace(test_string, 'b', 'c'), 8);
    EXPECT_STREQ(test_string, "qcccccccccccc");

    EXPECT_EQ(string_replace(test_string, 'c', '\0'), 12);
    EXPECT_STREQ(test_string, "q");
}

TEST(convert_string, parse_bool)
{
    bool result;
    EXPECT_EQ(string_parse_bool(NULL, &result), FUNCTION_RETURN_PARAM_ERROR);
    EXPECT_EQ(string_parse_bool("test", &result), FUNCTION_RETURN_PARAM_ERROR);
    
    EXPECT_EQ(string_parse_bool("true", NULL), FUNCTION_RETURN_OK);

    EXPECT_EQ(string_parse_bool("TRUE", &result), FUNCTION_RETURN_OK);
    EXPECT_TRUE(result);
    EXPECT_EQ(string_parse_bool("fAlsE", &result), FUNCTION_RETURN_OK);
    EXPECT_FALSE(result);
}

TEST(convert_string, unescape)
{
    char test_string[] = "\\a\\b\\f\\v\\n\\r\\t\\\"\\\\\\/";
    char result_string[11] = { 0 };

    EXPECT_EQ(string_unescape(NULL, strlen(test_string), result_string, 10), FUNCTION_RETURN_PARAM_ERROR);
    EXPECT_EQ(string_unescape(test_string, strlen(test_string), NULL, 10), FUNCTION_RETURN_PARAM_ERROR);

    EXPECT_EQ(string_unescape(test_string, strlen(test_string), result_string, 3), FUNCTION_RETURN_INSUFFICIENT_MEMORY);
    EXPECT_STREQ(result_string, "\a\b\f");

    EXPECT_EQ(string_unescape(test_string, strlen(test_string), result_string, 11), FUNCTION_RETURN_OK);
    EXPECT_STREQ(result_string, "\a\b\f\v\n\r\t\"\\/");
}