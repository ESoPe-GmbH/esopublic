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
 * @enum IMAGE_FORMAT_T
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

#if !EVE_USE_FT81X
	/// 4-byte per pixel: 8 Bit Alpha, 8-bit red, 8-bit green, 8-bit blue (FT80X only)
	IMAGE_FORMAT_PALETTED = 8,
#endif
#if EVE_USE_FT81X

	IMAGE_FORMAT_TEXT8X8 = 9,

	IMAGE_FORMAT_TEXTVGA = 10,

	IMAGE_FORMAT_BARGRAPH = 11,

	IMAGE_FORMAT_PALETTED565 = 14,

	IMAGE_FORMAT_PALETTED4444 = 15,

	IMAGE_FORMAT_PALETTED8 = 16,
	/// 2 bit per pixel: Black or white with grey tones (FT81X only)
	IMAGE_FORMAT_L2 = 17,
	/// @brief 8.0 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_4x4_KHR = 37808,
	/// @brief 6.40 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_5x4_KHR = 37809,
	/// @brief 5.12 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_5x5_KHR = 37810,
	/// @brief 4.27 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_6x5_KHR = 37811,
	/// @brief 3.56 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_6x6_KHR = 37812,
	/// @brief 3.20 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_8x5_KHR = 37813,
	/// @brief 2.67 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_8x6_KHR = 37814,
	/// @brief 2.00 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_8x8_KHR = 37815,
	/// @brief 2.56 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_10x5_KHR = 37816,
	/// @brief 2.13 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_10x6_KHR = 37817,
	/// @brief 1.60 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_10x8_KHR = 37818,
	/// @brief 1.28 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_10x10_KHR = 37819,
	/// @brief 1.07 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_12x10_KHR = 37820,
	/// @brief 0.89 Bits per Pixel
	IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_12x12_KHR = 37821,
#endif

}IMAGE_FORMAT_T;

/**
 * @enum IMAGE_FILEFORMAT_T
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

}IMAGE_FILEFORMAT_T;

/**
 * @enum IMAGE_FILTER_T
 * Is used to specify the filter of the image.
 */
typedef enum
{
	/// Nearest-neighbor filter
	IMAGE_FILTER_NEAREST =	0,

	/// Bilinear filter
	IMAGE_FILTER_BILINEAR =	1,

}IMAGE_FILTER_T;

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
	IMAGE_FORMAT_T format;

	/// Format of the file indicating if the content is compressed.
	IMAGE_FILEFORMAT_T fileformat;

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

	/// Scaling factor for the image. (drawn width / source width)
	float scale_x;

	/// Scaling factor for the image.  (drawn height / source height)
	float scale_y;

	/// @brief Address of the image in external flash. This is used to show images that are stored in external flash.
	/// The address is set to 0 if the image is not stored in external flash. Since address 0 in external flash is always used for the blob, it is an invalid address.
	uint32_t address_flash;
	/// true: Image is copied from external flash to eve ram. This might be necessary for some big images. 
	/// false: Image is shown directly from external flash.
	bool copy_from_external_flash;	

	/// Memory object for the image. Is needed for handling multiple images on the screen of the same image. Ensures
	/// that it is stored in memory only once.
//	eve_memory_file_t* mem;

	/// Filter that should be used for this image
	IMAGE_FILTER_T filter;
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
bool image_init_from_mmc(image_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, IMAGE_FORMAT_T format, const char* filename);

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
bool image_init_from_flash(image_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, IMAGE_FORMAT_T format, const char* filename, const uint8_t* buffer_ptr, uint32_t buffer_length);
/**
 * @brief Initializes the image object and sets the parameters into it.
 * 			Takes data that exists in external flash and shows it from there.
 * 
 * @param obj				Pointer to the image object.
 * @param x					x-Coordinate of the image object.
 * @param y					y-Coordinate of the image object.
 * @param width				Width of the image in pixel.
 * @param height			Height of the image in pixel.
 * @param format			Bitmap format of the image.
 * @param filename			Pointer to the filename of the image file.
 * @param address 			Address of the image in external flash.
 * @return					true: Image was loaded into the eve successfully.
 * 							false: Image could not be loaded into the eve.
 */
bool image_init_from_external_flash(image_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, IMAGE_FORMAT_T format, const char* filename, uint32_t address);
/**
 * @brief	Sets the image visible or invisible.
 *
 * @param obj				Pointer to the image object.
 * @param b					true: Image is drawn in screen_paint, false: Image is not drawn in screen_paint.
 */
void image_set_visible(image_t* obj, bool b);

/**
 * @brief Sets the flag that indicates if the image is copied from external flash before it is shown.
 * 
 * In some cases the image is too big to be shown directly from external flash. In this case the image is copied into the eve ram before it is shown.
 * 
 * @param obj 				Pointer to the image object.
 * @param b 				true: Image is copied from external flash to eve ram, false: Image is shown directly from external flash.
 */
void image_set_copy_from_external_flash(image_t* obj, bool b);

/**
 * @brief	Sets a callback function that is called when the image is pressed.
 * 			The callback function has the image_t of the pressed image as a parameter.
 *
 *			If the pointer to the image object is NULL, nothing happens.
 *
 * @param obj					Pointer to the image object.
 * @param pressed_callback		Pointer to the callback function that has a pointer to the pressed image as a parameter.
 * 								To disable the callback, set it to NULL or disable it with component_set_enabled.
 */
void image_set_action(image_t* obj, void(*pressed_callback)(image_t*));

/**
 * @brief	Sets the scaling factor of the image.
 *
 * @param obj					Pointer to the image object.
 * @param scale					Scaling factor of the image. 1.0 = 100%
 */
void image_set_scalef(image_t* obj, float scale);

/**
 * @brief 	Sets the target size of the image that is shown. The source image is transformed to match this values.
 *
 * @param obj					Pointer to the image object.
 * @param width					Width of the image that is shown
 * @param width					Height of the image that is shown
 */
void image_set_scale(image_t* obj, uint16_t width, uint16_t height);

#endif

#endif /* IMAGE_H_ */
