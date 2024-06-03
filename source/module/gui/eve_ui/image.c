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

#if IMAGE_MMC_READ_BUFFER_SIZE > 0
//
///// File object used to load the image from sd card.
//static FIL image_file;
//
///// Buffer to store data loaded from sd card.
//static uint8_t image_read_buffer[IMAGE_MMC_READ_BUFFER_SIZE];

#endif

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

//#if IMAGE_MMC_READ_BUFFER_SIZE > 0
bool image_init_from_mmc(image_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, IMAGE_FORMAT format, const char* filename)
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
//	obj->loaded = false;
	obj->component.is_visible = true;
	obj->buffer_ptr = NULL;
	obj->buffer_length = 0;
//	obj->load_image = (bool(*)(eve_t*, void*))image_load_from_file;
	obj->pressed = false;
	obj->action_callback = NULL;
	obj->component.pressed_callback = (component_pressed_cb_t)image_set_pressed;
	obj->scale_x = 256;//1.0
	obj->scale_y = 256;//1.0

	image_parse_fileformat(obj);
	image_calculate_stride(obj);

	obj->component.mem_file_ptr = eve_memory_register(&screen_get_default_device()->eve, filename, NULL, (obj->stride * obj->component.size.height));

//	// Register reset action to reload the image to the ram of the eve.
//	obj->reset_action.f = (void(*)(void*,void*))image_load_from_file;
//	obj->reset_action.p1 = &screen_get_default_device()->eve;
//	obj->reset_action.p2 = obj;
//	eve_register_reset_action(obj->reset_action.p1, &obj->reset_action);

//	return image_load_from_file(&screen_get_default_device()->eve, obj);
	return (obj->component.mem_file_ptr != NULL);
}
//#endif

bool image_init_from_flash(image_t* obj, int32_t x, int32_t y, uint16_t width, uint16_t height, IMAGE_FORMAT format, const char* filename, const uint8_t* buffer_ptr, uint32_t buffer_length)
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
//	obj->loaded = false;
	obj->component.is_visible = true;
	obj->buffer_ptr = (uint8_t*)buffer_ptr;
	obj->buffer_length = buffer_length;
//	obj->load_image = (bool(*)(eve_t*, void*))image_load_from_flash;
	obj->pressed = false;
	obj->action_callback = NULL;
	obj->component.pressed_callback = (component_pressed_cb_t)image_set_pressed;
	obj->scale_x = 256;//1.0
	obj->scale_y = 256;//1.0

	image_parse_fileformat(obj);
	image_calculate_stride(obj);

	obj->component.mem_file_ptr = eve_memory_register(&screen_get_default_device()->eve, filename, (uint8_t*)buffer_ptr, (obj->stride * obj->component.size.height));

//	// Register reset action to reload the image to the ram of the eve.
//	obj->reset_action.f = (void(*)(void*,void*))image_load_from_flash;
//	obj->reset_action.p1 = &screen_get_default_device()->eve;
//	obj->reset_action.p2 = obj;
//	eve_register_reset_action(obj->reset_action.p1, &obj->reset_action);

//	return image_load_from_flash(&screen_get_default_device()->eve, obj);
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

void image_set_scale(image_t* obj, uint16_t width, uint16_t height)
{
	if(obj == NULL)
		return;

	obj->component.size.width = width;
	obj->component.size.height = height;

	obj->scale_x = 256;//1.0
	obj->scale_y = 256;//1.0

	if(width != obj->raw_w)
		obj->scale_x = 256.0 * ((double)obj->raw_w / (double)width);

	if(height != obj->raw_h)
		obj->scale_y = 256.0 * ((double)obj->raw_h / (double)height);
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
	else
		obj->fileformat = IMAGE_FILEFORMAT_INVALID;
}

//
//#if IMAGE_MMC_READ_BUFFER_SIZE > 0
//static bool image_load_from_file(eve_t* eve, image_t* obj)
//{
//	uint32_t fsize = 0;
//	uint32_t bytes_written = 0;
//	uint32_t fsize2 = 0;
//	bool ret_value = true;
//
//	dbg_printf(DBG_STRING, "image_load_from_file(%s)\n", obj->filename);
//
//	if(obj->fileformat == IMAGE_FILEFORMAT_INVALID)
//	{
//#if IMAGE_DEBUG_ERROR
//		dbg_printf(DBG_STRING, "Invalid Fileformat\n");
//#endif
//		return false;
//	}
//
//	if(obj->fileformat == IMAGE_FILEFORMAT_BIN)
//	{
//#if IMAGE_DEBUG_ERROR
//		dbg_printf(DBG_STRING, "Bin fileformat not supported yet\n");
//#endif
//		return false;
//	}
//
//	if(!mmc_is_inserted())
//	{
//#if IMAGE_DEBUG_ERROR
//		dbg_printf(DBG_STRING, "No SD-card inserted\n");
//#endif
//		return false;		// Cannot load image without mmc
//	}
//
//	if(FR_OK != mmc_open_existing_file(obj->filename, &image_file, OPEN_NORMAL))
//	{
//#if IMAGE_DEBUG_ERROR
//		dbg_printf(DBG_STRING, "File %s could not be opened\n");
//#endif
//		return false;		// Cannot load image when file cannot be opened
//	}
//
//	fsize = mmc_get_filesize(&image_file);
//	fsize2 = fsize;
//#if IMAGE_DEBUG_INIT
//	dbg_printf(DBG_STRING, "File %s opened: %d Bytes\n", obj->filename, fsize);
//#endif
//
//
//	if(fsize != (obj->stride * obj->component.size.height))
//	{
//#if IMAGE_DEBUG_ERROR
//		dbg_printf(DBG_STRING, "Filesize %s does not match the format!\n", obj->filename);
//#endif
//		mmc_close_file(&image_file);
//		return false;
//	}
//	if(!obj->loaded)
//	{
//		fsize2 = (fsize + 3) & 0xFFFFFFFC;	// 4 Byte alignment
//		obj->ptr = eve_memory_get_address(eve, fsize2);
//
//		if(obj->ptr == 0xFFFFFFFF)
//		{
//			mmc_close_file(&image_file);
//	#if IMAGE_DEBUG_ERROR
//			dbg_printf(DBG_STRING, "Image %s does not fit in memory\n", obj->filename);
//	#endif
//			return false;
//		}
//	}
//	#if IMAGE_DEBUG_INIT
//		dbg_printf(DBG_STRING, "Current Pointer: %u\n", obj->ptr);
//	#endif
//
//	// Start transmitting jpg
//	while(fsize > 0 && ret_value)
//	{
//		uint16_t len = IMAGE_MMC_READ_BUFFER_SIZE;
//		if(fsize < len)
//			len = fsize;
//		fsize -= len;
//
//		if(len != mmc_gets(&image_file, image_read_buffer, len))
//		{
//			mmc_close_file(&image_file);
//#if IMAGE_DEBUG_ERROR
//		dbg_printf(DBG_STRING, "Image %s has a read error\n", obj->filename);
//#endif
//			ret_value = false;
//			break;
//		}
//
//		eve_memory_write(eve, obj->ptr + bytes_written, image_read_buffer, len);
////		eve_spi_write_data(eve, obj->ptr + bytes_written, image_read_buffer, len);
//
//		bytes_written += len;
//	}
//
//	mmc_close_file(&image_file);
//
//#if IMAGE_DEBUG_INIT
//	dbg_printf(DBG_STRING, "Bytes loaded: %u\n", bytes_written);
//	dbg_printf(DBG_STRING, "Next Pointer: %u\n", screen_get_default_device()->eve.memory.ram_pointer);
//#endif
//
//	obj->loaded = ret_value;
//
//	if(obj->loaded)
//		eve_memory_register_address(eve, obj->ptr, fsize2);
//
//	return ret_value;
//}
//#endif
//
//static bool image_load_from_flash(eve_t* eve, image_t* obj)
//{
//	uint32_t fsize2 = 0;
//
//	dbg_printf(DBG_STRING, "image_load_from_flash(%s)\n", obj->filename);
//
//	if(obj->fileformat == IMAGE_FILEFORMAT_INVALID)
//	{
//#if IMAGE_DEBUG_ERROR
//		dbg_printf(DBG_STRING, "Invalid Fileformat\n");
//#endif
//		return false;
//	}
//
//	if(obj->fileformat == IMAGE_FILEFORMAT_BIN)
//	{
//#if IMAGE_DEBUG_ERROR
//		dbg_printf(DBG_STRING, "Bin fileformat not supported yet\n");
//#endif
//		return false;
//	}
//
//	if(!obj->loaded)
//	{
//		fsize2 = (obj->buffer_length + 3) & 0xFFFFFFFC;	// 4 Byte alignment
//		obj->ptr = eve_memory_get_address(eve, fsize2);
//
//		if(obj->ptr == 0xFFFFFFFF)
//		{
//	#if IMAGE_DEBUG_ERROR
//			dbg_printf(DBG_STRING, "Image %s does not fit in memory\n", obj->filename);
//	#endif
//			return false;
//		}
//	}
//
//#if IMAGE_DEBUG_INIT
//	dbg_printf(DBG_STRING, "Current Pointer: %u\n", obj->ptr);
//#endif
//
//	eve_memory_write_to(eve, obj->ptr, obj->buffer_ptr, obj->buffer_length);
//
//#if IMAGE_DEBUG_INIT
//	dbg_printf(DBG_STRING, "Bytes loaded: %u\n", obj->buffer_length);
//	dbg_printf(DBG_STRING, "Next Pointer: %u\n", screen_get_default_device()->eve.memory.ram_pointer);
//#endif
//
//	obj->loaded = true;
//	return true;
//}

static void image_paint(image_t* obj, eve_ui_point_t p)
{
	eve_t* eve = component_get_eve((component_t*)obj);
//#if IMAGE_DEBUG_ERROR
//		dbg_printf(DBG_STRING, "Image:%s, Pointer:%d\n", obj->filename, obj->ptr);	// TIKO: THAT IS NO ERROR PRINT!!!!!!!
//#endif
	if(eve == NULL || obj == NULL)
	{
#if IMAGE_DEBUG_ERROR
		dbg_printf(DBG_STRING, "Invalid Object (%8x / %8x)\n", eve, obj);
#endif
		return;
	}

	if(!eve_memory_write_file_to(eve, obj->component.mem_file_ptr))
	{
#if IMAGE_DEBUG_ERROR
		dbg_printf(DBG_STRING, "Cannot load image Object (%8x / %8x)\n", eve, obj);
#endif
		return;
	}
//
//	if(!eve_copro_loadimage(eve, obj))
//		return;

	if(obj->action_callback != NULL)
		eve_copro_add_tag(eve, &obj->component);
//
//	eve_copro_write_command(eve, EVE_COLOR_RGB(0xFF, 0xFF, 0xFF));
//	eve_copro_write_command(eve, EVE_COLOR_A(0xFF));
	eve_copro_set_color(eve, color_get_argb(0xFF, 0xFF, 0xFF, 0xFF));
	eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
	// Set image on display
	eve_copro_write_command(eve, EVE_BEGIN(EVE_BITMAPS));		// Draw bitmap
	eve_copro_write_command(eve, EVE_BITMAP_HANDLE(0));
	eve_copro_write_command(eve, EVE_BITMAP_SOURCE(obj->component.mem_file_ptr->address));
	eve_copro_write_command(eve, EVE_BITMAP_LAYOUT(obj->format, obj->stride, obj->raw_h));
	eve_copro_write_command(eve, EVE_BITMAP_SIZE(obj->filter, EVE_WRAP_BORDER, EVE_WRAP_BORDER, obj->component.size.width, obj->component.size.height));

	if(obj->scale_x != 256)//If scale is not 1.0
		eve_copro_write_command(eve, EVE_BITMAP_TRANSFORM_A_8_8(obj->scale_x));
	if(obj->scale_y != 256)//If scale is not 1.0
		eve_copro_write_command(eve, EVE_BITMAP_TRANSFORM_E_8_8(obj->scale_y));

	p = component_get_origin(&obj->component, p);

	eve_copro_write_command(eve, EVE_VERTEX2F( p.x, p.y));
	eve_copro_write_command(eve, EVE_END());

	if(obj->scale_x != 256)//If scale is not 1.0
		eve_copro_write_command(eve, EVE_BITMAP_TRANSFORM_A_8_8(256));
	if(obj->scale_y != 256)//If scale is not 1.0
		eve_copro_write_command(eve, EVE_BITMAP_TRANSFORM_E_8_8(256));

	if(obj->action_callback != NULL)
		eve_copro_clear_tag(eve);
}

static void image_calculate_stride(image_t* obj)
{
	switch(obj->format)
	{
		case IMAGE_FORMAT_ARGB1555:		obj->stride = obj->component.size.width * 2;		break;
		case IMAGE_FORMAT_L1:			obj->stride = obj->component.size.width / 8;		break;
		case IMAGE_FORMAT_L4:			obj->stride = obj->component.size.width / 2;		break;
		case IMAGE_FORMAT_L8:			obj->stride = obj->component.size.width;			break;
		case IMAGE_FORMAT_RGB332:		obj->stride = obj->component.size.width;			break;
		case IMAGE_FORMAT_ARGB2:		obj->stride = obj->component.size.width;			break;
		case IMAGE_FORMAT_ARGB4:		obj->stride = obj->component.size.width * 2;		break;
		case IMAGE_FORMAT_RGB565:		obj->stride = obj->component.size.width * 2;		break;
		case IMAGE_FORMAT_PALETTED:		obj->stride = obj->component.size.width * 4;		break;
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

