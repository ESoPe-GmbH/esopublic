#include <gtest/gtest.h>

extern "C"
{
    #include "module/list/list.h"

    void app_main_init(void)
    {

    }

    void board_init(void)
    {
        
    }
    typedef struct test_data_s test_data_t;
    struct test_data_s
    {
        int integer;
        char string[20];
        test_data_t* next;
    };

    static int GetListSize(list_t* list)
    {
        int cnt = 0;
        void* it = list_first_element(list);
        
        while(it){cnt++;it = list_next_element(list, it);}
        return cnt;
    }
}

TEST(list_list, all_test)
{
    list_t list;
    test_data_t* test = NULL;
    test_data_t* elements = (test_data_t*)mcu_heap_calloc(5, sizeof(test_data_t));
    ASSERT_NE(elements, nullptr);

    EXPECT_EQ(list_init(NULL, test, &test->next), FUNCTION_RETURN_PARAM_ERROR);

    EXPECT_EQ(list_init(&list, test, &test->next), FUNCTION_RETURN_OK);
    EXPECT_EQ(list.offset_next, 24);

    for(int i = 0; i < 5; i++)
    {
        elements[i].integer = i;
        snprintf(elements[i].string, sizeof(elements[i].string), "TestString%d", i);
    }

    EXPECT_EQ(list_add_element(NULL, &elements[0]), FUNCTION_RETURN_PARAM_ERROR);
    EXPECT_EQ(list_add_element(&list, NULL), FUNCTION_RETURN_PARAM_ERROR);
    EXPECT_EQ(list_first_element(&list), nullptr);

    EXPECT_EQ(list_add_element(&list, &elements[0]), FUNCTION_RETURN_OK);
    test = (test_data_t*) list_first_element(&list);
    EXPECT_EQ(test, &elements[0]);
    
    //Element already in list
    EXPECT_EQ(list_add_element(&list, &elements[0]), FUNCTION_RETURN_NOT_READY);

    EXPECT_EQ(list_add_element(&list, &elements[1]), FUNCTION_RETURN_OK);
    test = (test_data_t*)list_next_element(&list, test);
    EXPECT_EQ(test, &elements[1]);
    EXPECT_EQ(list_next_element(&list, test), nullptr);

    //Add the rest
    for(int i = 2; i < 4; i++)
    {
        EXPECT_EQ(list_add_element(&list, &elements[i]), FUNCTION_RETURN_OK);
        EXPECT_EQ(list_add_element(&list, &elements[i]), FUNCTION_RETURN_NOT_READY);
    }

    EXPECT_EQ(GetListSize(&list), 4);
    //Remove element not in list
    EXPECT_EQ(list_remove_element(&list, &elements[4]), FUNCTION_RETURN_NOT_FOUND);
    EXPECT_EQ(GetListSize(&list), 4);
    EXPECT_EQ(list_add_element(&list, &elements[4]), FUNCTION_RETURN_OK);
    EXPECT_EQ(GetListSize(&list), 5);

    EXPECT_EQ(list_remove_element(&list, &elements[2]), FUNCTION_RETURN_OK);
    EXPECT_EQ(GetListSize(&list), 4);

    EXPECT_EQ(list_remove_element(&list, &elements[0]), FUNCTION_RETURN_OK);
    EXPECT_EQ(GetListSize(&list), 3);

    EXPECT_EQ(list_add_element(&list, &elements[0]), FUNCTION_RETURN_OK);
    EXPECT_EQ(GetListSize(&list), 4);

    test = (test_data_t*) list_first_element(&list);
    EXPECT_EQ(test->integer, 1);
    EXPECT_STREQ(test->string, "TestString1");

    test = (test_data_t*) list_next_element(&list, test);
    EXPECT_EQ(test, &elements[3]);

    test = (test_data_t*) list_next_element(&list, test);
    EXPECT_EQ(test, &elements[4]);

    test = (test_data_t*) list_next_element(&list, test);
    EXPECT_EQ(test, &elements[0]);

    test = (test_data_t*) list_next_element(&list, test);
    EXPECT_EQ(test, nullptr);

    mcu_heap_free(elements);
}
