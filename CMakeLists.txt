# version requirements
cmake_minimum_required(VERSION 3.16)

# Generate compile_commands.json (consumed by clangd for code intelligence)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform-agnostic translation units, shared by both targets. Platform-specific
# sources (HAL.c, RTC.c, DiskIO.c) are added per-platform below.
set(GUILL_COMMON_SRCS
    ${CMAKE_SOURCE_DIR}/src/Sample.c
    ${CMAKE_SOURCE_DIR}/src/lib/Driver/GC9A01/Driver.c
    ${CMAKE_SOURCE_DIR}/src/lib/LCD/LCD_1in28.c
    ${CMAKE_SOURCE_DIR}/src/lib/GUI/Canvas.c
    ${CMAKE_SOURCE_DIR}/src/lib/Fonts/font20.c
    ${CMAKE_SOURCE_DIR}/src/lib/Helper/FileHelper.c
    ${CMAKE_SOURCE_DIR}/src/lib/Helper/PNGHelper.c
)

# Platform selection
if(NOT DEFINED PLATFORM_NAME)
    set(PLATFORM_NAME "RP2040")
endif()

# Optional debug messages: pass -DDEBUGMSGS (no value needed) at configure time to
# define the DEBUGMSGS macro for the build (used via #ifdef DEBUGMSGS in the sources).
# Guarded with DEFINED so it triggers whether CMake stores it as "ON" or as an empty value.
if(DEFINED DEBUGMSGS)
    add_compile_definitions(DEBUGMSGS)
endif()

# Build system per platform
if(PLATFORM_NAME STREQUAL "ESP32")
    # ESP-IDF orchestrates the build.
    # EXTRA_COMPONENT_DIRS adds our component without losing ESP-IDF built-in components.
    set(EXTRA_COMPONENT_DIRS
        "${CMAKE_SOURCE_DIR}/src/lib/Platform/ESP32"
    )
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(gui.ll)

else() # RP2040 (default)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 14)

    if(NOT DEFINED PICO_SDK_PATH)
        set(PICO_SDK_PATH ~/pico-sdk)
    endif()
    include(${CMAKE_SOURCE_DIR}/src/Dependency/pico_sdk_import.cmake)

    project(gui.ll)
    pico_sdk_init()

    # Include platform-specific cmake fragments
    include(${CMAKE_SOURCE_DIR}/src/lib/Platform/RP2040/PreExecutable.cmake)

    add_executable(${PROJECT_NAME}
        ${GUILL_COMMON_SRCS}
        src/lib/Platform/RP2040/HAL.c
        src/lib/Platform/RP2040/RTC.c
    )

    include(${CMAKE_SOURCE_DIR}/src/lib/Platform/RP2040/PostExecutable.cmake)

    pico_enable_stdio_usb(${PROJECT_NAME} 1)  # enable usb output
    pico_enable_stdio_uart(${PROJECT_NAME} 0) # disable uart output
    pico_add_extra_outputs(${PROJECT_NAME})    # create map/bin/hex/uf2 files in addition to elf
endif()
