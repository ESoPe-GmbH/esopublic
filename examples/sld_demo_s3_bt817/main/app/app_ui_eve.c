#include <stdio.h>
#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32S3 && CONFIG_SLD_C_W_S3_BT817 && !KERNEL_USES_SLINT && !KERNEL_USES_LVGL

#include "board/board.h"
#include "module/version/version.h"
#include "module/gui/eve_ui/screen.h"
#include "module/gui/eve_ui/button.h"
#include "module/gui/eve_ui/image.h"
#include "module/gui/eve_ui/text.h"
#include "module/gui/eve_ui/font.h"
#include "resources/file_resources.h"
#include "module/console/dbg/debug_console.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

typedef enum font_e
{
    FONT_LARGE = 0,
    FONT_MEDIUM,
    FONT_SMALL,
}FONT_T;

struct screen_main_s
{
    /// Counter that is incremented with a button.
    uint32_t counter;
    /// Runtime counter, that is incremented every second
    uint32_t runtime_seconds;

    char str_counter[16];

    char str_runtime[16];

    char str_display[16];

    char str_resolution[30];

    char str_version[16];

    text_t text_counter_title;

    text_t text_counter_value;

    text_t text_runtime_title;

    text_t text_runtime_value;

    text_t text_display;

    text_t text_resolution;

    text_t text_title;

    text_t text_version;

    button_t button_increment;

    button_t button_image;

    button_t button_info;

    image_t image_logo;

    image_t image_button_image;

    image_t image_button_info;
};

struct screen_image_s
{
    /// Image that is shown on the screen
    image_t image;
    /// Back button to go back to the main screen
    button_t button_back;
};

struct screen_info_s
{
    /// QR code that is shown on the screen
    image_t image_qr_code;
    /// Back button to go back to the main screen
    button_t button_back;

    text_t text_title;

    text_t text_subtitle;

    text_t text_powered_by;

    image_t image_powered_by;
};

typedef struct screen_test_s
{
	image_t rgb_test_image;
	button_t test_buttons[5];
}screen_test_t;

typedef struct screen_data_s
{
    /// @brief Data for main screen
    struct screen_main_s main; 
    /// @brief Data for image screen
    struct screen_image_s image;
    /// @brief Data for info screen
    struct screen_info_s info;
    /// @brief Data for test screen
    struct screen_test_s test;
    
}screen_data_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
typedef enum
{
    /// @brief The main screen, shows a counter, runtime and buttons to enter the other screens.
    LCD_ACTIVE_SCREEN_MAIN,

    /// @brief The image screen shows an image with a back button
    LCD_ACTIVE_SCREEN_IMAGE,

    /// @brief The info screen shows a qr code and an info text
    LCD_ACTIVE_SCREEN_INFO,

    /// @brief The test screen shows a test image and 5 buttons that needs to be pressed.
    LCD_ACTIVE_SCREEN_TEST,

    /// @brief Limiter of the enum
    LCD_ACTIVE_SCREEN_MAX
}LCD_ACTIVE_SCREEN_T;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Create the initial image that is drawn
 */
static void _ui_init(void);
/**
 * @brief Create a screen based on the enum value.
 * 
 * @param screen    Screen that should be created.
 */
static void _show_screen(LCD_ACTIVE_SCREEN_T screen);
/**
 * @brief Create the screen according to @c LCD_ACTIVE_SCREEN_MAIN 
 */
static void _create_screen_main(void);
/**
 * @brief Create the screen according to @c LCD_ACTIVE_SCREEN_IMAGE 
 */
static void _create_screen_image(void);
/**
 * @brief Create the screen according to @c LCD_ACTIVE_SCREEN_INFO 
 */
static void _create_screen_info(void);
/**
 * @brief Create the screen according to @c LCD_ACTIVE_SCREEN_TEST 
 */
static void _create_screen_test(void);
/**
 * @brief Get the font that is used based on the screen resolution and the font type.
 * 
 * @param font      Font type that should be used.
 * @return int      Font that is used for the given type.
 */
static int _get_font(FONT_T font);
/**
 * @brief Get the spacing that is used for the given font type based on the screen resolution.
 * 
 * @param font      Font type that should be used.
 * @return int      Spacing that is used for the given font type.
 */
static int _get_spacing(FONT_T font);
/**
 * @brief Default button handle to show a screen. The user data of the event is a casted @c LCD_ACTIVE_SCREEN_T value that should be shown.
 * 
 * @param e     Pointer to the event data
 */
static void _button_handler(button_t* e);
/**
 * @brief Button used to increment the counter that is shown.
 * 
 * @param e     Pointer to the event data
 */
static void _button_increment_handler(button_t* e);
/**
 * @brief Callback set on a 1s timer thath increments the runtime
 * 
 * @param tmr           Timer that was triggered
 */
static int _timer_runtime_handle(struct pt* pt);
/**
 * Callback function for the test buttons, which makes the button invisible
 * 
 * @param b 			Pointer to the button that was pressed
 */

static void _test_button_handler(button_t* b);
/**
 * @brief Console for changing the screens.
 * 
 * @param data          Console data
 * @param args          List of arguments
 * @param args_len      Number of elements in args
 * @return FUNCTION_RETURN FUNCTION_RETURN_OK on success, other on error
 */
static FUNCTION_RETURN _cmd_console(console_data_t* data, char** args, uint8_t args_len);
/**
 * @brief Callback function that is called upon "test start" to show the test screen.
 * 
 * @param obj           Custom object pointer from debug_console_test_t.
 * @param data          Pointer to the console.
 * @param args          List of arguments.
 * @param args_len      Number of arguments.
 */
static void _dbc_test_handle(void* obj, console_data_t* data, char** args, uint8_t args_len);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Currently shown screen.
static LCD_ACTIVE_SCREEN_T _active_screen = LCD_ACTIVE_SCREEN_MAX;

static screen_t _screens[LCD_ACTIVE_SCREEN_MAX] = {0};

static system_task_t _task_runtime = {0};

static screen_data_t* _screen_data = NULL;

/// Bitmask that indicates which button were pressed.
/// When this is 0b11111, all buttons were pressed and the test was successfull
static int _test_pressed_buttons = 0b00000;

/// Structure for the display console command
static console_command_t _cmd = {
		.command = "display",
		.fnc_exec = _cmd_console,
		.use_array_param = true,
		.explanation = "Test Interface: start get show"
};
/// Handler for test start.
static debug_console_test_t _dbc_test;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

bool app_ui_init(void)
{
    if(board_screen_device.eve.status != EVE_STATUS_OK)
    {
        DBG_ERROR("Invalid display handle\n");
        return false;
    }

    _screen_data = mcu_heap_calloc(1, sizeof(screen_data_t));
    if(_screen_data == NULL)
    {
        DBG_ERROR("No memory for screen data\n");
        return false;
    }

	console_add_command(&_cmd);
    debug_console_register_test_callback(&_dbc_test, NULL, _dbc_test_handle);

    system_task_init_protothread(&_task_runtime, true, _timer_runtime_handle, _screen_data);

    _ui_init();

    return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _ui_init(void)
{
    _create_screen_main();
    _create_screen_info();
    _create_screen_image();
    _create_screen_test();

    _show_screen(LCD_ACTIVE_SCREEN_MAIN);

    screen_device_on(&board_screen_device);
    screen_device_set_dimming(&board_screen_device, 60);
}

static void _show_screen(LCD_ACTIVE_SCREEN_T screen)
{
    if(_active_screen != screen)
    {
        DBG_INFO("Show %d\n", screen);
        screen_paint(&_screens[screen], 0);
        _active_screen = screen;
    }
}

static void _create_screen_main(void)
{
    struct screen_main_s* data = &_screen_data->main;
    screen_t* scr = &_screens[LCD_ACTIVE_SCREEN_MAIN];

    uint32_t w = screen_device_get_width(&board_screen_device);
    uint32_t h = screen_device_get_height(&board_screen_device);

    screen_init_object(scr, color_get(COLOR_WHITE), NULL, NULL);
    scr->user = (void*)_screen_data;

    const file_resource_t* fr = file_resource_get_by_name("schukat-logo.raw");
    ASSERT_RET(fr, NO_ACTION, NO_RETURN, "Invalid logo resource\n");

    // image_init_from_flash(&data->image_logo, 0, 0, 650, 165, IMAGE_FORMAT_RGB565, "schukat-logo.bin", (const uint8_t*)fr->content, fr->filesize - 1);
    image_init_from_flash(&data->image_logo, 0, 0, 652, 168, IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_4x4_KHR, "schukat-logo.raw", (const uint8_t*)fr->content, fr->filesize - 1);
    image_set_scalef(&data->image_logo, ((float)w/2) / (float)data->image_logo.component.size.width);
    data->image_logo.filter = IMAGE_FILTER_BILINEAR;
    screen_add_component(scr, &data->image_logo.component);

    text_init(&data->text_display, w - 5, 5, data->str_display);
    text_set_font(&data->text_display, _get_font(FONT_SMALL));
    string_nprintf(data->str_display, sizeof(data->str_display), "Display: %s\"", board_screen_device.eve.sld_edid.screen_diagonal);
    text_set_horizontal_alignment(&data->text_display, TEXT_H_ALIGNMENT_RIGHT);
    screen_add_component(scr, &data->text_display.component);

    text_init(&data->text_resolution, w - 5, 5 + _get_spacing(FONT_SMALL), data->str_resolution);
    text_set_font(&data->text_resolution, _get_font(FONT_SMALL));
    string_nprintf(data->str_resolution, sizeof(data->str_resolution), "Resolution: %d x %d", w, h);
    text_set_horizontal_alignment(&data->text_resolution, TEXT_H_ALIGNMENT_RIGHT);
    screen_add_component(scr, &data->text_resolution.component);

    text_init(&data->text_title, w/2, h/2 - 50, "EVE Demo");
    text_set_font(&data->text_title, _get_font(FONT_LARGE));
    text_set_horizontal_alignment(&data->text_title, TEXT_H_ALIGNMENT_CENTER);
    screen_add_component(scr, &data->text_title.component);

    text_init(&data->text_runtime_title, 10, h/2, "Runtime:");
    text_set_font(&data->text_runtime_title, _get_font(FONT_MEDIUM));
    text_set_horizontal_alignment(&data->text_runtime_title, TEXT_H_ALIGNMENT_LEFT);
    screen_add_component(scr, &data->text_runtime_title.component);

    text_init(&data->text_runtime_value, w - 10, h/2, data->str_runtime);
    text_set_font(&data->text_runtime_value, _get_font(FONT_MEDIUM));
    string_nprintf(data->str_runtime, sizeof(data->str_runtime), "%02u:%02u min", (unsigned int)(data->runtime_seconds / 60), (unsigned int)(data->runtime_seconds % 60));
    text_set_horizontal_alignment(&data->text_runtime_value, TEXT_H_ALIGNMENT_RIGHT);
    screen_add_component(scr, &data->text_runtime_value.component);

    text_init(&data->text_counter_title, 10, h/2 + 25, "Counter:");
    text_set_font(&data->text_counter_title, _get_font(FONT_MEDIUM));
    text_set_horizontal_alignment(&data->text_counter_title, TEXT_H_ALIGNMENT_LEFT);
    screen_add_component(scr, &data->text_counter_title.component);

    text_init(&data->text_counter_value, w - 10, h/2 + 25, data->str_counter);
    text_set_font(&data->text_counter_value, _get_font(FONT_MEDIUM));
    string_nprintf(data->str_counter, sizeof(data->str_counter), "%u", (unsigned int)data->counter);
    text_set_horizontal_alignment(&data->text_counter_value, TEXT_H_ALIGNMENT_RIGHT);
    screen_add_component(scr, &data->text_counter_value.component);

    // y-coordinate of the buttons
    int by = h - 5 - _get_spacing(FONT_SMALL);

    button_init(&data->button_increment, 5, by, w/4, 35, "+");
    button_set_font(&data->button_increment, _get_font(FONT_SMALL));
    button_set_action(&data->button_increment, _button_increment_handler);
    button_set_backgroundcolor(&data->button_increment, color_get(COLOR_LIGHT_GRAY));
    button_set_textcolor(&data->button_increment, color_get(COLOR_BLACK));
    component_set_alignment(&data->button_increment.component, COMPONENT_ALIGNMENT_BOTTOM);
    screen_add_component(scr, &data->button_increment.component);

    fr = file_resource_get_by_name("button_landscape.raw");
    ASSERT_RET(fr, NO_ACTION, NO_RETURN, "Invalid button resource\n");
    image_init_from_flash(&data->image_button_image, 0, 0, 52, 32, IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_4x4_KHR, "button_landscape.raw", (const uint8_t*)fr->content, fr->filesize - 1);
    button_init(&data->button_image, w/2, by, w/4, 35, NULL);
    component_set_alignment(&data->button_image.component, COMPONENT_ALIGNMENT_CENTER_X | COMPONENT_ALIGNMENT_BOTTOM);
    button_set_figure(&data->button_image, &data->image_button_image.component, BUTTON_FIGURE_POS_CENTER);
    button_set_action(&data->button_image, _button_handler);
    button_set_backgroundcolor(&data->button_image, color_get(COLOR_LIGHT_GRAY));
    data->button_image.component.user = (void*)LCD_ACTIVE_SCREEN_IMAGE;
    screen_add_component(scr, &data->button_image.component);

    fr = file_resource_get_by_name("esope.raw");
    ASSERT_RET(fr, NO_ACTION, NO_RETURN, "Invalid esope resource\n");
    image_init_from_flash(&data->image_button_info, 0, 0, 64, 24, IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_4x4_KHR, "esope.raw", (const uint8_t*)fr->content, fr->filesize - 1);
    button_init(&data->button_info, w - 5, by, w/4, 35, NULL);
    component_set_alignment(&data->button_info.component, COMPONENT_ALIGNMENT_RIGHT | COMPONENT_ALIGNMENT_BOTTOM);
    button_set_figure(&data->button_info, &data->image_button_info.component, BUTTON_FIGURE_POS_CENTER);
    button_set_action(&data->button_info, _button_handler);
    button_set_backgroundcolor(&data->button_info, color_get(COLOR_LIGHT_GRAY));
    data->button_info.component.user = (void*)LCD_ACTIVE_SCREEN_INFO;
    screen_add_component(scr, &data->button_info.component);

    text_init(&data->text_version, w - 5, h - 5, data->str_version);
    text_set_font(&data->text_version, _get_font(FONT_SMALL));
    string_nprintf(data->str_version, sizeof(data->str_version), "Version: %s", version_get_string());
    text_set_horizontal_alignment(&data->text_version, TEXT_H_ALIGNMENT_RIGHT);
    text_set_vertical_alignment(&data->text_version, TEXT_V_ALIGNMENT_BOTTOM);
    screen_add_component(scr, &data->text_version.component);
}

static void _create_screen_image(void)
{
    screen_t* scr = &_screens[LCD_ACTIVE_SCREEN_IMAGE];
    struct screen_image_s* data = &_screen_data->image;

    uint32_t w = screen_device_get_width(&board_screen_device);
    uint32_t h = screen_device_get_height(&board_screen_device);

    screen_init_object(scr, color_get(COLOR_WHITE), NULL, NULL);
    scr->user = (void*)_screen_data;

    const file_resource_t* fr = file_resource_get_by_name("landscape.raw");
    ASSERT_RET(fr, NO_ACTION, NO_RETURN, "Invalid landscape resource\n");
    
    image_init_from_flash(&data->image, w/2, h/2, 1024, 600, IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_4x4_KHR, "landscape.raw", (const uint8_t*)fr->content, fr->filesize - 1);
    image_set_scale(&data->image, w, h);
    data->image.filter = IMAGE_FILTER_BILINEAR;
    component_set_alignment(&data->image.component, COMPONENT_ALIGNMENT_CENTER);
    screen_add_component(scr, &data->image.component);

    button_init(&data->button_back, 5, 5, 50, 30, "<");
    button_set_font(&data->button_back, _get_font(FONT_SMALL));
    button_set_action(&data->button_back, _button_handler);
    button_set_backgroundcolor(&data->button_back, color_get(COLOR_LIGHT_GRAY));
    button_set_textcolor(&data->button_back, color_get(COLOR_BLACK));
    data->button_back.component.user = (void*)LCD_ACTIVE_SCREEN_MAIN;
    screen_add_component(scr, &data->button_back.component);
}

static void _create_screen_info(void)
{
    screen_t* scr = &_screens[LCD_ACTIVE_SCREEN_INFO];
    struct screen_info_s* data = &_screen_data->info;

    uint32_t w = screen_device_get_width(&board_screen_device);
    uint32_t h = screen_device_get_height(&board_screen_device);

    screen_init_object(scr, color_get(COLOR_WHITE), NULL, NULL);
    scr->user = (void*)_screen_data;

    button_init(&data->button_back, 5, 5, 50, 30, "<");
    button_set_font(&data->button_back, _get_font(FONT_SMALL));
    button_set_action(&data->button_back, _button_handler);
    button_set_backgroundcolor(&data->button_back, color_get(COLOR_LIGHT_GRAY));
    button_set_textcolor(&data->button_back, color_get(COLOR_BLACK));
    data->button_back.component.user = (void*)LCD_ACTIVE_SCREEN_MAIN;
    screen_add_component(scr, &data->button_back.component);

    if(w == 320)
    {
        text_init(&data->text_title, w/2, h/8, "Demo Software\nand Description");
    }
    else
    {
        text_init(&data->text_title, w/2, h/8, "Demo Software and Description");
    }
    text_set_font(&data->text_title, _get_font(FONT_LARGE));
    text_set_horizontal_alignment(&data->text_title, TEXT_H_ALIGNMENT_CENTER);
    text_set_vertical_alignment(&data->text_title, TEXT_V_ALIGNMENT_BOTTOM);
    screen_add_component(scr, &data->text_title.component);

    if(w == 320)
    {
        text_init(&data->text_subtitle, w/2, h/8 + _get_spacing(FONT_LARGE), "Scan the QR-Code for the GitHub\nlink to this Demo.");
    }
    else
    {
        text_init(&data->text_subtitle, w/2, h/8 + 2, "Scan the QR-Code for the GitHub link to this Demo.");
    }
    text_set_font(&data->text_subtitle, _get_font(FONT_MEDIUM));
    text_set_horizontal_alignment(&data->text_subtitle, TEXT_H_ALIGNMENT_CENTER);
    text_set_vertical_alignment(&data->text_subtitle, TEXT_V_ALIGNMENT_TOP);
    screen_add_component(scr, &data->text_subtitle.component);

    const file_resource_t* fr = file_resource_get_by_name("qr_sld_demo.raw");
    ASSERT_RET(fr, NO_ACTION, NO_RETURN, "Invalid QR-Code resource\n");

    // image_init_from_flash(&data->image_qr_code, w/2, h/2, 160, 161, IMAGE_FORMAT_RGB565, "qr_sld_demo.raw", (const uint8_t*)fr->content, fr->filesize - 1);
    image_init_from_flash(&data->image_qr_code, w/2, h/2, 160, 164, IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_4x4_KHR, "qr_sld_demo.raw", (const uint8_t*)fr->content, fr->filesize - 1);
    if(w == 320)
    {
        image_set_scale(&data->image_qr_code, 100, 100);
    }
    component_set_alignment(&data->image_qr_code.component, COMPONENT_ALIGNMENT_CENTER_X);
    screen_add_component(scr, &data->image_qr_code.component);

    text_init(&data->text_powered_by, w - 74, h - 5, "powered by");
    text_set_font(&data->text_powered_by, _get_font(FONT_SMALL));
    text_set_horizontal_alignment(&data->text_powered_by, TEXT_H_ALIGNMENT_RIGHT);
    text_set_vertical_alignment(&data->text_powered_by, TEXT_V_ALIGNMENT_BOTTOM);
    screen_add_component(scr, &data->text_powered_by.component);

    fr = file_resource_get_by_name("esope.raw");
    ASSERT_RET(fr, NO_ACTION, NO_RETURN, "Invalid ESoPe resource\n");
    image_init_from_flash(&data->image_powered_by, w-5, h-5, 64, 24, IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_4x4_KHR, "esope.raw", (const uint8_t*)fr->content, fr->filesize - 1);
    component_set_alignment(&data->image_powered_by.component, COMPONENT_ALIGNMENT_RIGHT | COMPONENT_ALIGNMENT_BOTTOM);
    screen_add_component(scr, &data->image_powered_by.component);
}

static void _create_screen_test(void)
{
    screen_t* s = &_screens[LCD_ACTIVE_SCREEN_TEST];
    struct screen_test_s* scr = &_screen_data->test;

	// Initialize the screen
	screen_init_object(s, color_get(GUI_CONFIG_DEFAULT_SCREEN_BACKCOLOR), NULL, NULL);
	s->user = scr;

	console_add_command(&_cmd);
	uint16_t dev_w = screen_device_get_width(&board_screen_device);
	uint16_t dev_h = screen_device_get_height(&board_screen_device);
	uint8_t button_cnt = 0;

    const file_resource_t* fr = file_resource_get_by_name("rgb.raw");
    ASSERT_RET(fr, NO_ACTION, NO_RETURN, "Invalid rgb resource\n");
    image_init_from_flash(&scr->rgb_test_image, 0, 0, 320, 240, IMAGE_FORMAT_COMPRESSED_RGBA_ASTC_4x4_KHR, "rgb.raw", (const uint8_t*)fr->content, fr->filesize - 1);
	screen_add_component(s, &scr->rgb_test_image.component);

	// Test buttons
	button_init(&scr->test_buttons[button_cnt++], 10, 10, 40, 40, "1");
	button_init(&scr->test_buttons[button_cnt++], dev_w - 10 - 40, 10, 40, 40, "2");
	button_init(&scr->test_buttons[button_cnt++], 10, dev_h - 10 - 40, 40, 40, "3");
	button_init(&scr->test_buttons[button_cnt++], dev_w - 10 - 40, dev_h - 10 - 40, 40, 40, "4");
	button_init(&scr->test_buttons[button_cnt++], dev_w / 2 - 20, dev_h / 2 - 20, 40, 40, "5");

	for (button_cnt = 0; button_cnt < 5; button_cnt++)
	{
		button_set_action(&scr->test_buttons[button_cnt], _test_button_handler);
		screen_add_component(s, &scr->test_buttons[button_cnt].component);	
	}
}

static int _get_font(FONT_T font)
{
    int w = screen_device_get_width(&board_screen_device);
    switch(font)
    {
        case FONT_LARGE:
            if(w >= 800)
            {
                return 31;
            }
            else
            {
                return 29;
            }
        case FONT_MEDIUM:
            if(w >= 800)
            {
                return 29;
            }
            else
            {
                return 27;
            }
        case FONT_SMALL:
            if(w >= 800)
            {
                return 28;
            }
            else
            {
                return 26;
            }
        default:
            break;
    }
    return 0;
}

static int _get_spacing(FONT_T font)
{
    return font_get_height(&board_screen_device.eve, _get_font(font)) + 5;
}

static void _button_handler(button_t* e)
{
    LCD_ACTIVE_SCREEN_T screen = (LCD_ACTIVE_SCREEN_T)e->component.user;
    _show_screen(screen);
}

static void _button_increment_handler(button_t* e)
{
    screen_t* screen = (screen_t*)screen_get_from_component(&e->component);
    screen_data_t* data = (screen_data_t*)screen->user;

    data->main.counter++;
    string_nprintf(data->main.str_counter, sizeof(data->main.str_counter), "%u", data->main.counter);
    screen_repaint_by_component(&screen->component);
}

static int _timer_runtime_handle(struct pt* pt)
{
    screen_data_t* data = pt->obj;
    PT_BEGIN(pt);
    while(true)
    {
        PT_YIELD_MS(pt, 1000);
        data->main.runtime_seconds++;
        string_nprintf(data->main.str_runtime, sizeof(data->main.str_runtime), "%02u:%02u min", (unsigned int)(data->main.runtime_seconds / 60), (unsigned int)(data->main.runtime_seconds % 60));
        // Only repaint the screen if it is the main screen
        if(_active_screen == LCD_ACTIVE_SCREEN_MAIN)
        {
            screen_repaint(&board_screen_device);
        }
    }
    PT_END(pt);
}

static void _test_button_handler(button_t* b)
{
	uint8_t button_num = (uint8_t)strtol(b->text, NULL, 10) - 1;

	_test_pressed_buttons |= (1 << button_num);

	b->component.is_visible = false;
}

static FUNCTION_RETURN _cmd_console(console_data_t* data, char** args, uint8_t args_len)
{
	if(args_len < 1)
	{
		return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Not enough arguments");
	}	

	if(strcmp(args[0], "start") == 0)
	{
        _show_screen(LCD_ACTIVE_SCREEN_TEST);
	}
	else if(strcmp(args[0], "get") == 0)
	{
		return console_set_response_dynamic(data, FUNCTION_RETURN_OK, 7, "get %d", _test_pressed_buttons);
	}
    else if(strcmp(args[0], "show") == 0)
    {
        if(args_len < 2)
        {
            return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Not enough arguments");
        }
        LCD_ACTIVE_SCREEN_T screen = (LCD_ACTIVE_SCREEN_T)strtol(args[1], NULL, 10);
        if(screen >= LCD_ACTIVE_SCREEN_MAX)
        {
            return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Invalid screen number");
        }
        _show_screen(screen);
    }
	else
	{
		return console_set_response_static(data, FUNCTION_RETURN_PARAM_ERROR, "Invalid subcommand");
	}
	
	return FUNCTION_RETURN_OK;
}

static void _dbc_test_handle(void* obj, console_data_t* data, char** args, uint8_t args_len)
{
    _show_screen(LCD_ACTIVE_SCREEN_TEST);
}

#endif