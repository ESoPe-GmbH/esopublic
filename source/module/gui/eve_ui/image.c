/***
 * @file image.c
 * @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "image.h"


#include "../eve/eve.h"
#include "../eve/eve_spi.h"
#include "../eve/eve_copro.h"
#include "../eve/eve_memory.h"
#include "screen.h"
#if IMAGE_MMC_READ_BUFFER_SIZE > 0
#include "module/mmc/mmc.h"
#endif
#include "module/convert/math.h"
#include "module/convert/string.h"
#include "module/comm/dbg.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief 	Sets the fileformat variable based on the file extension of the saved filename.
 *
 * @param obj		Pointer to the image object.
 */
static void image_parse_fileformat(image_t* obj);

/**
 * @brief	Paints the image to the screen if it was loaded from the sd card before.
 *
 * @param obj		Pointer to the image object
 * @param p			Coordinate of the parent component
 */
static void image_paint(image_t* obj, eve_ui_point_t p);

/**
 * @brief	Calculates the stride based on the format and image width.
 *
 * @param obj		Pointer to the image object
 */
static void image_calculate_stride(image_t* obj);

/**
 * Changes the pressed status of the image. The function should only be used internally in the eve module.
 *
 * @param obj			Pointer to the image object.
 * @param b				true: Image is pressed, false: Image is not pressed.
 */
static void image_set_pressed(image_t* obj, bool b);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

bool image_init_from_mmc(image_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, IMAGE_FORMAT_T format, const char* filename)
{
	if(obj == NULL)
		return false;

	component_init((component_t*)obj, COMPONENT_TYPE_IMAGE, (component_paint_cb_t)image_paint);
	obj->component.origin.x = x;
	obj->component.origin.y = y;
	obj->raw_w = width;
	obj->raw_h = height;
	obj->component.size.width = width;
	obj->component.size.height = height;
	obj->format = format;
	obj->filename = (char*)filename;
	obj->component.is_visible = true;
	obj->buffer_ptr = NULL;
	obj->buffer_length = 0;
	obj->pressed = false;
	obj->action_callback = NULL;
	obj->component.pressed_callback = (component_pressed_cb_t)image_set_pressed;
	obj->scale_x = 1.0;
	obj->scale_y = 1.0;

	image_parse_fileformat(obj);
	image_calculate_stride(obj);

	if(obj->fileformat == IMAGE_FILEFORMAT_JPG)
	{		
		// TODO: Load filesize from mmc
		// obj->component.mem_file_ptr = eve_memory_register(&screen_get_default_device()->eve, filename, (const uint8_t*)obj->buffer_ptr, obj->buffer_length);
	}
	else
	{
		obj->component.mem_file_ptr = eve_memory_register(&screen_get_default_device()->eve, filename, NULL, (obj->stride * obj->component.size.height));
	}
	
	return (obj->component.mem_file_ptr != NULL);
}

bool image_init_from_flash(image_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, IMAGE_FORMAT_T format, const char* filename, const uint8_t* buffer_ptr, uint32_t buffer_length)
{
	if(obj == NULL)
		return false;

	component_init((component_t*)obj, COMPONENT_TYPE_IMAGE, (component_paint_cb_t)image_paint);

	obj->component.origin.x = x;
	obj->component.origin.y = y;
	obj->raw_w = width;
	obj->raw_h = height;
	obj->component.size.width = width;
	obj->component.size.height = height;
	obj->format = format;
	obj->filename = (char*)filename;
	obj->component.is_visible = true;
	obj->buffer_ptr = (uint8_t*)buffer_ptr;
	obj->buffer_length = buffer_length;
	obj->pressed = false;
	obj->action_callback = NULL;
	obj->component.pressed_callback = (component_pressed_cb_t)image_set_pressed;
	obj->scale_x = 1.0;
	obj->scale_y = 1.0;

	image_parse_fileformat(obj);
	image_calculate_stride(obj);

	if(obj->fileformat == IMAGE_FILEFORMAT_JPG || obj->fileformat == IMAGE_FILEFORMAT_PNG)
	{		
		obj->component.mem_file_ptr = eve_memory_register(&screen_get_default_device()->eve, filename, (const uint8_t*)obj->buffer_ptr, obj->buffer_length);
	}
	else
	{
		obj->component.mem_file_ptr = eve_memory_register(&screen_get_default_device()->eve, filename, (uint8_t*)buffer_ptr, (obj->stride * obj->component.size.height));
	}

	return (obj->component.mem_file_ptr != NULL);
}

void image_set_visible(image_t* obj, bool b)
{
	if(obj == NULL)
		return;

	obj->component.is_visible = b;
}

void image_set_action(image_t* obj, void(*pressed_callback)(image_t*))
{
	if(obj == NULL)
		return;

	obj->action_callback = (void(*)(void*))pressed_callback;
}

void image_set_scalef(image_t* obj, float scale)
{
	if(obj == NULL)
		return;

	obj->scale_x = (obj->raw_w / scale) / obj->raw_w;
	obj->scale_y = (obj->raw_h / scale) / obj->raw_h;

	obj->component.size.width = obj->raw_w / scale;
	obj->component.size.height = obj->raw_h / scale;
}

void image_set_scale(image_t* obj, uint16_t width, uint16_t height)
{
	if(obj == NULL)
		return;

	obj->component.size.width = width;
	obj->component.size.height = height;

	// obj->scale_x = (double)width / (double)obj->raw_w;
	// obj->scale_y = (double)height / (double)obj->raw_h;
	obj->scale_x = (double)obj->raw_w / (double)width;
	obj->scale_y = (double)obj->raw_h / (double)height;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void image_parse_fileformat(image_t* obj)
{
	if(obj->filename == NULL)
	{
		obj->fileformat = IMAGE_FILEFORMAT_INVALID;
		return;
	}

	if(string_ends_with(obj->filename, ".raw"))
		obj->fileformat = IMAGE_FILEFORMAT_RAW;
	else if(string_ends_with(obj->filename, ".bin"))
		obj->fileformat = IMAGE_FILEFORMAT_BIN;
	else if(string_ends_with(obj->filename, ".jpg") || string_ends_with(obj->filename, ".jpeg"))
		obj->fileformat = IMAGE_FILEFORMAT_JPG;
	else if(string_ends_with(obj->filename, ".png"))
		obj->fileformat = IMAGE_FILEFORMAT_PNG;
	else
		obj->fileformat = IMAGE_FILEFORMAT_INVALID;
}

static void image_paint(image_t* obj, eve_ui_point_t p)
{
	eve_t* eve = component_get_eve((component_t*)obj);
	
	if(eve == NULL || obj == NULL)
	{
#if IMAGE_DEBUG_ERROR
		dbg_printf(DBG_STRING, "Invalid Object (%8x / %8x)\n", eve, obj);
#endif
		return;
	}

	if(obj->action_callback != NULL)
		eve_copro_add_tag(eve, &obj->component);

	eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
	eve_copro_set_color(eve, color_get_argb(0xFF, 0xFF, 0xFF, 0xFF));
	eve_copro_write_command(eve, EVE_BITMAP_HANDLE(0));

#if EVE_USE_FT81X
	if(obj->fileformat == IMAGE_FILEFORMAT_JPG || obj->fileformat == IMAGE_FILEFORMAT_PNG)
	{
		eve_copro_loadimage(eve, obj->component.mem_file_ptr->address, 0, obj->component.mem_file_ptr->data, obj->component.mem_file_ptr->data_length);
		
		p = component_get_origin(&obj->component, p);

		eve_copro_write_command(eve, EVE_BEGIN(EVE_BITMAPS));		// Draw bitmap

		eve_copro_set_scale(eve, obj->scale_x, obj->scale_y);

		eve_copro_write_command(eve, EVE_VERTEX2F( p.x, p.y));
		eve_copro_write_command(eve, EVE_END());

		// uint32_t commands[3] = {
		// 	EVE_BEGIN(EVE_BITMAPS),
		// 	EVE_VERTEX2F(p.x, p.y),
		// 	EVE_END()
		// };

		// eve_copro_write_commands(eve, commands, 3);
	}
	else
#endif
	{
		if(!eve_memory_write_file_to(eve, obj->component.mem_file_ptr))
		{
	#if IMAGE_DEBUG_ERROR
			dbg_printf(DBG_STRING, "Cannot load image Object (%8x / %8x)\n", eve, obj);
	#endif
			return;
		}

		eve_copro_set_bitmap(eve, obj->component.mem_file_ptr->address, obj->format, obj->component.size.width, obj->component.size.height);
		// Set image on display
		eve_copro_write_command(eve, EVE_BEGIN(EVE_BITMAPS));		// Draw bitmap

		eve_copro_set_scale(eve, obj->scale_x, obj->scale_y);

		p = component_get_origin(&obj->component, p);

		eve_copro_write_command(eve, EVE_VERTEX2F( p.x, p.y));
		eve_copro_write_command(eve, EVE_END());
	}

	if(obj->scale_x != 1.0 || obj->scale_y != 1.0)
	{
		eve_copro_set_scale(eve, 1.0, 1.0);
	}

	if(obj->action_callback != NULL)
		eve_copro_clear_tag(eve);
}

static void image_calculate_stride(image_t* obj)
{
	switch(obj->format)
	{
		case IMAGE_FORMAT_ARGB1555:		obj->stride = obj->component.size.width * 2;		break;
		case IMAGE_FORMAT_ARGB2:		obj->stride = obj->component.size.width;			break;
		case IMAGE_FORMAT_ARGB4:		obj->stride = obj->component.size.width * 2;		break;
#if EVE_USE_FT81X
		case IMAGE_FORMAT_BARGRAPH:		obj->stride = obj->component.size.width;			break;
#endif
		case IMAGE_FORMAT_L1:			obj->stride = obj->component.size.width / 8;		break;
#if EVE_USE_FT81X
		case IMAGE_FORMAT_L2:			obj->stride = obj->component.size.width / 4;		break;		
#endif
		case IMAGE_FORMAT_L4:			obj->stride = obj->component.size.width / 2;		break;
		case IMAGE_FORMAT_L8:			obj->stride = obj->component.size.width;			break;
#if !EVE_USE_FT81X
		case IMAGE_FORMAT_PALETTED:		obj->stride = obj->component.size.width * 4;		break;
#endif
#if EVE_USE_FT81X
		case IMAGE_FORMAT_PALETTED565:	obj->stride = obj->component.size.width;			break;
		case IMAGE_FORMAT_PALETTED4444:	obj->stride = obj->component.size.width;			break;
		case IMAGE_FORMAT_PALETTED8:	obj->stride = obj->component.size.width;			break;
#endif
		case IMAGE_FORMAT_RGB332:		obj->stride = obj->component.size.width;			break;
		case IMAGE_FORMAT_RGB565:		obj->stride = obj->component.size.width * 2;		break;
#if EVE_USE_FT81X
		case IMAGE_FORMAT_TEXT8X8:		obj->stride = obj->component.size.width;			break;
		case IMAGE_FORMAT_TEXTVGA:		obj->stride = obj->component.size.width;			break;

		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_4x4_KHR:		obj->stride = obj->component.size.width;											break;		
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_5x4_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 6.40 / 8.0);			break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_5x5_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 5.12 / 8.0);			break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_6x5_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 4.27 / 8.0);			break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_6x6_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 3.56 / 8.0);			break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_8x5_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 3.20 / 8.0);			break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_8x6_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 2.67 / 8.0);			break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_8x8_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 2.00 / 8.0);			break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_10x5_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 2.56 / 8.0);		break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_10x6_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 2.13 / 8.0);		break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_10x8_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 1.60 / 8.0);		break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_10x10_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 1.28 / 8.0);		break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_12x10_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 1.07 / 8.0);		break;
		case IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_12x12_KHR:		obj->stride = (uint16_t)((float)obj->component.size.width * 0.89 / 8.0);		break;
#endif
		default:						obj->stride = 0; /* Invalid or unsupported*/break;
	}
}

static void image_set_pressed(image_t* obj, bool b)
{
	if(obj == NULL)
		return;

	if(!obj->component.is_enabled) // no action when not enabled
		return;

	if(obj->pressed == b)
		return;

	obj->pressed = b;

	if(!b)
	{
		// Trigger the action that was set for the button itself
		if(obj->action_callback != NULL)
			obj->action_callback(obj);
	}

	// Repaint screen to show the pressed color
	screen_repaint_by_component((component_t*)obj);
}

#endif

