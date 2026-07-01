# Requirements Document

## Introduction

A third platform target ("Simulator") for the gui.ll project that produces a native desktop executable instead of embedded firmware. The simulator displays the LCD output in an SDL2 window (240×240, RGB565), enabling rapid screen-design iteration without flashing to hardware. The same `app_entry()` code runs unchanged; only the platform layer (HAL, LCD, file I/O) is swapped at build time via CMake.

## Glossary

- **Simulator**: The desktop platform target that replaces hardware SPI/GPIO with SDL2 rendering and POSIX file I/O
- **Build_System**: The CMake infrastructure that selects platform sources and links dependencies based on `PLATFORM_NAME`
- **LCD_Window**: The SDL2 window (240×240 pixels) that visually represents the GC9A01 round LCD panel
- **HAL_Simulator**: The Hardware Abstraction Layer implementation for the Simulator platform (`Platform/Simulator/HAL.c`)
- **LCD_Simulator**: The LCD rendering layer for the Simulator platform (`LCD/Simulator/LCDSetup.c` + `LCDRenderer.c`)
- **File_Simulator**: The file I/O implementation that replaces FatFS with standard POSIX `fopen`/`fclose` operations against a local directory
- **SD_DIRECTORY**: A compile-time path defining the local filesystem directory that simulates the SD card contents
- **Event_Loop**: The SDL2 event-pump mechanism integrated into `Delay()` to keep the window responsive

## Requirements

### Requirement 1: SDL2 LCD Window Initialization

**User Story:** As a developer, I want `LCDInitialize()` in the simulator to create an SDL2 window that represents the 240×240 LCD panel, so that I can see rendered output on my desktop without hardware.

#### Acceptance Criteria

1. WHEN `LCDInitialize()` is called, THE LCD_Simulator SHALL call `SDL_Init(SDL_INIT_VIDEO)`, create a 240×240 pixel window titled "LCD Simulator", create an SDL renderer for that window, and return `EXIT_SUCCESS` (0)
2. WHEN `LCDInitialize()` is called and the renderer is available, THE LCD_Simulator SHALL create an SDL texture with format `SDL_PIXELFORMAT_RGB565`, access type `SDL_TEXTUREACCESS_STREAMING`, and dimensions 240×240
3. WHEN `LCDInitialize()` is called, THE LCD_Simulator SHALL initialize the `LCD` global with WIDTH=240, HEIGHT=240, and SCAN_DIR=HORIZONTAL
4. IF `SDL_Init`, window creation, renderer creation, or texture creation fails, THEN THE LCD_Simulator SHALL print an error message to stderr indicating which operation failed, destroy any SDL resources already allocated during the call, and return a non-zero exit code

### Requirement 2: Texture Rendering to SDL2 Window

**User Story:** As a developer, I want `LCDRenderTexture()` to display the RGB565 buffer in the SDL2 window, so that I can validate screen designs visually.

#### Acceptance Criteria

1. WHEN `LCDRenderTexture(texture)` is called, THE LCD_Simulator SHALL copy the 240×240 UINT16 buffer to the SDL texture via `SDL_UpdateTexture` with a pitch of 480 bytes per row (240 × sizeof(UINT16))
2. WHEN `LCDRenderTexture(texture)` is called, THE LCD_Simulator SHALL present the updated texture to the window via `SDL_RenderCopy` followed by `SDL_RenderPresent`
3. WHEN `LCDRenderTextureInArea(xStart, yStart, xEnd, yEnd, texture)` is called, THE LCD_Simulator SHALL update only the rectangular region defined by (xStart, yStart) to (xEnd-1, yEnd-1) of the SDL texture, reading source pixels from the full 240-wide buffer at offset `xStart + row * 240` for each row from yStart to yEnd-1, and then present the texture to the window
4. WHEN `LCDRenderPoint(x, y, color)` is called, THE LCD_Simulator SHALL update the single pixel at position (x, y) in the SDL texture and present the texture to the window
5. THE LCD_Simulator SHALL byte-swap each UINT16 pixel from big-endian (hardware SPI byte order) to native-endian before passing the buffer to `SDL_UpdateTexture`, so that displayed colors match what the GC9A01 panel would show

### Requirement 3: LCD Clear Operation

**User Story:** As a developer, I want `LCDClear(color)` to fill the entire SDL2 window with a uniform color, so that screen reset behaves identically to hardware.

#### Acceptance Criteria

1. WHEN `LCDClear(fillColor)` is called, THE LCD_Simulator SHALL fill the entire 240×240 texture with the specified RGB565 color and present it to the window
2. THE LCD_Simulator SHALL NOT byte-swap the fillColor in `LCDClear`, since the hardware `LCDClear` already byte-swaps before filling, and the simulator receives the color in native RGB565 format from the caller

### Requirement 4: HAL No-Op Implementations

**User Story:** As a developer, I want the simulator HAL functions to compile and link without errors, so that platform-agnostic code (Driver.c, Canvas.c) builds unchanged.

#### Acceptance Criteria

1. THE HAL_Simulator SHALL provide no-op implementations for `DigitalWrite`, `DigitalRead`, `SPIWriteByte`, `SPIWriteNByte`, `GPIOInit`, `GPIOSetDir`, `GPIOPullUp`, `SPIInit`, `GPIOSetFunction`, `PWMGPIOToSliceNum`, `PWMSetWrap`, `PWMSetChannelLevel`, `PWMSetClockDivider`, `PWMSetEnabled`, and `STDIOInitAll`, where each function signature (parameter types and return type) matches the RP2040/ESP32 HAL declarations exactly, and non-void functions (`DigitalRead`, `PWMGPIOToSliceNum`) return 0
2. THE HAL_Simulator SHALL provide Pico-SDK-compatible constant defines (`GPIO_FUNC_SPI`, `GPIO_IN`, `GPIO_OUT`, `GPIO_FUNC_PWM`, `PWM_CHAN_B`) as integer-valued `#define` macros so that Driver.c compiles unchanged
3. THE HAL_Simulator SHALL provide LCD pin defines (`LCD_DC_PIN`, `LCD_CS_PIN`, `LCD_CLK_PIN`, `LCD_MOSI_PIN`, `LCD_RST_PIN`, `LCD_BL_PIN`) and SD pin defines (`SD_SPI_CS`, `SD_SPI_SCLK`, `SD_SPI_MOSI`, `SD_SPI_MISO`, `SD_SPI_BAUDRATE`, `SD_DETECT_PIN`) as integer-valued `#define` macros with distinct non-negative dummy values so that any shared code including `HALConfig.h` compiles without undefined-symbol errors

### Requirement 5: Delay with SDL Event Pump

**User Story:** As a developer, I want `Delay(ms)` to pause execution while keeping the SDL2 window responsive, so that the simulator does not freeze during the main loop.

#### Acceptance Criteria

1. WHEN `Delay(milliseconds)` is called, THE HAL_Simulator SHALL wait for the specified duration using `SDL_GetTicks` to measure elapsed time in a polling loop
2. WHILE `Delay()` is executing its polling loop, THE HAL_Simulator SHALL call `SDL_PollEvent` on each iteration to process pending window events, and call `SDL_Delay(1)` to avoid busy-waiting CPU consumption
3. WHEN an `SDL_QUIT` event is received during `Delay()`, THE HAL_Simulator SHALL call `exit(0)` to terminate the application cleanly

### Requirement 6: SD Card Simulation via Filesystem Directory

**User Story:** As a developer, I want the simulator to read files from a local directory instead of a physical SD card, so that I can test PNG loading without hardware.

#### Acceptance Criteria

1. THE File_Simulator SHALL define `SD_DIRECTORY` in `HALConfig.h` pointing to a relative path (default: `"sample/sdcard"`)
2. WHEN `MountSdCard()` is called, THE File_Simulator SHALL verify that the `SD_DIRECTORY` path exists as an accessible directory and return true on success or false on failure
3. WHEN `OpenFile(file, name)` is called, THE File_Simulator SHALL open `SD_DIRECTORY/name` using standard `fopen` in read-binary mode and store the resulting `FILE*` in the `FIL` struct
4. IF `fopen` fails during `OpenFile(file, name)`, THEN THE File_Simulator SHALL return false without modifying the `file` parameter
5. WHEN `CloseFile(file)` is called, THE File_Simulator SHALL close the file using standard `fclose`
6. THE File_Simulator SHALL provide a `FIL` type as a typedef or struct containing a `FILE*` member, so that any function accepting `FIL*` (including `CanvasDrawPng` and `LCDRenderPng`) can compile and link against the simulator without source changes
7. THE File_Simulator SHALL provide an `f_read(FIL*, void*, UINT, UINT*)` function that delegates to standard `fread`, writing the number of bytes actually read to the fourth parameter, so that the existing `PngCustomReadData` callback operates without modification
8. WHEN `SelectActiveDrive()` is called, THE File_Simulator SHALL return true without performing any operation
9. WHEN `UnMountSdCard()` is called, THE File_Simulator SHALL return without performing any operation

### Requirement 7: PNG Read Function Compatibility

**User Story:** As a developer, I want libpng's custom read callback to work with the simulator's FILE*-based FIL type, so that PNG decoding works without FatFS.

#### Acceptance Criteria

1. THE File_Simulator SHALL provide an `f_read` function with the signature `FRESULT f_read(FIL* fp, void* buff, UINT btr, UINT* br)`, matching the FatFS parameter pattern (FIL pointer, destination buffer, bytes-to-read count, pointer to bytes-read output)
2. WHEN `f_read(file, buffer, length, &bytesRead)` is called in the simulator build, THE File_Simulator SHALL call `fread(buffer, 1, length, file->filePointer)` to read `length` bytes from the wrapped `FILE*` and store the return value of `fread` (the count of bytes actually read) in `*bytesRead`
3. WHEN `fread` reads fewer bytes than requested (partial read or EOF), THE File_Simulator SHALL still store the actual count in `*bytesRead` and return `FR_OK`
4. IF the `FILE*` inside the FIL struct is NULL when `f_read` is called, THEN THE File_Simulator SHALL set `*bytesRead` to 0 and return `FR_INVALID_OBJECT`
5. THE File_Simulator SHALL define the `FRESULT` enum (with at least `FR_OK` and `FR_INVALID_OBJECT`), the `UINT` type, and any other FatFS types referenced by the shared code, so that existing code using the FatFS return-value pattern compiles without modification

### Requirement 8: RTC Simulator Implementation

**User Story:** As a developer, I want the RTC module to compile in the simulator build, so that `RTCInitialize()` and `get_fattime()` resolve at link time.

#### Acceptance Criteria

1. WHEN `RTCInitialize()` is called, THE Simulator SHALL return without performing any operation
2. WHEN `get_fattime()` is called, THE Simulator SHALL return a `DWORD` (32-bit unsigned integer) containing the current host time formatted as a FAT timestamp using the standard C `time()` and `localtime()` functions, packed as: bits 31-25 = year-1980, bits 24-21 = month (1-12), bits 20-16 = day (1-31), bits 15-11 = hour (0-23), bits 10-5 = minute (0-59), bits 4-0 = second/2 (0-29)
3. THE Simulator RTC module SHALL define `DWORD` as `unsigned int` (or equivalent 32-bit unsigned type) if it is not already available from an included header

### Requirement 9: CMake Simulator Build Configuration

**User Story:** As a developer, I want to build the simulator with `cmake -DPLATFORM_NAME=Simulator`, so that the build system selects the correct sources and produces a native executable.

#### Acceptance Criteria

1. WHEN `PLATFORM_NAME` equals `"Simulator"`, THE Build_System SHALL compile `Platform/Simulator/HAL.c` and `Platform/Simulator/RTC.c` instead of `Platform/RP2040/` or `Platform/ESP32/` sources
2. WHEN `PLATFORM_NAME` equals `"Simulator"`, THE Build_System SHALL compile `LCD/Simulator/LCDSetup.c` and `LCD/Simulator/LCDRenderer.c` instead of `LCD/1in28/LCDSetup.c` and `LCD/1in28/LCDRenderer.c`
3. WHEN `PLATFORM_NAME` equals `"Simulator"`, THE Build_System SHALL compile a simulator-specific `FileHelper.c` (exposing the same API as the FatFS-based `Helper/FileHelper.c`) instead of the FatFS-based implementation
4. WHEN `PLATFORM_NAME` equals `"Simulator"`, THE Build_System SHALL link against SDL2 (located via `find_package` or `pkg-config`)
5. WHEN `PLATFORM_NAME` equals `"Simulator"`, THE Build_System SHALL compile libpng and zlib from the existing submodules using the `zlibstatic.cmake` approach
6. WHEN `PLATFORM_NAME` equals `"Simulator"`, THE Build_System SHALL NOT compile fatfs, DiskIO, or `Driver/GC9A01/Driver.c`
7. WHEN `PLATFORM_NAME` equals `"Simulator"`, THE Build_System SHALL produce a native executable (not a `.uf2` or `.bin`) using the host C compiler (gcc or clang)
8. WHEN `PLATFORM_NAME` equals `"Simulator"`, THE Build_System SHALL NOT include or require `pico_sdk_import.cmake`, Pico SDK, or ESP-IDF
9. WHEN `PLATFORM_NAME` equals `"Simulator"`, THE Build_System SHALL compile the shared sources (`Sample.c`, `Canvas.c`, `Trigonometry.c`, and font files) and set include paths to resolve headers from the `Platform/Simulator/` directory

### Requirement 10: Sample.c Unchanged

**User Story:** As a developer, I want `Sample.c` and `app_entry()` to compile and run in the simulator without any modifications, so that the simulator is purely additive.

#### Acceptance Criteria

1. THE Build_System SHALL compile `Sample.c` with identical source code for the Simulator platform as for RP2040 and ESP32, with no `#ifdef SIMULATOR` or platform-conditional code in `Sample.c`
2. WHEN `app_entry()` runs in the simulator, THE Simulator SHALL execute the same initialization sequence (`STDIOInitAll`, `RTCInitialize`, `LCDInitialize`, `LCDClear`, memory allocation, SD mount/open/close, canvas draw, `LCDRenderTexture`, main loop with `Delay`) and produce visible output in the SDL2 window

### Requirement 11: Sample SD Card Directory

**User Story:** As a developer, I want a versioned `sample/sdcard/` directory with test PNG images, so that the simulator has files to load out of the box.

#### Acceptance Criteria

1. THE Simulator SHALL include a `sample/sdcard/` directory at the project root containing a PNG file named `01.png`, tracked in version control so it is available immediately after cloning the repository
2. THE `01.png` file SHALL be a valid PNG image with dimensions of exactly 240×240 pixels, decodable by the existing libpng pipeline (any standard color type: RGB, RGBA, palette, or grayscale)
3. IF `01.png` is not present in the `sample/sdcard/` directory at runtime, THEN THE Simulator SHALL fail gracefully at the `OpenFile` call (returning false) and continue execution without rendering an image

### Requirement 12: VS Code Launch Configuration

**User Story:** As a developer, I want a launch.json entry for the simulator executable, so that I can debug it directly from VS Code/Kiro.

#### Acceptance Criteria

1. THE Simulator SHALL provide a `.vscode/launch.json` file containing a configuration named "Debug Simulator" that sets `"program"` to the simulator executable path and `"cwd"` to `"${workspaceFolder}"`
2. THE launch configuration SHALL use type `"cppdbg"` with `"MIMode"` set to `"gdb"` and a `"preLaunchTask"` referencing the simulator build task so the executable is compiled before each debug session
3. IF the `launch.json` file already exists, THEN THE Simulator SHALL append the new configuration to the existing `"configurations"` array without removing other entries

### Requirement 13: Simulator Toolchain Setup Task

**User Story:** As a developer, I want a "Setup: Simulator" task in tasks.json that installs SDL2 development libraries via apt, so that setting up the simulator build environment follows the same pattern as the RP2040 and ESP32 toolchains.

#### Acceptance Criteria

1. THE Simulator SHALL provide a `Toolchain/Simulator/Setup.sh` script that installs `libsdl2-dev` via `apt-get install -y`, checking first whether the package is already installed (idempotent)
2. THE Simulator SHALL provide a tasks.json entry named "Setup: Simulator toolchain in WSL2" that calls `Toolchain/Simulator/Setup.sh`, following the same structure and conventions as the existing "Setup: RP2040 toolchain in WSL2" and "Setup: ESP32 toolchain in WSL2" tasks
