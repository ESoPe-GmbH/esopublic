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
    idf_component_register(SRCS ${publicsrcs} INCLUDE_DIRS "source" REQUIRES config nvs_flash driver esp_timer esp_event)
endif()