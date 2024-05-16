/**
 * @file video.h
 * @copyright Urheberrecht 2024 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 * @author Tim Koczwara
 * 
 * @brief 
 * 
 * @version 1.00 (28.03.2024)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_VIDEO_H_
#define __MODULE_VIDEO_H_

#include "module_public.h"
#if MODULE_ENABLE_GUI
#include "module/enum/function_return.h"
#include "module/gui/eve_ui/component.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Source of the video that should be played
typedef enum video_source_e
{
    /// @brief Video file is stored in flash that is connected to the eve chip.
    VIDEO_SOURCE_EVE_FLASH = 0,
    /// @brief Stream the video from local ram or flash if it is completely in the memory.
    VIDEO_SOURCE_LOCAL_MEMORY, // TODO: Implement!
    /// @brief Video is streamed
    VIDEO_SOURCE_STREAM, // TODO: Implement!

}VIDEO_SOURCE_T;

/// @brief Source of the video that should be played
typedef enum video_control_e
{
    /// @brief Stop the playback of the video. On @c VIDEO_CONTROL_PLAY the playback starts from the beginning.
    VIDEO_CONTROL_STOP = 0,
    /// @brief Start playback of the video
    VIDEO_CONTROL_PLAY,
    /// @brief Pause playback of the video to resume later using @c VIDEO_CONTROL_PLAY.
    VIDEO_CONTROL_PAUSE,

}VIDEO_CONTROL_T;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// @brief Structure for a video component on the screen.
typedef struct video_s video_t;

/// @brief Options for a video component on the screen.
typedef struct video_options_s video_options_t;

/**
 * @brief Callback for the stop of the video.
 * 
 * @param video     Pointer to the video that stopped.
 * @param control   @c VIDEO_CONTROL_STOP or @c VIDEO_CONTROL_PAUSE to indicate a stop or pause.
 */
typedef void (*video_stopped_cb_t)(video_t* video, VIDEO_CONTROL_T control);

/// @brief Options for a video component on the screen.
struct video_options_s
{
    /// @brief Name for the video in memory. Should be unique.
    const char* name;
    /// @brief Source of the video that should be played
    VIDEO_SOURCE_T source;
    /// @brief Width and height of the video in pixel.
    eve_ui_size_t size;
    /// @brief If set, video will be played without audio.
    bool mute;
    /// @brief If set, video will be played in fullscreen.
    bool fullscreen;
    /// @brief Event for the video 
    video_stopped_cb_t f_cb_stopped;
    // Union used to differentiate between variables on different source types.
    union
    {
        /// @brief Options for @c VIDEO_SOURCE_EVE_FLASH
        struct 
        {
            /// @brief Address in eve external flash where the Video is stored.
            uint32_t flash_address;
        }eve_flash;
        /// @brief Options for @c VIDEO_SOURCE_LOCAL_MEMORY
        struct 
        {
            // TODO: Options for @c VIDEO_SOURCE_LOCAL_MEMORY
        }local;
        /// @brief Options for @c VIDEO_SOURCE_STREAM
        struct 
        {
            // TODO: Options for @c VIDEO_SOURCE_STREAM
        }stream;
    };
};

/// @brief Structure for a video component on the screen.
struct video_s
{
    /// Component Object used for painting this object on the screen
    component_t component;
    /// @brief Options for the video
    video_options_t options;
    /// @brief Indicates whether video is currently loaded or needs to be written into eve chip.
    /// Depends on @c options.source because the flash source is always loaded.
    bool is_loaded;
    /// @brief Indicates whether video should be played or not.
    VIDEO_CONTROL_T control;
    /// @brief Last control during paint to detect changes.
    VIDEO_CONTROL_T last_control;
    /// @brief Task used to check for playback end
    system_task_t task;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Intializes the video component.
 * 
 * @param video     Pointer to the video component that should be initialized.
 * @param options   Pointer to the options for the video. Will be copied.
 * @param location  Coordinates and size in pixel on the chart's parent.
 * @return          FUNCTION_RETURN_OK on success or other value if init failed.
**/
FUNCTION_RETURN_T video_init(video_t* video, const video_options_t* options, eve_ui_location_t location);
/**
 * @brief Controls the video playback.
 * 
 * @param video     Pointer to the video component.
 * @param control   Control action that should be done (play, pause, ...).
 * @return          FUNCTION_RETURN_OK on success or other value if control failed.
 */
FUNCTION_RETURN_T video_control(video_t* video, VIDEO_CONTROL_T control);

#endif // MODULE_ENABLE_GUI

#endif /* __MODULE_VIDEO_H_ */