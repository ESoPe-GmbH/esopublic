if(NOT DEFINED PROJECT_NAME)
    cmake_minimum_required(VERSION 3.14)
    project(esopublic)

    # GoogleTest requires at least C++11
    set(CMAKE_CXX_STANDARD 11)

    include(FetchContent)
    FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 0320f517fd920866d918e564105d68fd4362040a
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(test)
elseif(DEFINED ESP_PLATFORM)
    file(GLOB_RECURSE publicsrcs "source/*.c")
    
    set(KERNEL_REQUIRES esp_lcd nvs_flash driver esp_timer esp_event app_update esp_netif)

    # Auto detect LVGL bases on kproject config variable.
    if(DEFINED CONFIG_LV_COLOR_DEPTH AND NOT DEFINED KERNEL_ADD_DEPENDENCY_LVGL)
        set(KERNEL_ADD_DEPENDENCY_LVGL true)
    endif()

    if(${KERNEL_ADD_DEPENDENCY_SLINT})
        list(APPEND KERNEL_REQUIRES slint__slint espressif__esp_lcd_touch)
    endif()

    idf_component_register(SRCS ${publicsrcs} INCLUDE_DIRS "source" REQUIRES ${KERNEL_REQUIRES})

    # Alternatively to using the sdkconfig, the config can be inside an extra config component
    if(EXISTS ../config)
        message(STATUS "Esopublic uses config from config component")
        include_directories(../config)
    endif()

    if(${KERNEL_ADD_DEPENDENCY_SLINT})
        include_directories(../../managed_components/slint__slint/include)
        include_directories(../../managed_components/espressif__esp_lcd_touch/include)

        target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DKERNEL_USES_SLINT")

        message(STATUS "Kernel uses slint")
    endif()

    if(${KERNEL_ADD_DEPENDENCY_LVGL})
        include_directories(../../managed_components/lvgl__lvgl/src)
        target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DKERNEL_USES_LVGL")
        message(STATUS "Kernel uses lvgl")
    endif()
    
endif()