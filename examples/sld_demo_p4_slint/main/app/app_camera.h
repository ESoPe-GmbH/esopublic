/**
 * @file app_camera.h
 * @author Tim Koczwara
 * 
 * @brief 
 * 
 * @version 1.00 (16.04.2025)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef APP_APP_CAMERA_H_
#define APP_APP_CAMERA_H_

#include "module_public.h"
#include "module/enum/function_return.h"
#if CONFIG_IDF_TARGET_ESP32P4

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct camera_buffer_s camera_buffer_t;

typedef void (*camera_cb_t)(camera_buffer_t* buffer);

struct camera_buffer_s
{
    void* user;
    void *buffer;
    size_t width;
    size_t height;
    size_t bytes_per_pixel;
    camera_cb_t f;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Intializes the application module
**/
void app_camera_init(void);

/**
 * Deinitializes the application module
 * */
void app_camera_deinit(void);

/**
 * Starts the camera capture stream
 * @param buffer Pointer to the pixel buffer to store the image data.
 * @return FUNCTION_RETURN_OK on success, or an error code on failure
 * */
FUNCTION_RETURN_T app_camera_start(const camera_buffer_t* buffer);

/**
 * Stops the camera capture stream
 * @return FUNCTION_RETURN_OK on success, or an error code on failure
 * */
FUNCTION_RETURN_T app_camera_stop(void);

void app_camera_capture_frame(void);

bool app_camera_has_frame_captured(void);

/**
 * Checks if the camera is initialized
 * @return true if the camera is initialized, false otherwise
 * */
bool app_camera_is_initialized(void);

/**
 * Checks if the camera is capturing
 * @return true if the camera is capturing, false otherwise
 * */
bool app_camera_is_capturing(void);

#endif

#endif /* APP_APP_CAMERA_H_ */