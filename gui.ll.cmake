if(NOT DEFINED PLATFORM_NAME)
    set(PLATFORM_NAME "Simulator")
endif()

set(GUILL_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/src/lib)
set(DEPENDENCY_DIR ${CMAKE_CURRENT_LIST_DIR}/src/Dependency)

if(PLATFORM_NAME STREQUAL "Simulator")
    set(LCD_TYPE "Simulator")
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
else()
    set(LCD_TYPE "1in28")
    set(SOURCES ${SOURCES} "${GUILL_LIB_DIR}/Driver/GC9A01/Driver.c")
    set(INCLUDE_DIRS ${INCLUDE_DIRS} "${GUILL_LIB_DIR}/Driver/GC9A01")
endif()

# gui.ll
set(SOURCES
    ${SOURCES}
    "${GUILL_LIB_DIR}/LCD/${LCD_TYPE}/LCDSetup.c"
    "${GUILL_LIB_DIR}/LCD/${LCD_TYPE}/LCDRenderer.c"
    "${GUILL_LIB_DIR}/GUI/Canvas.c"
    "${GUILL_LIB_DIR}/GUI/Fonts/font8.c"
    "${GUILL_LIB_DIR}/GUI/Fonts/font12.c"
    "${GUILL_LIB_DIR}/GUI/Fonts/font16.c"
    "${GUILL_LIB_DIR}/GUI/Fonts/font20.c"
    "${GUILL_LIB_DIR}/GUI/Fonts/font24.c"
    "${GUILL_LIB_DIR}/Helper/Trigonometry.c"
    "${GUILL_LIB_DIR}/Platform/${PLATFORM_NAME}/HAL.c")

# zlib
set(SOURCES
    ${SOURCES}
    "${DEPENDENCY_DIR}/zlib/adler32.c"
    "${DEPENDENCY_DIR}/zlib/compress.c"
    "${DEPENDENCY_DIR}/zlib/crc32.c"
    "${DEPENDENCY_DIR}/zlib/deflate.c"
    "${DEPENDENCY_DIR}/zlib/inflate.c"
    "${DEPENDENCY_DIR}/zlib/inftrees.c"
    "${DEPENDENCY_DIR}/zlib/inffast.c"
    "${DEPENDENCY_DIR}/zlib/trees.c"
    "${DEPENDENCY_DIR}/zlib/uncompr.c"
    "${DEPENDENCY_DIR}/zlib/zutil.c")

# libpng
set(SOURCES
    ${SOURCES}
    "${DEPENDENCY_DIR}/libpng/png.c"
    "${DEPENDENCY_DIR}/libpng/pngerror.c"
    "${DEPENDENCY_DIR}/libpng/pngget.c"
    "${DEPENDENCY_DIR}/libpng/pngmem.c"
    "${DEPENDENCY_DIR}/libpng/pngpread.c"
    "${DEPENDENCY_DIR}/libpng/pngread.c"
    "${DEPENDENCY_DIR}/libpng/pngrio.c"
    "${DEPENDENCY_DIR}/libpng/pngrtran.c"
    "${DEPENDENCY_DIR}/libpng/pngrutil.c"
    "${DEPENDENCY_DIR}/libpng/pngset.c"
    "${DEPENDENCY_DIR}/libpng/pngtrans.c"
    "${DEPENDENCY_DIR}/libpng/pngwio.c"
    "${DEPENDENCY_DIR}/libpng/pngwrite.c"
    "${DEPENDENCY_DIR}/libpng/pngwtran.c"
    "${DEPENDENCY_DIR}/libpng/pngwutil.c")

if(NOT CMAKE_SCRIPT_MODE_FILE)
    configure_file(
        "${DEPENDENCY_DIR}/libpng/scripts/pnglibconf.h.prebuilt" # libpng default configs
        "${DEPENDENCY_DIR}/libpng/pnglibconf.h"
        COPYONLY)
    set_source_files_properties(
        "${DEPENDENCY_DIR}/libpng/pngerror.c"
        "${DEPENDENCY_DIR}/libpng/png.c"
        PROPERTIES COMPILE_OPTIONS "-Wno-maybe-uninitialized") # suppress warnings for libpng
endif()

set(INCLUDE_DIRS
    ${INCLUDE_DIRS}
    "${GUILL_LIB_DIR}"
    "${GUILL_LIB_DIR}/GUI"
    "${GUILL_LIB_DIR}/GUI/Fonts"
    "${GUILL_LIB_DIR}/Helper"
    "${GUILL_LIB_DIR}/LCD/${LCD_TYPE}"
    "${GUILL_LIB_DIR}/Platform/${PLATFORM_NAME}"
    "${DEPENDENCY_DIR}/libpng"
    "${DEPENDENCY_DIR}/zlib"
)

include(${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fs.ll/fs.ll.cmake)
list(FILTER SOURCES EXCLUDE REGEX "fs\\.ll/src/lib/Platform/.*/HAL\\.c$") # removes HAL.c from fs.ll to use HAL.c from gui.ll
