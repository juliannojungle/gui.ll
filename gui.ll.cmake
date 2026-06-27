
# Platform-agnostic translation units, shared by both targets. Platform-specific
# sources (HAL.c, RTC.c, DiskIO.c) are added per-platform below.
set(GUILL_COMMON_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/Driver/GC9A01/Driver.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/LCD/1in28/LCDSetup.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/LCD/1in28/LCDRenderer.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/GUI/Canvas.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/GUI/Fonts/font8.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/GUI/Fonts/font12.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/GUI/Fonts/font16.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/GUI/Fonts/font20.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/GUI/Fonts/font24.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/Helper/FileHelper.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/Helper/Trigonometry.c
)

if(PLATFORM_NAME STREQUAL "ESP32")
    # ESP-IDF orchestrates the build.
    # EXTRA_COMPONENT_DIRS adds our component without losing ESP-IDF built-in components.
    set(EXTRA_COMPONENT_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/ESP32"
    )
elseif(PLATFORM_NAME STREQUAL "RP2040")
    set(GUILL_RP2040_SRCS
        ${GUILL_COMMON_SRCS}
        ${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/RP2040/HAL.c
        ${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/RP2040/RTC.c
    )
endif()
