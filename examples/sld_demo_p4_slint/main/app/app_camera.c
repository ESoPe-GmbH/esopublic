/**
 * @file app_camera.c
 **/

#include "sdkconfig.h"
#if CONFIG_IDF_TARGET_ESP32P4

#include "app_camera.h"
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/errno.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_check.h"
#include "linux/videodev2.h"
#include "esp_video_device.h"
#include "esp_video_init.h"
#if CONFIG_EXAMPLE_SCCB_I2C_INIT_BY_APP
#include "driver/i2c_master.h"
#endif
#include "board/board.h"
#include "module/display/display_common.h"
#include "driver/ppa.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
// #include "freertos/portmacro.h"
// #include "freertos/projdefs.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#if CONFIG_EXAMPLE_VIDEO_BUFFER_TYPE_USER
#include "esp_heap_caps.h"

#define MEMORY_TYPE V4L2_MEMORY_USERPTR
#define MEMORY_ALIGN 64
#else
#define MEMORY_TYPE V4L2_MEMORY_MMAP
#endif

#define BUFFER_COUNT 2
#define CAPTURE_SECONDS 3

#define FLAG_CAPTURE_DONE (1 << 0)
#define FLAG_IMAGE_DONE (1 << 1)
#define FLAG_CAPTURE_FRAME (1 << 2)

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _task_camera_capture(void *arg);

static esp_err_t _camera_capture_stream(void);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static const char *TAG = "app_camera";

static const esp_video_init_csi_config_t csi_config[] = {
    {
        .sccb_config = {
            .init_sccb = true,
            .i2c_config = {
                .port      = 1,
                .scl_pin   = 53,
                .sda_pin   = 54,
            },
            .freq = 100000,
        },
        .reset_pin = -1,
        .pwdn_pin  = -1,
    },
};

static const esp_video_init_config_t cam_config = {
    .csi      = csi_config,
};

static ppa_client_handle_t _ppa_handle = NULL;

static EventGroupHandle_t _event_group = NULL;

static bool _is_capturing = false;

static bool _has_camera = false;

static bool _has_frame_captured = false;

static camera_buffer_t _buffer = {0};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void app_camera_init(void)
{
    if(board_lcd == NULL || board_lcd->display == NULL)
    {
        DBG_ERROR("No display connected\n");
    }

    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "Init camera");

    _event_group = xEventGroupCreate();
    if (_event_group == NULL) 
    {
        ESP_LOGE(TAG, "Failed to create event group");
        return;
    }

    DBG_INFO("SCCB I2C: %d SCL=%d SDA=%d\n", csi_config->sccb_config.i2c_config.port, csi_config->sccb_config.i2c_config.scl_pin, csi_config->sccb_config.i2c_config.sda_pin);

    ret = esp_video_init(&cam_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", ret);
        return;
    }

    ppa_client_config_t ppa_config = 
    {
        .oper_type = PPA_OPERATION_SRM,
        .max_pending_trans_num = 1,
        .data_burst_length = PPA_DATA_BURST_LENGTH_16
    };

    ppa_register_client(&ppa_config, &_ppa_handle);
    if (_ppa_handle == NULL) {
        ESP_LOGE(TAG, "Failed to register PPA client");
        esp_video_deinit();
        return;
    }
    

    int fd = open(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, O_RDONLY);
    if (fd < 0) {
        ESP_LOGE(TAG, "failed to open device");
        ppa_unregister_client(_ppa_handle);
        esp_video_deinit();
        return;
    }
    else
    {
        close(fd);
        DBG_INFO("Camera found\n");
    }

    _has_camera = true;
}

FUNCTION_RETURN_T app_camera_start(const camera_buffer_t* buffer)
{
    if(_is_capturing)
    {
        ESP_LOGE(TAG, "Capturing already active");
        return FUNCTION_RETURN_NOT_READY;
    }

    if (buffer == NULL) 
    {
        ESP_LOGE(TAG, "Buffer is NULL");
        return FUNCTION_RETURN_PARAM_ERROR;
    }

    if (buffer->width <= 0 || buffer->height <= 0) 
    {
        ESP_LOGE(TAG, "Invalid image dimensions");
        return FUNCTION_RETURN_PARAM_ERROR;
    }

    if(buffer->bytes_per_pixel < 2 || buffer->bytes_per_pixel > 3)
    {
        ESP_LOGE(TAG, "Invalid bytes per pixel");
        return FUNCTION_RETURN_PARAM_ERROR;
    }

    memcpy(&_buffer, buffer, sizeof(camera_buffer_t));

    if(!_has_camera) 
    {
        ESP_LOGE(TAG, "Camera not initialized");
        return FUNCTION_RETURN_DEVICE_ERROR;
    }

    _is_capturing = true;

    // Start the camera capture stream
    xTaskCreate(_task_camera_capture, "CAM", 8192, NULL, 7, NULL);

    app_camera_capture_frame();

    return FUNCTION_RETURN_OK;
}

FUNCTION_RETURN_T app_camera_stop(void)
{
    if(!_has_camera) 
    {
        ESP_LOGE(TAG, "Camera not initialized");
        return FUNCTION_RETURN_DEVICE_ERROR;
    }

    if (_is_capturing) 
    {
        _is_capturing = false;
        xEventGroupSetBits(_event_group, FLAG_CAPTURE_DONE);
    }
    return FUNCTION_RETURN_OK;
}

void app_camera_capture_frame(void)
{
    if(!_has_camera) 
    {
        ESP_LOGE(TAG, "Camera not initialized");
        return;
    }

    _has_frame_captured = false;
    xEventGroupSetBits(_event_group, FLAG_CAPTURE_FRAME);
}

bool app_camera_has_frame_captured(void)
{
    return _has_frame_captured;
}

bool app_camera_is_initialized(void)
{
    return _has_camera;
}

bool app_camera_is_capturing(void)
{
    return _is_capturing;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _task_camera_capture(void *arg)
{
    esp_err_t err = _camera_capture_stream();
    DBG_INFO("Camera capture stream finished with error: %d", err);

    vTaskDelete(NULL);
}

static esp_err_t _camera_capture_stream(void)
{    
    int fd;
    esp_err_t ret;
    struct v4l2_buffer buf;
    uint8_t *buffer[BUFFER_COUNT];
#if CONFIG_EXAMPLE_VIDEO_BUFFER_TYPE_USER
    uint32_t buffer_size[BUFFER_COUNT];
#endif
    struct v4l2_format init_format;
    struct v4l2_requestbuffers req;
    struct v4l2_capability capability;
    const int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;    
    static uint8_t* display_buffer = NULL;

    // uint32_t width = display_device_get_width(board_lcd->display);
    // uint32_t height = display_device_get_height(board_lcd->display);

    display_buffer = heap_caps_aligned_alloc(MEMORY_ALIGN, _buffer.bytes_per_pixel * _buffer.width * _buffer.height, MALLOC_CAP_SPIRAM | MALLOC_CAP_CACHE_ALIGNED | MALLOC_CAP_DMA);//mcu_heap_calloc(3, width * height);
    if (display_buffer == NULL) 
    {
        ESP_LOGE(TAG, "failed to allocate display buffer");
        return ESP_FAIL;
    }

    fd = open(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, O_RDONLY);
    if (fd < 0) {
        ESP_LOGE(TAG, "failed to open device");
        return ESP_FAIL;
    }

    if (ioctl(fd, VIDIOC_QUERYCAP, &capability)) {
        ESP_LOGE(TAG, "failed to get capability");
        ret = ESP_FAIL;
        goto exit_0;
    }

    ESP_LOGI(TAG, "version: %d.%d.%d", (uint16_t)(capability.version >> 16),
             (uint8_t)(capability.version >> 8),
             (uint8_t)capability.version);
    ESP_LOGI(TAG, "driver:  %s", capability.driver);
    ESP_LOGI(TAG, "card:    %s", capability.card);
    ESP_LOGI(TAG, "bus:     %s", capability.bus_info);
    ESP_LOGI(TAG, "capabilities:");
    if (capability.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        ESP_LOGI(TAG, "\tVIDEO_CAPTURE");
    }
    if (capability.capabilities & V4L2_CAP_READWRITE) {
        ESP_LOGI(TAG, "\tREADWRITE");
    }
    if (capability.capabilities & V4L2_CAP_ASYNCIO) {
        ESP_LOGI(TAG, "\tASYNCIO");
    }
    if (capability.capabilities & V4L2_CAP_STREAMING) {
        ESP_LOGI(TAG, "\tSTREAMING");
    }
    if (capability.capabilities & V4L2_CAP_META_OUTPUT) {
        ESP_LOGI(TAG, "\tMETA_OUTPUT");
    }
    if (capability.capabilities & V4L2_CAP_DEVICE_CAPS) {
        ESP_LOGI(TAG, "device capabilities:");
        if (capability.device_caps & V4L2_CAP_VIDEO_CAPTURE) {
            ESP_LOGI(TAG, "\tVIDEO_CAPTURE");
        }
        if (capability.device_caps & V4L2_CAP_READWRITE) {
            ESP_LOGI(TAG, "\tREADWRITE");
        }
        if (capability.device_caps & V4L2_CAP_ASYNCIO) {
            ESP_LOGI(TAG, "\tASYNCIO");
        }
        if (capability.device_caps & V4L2_CAP_STREAMING) {
            ESP_LOGI(TAG, "\tSTREAMING");
        }
        if (capability.device_caps & V4L2_CAP_META_OUTPUT) {
            ESP_LOGI(TAG, "\tMETA_OUTPUT");
        }
    }

    memset(&init_format, 0, sizeof(struct v4l2_format));
    init_format.type = type;
    if (ioctl(fd, VIDIOC_G_FMT, &init_format) != 0) {
        ESP_LOGE(TAG, "failed to get format");
        ret = ESP_FAIL;
        goto exit_0;
    }

    struct v4l2_fmtdesc fmtdesc = {
        .index = 2,//fmt_index++,
        .type = type
    };

    if (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != 0) 
    {
        ESP_LOGE(TAG, "failed to enumerate format");
        ret = ESP_FAIL;
        goto exit_0;
    }

    struct v4l2_format format = {
        .type = type,
        .fmt.pix.width = init_format.fmt.pix.width,
        .fmt.pix.height = init_format.fmt.pix.height,
        .fmt.pix.pixelformat = fmtdesc.pixelformat,
    };

    if (ioctl(fd, VIDIOC_S_FMT, &format) != 0) 
    {        
        ESP_LOGE(TAG, "failed to set format");
        ret = ESP_FAIL;
        goto exit_0;
    }

    EventBits_t capture_bit;

    ESP_LOGI(TAG, "Capture %s format frames", (char *)fmtdesc.description);
    ESP_LOGI(TAG, "\twidth:  %" PRIu32, format.fmt.pix.width);
    ESP_LOGI(TAG, "\theight: %" PRIu32, format.fmt.pix.height);
    ESP_LOGI(TAG, "Target size");
    ESP_LOGI(TAG, "\twidth:  %" PRIu32, (uint32_t)_buffer.width);
    ESP_LOGI(TAG, "\theight: %" PRIu32, (uint32_t)_buffer.height);
    
    memset(&req, 0, sizeof(req));
    req.count  = BUFFER_COUNT;
    req.type   = type;
    req.memory = MEMORY_TYPE;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) != 0) {
        ESP_LOGE(TAG, "failed to require buffer");
        ret = ESP_FAIL;
        goto exit_0;
    }

    for (int i = 0; i < BUFFER_COUNT; i++) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type        = type;
        buf.memory      = MEMORY_TYPE;
        buf.index       = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) != 0) {
            ESP_LOGE(TAG, "failed to query buffer");
            ret = ESP_FAIL;
            goto exit_0;
        }

#if CONFIG_EXAMPLE_VIDEO_BUFFER_TYPE_USER
        buffer[i] = heap_caps_aligned_alloc(MEMORY_ALIGN, buf.length, MALLOC_CAP_SPIRAM | MALLOC_CAP_CACHE_ALIGNED | MALLOC_CAP_DMA);
#else
        buffer[i] = (uint8_t *)mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, fd, buf.m.offset);
#endif
        if (!buffer[i]) {
            ESP_LOGE(TAG, "failed to map buffer");
            ret = ESP_FAIL;
            goto exit_0;
        }
#if CONFIG_EXAMPLE_VIDEO_BUFFER_TYPE_USER
        else {
            buf.m.userptr = (unsigned long)buffer[i];
            buffer_size[i] = buf.length;
        }
#endif

        if (ioctl(fd, VIDIOC_QBUF, &buf) != 0) {
            ESP_LOGE(TAG, "failed to queue video frame");
            ret = ESP_FAIL;
            goto exit_0;
        }
    }

    if (ioctl(fd, VIDIOC_STREAMON, &type) != 0) {
        ESP_LOGE(TAG, "failed to start stream");
        ret = ESP_FAIL;
        goto exit_0;
    }

    do
    {
        EventBits_t events = xEventGroupWaitBits(_event_group, FLAG_CAPTURE_FRAME | FLAG_CAPTURE_DONE, pdTRUE, pdFALSE, portMAX_DELAY);    

        if((events & FLAG_CAPTURE_DONE) == 0)
        {
            memset(&buf, 0, sizeof(buf));
            buf.type   = type;
            buf.memory = MEMORY_TYPE;
            if (ioctl(fd, VIDIOC_DQBUF, &buf) != 0) {
                ESP_LOGE(TAG, "failed to receive video frame");
                ret = ESP_FAIL;
                goto exit_0;
            }
    
    #if CONFIG_EXAMPLE_VIDEO_BUFFER_TYPE_USER
            buf.m.userptr = (unsigned long)buffer[buf.index];
            buf.length = buffer_size[buf.index];
    #endif
    
            ppa_srm_oper_config_t srm_config = 
            {
                .in = {
                    .srm_cm = PPA_SRM_COLOR_MODE_RGB888,
                    .buffer = buffer[buf.index],
                    .pic_w = format.fmt.pix.width,
                    .pic_h = format.fmt.pix.height,
                    // .block_w = format.fmt.pix.width,
                    // .block_h = format.fmt.pix.height,
                    // .block_w = width,
                    // .block_h = height,
                    .block_w = _buffer.width,
                    .block_h = _buffer.height,
                    .block_offset_x = 0,
                    .block_offset_y = 0,
                },
                .out = {
                    .srm_cm = _buffer.bytes_per_pixel == 3 ? PPA_SRM_COLOR_MODE_RGB888 : PPA_SRM_COLOR_MODE_RGB565,
                    .buffer = display_buffer,
                    // .pic_w = width,
                    // .pic_h = height,
                    .pic_w = _buffer.width,
                    .pic_h = _buffer.height,
                    .block_offset_x = 0,
                    .block_offset_y = 0,
                    .buffer_size = _buffer.width * _buffer.height * _buffer.bytes_per_pixel
                    // .buffer_size = width * height * 3
                },
                .mode = PPA_TRANS_MODE_BLOCKING,
                // .scale_x = (float)_buffer.width / (float)format.fmt.pix.width,
                // .scale_y = (float)_buffer.height / (float)format.fmt.pix.height,
                .scale_x = 1.0f,
                .scale_y = 1.0f,
                .rotation_angle = PPA_SRM_ROTATION_ANGLE_180,
            };
            ESP_LOGI(TAG, "scale_x: %f, scale_y: %f", srm_config.scale_x, srm_config.scale_y);
            ppa_do_scale_rotate_mirror(_ppa_handle, &srm_config);
            
            memcpy(_buffer.buffer, display_buffer, _buffer.bytes_per_pixel * _buffer.width * _buffer.height);
    
            if(_buffer.f)
            {
                _buffer.f(&_buffer);
            }
            _has_frame_captured = true;
    
            if (ioctl(fd, VIDIOC_QBUF, &buf) != 0) {
                ESP_LOGE(TAG, "failed to queue video frame");
                ret = ESP_FAIL;
                goto exit_0;
            }        

            capture_bit = xEventGroupWaitBits(_event_group, FLAG_CAPTURE_DONE, pdTRUE, pdFALSE, 0);
        }
        else
        {
            ESP_LOGI(TAG, "Stop capture");
            capture_bit = FLAG_CAPTURE_DONE;
        }

    }while(capture_bit == 0);

    if (ioctl(fd, VIDIOC_STREAMOFF, &type) != 0) {
        ESP_LOGE(TAG, "failed to stop stream");
        ret = ESP_FAIL;
        goto exit_0;
    }

#if CONFIG_EXAMPLE_VIDEO_BUFFER_TYPE_USER
    for (int i = 0; i < BUFFER_COUNT; i++) {
        heap_caps_free(buffer[i]);
        buffer[i] = NULL;
    }
#endif

    ret = ESP_OK;

exit_0:
    ESP_LOGI(TAG, "exit_0");
    close(fd);
    return ret;
}

#endif