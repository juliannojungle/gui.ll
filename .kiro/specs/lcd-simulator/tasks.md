# Implementation Plan: LCD Simulator

## Overview

Add a third platform target (`Simulator`) to gui.ll that produces a native desktop executable rendering the 240×240 RGB565 display in an SDL2 window. The implementation swaps in platform-specific stubs (HAL no-ops, SDL2-backed LCD, POSIX file I/O) selected at build time via `cmake -DPLATFORM_NAME=Simulator`, reusing `Sample.c` and all shared code unchanged.

## Tasks

- [x] 1. Create Platform/Simulator HAL layer
  - [x] 1.1 Create `src/lib/Platform/Simulator/HALConfig.h`
    - Define dummy LCD pin macros (`LCD_DC_PIN` through `LCD_BL_PIN`) with distinct non-negative integers
    - Define dummy SD pin macros (`SD_SPI_CS`, `SD_SPI_SCLK`, `SD_SPI_MOSI`, `SD_SPI_MISO`, `SD_SPI_BAUDRATE`, `SD_DETECT_PIN`)
    - Define `SD_DIRECTORY "sample/sdcard"`
    - Include `<stdbool.h>` for consistency with other HALConfig.h files
    - _Requirements: 4.3, 6.1_

  - [x] 1.2 Create `src/lib/Platform/Simulator/HAL.h`
    - Include `"Types.h"` and `"HALConfig.h"`
    - Define Pico-SDK-compatible constants: `GPIO_FUNC_SPI`, `GPIO_IN`, `GPIO_OUT`, `GPIO_FUNC_PWM`, `PWM_CHAN_B`
    - Declare all HAL function prototypes matching RP2040 signatures exactly: `DigitalWrite`, `DigitalRead`, `SPIWriteByte`, `SPIWriteNByte`, `GPIOInit`, `GPIOSetDir`, `GPIOPullUp`, `Delay`, `STDIOInitAll`, `SPIInit`, `GPIOSetFunction`, `PWMGPIOToSliceNum`, `PWMSetWrap`, `PWMSetChannelLevel`, `PWMSetClockDivider`, `PWMSetEnabled`
    - _Requirements: 4.1, 4.2_

  - [x] 1.3 Create `src/lib/Platform/Simulator/HAL.c`
    - Include `"HAL.h"` and SDL2 headers
    - Implement `Delay(ms)` with SDL event-pump loop: use `SDL_GetTicks` for elapsed time, `SDL_PollEvent` per iteration, `SDL_Delay(1)` to avoid busy-wait, `exit(0)` on `SDL_QUIT`
    - Implement all other HAL functions as empty bodies; non-void ones (`DigitalRead`, `PWMGPIOToSliceNum`) return 0
    - _Requirements: 4.1, 5.1, 5.2, 5.3_

- [x] 2. Create Platform/Simulator RTC module
  - [x] 2.1 Create `src/lib/Platform/Simulator/RTC.h`
    - Declare `RTCInitialize(void)` prototype
    - _Requirements: 8.1_

  - [x] 2.2 Create `src/lib/Platform/Simulator/RTC.c`
    - Include `"RTC.h"`, `<time.h>`
    - Define `DWORD` as `unsigned int`
    - Implement `RTCInitialize()` as no-op
    - Implement `get_fattime()` returning host time packed as FAT timestamp using `time()` + `localtime()`
    - _Requirements: 8.1, 8.2, 8.3_

- [x] 3. Create Platform/Simulator FileHelper module
  - [x] 3.1 Create `src/lib/Platform/Simulator/FileHelper.h`
    - Define `FIL` struct wrapping `FILE*` (member `filePointer`)
    - Define FatFS compatibility types: `UINT`, `DWORD`, `BYTE`, `FRESULT` enum (with `FR_OK` and `FR_INVALID_OBJECT`)
    - Declare `f_read(FIL*, void*, UINT, UINT*)` prototype
    - Declare public API: `MountSdCard`, `SelectActiveDrive`, `OpenFile`, `CloseFile`, `UnMountSdCard`
    - _Requirements: 6.6, 7.1, 7.5_

  - [x] 3.2 Create `src/lib/Platform/Simulator/FileHelper.c`
    - Include `"FileHelper.h"`, `"HALConfig.h"`, `<stdio.h>`, `<dirent.h>`, `<string.h>`
    - Implement `f_read` delegating to `fread`; handle NULL `FILE*` by setting `*br = 0` and returning `FR_INVALID_OBJECT`
    - Implement `MountSdCard` checking `SD_DIRECTORY` exists via `opendir`/`closedir`
    - Implement `SelectActiveDrive` returning `true`
    - Implement `OpenFile` using `fopen` in `"rb"` mode, constructing path as `SD_DIRECTORY/filename`
    - Implement `CloseFile` calling `fclose`
    - Implement `UnMountSdCard` as no-op
    - _Requirements: 6.2, 6.3, 6.4, 6.5, 6.7, 6.8, 6.9, 7.1, 7.2, 7.3, 7.4_

- [x] 4. Create LCD/Simulator display layer
  - [x] 4.1 Create `src/lib/LCD/Simulator/LCDSetup.h`
    - Same interface as `LCD/1in28/LCDSetup.h`: define `LCD_HEIGHT`, `LCD_WIDTH`, `HORIZONTAL`, `VERTICAL`, `LCD_ATTRIBUTES` struct, `extern LCD_ATTRIBUTES LCD`, `LCDInitialize()` prototype
    - _Requirements: 1.3_

  - [x] 4.2 Create `src/lib/LCD/Simulator/LCDSetup.c`
    - Include `"LCDSetup.h"` and SDL2 headers
    - Define static globals: `sdlWindow`, `sdlRenderer`, `sdlTexture`
    - Implement `LCDInitialize()`: call `SDL_Init(SDL_INIT_VIDEO)`, create centered 240×240 window titled "LCD Simulator", create accelerated renderer, create RGB565 streaming texture, set `LCD.WIDTH=240`, `LCD.HEIGHT=240`, `LCD.SCAN_DIR=HORIZONTAL`
    - On any SDL failure: print to stderr, destroy already-allocated resources, return non-zero
    - On success return `EXIT_SUCCESS`
    - _Requirements: 1.1, 1.2, 1.3, 1.4_

  - [x] 4.3 Create `src/lib/LCD/Simulator/LCDRenderer.h`
    - Same interface as `LCD/1in28/LCDRenderer.h`: declare `LCDSetDisplayArea`, `LCDClear`, `LCDRenderTexture`, `LCDRenderTextureInArea`, `LCDRenderPoint`, `LCDRenderPng`
    - Include the simulator's `FileHelper.h` (for `FIL` type) instead of `"ff.h"`
    - _Requirements: 2.1, 2.2_

  - [x] 4.4 Create `src/lib/LCD/Simulator/LCDRenderer.c`
    - Include `"LCDRenderer.h"`, `"LCDSetup.h"`, SDL2 headers, `<png.h>`
    - Access SDL statics from LCDSetup.c via `extern` declarations
    - Implement `LCDRenderTexture`: byte-swap all 240×240 pixels (big-endian → native), `SDL_UpdateTexture` with pitch 480, `SDL_RenderCopy`, `SDL_RenderPresent`
    - Implement `LCDRenderTextureInArea`: byte-swap sub-region rows, `SDL_UpdateTexture` with rect, present
    - Implement `LCDRenderPoint`: byte-swap single pixel, update 1×1 rect, present
    - Implement `LCDClear`: fill buffer with `fillColor` (no byte-swap per design), update full texture, present
    - Implement `LCDSetDisplayArea` as no-op
    - Implement `LCDRenderPng` as minimal stub (or full libpng pipeline writing to SDL texture)
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 3.1, 3.2_

- [x] 5. Checkpoint
  - Ensure all source files compile individually (no syntax errors). Ask the user if questions arise.

- [x] 6. Add CMake Simulator branch
  - [x] 6.1 Add `elseif(PLATFORM_NAME STREQUAL "Simulator")` block to root `CMakeLists.txt`
    - Set `CMAKE_C_STANDARD 11`
    - Define simulator-specific source list: `Platform/Simulator/HAL.c`, `Platform/Simulator/RTC.c`, `Platform/Simulator/FileHelper.c`, `LCD/Simulator/LCDSetup.c`, `LCD/Simulator/LCDRenderer.c`
    - Include shared sources: `Sample.c`, `Canvas.c`, `Trigonometry.c`, font files
    - Do NOT include `Driver/GC9A01/Driver.c`, fatfs, DiskIO, `LCD/1in28/` sources, or `pico_sdk_import.cmake`
    - Set include directories: `Platform/Simulator/`, `LCD/Simulator/`, `GUI/`, `Helper/`, `src/lib/`
    - Find and link SDL2 via `find_package(SDL2 REQUIRED)`
    - Build zlib and libpng from submodules using the existing `zlibstatic.cmake` approach
    - Link `SDL2`, `png_static`, `zlibstatic`, `m`
    - Produce native executable named `gui.ll`
    - _Requirements: 9.1, 9.2, 9.3, 9.4, 9.5, 9.6, 9.7, 9.8, 9.9, 10.1_

- [x] 7. Add toolchain setup and VS Code integration
  - [x] 7.1 Create `Toolchain/Simulator/Setup.sh`
    - Check if `libsdl2-dev` is installed; if not, install via `sudo apt-get install -y libsdl2-dev`
    - Make script idempotent (skip if already installed)
    - _Requirements: 13.1_

  - [x] 7.2 Add tasks.json entries for Simulator
    - Add "Build: Simulator Full" task: `rm -rf build && mkdir build && cd build && cmake .. -DPLATFORM_NAME=Simulator && make`
    - Add "Setup: Simulator toolchain in WSL2" task calling `Toolchain/Simulator/Setup.sh`
    - Follow the same structure/conventions as existing RP2040 and ESP32 tasks
    - _Requirements: 13.2_

  - [x] 7.3 Create `.vscode/launch.json` with "Debug Simulator" configuration
    - Type `"cppdbg"`, MIMode `"gdb"`, program `"${workspaceFolder}/build/gui.ll"`, cwd `"${workspaceFolder}"`
    - Set `"preLaunchTask"` to the simulator build task
    - If file already exists, append to the `"configurations"` array
    - _Requirements: 12.1, 12.2, 12.3_

- [x] 8. Build verification
  - [x] 8.1 Verify the full Simulator build
    - Run `cmake -DPLATFORM_NAME=Simulator .. && make` in the build directory
    - Confirm the native executable is produced without errors or warnings
    - Confirm existing RP2040 cmake path is not affected (no shared source files modified)
    - _Requirements: 9.7, 10.1, 10.2_

## Notes

- No test tasks are included per the project's strict NO TESTS policy (see AGENTS.md Testing Policy)
- `sample/sdcard/01.png` already exists — no task needed to create it
- The simulator FileHelper.h replaces `"ff.h"` as the header providing `FIL` and `FRESULT` types for the simulator build; include path resolution ensures the correct header is picked up
- All naming follows project conventions: PascalCase files/functions, camelCase variables, UPPER_SNAKE_CASE constants
- The `PWMSetClockDivider` stub accepts `float divider` to match the RP2040 HAL signature exactly

## Task Dependency Graph

```json
{
  "waves": [
    { "id": 0, "tasks": ["1.1"] },
    { "id": 1, "tasks": ["1.2", "2.1", "3.1", "4.1"] },
    { "id": 2, "tasks": ["1.3", "2.2", "3.2", "4.2", "4.3"] },
    { "id": 3, "tasks": ["4.4"] },
    { "id": 4, "tasks": ["6.1"] },
    { "id": 5, "tasks": ["7.1", "7.2", "7.3"] },
    { "id": 6, "tasks": ["8.1"] }
  ]
}
```
