// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/**
 * 	@file base64.h
 * 	@copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief	Contains base64 encode and decode functions.
 *
 *	@version	1.04 (10.05.2022)
 *      - base64_encodeblock uses an internal 3 byte buffer if input pointer uses less than 3 byte.
 *	@version	1.03 (20.01.2020)
 *		- Added documentation
 *		- Added base64_encodebuffer
 *		- Added base64_encodebuffer_direct
 *		- Added base64_decodebuffer
 *		- Added base64_decodebuffer_direct
 *	@version	1.02 (08.06.2018)
 *		- Removed warning
 *	@version	1.01 (07.06.2018)
 *		- Added module.h support
 *  @version	1.00 (22.01.2016)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef MODULE_CONVERT_BASE64_H_
#define MODULE_CONVERT_BASE64_H_

#include "module_public.h"
#if MODULE_ENABLE_CONVERT_BASE64

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @deprecated Use @see base64_encodebuffer or @see base64_encodebuffer_direct.
 * Encodes a single base64 block which has to be 1 - 3 bytes length.
 * @param in		Pointer to the input buffer. Should have 1 to 3 bytes of data as len parameter suggests
 * @param out		Output buffer with a size of 4.
 * @param len		Number of bytes from the input buffer to encode. Values 1 to 3 are allowed.
 */
void base64_encodeblock(uint8_t* in, uint8_t out[4], uint8_t len);
/**
 * Encodes a buffer into a base64 string. A zero termination is not added! If you need a zero termination, use the return value and add it.
 * @param buf		Pointer to the buffer that contains the plain data.
 * @param out		Pointer to the buffer where the result is stored. Make sure the output buffer is big enough (4/3 of input buffer).
 * 					If out should be the same as buf, use @see base64_encodebuffer_direct instead!
 * @param len		Number of bytes to encode.
 * @return			Number of bytes in the output buffer.
 */
uint32_t base64_encodebuffer(uint8_t* buf, uint8_t* out, uint32_t len);
/**
 * Encodes a buffer into a base64 string. A zero termination is not added! If you need a zero termination, use the return value and add it.
 * @param buf		Pointer to the buffer that contains the plain data and will be written with the encoded data. Make sure the output buffer is big enough (4/3 of input buffer).
 * @param len		Number of bytes to encode.
 * @return			Number of bytes in the buffer after encoding.
 */
uint32_t base64_encodebuffer_direct(uint8_t* buf, uint32_t len);
/**
 * @deprecated Use @see base64_encodebuffer or @see base64_encodebuffer_direct.
 * Encodes a string into a base64 string.
 * @param str		Pointer to the buffer that contains the plain data.
 * @param out		Pointer to the buffer where the result is stored. Make sure the output buffer is big enough (4/3 of input buffer).
 * 					If out should be the same as buf, use @see base64_encodebuffer_direct instead!
 */
void base64_encodestring(char* str, uint8_t* out);
/**
 * @deprecated Use @see base64_decodebuffer or @see base64_decodebuffer_direct.
 * Encodes a single base64 block which has to be 1 - 3 bytes length.
 * @param in		Input buffer with a size of 3.
 * @param out		Output buffer with a size of 4.
 * @param len		Number of bytes from the input buffer to encode.
 */
void base64_decodeblock(uint8_t in[4], uint8_t out[3]);
/**
 * Decodes a base64 string into a binary buffer.
 * @param buf		Pointer to the buffer that contains the encoded data.
 * @param out		Pointer to the buffer where the result is stored.
 * 					If out should be the same as buf, use @see base64_decodebuffer_direct instead!
 * @param len		Number of bytes to decode.
 * @return			Number of bytes in the buffer after decoding.
 */
uint32_t base64_decodebuffer(uint8_t* buf, uint8_t* out, uint32_t len);
/**
 * Decodes a base64 string into a binary buffer.
 * @param buf		Pointer to the buffer that contains the encoded data and will be written with the decoded data.
 * @param len		Number of bytes to decode.
 * @return			Number of bytes in the buffer after decoding.
 */
uint32_t base64_decodebuffer_direct(uint8_t* buf, uint32_t len);
/**
 * @deprecated Use @see base64_decodebuffer or @see base64_decodebuffer_direct.
 * Decodes a base64 string into a binary buffer.
 * @param str		Pointer to the buffer that contains the encoded data.
 * @param out		Pointer to the buffer where the result is stored.
 * 					If out should be the same as buf, use @see base64_decodebuffer_direct instead!
 */
void base64_decodestring(char* str, uint8_t* out);
/**
 * @deprecated Use @see base64_decodebuffer or @see base64_decodebuffer_direct.
 * Decodes a base64 string into a binary buffer.
 * @param str		Pointer to the buffer that contains the encoded data and will be written with the decoded data.
 */
void base64_decodestring_direct(char* str);
/**
 * Decodes a base64 string into a uint32_t number. The decoded binary value is treated as a 4-Byte little endian number.
 * @param b64_buf	Pointer to the buffer that contains the encoded data.
 * @return			Decoded uint32_t value.
 */
uint32_t base64_decode_to_uint32_t(char* b64_buf);

#endif

#endif /* MODULE_CONVERT_BASE64_H_ */
