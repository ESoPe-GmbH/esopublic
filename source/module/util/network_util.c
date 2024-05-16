/**
 * @file network_util.c
 * @copyright Urheberrecht 2018-2022 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 */

#include "network_util.h"
#include <string.h>

bool network_util_mac_is_unicast(uint8_t* mac)
{
	static const uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	static const uint8_t multicast_mac[3] = {0x01, 0x00, 0x5E};
	static const uint8_t multicast_mac_ipv6[2] = {0x33, 0x33};

	// If destination mac is broadcast -> Mark as multicast.
	if(memcmp(mac, broadcast_mac, sizeof(broadcast_mac)) == 0)
		return false;

	// If destination mac is multicast -> Mark as multicast
	else if(memcmp(mac, multicast_mac, sizeof(multicast_mac)) == 0)
		return false;

	// If destination mac is multicast for ipv6 -> Mark as multicast
	else if(memcmp(mac, multicast_mac_ipv6, sizeof(multicast_mac_ipv6)) == 0)
		return false;

	// It's a unicast
	return true;
}
