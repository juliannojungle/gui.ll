# Implementation Plan: Refactor File I/O Layer

## Overview

Refactor the Simulator file I/O to use real FatFS backed by a disk image file, eliminating the POSIX shim. The implementation creates a new Simulator DiskIO.c, updates the build system and setup script, then removes the old shim files. Tasks are ordered so each step builds on the previous and nothing is left orphaned.

## Tasks

- [x] 1. Create Simulator DiskIO and update HALConfig
  - [x] 1.1 Create `src/lib/Platform/Simulator/DiskIO.c`
    - Implement `disk_initialize(BYTE pdrv)`: open `SD_DISK_IMAGE` with `fopen("rb+")`, return `0` on success or `STA_NOINIT` on failure; only accept `pdrv == 0`
    - Implement `disk_status(BYTE pdrv)`: return `0` if `diskFile != NULL`, else `STA_NOINIT`
    - Implement `disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)`: `fseek` to `sector * 512`, `fread` `count * 512` bytes
    - Implement `disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)`: `fseek` to `sector * 512`, `fwrite` `count * 512` bytes
    - Implement `disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)`: handle `CTRL_SYNC` (fflush), `GET_SECTOR_SIZE` (512), `GET_BLOCK_SIZE` (1), `GET_SECTOR_COUNT` (file size / 512)
    - Implement `SDCardInit(void)`: call `disk_initialize(0)` and return success status
    - Include `"ff.h"`, `"diskio.h"`, `"HALConfig.h"`, `<stdio.h>`, `<stdbool.h>`
    - Use `static FILE *diskFile = NULL;` for state
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8_

  - [x] 1.2 Update `src/lib/Platform/Simulator/HALConfig.h`
    - Remove `#define SD_DIRECTORY "sample/sdcard"`
    - Add `#define SD_DISK_IMAGE "sample/sdcard.img"`
    - Leave all other defines (dummy pins, SPI constants) unchanged
    - _Requirements: 6.1, 6.2_

- [x] 2. Update Setup script for disk image creation
  - [x] 2.1 Update `Toolchain/Simulator/Setup.sh`
    - Add `dosfstools` and `mtools` to the idempotent package installation check (alongside `libsdl2-dev` and `gdb`)
    - After the package section and gdbinit block, add disk image creation logic:
      - Compute required size from `sample/sdcard/` contents (minimum 1 MB)
      - `dd if=/dev/zero of=sample/sdcard.img bs=1M count=<size>` to create zeroed file
      - `mkfs.fat sample/sdcard.img` to format as FAT
      - `mcopy -i sample/sdcard.img sample/sdcard/* ::` to copy sample files into the image
    - The disk image is always recreated (not idempotent — intentional so sample file changes are reflected)
    - _Requirements: 3.1, 3.2, 3.3, 3.4_

- [x] 3. Checkpoint - Verify setup script
  - Ensure `Toolchain/Simulator/Setup.sh` runs without errors and produces a valid `sample/sdcard.img`. Ask the user if questions arise.

- [x] 4. Update CMakeLists.txt Simulator block
  - [x] 4.1 Modify the `elseif(PLATFORM_NAME STREQUAL "Simulator")` block in `CMakeLists.txt`
    - Remove `${CMAKE_SOURCE_DIR}/src/lib/Platform/Simulator/FileHelper.c` from `GUILL_SIM_SRCS`
    - Add to `GUILL_SIM_SRCS`:
      - `${CMAKE_SOURCE_DIR}/src/lib/Platform/Simulator/DiskIO.c`
      - `${CMAKE_SOURCE_DIR}/src/lib/Helper/FileHelper.c`
      - `${CMAKE_SOURCE_DIR}/src/Dependency/fatfs/source/ff.c`
      - `${CMAKE_SOURCE_DIR}/src/Dependency/fatfs/source/ffsystem.c`
      - `${CMAKE_SOURCE_DIR}/src/Dependency/fatfs/source/ffunicode.c`
    - Add `include(${CMAKE_SOURCE_DIR}/src/Dependency/fatfs.ffconf_patch.cmake)` before the `add_executable` call
    - Add `include_directories(${CMAKE_SOURCE_DIR}/src/Dependency/fatfs/source)` so `#include "ff.h"` and `#include "diskio.h"` resolve to the real FatFS headers
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 4.4_

- [x] 5. Delete old Simulator shim files
  - [x] 5.1 Delete the following files from the repository
    - `src/lib/Platform/Simulator/FileHelper.c`
    - `src/lib/Platform/Simulator/FileHelper.h`
    - `src/lib/Platform/Simulator/ff.h`
    - _Requirements: 4.1, 4.2, 4.3_

- [x] 6. Build verification checkpoint
  - Run `rm -rf build && mkdir build && cd build && cmake .. -DPLATFORM_NAME=Simulator && make` and confirm the Simulator compiles and links without errors. Ensure all tests pass, ask the user if questions arise.
  - _Requirements: 1.1, 1.2, 1.3, 7.1, 7.2, 7.3_

## Notes

- Per the project's DO NOT WRITE TESTS policy, no automated tests are written. Verification is by clean compile + link and runtime behavior.
- The shared `src/lib/Helper/FileHelper.c` is not modified — it already uses FatFS APIs and forward-declares `SDCardInit`.
- RP2040 and ESP32 builds are unaffected since no changes touch their platform directories or the shared FileHelper source/header.
- The FatFS submodule at `src/Dependency/fatfs/` is not modified; only the build-time `ffconf_patch.cmake` is applied as before.
- Each task references specific requirements for traceability.
- Checkpoints ensure incremental validation.

## Task Dependency Graph

```json
{
  "waves": [
    { "id": 0, "tasks": ["1.1", "1.2", "2.1"] },
    { "id": 1, "tasks": ["4.1"] },
    { "id": 2, "tasks": ["5.1"] }
  ]
}
```
