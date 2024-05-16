/**
 * 	@file image.h
 *  @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief		The image module loads a jpeg from the sd card and writes it to the eve ram for decoding.
 *  			The whole module can be disabled when IMAGE_MMC_READ_BUFFER_SIZE is set to 0.
 *
 *  @version	1.00 (08.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef IMAGE_H_
#define IMAGE_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "color.h"
#include "component.h"
#include "../eve/eve.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//#define IMAGE_MMC_READ_BUFFER_SIZE		250		///< Number of bytes used in mmc buffer or 0 if only rom image is needed

#define IMAGE_DEBUG						true
#if IMAGE_DEBUG
#define IMAGE_DEBUG_INIT				true
#define IMAGE_DEBUG_ERROR				true
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @enum IMAGE_FORMAT
 * Is used to set the format of the image.
 */
typedef enum
{
	/// 2-byte per pixel: 1 Bit Alpha, 5-bit red, 5-bit green, 5-bit blue
	IMAGE_FORMAT_ARGB1555 = 0,

	/// 1 bit per pixel: Black or white
	IMAGE_FORMAT_L1 = 1,

	/// 4 bit per pixel: Black or white with grey tones
	IMAGE_FORMAT_L4 = 2,

	/// 1 byte per pixel: Black or white with grey tones
	IMAGE_FORMAT_L8 = 3,

	/// 1-byte per pixel: 3-bit red, 3-bit green, 2-bit blue
	IMAGE_FORMAT_RGB332 = 4,

	/// 1-byte per pixel: 2 Bit Alpha, 2-bit red, 2-bit green, 2-bit blue
	IMAGE_FORMAT_ARGB2 = 5,

	/// 2-byte per pixel: 4 Bit Alpha, 4-bit red, 4-bit green, 4-bit blue
	IMAGE_FORMAT_ARGB4 = 6,

	/// 2-byte per pixel: 5-bit red, 6-bit green, 5-bit blue
	IMAGE_FORMAT_RGB565 = 7,

	/// 4-byte per pixel: 8 Bit Alpha, 8-bit red, 8-bit green, 8-bit blue (FT80X only)
	IMAGE_FORMAT_PALETTED = 8,

	/// 2 bit per pixel: Black or white with grey tones (FT81X only)
	IMAGE_FORMAT_L2 = 9

}IMAGE_FORMAT;

/**
 * @enum IMAGE_FILEFORMAT
 * Is used to set the fileformat which is needed to check if the image is compressed.
 */
typedef enum
{
	/// Default value: Fileformat is invalid until it is set.
	IMAGE_FILEFORMAT_INVALID,

	/// Raw fileformat is used for an uncompressed image.
	IMAGE_FILEFORMAT_RAW,

	/// Bin fileformat is used for compressed zlib images that needs to be inflated by the eve. NOT SUPPORTED YET!
	IMAGE_FILEFORMAT_BIN,

	/// @brief JPG fileformat is used for compressed images that will be inflated by eve. Cannot have an alpha channel.
	IMAGE_FILEFORMAT_JPG,

	/// @brief PNG fileformat is used for compressed images that will be inflated by eve. Can have alpha channel. If any PNG is used, EVE uses last 40k Byte of RAM as buffer for inflation.
	IMAGE_FILEFORMAT_PNG

}IMAGE_FILEFORMAT;

/**
 * @struct image_t
 *
 * The image object is used to load the image from the file and to show it from the eve ram.
 */
typedef struct
{
	/// Component object of the image object.
	component_t component;

	/// Raw images width (without scaling)
	uint16_t raw_w;

	/// Raw images height (without scaling
	uint16_t raw_h;

	/// Format of the image.
	IMAGE_FORMAT format;

	/// Format of the file indicating if the content is compressed.
	IMAGE_FILEFORMAT fileformat;

	/// Pointer to the filename of the image file.
	char* filename;

//	/// Pointer of the image inside eve ram. Is needed for the painting.
//	uint32_t ptr;

	/// In case of a rom image, the pointer points to the buffer where the data is stored.
	uint8_t* buffer_ptr;

	/// In case of a rom image, the length contains the size of the buffer that stores the image.
	uint32_t buffer_length;

	/// Stride is a value related to the width and image format.
	uint16_t stride;

//	/// Is set to true when the image is stored successfully inside eve ram.
//	bool loaded;

//	/// Internal function used for loading the image. Might be different whether the image is loaded from rom or mmc.
//	bool (*load_image)(eve_t* eve, void* obj);

	///< true: Image is pressed, false: Image is not pressed.
	bool pressed;

	void(*action_callback)(void*);			///< Pointer to the function that is called when the button is pressed.
											///< The parameter is a pointer to the button_t that was pressed.
											///< void* is a placeholder because this button_t structure cannot use
											///< itself inside.

//	/// Object for the reset action, which is used to reload the images into the eve ram when the eve is
//	/// re-initialized.
//	eve_reset_action_t reset_action;

	/// Scaling factor for the image. 256 is no scaling, < 256 is upscaling, > 256 is downscaling. Format ist 8.8 Bit
	uint16_t scale_x;

	/// Scaling factor for the image. 256 is no scaling, < 256 is upscaling, > 256 is downscaling. Format ist 8.8 Bit
	uint16_t scale_y;

	/// Memory object for the image. Is needed for handling multiple images on the screen of the same image. Ensures
	/// that it is stored in memory only once.
//	eve_memory_file_t* mem;
}image_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief	Initializes the image object and sets the parameters into it.
 * 			Starts loading the image from sd card into the eve ram.
 *
 * @param obj				Pointer to the image object.
 * @param x					x-Coordinate of the image object.
 * @param y					y-Coordinate of the image object.
 * @param width				Width of the image in pixel.
 * @param height			Height of the image in pixel.
 * @param format			Bitmap format of the image.
 * @param filename			Pointer to the filename of the image file.
 * @return					true: Image was loaded into the eve successfully.
 * 							false: Image could not be loaded into the eve.
 **/
bool image_init_from_mmc(image_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, IMAGE_FORMAT format, const char* filename);

/**
 * @brief	Initializes the image object and sets the parameters into it.
 * 			Starts loading the image from the rom into the eve ram.
 *
 * @param obj				Pointer to the image object.
 * @param x					x-Coordinate of the image object.
 * @param y					y-Coordinate of the image object.
 * @param width				Width of the image in pixel.
 * @param height			Height of the image in pixel.
 * @param format			Bitmap format of the image.
 * @param filename			Pointer to the filename of the image file.
 * @param buffer_ptr		Pointer to the rom area where the image is located.
 * @param buffer_length		Length of the buffer that contains the image data (size of the image).
 * @return					true: Image was loaded into the eve successfully.
 * 							false: Image could not be loaded into the eve.
 */
bool image_init_from_flash(image_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, IMAGE_FORMAT format, const char* filename, const uint8_t* buffer_ptr, uint32_t buffer_length);

/**
 * @brief	Sets the image visible or invisible.
 *
 * @param obj				Pointer to the image object.
 * @param b					true: Image is drawn in screen_paint, false: Image is not drawn in screen_paint.
 */
void image_set_visible(image_t* obj, bool b);

/**
 * @brief	Sets a callback function that is called when the image is pressed.
 * 			The callback function has the image_t of the pressed image as a parameter.
 *
 *			If the pointer to the image object is NULL, nothing happens.
 *
 * @param obj					Pointer to the button object.
 * @param pressed_callback		Pointer to the callback function that has a pointer to the pressed image as a parameter.
 * 								To disable the callback, set it to NULL or disable it with component_set_enabled.
 */
void image_set_action(image_t* obj, void(*pressed_callback)(image_t*));

/**
 * @brief 	Sets the inverse scaling of the image in fixed 8.8bit notation
 * 			If you write the scaling as a hex number the encoding becomes more transparent:
 * 			0x0200 = 2.0 -> Half the original size
 * 			0x0201 = 2.00391
 * 			0x0080 = 0.5 -> Double the original size
 *
 *
 * @param obj					Pointer to the button object.
 * @param scale_factor			The scaling of the image in fixed 8.8bit notation (e.g 1.0 = 0x0100 = 256)
 */
void image_set_scale(image_t* obj, uint16_t width, uint16_t height);

#endif

#endif /* IMAGE_H_ */
