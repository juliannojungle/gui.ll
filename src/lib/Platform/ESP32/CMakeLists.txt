set(PROJ_ROOT ${CMAKE_SOURCE_DIR})

# Fallback: if CMAKE_SOURCE_DIR doesn't point to project root (ESP-IDF context),
# derive it from this file's location (src/lib/Platform/ESP32/CMakeLists.txt)
if(NOT EXISTS "${PROJ_ROOT}/src/Dependency/fatfs.ffconf_patch.cmake")
    get_filename_component(PROJ_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
endif()

set(PLATFORM_NAME "ESP32")
include(${PROJ_ROOT}/gui.ll.cmake)

idf_component_register(
    SRCS
        "${PROJ_ROOT}/src/Sample.c"
        ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES driver esp_system)

set(PNG_SRC ${PROJ_ROOT}/src/Dependency/libpng)

# Generate pnglibconf.h required by libpng
configure_file(
    ${PNG_SRC}/scripts/pnglibconf.h.prebuilt
    ${PNG_SRC}/pnglibconf.h
    COPYONLY)

# Disable -Werror for third-party dependencies (libpng, zlib) that produce
# warnings with the xtensa toolchain under ESP-IDF's strict -Werror=all.
# Using target_compile_options ensures -Wno-error comes after -Werror=all.
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=maybe-uninitialized
    -Wno-error=unused-but-set-variable)

# Suppress -Wmaybe-uninitialized specifically for libpng files that produce
# false positives when GCC analyzes data flow through macros (PNG_FORMAT_NUMBER).
# The code is correct — this is a static analysis limitation of the compiler.
set_source_files_properties(
    "${PNG_SRC}/pngerror.c"
    "${PNG_SRC}/png.c"
    PROPERTIES COMPILE_OPTIONS "-Wno-maybe-uninitialized")
