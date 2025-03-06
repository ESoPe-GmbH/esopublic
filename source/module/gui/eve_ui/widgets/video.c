/**
 * @file video.c
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "video.h"
#include "module/convert/string.h"
#include "module/comm/dbg.h"
#include "module/util/assert.h"
#include "module/gui/eve/eve_copro.h"
#include "module/gui/eve/eve_spi.h"
#include "module/gui/eve/eve_memory.h"
#include "module/gui/eve_ui/font.h"
#include "module/gui/eve_ui/screen.h"

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

/**
 * @brief Draw the video on the display
 *
 * @param c     Pointer to the video
 * @param p     Coordinates of the parent component on the screen
 */
static void _paint(video_t* c, eve_ui_point_t p);

static int _handle_video_playback(struct pt* pt);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

FUNCTION_RETURN_T video_init(video_t* video, const video_options_t* options, eve_ui_location_t location)
{
    ASSERT_RET_NOT_NULL(video, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    ASSERT_RET_NOT_NULL(options, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);
    component_init(&video->component, COMPONENT_TYPE_VIDEO, (component_paint_cb_t)_paint);

    // Copy options into the data.
    memcpy(&video->options, options, sizeof(video_options_t));

    video->component.size = location.size;
    video->component.origin = location.origin;

    video->component.mem_file_ptr = eve_memory_register(&screen_get_default_device()->eve, options->name, NULL, 0);
    system_task_init_protothread(&video->task, false, _handle_video_playback, video);

    return FUNCTION_RETURN_OK;    
}

FUNCTION_RETURN_T video_control(video_t* video, VIDEO_CONTROL_T control)
{
    ASSERT_RET_NOT_NULL(video, NO_ACTION, FUNCTION_RETURN_PARAM_ERROR);

    if(video->last_control == VIDEO_CONTROL_STOP && control == VIDEO_CONTROL_PAUSE)
    {
        DBG_ERROR("Cannot pause a stopped video\n");
        return FUNCTION_RETURN_PARAM_ERROR;
    }
    
    if(control == video->control)
    {
        return FUNCTION_RETURN_OK;
    }

    DBG_INFO("video_control %d / %d / %d\n", video->last_control, video->control, control);

    video->control = control;
    
    if(control == VIDEO_CONTROL_PLAY && video->last_control == VIDEO_CONTROL_STOP)
    {        
        // Repainting the component will load the video in the _paint function
        screen_repaint_by_component(&video->component);
    }
    else
    {
        eve_t* eve = component_get_eve(&video->component);
        switch(control)
        {
            case VIDEO_CONTROL_PLAY:
                eve_spi_write_8(eve, EVE_REG_PLAY_CONTROL, 1);
                break;
            case VIDEO_CONTROL_PAUSE:
                eve_spi_write_8(eve, EVE_REG_PLAY_CONTROL, 0);
                break;
            default:
                eve_spi_write_8(eve, EVE_REG_PLAY_CONTROL, 0xFF);
                break;
        }
        video->last_control = video->control;
    }

    return FUNCTION_RETURN_OK;   
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _paint(video_t* c, eve_ui_point_t p)
{
    eve_t* eve = component_get_eve(&c->component);

    DBG_INFO("Paint %s -> %d %d\n", c->component.mem_file_ptr->filename, c->control, c->last_control);
    
    if(c->last_control == VIDEO_CONTROL_STOP && c->control == VIDEO_CONTROL_PLAY) // Stop -> Play
    {
        uint32_t options = EVE_OPT_PLAYVIDEO_NOTEAR;
        if(c->options.fullscreen)
        {
            options |= EVE_OPT_PLAYVIDEO_FULLSCREEN;
        }
        else
        {
            // TODO: Check if correct
            options |= EVE_OPT_PLAYVIDEO_OVERLAY;
        }
        if(!c->options.mute)
        {
            options |= EVE_OPT_PLAYVIDEO_SOUND;
            if(eve->hw.io_sound_enable.pin != PIN_NONE)
            {
                MCU_IO_SET_HANDLER(eve->hw.io_sound_enable, 1);
            }
        }

        // Start playing the video.
        if(c->options.source == VIDEO_SOURCE_EVE_FLASH)
        {
            options |= EVE_OPT_PLAYVIDEO_FLASH;
            DBG_INFO("Play video\n");
            eve_copro_flashsource(eve, c->options.eve_flash.flash_address);
            eve_copro_playvideo(eve, options, NULL, 0);
            // Video is stored at the beginning of the RAM and overwrites existing images at this place. Therefore we need to clear all memory pointers and reserve.
            eve_memory_clear(eve);
            
            // Read the size of the ram that was allocated for the video.
            // uint32_t size = eve_copro_getptr(eve);
            // Update memory file pointer
            c->component.mem_file_ptr->address = 0;
            c->component.mem_file_ptr->data_length = c->options.size.width * c->options.size.height * 2;
            eve_memory_register_address(eve, 0, c->component.mem_file_ptr->data_length);
        }
        else if(c->options.source == VIDEO_SOURCE_LOCAL_MEMORY)
        {
            // TODO: Implement direct playback.
        }
        else if(c->options.source == VIDEO_SOURCE_STREAM)
        {
            options |= EVE_OPT_PLAYVIDEO_MEDIAFIFO;
            // TODO: Implement playback via mediafifo.
        }

        system_task_add(&c->task); 
    }
    else if(c->control == VIDEO_CONTROL_PAUSE)
    {
        if(c->last_control == VIDEO_CONTROL_PLAY) // Play -> Pause
        {
            // Set video control to pause
            eve_spi_write_8(eve, EVE_REG_PLAY_CONTROL, 0);
        }
        // Other cases are ignored, since nothing will be played.
    }
    else if(c->control == VIDEO_CONTROL_PLAY)
    {
        // Do nothing
    }
    else // Stop
    {
        if(c->last_control == VIDEO_CONTROL_PLAY) // Play -> Stop
        {
            // Set video control to stop
            eve_spi_write_8(eve, EVE_REG_PLAY_CONTROL, 0xFF);            
        }
        else if(c->last_control == VIDEO_CONTROL_PAUSE) // Pause -> Stop
        {
            // Set video control to stop
            eve_spi_write_8(eve, EVE_REG_PLAY_CONTROL, 0xFF);
        }
    }
    c->last_control = c->control;
}

static int _handle_video_playback(struct pt* pt)
{
    video_t* video = pt->obj;
    PT_BEGIN(pt);

    do
    {
        if(video->control == video->last_control)
        {
            if(video->control == VIDEO_CONTROL_PLAY || video->control == VIDEO_CONTROL_STOP)
            {
                eve_t* eve = component_get_eve(&video->component);
                uint8_t val = eve_spi_read_8(eve, EVE_REG_PLAY_CONTROL);
                // DBG_INFO("Playback: %02x\n", (uint32_t)val);
                if(val != 0 && eve_copro_has_empty_list(eve))
                {
                    if(eve->hw.io_sound_enable.pin != PIN_NONE)
                    {
                        MCU_IO_SET_HANDLER(eve->hw.io_sound_enable, 0);
                    }
                    video->control = VIDEO_CONTROL_STOP;
                    video->last_control = video->control;
                    PT_INIT(pt);
                    if(val != 0xFF)
                    {                        
                        eve_spi_write_8(eve, EVE_REG_PLAY_CONTROL, 0xFF);
                    }
                    // Video is stored at the beginning of the RAM and overwrites existing images at this place. Therefore we need to clear all memory pointers and reserve.
                    eve_memory_clear(eve);
                    font_initalize_default_fonts(eve);
                    if(video->options.f_cb_stopped)
                    {
                        video->options.f_cb_stopped(video, video->control);
                    }
                    PT_EXIT(pt);
                }
            }
        }
        PT_YIELD(pt);
    }while(true);

    PT_END(pt);
}

#endif