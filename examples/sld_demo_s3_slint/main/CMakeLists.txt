file(GLOB_RECURSE srcs *.c *.cpp)

set(MAIN_REQUIRES esopublic esp_lcd mbedtls esp_wifi esp_http_server app_update json)

if(DEFINED CONFIG_LV_COLOR_DEPTH AND NOT DEFINED KERNEL_ADD_DEPENDENCY_LVGL AND NOT DEFINED KERNEL_ADD_DEPENDENCY_SLINT)
    set(KERNEL_ADD_DEPENDENCY_LVGL true)
endif()

if(${IDF_TARGET} STREQUAL "esp32p4")
    message("ESP32P4 detected")
    list(APPEND MAIN_REQUIRES esp_driver_ppa)
endif()

if(${KERNEL_ADD_DEPENDENCY_LVGL})
    list(APPEND MAIN_REQUIRES lvgl__lvgl)
endif()

if(${KERNEL_ADD_DEPENDENCY_SLINT})
    list(APPEND MAIN_REQUIRES slint espressif__esp_lcd_touch)
endif()

idf_component_register(SRCS ${srcs} INCLUDE_DIRS "." REQUIRES ${MAIN_REQUIRES})

if(${KERNEL_ADD_DEPENDENCY_SLINT})
    slint_target_sources(${COMPONENT_LIB} ui/appwindow.slint)
endif()
