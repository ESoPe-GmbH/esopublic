/**
 * @file app_webserver.c
 **/

#include "app_webserver.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "module/comm/dbg.h"
#include "esp_http_server.h"
#include <esp_ota_ops.h>
#include <esp_flash_partitions.h>
#include <esp_partition.h>
#include <cJSON.h>
#include "module/flash_info/flash_info.h"
#include "module/version/version.h"
#include "resources/file_resources.h"

#if KERNEL_USES_LVGL
#include "lvgl.h"
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

static esp_err_t _html_get_handler(httpd_req_t *req);

static esp_err_t _screen_get_handler(httpd_req_t *req);

static esp_err_t _info_get_handler(httpd_req_t *req);

static esp_err_t _ota_update_handler(httpd_req_t *req);

static void _reboot(void* pvArgs);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static httpd_handle_t _server = NULL;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void app_webserver_init(void)
{
	esp_netif_create_default_wifi_sta();
	esp_netif_create_default_wifi_ap();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, _wifi_event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, _wifi_event_handler, NULL));

    wifi_config_t config = 
    {
        .sta = {
            .ssid = CONFIG_SLD_DEMO_WIFI_SSID,
            .password = CONFIG_SLD_DEMO_WIFI_PASSWORD
        }
    };
    esp_wifi_set_config(WIFI_IF_STA, &config);

#if CONFIG_SLD_DEMO_WIFI_AP_ENABLE
    wifi_config_t ap_config = {
        .ap = {
            .ssid = CONFIG_SLD_DEMO_WIFI_AP_SSID,
            .password = CONFIG_SLD_DEMO_WIFI_AP_PASSWORD,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK
        }
    };
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    if(config.sta.ssid[0])
    {
        esp_wifi_set_mode(WIFI_MODE_APSTA);
    }
    else
    {
        esp_wifi_set_mode(WIFI_MODE_AP);
    }
#else
    if(config.sta.ssid[0])
    {
        esp_wifi_set_mode(WIFI_MODE_STA);
    }
    else
    {
        esp_wifi_set_mode(WIFI_MODE_NULL);
    }
#endif

	
	ESP_ERROR_CHECK(esp_wifi_start() );

    if(config.sta.ssid[0])
    {
        // Uses the latest wifi configuration
        esp_wifi_connect();
    }

    httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
    http_config.server_port = CONFIG_SLD_DEMO_WEBSERVER_PORT;

    DBG_INFO("Starting server on port: '%d'\n", http_config.server_port);
    if (httpd_start(&_server, &http_config) == ESP_OK) 
    {
        httpd_uri_t info_uri = {
            .uri       = "/info",
            .method    = HTTP_GET,
            .handler   = _info_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(_server, &info_uri);
        
        httpd_uri_t ota_update_uri = {
            .uri       = "/ota",
            .method    = HTTP_POST,
            .handler   = _ota_update_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(_server, &ota_update_uri);

        httpd_uri_t screen_uri = {
            .uri       = "/screen",
            .method    = HTTP_GET,
            .handler   = _screen_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(_server, &screen_uri);

        httpd_uri_t html_uri = {
            .uri       = "/index.html",
            .method    = HTTP_GET,
            .handler   = _html_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(_server, &html_uri);
    }
    else
    {
        DBG_ERROR("Error starting server!\n");
    }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


static void _wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
        {
            DBG_INFO("STA Start\n");
            
        }
        else if (event_id == WIFI_EVENT_STA_STOP)
        {
            DBG_INFO("STA Stop\n");
            
        }
        else if (event_id == WIFI_EVENT_STA_CONNECTED)
        {
            DBG_INFO("STA connected\n");
            
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            DBG_INFO("STA disconnected\n");
            esp_wifi_connect();
        }
        else if (event_id == WIFI_EVENT_STA_AUTHMODE_CHANGE)
        {
            DBG_INFO("STA Authmode change\n");
            
        }
        else if (event_id == WIFI_EVENT_AP_START)
        {			                
            DBG_INFO("AP Start\n");
            
        }
        else if (event_id == WIFI_EVENT_AP_STACONNECTED)
        {
            DBG_INFO("AP connected\n");
        }
        else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
        {
            DBG_INFO("AP disconnected\n");

        }
        else if(event_id == WIFI_EVENT_SCAN_DONE)
        {
            DBG_INFO("Scan done\n");
            
        }
    }
    else if(event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;

            DBG_INFO("Wifi IP=" IPSTR " SN=" IPSTR " GW=" IPSTR "\n", IP2STR(&event->ip_info.ip), IP2STR(&event->ip_info.netmask), IP2STR(&event->ip_info.gw));

        }
    }
}

static esp_err_t _html_get_handler(httpd_req_t *req)
{
    const file_resource_t* fr = file_resource_get_by_name("index.html");

    if(fr)
    {
        httpd_resp_set_type(req, "text/html");    
        httpd_resp_send(req, fr->content, fr->filesize - 1);        
        return ESP_OK;
    }

    httpd_resp_send_404(req);
    return ESP_FAIL;
}

static esp_err_t _screen_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/binary");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=\"screen.bin\"");
    httpd_resp_set_hdr(req, "Content-Transfer-Encoding", "binary");

#if KERNEL_USES_LVGL
    lv_draw_buf_t* buf = lv_display_get_buf_active(NULL);

    httpd_resp_send(req, (char*)buf->data, buf->data_size);

    return ESP_OK;
#else
    httpd_resp_send_404(req);
    return ESP_FAIL;
#endif
}

static esp_err_t _info_get_handler(httpd_req_t *req) 
{
    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "serial", flash_info_get_hardware_id());
    cJSON_AddStringToObject(root, "version", version_get_string());

    char *json_string = cJSON_Print(root);

    cJSON_Delete(root);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_string, strlen(json_string));

    free(json_string);
    
    return ESP_OK;
}

static esp_err_t _ota_update_handler(httpd_req_t *req) 
{
    esp_ota_handle_t ota_handle;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

    DBG_INFO("Prepare OTA update\n");

    esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    int total_len = req->content_len;
    int received_len = 0;
    char buffer[1024];
    int ret;

    int tenth_total_len = total_len / 10;
    int printed_len = tenth_total_len;

    DBG_INFO("Starting OTA update...\n");

    while (received_len < total_len) {
        ret = httpd_req_recv(req, buffer, sizeof(buffer));
        if (ret <= 0) {
            esp_ota_end(ota_handle);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        err = esp_ota_write(ota_handle, buffer, ret);
        if (err != ESP_OK) {
            esp_ota_end(ota_handle);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        received_len += ret;
        if (received_len >= printed_len) {
            DBG_INFO("Received %d of %d bytes\n", received_len, total_len);
            printed_len += tenth_total_len;
        }
    }

    err = esp_ota_end(ota_handle);
    if (err != ESP_OK) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    DBG_INFO("OTA write complete, setting boot partition\n");

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    DBG_INFO("Boot partition set, rebooting...\n");

    httpd_resp_sendstr(req, "OTA Update successful! Rebooting...");

    xTaskCreate(_reboot, "reboot_task", 2048, NULL, 5, NULL);

    return ESP_OK;
}

static void _reboot(void* pvArgs)
{
    vTaskDelay(pdMS_TO_TICKS(2000));
    DBG_INFO("Rebooting...\n");

    // Stop the server before rebooting
    esp_wifi_stop();

    esp_restart();
}
