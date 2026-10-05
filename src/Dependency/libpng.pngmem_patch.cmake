# libpng.pngmem_patch.cmake
# Redirects libpng's zlib allocator (png_zalloc/png_zfree in png.c) to the
# runtime heap through hal.ll's HeapAlloc/HeapFree, at configure time.
#
# Why: decoding a PNG makes zlib allocate its inflate window (up to 32 KB) through
# png_zalloc, which lands on the C library heap. A consumer that also keeps a large
# framebuffer or texture on the libc heap can run it out of contiguous space on a
# small-RAM device. Serving the window from the FreeRTOS heap instead keeps the two
# pools apart. The window is freed at the end of each decode, so it is a transient
# allocation, not a reservation.
#
# Opt-in: gui.ll.cmake includes this only when GUI_LL_PNG_HEAP_VIA_HAL is set, so
# the default build keeps libpng on the libc heap. Applied in place, never
# versioned (the libpng checkout is git-ignored), the same mechanism fs.ll uses
# for ffconf.h.

if(NOT DEFINED PNGMEM_PNG_C_PATH)
    set(PNGMEM_PNG_C_PATH ${CMAKE_CURRENT_LIST_DIR}/libpng/png.c)
endif()

if(NOT EXISTS ${PNGMEM_PNG_C_PATH})
    message(FATAL_ERROR "libpng.pngmem_patch: png.c not found at ${PNGMEM_PNG_C_PATH}")
endif()

file(READ ${PNGMEM_PNG_C_PATH} PNGMEM_CONTENT)

if(PNGMEM_CONTENT MATCHES "HAL_LL_PNGMEM_PATCH")
    message(STATUS "libpng.pngmem_patch: already applied")
    return()
endif()

# Forward-declare the hal.ll heap API right after the first libpng include.
# A bracket argument keeps the semicolons literal (they are list separators
# otherwise) and avoids pulling the whole HAL.h into this third-party unit.
string(REPLACE
    [=[#include "pngpriv.h"]=]
    [=[#include "pngpriv.h"

/* HAL_LL_PNGMEM_PATCH: zlib allocations served from the runtime heap. */
extern void *HeapAlloc(unsigned int size);
extern void HeapFree(void *pointer);]=]
    PNGMEM_CONTENT "${PNGMEM_CONTENT}")

# png_zalloc: return the window from the runtime heap instead of the libc heap.
string(REGEX REPLACE
    "return png_malloc_warn\\(png_voidcast\\(png_structrp, png_ptr\\), num_bytes\\);"
    [=[return HeapAlloc((unsigned int)num_bytes);]=]
    PNGMEM_CONTENT "${PNGMEM_CONTENT}")

# png_zfree: free it through the matching deallocator.
string(REGEX REPLACE
    "png_free\\(png_voidcast\\(png_const_structrp,png_ptr\\), ptr\\);"
    [=[(void)png_ptr; HeapFree(ptr);]=]
    PNGMEM_CONTENT "${PNGMEM_CONTENT}")

file(WRITE ${PNGMEM_PNG_C_PATH} "${PNGMEM_CONTENT}")
message(STATUS "libpng.pngmem_patch: png_zalloc/png_zfree redirected to hal.ll heap")
