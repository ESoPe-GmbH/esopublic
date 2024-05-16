/**
 * 	@file 	assert.h
 * 	@copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Contains macros for assertions
 *
 *  @version	1.00 (31.01.2023)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/
#ifndef __UTIL_ASSERT__FIRST_INCL
#define __UTIL_ASSERT__FIRST_INCL

#include "module_public.h"

//------------------------------------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------------------------------------

#ifndef ASSERT_PRINT_ERROR
/// If set to 1, the assert macro will use dbg_error to print errors, otherwise it will skip the debug message.
#define ASSERT_PRINT_ERROR      0
#endif

#if ASSERT_PRINT_ERROR
#include "module/comm/dbg.h"
#define _ASSERT_PRINT(...)     DBG_ERROR(__VA_ARGS__)
#else
#define _ASSERT_PRINT(...)     do{}while(0)
#endif

/// Empty define that can be used in DBG_ASSERT for the action if no action is needed. You could also leave it empty, but this is better for reading.
#define NO_ACTION

/// Empty define that can be used in DBG_ASSERT for the return value if it is a void function. You could also leave it empty, but this is better for reading.
#define NO_RETURN

/**
 * @brief Macro for asserting a certain boolean expression. If this expression is not met, action a is executed and the given return value r is returned.
 * @param b     Boolean expression to evaluate
 * @param a     Action to execute before returning (e.g. free a buffer). Keep empty to do nothing
 * @param r     Return value when expression is not met
 * @param ...   Error message when expression is not met. Uses the variable argument list for the error printing.
 */
#define ASSERT_RET(b, a, r, ...)        \
    if(!(b)) \
    {\
        _ASSERT_PRINT(__VA_ARGS__); \
        a; \
        return r; \
    }

/**
 * @brief Macro for asserting that a variable is not NULL. If \c v is NULL, action a is executed and the given return value r is returned.
 * 
 * @param v     Value that should not be NULL.
 * @param a     Action to execute before returning (e.g. free a buffer). Keep empty to do nothing
 * @param r     Return value when expression is not met
 */
#define ASSERT_RET_NOT_NULL(v, a, r)   ASSERT_RET(v != NULL, a, r, #v " cannot be NULL\n")


#endif // __UTIL_ASSERT__FIRST_INCL