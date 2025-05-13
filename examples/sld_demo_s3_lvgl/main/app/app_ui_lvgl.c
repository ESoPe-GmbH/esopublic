#include <stdio.h>

#if defined(KERNEL_USES_LVGL)

#include "board/board.h"
#include "module/gui/lvgl/lvgl_helper.h"
#include "module/version/version.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


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

    /// @brief Limiter of the enum
    LCD_ACTIVE_SCREEN_MAX
}LCD_ACTIVE_SCREEN_T;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Create the initial image that is drawn
 * 
 * @param disp Pointer to lvgl display object
 */
static void _ui_init(lv_display_t *disp);
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
 * @brief Default button handle to show a screen. The user data of the event is a casted @c LCD_ACTIVE_SCREEN_T value that should be shown.
 * 
 * @param e     Pointer to the event data
 */
static void _button_handler(lv_event_t* e);
/**
 * @brief Button used to increment the counter that is shown.
 * 
 * @param e     Pointer to the event data
 */
static void _button_increment_handler(lv_event_t* e);
/**
 * @brief Creates a button with a default background color
 * 
 * @param parent        pointer to an object, it will be the parent of the new button
 * @return lv_obj_t*    pointer to the created button
 */
static lv_obj_t* _create_button(lv_obj_t* parent);
/**
 * @brief Callback set on a 1s timer thath increments the runtime
 * 
 * @param tmr           Timer that was triggered
 */
void _timer_runtime(lv_timer_t *tmr);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Currently shown screen.
static LCD_ACTIVE_SCREEN_T _active_screen = LCD_ACTIVE_SCREEN_MAX;
/// Lookup table to create screens based on @c LCD_ACTIVE_SCREEN_T
static void (*f_create_screen[LCD_ACTIVE_SCREEN_MAX])(void) = 
{
    _create_screen_main,
    _create_screen_image,
    _create_screen_info
};
/// Image for the button that switches to LCD_ACTIVE_SCREEN_IMAGE
extern const lv_image_dsc_t button_landscape;
/// Image for the button that switches to LCD_ACTIVE_SCREEN_INFO
extern const lv_image_dsc_t esope;
/// Image shown in LCD_ACTIVE_SCREEN_IMAGE
extern const lv_image_dsc_t landscape;
/// Image shwon in LCD_ACTIVE_SCREEN_INFO (QR-Code to this repo)
extern const lv_image_dsc_t qr_sld_demo;
/// Image shown in LCD_ACTIVE_SCREEN_MAIN (Logo of Schukat)
extern const lv_image_dsc_t schukat_logo;
/// Counter that is incremented with a button.
static uint32_t _counter = 0;
/// Runtime counter, that is incremented every second
static uint32_t _runtime_seconds = 0;
/// Pointer to the label of the runtime
static lv_obj_t* _lbl_runtime;
/// Pointer to the label of the counter.
static lv_obj_t* _lbl_counter;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

bool app_ui_init(void)
{
    if(board_lcd == NULL || board_lcd->display == NULL)
    {
        DBG_ERROR("Invalid display handle\n");
        return false;
    }

    lvgl_helper_config_t lvgl_helper = 
    {
        .display = board_lcd->display,
        .touch = board_lcd->touch,
        .f_ui_init = _ui_init,
        .rotation = display_device_get_width(board_lcd->display) > display_device_get_height(board_lcd->display) ? LV_DISP_ROTATION_0 : LV_DISP_ROTATION_90
    };

    if(board_lcd->data_width == 24)
    {
        DBG_INFO("Using RGB888 color format\n");
        lvgl_helper.color_format = LV_COLOR_FORMAT_RGB888;
    }
    else if(board_lcd->data_width == 16)
    {
        DBG_INFO("Using RGB565 color format\n");
        lvgl_helper.color_format = LV_COLOR_FORMAT_RGB565;
    }
    else
    {
        lvgl_helper.color_format = LV_COLOR_FORMAT_UNKNOWN;
    }

    lvgl_helper_init(&lvgl_helper);

    return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _ui_init(lv_display_t *disp)
{
    _show_screen(LCD_ACTIVE_SCREEN_MAIN);

    board_set_backlight(60.0);

    lv_timer_create(_timer_runtime, 1000, NULL);
}

static void _show_screen(LCD_ACTIVE_SCREEN_T screen)
{
    _active_screen = screen;
    _lbl_runtime = NULL;
    _lbl_counter = NULL;
    if(screen < LCD_ACTIVE_SCREEN_MAX && f_create_screen[screen])
    {
        f_create_screen[screen]();
    }
}

static void _create_screen_main(void)
{
    // Clean the screen
    lv_obj_t* scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // Create the logo at the top left
    lv_obj_t* image_logo = lv_img_create(scr);
    lv_obj_align(image_logo, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_img_set_src(image_logo, &schukat_logo);
    // Create the display information on the top right
    lv_obj_t* label_display_size = lv_label_create(scr);
    lv_label_set_text_fmt(label_display_size, "Display: %s\"", board_lcd->screen_diagonal);
    lv_obj_set_style_text_font(label_display_size, &lv_font_montserrat_10, LV_STATE_DEFAULT);
    lv_obj_align(label_display_size, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_t* label_display_resolution = lv_label_create(scr);
    lv_label_set_text_fmt(label_display_resolution, "Resolution: %d x %d", (int)display_device_get_width(board_lcd->display), (int)display_device_get_height(board_lcd->display));
    lv_obj_set_style_text_font(label_display_resolution, &lv_font_montserrat_10, LV_STATE_DEFAULT);
    lv_obj_align(label_display_resolution, LV_ALIGN_TOP_RIGHT, -5, 20);
    // Create the title in the middle
    lv_obj_t* label = lv_label_create(scr);
    lv_label_set_text(label, "LVGL Demo");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 80);
    // Create the runtime labels
    lv_obj_t* label_runtime_title = lv_label_create(scr);
    lv_label_set_text(label_runtime_title, "Runtime:");
    lv_obj_align(label_runtime_title, LV_ALIGN_LEFT_MID, 10, -10);
    _lbl_runtime = lv_label_create(scr);
    lv_label_set_text_fmt(_lbl_runtime, "%02u:%02u min", (unsigned int)(_runtime_seconds / 60), (unsigned int)(_runtime_seconds % 60));
    lv_obj_align(_lbl_runtime, LV_ALIGN_RIGHT_MID, -10, -10);
    // Create the counter labels
    lv_obj_t* label_counter_title = lv_label_create(scr);
    lv_label_set_text(label_counter_title, "Counter:");
    lv_obj_align(label_counter_title, LV_ALIGN_LEFT_MID, 10, 10);
    _lbl_counter = lv_label_create(scr);
    lv_label_set_text_fmt(_lbl_counter, "%u", (unsigned int)_counter);
    lv_obj_align(_lbl_counter, LV_ALIGN_RIGHT_MID, -10, 10);
    // Create the buttons
    lv_obj_t* buttons[3] = {0};
    // Create the button that increments the counter
    buttons[0] = _create_button(scr);
    lv_obj_add_event_cb(buttons[0], _button_increment_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(buttons[0], LV_ALIGN_BOTTOM_LEFT, 5, -35);
    lv_obj_t* button_increment_label = lv_label_create(buttons[0]);
    lv_label_set_text(button_increment_label, "+");
    lv_obj_set_style_text_color(button_increment_label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_center(button_increment_label);
    // Create the button that switches to LCD_ACTIVE_SCREEN_IMAGE
    buttons[1] = _create_button(scr);
    lv_obj_add_event_cb(buttons[1], _button_handler, LV_EVENT_CLICKED, (void*)LCD_ACTIVE_SCREEN_IMAGE);
    lv_obj_align(buttons[1], LV_ALIGN_BOTTOM_MID, 0, -35);
    lv_obj_remove_flag(buttons[1], LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_t* image_button_image = lv_img_create(buttons[1]);
    lv_obj_center(image_button_image);
    lv_img_set_src(image_button_image, &button_landscape);
    // Create the button that switches to LCD_ACTIVE_SCREEN_INFO
    buttons[2] = _create_button(scr);
    lv_obj_add_event_cb(buttons[2], _button_handler, LV_EVENT_CLICKED, (void*)LCD_ACTIVE_SCREEN_INFO);
    lv_obj_align(buttons[2], LV_ALIGN_BOTTOM_RIGHT, -5, -35);
    lv_obj_remove_flag(buttons[2], LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_t* image_button_info = lv_img_create(buttons[2]);
    lv_obj_center(image_button_info);
    lv_img_set_src(image_button_info, &esope);
    // Make all three buttons the same size
    for(int i = 0; i < 3; i++)
    {
        lv_obj_set_width(buttons[i], display_device_get_width(board_lcd->display) / 4);
        lv_obj_set_height(buttons[i], 35);
    }
    // Create the version label
    lv_obj_t* label_version = lv_label_create(scr);
    lv_label_set_text_fmt(label_version, "Version: %s", version_get_string());
    lv_obj_align(label_version, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
}

static void _create_screen_image(void)
{
    // Clean the screen
    lv_obj_t* scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // Image shown in the center
    lv_obj_t* image = lv_img_create(scr);
    lv_obj_center(image);
    lv_img_set_src(image, &landscape);
    // Button for back
    lv_obj_t* button_back = _create_button(scr);
    lv_obj_add_event_cb(button_back, _button_handler, LV_EVENT_CLICKED, (void*)LCD_ACTIVE_SCREEN_MAIN);
    lv_obj_align(button_back, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_remove_flag(button_back, LV_OBJ_FLAG_PRESS_LOCK);
    // Label for the back button
    lv_obj_t* button_label = lv_label_create(button_back);
    lv_label_set_text(button_label, "<");
    lv_obj_set_style_text_color(button_label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_center(button_label);
}

static void _create_screen_info(void)
{
    // Clean the screen
    lv_obj_t* scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // Button for back
    lv_obj_t* button_back = _create_button(scr);
    lv_obj_add_event_cb(button_back, _button_handler, LV_EVENT_CLICKED, (void*)LCD_ACTIVE_SCREEN_MAIN);
    lv_obj_align(button_back, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_remove_flag(button_back, LV_OBJ_FLAG_PRESS_LOCK);
    // Label for the back button
    lv_obj_t* button_label = lv_label_create(button_back);
    lv_label_set_text(button_label, "<");
    lv_obj_set_style_text_color(button_label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_center(button_label);
    // Show the header and make it wrap
    lv_obj_t* label = lv_label_create(scr);
    lv_label_set_text(label, "Demo Software and Description");
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label, display_device_get_width(board_lcd->display) - 10);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
    // Show the description and make it wrap
    lv_obj_t* label2 = lv_label_create(scr);
    lv_label_set_text(label2, "Scan the QR-Code for the GitHub Link to this Demo.");
    lv_label_set_long_mode(label2, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label2, display_device_get_width(board_lcd->display) - 10);
    lv_obj_align(label2, LV_ALIGN_TOP_MID, 0, 40);
    // Show the QR-Code
    lv_obj_t* image = lv_img_create(scr);
    lv_img_set_src(image, &qr_sld_demo);
    lv_obj_align(image, LV_ALIGN_BOTTOM_MID, 0, -40);
    // Show powered by ESoPe
    lv_obj_t* label_powered_by = lv_label_create(scr);
    lv_label_set_text(label_powered_by, "powered by");
    lv_obj_align(label_powered_by, LV_ALIGN_BOTTOM_RIGHT, -75, -5);
    // Create the image of ESoPe
    lv_obj_t* image_esope = lv_img_create(scr);
    lv_obj_align(image_esope, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
    lv_img_set_src(image_esope, &esope);
}

static void _button_handler(lv_event_t* e)
{
    LCD_ACTIVE_SCREEN_T screen = (LCD_ACTIVE_SCREEN_T)lv_event_get_user_data(e);
    _show_screen(screen);
}

static void _button_increment_handler(lv_event_t* e)
{
    _counter++;
    lv_label_set_text_fmt(_lbl_counter, "%u", (unsigned int)_counter);
    lv_obj_invalidate(_lbl_counter);
}

static lv_obj_t* _create_button(lv_obj_t* parent)
{
    lv_obj_t* button = lv_button_create(parent);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xB0B0B0), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, 0x80, LV_PART_MAIN);
    return button;
}

void _timer_runtime(lv_timer_t *tmr)
{
    _runtime_seconds++;
    if(_lbl_runtime)
    {
        lv_label_set_text_fmt(_lbl_runtime, "%02u:%02u min", (unsigned int)(_runtime_seconds / 60), (unsigned int)(_runtime_seconds % 60));
        lv_obj_invalidate(_lbl_runtime);
    }
}

#endif // defined(KERNEL_USES_LVGL)