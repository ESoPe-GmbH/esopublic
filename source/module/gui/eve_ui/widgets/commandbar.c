/**
 * @file commandbar.c
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "commandbar.h"
#include "module/util/assert.h"
#include "../font.h"
#include "module/convert/math.h"
#include <string.h>

#if MODULE_ENABLE_GUI

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _add_item(commandbar_t* c, commandbar_item_t** first, commandbar_item_t* item);

static void _remove_item(commandbar_item_t* first, commandbar_item_t* item);

static void _init_item_button(commandbar_item_t* item);

static eve_ui_size_t _prepare_item(commandbar_t* commandbar, commandbar_item_t* item, eve_t* eve, size_t w, size_t h);

static void _paint_separator(commandbar_t* commandbar, eve_ui_point_t p, eve_t* eve, size_t w, size_t h);

/**
 * @brief Draw the commandbar on the display
 *
 * @param c     Pointer to the commandbar
 * @param p     Coordinates of the parent component on the screen
 */
static void _paint(commandbar_t* commandbar, eve_ui_point_t p);

static void _cb_button(button_t* button);

static void _event(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data);

extern void component_trigger_event(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN_T commandbar_init(commandbar_t* c, const commandbar_options_t* options, eve_ui_location_t location)
{
    ASSERT_RET_NOT_NULL(c, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(options, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    component_init(&c->component, COMPONENT_TYPE_COMMANDBAR, (component_paint_cb_t)_paint);
    c->component.f_cb_event_internal = _event;
    
    // Copy options into the chart data.
    memcpy(&c->options, options, sizeof(commandbar_options_t));

    c->component.size.width = location.size.width;
    c->component.size.height = location.size.height;
    c->component.origin.x = location.origin.x;
    c->component.origin.y = location.origin.y;
    rect_init(&c->rect_background, 0, 0, location.size.width, location.size.height, 1);
    rect_set_color(&c->rect_background, color_get(BUTTON_DEFAULT_BACKGROUND_COLOR));
    c->rect_background.component.parent_component = &c->component;

    if(options->num_items > 0)
    {
        c->items = options->items;
        for(int i = 1; i < options->num_items; i++)
        {
            options->items[i - 1].next = &options->items[i];
        }
        options->items[options->num_items - 1].next = NULL;

        for(int i = 0; i < options->num_items; i++)
        {
            options->items[i].commandbar = c;
            _init_item_button(&options->items[i]);
        }
    }

    if(options->num_footer > 0)
    {
        c->footer = options->footer;
        for(int i = 1; i < options->num_footer; i++)
        {
            options->footer[i - 1].next = &options->footer[i];
        }
        options->footer[options->num_footer - 1].next = NULL;

        for(int i = 0; i < options->num_footer; i++)
        {
            options->footer[i].commandbar = c;
            _init_item_button(&options->footer[i]);
        }
    }

    return FUNCTION_RETURN_OK;
}

void commandbar_item_add(commandbar_t* c, commandbar_item_t* item)
{
    ASSERT_RET_NOT_NULL(c, NO_ACTION, NO_RETURN);
    _add_item(c, &c->items, item);
}

void commandbar_item_remove(commandbar_t* c, commandbar_item_t* item)
{
    ASSERT_RET_NOT_NULL(c, NO_ACTION, NO_RETURN);
    _remove_item(c->items, item);
}

void commandbar_footer_add(commandbar_t* c, commandbar_item_t* item)
{
    ASSERT_RET_NOT_NULL(c, NO_ACTION, NO_RETURN);
    _add_item(c, &c->footer, item);
}

void commandbar_footer_remove(commandbar_t* c, commandbar_item_t* item)
{
    ASSERT_RET_NOT_NULL(c, NO_ACTION, NO_RETURN);
    _remove_item(c->footer, item);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _add_item(commandbar_t* c, commandbar_item_t** first, commandbar_item_t* item)
{
    ASSERT_RET_NOT_NULL(c, NO_ACTION, NO_RETURN);
    ASSERT_RET_NOT_NULL(item, NO_ACTION, NO_RETURN);

    if(*first == NULL)
    {
        *first = item;
        item->next = NULL;
    }
    else
    {
        commandbar_item_t* comp = *first;
        if(comp == item)
        {
            return;
        }
        while(comp->next)
        {
            if(comp == item)
            {
                return;
            }
            comp = comp->next;
        }
        comp->next = item;
    }
    item->commandbar = c;
    _init_item_button(item);
}

static void _remove_item(commandbar_item_t* first, commandbar_item_t* item)
{
    ASSERT_RET_NOT_NULL(item, NO_ACTION, NO_RETURN);

    if(first == NULL)
    {
        return;
    }
    else
    {
        if(first == item)
        {
            first = item->next;
            item->next = NULL;
            return;
        }

        commandbar_item_t* comp = first;
        do
        {
            if(comp->next == item)
            {
                comp->next = item->next;
                item->next = NULL;
                return;
            }
            comp = comp->next;
        }while(comp);
    }
}

static void _init_item_button(commandbar_item_t* item)
{
    button_init(&item->internal_button, 0, 0, 0, 0, item->caption);
    if(item->icon)
    {
        button_set_figure(&item->internal_button, item->icon, item->icon_pos);
        // TODO: Change scale of figure based on commandbar height and width
    }            
    if(item->caption_font)
    {
        button_set_font(&item->internal_button, item->caption_font);
    }
    button_set_type(&item->internal_button, BUTTON_TYPE_RECT);
    button_set_textcolor(&item->internal_button, color_get(COLOR_BLACK));
    button_set_text_horizontal_alignment(&item->internal_button, TEXT_H_ALIGNMENT_LEFT);
    button_set_action(&item->internal_button, _cb_button);
    item->internal_button.enable_text_shadow = false;
    item->internal_button.component.parent_component = &item->commandbar->component;
}

static eve_ui_size_t _prepare_item(commandbar_t* commandbar, commandbar_item_t* item, eve_t* eve, size_t w, size_t h)
{
    button_t* b = &item->internal_button;
    
    // Icon width and icon height
    size_t iw = 0;
    size_t ih = 0;
    
    // Text width and text height
    size_t tw = 0;
    size_t th = 0;

    // Button width and button height
    size_t bw = 10;
    size_t bh = 10;

    if(b->text)
    {
        th = font_get_height(eve, b->font) + 5;
        tw = font_get_width_string(eve, b->font, b->text) + 5;
    }

    if(item->icon)
    {
        iw = item->icon->size.width + 2 * b->figure_margin;
        ih = item->icon->size.height + 2 * b->figure_margin;
    }

    // TODO: Padding is fixed with 5 below, use custom padding

    if(b->figure)
    {
        if(item->icon_pos & (BUTTON_FIGURE_POS_LEFT | BUTTON_FIGURE_POS_RIGHT | BUTTON_FIGURE_POS_CENTER_X))
        {
            // Text and image are next to each other
            bw = 5 + tw + iw + 5;
            bh = 5 + MATH_MAX(th, ih) + 5;
        }
        else if(item->icon_pos & (BUTTON_FIGURE_POS_TOP | BUTTON_FIGURE_POS_BOTTOM | BUTTON_FIGURE_POS_CENTER_Y))
        {
            // Text and image are above to each other
            bw = 5 + MATH_MAX(tw, iw) + 5;
            bh = 5 + th + ih + 5;
        }
        else // Custom -> based on x/y from image... unsupported here
        {
            button_set_figure(&item->internal_button, NULL, item->icon_pos);
            bw = 5 + tw + 5;
            bh = 5 + th + 5;
        }
    }
    else // No image, only text
    {
        bw = 5 + tw + 5;
        bh = 5 + th + 5;
    }

    if(commandbar->options.orientation == COMMANDBAR_ORIENTATION_VERTICAL)
    {
        bw = w;
        // if(bh > h)
        // {
        //     bh = h;
        // }
        // h -= bh;
    }
    else
    {
        bh = h;
        // if(bw > w)
        // {
        //     bw = w;
        // }
        // w -= bw;
    }

    b->component.size.width = bw;
    b->component.size.height = bh;

    return (eve_ui_size_t){.width = bw, .height = bh};
}

static void _paint_separator(commandbar_t* commandbar, eve_ui_point_t p, eve_t* eve, size_t w, size_t h)
{

    eve_copro_check_command_buffer(eve, 4 * 7);
    eve_copro_set_color(eve, color_get(COLOR_DIM_GRAY));
    eve_copro_write_command(eve, EVE_VERTEX_FORMAT(0)); // Pixel precision: 1
    // Set line width
    eve_copro_write_command(eve, EVE_LINE_WIDTH(16));
    // Draw axis
    eve_copro_write_command(eve, EVE_BEGIN(EVE_LINES));
    // Y-Axis
    if(commandbar->options.orientation == COMMANDBAR_ORIENTATION_HORIZONTAL)
    {
        eve_copro_write_command(eve, EVE_VERTEX2F(p.x + 2, p.y + 2));
        eve_copro_write_command(eve, EVE_VERTEX2F(p.x + 2, p.y + h - 4));
    }
    else
    {
        eve_copro_write_command(eve, EVE_VERTEX2F(p.x + 2, p.y + 2));
        eve_copro_write_command(eve, EVE_VERTEX2F(p.x + w - 4, p.y + 2));
    }
    eve_copro_write_command(eve, EVE_END());
}

static void _paint(commandbar_t* commandbar, eve_ui_point_t p)
{
	eve_t* eve = component_get_eve(&commandbar->component);

    p.x += commandbar->component.origin.x + commandbar->options.margin.left;
    p.y += commandbar->component.origin.y + commandbar->options.margin.top;
    size_t w = commandbar->component.size.width - commandbar->options.margin.left - commandbar->options.margin.right;
    size_t h = commandbar->component.size.height - commandbar->options.margin.top - commandbar->options.margin.bottom;

    eve_ui_point_t p_origin_footer;

    if(commandbar->options.orientation == COMMANDBAR_ORIENTATION_HORIZONTAL)
    {
        p_origin_footer.x = p.x + w;
        p_origin_footer.y = p.y;
    }
    else
    {
        p_origin_footer.x = p.x;
        p_origin_footer.y = p.y + h;
    }
    
    commandbar_item_t* item = commandbar->items;

    commandbar->rect_background.component.size.width = w;
    commandbar->rect_background.component.size.height = h;
    commandbar->rect_background.component.paint_function(&commandbar->rect_background.component, p);

    while(item)
    {
        eve_ui_size_t item_size = _prepare_item(commandbar, item, eve, w, h);
        
        item->internal_button.component.paint_function(&item->internal_button.component, p);

        if(commandbar->options.orientation == COMMANDBAR_ORIENTATION_HORIZONTAL)
        {
            p.x += item_size.width;
        }
        else // Vertical
        {
            p.y += item_size.height;
        }

        // Add a separator line between the buttons.
        _paint_separator(commandbar, p, eve, w, h);

        if(commandbar->options.orientation == COMMANDBAR_ORIENTATION_HORIZONTAL)
        {
            p.x += 5;
        }
        else // Vertical
        {
            p.y += 5;
        }

        item = item->next;
    }

    item = commandbar->footer;
    p = p_origin_footer;

    while(item)
    {
        eve_ui_size_t item_size = _prepare_item(commandbar, item, eve, w, h);

        if(commandbar->options.orientation == COMMANDBAR_ORIENTATION_HORIZONTAL)
        {
            p.x -= item_size.width;
        }
        else // Vertical
        {
            p.y -= item_size.height;
        }

        item->internal_button.component.paint_function(&item->internal_button.component, p);

        if(commandbar->options.orientation == COMMANDBAR_ORIENTATION_HORIZONTAL)
        {
            p.x -= 5;
        }
        else // Vertical
        {
            p.y -= 5;
        }

        // Add a separator line between the buttons.
        _paint_separator(commandbar, p, eve, w, h);

        item = item->next;
    }
}

static void _cb_button(button_t* button)
{
    commandbar_item_t* item = __containerof(button, commandbar_item_t, internal_button);
    if(item->cb_click)
    {
        item->cb_click(item);
    }
}

static void _event(component_t* c, COMPONENT_EVENT_T event, const component_event_t* data)
{
	commandbar_t* commandbar = __containerof(c, commandbar_t, component);
    
    commandbar_item_t* item = commandbar->items;
	
	while(item)
	{
		component_trigger_event(&item->internal_button.component, event, data);
		item = item->next;
	}
    
    item = commandbar->footer;
	
	while(item)
	{
		component_trigger_event(&item->internal_button.component, event, data);
		item = item->next;
	}
}

#endif // MODULE_ENABLE_GUI