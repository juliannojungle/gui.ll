# Requirements Document

## Introduction

Refactor the file I/O layer so that FatFS is used on all platforms (RP2040, ESP32, and Simulator), eliminating the duplicate Simulator-specific FileHelper. The Simulator gains a new DiskIO.c that backs FatFS against a local FAT-formatted disk image file, making file I/O behavior identical across all three targets. The setup script is updated to create the disk image from the sample directory contents, and the CMake build is adjusted to compile the real FatFS for the Simulator.

## Glossary

- **FatFS**: ChaN's FAT filesystem module (source in `src/Dependency/fatfs/source/`)
- **DiskIO**: Platform-specific low-level disk I/O implementation providing `disk_initialize`, `disk_status`, `disk_read`, `disk_write`, `disk_ioctl` as required by FatFS
- **Shared_FileHelper**: The single FatFS-based FileHelper module at `src/lib/Helper/FileHelper.c/.h`, shared by all platforms
- **Simulator_DiskIO**: The new `src/lib/Platform/Simulator/DiskIO.c` implementing FatFS disk I/O against a local disk image file
- **Disk_Image**: The FAT-formatted file at `sample/sdcard.img` used by Simulator_DiskIO as a virtual SD card
- **Setup_Script**: The shell script at `Toolchain/Simulator/Setup.sh` that prepares the Simulator build environment
- **Simulator_Build**: The CMake build configuration block for `PLATFORM_NAME STREQUAL "Simulator"`
- **HALConfig**: The platform-specific configuration header (`Platform/Simulator/HALConfig.h`) defining hardware and path constants

## Requirements

### Requirement 1: Shared FileHelper Uses FatFS on All Platforms

**User Story:** As a developer, I want a single FileHelper implementation that uses FatFS on all platforms, so that file I/O behavior is consistent and bugs are caught in the Simulator.

#### Acceptance Criteria

1. THE Shared_FileHelper SHALL use FatFS API functions (`f_mount`, `f_open`, `f_read`, `f_write`, `f_close`, `f_chdrive`, `f_unmount`) for all file operations on all platforms (RP2040, ESP32, Simulator)
2. THE Shared_FileHelper SHALL reside at `src/lib/Helper/FileHelper.c` and `src/lib/Helper/FileHelper.h`
3. THE Shared_FileHelper SHALL contain no platform-specific conditional compilation (`#ifdef`) for file I/O logic

### Requirement 2: Simulator DiskIO Implementation

**User Story:** As a developer, I want the Simulator to have its own DiskIO.c that operates on a local disk image file, so that FatFS can function identically to hardware platforms.

#### Acceptance Criteria

1. THE Simulator_DiskIO SHALL reside at `src/lib/Platform/Simulator/DiskIO.c`
2. THE Simulator_DiskIO SHALL implement `disk_initialize` by opening the Disk_Image file specified by the `SD_DISK_IMAGE` define from HALConfig
3. THE Simulator_DiskIO SHALL implement `disk_status` by returning status based on whether the Disk_Image file handle is valid
4. THE Simulator_DiskIO SHALL implement `disk_read` by seeking to the sector offset (`sector * 512`) in the Disk_Image file and reading `count * 512` bytes into the buffer
5. THE Simulator_DiskIO SHALL implement `disk_write` by seeking to the sector offset (`sector * 512`) in the Disk_Image file and writing `count * 512` bytes from the buffer
6. THE Simulator_DiskIO SHALL implement `disk_ioctl` with `CTRL_SYNC` by calling `fflush` on the Disk_Image file handle
7. THE Simulator_DiskIO SHALL implement `SDCardInit` by calling `disk_initialize(0)` and returning the success status
8. IF the Disk_Image file cannot be opened, THEN THE Simulator_DiskIO SHALL return `STA_NOINIT` from `disk_initialize`

### Requirement 3: Disk Image Creation in Setup Script

**User Story:** As a developer, I want the setup script to always recreate the disk image from the sample directory, so that changes to sample files are reflected without manual steps.

#### Acceptance Criteria

1. THE Setup_Script SHALL include `mtools` and `dosfstools` in the idempotent package installation list (alongside `libsdl2-dev` and `gdb`) since `mkfs.fat` and `mcopy` are provided by these packages
2. THE Setup_Script SHALL always recreate the Disk_Image at `sample/sdcard.img` on every invocation, regardless of whether it already exists
3. THE Setup_Script SHALL create the Disk_Image by allocating a zeroed file with `dd`, formatting it with `mkfs.fat`, and copying all files from `sample/sdcard/` into it with `mcopy`
4. THE Setup_Script SHALL size the Disk_Image large enough to hold all files in `sample/sdcard/` plus FAT metadata overhead (minimum 1 MB)

### Requirement 4: Remove Simulator-Specific FileHelper and ff.h Shim

**User Story:** As a developer, I want the old Simulator file I/O shim removed, so that the codebase has no dead code and the Simulator uses the real FatFS path.

#### Acceptance Criteria

1. WHEN the refactoring is complete, THE Simulator_Build SHALL not compile `src/lib/Platform/Simulator/FileHelper.c`
2. WHEN the refactoring is complete, THE Simulator_Build SHALL not reference `src/lib/Platform/Simulator/FileHelper.h`
3. WHEN the refactoring is complete, THE Simulator_Build SHALL not reference `src/lib/Platform/Simulator/ff.h`
4. THE Shared_FileHelper SHALL resolve `#include "ff.h"` to the real FatFS header at `src/Dependency/fatfs/source/ff.h` on all platforms including the Simulator

### Requirement 5: CMake Simulator Build Update

**User Story:** As a developer, I want the Simulator CMake block to compile FatFS and the new DiskIO, so that the Simulator links against the real filesystem stack.

#### Acceptance Criteria

1. THE Simulator_Build SHALL compile FatFS source files (`ff.c`, `ffsystem.c`, `ffunicode.c`) from `src/Dependency/fatfs/source/`
2. THE Simulator_Build SHALL compile the Simulator_DiskIO (`src/lib/Platform/Simulator/DiskIO.c`)
3. THE Simulator_Build SHALL compile the Shared_FileHelper (`src/lib/Helper/FileHelper.c`)
4. THE Simulator_Build SHALL include `src/Dependency/fatfs/source/` in the include path so that `#include "ff.h"` and `#include "diskio.h"` resolve correctly
5. THE Simulator_Build SHALL apply the `fatfs.ffconf_patch.cmake` patch before compiling FatFS sources
6. THE Simulator_Build SHALL not include `src/lib/Platform/Simulator/FileHelper.c` in the source list

### Requirement 6: HALConfig Update for Disk Image Path

**User Story:** As a developer, I want the Simulator HALConfig to define the disk image path, so that DiskIO.c knows which file to open without hardcoding paths.

#### Acceptance Criteria

1. THE HALConfig SHALL define `SD_DISK_IMAGE` as `"sample/sdcard.img"`
2. THE HALConfig SHALL remove the `SD_DIRECTORY` define (no longer needed since FatFS replaces direct directory access)
3. THE Simulator_DiskIO SHALL use the `SD_DISK_IMAGE` define from HALConfig to locate the Disk_Image file

### Requirement 7: Existing Platform Builds Unaffected

**User Story:** As a developer, I want RP2040 and ESP32 builds to remain unchanged, so that this refactoring does not break working hardware targets.

#### Acceptance Criteria

1. THE RP2040 build SHALL continue to compile and link with the existing `Platform/RP2040/DiskIO.c` and the Shared_FileHelper
2. THE ESP32 build SHALL continue to compile and link with the existing `Platform/ESP32/DiskIO.c` and the Shared_FileHelper
3. THE Shared_FileHelper SHALL maintain its existing public API (`MountSdCard`, `SelectActiveDrive`, `OpenFile`, `CloseFile`, `UnMountSdCard`) with identical signatures
