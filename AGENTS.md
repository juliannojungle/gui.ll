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
│   │   │   ├── FileHelper.c        # SD card mount/open/close (parametrized by sd_card_t)
│   │   │   └── PNGHelper.c         # PNG decode + LCD display via libpng
│   │   │
│   │   ├── Platform/
│   │   │   ├── RP2040/
│   │   │   │   ├── HAL.c           # HAL: GPIO, SPI, PWM, I2C (Pico SDK) — LCD SPI uses LCD_SPI from HALConfig.h
│   │   │   │   ├── HALConfig.h     # SD pins + SD_SPI(spi0); LCD pins + LCD_SPI(spi1); SD_DETECT_PIN
│   │   │   │   ├── SDConfig.h      # spi_t, sd_card_t struct definitions
│   │   │   │   ├── SDHWConfig.h    # Default SPI/SD arrays + sd_get_num/sd_get_by_num
│   │   │   │   ├── RTC.h           # RTC via hardware/rtc.h + get_fattime()
│   │   │   │   ├── DiskIO.c        # FatFS disk I/O (SPI SD) — real CRC7 on all cmds; faithful no-OS-FatFS handshake; card detect ISR
│   │   │   │   ├── PreExecutable.cmake   # fatfs lib, patch inclusion
│   │   │   │   └── PostExecutable.cmake  # zlib, libpng, link libraries
│   │   │   │
│   │   │   └── ESP32/
│   │   │       ├── CMakeLists.txt   # idf_component_register (ESP-IDF component)
│   │   │       ├── HAL.c           # HAL: GPIO, SPI (ESP-IDF), LEDC PWM compat — LCD SPI uses LCD_SPI from HALConfig.h
│   │   │       ├── HALConfig.h     # SD pins + SD_SPI(SPI2_HOST); LCD pins + LCD_SPI(SPI3_HOST); SD_DETECT_PIN
│   │   │       ├── SDConfig.h      # spi_t, sd_card_t struct definitions (ESP32 types)
│   │   │       ├── SDHWConfig.h    # Default SPI/SD arrays
│   │   │       ├── RTC.h           # RTC via settimeofday + get_fattime()
│   │   │       └── DiskIO.c        # FatFS disk I/O (ESP-IDF SPI master) — parity with RP2040: CRC7, manual CS, card detect ISR (IRAM_ATTR)
│   │   │
│   │   ├── Driver/GC9A01/          # LCD driver — uses LCD_* defines from HALConfig.h; DriverInit configures SPI, GPIO and backlight PWM
│   │   ├── LCD/                     # LCD commands (LCD_1in28)
│   │   ├── GUI/                     # Paint/drawing utilities
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

### 2. Multi-SD-Card Support

The design supports multiple SPI buses and multiple SD cards. Each platform defines:
- `SDConfig.h` — struct definitions (`spi_t`, `sd_card_t`, `sd_spi_if_t`)
- `SDHWConfig.h` — default hardware arrays (`spis[]`, `sd_cards[]`) and
  `sd_get_num()` / `sd_get_by_num()` implementations

The `FileHelper.c` functions are parametrized by `sd_card_t*`.
Users expand the arrays in `SDHWConfig.h` to add more SD cards.

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
   for portability across platforms. The `sd_card_t` parametrization was preserved in
   `SDConfig.h` / `SDHWConfig.h` to maintain multi-card support.

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
   - ISR registered on both edges (insert and remove) resets `sd_state.initialized = false`,
     forcing `disk_initialize()` to run again on the next `MountSdCard()` call
   - `Platform_SDCard_Init()` checks the detect pin before attempting SPI — returns `false`
     immediately if card is absent, avoiding the ~1s SPI timeout
   - RP2040: ISR via `gpio_set_irq_enabled_with_callback` (Pico SDK)
   - ESP32-S3: ISR via `gpio_isr_handler_add` with `IRAM_ATTR` (required by ESP-IDF)

9. **SD SPI driver is a faithful port of no-OS-FatFS**: `DiskIO.c` on both platforms mirrors the
   proven no-OS-FatFS-SD-SPI-RPi-Pico init/command logic. Key points that MUST be preserved:
   - **Real CRC7 on every command** (table from no-OS-FatFS `crc.c`), not just CMD0/CMD8.
     Some cards reject commands with a dummy CRC even in SPI mode. CMD59 enables CRC on the card.
   - **CS held LOW continuously** through the whole init handshake (one `sd_acquire` at the
     start, one `sd_release` at the end) — no per-command CS toggling, no deselect pulses.
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
