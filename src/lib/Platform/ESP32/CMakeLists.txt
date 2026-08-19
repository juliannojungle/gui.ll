set(PROJ_ROOT ${CMAKE_SOURCE_DIR})

# Fallback: if CMAKE_SOURCE_DIR doesn't point to project root (ESP-IDF context),
# derive it from this file's location (src/lib/Platform/ESP32/CMakeLists.txt)
if(NOT EXISTS "${PROJ_ROOT}/src/lib/Platform/ESP32/CMakeLists.txt")
    get_filename_component(PROJ_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
endif()

set(PLATFORM_NAME "ESP32")

# In-tree build: this repository is the gui.ll root, so no lookup/download is needed.
# Set explicitly, since ESP-IDF may process this component in a scope that did not
# inherit GUI_LL_PATH from the root CMakeLists.txt.
if(NOT GUI_LL_PATH)
    set(GUI_LL_PATH "${PROJ_ROOT}")
endif()

include(${PROJ_ROOT}/gui.ll.cmake)

idf_component_register(
    SRCS
        "${PROJ_ROOT}/src/Sample.c"
        ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES driver esp_system)

# Disable -Werror for third-party dependencies (libpng, zlib) that produce
# warnings with the xtensa toolchain under ESP-IDF's strict -Werror=all.
# Using target_compile_options ensures -Wno-error comes after -Werror=all.
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=maybe-uninitialized
    -Wno-error=unused-but-set-variable)
