/**
 * 	@file 	list.h
 * 	@copyright Urheberrecht 2020-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * 	@author Tim Koczwara
 *
 *  @brief
 *  		The list module is used to offer a dynamic list like approach in C.
 *  		To use this with your code, you need a structure that contains a pointer variable that is used to point to the
 *  		next element in this list. If you use this, do not manipulate it manually and ensure it is not overwritten.
 *  		Use it only with the list module's API.
 *
 *	@version	1.02 (19.01.2022)
 * 	    - Modified to be used in esopekernel
 *  @version  	1.01 (27.06.2021)
 * 		- Used uintptr_t internally for pointer calculations.
 *  @version  	1.00 (29.08.2020)
 *  	- Initial release
 *
 ******************************************************************************/

#ifndef MODULE_LIST_LIST_H_
#define MODULE_LIST_LIST_H_

#include "module_public.h"

#include "module/enum/function_return.h"

//------------------------------------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------------------------------------
/// Version of the list module
#define LIST_STR_VERSION "1.02"

/// This define is used to abstract the legacy name, used by our modules to keep compatible.
/// @see list_add_element for details
#define list_add			list_add_element
/// This define is used to abstract the legacy name, used by our modules to keep compatible.
/// @see list_remove_element for details
#define list_remove			list_remove_element
/// This define is used to abstract the legacy name, used by our modules to keep compatible.
/// @see list_first_element for details
#define list_get_first		list_first_element
/// This define is used to abstract the legacy name, used by our modules to keep compatible.
/// @see list_next_element for details
#define list_get_next		list_next_element

//------------------------------------------------------------------------------------------------------------
// Structures
//------------------------------------------------------------------------------------------------------------

/**
 * Type for the list structure.
 */
typedef struct list_s list_t;

/**
 * @struct list_s
 * @brief Contains the context for all list functions.
 * Is used as a context for the list functions. Use @see list_init to initialize the context.
 * Afterwards, you can use the structure to add elements to the list or remove elements from the list.
 */
struct list_s
{
	/// Pointer to the first element of the list.
	void* first;
	/// Offset to the next variable inside the structure that is used for the list.
	uintptr_t offset_next;
};

//------------------------------------------------------------------------------------------------------------
// External functions
//------------------------------------------------------------------------------------------------------------

/**
 * @fn FUNCTION_RETURN list_init(list_t*)
 * @brief Initializes the list.
 * Make sure to call this initialization only once on the list. If you call list_init again for an already build list, the list might run into memory problems.
 * You can use a NULL Pointer for your structure to initialize the list. Example:
 * @code
struct test_s
{
	// ... your variables
	void* next; // This is important! This pointer is used by the list for storing the pointer to the next element. Type might be a pointer of your structure type instead of void.
	// ... maybe more of your variables
}

list_t list;
struct test_s* element = NULL;

// Now initialize the list.
list_init(&list, element, &element->next); // Even with a NULL Pointer, the next pointer will deliver the offset of next!
 * @endcode
 * @param list							Pointer to the list structure that needs to be initialized.
 * @param s								Pointer to your structure. This pointer is not added into the list! If you need to add it, call @see list_add afterwards.
 * @param s_next						Pointer to the address of the next variable inside your structure.
 * @retval FUNCTION_RETURN_OK			List was initialized successfully.
 * @retval FUNCTION_RETURN_PARAM_ERROR	List is NULL.
 */
FUNCTION_RETURN list_init(list_t* list, void* s, void* s_next);
/**
 * @fn FUNCTION_RETURN list_add(list_t*, void*)
 * @brief Add an element to the list.
 * Add an element to the list. If element's address is already added in the list, it is not added again. If you want to add a copy of a structure to the list,
 * you have to make the copy manually, since only the pointer addresses are compared, not the content.
 * @pre Make sure the list was initialized with @see list_init.
 * @post You can now remove element with @see list_remove and iterate the list via @see list_get_first and @see list_get_next.
 * @param list							Pointer to the list that was initialized with @see list_init.
 * @param s								Pointer to the structure that needs to be added to the list.
 * @retval FUNCTION_RETURN_OK			Element was added to the list.
 * @retval FUNCTION_RETURN_NOT_READY	Element is already inside the list.
 * @retval FUNCTION_RETURN_PARAM_ERROR	List or s are NULL.
 */
FUNCTION_RETURN list_add_element(list_t* list, void* s);
/**
 * @fn FUNCTION_RETURN list_remove(list_t*)
 * @brief Removes an element from the list.
 * Removes an element from the list. If element is not inside the list, nothing is removed. Only the address is checked for removal, not the content of your
 * structure.
 * @pre Make sure the list was initialized with @see list_init.
 * @param list							Pointer to the list that was initialized with @see list_init.
 * @param s								Pointer to the structure that needs to be removed from the list.
 * @retval FUNCTION_RETURN_OK			Element was removed to the list.
 * @retval FUNCTION_RETURN_NOT_FOUND	Element was not found in the list.
 * @retval FUNCTION_RETURN_PARAM_ERROR	List or s are NULL.
 */
FUNCTION_RETURN list_remove_element(list_t* list, void* s);
/**
 * @fn list_t list_get_first*(list_t*)
 * @brief Returns the first element of the list.
 * Returns the first element of the list. If list is not initialized, the pointer might be invalid. If no elements are found in the list, NULL is returned.
 * @pre Make sure the list was initialized with @see list_init.
 * @post Use @see list_get_next to iterate through the list.
 * @param list							Pointer to the list that was initialized with @see list_init.
 * @return								NULL if no element is present in the list, otherwise it is a pointer to the first list element.
 */
void* list_first_element(list_t* list);
/**
 * @fn list_t list_get_next*(list_t*, void*)
 * @brief Returns the next element in the list.
 * Returns the element that follows s from the parameter inside the list. If list is not initialized, the pointer might be invalid.
 * @pre Make sure the list was initialized with @see list_init.
 * @param list							Pointer to the list that was initialized with @see list_init.
 * @param s								Pointer to the structure that is inside the list. Should have been retrieved with @see list_get_first or @see list_get_next.
 * @return								NULL if no next element is present in the list, otherwise it is a pointer to the next list element.
 */
void* list_next_element(list_t* list, void* s);

#endif /* MODULE_LIST_LIST_H_ */
