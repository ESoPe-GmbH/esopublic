// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file hid.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Felix Herrmann
 *
 *  @brief
 *			Contains the HID key codes for various keyboard keys
 *
 *  @version	1.00 (12.06.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/
#include "module_public.h"
#if MODULE_ENABLE_ENUM

#ifndef MODULE_ENUM_HID_H_
#define MODULE_ENUM_HID_H_
typedef enum
{
	HID_KEY_A = 0x04,
	HID_KEY_B = 0x05,
	HID_KEY_C = 0x06,
	HID_KEY_D = 0x07,
	HID_KEY_E = 0x08,
	HID_KEY_F = 0x09,
	HID_KEY_G = 0x0a,
	HID_KEY_H = 0x0b,
	HID_KEY_I = 0x0c,
	HID_KEY_J = 0x0d,
	HID_KEY_K = 0x0e,
	HID_KEY_L = 0x0f,
	HID_KEY_M = 0x10,
	HID_KEY_N = 0x11,
	HID_KEY_O = 0x12,
	HID_KEY_P = 0x13,
	HID_KEY_Q = 0x14,
	HID_KEY_R = 0x15,
	HID_KEY_S = 0x16,
	HID_KEY_T = 0x17,
	HID_KEY_U = 0x18,
	HID_KEY_V = 0x19,
	HID_KEY_W = 0x1a,
	HID_KEY_X = 0x1b,
	HID_KEY_Y = 0x1c,
	HID_KEY_Z = 0x1d,
	HID_KEY_RETURN = 0x28,
	HID_KEY_ESCAPE = 0x29,
	HID_KEY_BACKSPACE = 0x2A,
	HID_KEY_F1 = 0x3A,
	HID_KEY_F2 = 0x3B,
	HID_KEY_F3 = 0x3C,
	HID_KEY_F4 = 0x3D,
	HID_KEY_F5 = 0x3E,
	HID_KEY_F6 = 0x3F,
	HID_KEY_F7 = 0x40,
	HID_KEY_F8 = 0x41,
	HID_KEY_F9 = 0x42,
	HID_KEY_F10 = 0x43,
	HID_KEY_F11 = 0x44,
	HID_KEY_F12 = 0x45,
	HID_KEY_DELETE = 0x4C,
	HID_KEY_RIGHT_ARROW = 0x4F,
	HID_KEY_LEFT_ARROW = 0x50,
	HID_KEY_DOWN_ARROW = 0x51,
	HID_KEY_UP_ARROW = 0x52,

}HID_KEY;


#endif /* MODULE_ENUM_HID_H_ */
#endif // MODULE_ENABLE_ENUM

