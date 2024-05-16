/**
 * 	@file eve_copro.h
 *  @copyright Urheberrecht 2015-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief
 *			Contains functions for co-processor commands of the eve chip. The function works together with
 *			source files from the gui module (for example the button module).
 *
 *  @version	1.02 (16.01.2015)
 *  	- Tags can be added and cleared via function. This is needed if a tag for a non-standard component is needed.
 *  @version	1.01 (06.01.2015)
 *  	- Added calibration function (Only for testing new displays, not for recalibration in products).
 *  @version	1.00 (08.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef EVE_COPRO_H_
#define EVE_COPRO_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "../eve_ui/switch_button.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @enum EVE_OPT_DIMENSION
 *
 * Contains the options for the dimension of components like buttons that can have a 3D effect.
 * The 3D effect is default, but the components can also be flat (2D).
 */
typedef enum
{
	EVE_OPT_DIMENSION_3D = 	0x0000,		///< Co-processor widget is drawn in 3D effect. The default option.
	EVE_OPT_DIMENSION_2D = 	0x0100		///< Co-processor widget is drawn without 3D effect.
}EVE_OPT_DIMENSION;

/**
 * @enum EVE_OPT_IMAGE
 *
 * Contains the options for loading images.
 */
typedef enum
{
	EVE_OPT_IMAGE_RGB565 = 	0,		///< Co-processor option to decode the JPEG image to RGB565 format
	EVE_OPT_IMAGE_MONO = 		1,		///< Co-processor option to decode the JPEG image to L8 format, i.e., monochrome
	EVE_OPT_IMAGE_NODL = 		2		///< No display list commands generated for bitmap decoded from JPEG image
}EVE_OPT_IMAGE;

/**
 * @enum EVE_OPT_TEXT
 *
 * Contains the options for the alignment in the text command.
 * Default is Left/Top alignment.
 */
typedef enum
{
	EVE_OPT_TEXT_DEFAULT = 	0x0000,		///< Co-processor widget is aligned Top/Left
	EVE_OPT_TEXT_CENTERX = 	0x0200,		///< Co-processor widget centers horizontally
	EVE_OPT_TEXT_CENTERY = 	0x0400,		///< Co-processor widget centers vertically
	EVE_OPT_TEXT_CENTER = 	0x0600,		///< Co-processor widget centers horizontally and vertically
	EVE_OPT_TEXT_RIGHTX = 	0x0800		///< The label on the Co-processor widget is right justified
}EVE_OPT_TEXT;

/**
 * @enum EVE_OPT_NUMBER
 *
 * Contains the options for the signedness and alignment in the number command.
 * Default is unsigned and Left/Top alignment.
 */
typedef enum
{
	EVE_OPT_NUMBER_DEFAULT = 	0x0000,		///< Co-processor widget is aligned Top/Left and unsigned.
	EVE_OPT_NUMBER_SIGNED = 	0x0100,		///< The number is treated as 32 bit signed integer
	EVE_OPT_NUMBER_CENTERX = 	0x0200,		///< Co-processor widget centers horizontally
	EVE_OPT_NUMBER_CENTERY = 	0x0400,		///< Co-processor widget centers vertically
	EVE_OPT_NUMBER_RIGHTX = 	0x0800		///< The label on the Co-processor widget is right justified
}EVE_OPT_NUMBER;

/**
 * @enum EVE_OPT_CLOCK
 *
 * Contains the options for the clock command to suppress the background and hands.
 * Default is a clock with a background, hour ticks and all hands.
 */
typedef enum
{
	EVE_OPT_CLOCK_DEFAULT = 	0x0000,		///< Co-processor widget has background and all hands and ticks.
	EVE_OPT_CLOCK_NOBACK = 	0x1000,		///< Co-processor widget has no background drawn
	EVE_OPT_CLOCK_NOTICKS = 	0x2000,		///< Co-processor clock widget is drawn without hour ticks.
	EVE_OPT_CLOCK_NOHM = 		0x4000,		///< Co-processor clock widget is drawn without hour and minutes hands, only seconds hand is drawn
	EVE_OPT_CLOCK_NOSECS = 	0x8000,		///< Co-processor clock widget is drawn without seconds hand
	EVE_OPT_CLOCK_NOHANDS = 	0xC000		///< Co-processor clock widget is drawn without hour, minutes and seconds hands
}EVE_OPT_CLOCK;

/**
 * @enum EVE_OPT_GAUGE
 *
 * Contains the options for the gauge command to suppress the background, ticks and pointer.
 * Default is a gauge with a background, all ticks and a pointer.
 */
typedef enum
{
	EVE_OPT_GAUGE_DEFAULT = 	0x0000,		///< Co-processor widget has background and all ticks.
	EVE_OPT_GAUGE_NOBACK = 	0x1000,		///< Co-processor widget has no background drawn
	EVE_OPT_GAUGE_NOTICKS = 	0x2000,		///< Gauge widget is drawn without major and minor ticks
	EVE_OPT_GAUGE_NOPOINTER = 0x4000		///< The Co-processor gauge has no pointer
}EVE_OPT_GAUGE;

/**
 * @enum eve_opt_playvideo_e
 * 
 * @brief Contains options for the playvideo command that starts playing a video.
 */
typedef enum eve_opt_playvideo_e
{
	/// @brief Default option
	EVE_OPT_PLAYVIDEO_DEFAULT = 0,
	/// @brief zoom the video so that it fills as much of the screen as possible
	EVE_OPT_PLAYVIDEO_FULLSCREEN = 8,
	/// @brief Instead of sourcing the AVI video data from the command buffer, source it from the media FIFO in RAM_G. 
	EVE_OPT_PLAYVIDEO_MEDIAFIFO = 16,
	/// @brief Source video data from flash.When flash is the source, call CMD_FLASHSOURCE. Before this command to specify the address. See CMD_FLASHSOURCE.
	EVE_OPT_PLAYVIDEO_FLASH = 64,
	/// @brief Synchronize video updates to the display blanking interval, avoiding horizontal tearing artifacts. 
	EVE_OPT_PLAYVIDEO_NOTEAR = 4,
	/// @brief Decode the audio data encoded in the data following, if any.
	EVE_OPT_PLAYVIDEO_SOUND = 32,
	/// @brief Append the video bitmap to an existing display list, instead of starting a new display list.
	EVE_OPT_PLAYVIDEO_OVERLAY = 128,
	/// @brief Will not change the current display list. There should already be a display list rendering the video bitmap. 
	EVE_OPT_PLAYVIDEO_NODL = 2,
}EVE_OPT_PLAYVIDEO_T;

/**
 * @brief Callback function for the touch event of the eve coprocessor.
 * 
 * @param obj			Pointer that was used to register the touch event.
 * @param is_touched	True if display is touched, false if finger was removed from touch.
 * @param x				x-Coordinate of the finger on the touch from the top left screen. Only valid if @c is_touched is true.
 * @param y				y-Coordinate of the finger on the touch from the top left screen. Only valid if @c is_touched is true.
 */
typedef void (*eve_copro_touch_cb_t)(void* obj, bool is_touched, int32_t x, int32_t y);

/**
 * @brief Structure for co-processor commands to eve chip.
 * @c command is a mandatory field that needs to be filled.
 * @c options is optional, but is used for many commands that have options like coordinates and such. If you put options into the buffer, you also need to
 * set the number of options in the buffer to @c num_options. Be aware not to use sizeof, because @c options is a 32-Bit value.
 * @c data is optional, but some commands have it for texts or loadimage for the content of the image. If you set data, make sure to also set @c sizeof_data to
 * the size of the data that needs to be written. In case of texts, make sure to include the 0-termination into the variable. 
 * Padding is not needed, because this is done internally.
 */
typedef struct eve_copro_command_s
{
	/// @brief Command that needs to be send.
	uint32_t command;
	/// @brief Array containing the options for the command.
	const uint32_t* options;
	/// @brief Number of options in the @c options array.
	uint32_t num_options;
	/// @brief Buffer for optional command data like the image data for loadimage or strings on text widgets.
	const uint8_t* data;
	/// @brief Number of bytes in @c data that needs to be written. Be aware that strings need to include the zero termination.
	uint32_t sizeof_data;
}eve_copro_command_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Reset the eve coprocessor, which might be necessary on faulty conditions.
 * 
 * @param eve 		Pointer to eve handler.
 */
void eve_copro_reset(eve_t* eve);
/**
 * @brief	Checks if there is a timeout for a touch release and releases it.
 */
void eve_copro_handle(eve_t* eve);

#if EVE_COPRO_USE_CALIBRATION
/**
 * @brief 	Calibrates the touch screen and prints the resulting register values via dbg_printf.
 * 			This function should only be used to retrieve the touch values of new displays!
 * 			It is not intended to use for recalibration. This function blocks until the calibration is finished and
 * 			does only print the values.
 */
void eve_copro_calibrate_touch(eve_t* eve);
#endif // EVE_COPRO_USE_CALIBRATION

/**
 * @brief	Sets a touch callback that is called if the screen is touched at any position.
 *
 * @param f		Pointer to the callback function for the screen touch event.
 */
void eve_copro_set_touch_callback(eve_t* eve, eve_copro_touch_cb_t f, void* f_obj);

void eve_copro_set_key_callback(eve_t* eve, void(*)(void*, char), void*);

/**
 * @brief	Only repaint the screen when this function returns true. The screen_handle function in gui/screen.c makes use of
 * 			this function by only repainting the screen if this function returns true
 */
bool eve_copro_is_ready(eve_t* eve);

/**
 * @brief	Writes a 32-bit command into the command buffer and increments the command write pointer.
 */
void eve_copro_write_command(eve_t* eve, uint32_t val);

/**
 * @brief 	Writes multiple coprocessor commands at once.
 * 
 * @param eve 			Pointer to the eve context.
 * @param commands 		Array containing multiple coprocessor commands that should be written.
 * @param num_commands 	Number of commands inside the array.
 */
void eve_copro_write_commands(eve_t* eve, uint32_t* commands, uint32_t num_commands);

/**
 * @brief 	Writes a command with its options and optional data into the command buffer of the eve chip.
 * 
 * @param eve 		Pointer to the eve context.
 * @param command 	Pointer to the structure containing the command data.
 */
void eve_copro_write_command_data(eve_t* eve, const eve_copro_command_t* command);
/**
 * @brief Checks if coprocessor list is empty. Is non-blocking.
 * 
 * @param eve 		Pointer to the eve context.
 * @return true 	Coprocessor list is empty
 * @return false 	Coprocessor list is not empty, therefore coprocessor is doing something.
 */
bool eve_copro_has_empty_list(eve_t* eve);
/**
 * @brief 	Waits internally until the coprocessor commands were executed.
 * 
 * @param eve 		Pointer to the eve context.
 * @retval FUNCTION_RETURN_OK				All co-processor commands were processed successfully.
 * @retval FUNCTION_RETURN_TIMEOUT			While waiting for the execution, the coprocessor did not finish.
 * @retval FUNCTION_RETURN_DEVICE_RESET		The co-processor entered a faulty condition and was reset. Last commands might not have been processed.
 * @retval FUNCTION_RETURN_PARAM_ERROR		Eve pointer is null.
 */
FUNCTION_RETURN eve_copro_wait_for_execution(eve_t* eve);

/**
 * @brief 	When the co-processor engine executes this command, it waits until the current display list is scanned out,
 * 			then sets REG_CMD_DL to zero. Afterwards the new screen is cleared to the color in the parameter
 */
void eve_copro_dlstart(eve_t* eve, color_t c);

/**
 * @brief	This command sets co-processor engine to reset default states.
 */
void eve_copro_coldstart(eve_t* eve);

/**
 * @brief 	When the co-processor engine executes this command, it requests a display list swap immediately after current
 * 			display list is scanned out. Internally, the co-processor engine implements this command by writing to REG_DLSWAP.
 * 			Please see REG_DLSWAP Definition.
 *
 * 			This function sets also the write pointer in the EVE_REG_CMD_WRITE to the current pointer value.
 */
void eve_copro_swap(eve_t* eve);

// TODO: Description
void eve_copro_set_bitmap(eve_t* eve, uint32_t address, uint32_t format, uint16_t width, uint16_t height);

// TODO: Description
void eve_copro_loadimage(eve_t* eve, uint32_t ptr, EVE_OPT_IMAGE opt_image, const uint8_t* data, uint32_t length);
/**
 * @brief Erases the complete flash chip.
 * 
 * @param eve 	Pointer to the eve handler.
 */
void eve_copro_flash_erase(eve_t* eve);

void eve_copro_flash_write(eve_t* eve, uint32_t ptr, const uint8_t* data, uint32_t length);

void eve_copro_flash_write_from_ram(eve_t* eve, uint32_t ptr_ram, uint32_t ptr_flash, uint32_t length);

void eve_copro_flash_read_to_ram(eve_t* eve, uint32_t ptr_ram, uint32_t ptr_flash, uint32_t length);

void eve_copro_appendf(eve_t* eve, uint32_t ptr_flash, uint32_t length);

void eve_copro_flash_update_from_ram(eve_t* eve, uint32_t ptr_ram, uint32_t ptr_flash, uint32_t length);

void eve_copro_flash_detach(eve_t* eve);

void eve_copro_flash_attach(eve_t* eve);

uint16_t eve_copro_flash_fast(eve_t* eve);

void eve_copro_flashsource(eve_t* eve, uint32_t ptr_flash);

/**
 * @brief Starts playing a video. Use options to specify the source for the video. It can either be played from flash, from provided data or from media fifo.
 * 
 * @param eve			Pointer to the EVE device
 * @param options 		Options for the playback, can be ored.
 * @param data			Pointer to the data of the video in local flash or RAM. Can be NULL, if options are flash or mediafifo
 * @param length		Number of bytes in data.
 */
void eve_copro_playvideo(eve_t* eve, EVE_OPT_PLAYVIDEO_T options, const uint8_t* data, size_t length);
/**
 * @brief This command returns the first unallocated memory location.
 * 
 * At API level 1, the allocation pointer is advanced by the following commands:
 * • cmd_inflate
 * • cmd_inflate2
 * 
 * At API level 2, the allocation pointer is also advanced by: 
 * • cmd_loadimage
 * • cmd_playvideo
 * • cmd_videoframe
 * • cmd_endlist
 * 
 * @param eve			Pointer to the EVE device
 * @return uint32_t 	The first unallocated memory location.
 */
uint32_t eve_copro_getptr(eve_t* eve);
/**
 * @brief This command returns the source address and size of the bitmap loaded by the previous CMD_LOADIMAGE.
 * 
 * @param eve			Pointer to the EVE device
 * @param ptr 			Source address of bitmap.
 * Note :
 * At API Level 2 this parameter returns the source address of the decoded image data in RAM_G.
 * At API level 1, this parameter has different meaning based on the input image format of CMD_LOADIMAGE: 
 *  - For JPEG, it is the source address of the decoded image data in RAM_G. 
 *  - For PNG, it is the first unused address in RAM_G after decoding process.
 * @param size 			Pointer to the size of the image.
 * 	width: The width of the image which was decoded by the last CMD_LOADIMAGE before this command. 
 * 	height: The height of the image which was decoded by the last CMD_LOADIMAGE before this command. 
 */
void eve_copro_getprops(eve_t* eve, uint32_t* ptr, eve_ui_size_t* size);
/**
 * @brief This command sets the API level used by the coprocessor. 
 * 
 * @param eve			Pointer to the EVE device
 * @param level			API level to use. Level 1 is BT815 compatible, and is the default. Level 2 is BT817/8.
 */
void eve_copro_set_apilevel(eve_t* eve, uint32_t level);

/**
 * @brief CMD_SETFONT is used to register one custom defined bitmap font into the co-processor engine.
 * After registration, the co-processor engine is able to use the bitmap font with corresponding commands.
 *
 * @param eve			Pointer to the EVE device
 * @param font			The bitmap handle from 0 to 31
 * @param address		The metrics block address in RAM. 4 bytes aligned is required.
 */
void eve_copro_set_font(eve_t* eve, uint8_t font, uint32_t address);

/**
 * To use a custom font with the co-processor objects, create the font definition data in RAM_G and issue CMD_SETFONT2,
 * as described in ROM and RAM Fonts. For details about how to set up a custom font, refer to ROM and RAM Fonts.
 *
 * @param eve			Pointer to the EVE device
 * @param font			The bitmap handle from 0 to 31
 * @param address		32 bit aligned memory address in RAM_G of font metrics block
 * @param firstchar		The ASCII value of first character in the font.
 */
void eve_copro_set_font2(eve_t* eve, uint8_t font, uint32_t address, uint8_t firstchar);

/**
 * @brief	Sets the color for the following components:
 * 			- Color of the text in text component
 * 			- Color of the text in button component
 * 			- Color of the needle and mark in gauge component
 * 			- Color of the text in keys component
 * 			- Color of the bar in progress component
 * 			- Color of the left bar of a slider
 * 			- Color of the marker of a dial
 * 			- Color of the text of a toggle
 * 			- Color of the text of a number
 * 			- Color of the spinner
 *
 * @param c		Color Object with the red, green and blue value.
 */
void eve_copro_set_color(eve_t* eve, color_t c);

///**
// * @brief	Sets the alpha for the color.
// *
// * @param alpha	Alpha of the color.
// */
//void eve_copro_set_color_alpha(eve_t* eve, uint8_t alpha);

/**
 * @brief	Sets the color for the following components:
 * 			- Background color of Buttons
 * 			- Background color of Keys
 * 			- Inner Bar of the scrollbar
 * 			- Color of the knob of a slider
 * 			- Color of the knob of a dial
 * 			- Color of the knob of a toggle
 * 			- Color of the animating dot of the touch screen calibration
 *
 * @param c		Color Object with the red, green and blue value.
 */
void eve_copro_set_foreground(eve_t* eve, color_t c);

/**
 * @brief	Sets the color of the following components:
 * 			- Background color of gauge
 * 			- Background color of progress
 * 			- Color of the outer Bar of the scrollbar
 * 			- Color of the right bar of the knob of a slider
 * 			- Color of the bar of a toggle
 * 			- Color of the outer dot of the touch screen calibration.
 *
 * @param c		Color Object with the red, green and blue value.
 */
void eve_copro_set_background(eve_t* eve, color_t c);

/**
 * @brief	Sets a gradient color for components with 3D effect. This is the 3D highlight color of:
 * 			- Button
 * 			- Keys
 *
 * @param c		Color Object with the red, green and blue value.
 */
void eve_copro_set_gradientcolor(eve_t* eve, color_t c);

/**
 * @brief	Draws a smooth color gradient from x1/y1 to x2/y2.
 *
 * 			All the color's step values are calculated based on smooth curve interpolated from
 * 			the RGB0 to RGB1 parameter. The smooth curve equation is independently
 * 			calculated for all three colors and the equation used is R0 + t * (R1 - R0), where t is
 * 			interpolated between 0 and 1. Gradient must be used with Scissor function to get
 * 			the intended gradient display.
 *
 * @param x1		Gradient start x-position
 * @param y1		Gradient start y-position
 * @param c1		Color Object with the red, green and blue value at position x1/y1.
 * @param x2		Gradient end x-position
 * @param y2		Gradient end y-position
 * @param c2		Color Object with the red, green and blue value at position x2/y2.
 *
 * @attention	This function is not completely implemented yet!
 */
void eve_copro_gradient(eve_t* eve,
								int32_t x1, int32_t y1, color_t c1,
								int32_t x2, int32_t y2, color_t c2);

/**
 * @brief	Draws a text at the position of x/y. This is used in gui/text.c -> Do not use it from somewhere else!
 *
 * 			The x/y position depends on the alignment of the text:
 * 			Horizontal (x):
 * 			- Left aligned (options 0): x is the point at the left side of the text.
 * 			- Right aligned (options EVE_OPT_TEXT_RIGHTX): x is the point at the right side of the text.
 * 			- Center aligned (options EVE_OPT_TEXT_CENTERX): x is the point at the center of the text.
 * 			Vertical (x):
 * 			- Top aligned (options 0): y is the point at the upper side of the text.
 * 			- Center aligned (options EVE_OPT_TEXT_CENTERY): y is the point at the center of the text.
 *
 * 			If the option is EVE_OPT_TEXT_CENTER the x/y points to the vertical and horizontal center of the text.
 *
 * @param x			x-Position (See description for details!)
 * @param y			y-Position (See description for details!)
 * @param font		Font to use for the text (ROM Fonts 0 - 31).
 * @param options	Options used to draw the text (See description for details!)
 * @param str		Pointer to the string that should be drawn.
 */
void eve_copro_text(eve_t* eve, int32_t x, int32_t y, uint16_t font, EVE_OPT_TEXT options, const char *str);
// TODO: Description
void eve_copro_number(eve_t* eve, int32_t x, int32_t y, uint16_t font, EVE_OPT_TEXT options, uint32_t num);

//void eve_copro_key(eve_t* eve, key_t* obj, uint8_t x, uint8_t y);

/**
 * @brief	Draws a button at the x/y position in the button object. This function is only for usage in gui/button.c.
 * 			This function does nothing if the pointer to the button object is NULL.
 *
 * 			Adds a tag for the Button to detect if it is pressed. The Button is drawn at position x/y of its component
 * 			object with the width and height, that is specified in the button object.
 *
 * @param obj		Pointer to the button that needs to be drawn.
 * @param x			x-offset of the parent component.
 * @param y			y-offset of the parent component.
 */
void eve_copro_button(eve_t* eve, int32_t x, int32_t y, uint16_t w, uint16_t h, uint16_t option, uint16_t font, char* text);

/**
 * @brief	Adds a tag number to the next drawn co-processor commands until eve_copro_clear_tag() is called.
 *
 * @param obj		Pointer to the component that contains the pressed callback which is called when the component is tabbed.
 */
void eve_copro_add_tag(eve_t* eve, component_t* obj);

/**
 * @brief	Adds the last tag number to the next drawn co-processor commands until eve_copro_clear_tag() is called.
 *
 * @param eve		Pointer to the responsible eve object.
 */
void eve_copro_use_last_tag(eve_t* eve);

/**
 * @brief	Is needed when eve_copro_add_tag was used. Indicates the end of the component that needs to be tagged.
 */
void eve_copro_clear_tag(eve_t* eve);

/**
 * @brief	Draws a toggle at the x/y position in the switch button object. This function is only for usage in gui/switch_button.c.
 * 			This function does nothing if the pointer to the button object is NULL.
 *
 * 			Adds a tag for the Button to detect if it is pressed. The Button is drawn at position x/y of its component
 * 			object with the width and height, that is specified in the button object.
 *
 * @param obj		Pointer to the toggle that needs to be drawn.
 * @param x			x-offset of the parent component.
 * @param y			y-offset of the parent component.
 */
void eve_copro_switch_button(eve_t* eve, switch_button_t* obj, int32_t x, int32_t y);

/**
 * @brief	Function is called when the currently pressed button should be released.
 * 			This function only triggers that the color of the button changes from the pressed colors to the not pressed
 * 			colors. No action is triggered.
 */
void eve_copro_touch_released(eve_t* eve);

/**
 * @brief	Indicates that a new screen is drawn that is not just a repaint of the old screen.
 * 			Function is used to check if the finger remains on the screen after a screen change to ignore the touch on a
 * 			button if the finger is on a button in the new screen, while changing the screen.
 */
void eve_copro_set_screen_changed(eve_t* eve);
#if EVE_COPRO_ENABLE_SNAPSHOT && MODULE_ENABLE_MMC
/**
 * Stores an image of the current screen on the mmc.
 * @param filename		Filename of the screenshot
 */
void eve_copro_snapshot(eve_t* eve, const char* filename);
#endif
/**
 * @brief	Checks if the 4096 Byte Command Buffer has space to store >length< bytes, where length is the parameter.
 * 			If there is not enough space empty the function waits until there is enough space.
 *
 * @param eve 		Pointer to the eve context.
 * @param length	Number of bytes to write into the command buffer.
 * @retval FUNCTION_RETURN_OK				All co-processor commands were processed successfully.
 * @retval FUNCTION_RETURN_TIMEOUT			While waiting for the execution, the coprocessor did not finish.
 * @retval FUNCTION_RETURN_DEVICE_RESET		The co-processor entered a faulty condition and was reset. Last commands might not have been processed.
 * @retval FUNCTION_RETURN_PARAM_ERROR		Eve pointer is null.
 */
FUNCTION_RETURN eve_copro_check_command_buffer(eve_t* eve, uint32_t length);
#endif

#endif /* EVE_COPRO_H_ */
