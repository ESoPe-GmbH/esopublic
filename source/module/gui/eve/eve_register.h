/**
 * 	@file eve_register.h
 *  @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Contains definitions and Macors for registers and commands of the eve chip.
 *
 *  @version	1.00 (08.05.2014)
 *  	- Intial release
 *
 *	@defgroup grp_cmd_begin		Defines for the graphics primitive of begin
 *	@defgroup grp_bitmap_format	Defines the bitmap format for graphics
 *	@defgroup grp_bitmap_wrap	Defines the bitmap wrap for graphics
 *	@defgroup grp_bitmap_filter	Defines the bitmap filter for graphics
 *
 *
 *
 ******************************************************************************/

#ifndef EVE_REGISTER_H_
#define EVE_REGISTER_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Display list commands
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if EVE_USE_FT81X // FT81x Register Addresses

/**
 * Begin drawing a graphics primitive
 * It is recommended to have an END for each BEGIN.
 * Whereas advanced users can avoid the usage of END in order to save extra graphics instructions in the display list RAM.
 *
 * @param x		Graphics primitive. See @see grp_cmd_begin.
 */
#define EVE_BEGIN(x)							(0x1F000000 | (x))

/**
 * End drawing a graphics primitive.
 * It is recommended to have an END for each BEGIN.
 * Whereas advanced users can avoid the usage of END in order to save extra graphics instructions in the display list RAM.
 */
#define EVE_END()								0x21000000

/**
 * Clear buffers to preset values
 * The scissor test and the buffer write masks affect the operation of the clear. Scissor limits the cleared rectangle,
 * and the buffer write masks limit the affected buffers.
 * The state of the alpha function, blend function, and stenciling do not affect the clear.
 *
 * @param c		Clear color buffer. Setting this bit to 1 will clear the color buffer of the EVE to the preset value.
 * 				Setting this bit to 0 will maintain the color buffer of the EVE with an unchanged value.
 * 				The preset value is defined in command CLEAR_COLOR_RGB for RGB channel and CLEAR_COLOR_A for alpha channel.
 *
 * @param s		Clear stencil buffer. Setting this bit to 1 will clear the stencil buffer of the EVE to the preset value.
 * 				Setting this bit to 0 will maintain the stencil buffer of the EVE with an unchanged value.
 * 				The preset value is defined in command CLEAR_STENCIL.
 *
 * @param t		Clear tag buffer. Setting this bit to 1 will clear the tag buffer of the EVE to the preset value.
 * 				Setting this bit to 0 will maintain the tag buffer of the EVE with an unchanged value.
 * 				The preset value is defined in command CLEAR_TAG.
 */
#define EVE_CLEAR(c, s, t)					(0x26000000 | (c << 2) | (s << 1) | t)

/**
 * Specify clear values for red, green and blue channels
 * Sets the color values used by a following CLEAR.
 *
 * @param r		Red value used when the color buffer is cleared. The initial value is 0
 *
 * @param g		Green value used when the color buffer is cleared. The initial value is 0
 *
 * @param b		Blue value used when the color buffer is cleared. The initial value is 0
 */
#define EVE_CLEAR_COLOR_RGB(r, g, b)			(0x02000000 | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

/**
 * Set the current color red, green and blue
 * Sets red, green and blue values of the EVE color buffer which will be applied to the following draw operation.
 *
 * @param r		Red value for the current color. The initial value is 255
 *
 * @param g		Green value for the current color. The initial value is 255
 *
 * @param b		Blue value for the current color. The initial value is 255
 */
#define EVE_COLOR_RGB(r, g, b)				(0x04000000 | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

/**
 * Set the current color alpha
 * Sets alpha value of the EVE color buffer which will be applied to the following draw operation.
 *
 * @param a		Alpha value for the current color. The initial value is 255
 */
#define EVE_COLOR_A(a)						(0x10000000 | (uint32_t)(a))

/**
 * 	End the display list. EVE will ignore all the commands following this command.
 */
#define EVE_DISPLAY()							0x00

/**
 *	Specify the width of lines to be drawn with primitive LINES in 1/16th pixel precision.
 *	Sets the width of drawn lines. The width is the distance from the center of the line to the outermost drawn pixel,
 *	in units of 1/16 pixel. The valid range is from 16 to 4095 in terms of 1/16th pixel units.
 *	Please note the LINE_WIDTH command will affect the LINES, LINE_STRIP, RECTS, EDGE_STRIP_A/B/R/L primitives.
 *
 * @param x		Line width in 1/16 pixel. The initial value is 16.
 */
#define EVE_LINE_WIDTH(x)						(0x0E000000 | (x))

/**
 *	Specify the radius of points
 *	Sets the size of drawn points. The width is the distance from the center of the point to the outermost drawn pixel,
 *	in units of 1/16 pixels.
 *	The valid range is from 16 to 8191 with respect to 1/16th pixel unit.
 *
 * @param x		Point radius in 1/16 pixel. The initial value is 16.
 */
#define EVE_POINT_SIZE(x)						(0x0D000000 | (x))

/**
 *	Start the operation of graphics primitives at the specified screen coordinate, in 1/16th pixel precision.
 *	The range of coordinates can be from -16384 to +16383 in terms of 1/16th pixel units.
 *	Please note the negative x coordinate value means the coordinate in the left virtual screen from (0, 0),
 *	while the negative y coordinate value means the coordinate in the upper virtual screen from (0, 0).
 *	If drawing on the negative coordinate position, the drawing operation will not be visible.
 *
 * @param x		Signed x-coordinate in 1/16 pixel precision
 * @param y		Signed y-coordinate in 1/16 pixel precision
 */
#define EVE_VERTEX2F(x, y)					(0x40000000 | ((y) & 0x3FFF) | ((uint32_t)((x) & 0x3FFF) << 15))

/**
 *	Start the operation of graphics primitive at the specified coordinates. The handle and cell parameters
 *	will be ignored unless the graphics primitive is specified as bitmap by command BEGIN, prior to this command.
 *
 * @param x		Signed x-coordinate in pixel precision
 * @param y		Signed y-coordinate in pixel precision
 * @param h		Bitmap handle. The valid range is from 0 to 31. From 16 to 31,
 * 				the bitmap handle is dedicated to the EVE built-in font.
 * @param c		Cell number. Cell number is the index of bitmap with same bitmap layout and format.
 * 				For example, for handle 31, the cell 65 means the character "A" in the largest built in font.
 */
#define EVE_VERTEX2II(x, y, h, c)				(0x80000000 | ((c) & 0x7F) | ((uint32_t)((x) & 0x1FF) << 21) | ((uint32_t)((y) & 0x1FF) << 12) | ((uint32_t)((h) & 0x1F) << 7))
/**
 *	Set the precision of VERTEX2F coordinates
 *
 *	VERTEX2F uses 15 bit signed numbers for its (X,Y) coordinates.
 *	This command controls the interpretation of these numbers by specifying the number of fractional bits.
 *	By varying the format, an application can trade range against precision.
 *
 * @param f		Number of fractional bits in X,Y coordinates. Valid range is from 0 to 4. The initial value is 4.
 */
#define EVE_VERTEX_FORMAT(f)					(0x27000000 | (f & 0x07))

#define EVE_SCISSOR_XY(x, y)					(0x1B000000 | ((x) << 11) | (y))

#define EVE_SCISSOR_SIZE(w, h)				(0x1C000000 | ((w) << 12) | (h))

/***
 * @brief Specify the extended format of the bitmap
 * If BITMAP_LAYOUT specifies a format for GLFORMAT (31), then the format is taken from BITMAP_EXT_FORMAT instead.
 * 
 * @param f		Bitmap pixel format. See @see IMAGE_FORMAT_T
 */
#define EVE_BITMAP_EXT_FORMAT(f)				(0x2E000000 | (f & 0xFFFF))

/**
 * Handles 16 to 31 are defined by the EVE for built-in font and handle 15 is defined in the co-processor engine
 * commands CMD_GRADIENT, CMD_BUTTON and CMD_KEYS. Users can define new bitmaps using handles from 0 to 14.
 * If there is no co-processor engine command CMD_GRADIENT, CMD_BUTTON and CMD_KEYS in the current display list,
 * users can even define a bitmap using handle 15.
 *
 * @param h		Bitmap handle. The initial value is 0. The valid value range is from 0 to 31.
 */
#define EVE_BITMAP_HANDLE(h)					(0x05000000 | (h))

/**
 *	Specify the source bitmap memory format and layout for the current handle.
 *
 * @param f		Bitmap pixel format. The valid range is from 0 to 11 and defined as EVE_FORMAT_ defines.
 * @param l		Bitmap linestride, in bytes.
 * @param h		Bitmap height, in lines
 */
#define EVE_BITMAP_LAYOUT(f, l, h)			(0x07000000 | ((uint32_t)(f) << 19) | ((uint32_t)((l) & 0x3FF) << 9) | ((h)  & 0x1FF))

/**
 * Specify the 2 most significant bits of the source bitmap memory format and layout for the current handle.
 * @param l		Bitmap linestride, in bytes.
 * @param h		Bitmap height, in lines
 */
#define EVE_BITMAP_LAYOUT_H(l, h)				(0x28000000 | ((uint32_t)(((l) & 0xC00) >> 10) << 2) | (((h)  & 0x600) >> 9))

/**
 *	Specify the source bitmap memory format and layout for the current handle.
 *
 * @param f		Bitmap filtering mode, one of EVE_FILTER_NEAREST or EVE_FILTER_BILINEAR.
 * @param wx	Bitmap x wrap mode, one of EVE_WRAP_REPEAT or EVE_WRAP_BORDER
 * @param wy	Bitmap y wrap mode, one of EVE_WRAP_REPEAT or EVE_WRAP_BORDER
 * @param w		Drawn bitmap width, in pixels
 * @param h		Drawn bitmap height, in pixels
 */
#define EVE_BITMAP_SIZE(f, wx, wy, w, h)		(0x08000000 | ((uint32_t)(f) << 20)  | ((uint32_t)(wx) << 19) | ((uint32_t)(wy) << 18) | ((uint32_t)((w) & 0x1FF) << 9) | ((h) & 0x1FF))

/**
 * Specify the 2 most significant bits of bitmaps dimension for the current handle.
 * @param w		Drawn bitmap width, in pixels
 * @param h		Drawn bitmap height, in pixels
 */
#define EVE_BITMAP_SIZE_H(w, h)				(0x29000000 | ((uint32_t)(((w) & 0x600) >> 9) << 2) | (((h)  & 0x600) >> 9))

/**
 * @brief Specify the source address of bitmap data in EVE graphics memory RAM_G.
 *
 * The bitmap source address is normally the address in main memory where the bitmap graphic data is loaded.
 *
 * @param h		Bitmap address in graphics SRAM EVE, aligned with respect to the bitmap format.
 */
#define EVE_BITMAP_SOURCE(h)					(0x01000000 | ((h) & 0x003FFFFF))

/**
 * @brief Specify the A coefficient of the bitmap transform matrix.
 *
 * BITMAP_TRANSFORM_A-F  coefficients  are  used  to  perform  bitmap  transform  functionalities such as
 * scaling, rotation and translation. These are similar to openGL transform functionality.
 *
 * @param a		Coefficient A of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial
 * 				value is 256.
 */
#define EVE_BITMAP_TRANSFORM_A_8_8(a)				(0x15000000 | ((a) & 0x0FFFF))

/**
 * @brief Specify the E coefficient of the bitmap transform matrix.
 *
 * BITMAP_TRANSFORM_A-F  coefficients  are  used  to  perform  bitmap  transform  functionalities such as
 * scaling, rotation and translation. These are similar to openGL transform functionality.
 *
 * @param e		Coefficient E of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial
 * 				value is 256.
 */
#define EVE_BITMAP_TRANSFORM_E_8_8(e)				(0x19000000 | ((e) & 0x0FFFF))

/**
 * @brief Specify the A coefficient of the bitmap transform matrix.
 *
 * BITMAP_TRANSFORM_A-F  coefficients  are  used  to  perform  bitmap  transform  functionalities such as
 * scaling, rotation and translation. These are similar to openGL transform functionality.
 *
 * @param a		Coefficient A of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial
 * 				value is 256.
 */
#define EVE_BITMAP_TRANSFORM_A_1_15(a)				(0x15010000 | ((a) & 0x0FFFF))

/**
 * @brief Specify the E coefficient of the bitmap transform matrix.
 *
 * BITMAP_TRANSFORM_A-F  coefficients  are  used  to  perform  bitmap  transform  functionalities such as
 * scaling, rotation and translation. These are similar to openGL transform functionality.
 *
 * @param e		Coefficient E of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial
 * 				value is 256.
 */
#define EVE_BITMAP_TRANSFORM_E_1_15(e)				(0x19010000 | ((e) & 0x0FFFF))

/**
 * @brief No operation
 * 
 * Does nothing. May be used as a spacer in display lists, if required.
 * 
 */
#define EVE_NOP()								0x2D000000

#else

/**
 * Begin drawing a graphics primitive
 * It is recommended to have an END for each BEGIN.
 * Whereas advanced users can avoid the usage of END in order to save extra graphics instructions in the display list RAM.
 *
 * @param x		Graphics primitive. See @see grp_cmd_begin.
 */
#define EVE_BEGIN(x)							(0x1F000000 | (x))

/**
 * End drawing a graphics primitive.
 * It is recommended to have an END for each BEGIN.
 * Whereas advanced users can avoid the usage of END in order to save extra graphics instructions in the display list RAM.
 */
#define EVE_END()								0x21000000

/**
 * Clear buffers to preset values
 * The scissor test and the buffer write masks affect the operation of the clear. Scissor limits the cleared rectangle,
 * and the buffer write masks limit the affected buffers.
 * The state of the alpha function, blend function, and stenciling do not affect the clear.
 *
 * @param c		Clear color buffer. Setting this bit to 1 will clear the color buffer of the EVE to the preset value.
 * 				Setting this bit to 0 will maintain the color buffer of the EVE with an unchanged value.
 * 				The preset value is defined in command CLEAR_COLOR_RGB for RGB channel and CLEAR_COLOR_A for alpha channel.
 *
 * @param s		Clear stencil buffer. Setting this bit to 1 will clear the stencil buffer of the EVE to the preset value.
 * 				Setting this bit to 0 will maintain the stencil buffer of the EVE with an unchanged value.
 * 				The preset value is defined in command CLEAR_STENCIL.
 *
 * @param t		Clear tag buffer. Setting this bit to 1 will clear the tag buffer of the EVE to the preset value.
 * 				Setting this bit to 0 will maintain the tag buffer of the EVE with an unchanged value.
 * 				The preset value is defined in command CLEAR_TAG.
 */
#define EVE_CLEAR(c, s, t)					(0x26000000 | (c << 2) | (s << 1) | t)

/**
 * Specify clear values for red, green and blue channels
 * Sets the color values used by a following CLEAR.
 *
 * @param r		Red value used when the color buffer is cleared. The initial value is 0
 *
 * @param g		Green value used when the color buffer is cleared. The initial value is 0
 *
 * @param b		Blue value used when the color buffer is cleared. The initial value is 0
 */
#define EVE_CLEAR_COLOR_RGB(r, g, b)			(0x02000000 | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

/**
 * Set the current color red, green and blue
 * Sets red, green and blue values of the EVE color buffer which will be applied to the following draw operation.
 *
 * @param r		Red value for the current color. The initial value is 255
 *
 * @param g		Green value for the current color. The initial value is 255
 *
 * @param b		Blue value for the current color. The initial value is 255
 */
#define EVE_COLOR_RGB(r, g, b)				(0x04000000 | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

/**
 * Set the current color alpha
 * Sets alpha value of the EVE color buffer which will be applied to the following draw operation.
 *
 * @param a		Alpha value for the current color. The initial value is 255
 */
#define EVE_COLOR_A(a)						(0x10000000 | (uint32_t)(a))

/**
 * 	End the display list. EVE will ignore all the commands following this command.
 */
#define EVE_DISPLAY()							0x00

/**
 *	Specify the width of lines to be drawn with primitive LINES in 1/16th pixel precision.
 *	Sets the width of drawn lines. The width is the distance from the center of the line to the outermost drawn pixel,
 *	in units of 1/16 pixel. The valid range is from 16 to 4095 in terms of 1/16th pixel units.
 *	Please note the LINE_WIDTH command will affect the LINES, LINE_STRIP, RECTS, EDGE_STRIP_A/B/R/L primitives.
 *
 * @param x		Line width in 1/16 pixel. The initial value is 16.
 */
#define EVE_LINE_WIDTH(x)						(0x0E000000 | (x))

/**
 *	Specify the radius of points
 *	Sets the size of drawn points. The width is the distance from the center of the point to the outermost drawn pixel,
 *	in units of 1/16 pixels.
 *	The valid range is from 16 to 8191 with respect to 1/16th pixel unit.
 *
 * @param x		Point radius in 1/16 pixel. The initial value is 16.
 */
#define EVE_POINT_SIZE(x)						(0x0D000000 | (x))

/**
 *	Start the operation of graphics primitives at the specified screen coordinate, in 1/16th pixel precision.
 *	The range of coordinates can be from -16384 to +16383 in terms of 1/16th pixel units.
 *	Please note the negative x coordinate value means the coordinate in the left virtual screen from (0, 0),
 *	while the negative y coordinate value means the coordinate in the upper virtual screen from (0, 0).
 *	If drawing on the negative coordinate position, the drawing operation will not be visible.
 *
 * @param x		Signed x-coordinate in 1/16 pixel precision
 * @param y		Signed y-coordinate in 1/16 pixel precision
 */
#define EVE_VERTEX2F(x, y)					(0x40000000 | (y) | ((uint32_t)(x) << 15))

/**
 *	Start the operation of graphics primitive at the specified coordinates. The handle and cell parameters
 *	will be ignored unless the graphics primitive is specified as bitmap by command BEGIN, prior to this command.
 *
 * @param x		Signed x-coordinate in pixel precision
 * @param y		Signed y-coordinate in pixel precision
 * @param h		Bitmap handle. The valid range is from 0 to 31. From 16 to 31,
 * 				the bitmap handle is dedicated to the EVE built-in font.
 * @param c		Cell number. Cell number is the index of bitmap with same bitmap layout and format.
 * 				For example, for handle 31, the cell 65 means the character "A" in the largest built in font.
 */
#define EVE_VERTEX2II(x, y, h, c)				(0x80000000 | (c) | ((uint32_t)(x) << 21) | ((uint32_t)(y) << 12) | ((uint32_t)(h) << 7))

#define EVE_SCISSOR_XY(x, y)					(0x1B000000 | (((x) & 0x1FF) << 9) | ((y) & 0x1FF))

#define EVE_SCISSOR_SIZE(w, h)				(0x1C000000 | (((w) & 0x3FF) << 10) | ((h) & 0x3FF))

/**
 * Handles 16 to 31 are defined by the EVE for built-in font and handle 15 is defined in the co-processor engine
 * commands CMD_GRADIENT, CMD_BUTTON and CMD_KEYS. Users can define new bitmaps using handles from 0 to 14.
 * If there is no co-processor engine command CMD_GRADIENT, CMD_BUTTON and CMD_KEYS in the current display list,
 * users can even define a bitmap using handle 15.
 *
 * @param h		Bitmap handle. The initial value is 0. The valid value range is from 0 to 31.
 */
#define EVE_BITMAP_HANDLE(h)					(0x05000000 | (h))

/**
 *	Specify the source bitmap memory format and layout for the current handle.
 *
 * @param f		Bitmap pixel format. The valid range is from 0 to 11 and defined as EVE_FORMAT_ defines.
 * @param l		Bitmap linestride, in bytes.
 * @param h		Bitmap height, in lines
 */
#define EVE_BITMAP_LAYOUT(f, l, h)			(0x07000000 | ((uint32_t)(f) << 19) | ((uint32_t)(l) << 9) | (h))

/**
 *	Specify the source bitmap memory format and layout for the current handle.
 *
 * @param f		Bitmap filtering mode, one of EVE_FILTER_NEAREST or EVE_FILTER_BILINEAR.
 * @param wx	Bitmap x wrap mode, one of EVE_WRAP_REPEAT or EVE_WRAP_BORDER
 * @param wy	Bitmap y wrap mode, one of EVE_WRAP_REPEAT or EVE_WRAP_BORDER
 * @param w		Drawn bitmap width, in pixels
 * @param h		Drawn bitmap height, in pixels
 */
#define EVE_BITMAP_SIZE(f, wx, wy, w, h)		(0x08000000 | ((uint32_t)(f) << 20)  | ((uint32_t)(wx) << 19) | ((uint32_t)(wy) << 18) | ((uint32_t)(w) << 9) | (h))

/**
 * @brief Specify the source address of bitmap data in EVE graphics memory RAM_G.
 *
 * The bitmap source address is normally the address in main memory where the bitmap graphic data is loaded.
 *
 * @param h		Bitmap address in graphics SRAM EVE, aligned with respect to the bitmap format.
 */
#define EVE_BITMAP_SOURCE(h)					(0x01000000 | (h))

/**
 * @brief Specify the A coefficient of the bitmap transform matrix.
 *
 * BITMAP_TRANSFORM_A-F  coefficients  are  used  to  perform  bitmap  transform  functionalities such as
 * scaling, rotation and translation. These are similar to openGL transform functionality.
 *
 * @param a		Coefficient A of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial
 * 				value is 256.
 */
#define EVE_BITMAP_TRANSFORM_A(a)				(0x15000000 | ((a) & 0x0FFFF))

/**
 * @brief Specify the E coefficient of the bitmap transform matrix.
 *
 * BITMAP_TRANSFORM_A-F  coefficients  are  used  to  perform  bitmap  transform  functionalities such as
 * scaling, rotation and translation. These are similar to openGL transform functionality.
 *
 * @param e		Coefficient E of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial
 * 				value is 256.
 */
#define EVE_BITMAP_TRANSFORM_E(e)				(0x19000000 | ((e) & 0x0FFFF))

/**
 *	Set the precision of VERTEX2F coordinates. NOT SUPPORTED BY EVE.
 *	This register is checked on EVE to simulate the behaviour of this register from FT810
 *
 * @param f		Number of fractional bits in X,Y coordinates. Valid range is from 0 to 4. The initial value is 4.
 */
#define EVE_VERTEX_FORMAT(f)					(0x27000000 | (f & 0x07))
/// Not supported and not needed by EVE -> Is not sent to EVE
#define EVE_BITMAP_SIZE_H(w, h)				0x01
/// Not supported and not needed by EVE -> Is not sent to EVE
#define EVE_BITMAP_LAYOUT_H(w, h)				0x01


#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Co-processor only commands
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Attach the tag value for the following graphics objects drawn on the screen. The initial tag buffer value is 255.
 *
 * The initial value of the tag buffer of the EVE is specified by command CLEAR_TAG and taken effect by command CLEAR.
 * TAG command can specify the value of the tag buffer of the EVE that applies to the graphics objects when they are
 * drawn on the screen. Please note that this TAG value will be assigned to all the following objects, unless the TAG_MASK
 * command is used to disable it. Once the following graphics objects are drawn, they are attached with the tag value
 * successfully. When the graphics objects attached with the tag value are touched, the register REG_TOUCH_TAG will be
 * updated with the tag value of the graphics object being touched.
 * If there is no TAG commands in one display list, all the graphics objects rendered by the display list will report tag
 * value as 255 in REG_TOUCH_TAG when they were touched.
 *
 * @param s		Tag value. Valid value range is from 1 to 255.
 */
#define EVE_TAG(x)							(0x03000000 | (x))

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Defines for BEGIN()
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @addtogroup grp_cmd_begin
/// @{
#define EVE_BITMAPS							1		///< Bitmap drawing primitive
#define EVE_POINTS							2		///< Point drawing primitive
#define EVE_LINES								3		///< Line drawing primitive
#define EVE_LINE_STRIP						4		///< Line strip drawing primitive
#define EVE_EDGE_STRIP_R						5		///< Edge strip right side drawing primitive
#define EVE_EDGE_STRIP_L						6		///< Edge strip left side drawing primitive
#define EVE_EDGE_STRIP_A						7		///< Edge strip above drawing primitive
#define EVE_EDGE_STRIP_B						8		///< Edge strip below side drawing primitive
#define EVE_RECTS								9		///< Rectangle drawing primitive
/// @}

/// @addtogroup grp_bitmap_format
/// @{
#define EVE_FORMAT_ARGB1555					0		///< Bitmap format for ARGB1555
#define EVE_FORMAT_L1							1		///< Bitmap format for L1
#define EVE_FORMAT_L4							2		///< Bitmap format for L4
#define EVE_FORMAT_L8							3		///< Bitmap format for L8
#define EVE_FORMAT_RGB332						4		///< Bitmap format for RGB332
#define EVE_FORMAT_ARGB2						5		///< Bitmap format for ARGB3
#define EVE_FORMAT_ARGB4						6		///< Bitmap format for ARGB4
#define EVE_FORMAT_RGB565						7		///< Bitmap format for RGB565
#define EVE_FORMAT_PALETTED					8		///< Bitmap format for PALETTED
#define EVE_FORMAT_TEXT8X8					9		///< Bitmap format for TEXT8X8
#define EVE_FORMAT_TEXTVGA					10		///< Bitmap format for TEXTVGA
#define EVE_FORMAT_BARGRAPH					11		///< Bitmap format for BARGRAPH
#define EVE_FORMAT_PALETTED565				14		///< Bitmap format for PALETTED565
#define EVE_FORMAT_PALETTED4444				15		///< Bitmap format for PALETTED4444
#define EVE_FORMAT_PALETTED8					16		///< Bitmap format for PALETTED8
#define EVE_FORMAT_L2							17		///< Bitmap format for L2
/// @}

/// @addtogroup grp_bitmap_filter
/// @{
#define EVE_FILTER_NEAREST					0		///< Nearest filter
#define EVE_FILTER_BILINEAR					1		///< Bilinear filter
/// @}

/// @addtogroup grp_bitmap_wrap
/// @{
#define EVE_WRAP_BORDER						0		///< Border wrapping.
#define EVE_WRAP_REPEAT						1		///< Repeats the bitmap if it is smaller then the given size
/// @}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
typedef enum
{
	// Power Modes
	
	/// Switch from Standby/Sleep modes to active mode. Dummy read from address 0 generates ACTIVE command.
	EVE_HOST_CMD_ACTIVE = 	0x00,
	/// Put EVE core to standby mode. Clock gate off, PLL and Oscillator remain on (default).	
	EVE_HOST_CMD_STANDBY = 	0x41,
	/// Put EVE core to sleep mode. Clock gate off, PLL and Oscillator off.
	EVE_HOST_CMD_SLEEP = 		0x42,	
	/// Switch off 1.2V internal regulator. Clock, PLL and Oscillator off.
	EVE_HOST_CMD_POWERDOWN = 	0x50,	
	
	// Clock Switching
	
	/// Enable PLL input from Crystal oscillator or external input clock.
	EVE_HOST_CMD_CLKEXT = 	0x44,
	/// Switch PLL output clock to 48MHz (default).
	EVE_HOST_CMD_CLK48M = 	0x62,
	/// Switch PLL output clock to 36MHz.
	EVE_HOST_CMD_CLK36M = 	0x61,
	
	// Miscellaneous

	/// Send reset pulse to EVE core. All registers and state machines will be reset.
	EVE_HOST_CMD_CORERESET = 	0x68	
}EVE_HOST_CMD;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Memory map
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if EVE_USE_FT81X // FT81x Register Addresses

/// 1024 kByte	Main graphics RAM
#define EVE_RAM_G				0x00000000
/// 4 Byte		EVE chip identification and revision information:
///				[0:1] Chip ID: "0800"
///				[2:3] Version ID: "0100"
#define EVE_ROM_CHIPID			0x000C0000
/// 1152 kByte	Font table and bitmap
#define EVE_ROM_FONT			0x001E0000
/// 4 Byte		Font table pointer address
#define EVE_ROM_FONT_ADDR		0x002FFFFC
/// 8 kByte		Display List RAM
#define EVE_RAM_DL				0x00300000
/// 1 kByte		Palette RAM
#define EVE_RAM_PAL				0x00302000
/// 4 kByte		Command Buffer
#define EVE_RAM_CMD				0x00308000

#if EVE_GENERATION > 2
/// 128b: 		Coprocessor fault report RAM
#define EVE_RAM_ERR_REPORT		0x00309800
/// 256MB: 		External NOR flash memory. Maximum 256MB. The address is used by internal command only.
#define EVE_FLASH 				0x00800000
#endif

/// 1024 kByte	Maximum numbers of bytes tht can be written to EVE_RAM_G
#define EVE_RAM_SIZE			0x00100000

#else

/// 256 kByte	Main graphics RAM
#define EVE_RAM_G				0x00000000
/// 4 Byte		EVE chip identification and revision information:
///				[0:1] Chip ID: "0800"
///				[2:3] Version ID: "0100"
#define EVE_ROM_CHIPID			0x000C0000
/// 275 kByte	Font table and bitmap
#define EVE_ROM_FONT			0x000BB23C
/// 4 Byte		Font table pointer address
#define EVE_ROM_FONT_ADDR		0x000FFFFC
/// 8 kByte		Display List RAM
#define EVE_RAM_DL				0x00100000
/// 1 kByte		Palette RAM
#define EVE_RAM_PAL				0x00102000
/// 4 kByte		Command Buffer
#define EVE_RAM_CMD				0x00108000
/// 256 kByte	Maximum numbers of bytes tht can be written to EVE_RAM_G
#define EVE_RAM_SIZE			0x00040000

#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Register Addresses
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if EVE_USE_FT81X // FT81x Register Addresses

#define EVE_REG_ID				0x00302000		///< 8 Bits		r/o		Identification register, always reads as 0x7C
#define EVE_REG_FRAMES			0x00302004		///< 32 Bits	r/o		Frame counter, since reset
#define EVE_REG_CLOCK				0x00302008		///< 32 Bits	r/o		Clock cycles, since reset
#define EVE_REG_FREQUENCY			0x0030200C		///< 28 Bits	r/w		Main clock frequency
#define EVE_REG_RENDERMODE		0x00302010		///< 1 Bits		r/w		Rendering mode: 0 = normal, 1 = single-line
#define EVE_REG_SNAPY				0x00302014		///< 11 Bits	r/w		Scan line select for RENERMODE 1
#define EVE_REG_SNAPSHOT			0x00302018		///< 1 Bits		r/w		trigger for RENDERMODE 1
#define EVE_REG_SNAPFORMAT		0x0030201C		///< 6 Bits		r/w		Pixel format for scanline readout
#define EVE_REG_CPURESET			0x00302020		///< 3 Bits		r/w		Graphics, audio and touch engines reset control
#define EVE_REG_TAP_CRC			0x00302024		///< 32 Bits	r/o		Live video tap crc. Frame CRC is computed every DL SWAP.
#define EVE_REG_TAP_MASK			0x00302028		///< 32 Bits	r/w		Live video tap mask
#define EVE_REG_HCYCLE			0x0030202C		///< 10 Bits	r/w		Horizontal total cycle count
#define EVE_REG_HOFFSET			0x00302030		///< 10 Bits	r/w		Horizontal display start offset
#define EVE_REG_HSIZE				0x00302034		///< 10 Bits	r/w		Horizontal display pixel count
#define EVE_REG_HSYNC0			0x00302038		///< 10 Bits	r/w		Horizontal sync fall offset
#define EVE_REG_HSYNC1			0x0030203C		///< 10 Bits	r/w		Horizontal sync rise offset
#define EVE_REG_VCYCLE			0x00302040		///< 10 Bits	r/w		Vertical total cycle count
#define EVE_REG_VOFFSET			0x00302044		///< 10 Bits	r/w		Vertical display start offset
#define EVE_REG_VSIZE				0x00302048		///< 10 Bits	r/w		Vertical display line count
#define EVE_REG_VSYNC0			0x0030204C		///< 10 Bits	r/w		Vertical sync fall offset
#define EVE_REG_VSYNC1			0x00302050		///< 10 Bits	r/w		Vertical sync rise offset
#define EVE_REG_DLSWAP			0x00302054		///< 2 Bits		r/w		Display list swap control
	#define EVE_VAL_DLSWAP_DONE          0UL
	#define EVE_VAL_DLSWAP_FRAME         2UL
	#define EVE_VAL_DLSWAP_LINE          1UL
#define EVE_REG_ROTATE			0x00302058		///< 1 Bits		r/w		Screen 180 degree rotate
#define EVE_REG_OUTBITS			0x0030205C		///< 9 Bits		r/w		Output bit resolution, 3 x 3 x 3 bits
#define EVE_REG_DITHER			0x00302060		///< 1 Bits		r/w		Output dither enable
#define EVE_REG_SWIZZLE			0x00302064		///< 4 Bits		r/w		Output RGB signal swizzle
#define EVE_REG_CSPREAD			0x00302068		///< 1 Bits		r/w		Output clock spreading enable
#define EVE_REG_PCLK_POL			0x0030206C		///< 1 Bits		r/w		PCLK polarity:
													///<						0 = output on PCLK rising edge,
													///<						1 = output on PCLK falling edge
#define EVE_REG_PCLK				0x00302070		///< 8 Bits		r/w		PCLK frequency divider, 0 = disable
#define EVE_REG_TAG_X				0x00302074		///< 9 Bits		r/w		Tag query X coordinate
#define EVE_REG_TAG_Y				0x00302078		///< 9 Bits		r/w		Tag query Y coordinate
#define EVE_REG_TAG				0x0030207C		///< 8 Bits		r/o		Tag query result
#define EVE_REG_VOL_PB			0x00302080		///< 8 Bits		r/w		Volume for playback
#define EVE_REG_VOL_SOUND			0x00302084		///< 8 Bits		r/w		Volume for synthesizer sound
#define EVE_REG_SOUND				0x00302088		///< 16 Bits	r/w		Sound effect select
#define EVE_REG_PLAY				0x0030208C		///< 1 Bits		r/w		Start effect playback
#define EVE_REG_GPIO_DIR			0x00302090		///< 8 Bits		r/w		GPIO pin direction, 0 = input, 1 = output
#define EVE_REG_GPIO				0x00302094		///< 8 Bits		r/w		GPIO pin value (bit 0, 1, 7); output pin drive strength (bit 2-6)
#define EVE_REG_GPIOX_DIR			0x00302098		///< 8 Bits		r/w		Extended GPIO pin direction, 0 = input, 1 = output
#define EVE_REG_GPIOX				0x0030209C		///< 8 Bits		r/w		Extended GPIO pin value (bit 0, 1, 7); output pin drive strength (bit 2-6)
#define EVE_REG_INT_FLAGS			0x003020A8		///< 8 Bits		r/o		Interrupt flags, cleared by read
	#define EVE_VAL_INT_CONVCOMPLETE	0x80		///< 					Interrupt Flag Bit 7 Touch-screen conversions completed
	#define EVE_VAL_INT_CMDFLAG		0x40		///< 					Interrupt Flag Bit 6 Command FIFO flag
	#define EVE_VAL_INT_CMDEMPTY		0x20		///< 					Interrupt Flag Bit 5 Command FIFO empty
	#define EVE_VAL_INT_PLAYBACK		0x10		///< 					Interrupt Flag Bit 4 Audio playback ended
	#define EVE_VAL_INT_SOUND			0x08		///< 					Interrupt Flag Bit 3 Sound effect ended
	#define EVE_VAL_INT_TAG			0x04		///< 					Interrupt Flag Bit 2 Touch-screen tag value change
	#define EVE_VAL_INT_TOUCH			0x02		///< 					Interrupt Flag Bit 1 Touch-screen touch detected
	#define EVE_VAL_INT_SWAP			0x01		///< 					Interrupt Flag Bit 0 Display list swap occurred
#define EVE_REG_INT_EN			0x003020AC		///< 1 Bits		r/w		Global interrupt enable
#define EVE_REG_INT_MASK			0x003020B0		///< 8 Bits		r/w		Interrupt enable mask
#define EVE_REG_PLAYBACK_START	0x003020B4		///< 20 Bits	r/w		Audio playback RAM start address
#define EVE_REG_PLAYBACK_LENGTH	0x003020B8		///< 20 Bits	r/w		Audio playback sample length
#define EVE_REG_PLAYBACK_READPTR	0x003020BC		///< 20 Bits	r/o		Audio playback current pointer
#define EVE_REG_PLAYBACK_FREQ		0x003020C0		///< 16 Bits	r/w		Audio playback sampling frequency (Hz)
#define EVE_REG_PLAYBACK_FORMAT	0x003020C4		///< 2 Bits		r/w		Audio playback format
#define EVE_REG_PLAYBACK_LOOP		0x003020C8		///< 1 Bits		r/w		Audio playback loop enable
#define EVE_REG_PLAYBACK_PLAY		0x003020CC		///< 1 Bits		r/o		Start audio playback
#define EVE_REG_PWM_HZ			0x003020D0		///< 14 Bits	r/w		Backlight PWM output frequency (Hz)
#define EVE_REG_PWM_DUTY			0x003020D4		///< 8 Bits		r/w		Backlight PWM output duty cycle 0 = 0%, 128 = 100%
#define EVE_REG_MACRO_0			0x003020D8		///< 32 Bits	r/w		Display list macro command 0
#define EVE_REG_MACRO_1			0x003020DC		///< 32 Bits	r/w		Display list macro command 1
// 0x003020D0 - 0x003020E0 reserved
#define EVE_REG_CMD_READ			0x003020F8		///< 12 Bits	r/w		Command buffer read pointer
#define EVE_REG_CMD_WRITE			0x003020FC		///< 12 Bits	r/w		Command buffer write pointer
#define EVE_REG_CMD_DL			0x00302100		///< 13 Bits	r/w		Command display list offset
#define EVE_REG_TOUCH_MODE		0x00302104		///< 2 Bits		r/w		Touch-screen sampling mode
	#define EVE_VAL_TOUCH_MODE_OFF		0x00	///<  	Acquisition stopped, only touch detection interrupt is still valid.
	#define EVE_VAL_TOUCH_MODE_ONE_SHOT	0x01	///< 	Perform acquisition once every time MPU write '1' to REG_TOUCH_MODE.
	#define EVE_VAL_TOUCH_MODE_FRAME_SYNC	0x02	///< 	Perform acquisition for every frame sync (~60 data acquisition/second.
	#define EVE_VAL_TOUCH_MODE_CONTINUOUS	0x03	///< 	Perform acquisition continuously at approximately 1000 data acquisition / second.
#define EVE_REG_TOUCH_ADC_MODE	0x00302108		///< 1 Bits		r/w		Select single ended (low power) or differential (accurate) sampling
#define EVE_REG_TOUCH_CHARGE		0x0030210C		///< 16 Bits	r/w		Touch-screen charge time, units of 6 clocks
#define EVE_REG_TOUCH_SETTLE		0x00302110		///< 4 Bits		r/w		Touch-screen settle time, units of 6 clocks
#define EVE_REG_TOUCH_OVERSAMPLE	0x00302114		///< 4 Bits		r/w		Touch-screen oversample factor
#define EVE_REG_TOUCH_RZTHRESH	0x00302118		///< 16 Bits	r/w		Touch-screen resistance threshold
#define EVE_REG_TOUCH_RAW_XY		0x0030211C		///< 32 Bits	r/o		Touch-screen raw (x-MSB16; y-LSB16)
#define EVE_REG_TOUCH_RZ			0x00302120		///< 16 Bits	r/o		Touch-screen resistance
#define EVE_REG_TOUCH_SCREEN_XY	0x00302124		///< 32 Bits	r/o		Touch-screen screen (x-MSB16; y-LSB16)
#define EVE_REG_TOUCH_TAG_XY		0x00302128		///< 32 Bits	r/o		Touch-screen screen (x-MSB16; y-LSB16) used for tag lookup
#define EVE_REG_TOUCH_TAG			0x0030212C		///< 8 Bits		r/o		Touch-screen tag result
#define EVE_REG_TOUCH_TAG1_XY		0x00302130		///< 32 Bits	r/o		Touch-screen screen (x-MSB16; y-LSB16) used for tag lookup
#define EVE_REG_TOUCH_TAG1		0x00302134		///< 8 Bits		r/o		Touch-screen tag result
#define EVE_REG_TOUCH_TAG2_XY		0x00302138		///< 32 Bits	r/o		Touch-screen screen (x-MSB16; y-LSB16) used for tag lookup
#define EVE_REG_TOUCH_TAG2		0x0030213C		///< 8 Bits		r/o		Touch-screen tag result
#define EVE_REG_TOUCH_TAG3_XY		0x00302140		///< 32 Bits	r/o		Touch-screen screen (x-MSB16; y-LSB16) used for tag lookup
#define EVE_REG_TOUCH_TAG3		0x00302144		///< 8 Bits		r/o		Touch-screen tag result
#define EVE_REG_TOUCH_TAG4_XY		0x00302148		///< 32 Bits	r/o		Touch-screen screen (x-MSB16; y-LSB16) used for tag lookup
#define EVE_REG_TOUCH_TAG4		0x0030214C		///< 8 Bits		r/o		Touch-screen tag result
#define EVE_REG_TOUCH_TRANSFORM_A	0x00302150		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)
#define EVE_REG_TOUCH_TRANSFORM_B	0x00302154		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)
#define EVE_REG_TOUCH_TRANSFORM_C	0x00302158		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)
#define EVE_REG_TOUCH_TRANSFORM_D	0x0030215C		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)
#define EVE_REG_TOUCH_TRANSFORM_E	0x00302160		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)
#define EVE_REG_TOUCH_TRANSFORM_F	0x00302164		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)

#define EVE_REG_TOUCH_CONFIG		0x00302168		///< 16 Bits	r/w		Touch configuration (RTP/CTP select)
#define EVE_REG_TOUCH_TOUCH4_X	0x0030216C		///< 16 Bits	r/o		Extended mode: touch-screen screen X data for touch 4
#define EVE_REG_BIST_EN			0x00302174		///< 1 Bits		r/w		BIST memory mapping enable
#define EVE_REG_TRIM				0x00302180		///< 8 Bits		r/w		Internal relaxation clock trimming
#define EVE_REG_ANA_COMP			0x00302184		///< 8 Bits		r/w		Analogue control register
#define EVE_REG_SPI_WIDTH			0x00302188		///< 3 Bits		r/w		QSPI bus width setting

#define EVE_REG_TOUCH_DIRECT_XY	0x0030218C		///< 32 Bits	r/o		Touch screen direct (x-MSB16; y-LSB16) conversions
#define EVE_REG_TOUCH_DIRECT_Z1Z2	0x00302190		///< 32 Bits	r/o		Touch screen direct (z1-MSB16; z2-LSB16) conversions

#define EVE_REG_DATESTAMP			0x00302564		///< 128 Bits	r/o		Stamp date code
#define EVE_REG_CMDB_SPACE		0x00302574		///< 12 Bits	r/w		Command DL (bulk) space available
#define EVE_REG_CMDB_WRITE		0x00302578		///< 32 Bits	w/o		Command DL (bulk) write

#if EVE_GENERATION > 2

/// Touch x coordinate
#define EVE_REG_EHOST_TOUCH_X			0x0030210C
/// Touch y coordinate 
#define EVE_REG_EHOST_TOUCH_Y			0x00302118
/// Touch ID / phase
#define EVE_REG_EHOST_TOUCH_ID			0x00302114
/// Acknowledgement
#define EVE_REG_EHOST_TOUCH_ACK			0x00302170

/// Touch screen sampling Mode 
#define EVE_REG_CTOUCH_MODE			0x00302104
/// Select ADC working mode
#define EVE_REG_CTOUCH_EXTENDED		0x00302108
/// Coordinate of first touch point 
#define EVE_REG_CTOUCH_TOUCH_XY		0x00302124
/// Coordinate of second touch point 
#define EVE_REG_CTOUCH_TOUCH1_XY	0x0030211C
/// Coordinate of third touch point
#define EVE_REG_CTOUCH_TOUCH2_XY	0x0030218C
/// Coordinate of fourth touch point
#define EVE_REG_CTOUCH_TOUCH3_XY	0x00302190
/// X Coordinate of fifth touch point
#define EVE_REG_CTOUCH_TOUCH4_X		0x0030216C
/// Y Coordinate of fifth touch point
#define EVE_REG_CTOUCH_TOUCH4_Y		0x00302120
/// Coordinate used to calculate the tag of first touch point 
#define EVE_REG_CTOUCH_TAG_XY		0x00302128
/// Touch screen Tag result of fist touch point 
#define EVE_REG_CTOUCH_TAG			0x0030212C
/// XY used to tag of second touch point
#define EVE_REG_CTOUCH_TAG1_XY		0x00302130
/// Tag result of second touch point
#define EVE_REG_CTOUCH_TAG1			0x00302134
/// XY used to tag of third touch point
#define EVE_REG_CTOUCH_TAG2_XY		0x00302138
/// Tag result of third touch point
#define EVE_REG_CTOUCH_TAG2			0x0030213C
/// XY used to tag of fourth touch point
#define EVE_REG_CTOUCH_TAG3_XY		0x00302140
/// Tag result of fourth touch point
#define EVE_REG_CTOUCH_TAG3			0x00302144
/// XY used to tag of fifth touch point
#define EVE_REG_CTOUCH_TAG4_XY		0x00302148
/// Tag result of fifth touch point
#define EVE_REG_CTOUCH_TAG4			0x0030214C

#define EVE_REG_FLASH_STATUS		0x003025F0		

/// 2 Bits	r/w		Flash Status
#define EVE_REG_FLASH_STATUS		0x003025F0		
/// 32 Bits	r/o		Line underrun counter
#define EVE_REG_UNDERRUN			0x0030260C		
/// 12 Bits	r/w		Adaptive Hsync: maximum horizontal total PCLK cycles. 0 means Adaptive Hsync is disabled.
#define EVE_REG_AH_HCYCLE_MAX		0x00302610
/// 16 Bits	r/w		Fractional PCLK frequency/enable. Firmware sets to 08A1h at boot.
#define EVE_REG_PCLK_FREQ			0x00302614
/// 1 Bits	r/w		 Core scan out 2 pixel data per system clock
#define EVE_REG_PCLK_2X				0x00302618
/// 32 Bits	r/w		  Tracker register 0
#define EVE_REG_TRACKER				0x00309000
/// 32 Bits	r/w		  Tracker register 1
#define EVE_REG_TRACKER_1			0x00309004
/// 32 Bits	r/w		  Tracker register 2
#define EVE_REG_TRACKER_2			0x00309008
/// 32 Bits	r/w		  Tracker register 3
#define EVE_REG_TRACKER_3			0x0030900C
/// 32 Bits	r/w		  Tracker register 4
#define EVE_REG_TRACKER_4			0x00309010
/// 32 Bits	r/o		   Media FIFO read offset
#define EVE_REG_MEDIAFIFO_READ		0x00309014
/// 32 Bits	r/w		   Media FIFO write offset
#define EVE_REG_MEDIAFIFO_WRITE		0x00309018
/// 32 Bits	r/o		   Detected flash capacity, in Mbytes
#define EVE_REG_FLASH_SIZE			0x00309024
/// 32 Bits	r/o		   32-bit mask of currently playing animations
#define EVE_REG_ANIM_ACTIVE			0x0030902C
/// 8 Bits	r/w		    Video playback control. 0: pause, 1: play, ffh: exit
#define EVE_REG_PLAY_CONTROL		0x0030914E

#endif

#else // EVE Register Addresses

#define EVE_REG_ID				0x00102400		///< 8 Bits		r/o		Identification register, always reads as 0x7C
#define EVE_REG_FRAMES			0x00102404		///< 32 Bits	r/o		Frame counter, since reset
#define EVE_REG_CLOCK				0x00102408		///< 32 Bits	r/o		Clock cycles, since reset
#define EVE_REG_FREQUENCY			0x0010240C		///< 27 Bits	r/w		Main clock frequency
#define EVE_REG_RENDERMODE		0x00102410		///< 1 Bits		r/w		Rendering mode: 0 = normal, 1 = single-line
#define EVE_REG_SNAPY				0x00102414		///< 9 Bits		r/w		Scan line select for RENERMODE 1
#define EVE_REG_SNAPSHOT			0x00102418		///< 1 Bits		r/o		trigger for RENDERMODE 1
#define EVE_REG_CPURESET			0x0010241C		///< 1 Bits		r/w		Graphics, audio and touch engines reset control
#define EVE_REG_TAP_CRC			0x00102420		///< 32 Bits	r/o		Live video tap crc. Frame CRC is computed every DL SWAP.
#define EVE_REG_TAP_MASK			0x00102424		///< 1 Bits		r/w		Live video tap mask
#define EVE_REG_HCYCLE			0x00102428		///< 10 Bits	r/w		Horizontal total cycle count
#define EVE_REG_HOFFSET			0x0010242C		///< 10 Bits	r/w		Horizontal display start offset
#define EVE_REG_HSIZE				0x00102430		///< 10 Bits	r/w		Horizontal display pixel count
#define EVE_REG_HSYNC0			0x00102434		///< 10 Bits	r/w		Horizontal sync fall offset
#define EVE_REG_HSYNC1			0x00102438		///< 10 Bits	r/w		Horizontal sync rise offset
#define EVE_REG_VCYCLE			0x0010243C		///< 10 Bits	r/w		Vertical total cycle count
#define EVE_REG_VOFFSET			0x00102440		///< 10 Bits	r/w		Vertical display start offset
#define EVE_REG_VSIZE				0x00102444		///< 10 Bits	r/w		Vertical display line count
#define EVE_REG_VSYNC0			0x00102448		///< 10 Bits	r/w		Vertical sync fall offset
#define EVE_REG_VSYNC1			0x0010244C		///< 10 Bits	r/w		Vertical sync rise offset
#define EVE_REG_DLSWAP			0x00102450		///< 2 Bits		r/w		Display list swap control
	#define EVE_VAL_DLSWAP_DONE          0UL
	#define EVE_VAL_DLSWAP_FRAME         2UL
	#define EVE_VAL_DLSWAP_LINE          1UL
#define EVE_REG_ROTATE			0x00102454		///< 1 Bits		r/w		Screen 180 degree rotate
#define EVE_REG_OUTBITS			0x00102458		///< 9 Bits		r/w		Output bit resolution, 3 x 3 x 3 bits
#define EVE_REG_DITHER			0x0010245C		///< 1 Bits		r/w		Output dither enable
#define EVE_REG_SWIZZLE			0x00102460		///< 4 Bits		r/w		Output RGB signal swizzle
#define EVE_REG_CSPREAD			0x00102464		///< 1 Bits		r/w		Output clock spreading enable
#define EVE_REG_PCLK_POL			0x00102468		///< 1 Bits		r/w		PCLK polarity:
													///<						0 = output on PCLK rising edge,
													///<						1 = output on PCLK falling edge
#define EVE_REG_PCLK				0x0010246C		///< 8 Bits		r/w		PCLK frequency divider, 0 = disable
#define EVE_REG_TAG_X				0x00102470		///< 9 Bits		r/w		Tag query X coordinate
#define EVE_REG_TAG_Y				0x00102474		///< 9 Bits		r/w		Tag query Y coordinate
#define EVE_REG_TAG				0x00102478		///< 8 Bits		r/o		Tag query result
#define EVE_REG_VOL_PB			0x0010247C		///< 8 Bits		r/w		Volume for playback
#define EVE_REG_VOL_SOUND			0x00102480		///< 8 Bits		r/w		Volume for synthesizer sound
#define EVE_REG_SOUND				0x00102484		///< 16 Bits	r/w		Sound effect select
#define EVE_REG_PLAY				0x00102488		///< 1 Bits		r/w		Start effect playback
#define EVE_REG_GPIO_DIR			0x0010248C		///< 8 Bits		r/w		GPIO pin direction, 0 = input, 1 = output
#define EVE_REG_GPIO				0x00102490		///< 8 Bits		r/w		GPIO pin value (bit 0, 1, 7); output pin drive strength (bit 2-6)
// 0x00102494 is reserved
#define EVE_REG_INT_FLAGS			0x00102498		///< 8 Bits		r/o		Interrupt flags, cleared by read
	#define EVE_VAL_INT_CONVCOMPLETE	0x80		///< 					Interrupt Flag Bit 7 Touch-screen conversions completed
	#define EVE_VAL_INT_CMDFLAG		0x40		///< 					Interrupt Flag Bit 6 Command FIFO flag
	#define EVE_VAL_INT_CMDEMPTY		0x20		///< 					Interrupt Flag Bit 5 Command FIFO empty
	#define EVE_VAL_INT_PLAYBACK		0x10		///< 					Interrupt Flag Bit 4 Audio playback ended
	#define EVE_VAL_INT_SOUND			0x08		///< 					Interrupt Flag Bit 3 Sound effect ended
	#define EVE_VAL_INT_TAG			0x04		///< 					Interrupt Flag Bit 2 Touch-screen tag value change
	#define EVE_VAL_INT_TOUCH			0x02		///< 					Interrupt Flag Bit 1 Touch-screen touch detected
	#define EVE_VAL_INT_SWAP			0x01		///< 					Interrupt Flag Bit 0 Display list swap occurred
#define EVE_REG_INT_EN			0x0010249C		///< 1 Bits		r/w		Global interrupt enable
#define EVE_REG_INT_MASK			0x001024A0		///< 8 Bits		r/w		Interrupt enable mask
#define EVE_REG_PLAYBACK_START	0x001024A4		///< 20 Bits	r/w		Audio playback RAM start address
#define EVE_REG_PLAYBACK_LENGTH	0x001024A8		///< 20 Bits	r/w		Audio playback sample length
#define EVE_REG_PLAYBACK_READPTR	0x001024AC		///< 20 Bits	r/o		Audio playback current pointer
#define EVE_REG_PLAYBACK_FREQ		0x001024B0		///< 16 Bits	r/w		Audio playback sampling frequency (Hz)
#define EVE_REG_PLAYBACK_FORMAT	0x001024B4		///< 2 Bits		r/w		Audio playback format
#define EVE_REG_PLAYBACK_LOOP		0x001024B8		///< 1 Bits		r/w		Audio playback loop enable
#define EVE_REG_PLAYBACK_PLAY		0x001024BC		///< 1 Bits		r/o		Start audio playback
#define EVE_REG_PWM_HZ			0x001024C0		///< 14 Bits	r/w		Backlight PWM output frequency (Hz)
#define EVE_REG_PWM_DUTY			0x001024C4		///< 8 Bits		r/w		Backlight PWM output duty cycle 0 = 0%, 128 = 100%
#define EVE_REG_MACRO_0			0x001024C8		///< 32 Bits	r/w		Display list macro command 0
#define EVE_REG_MACRO_1			0x001024CC		///< 32 Bits	r/w		Display list macro command 1
// 0x001024D0 - 0x001024E0 reserved
#define EVE_REG_CMD_READ			0x001024E4		///< 12 Bits	r/w		Command buffer read pointer
#define EVE_REG_CMD_WRITE			0x001024E8		///< 12 Bits	r/w		Command buffer write pointer
#define EVE_REG_CMD_DL			0x001024EC		///< 13 Bits	r/w		Command display list offset
#define EVE_REG_TOUCH_MODE		0x001024F0		///< 2 Bits		r/w		Touch-screen sampling mode
	#define EVE_VAL_TOUCH_MODE_OFF		0x00	///<  	Acquisition stopped, only touch detection interrupt is still valid.
	#define EVE_VAL_TOUCH_MODE_ONE_SHOT	0x01	///< 	Perform acquisition once every time MPU write '1' to REG_TOUCH_MODE.
	#define EVE_VAL_TOUCH_MODE_FRAME_SYNC	0x02	///< 	Perform acquisition for every frame sync (~60 data acquisition/second.
	#define EVE_VAL_TOUCH_MODE_CONTINUOUS	0x03	///< 	Perform acquisition continuously at approximately 1000 data acquisition / second.
#define EVE_REG_TOUCH_ADC_MODE	0x001024F4		///< 1 Bits		r/w		Select single ended (low power) or differential (accurate) sampling
#define EVE_REG_TOUCH_CHARGE		0x001024F8		///< 16 Bits	r/w		Touch-screen charge time, units of 6 clocks
#define EVE_REG_TOUCH_SETTLE		0x001024FC		///< 4 Bits		r/w		Touch-screen settle time, units of 6 clocks
#define EVE_REG_TOUCH_OVERSAMPLE	0x00102500		///< 4 Bits		r/w		Touch-screen oversample factor
#define EVE_REG_TOUCH_RZTHRESH	0x00102504		///< 16 Bits	r/w		Touch-screen resistance threshold
#define EVE_REG_TOUCH_RAW_XY		0x00102508		///< 32 Bits	r/o		Touch-screen raw (x-MSB16; y-LSB16)
#define EVE_REG_TOUCH_RZ			0x0010250C		///< 16 Bits	r/o		Touch-screen resistance
#define EVE_REG_TOUCH_SCREEN_XY	0x00102510		///< 32 Bits	r/o		Touch-screen screen (x-MSB16; y-LSB16)
#define EVE_REG_TOUCH_TAG_XY		0x00102514		///< 32 Bits	r/o		Touch-screen screen (x-MSB16; y-LSB16) used for tag lookup
#define EVE_REG_TOUCH_TAG			0x00102518		///< 8 Bits		r/o		Touch-screen tag result
#define EVE_REG_TOUCH_TRANSFORM_A	0x0010251C		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)
#define EVE_REG_TOUCH_TRANSFORM_B	0x00102520		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)
#define EVE_REG_TOUCH_TRANSFORM_C	0x00102524		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)
#define EVE_REG_TOUCH_TRANSFORM_D	0x00102528		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)
#define EVE_REG_TOUCH_TRANSFORM_E	0x0010252C		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)
#define EVE_REG_TOUCH_TRANSFORM_F	0x00102530		///< 32 Bits	r/w		Touch-screen transform coefficient (s15.16)
// 0x00102534 - 0x00102570 reserved
#define EVE_REG_TOUCH_DIRECT_XY	0x00102574		///< 32 Bits	r/o		Touch screen direct (x-MSB16; y-LSB16) conversions
#define EVE_REG_TOUCH_DIRECT_Z1Z2	0x00102578		///< 32 Bits	r/o		Touch screen direct (z1-MSB16; z2-LSB16) conversions
#define EVE_REG_TRACKER			0x00109000		///< 32 Bits	r/w		Track register (Track value - MSB16; Tag value - LSB8)

#endif

#endif /* EVE_REGISTER_H_ */

#endif
