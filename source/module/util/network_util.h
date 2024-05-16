/**
 * @file network_util.h
 * @author Tim Koczwara (tim.koczwara@esope.de)
 * @brief Types to be internally used for the mesh implementation.
 * @version 1.0
 * @date 22.05.2020
 * 
 * @copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * 
 */

#ifndef _NETWORK_UTIL_H_
#define _NETWORK_UTIL_H_

#include "module_public.h"

/// Takes a pointer and makes the arguments for a IPSTR argument in a printf string.
/// Sample: uint8_t my_ip[4] = { ... }; printf("My IP: " IPSTR "\n", IPPTR2STR(my_ip));
#define IPPTR2STR(ptr)      ((uint8_t*)(ptr))[0], ((uint8_t*)(ptr))[1], ((uint8_t*)(ptr))[2], ((uint8_t*)(ptr))[3]
#ifndef IPSTR
/// String to use in a printf formatstring to print an IP.
/// Sample: uint8_t my_ip[4] = { ... }; printf("My IP: " IPSTR "\n", IPPTR2STR(my_ip));
#define IPSTR "%d.%d.%d.%d"
#endif

/**
 * @brief Check if hardware address is a unicast address.
 * 
 * @param mac       Pointer to 48-Bit (6 Byte) hardware address. Make sure the Pointer is not NULL, since it is not checked internally.
 * @return true     Address is a unicast address.
 * @return false    Address is a multicast or broadcast address.
 */
bool network_util_mac_is_unicast(uint8_t* mac);

#endif /* _NETWORK_UTIL_H_ */
