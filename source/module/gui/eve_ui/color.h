/**
 * 	@file color.h
 *  @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			
 *
 *  @version	1.00 (09.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef COLOR_H_
#define COLOR_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Color definitions (X11 colors: http://en.wikipedia.org/wiki/X11_color_names)
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define COLOR_INDIAN_RED				205, 92, 92		///< Color Indian Red (X11). Values in format red, green, blue
#define COLOR_LIGHT_CORAL				240, 128, 128	///< Color light coral (X11). Values in format red, green, blue
#define COLOR_SALMON					250, 128, 114	///< Color Salmon (X11). Values in format red, green, blue
#define COLOR_DARK_SALMON				233, 150, 122	///< Color Dark salmon (X11). Values in format red, green, blue
#define COLOR_LIGHT_SALMON				255, 160, 122	///< Color Light salmon (X11). Values in format red, green, blue
#define COLOR_CRIMSON					220, 20, 60		///< Color Crimson (X11). Values in format red, green, blue
#define COLOR_RED						255, 0, 0		///< Color Red (X11). Values in format red, green, blue
#define COLOR_FIRE_BRICK				178, 34, 34		///< Color Fire Brick (X11). Values in format red, green, blue
#define COLOR_DARK_RED					139, 0, 0		///< Color Dark red (X11). Values in format red, green, blue
#define COLOR_PINK						255, 192, 203	///< Color Pink (X11). Values in format red, green, blue
#define COLOR_LIGHT_PINK				255, 182, 193	///< Color Light pink (X11). Values in format red, green, blue
#define COLOR_HOT_PINK					255, 105, 180	///< Color Hot pink (X11). Values in format red, green, blue
#define COLOR_DEEP_PINK					255, 20, 147	///< Color Deep pink (X11). Values in format red, green, blue
#define COLOR_MEDIUM_VIOLET_RED			199, 21, 133	///< Color Medium violet red (X11). Values in format red, green, blue
#define COLOR_PALE_VIOLET_RED			219, 112, 147	///< Color Pale violet red (X11). Values in format red, green, blue
#define COLOR_CORAL						255, 127, 80	///< Color Coral (X11). Values in format red, green, blue
#define COLOR_TOMATO					255, 99, 71		///< Color Tomato (X11). Values in format red, green, blue
#define COLOR_ORANGE_RED				255, 69, 0		///< Color Orange red (X11). Values in format red, green, blue
#define COLOR_DARK_ORANGE				255, 140, 0		///< Color Dark orange (X11). Values in format red, green, blue
#define COLOR_ORANGE					255, 165, 0		///< Color Orange (X11). Values in format red, green, blue
#define COLOR_GOLD						255, 215, 0		///< Color Gold (X11). Values in format red, green, blue
#define COLOR_YELLOW					255, 255, 0		///< Color Yellow (X11). Values in format red, green, blue
#define COLOR_LIGHT_YELLOW				255, 255, 224	///< Color Light yellow (X11). Values in format red, green, blue
#define COLOR_LEMON_CHIFFON				255, 250, 205	///< Color Lemon chiffon (X11). Values in format red, green, blue
#define COLOR_LIGHT_GOLDENROD_YELLOW	250, 250, 210	///< Color Light goldenrod yellow (X11). Values in format red, green, blue
#define COLOR_PAPAY_WHIP				255, 239, 213	///< Color Papay whip (X11). Values in format red, green, blue
#define COLOR_MOCCASIN					255, 228, 181	///< Color Moccasin (X11). Values in format red, green, blue
#define COLOR_PEACH_PUFF				255, 218, 185	///< Color Peach puff (X11). Values in format red, green, blue
#define COLOR_PALE_GOLDENROD			238, 232, 170	///< Color Pale goldenrod (X11). Values in format red, green, blue
#define COLOR_KHAKI						240, 230, 140	///< Color Khaki (X11). Values in format red, green, blue
#define COLOR_DARK_KHAKI				189, 183, 107	///< Color Dark Khaki (X11). Values in format red, green, blue
#define COLOR_LAVENDER					230, 230, 250	///< Color Lavender (X11). Values in format red, green, blue
#define COLOR_THISTLE					216, 191, 216	///< Color Thistle (X11). Values in format red, green, blue
#define COLOR_PLUM						221, 160, 221	///< Color Plum (X11). Values in format red, green, blue
#define COLOR_VIOLET					238, 130, 238	///< Color Violet (X11). Values in format red, green, blue
#define COLOR_ORCHID					218, 112, 214	///< Color Orchid (X11). Values in format red, green, blue
#define COLOR_FUCHSIA					255, 0, 255		///< Color Fuchsia (X11). Values in format red, green, blue
#define COLOR_MAGENTA					COLOR_FUCHSIA	///< Color Magenta (X11). Values in format red, green, blue
#define COLOR_MEDIUM_ORCHID				186, 85, 211	///< Color Medium Orchid (X11). Values in format red, green, blue
#define COLOR_MEDIUM_PURPLE				147, 112, 219	///< Color Medium purple (X11). Values in format red, green, blue
#define COLOR_BLUE_VIOLET				138, 43, 226	///< Color Blue violet (X11). Values in format red, green, blue
#define COLOR_DARK_VIOLET				148, 0, 211		///< Color Dark violet (X11). Values in format red, green, blue
#define COLOR_DARK_ORCHID				153, 50, 204	///< Color Dark orchid (X11). Values in format red, green, blue
#define COLOR_DARK_MAGENTA				139, 0, 139		///< Color Dark magenta (X11). Values in format red, green, blue
#define COLOR_PURPLE					128, 0, 128		///< Color Purple (X11). Values in format red, green, blue
#define COLOR_INDIGO					75, 0, 130		///< Color Indigo (X11). Values in format red, green, blue
#define COLOR_SLATE_BLUE				106, 90, 205	///< Color Slate blue (X11). Values in format red, green, blue
#define COLOR_DARK_SLATE_BLUE			72, 61, 139		///< Color Dark slate blue (X11). Values in format red, green, blue
#define COLOR_GREEN_YELLOW				173, 255, 47	///< Color Green yellow (X11). Values in format red, green, blue
#define COLOR_CHARTREUSE				127, 255, 0		///< Color Chartreuse (X11). Values in format red, green, blue
#define COLOR_LAWN_GREEN				124, 252, 0		///< Color Lawn green (X11). Values in format red, green, blue
#define COLOR_LIME						0, 255, 0		///< Color Lime (X11). Values in format red, green, blue
#define COLOR_LIME_GREEN				50, 205, 50		///< Color Lime green (X11). Values in format red, green, blue
#define COLOR_PALE_GREEN				152, 251, 152	///< Color Pale green (X11). Values in format red, green, blue
#define COLOR_LIGHT_GREEN				144, 238, 144	///< Color Light green (X11). Values in format red, green, blue
#define COLOR_MEDIUM_SPRING_GREEN		0, 250, 154		///< Color Medium Spring green (X11). Values in format red, green, blue
#define COLOR_SPRING_GREEN				0, 255, 127		///< Color Spring green (X11). Values in format red, green, blue
#define COLOR_MEDIUM_SEA_GREEN			60, 179, 133	///< Color Medium sea green (X11). Values in format red, green, blue
#define COLOR_SEA_GREEN					46, 139, 87		///< Color Sea green (X11). Values in format red, green, blue
#define COLOR_FOREST_GREEN				34, 139, 34		///< Color Forest Green (X11). Values in format red, green, blue
#define COLOR_GREEN						0, 255, 0		///< Color Green (X11). Values in format red, green, blue -> WRONG 255 -> 128

#define COLOR_BLUE						0, 0, 255		///< Color Blue (X11). Values in format red, green, blue
#define COLOR_ROYAL_BLUE				0x41, 0x69, 0xE1///< Color Royal Blue (X11). Values in format red, green, blue
#define COLOR_SKY_BLUE					0x87, 0xCE, 0xEB///< Color Sky Blue (X11). Values in format red, green, blue

#define COLOR_WHITE						255, 255, 255	///< Color White (X11). Values in format red, green, blue

#define COLOR_LIGHT_GRAY				211, 211, 211	///< Color Light grey (X11). Values in format red, green, blue
#define COLOR_SILVER					192, 192, 192	///< Color Silver (X11). Values in format red, green, blue
#define COLOR_DARK_GRAY					169, 169, 169	///< Color Dark Gray (X11). Values in format red, green, blue
#define COLOR_GRAY						128, 128, 128	///< Color Gray (X11). Values in format red, green, blue
#define COLOR_DIM_GRAY					105, 105, 105	///< Color Dim Gray (X11). Values in format red, green, blue

#define COLOR_BLACK						0, 0, 0			///< Color Black (X11). Values in format red, green, blue

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Color definitions (Custom colors added)
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define COLOR_RED_ESOPE					205, 22, 27
#define COLOR_10_EUR					255, 128, 128
#define COLOR_20_EUR					0, 102, 204
#define COLOR_1_EUR						255, 255, 102
#define COLOR_50_EUR					255, 211, 32
#define COLOR_100_EUR					102, 204, 0
#define COLOR_2_EUR						207, 231, 245

#define COLOR_DARK_YELLOW				192, 192, 0

#define COLOR_BACKGROUND_GREY			40, 40, 40
#define COLOR_INNER_GREY				126, 115, 95
#define COLOR_INNER_GREY_BTN			95, 86, 71
#define COLOR_TEXT_GREY					209, 204, 194
#define COLOR_SCROLLBAR_INNER			230, 225, 214

/// Parse the hex values from the config to the arguments for the color_rgb
#define COLOR_RGB_FROM_HEX(hex) ((hex >> 16) & 0xFF), ((hex >> 8) & 0xFF), (hex & 0xFF)

/// Parse the hex values from the config to the arguments for the color_argb
#define COLOR_ARGB_FROM_HEX(hex) ((hex >> 24) & 0xFF), ((hex >> 16) & 0xFF), ((hex >> 8) & 0xFF), (hex & 0xFF)

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @struct color_t
 *
 * Structure for colors (rgb). Can be initialized directly with a color. For example:
 * color_t c = {COLOR_SEA_GREEN};
 */
typedef struct
{
	uint8_t a;
	uint8_t r;	///< Red amount
	uint8_t g;	///< Green amount
	uint8_t b;	///< Blue amount
}color_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Function
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

color_t color_get(uint8_t r, uint8_t g, uint8_t b);

color_t color_get_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b);

#endif

#endif /* COLOR_H_ */
