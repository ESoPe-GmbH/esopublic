# List

Provides a singly linked list without a specific element type. The element type needs to be a structure which contains a pointer to the next element.  
To use it you need to call `list_init` with a pointer to a `list_t` struct to store the configuration and state of the list, a pointer to a list element and a pointer to the pointer to the next element inside list element structure. The two pointers are only used to calculate the offset of the pointer to the next element in the structure.  
If you want to add given list element you need to call `list_add_element` with a pointer to the initialized `list_t` structure and a pointer to the element you want to add.  
If element's address is already added in the list, it is not added again. If you want to add a copy of a structure to the list, you have to make the copy manually, since only the pointer addresses are compared, not the content.  
Similarly when removing an element using `list_remove_element` only the pointer is compared. If the element isn't found in the list nothing will happen.  
To iterate over the list you can use `list_first_element` to retrieve a pointer to the first element, which you can then use to get the next element by calling `list_next_element` providing a pointer to the current element.

```c
typedef struct test_data_s test_data_t;
struct test_data_s
{
    int integer;
    test_data_t* next;//Important. This pointer is needed for the list module to work
};

void list_test(void)
{
    list_t list;
    test_data_t* test = NULL;
    test_data_t* elements = (test_data_t*)mcu_heap_calloc(5, sizeof(test_data_t));

    list_init(&list, test, &test->next);//It's not a problem, that test is a null pointer. It's only used to calculate the offset inside the struct

    //Filling the list
    for(int i = 0; i < 5; i++)
    {
        elements[i].integer = i;
        list_add_element(&list, &elements[i])
    }

    list_remove_element(&list, &elements[2]);//Removing a random element

    //Iterating the list
    test = (test_data_t*) list_first_element(&list);
    while(test)
    {
        DBG_VERBOSE("%d\n", test->integer);
        test = (test_data_t*) list_next_element(&list, test);
    }
    mcu_heap_free(elements);
}
```

The output of the debug console after running this code would look like:

```bash
0
1
3
4
```
