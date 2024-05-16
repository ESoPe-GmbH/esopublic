# GUI

Contains code for controlling the FTDI EVE graphic chip and a library of UI elements to draw.

## EVE

The EVE chip offers 2 ways to draw something on the display.

1. Draw directly by writing commands into the display list of the eve chip.
    1. `eve_dl()`_start is needed to start a new display list
    1. `eve_dl()` can be used to add display list commands to the display list.
        See EVE manual and eve_register.h for details. The defines in eve_register.h are named after
        the manual, but every define has the prefix EVE_.
    1. `eve_dl_swap()` is needed to end the display list and show the graphic on the screen.
1. Use the co-processor of the eve to store commands and then print it to the screen.
    See eve_copro.h for details. The process is like the display list process.
    1. Check if `eve_copro_is_ready()` returns true -> Otherwise it might not work.
    1. Use `eve_copro_dlstart()` to start a new display list via the co-processor.
    1. (Optional) Use `eve_copro_coldstart()` to reactivate the standards of the co-processor.
        For example if the color of buttons in the screen before were changed, this sets it back to default
        color.
    1. Now the co-processor commands can be called. See `eve_copro.h` for detailed description of the
        following functions.
        1. `eve_copro_write_command()` can add display list commands (same as eve_dl to co-processor).
        1. `eve_copro_set_color()` to change the color of the following component.
        1. `eve_copro_set_foreground()` to change the color of the following component.
        1. `eve_copro_set_background()` to change the color of the following component.
        1. `eve_copro_set_gradientcolor()` to change the color of the following component.
        1. `eve_copro_gradient()` to draw a rectangle which was selected via scissor.
        1. `eve_copro_text()` to add a text to the display.
        1. `eve_copro_button()` to to add a button to the display.
    1. End the co-processor display list via `eve_copro_swap()`.

The gui module uses already the functionality mentioned above. So when using the gui, eve_copro.h might
not be accessed manually. Initialization and handle of eve.h are needed nevertheless.

## EVE UI

The screen is the basic component in the UI module, to it you can add other components like buttons, text objects and images.

When using the screen module, it is necessary you have initialized the eve chip via `eve_init` in
`eve.h`.

The screen module has 2 purposes. First is the interface to the complete screen with the following functions:

- `screen_device_init` -> Initializes the screen device based on the device configuration
- `screen_device_on`				-> Switches the display on
- `screen_device_off`			-> Switches the display completely off
- `screen_device_set_dimming`	-> Can be used to dim the brightness of the display.
- `screen_device_get_width`		-> Returns the width of the display in pixel
- `screen_device_get_height`		-> Returns the height of the display in pixel

Second purpose is to offer a screen objects which works like a panel for the whole display. Like a Frame for
a fullscreen application. The screen can always have one screen object active which can be repainted if
necessary.

The screen object functions are:

- `screen_init_object`	-> Initializes a screen object. Must be called first for every screen object.
- `screen_add_component`	-> Adds a component to the screen object. A component can be a text, button, etc.
- `screen_paint`			-> Sets the new display object and sets a flag for repainting.
- `screen_repaint`		-> Sets a flag for repainting the last set display object.

1. To implement a screen, declare a `screen_t`.
2. Initialize the screen object with `screen_init`. If you need a handle function, that is called regularly when the screen is visible, add it in the `screen_init` function. Also you can provide `prepaint` function that will be called before the screen is painted to the display. You can use it for example to fetch current data to be displayed.
3. Add components like buttons, etc. via `screen_add_component` to the screen.

```c
screen_t 	screen_x_object;
button_t 	screen_x_button_a;
text_t	screen_x_text_a;

void screen_x_handle(screen_t* obj, uint32_t option)	// Handle function only if needed -> If not needed set NULL in screen_init_object
{
    //... Handle something
}
//...

// Initializes the screen object, sets the background color to white and adds a handle function.
screen_init_object(&screen_x_object, color_get(COLOR_WHITE), screen_x_handle, NULL);

// Initialize the components
text_init(&screen_x_text_a, 10, 20, "ESoPe");
button_init(&screen_x_button_a, 10,  100, 100, 70, "Test 1");

// Add components to the screen_object
screen_add_component(&screen_x_object, (component_t*)&screen_x_text_a);
screen_add_component(&screen_x_object, (component_t*)&screen_x_button_a);

// Paint the screen component.
screen_paint(&screen_x_object);
```

If you plan on using multiple screens we recommended you use one header and source file per screen exposing only an init function which takes as an input a pointer to a `screen_t`. In it you call the init functions for the screen object and components and add the components to the screen. You can either store the components and their state in static variables(that is in the RAM) or put them in a structure and allocate them dynamically on the heap. If you use the dynamic allocation you can store the pointer to the structure in the `user` variable of the `screen_t` structure to easily access it in the callback functions which take `screen_t*` as a parameter.

Then in another source file you declare one `screen_t` structure per screen and initialize them by calling the init functions from your source files. After that you can implement some logic to show the different screens and change between them by calling `screen_paint` with the `screen_t` structure of the screen you want to show on the display.