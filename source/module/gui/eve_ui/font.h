/**
 * 	@file font.h
 *  @copyright Urheberrecht 2017-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		The font module loads fonts into the memory of the EVE
 *
 *  @version	1.00 (13.09.2017)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef FONT_H_
#define FONT_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "color.h"
#include "component.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define FONT_MMC_READ_BUFFER_SIZE		250		///< Number of bytes used in mmc buffer or 0 if only rom font is needed

#define FONT_DEBUG						false
#if FONT_DEBUG
#define FONT_DEBUG_INIT				true
#define FONT_DEBUG_ERROR				true
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @enum FONT_FORMAT
 * Is used to set the format of the font.
 */
typedef enum
{
	/// 1 bit per pixel: Black or white
	FONT_FORMAT_L1 = 1,

	/// 4 bit per pixel: Black or white with grey tones
	FONT_FORMAT_L4 = 2,

	/// 1 byte per pixel: Black or white with grey tones
	FONT_FORMAT_L8 = 3,

	/// 2 bit per pixel: Black or white with grey tones (FT81X only)
	FONT_FORMAT_L2 = 17

}FONT_FORMAT;

/**
 * @enum FONT_FILEFORMAT
 * Is used to set the fileformat which is needed to check if the font is compressed.
 */
typedef enum
{
	/// Default value: Fileformat is invalid until it is set.
	FONT_FILEFORMAT_INVALID,

	/// Raw fileformat is used for an uncompressed font.
	FONT_FILEFORMAT_RAW,
}FONT_FILEFORMAT;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Loads the fonts from the module/gui/fonts folder into the RAM of the EVE.
 * @TODO: Describe font names and numbers
 *
 * @param eve				Pointer to the EVE device
 */
void font_initalize_default_fonts(eve_t* eve);

/**
 * @brief	Initializes the font object and sets the parameters into it.
 * 			Starts loading the font from the rom into the eve ram.
 *
 * @attention 				Make sure to load all fonts before showing the first screen! Fonts will not be loaded dynamically,
 * 							therefore the memory manage must know the ram for the fonts at starting time.
 *
 * @param eve				Pointer to the EVE device
 * @param num				Num that is later used in text_set_font, etc. as the font number.
 * @param metric			Pointer to the 148-Byte metric data
 * @param data				Pointer to the font data
 * @param data_len			Number of bytes in the font data
 * @param first_character	Character, the fonts starts with. For example, if first character is the Space, this value must be 0x20.
 * @return					true: Image was loaded into the eve successfully.
 * 							false: Image could not be loaded into the eve.
 */
//bool font_init_from_flash(eve_t* eve, uint8_t num, uint8_t* metric, uint8_t* data, uint32_t data_len, uint8_t first_character);
bool font_init_from_flash(eve_t* eve, uint8_t num, const uint8_t* data, uint32_t data_len, uint8_t first_character);

/**
 * Returns the height of the font in pixel
 *
 * @param eve			Pointer to the eve
 * @param font			Font number (0-31)
 * @return				Height of the font in pixel
 */
uint16_t font_get_height(eve_t* eve, uint8_t font);

/**
 * Returns the width of the letter inside the font in pixel.
 *
 * @param eve			Pointer to the eve
 * @param font			Font number (0-31)
 * @param letter		Letter in the font to check
 * @return				Width of the letter inside the font in pixel.
 */
uint16_t font_get_width(eve_t* eve, uint8_t font, char letter);
/**
 * @brief Returns the width of a string inside the font in pixel.
 * 
 * @param eve			Pointer to the eve
 * @param font			Font number (0-31)
 * @param str           0-terminated string.
 * @return uint16_t 	Width of @c str inside the font in pixel.
 */
uint16_t font_get_width_string(eve_t* eve, uint8_t font, const char* str);

#endif

#endif /* FONT_H_ */
