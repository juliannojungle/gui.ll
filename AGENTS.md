# AGENTS.md — Project Knowledge Base

## Overview

**gui.ll** is a multi-platform embedded C project that reads PNG files from an SD card
and displays them on a GC9A01 round LCD (240x240). It targets **RP2040** (Raspberry Pi Pico)
and **ESP32-S3** from a single codebase with platform-specific abstractions.

Target devices (both expose 2×20 pin headers at 1.27mm pitch with an embedded round LCD on the back):
- **[RP2040-LCD-1.28](https://www.waveshare.com/wiki/RP2040-LCD-1.28)** — Waveshare RP2040 board
- **[ESP32-S3-LCD-1.28](https://www.waveshare.com/wiki/ESP32-S3-LCD-1.28)** — Waveshare ESP32-S3 board

---

## Project Structure

```
gui.ll/
├── CMakeLists.txt                  # Root cmake: if/else by PLATFORM_NAME
├── AGENTS.md                       # This file
├── .gitmodules                     # Submodule config (all locked with update=none)
├── .gitignore                      # build/, sdkconfig
│
├── Toolchain/
│   ├── RP2040/Setup.sh             # Installs arm-none-eabi-gcc, pico-sdk
│   └── ESP32/Setup.sh              # Installs ESP-IDF, xtensa toolchain, Rust, espflash
│
├── src/
│   ├── Sample.c                    # Entry point (app_entry → main or app_main)
│   │
│   ├── lib/
│   │   ├── Helper/
│   │   │   ├── FileHelper.c        # SD card mount/open/close (single FatFS volume SD_DRIVE)
│   │   │   └── PNGHelper.c         # PNG decode + LCD display via libpng
│   │   │
│   │   ├── Platform/
│   │   │   ├── RP2040/
│   │   │   │   ├── HAL.c           # HAL: GPIO, SPI, PWM, I2C (Pico SDK) — LCD SPI uses LCD_SPI from HALConfig.h
│   │   │   │   ├── HALConfig.h     # SD pins + SD_SPI(spi0) + SD_SPI_BAUDRATE; LCD pins + LCD_SPI(spi1); SD_DETECT_PIN
│   │   │   │   ├── RTC.h           # RTC via hardware/rtc.h + get_fattime()
│   │   │   │   ├── DiskIO.c        # FatFS disk I/O (SPI SD) — real CRC7 on all cmds; faithful no-OS-FatFS handshake; card detect ISR
│   │   │   │   ├── PreExecutable.cmake   # fatfs lib, patch inclusion
│   │   │   │   └── PostExecutable.cmake  # zlib, libpng, link libraries
│   │   │   │
│   │   │   └── ESP32/
│   │   │       ├── CMakeLists.txt   # idf_component_register (ESP-IDF component)
│   │   │       ├── HAL.c           # HAL: GPIO, SPI (ESP-IDF), LEDC PWM compat — LCD SPI uses LCD_SPI from HALConfig.h
│   │   │       ├── HALConfig.h     # SD pins + SD_SPI(SPI2_HOST) + SD_SPI_BAUDRATE; LCD pins + LCD_SPI(SPI3_HOST); SD_DETECT_PIN
│   │   │       ├── RTC.h           # RTC via settimeofday + get_fattime()
│   │   │       └── DiskIO.c        # FatFS disk I/O (ESP-IDF SPI master) — parity with RP2040: CRC7, manual CS, card detect ISR (IRAM_ATTR)
│   │   │
│   │   ├── Driver/GC9A01/          # LCD driver — uses LCD_* defines from HALConfig.h; DriverInit configures SPI, GPIO and backlight PWM
│   │   ├── LCD/                     # LCD commands (LCD_1in28)
│   │   ├── GUI/                     # Canvas/drawing utilities (Canvas.c/.h)
│   │   └── Fonts/                   # Font data
│   │
│   └── Dependency/
│       ├── fatfs/                   # Submodule: ChaN FatFS (DO NOT MODIFY)
│       ├── libpng/                  # Submodule: libpng (DO NOT MODIFY)
│       ├── zlib/                    # Submodule: zlib (DO NOT MODIFY)
│       ├── fatfs.ffconf_patch.cmake # Patches ffconf.h at build time
│       ├── zlibstatic.cmake         # Replaces zlib CMakeLists.txt at build time
│       └── pico_sdk_import.cmake    # Pico SDK cmake helper
│
└── .vscode/
    ├── tasks.json                   # Build/setup tasks (run via WSL)
    └── settings.json                # Git plugin config
```

---

## Architecture Principles

### 0. Naming Convention

- **PascalCase** for all project file and directory names
- **Acronyms in UPPERCASE** (HAL, RTC, PNG, SD, IO, HW)
- **Identifiers** (applied to project code; third-party/port code keeps its original style).
  The casing rules are:
  - **Functions / methods** → **PascalCase**, with a module prefix: the drawing API is `Canvas*`
    (`CanvasNewImage`, `CanvasClear`, `CanvasDrawLine`, `CanvasDrawText`, `CanvasDrawTime`, …).
  - **Types** (structs/typedefs) and **enum type names** ("classes") → **PascalCase**: `Canvas`,
    `SdCard`, `Rotate`, `Flip`, `PixelSize`, `PixelFillStyle`, `LineStyle`, `DrawFillStyle`,
    `DateTime`.
  - **Variables and function parameters** → **camelCase**: `xPoint`, `lineWidth`, `sdcard`.
  - **Struct / "class" attributes (members)** → **PascalCase**: `canvas.Width`, `canvas.Height`,
    `sdState.SectorCount`, `sdState.HighCapacity`, `dateTime.Year`.
  - **Constants** (macros, `#define`s, enum values) → **UPPER_SNAKE_CASE**: `LCD_BL_PIN`,
    `DEFAULT_PIXEL_SIZE`, `ROTATE_0`, `FLIP_HORIZONTAL`, `PIXEL_SIZE_2X2`, `LINE_STYLE_SOLID`,
    `DRAW_FILL_STYLE_EMPTY`. Enum values additionally carry a type-namespaced prefix because C
    enum values share the enclosing scope, so the prefix avoids collisions.
  - **Known exceptions (kept in original style on purpose):** only names that are a **contract
    with an external dependency** stay off-convention:
    - The FatFS diskio interface ChaN FatFS calls by name: `disk_initialize`, `disk_status`,
      `disk_read`, `disk_write`, `disk_ioctl` (and `get_fattime`), plus FatFS types/constants
      (`DSTATUS`, `DRESULT`, `BYTE`, `RES_OK`, `STA_NOINIT`, …).
    - Pico SDK / ESP-IDF API calls (`spi_write_blocking`, `gpio_put`, `spi_device_transmit`, …).
    - `HAL.c` provides a set of Pico-SDK-compatible **constants** (`GPIO_FUNC_SPI`, `GPIO_IN`,
      `GPIO_OUT`, `GPIO_FUNC_PWM`, `PWM_CHAN_B`, …) so the platform-agnostic `Driver.c` compiles
      unchanged on both targets. These are `#define`s, so they already fit the UPPER_SNAKE_CASE
      constant rule — they just happen to share names with the Pico SDK by design. HAL.c
      *functions* follow the project convention (`PWMGPIOToSliceNum`, `DigitalWrite`, …).

    Everything else in `DiskIO.c` (our own port internals — `SdCmd`, `SdAcquire`, `Crc7`,
    `SdCardState`, `sdState`, `SDCardInit`, …) now follows the project convention.
- **Exceptions** (not renamed):
  - Project meta files: `AGENTS.md`, `.gitmodules`, `.gitignore`, `CMakeLists.txt`
  - IDE directories/files: `.kiro/`, `.vscode/`, `tasks.json`, `settings.json`
  - Source root: `src/`, `lib/`
  - Anything inside `src/Dependency/` (submodule contents are untouched)

### 1. Platform Abstraction

All platform-specific code lives under `src/lib/Platform/<PLATFORM_NAME>/`.
The helpers and drivers are platform-agnostic — they call abstract functions
(`DigitalWrite`, `SPIWriteByte`, `Delay`, etc.) defined in each platform's `HAL.c`.

Include resolution works via cmake `include_directories` pointing to the active
platform folder. A single `#include "HAL.c"` in `Sample.c` pulls the right one.

### 2. Single SD Card

The design targets a **single SD card** on one SPI bus. The pins, SPI peripheral and baud rate
are defined in each platform's `HALConfig.h` (`SD_SPI`, `SD_SPI_SCLK/MOSI/MISO/CS`,
`SD_SPI_BAUDRATE`, `SD_DETECT_PIN`). `DiskIO.c` reads those defines directly and is wired to
FatFS physical drive 0.

`FileHelper.c` mounts/opens/closes that single card using a local `#define SD_DRIVE "0:"` — the
FatFS logical volume name. `"0:"` is a FatFS-level concept (maps to physical drive 0) and is
identical across platforms, so it lives in `FileHelper.c`, not in `HALConfig.h`. The helper
functions take no `SdCard` argument; they operate implicitly on `SD_DRIVE`.

> Multi-card support (the old `spi_t` / `SdCard` structs, `spis[]` / `sd_cards[]` arrays and
> `sd_get_num()` / `sd_get_by_num()` in `SDConfig.h` / `SDHWConfig.h`) was **removed** — see
> Decision 12. The hardware (a round LCD board with a single slot) never warranted it.

### 3. Entry Point

```c
void app_entry(void) { /* main logic */ }

#ifdef ESP_PLATFORM
void app_main(void) { app_entry(); }  // ESP-IDF entry
#else
int main(void) { app_entry(); return 0; }  // RP2040 entry
#endif
```

### 4. RTC / Timestamps

Each platform provides `RTC.h` with:
- `time_init()` — initializes timekeeping (hardware RTC on RP2040, settimeofday on ESP32)
- `get_fattime()` — provides FAT timestamps to FatFS (required when `FF_FS_NORTC == 0`)

`get_fattime()` must NOT be `static` — FatFS declares it as extern in `ff.h`.

### 5. Header / Source File Discipline

Target convention for **our own code** (migrate existing code toward this gradually — do NOT do a
sweeping rewrite; fix files as they are touched):

- **No function/method declarations in `.h`** — and certainly no definitions. Headers carry only
  type definitions (structs/enums/typedefs), macros and constants.
- **Function/method definitions always live in `.c`.** Because of the unity-build approach
  (Decision 5), `.c` files are `#include`d directly, so prototypes in headers are unnecessary —
  the definition is already visible at the include site.
- **`extern` variable declarations in `.h` only as a last resort** — when there is no more elegant
  alternative. The `.c` should "own" the instances/objects (define them); avoid exposing globals
  through `extern` in a header unless unavoidable.

Known current violations (inherited, to be migrated gradually — they work only because of the
single-translation-unit build):
- `Canvas.h` exposes `extern Canvas canvas;` (global via `extern`).
- `RTC.h` defines `time_init()` / `get_fattime()` (definitions in a header).

These are not failures of the unity-build design — they compile because there is effectively one
translation unit. They simply don't match the discipline above yet and will be reorganized as the
surrounding files are touched.

---

## Build System

### RP2040
- Uses **cmake + make** directly
- Pico SDK is included via `pico_sdk_import.cmake`
- FatFS compiled as static library in `PreExecutable.cmake`
- zlib/libpng compiled as static libraries in `PostExecutable.cmake`
- Generates `.uf2` for drag-and-drop flashing

### ESP32
- Uses **idf.py** (ESP-IDF build system) which internally calls cmake + ninja
- `EXTRA_COMPONENT_DIRS` points to `src/lib/Platform/ESP32` (avoids needing a `main/` folder)
- Component registered via `idf_component_register()` with all sources and includes
- Requires `source ~/esp-idf/export.sh` before build (sets toolchain in PATH)
- Third-party code (libpng, zlib) compiled with `-Wno-error=maybe-uninitialized` to suppress
  warnings promoted to errors by ESP-IDF's strict `-Werror=all`

### Incremental Build (both platforms)

The "Build: Incremental" task auto-detects the platform:
```bash
cd ~/gui.ll/build && if [ -f build.ninja ]; then
    source ~/esp-idf/export.sh 2>/dev/null && ninja
else
    make
fi
```

---

## Dependency Management — CRITICAL RULES

### DO NOT modify submodule contents directly

All dependencies are git submodules with `ignore = all`:
- `src/Dependency/fatfs` — ChaN FatFS
- `src/Dependency/libpng` — libpng
- `src/Dependency/zlib` — zlib

**If a dependency needs configuration changes, use cmake patches applied at build time.**

### Existing patches:

| File | What it does |
|------|-------------|
| `fatfs.ffconf_patch.cmake` | Sets `FF_FS_RPATH=1` and `FF_VOLUMES=2` in `ffconf.h` |
| `zlibstatic.cmake` | Replaces zlib's `CMakeLists.txt` with a minimal static-only build |
| `configure_file(pnglibconf.h.prebuilt → pnglibconf.h)` | Generates required libpng config header |

### Rationale
- Submodules stay at pinned commits (shallow clones)
- `ignore = all` prevents git from showing false "modified" status inside submodules
- Anyone cloning the repo gets a working build without manual intervention

---

## Development Environment

### Host: Windows + WSL (Ubuntu)

- Code edited on Windows (VS Code / Kiro) via `\\wsl.localhost\Ubuntu\...`
- Compilation happens inside WSL
- Tasks use `wsl -e bash -c "..."` to invoke Linux commands from Windows

### Toolchain Setup

Run the setup tasks (idempotent — safe to re-run):
- **"Setup: RP2040 toolchain in WSL"** → `Toolchain/RP2040/Setup.sh`
- **"Setup: ESP32 toolchain in WSL"** → `Toolchain/ESP32/Setup.sh`

Setup scripts check for existing installations before downloading.
ESP32 setup installs: apt deps, ESP-IDF, idf_tools (xtensa, gdb, openocd),
Python env, Rust, and espflash.

### Known Issues

- **Git pager in WSL**: The git config may have a pager that blocks non-interactive
  sessions. Use `GIT_PAGER=cat` or `git -c core.pager=` when scripting.
- **cmd.exe escaping**: Complex bash commands with pipes, parentheses, or nested quotes
  fail when passed inline via `wsl -e bash -c "..."`. Solution: put scripts in `.sh` files
  and call them via `wsl -e bash -c "~/path/to/Script.sh"`.
- **Pico SDK `picotool`**: Auto-downloaded by Pico SDK 2.x if not installed globally.
  Takes time on first full build. Can be pre-installed for faster builds.

### Language

All code comments, documentation, and commit messages must be in **English**.
The AGENTS.md itself is the reference for this rule.

---

## VS Code / Kiro Settings

`.vscode/settings.json`:
```json
{
    "git.detectSubmodules": false,
    "git.autoRepositoryDetection": "openEditors"
}
```
This prevents the git plugin from showing false "modified" files in submodules
(caused by line-ending differences between Windows and Linux).

---

## Build Tasks Reference

| Task | What it does |
|------|-------------|
| Build: Full RP2040 | Clean build with cmake + make |
| Build: Full ESP32 | Clean build with idf.py |
| Build: Incremental | Detects platform, runs make or ninja |
| Copy UF2 to Windows | Copies .uf2 to C:\temp for flashing |
| Setup: RP2040 toolchain | Installs arm toolchain + pico-sdk |
| Setup: ESP32 toolchain | Installs ESP-IDF + tools + Rust + espflash |

---

## Current Status (as of last session)

- **RP2040**: Builds and runs. SD card init + read confirmed working on a solid prototype.
- **ESP32-S3**: Builds successfully (`.bin`); SD path rewritten for parity but **not yet
  hardware-tested**.

Recent work:
- **Moved the `SDCardInit` prototype out of `HALConfig.h`** (§5 discipline): the header had a
  function declaration (a §5 violation) only because `SDCardInit` lives in the separately-compiled
  `DiskIO.c` (fatfs lib) and the caller `FileHelper.c` needed a prototype across TUs. The prototype
  is now a forward declaration inside `FileHelper.c` (the caller); both `HALConfig.h` are back to
  pure `#define`s. RP2040 builds and links.
- **ESP32 `HAL.c` internal identifiers standardized**: private statics/locals renamed to the
  project convention — `lcd_spi_handle`→`lcdSpiHandle`, `pwm_slice_pin`→`pwmSlicePin`, and locals
  `bus_cfg`/`dev_cfg`/`timer_cfg`/`ch_cfg`→`busCfg`/`devCfg`/`timerCfg`/`chCfg`. (Not compile-tested
  here — only RP2040 has a configured build, and it doesn't exercise the ESP32 HAL.)
- **Standardized `DiskIO.c` to the project naming convention** (both platforms): the no-OS-FatFS
  port internals were renamed to PascalCase functions/types + camelCase locals + PascalCase struct
  members (`sd_cmd`→`SdCmd`, `sd_acquire`→`SdAcquire`, `crc7`→`Crc7`, `sd_card_state_t`→
  `SdCardState`, `sd_state`→`sdState`, members `Initialized`/`SectorCount`/…). `Platform_SDCard_Init`
  → `SDCardInit` (dropped the `Platform` prefix; updated both `HALConfig.h` and `FileHelper.c`).
  Only external-contract names stay off-convention (FatFS `disk_*` + types, Pico/ESP-IDF APIs, and
  the `HAL.c` Pico-SDK compat shims). Logic is byte-for-byte the same; RP2040 builds and runs.
- **Removed the multi-SD-card abstraction** (Decision 12): deleted `SDConfig.h` and
  `SDHWConfig.h` on both platforms (`spi_t`, `SdCard`, `sd_spi_if_t`, `sd_if_type_t`, the
  `spis[]` / `sd_cards[]` arrays and `sd_get_num()` / `sd_get_by_num()` — all dead weight, since
  `DiskIO.c` reads pins straight from `HALConfig.h`). `FileHelper.c` now uses a local
  `#define SD_DRIVE "0:"` and its functions take no `SdCard*`; `Sample.c` calls them directly.
  RP2040 builds and runs.
- **PascalCase identifier pass** (see Naming Convention §0): `GUI_Paint.c/.h` → `Canvas.c/.h`,
  the `Paint` struct → `Canvas` (global `canvas`), the `Paint_*` drawing API → `Canvas*`
  (`CanvasNewImage`, `CanvasDrawLine`, `CanvasDrawText`, `CanvasDrawTime`, …), and the drawing
  enums renamed to PascalCase types with namespaced values (`Rotate`/`ROTATE_*`, `Flip`/`FLIP_*`,
  `PixelSize`/`PIXEL_SIZE_*`, `PixelFillStyle`/`PIXEL_FILL_STYLE_*`, `LineStyle`/`LINE_STYLE_*`,
  `DrawFillStyle`/`DRAW_FILL_STYLE_*`); the time struct is now `DateTime`. Also `sd_card_t` → `SdCard`.
  RP2040 builds and runs after the rename.
- SD pins / shared-shield design, LCD pin defines, `EPD_*`→`LCD_*`, backlight in `DriverInit`,
  card detect (see Design Decisions 6-8).
- **SD driver rewritten as a faithful port of no-OS-FatFS** (Design Decision 9): real CRC7 on
  every command, CMD0/CMD8/CMD58/ACMD41 handshake, CS held low across the whole init, block vs
  byte addressing (SDHC vs SDSC). Same structure on both platforms.
- **SPI instance/host is configurable** in `HALConfig.h` (Design Decision 10): `SD_SPI` and
  `LCD_SPI` defines; no hardcoded `spi0`/`SPI_PORT` in driver code. RP2040: SD=spi0, LCD=spi1.
  ESP32-S3: SD=SPI2_HOST, LCD=SPI3_HOST.
- **Hard-won debugging lesson** (Design Decision 11): a flaky "CMD0 works, CMD8/ACMD41 silent"
  symptom was **insufficient power**, not the SPI protocol. The SD card was powered from the
  RP2040 `ADC_AVDD` pin (filtered, high-impedance) which sagged to 2.8V under load. CMD0 (low
  current) worked; ACMD41 (card init current spike) browned the card out. Always rule out power
  before chasing protocol bugs.

---

## Design Decisions Log

1. **FatFS over no-OS-FatFS**: Switched from no-OS-FatFS-SD-SPI-RPi-Pico to pure ChaN FatFS
   for portability across platforms. A `sd_card_t`/`SdCard` parametrization was initially kept in
   `SDConfig.h` / `SDHWConfig.h` to preserve multi-card support, but was **later removed**
   (Decision 12) once it proved to be unused dead weight.

2. **EXTRA_COMPONENT_DIRS over COMPONENT_DIRS**: Using `EXTRA_COMPONENT_DIRS` in ESP32 cmake
   adds our component alongside ESP-IDF's built-in components (driver, esp_system, etc.).
   `COMPONENT_DIRS` would replace all defaults and break things.

3. **No `main/` directory for ESP32**: The ESP-IDF convention of requiring a `main/` folder
   is bypassed via `EXTRA_COMPONENT_DIRS` pointing directly to the platform folder.

4. **Setup scripts in `Toolchain/`**: Avoids cmd.exe escaping issues with complex inline
   bash commands in tasks.json. Scripts are idempotent and self-documenting.

5. **`#include "HAL.c"` pattern**: The project uses a single-translation-unit approach
   where .c files are included directly (not compiled separately). This is intentional —
   do not refactor into separate compilation units unless explicitly requested.

6. **LCD pin ownership**: All LCD GPIO definitions (`LCD_DC_PIN`, `LCD_CS_PIN`, `LCD_CLK_PIN`,
   `LCD_MOSI_PIN`, `LCD_RST_PIN`, `LCD_BL_PIN`) live exclusively in each platform's `HALConfig.h`.
   The `Driver.c` reads them via `#include "HALConfig.h"` — no pin numbers hardcoded in driver code.
   Prefix is `LCD_*` (not `EPD_*`). Backlight PWM is initialized inside `DriverInit` — never in
   helpers or application code. `PNGHelper.c` has no pin knowledge whatsoever.

7. **Shared shield GPIO selection**: Both target boards expose 2×20 pin headers (1.27mm pitch).
   The LCD is internally wired to the same GPIO numbers on both devices, but those GPIOs occupy
   *different physical positions* on the headers. After overlaying the two pinouts, the only
   free GPIOs that land on the same header position on both boards are listed below.
   Already assigned to SD card SPI (MISO, CS, SCK, MOSI respectively):
   - RP2040 GP0 = ESP32-S3 GP46 → SD MISO
   - RP2040 GP1 = ESP32-S3 GP45 → SD CS
   - RP2040 GP2 = ESP32-S3 GP42 → SD SCK
   - RP2040 GP3 = ESP32-S3 GP41 → SD MOSI

   Already assigned to SD card detect:
   - RP2040 GP5 = ESP32-S3 GP39 → SD_DETECT_PIN (H1 pin 12, normally-open switch to GND)

   Additional overlapping free pins available for future features — **header H1** (same physical
   pin number on both boards):
   | H1 Pin | RP2040 GPIO | ESP32-S3 GPIO | Notes |
   |--------|-------------|---------------|-------|
   | 11     | GP13        | GP18          | ✅ usable |
   | 13     | GP14        | GP17          | ✅ usable |
   | 15     | GP15        | GP16          | ✅ usable |
   | 17     | SWCLK       | GP15          | ⚠️ RP2040 SWD debug pin — usable as GPIO but disables SWD debug while shield is connected |
   | 19     | SWDIO       | GP14          | ⚠️ RP2040 SWD debug pin — usable as GPIO but disables SWD debug while shield is connected |

   **Header H2** — pin numbering order differs between boards, listed as RP2040 pin = ESP32-S3 pin:
   | RP2040 H2 Pin | ESP32-S3 H2 Pin | RP2040 GPIO | ESP32-S3 GPIO | Notes |
   |---------------|-----------------|-------------|---------------|-------|
   | 2             | 19              | GP16        | GP13          | ✅ usable |
   | 7             | 14              | GP27        | GP2           | ✅ usable |
   | 5             | 16              | GP26        | GP3           | ❌ ESP32-S3 GP3 is a strapping pin — HIGH at boot triggers UART download mode, unsafe for shield use |

   ⚠️ **Total: 9 overlapping physical pins** (6 on H1 + 3 on H2). No other pins can be
   reused on a shield compatible with both boards. Of these 9: 5 are assigned (SD SPI + detect),
   H2-5/16 (GP26/GP3) is excluded due to the ESP32-S3 strapping pin restriction, leaving
   **3 fully usable pins** + 2 with SWD caveat (H1-17, H1-19) for future features.

   Reference pinout images: `Documentation/Image/RP2040_LCD_1_28.png` and
   `Documentation/Image/ESP32_S3_LCD_1_28.png` (also linked in README.md Supported Platforms table).

8. **SD card detect design**: The target device is subject to physical impacts that can eject
   the SD card. A dedicated card detect pin (`SD_DETECT_PIN`) is wired to H1 pin 12 on the
   shared shield:
   - RP2040: GP5 — ESP32-S3: GP39
   - Switch type: **normally open**, closes to GND when card is present
   - Firmware configures internal pull-up; `card_detected_true = 0` (LOW = present)
   - ISR registered on both edges (insert and remove) resets `sdState.Initialized = false`,
     forcing `disk_initialize()` to run again on the next `MountSdCard()` call
   - `SDCardInit()` checks the detect pin before attempting SPI — returns `false`
     immediately if card is absent, avoiding the ~1s SPI timeout
   - RP2040: ISR via `gpio_set_irq_enabled_with_callback` (Pico SDK)
   - ESP32-S3: ISR via `gpio_isr_handler_add` with `IRAM_ATTR` (required by ESP-IDF)

9. **SD SPI driver is a faithful port of no-OS-FatFS**: `DiskIO.c` on both platforms mirrors the
   proven no-OS-FatFS-SD-SPI-RPi-Pico init/command logic. (Identifiers were renamed to the
   project convention — §0 — but the init/command *logic* is unchanged.) Key points that MUST be
   preserved:
   - **Real CRC7 on every command** (table from no-OS-FatFS `crc.c`), not just CMD0/CMD8.
     Some cards reject commands with a dummy CRC even in SPI mode. CMD59 enables CRC on the card.
   - **CS held LOW continuously** through the whole init handshake (one `SdAcquire` at the
     start, one `SdRelease` at the end) — no per-command CS toggling, no deselect pulses.
   - Handshake order: 74+ init clocks (CS high) → CMD0 (retried) → CMD8 (SDv2 detect) → CMD59 →
     ACMD41 loop until idle bit clears → CMD58 (CCS/capacity) → read CSD.
   - **Block vs byte addressing**: SDHC/SDXC use block addressing; SDSC multiplies the LBA by 512.
   - Do NOT reintroduce deselect pulses, whole-sequence retries, or per-command CS toggling —
     those were dead-ends tried during debugging; the real bug was power (Decision 11).

10. **SPI instance/host lives in HALConfig**: The SPI peripheral for SD and LCD is declared in
    each platform's `HALConfig.h`, never hardcoded in driver code:
    - `SD_SPI`  — RP2040: `spi0`; ESP32-S3: `SPI2_HOST`
    - `LCD_SPI` — RP2040: `spi1`; ESP32-S3: `SPI3_HOST`
    - **RP2040**: the SPI peripheral is fixed by the pin (GP0-7/16-23 = spi0; GP8-15/26-28 = spi1).
      `SD_SPI`/`LCD_SPI` MUST match the chosen pins or the bus is silent.
    - **ESP32-S3**: the GPIO matrix routes any pin to any host, so the host isn't pin-fixed;
      SD and LCD are kept on separate hosts (SPI2/SPI3) so they don't contend.
    - ESP32-S3 SD uses **manual CS** (`spics_io_num = -1`, CS driven as GPIO) so CS can be held
      low across a full command/response/data sequence; clock changes use
      `spi_bus_remove_device`/`spi_bus_add_device` (NOT a second `spi_bus_initialize`).

11. **Power before protocol (debugging lesson)**: A long debugging session chasing an
    "CMD0 ok, CMD8/ACMD41 return 0xFF" symptom turned out to be **insufficient supply voltage**,
    not a firmware bug. The SD card had been powered from the RP2040 `ADC_AVDD` header pin, which
    is a filtered, high-impedance analog reference — it read 3.2V unloaded but sagged to 2.8V
    under the card's load, browning out the card during ACMD41's init-current spike. Low-current
    commands (CMD0) survived; higher-current ACMD41 did not. Lessons:
    - Do NOT power the SD card from `ADC_AVDD`; use the real 3V3 rail or a dedicated LDO from VSYS.
    - Keep the 10µF decoupling cap physically next to the card.
    - When commands fail *intermittently* or *partway through* the handshake with correct CRC,
      suspect power/signal integrity (supply sag, weak source, long flying wires, common ground)
      before rewriting protocol code.

12. **Dropped multi-SD-card support**: The `sd_card_t`/`SdCard` parametrization inherited from
    no-OS-FatFS (Decision 1) was removed. Investigation showed it was dead weight:
    - `DiskIO.c` (both platforms) reads pins, SPI instance and baud rate **directly from
      `HALConfig.h`** — it never touched `spi_t`, `spis[]`, or any SPI/pin field of `SdCard`.
    - The only field of `sd_cards[]` actually consumed at runtime was `pcName` (`"0:"`), used by
      FatFS in `FileHelper.c`.
    - `sd_get_num()` returned `count_of(sd_cards)`, a compile-time `1`, so the `if (sd_get_num() >
      0)` guard in `Sample.c` was always true.

    Removed `SDConfig.h` and `SDHWConfig.h` on both platforms. `FileHelper.c` now hardcodes the
    FatFS volume via `#define SD_DRIVE "0:"` (a FatFS-level name, platform-independent — ChaN
    FatFS uses the same `pdrv 0` everywhere) and its functions take no `SdCard*`. `Sample.c`
    calls `MountSdCard()` / `SelectActiveDrive()` / `OpenFile(&file, …)` / `UnMountSdCard()`
    directly. The hardware (single round-LCD board, one SD slot) never justified multi-card
    support. If multiple cards are ever needed, reintroduce a small parametrization then — don't
    keep unused abstraction "just in case".
