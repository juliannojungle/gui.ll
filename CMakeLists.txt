# version requirements
cmake_minimum_required(VERSION 3.16)

# Generate compile_commands.json (consumed by clangd for code intelligence)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform-agnostic translation units, shared by both targets. Platform-specific
# sources (HAL.c, RTC.c, DiskIO.c) are added per-platform below.
set(GUILL_COMMON_SRCS
    ${CMAKE_SOURCE_DIR}/src/Sample.c
    ${CMAKE_SOURCE_DIR}/src/lib/Driver/GC9A01/Driver.c
    ${CMAKE_SOURCE_DIR}/src/lib/LCD/1in28/LCDSetup.c
    ${CMAKE_SOURCE_DIR}/src/lib/LCD/1in28/LCDRenderer.c
    ${CMAKE_SOURCE_DIR}/src/lib/GUI/Canvas.c
    ${CMAKE_SOURCE_DIR}/src/lib/GUI/Fonts/font8.c
    ${CMAKE_SOURCE_DIR}/src/lib/GUI/Fonts/font12.c
    ${CMAKE_SOURCE_DIR}/src/lib/GUI/Fonts/font16.c
    ${CMAKE_SOURCE_DIR}/src/lib/GUI/Fonts/font20.c
    ${CMAKE_SOURCE_DIR}/src/lib/GUI/Fonts/font24.c
    ${CMAKE_SOURCE_DIR}/src/lib/Helper/FileHelper.c
    ${CMAKE_SOURCE_DIR}/src/lib/Helper/Trigonometry.c
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

elseif(PLATFORM_NAME STREQUAL "Simulator")
    set(CMAKE_C_STANDARD 11)
    project(gui.ll)

    set(GUILL_SIM_SRCS
        ${CMAKE_SOURCE_DIR}/src/Sample.c
        ${CMAKE_SOURCE_DIR}/src/lib/Platform/Simulator/HAL.c
        ${CMAKE_SOURCE_DIR}/src/lib/Platform/Simulator/RTC.c
        ${CMAKE_SOURCE_DIR}/src/lib/Platform/Simulator/FileHelper.c
        ${CMAKE_SOURCE_DIR}/src/lib/LCD/Simulator/LCDSetup.c
        ${CMAKE_SOURCE_DIR}/src/lib/LCD/Simulator/LCDRenderer.c
        ${CMAKE_SOURCE_DIR}/src/lib/GUI/Canvas.c
        ${CMAKE_SOURCE_DIR}/src/lib/GUI/Fonts/font8.c
        ${CMAKE_SOURCE_DIR}/src/lib/GUI/Fonts/font12.c
        ${CMAKE_SOURCE_DIR}/src/lib/GUI/Fonts/font16.c
        ${CMAKE_SOURCE_DIR}/src/lib/GUI/Fonts/font20.c
        ${CMAKE_SOURCE_DIR}/src/lib/GUI/Fonts/font24.c
        ${CMAKE_SOURCE_DIR}/src/lib/Helper/Trigonometry.c
    )

    add_executable(${PROJECT_NAME} ${GUILL_SIM_SRCS})

    set(GUILL_LIB ${CMAKE_SOURCE_DIR}/src/lib)
    include_directories(${GUILL_LIB})
    include_directories(${GUILL_LIB}/Platform/Simulator)
    include_directories(${GUILL_LIB}/LCD/Simulator)
    include_directories(${GUILL_LIB}/GUI)
    include_directories(${GUILL_LIB}/GUI/Fonts)
    include_directories(${GUILL_LIB}/Helper)

    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})

    set(LIB_ZLIB ${CMAKE_SOURCE_DIR}/src/Dependency/zlib)
    file(REMOVE ${LIB_ZLIB}/CMakeLists.txt)
    file(COPY ${CMAKE_SOURCE_DIR}/src/Dependency/zlibstatic.cmake DESTINATION ${LIB_ZLIB})
    file(RENAME ${LIB_ZLIB}/zlibstatic.cmake ${LIB_ZLIB}/CMakeLists.txt)
    add_subdirectory(${LIB_ZLIB} zlib_build)
    set(LIB_ZCONF ${CMAKE_BINARY_DIR}/zlib_build)
    include_directories(${LIB_ZLIB} ${LIB_ZCONF})
    set(PNG_BUILD_ZLIB ON CACHE BOOL "" FORCE)
    set(SKIP_INSTALL_ALL ON)
    set(ZLIB_LIBRARIES zlibstatic)
    set(ZLIB_INCLUDE_DIRS ${LIB_ZLIB} ${LIB_ZCONF})

    set(PNG_LIB ${CMAKE_SOURCE_DIR}/src/Dependency/libpng)
    set(PNG_SHARED OFF CACHE BOOL "")
    set(PNG_STATIC ON CACHE BOOL "")
    set(PNG_FRAMEWORK OFF CACHE BOOL "")
    set(PNG_EXECUTABLES OFF CACHE BOOL "")
    set(PNG_TESTS OFF CACHE BOOL "")
    set(PNG_DEBUG OFF CACHE BOOL "")
    include_directories(${PNG_LIB})
    add_subdirectory(${PNG_LIB})
    configure_file(${PNG_LIB}/scripts/pnglibconf.h.prebuilt ${PNG_LIB}/pnglibconf.h)

    target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES} png_static zlibstatic m)

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
