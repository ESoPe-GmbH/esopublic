/**
 * 	@file eve.h
 *  @copyright Urheberrecht 2014-2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 *  @author 	Tim Koczwara
 *
 *  @brief	The eve module is used for controlling the FTDI EVE graphic chip.
 *
 *  		The EVE chip offers 2 ways to draw something on the display.
 *  			1. Draw directly by writing commands into the display list of the eve chip.
 *  				1.1 eve_dl()_start is needed to start a new display list
 *  				1.2 eve_dl() can be used to add display list commands to the display list.
 *  					See EVE manual and eve_register.h for details. The defines in eve_register.h are named after
 *  					the manual, but every define has the prefix EVE_.
 *  				1.3 eve_dl_swap() is needed to end the display list and show the graphic on the screen.
 *  			2. Use the co-processor of the eve to store commands and then print it to the screen.
 *  			   See eve_copro.h for details. The process is like the display list process.
 *  				2.1 Check if eve_copro_is_ready() returns true -> Otherwise it might not work.
 *  				2.2 Use eve_copro_dlstart() to start a new display list via the co-processor.
 *  				2.3 (Optional) Use eve_copro_coldstart() to reactivate the standards of the co-processor.
 *  					For example if the color of buttons in the screen before were changed, this sets it back to default
 *  					color.
 *  				2.4 Now the co-processor commands can be called. See eve_copro.h for detailed description of the
 *  					following functions.
 *  					2.4.1 eve_copro_write_command() can add display list commands (same as eve_dl to co-processor).
 *  					2.4.2 eve_copro_set_color() to change the color of the following component.
 *  					2.4.3 eve_copro_set_foreground() to change the color of the following component.
 *  					2.4.4 eve_copro_set_background() to change the color of the following component.
 *  					2.4.5 eve_copro_set_gradientcolor() to change the color of the following component.
 *  					2.4.6 eve_copro_gradient() to draw a rectangle which was selected via scissor.
 *  					2.4.7 eve_copro_text() to add a text to the display.
 *  					2.4.8 eve_copro_button() to to add a button to the display.
 *  				2.5 End the co-processor display list via eve_copro_swap().
 *
 *			The gui module uses already the functionality mentioned above. So when using the gui, eve_copro.h might
 *			not be accessed manually. Initialization and handle of eve.h are needed nevertheless.
 *
 *	@version 	1.03 (13.11.2018)
 *		- Many undocumented changes
 *		- Solved bug with pwm cycle -> Displays stays dark after power on solved!
 *  @version	1.02 (18.09.2015)
 *  	- Interrupt register is now additionally checked every second in case an interrupt is missing
 *  	- Calibration for 5.7" changed
 *  @version	1.01 (06.01.2015)
 *  	- Touch Values for 5.7" added
 *  	- Added rotation parameter to eve_init
 *  	- Added rotated touch values for 4.3" and 5.7"
 *  @version	1.00 (07.05.2014)
 *  	- Intial release
 *
 *	@par 	References
 *
 ******************************************************************************/

#ifndef EVE_H_
#define EVE_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "mcu/sys.h"
#include "../eve_ui/color.h"
#include "eve_register.h"
#include "eve_errorcodes.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if !MODULE_ENABLE_MMC
#undef EVE_MMC_READ_BUFFER_SIZE
#define EVE_MMC_READ_BUFFER_SIZE 				0
#endif

#if MODULE_ENABLE_DISPLAY && DISPLAY_ENABLE_SLD
	#define EVE_ENABLE_SLD			1
#else 
	#define EVE_ENABLE_SLD			0
#endif

#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
#include "module/lcd_touch/lcd_touch.h"
#include "module/lcd_touch/driver/st1633i/st1633i.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Hardware interface to EVE
 */
typedef struct
{
	/// SPI Interface
	mcu_spi_t spi;
	/// Interrupt
	mcu_io_int_t io_int;
	/// Optional power switch, if not used set the pin of the structure to PIN_NONE
	mcu_io_handler_t io_h_pwr;
	/// Power Down pin
	MCU_IO_PIN io_pd;
	/// @brief I/O pin that might be needed to enable the sound externally.
	mcu_io_handler_t io_sound_enable;
#if EVE_USE_FT81X
	/// @brief If true, eve uses spi in quad mode for faster communication.
	bool enable_quad_spi;
#endif
	// /// @brief Pointer to an external i2c for touch interface. In case of SMM Displays, this is the i2c of the eeprom to read edid for automatic configuration.
	// i2c_t* i2c_touch;
#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
	/// Hardware config for external touch that is connected to host cpu. To enable it, set an i2c handler to the i2c pointer. Is needed for SMM Displays.
	st1633i_hw_config_t external_touch;
#endif
}eve_hw_interface_t;

/**
 * @enum EVE_DISPLAY_TYPE
 *
 * Possible displays that are supported by the eve module.
 */
typedef enum
{
	/// 4.3 inch display with a resolution of 480 x 272
	EVE_DISPLAY_TYPE_4_3_480_272 = 0,	
	/// 5.7 inch display with a resolution of 320 x 240
	EVE_DISPLAY_TYPE_5_7_320_240,		
	/// 3.5 inch display with a resolution of 320 x 240
	EVE_DISPLAY_TYPE_ETML035023UDRA,
	/// 4.3 inch display with a resolution of 480 x 272
	EVE_DISPLAY_TYPE_ETML043023UDRA,
#if EVE_USE_FT81X
	/// 7 inch display with a resolution of 800 x 480
	EVE_DISPLAY_TYPE_7_800_480,			
	/// 5 inch display with a resolution of 800 x 480
	EVE_DISPLAY_TYPE_ETML050023UDRA,	
	/// 7 inch display with a resolution of 800 x 480
	EVE_DISPLAY_TYPE_ETML070023UDBA,
#if MODULE_ENABLE_DISPLAY && DISPLAY_ENABLE_SLD
	/// Schukat SMM Displays, with I2C EEPROM to read out the configuration and set it internally.
	EVE_DISPLAY_TYPE_SMM,
#endif // MODULE_ENABLE_DISPLAY && DISPLAY_ENABLE_SLD
#endif
	/// Limiter for display type enumeration.
	EVE_DISPLAY_TYPE_MAX
}EVE_DISPLAY_TYPE;

/**
 * @brief Possible flash status values that can be read in @c EVE_REG_FLASH_STATUS. 
 */
typedef enum eve_flash_status_e
{
	/// Flash state during initialization. Changes to detached or basic, whether a flash is detected.
	EVE_FLASH_STATUS_INIT = 0,
	/// Flash is detached and cannot be used. To use it, you need to trigger @c eve_copro_flash_attach().
	EVE_FLASH_STATUS_DETACHED = 1,
	/// Flash is attached and can be used. To change into high speed mode, the blob for the first sector needs to be written into the flash. Afterwards you need to trigger @c eve_copro_flash_fast().
	EVE_FLASH_STATUS_BASIC = 2,
	/// Flash is attached and in high speed mode. This is the best operating mode to use the flash for displaying data from it. Direct rendering of ASTC based bitmaps from flash is only possible in this mode.
	EVE_FLASH_STATUS_FULL = 3,
}EVE_FLASH_STATUS_T;

/**
 * @brief Callback for an error of the eve chip
 * @param dev		Pointer to the eve chip that had an error.
 * @param err		Code of the error.
 * @param msg		String message of the error.
 */
typedef void (*eve_error_cb_t)(void* user_ctx, EVE_ERROR error, const char* msg);

/**
 * @struct eve_reset_action_t
 * Is used to register actions that need to be done when the display is re-initialized.
 * The action contains of a callback function with two pointer parameter.
 * These objects are used in a list internally, so do not change the next variable outside of this module.
 */
typedef struct
{
	/// Callback function for this action, which takes two pointer as parameters.
	void (*f)(void*,void*);
	/// First parameter of the callback function.
	void* p1;
	/// Second parameter of the callback function.
	void* p2;
	/// Pointer to the next eve_reset_action_t inside the internal list.
	void* next;
}eve_reset_action_t;

typedef struct
{
	bool is_ram;
	bool handler_is_loaded;
	uint32_t address;
	const uint8_t* data;
	uint8_t first_character;
	uint32_t height;
}eve_font_info_t;

typedef struct
{
	const char* filename;

	uint32_t address;

	const uint8_t* data;

	uint32_t data_length;

}eve_memory_file_t;

/**
 * @struct eve_memory_t
 * Variables needed to manage the memory of the eve.
 */
typedef struct
{
	/// Ram Pointer for current components written into the ram of eve.
	uint32_t ram_pointer;

	/// metric address of the rom font
	uint32_t metric_address;

	/// Info over every 32 fonts
	eve_font_info_t font[32];

}eve_memory_t;

typedef struct
{
	uint16_t 	eve_dli;				///< Display list index, that is used incremented with each written display command.

	bool 		eve_display_is_rotated;

	bool 		eve_is_initialized;			///< Is set to true when the eve chip responded correctly during initialization.

	uint16_t	eve_display_width;		///< Width of the display in pixel. Is set in eve_init.

	uint16_t	eve_display_height;		///< Height of the display in pixel. Is set in eve_init.

	bool 		eve_spi_int_triggered;	///< Flag indicates if an interrupt occured. Is set inside
															///< the interrupt function and cleared inside the handle function.

	uint32_t	int_timestamp;				///< Timestamp used to check interrupt register in case an interrupt was missed

	system_task_t eve_spi_task;				///< Task for the eve spi handle

	/// Hardware interface to eve
	eve_hw_interface_t hw;

	/// Indicates if the power for EVE and display is on or off. When initializing, it is set to true.
	/// Is changed via eve_switch_power function.
	bool is_powered;

	/// Timestamp is set when power is switched off with eve_switch_power. When power is switched back on, the timestamp is
	/// checked to have a minimum power off time of 20ms.
	uint32_t power_off_timestamp;

	/// Flag is set to true when the eve was reinitialized and must be repainted asap.
	bool force_repaint;

	/// Flag indicating whether the display has touch
	bool has_touch;

	/// @brief Flag indicating whether quad/dual spi can be used for communication. Is set to none by default
	MCU_SPI_TRANS_FLAGS_T spi_width_flags;

	/// EVE Command List Index -> Used for the Address of Co-processor commands. Must always be a multiple of 4.
	uint32_t eve_copro_cli;

	/// EVE Command List Index -> Used for the Address of Co-processor commands. Must always be a multiple of 4.
	/// This is the last one written into the register via spi -> used to minimize access
	uint32_t eve_copro_cli_last_written;

	/// Counts the number of active buttons for the eve_current_buttons array, which is used for callbacks, when it is pressed.
	uint32_t eve_copro_current_button_cnt;

	/// Contains Pointer to the currently visible button objects and is used for callbacks when a button is pressed or released.
	/// The pointer is used for objects in general because there exist more then one type of buttons.
	void* eve_copro_current_buttons[EVE_COPRO_MAX_TAGS];

	/// Pointer to the currently pressed button. Is used for pressed / released color change.
	/// The pointer is used for objects in general because there exist more then one type of buttons.
	void* eve_copro_selected_button;

	/// Indicates if the co-processor is ready to receive commands. Is set to false when the screen is repainted and set to true
	/// when the CMDFLAG_EMPTY interrupt occurs.
	bool eve_copro_is_ready_flag;

	/// Timestamp set each time the eve_copro_is_ready_flag is set to false.
	uint32_t component_timestamp;

	/// Is used to recognize a touch after the screen is changed. If the finger remains on the screen after a change and the
	/// finger is on a button of the new screen, the button is not pressed immediately.
	//static bool eve_copro_ignore_first_touch = false;

	bool eve_copro_release_touch;

	/// Is used to recognize changes in touch / release jitter. Sometimes the touch pressed / released changes while the finger is
	/// not pressed completely on the screen.
	uint32_t eve_copro_touch_timestamp;

	/// Pointer to a callback function when the display is touched.
	void (*eve_copro_touch_callback)(void*, bool, int32_t, int32_t);

	void* eve_copro_touch_callback_object;

	/// Current color -> Used to check if eve_copro_set_color must be executed -> only executed when color changes
	/// because setting the color is an enormous amount of bytes lost in the display list!
	color_t col;

	/// Type of the display connected to the eve chip.
	EVE_DISPLAY_TYPE type;

	/// Data for the managing the memory of the eve.
	eve_memory_t memory;

#if EVE_COPRO_ENABLE_DUMP
	/// Is set to true when the memory is changed. Is used in the dumps to check whether ram content must be sent or not
	bool memory_changed;
#endif

	///
	eve_memory_file_t memory_files[EVE_MEMORY_FILES_MAX];

	/// Pointer to a structure that can be set for the error callback.
	void* error_obj;

	/// Callback function that needs to be called in case of an error. First parameter is a pointer to a structure that can be set as first parameter.
	/// Second parameter is the error code.
	eve_error_cb_t error_callback;

	/// Flag is set during re-initialization to suppress errors
	bool in_reintialization;

	#if EVE_COPRO_DEBUG_COMMAND_COUNT
	/// Debug variable to count the number of commands that are currently written inside the co-processor buffer in one cycle.
	uint32_t eve_copro_cmd_cnt;
	#endif

	/// Stores the latest touch sampling mode
	/// - 0 - Touch off
	/// - 3 - Continuous mode.
	uint8_t touch_mode;

	/// Points to the first reset action object. Is set to NULL during eve_init, so do not set it before!
	eve_reset_action_t* first_reset_action_obj;

	/// Points to the next eve object to form a list
	void* next;

#if !EVE_USE_FT81X
	/// Variable used to simulate the vertex format of FT810
	uint8_t vertex_format;
#endif

	/// Pointer to the currently pressed key. Is used for pressed / released color change.
	/// The pointer is used for objects in general because there exist more then one type of keys.
	char selected_key;

	/// Pointer to a callback function when the display is touched.
	void(*key_callback)(void*, char);

	void* key_callback_object;
	/// @brief Buffer for error messages read from eve.
	char msg[128];
	/// @brief API level that is relevant for some tasks.
	/// level 1 is BT815 compatible
	/// level 2 is BT817/8 compatible
	uint8_t api_level;
#if MODULE_ENABLE_LCD_TOUCH_DRIVER_ST1633I && MODULE_ENABLE_LCD_TOUCH
	/// @brief Pointer to the touch driver that is used for the touch interface.
	lcd_touch_device_handle_t touch_device;
	/// @brief Handle for the touch
	lcd_touch_handle_t touch;
	/// @brief Touch to handle an external connected touch driver to write values into eve chip
	system_task_t touch_task;
#endif
}eve_t;

#pragma pack(push, 1)
/// @brief Describes a single asset in the flash
typedef struct eve_asset_info_s
{
	/// @brief ID of the asset
	uint16_t  assetID;
	/// @brief Offset from 0
	uint32_t  startAddress;       
	/// @brief count in bytes
	uint32_t  size;
	/// @brief 0: raw (no compression), 1: bin (deflation)
	uint8_t   compressionMethod;
	/// @brief refer to Table 1 in User Guide of EVE Asset Builder
	uint8_t   type;
	/// @brief refer to Table 2, 3 in User Guide of EVE Asset Builder
	uint16_t  subType;
	/// @brief Width of the image / video
	uint16_t  width;
	/// @brief Height of the image / video
	uint16_t  height;
}eve_asset_info_t;
#pragma pack(pop)

/// @brief Contains all asset informations in the flash
typedef struct eve_asset_infos_s
{
	/// @brief Number of elements in the information array
	uint32_t number_of_infos;
	/// @brief Array containing the information
	eve_asset_info_t* infos;
}eve_asset_infos_t;

/// @brief Handle for the asset informations
typedef eve_asset_infos_t* eve_asset_infos_handle_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief	Initializes this module. Sets the parameters into the spi_handler and writes the initialization routine
 * 			to the eve chip. If the chip does not respond correctly to a command, this function returns false,
 * 			indicating that the display will not work correctly. If the display type is invalid (not implemented),
 * 			the function returns false as well.
 *
 * @param obj				Pointer to the eve that is used.
 * @param hw				Hardware interface to eve
 * @param type				Display type that is used. This determines the display resolution as well.
 * @param rotate			Rotates the display by 180° if set to true.
 * @param has_touch			True: Touch display functions available.
 * 							False: No touch available
 *
 * @return					true: Initialization was successfull and eve chip responded correctly.
 * 							false: Invalid display type or eve chip did not respond correctly.
 **/
bool eve_init(eve_t* obj, eve_hw_interface_t* hw, EVE_DISPLAY_TYPE type, bool rotate, bool has_touch, void* error_obj, eve_error_cb_t f_error);

/**
 * @brief	Initializes the touch screen based on the display type.
 * @param obj				Pointer to the eve that is used.
 */
void eve_init_touch(eve_t* obj);

/**
 * @brief	Switches the power for the eve and display on or off. Does nothing if it was already in the target state.
 * @param obj				Pointer to the eve that is used.
 * @param b					true: EVE and display are turned on.\n
 * 							false: EVE and display are turned off.
 */
void eve_switch_power(eve_t* obj, bool b);

/**
 * @brief 	Registers a reset action for the eve. The callback function of the action is called with the parameters,
 * 			when the display is re-initialized.
 * 			Set the parameter f, p1 and p2 of the eve_reset_action_t structure outside of this function.
 * 			- The parameter p1 is used as the first parameter of f when f is called.
 * 			- The parameter p2 is used as the second parameter of f when f is called.
 * 			The next parameter is managed internally, do not modify it outside!
 * @param obj				Pointer to the eve that is used.
 * @param action			Pointer to the eve_reset_action_t action. Set parameter f, p1 and p2 manually.
 */
void eve_register_reset_action(eve_t* obj, eve_reset_action_t* action);

void eve_do_reset_actions(eve_t* obj);
/**
 * @brief	Sets the touch sampling mode, which enables/disables the touch.
 * 			b = true -> Touch mode 3 = Continuous mode. 1000 times per second. (default)
 * 			b = false -> Touch mode 0 = Off mode. No sampling happens.
 * @param obj				Pointer to the eve that is used.
 * @param b		true -> Touch mode 3 = Continuous mode. 1000 times per second. (default)
 * 				false -> Touch mode 0 = Off mode. No sampling happens.
 */
void eve_set_touch_enabled(eve_t* obj, bool b);

/**
 * @brief	Function starts a display list, so that eve_dl and eve_dl_swap can be used.
 */
void eve_dl_start(eve_t* obj);

/**
 * @brief	Writes a display command to the display list. See display list commands in eve_register.h for details.
 * 			See the eve programming guide as well for details.
 *
 * 			Example:
 * 			eve_dl(EVE_CLEAR(1, 1, 1));
 * 			eve_dl(EVE_DISPLAY());
 *
 * 			eve_dl(EVE_DISPLAY()); must be called before eve_dl_swap() is called.
 *
 * @pre	eve_dl_start must be called before to start a display list
 * @post eve_dl_swap must be called after all commands are written to the display list with eve_dl.
 */
void eve_dl(eve_t* obj, uint32_t cmd);

/**
 * @brief	Swaps the currently written display list, which shows the graphic on the screen.
 *
 * @pre	eve_dl_start must be called before to start a display list
 * @pre	eve_dl must be used to draw graphics on the screen.
 * @pre	eve_dl(EVE_DISPLAY()); must be the last eve_dl command before calling this function.
 */
void eve_dl_swap(eve_t* obj);
#if EVE_GENERATION > 2
/**
 * @brief 	Initialize the external flash that is attched to the eve chip.
 * 
 * @param eve 	Pointer to eve data context.
 * @retval FUNCTION_RETURN_OK					External flash was attached and set into flash fast mode. Flash is now usable.
 * @retval FUNCTION_RETURN_UNSUPPORTED			Connected flash is not supported.
 * @retval FUNCTION_RETURN_EXECUTION_ERROR		Tried to attach flash, but eve still remains in a non-usable flash status.
 */
FUNCTION_RETURN_T eve_init_flash(eve_t* eve);
/**
 * @brief 	Returns the status of the external flash that is attached to the eve chip.
 * 
 * @param eve 	Pointer to eve data context.
 * @return EVE_FLASH_STATUS_T 	See @see EVE_FLASH_STATUS_T for possible values.
 */
EVE_FLASH_STATUS_T eve_get_flash_status(eve_t* eve);
/**
 * @brief The value indicates the capacity of attached flash, in Mbytes. 
 * 
 * @param eve 	Pointer to eve data context.
 * @return uint32_t The value indicates the capacity of attached flash, in Mbytes. 
 */
uint32_t eve_get_flash_size(eve_t* eve);
/**
 * @brief Reads the asset informations from flash if created using EVE Asset Builder with enabled EDF Block
 * 
 * @param eve 	Pointer to eve data context.
 * @param infos 			Pointer to a handle for the asset infos. The asset info structure is allocated internally for it.
 * @return FUNCTION_RETURN_T 	FUNCTION_RETURN_OK on success, other on reading error.
 */
FUNCTION_RETURN_T eve_flash_read_asset_infos(eve_t* eve, eve_asset_infos_handle_t* infos);
/**
 * @brief Free the handle for asset informations, that was created using @c eve_flash_read_asset_infos.
 * 
 * @param infos 	Pointer to a handle for the asset infos, that was created using @c eve_flash_read_asset_infos.
 */
void eve_free_asset_infos(eve_asset_infos_handle_t* infos);
#endif

#endif

#endif /* EVE_H_ */
