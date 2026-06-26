# AGENTS.md — Project Knowledge Base

## Overview

**gui.ll** is a multi-platform embedded C project that reads PNG files from an SD card
and displays them on a GC9A01 round LCD (240x240). It targets **RP2040** (Raspberry Pi Pico)
and **ESP32-S3** from a single codebase with platform-specific abstractions.

Target devices (both expose 2×20 pin headers at 1.27mm pitch with an embedded round LCD on the back):
- **[RP2040-LCD-1.28](https://www.waveshare.com/wiki/RP2040-LCD-1.28)** — Waveshare RP2040 board
- **[ESP32-S3-LCD-1.28](https://www.waveshare.com/wiki/ESP32-S3-LCD-1.28)** — Waveshare ESP32-S3 board

---

## Reference Documentation (GC9A01A datasheet)

The LCD driver datasheet lives in `Documentation/` and is versioned in two forms that serve
different roles:

- **`Documentation/GC9A01A.md`** — **primary / quick data source.** A markdown conversion of the
  datasheet. Use this first when looking up registers, command opcodes, parameter bit fields,
  defaults, memory layout, timing values, etc. Section 6 (commands) is a faithful reproduction:
  each command has its bit-layout table (D/CX, RDX, WRX, D17-8, D7..D0, HEX), the verbatim
  Description (with embedded tables rendered as markdown) and verbatim Restriction.
- **`Documentation/GC9A01A.pdf`** — **secondary source, for disambiguation.** The original 192-page
  PDF. Consult it only to resolve any doubt or ambiguity in the markdown (e.g. graphical
  waveform/timing diagrams and figures, which were summarized in text and not extracted as images).

Notes / known limitations of the markdown:
- Figures and graphical diagrams are described in text, **not** embedded as images (intentional —
  images are not extracted).
- Sections 1–5 and 7 reproduce all tables, but some prose/figure captions are summarized; section 6
  is verbatim. When precision matters, cross-check against the PDF.
- The markdown was produced from text/table extraction (pdfplumber + pdftotext); obvious OCR typos
  were corrected, but the PDF remains the ground truth.

---

## Project Structure

```
gui.ll/
├── CMakeLists.txt                  # Root cmake: if/else by PLATFORM_NAME
├── AGENTS.md                       # This file
├── .gitmodules                     # Submodule config (all locked with update=none)
├── .gitignore                      # build/, sdkconfig
│
├── Documentation/
│   ├── GC9A01A.pdf                 # LCD driver datasheet (original, 192 pages) — versioned
│   ├── GC9A01A.md                  # Markdown conversion of the datasheet — versioned
│   └── Image/                      # RP2040 / ESP32-S3 pinout reference images
│
├── Toolchain/
│   ├── RP2040/Setup.sh             # Installs arm-none-eabi-gcc, pico-sdk
│   ├── ESP32/Setup.sh              # Installs ESP-IDF, xtensa toolchain, Rust, espflash
│   └── wsl.sh                      # Restores WSL Windows interop (.exe) under systemd
│
├── src/
│   ├── Sample.c                    # Entry point (app_entry → main or app_main)
│   │
│   ├── lib/
│   │   ├── Types.h                 # Shared scalar aliases (UBYTE/UWORD/UDOUBLE)
│   │   ├── Helper/
│   │   │   ├── FileHelper.c/.h     # SD card mount/open/close (single FatFS volume SD_DRIVE)
│   │   │   ├── PNGHelper.c/.h      # PNG decode + LCD display via libpng
│   │   │   └── Trigonometry.c/.h   # Q16.16 integer cos/sin LUT (TrigCosQ16/TrigSinQ16) — float-free, deterministic
│   │   │
│   │   ├── Platform/
│   │   │   ├── RP2040/
│   │   │   │   ├── HAL.c/.h        # HAL: GPIO, SPI, PWM, I2C (Pico SDK) — LCD SPI uses LCD_SPI from HALConfig.h
│   │   │   │   ├── HALConfig.h     # SD pins + SD_SPI(spi0) + SD_SPI_BAUDRATE; LCD pins + LCD_SPI(spi1); SD_DETECT_PIN
│   │   │   │   ├── RTC.c/.h        # RTC via hardware/rtc.h; defines RTCInitialize() + get_fattime()
│   │   │   │   ├── DiskIO.c        # FatFS disk I/O (SPI SD) — real CRC7 on all cmds; faithful no-OS-FatFS handshake; card detect ISR
│   │   │   │   ├── PreExecutable.cmake   # fatfs lib, patch inclusion
│   │   │   │   └── PostExecutable.cmake  # zlib, libpng, link libraries
│   │   │   │
│   │   │   └── ESP32/
│   │   │       ├── CMakeLists.txt   # idf_component_register (ESP-IDF component)
│   │   │       ├── HAL.c/.h        # HAL: GPIO, SPI (ESP-IDF), LEDC PWM compat — LCD SPI uses LCD_SPI from HALConfig.h
│   │   │       ├── HALConfig.h     # SD pins + SD_SPI(SPI2_HOST) + SD_SPI_BAUDRATE; LCD pins + LCD_SPI(SPI3_HOST); SD_DETECT_PIN
│   │   │       ├── RTC.c/.h        # RTC via settimeofday; defines RTCInitialize() + get_fattime()
│   │   │       └── DiskIO.c        # FatFS disk I/O (ESP-IDF SPI master) — parity with RP2040: CRC7, manual CS, card detect ISR (IRAM_ATTR)
│   │   │
│   │   ├── Driver/GC9A01/          # LCD driver (Driver.c/.h) — uses LCD_* defines from HALConfig.h; DriverInitialize configures SPI, GPIO and backlight PWM; DriverSetBacklightBrightness sets PWM level
│   │   ├── LCD/1in28/               # GC9A01 1.28" panel layer, split in two TUs:
│   │   │   ├── LCDSetup.c/.h       # Panel bring-up: DriverInitialize + reset, scan/attributes, register init, backlight; owns the LCD_ATTRIBUTES LCD global; exposes LCDInitialize()
│   │   │   └── LCDRenderer.c/.h    # Pixel/area blitting: LCDSetDisplayArea, LCDClear, LCDDisplayTexture(/InArea/Point)
│   │   ├── GUI/                     # Canvas/drawing utilities (Canvas.c/.h) — includes CanvasDrawPng (PNG → RAM texture)
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
(`DigitalWrite`, `SPIWriteByte`, `Delay`, etc.) declared in each platform's `HAL.h` and
defined in the matching `HAL.c`.

Include resolution works via cmake `include_directories` pointing to the active platform folder,
so a single `#include "HAL.h"` resolves to the right platform's HAL. The HAL `.c` for the
selected platform is added to the build's source list (RP2040: `add_executable`; ESP32:
`idf_component_register SRCS`).

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

Each platform provides an `RTC.h` (prototype) + `RTC.c` (definitions) pair with:
- `RTCInitialize()` — initializes timekeeping (hardware RTC on RP2040, settimeofday on ESP32)
- `get_fattime()` — provides FAT timestamps to FatFS (required when `FF_FS_NORTC == 0`)

`get_fattime()` must NOT be `static` — FatFS declares it as extern in `ff.h`. It is defined in
`RTC.c` (exactly one definition) rather than in the header.

### 5. Header / Source File Discipline

The project uses **standard C separate compilation**: every `.c` is its own translation unit,
compiled to a `.o` and linked (see Decision 5). Each module is a `.c`/`.h` pair, so headers carry
the **public interface** of the module:

- **Each module's `.h` declares its public symbols** — function prototypes for the functions
  defined in the matching `.c`, plus the type definitions (structs/enums/typedefs), macros and
  constants that callers need. A `.c` includes its own `.h` (and the `.h` of any module it calls).
- **Function/method definitions always live in `.c`.** Never define functions in headers — that
  causes duplicate-symbol link errors when the header is included by more than one `.c`. Mark
  helpers that are private to a `.c` as `static`; only non-`static` functions belong in the `.h`.
- **Shared globals are owned by one `.c` and exposed via `extern` in the `.h`** (e.g.
  `extern Canvas canvas;` in `Canvas.h`, owned by `Canvas.c`; `extern LCD_ATTRIBUTES LCD;` in
  `LCDSetup.h`, owned by `LCDSetup.c`). Keep shared globals to a minimum, but `extern` in a
  header is the normal, accepted mechanism — not a last resort.
- **External-contract names stay off-convention**, as before: FatFS `disk_*` / `get_fattime`
  (declared by `ff.h`), Pico SDK / ESP-IDF APIs, and the `HAL.h` Pico-SDK compat constants.

> Historical note: this project previously used a unity build (`#include "X.c"`), which made
> prototypes in headers unnecessary and was the reason an earlier version of this rule *forbade*
> declarations in headers. That approach broke IDE/clangd intelligence (only `Sample.c` appeared
> in `compile_commands.json`), so the project migrated to traditional separate compilation. The
> old "no declarations in `.h`" rule is therefore retired — prototypes in headers are now required.

---

## Testing Policy — DO NOT WRITE TESTS

**Rule: do NOT write tests for this project.** Do not add unit tests, property-based tests, test
harnesses, mocking/stub layers, or any test build system. This project intentionally has **no test
structure** — the maintainer does not want that extra scaffolding in the repository.

- Do not create or expand a `test/` directory, test sources, test Makefiles/CMake, or vendor any
  testing library (Theft, Unity, CMocka, etc.).
- Verification is done by **building the firmware** (RP2040 / ESP32-S3) and **running on the device**.
  Confirm correctness with a clean compile + link and on-hardware behavior, not automated tests.
- Any `test/` directory that may exist transiently is **not versioned** (it is git-ignored) and will
  be deleted after on-device validation. Treat it as disposable scratch — never rely on it and never
  reference it from project code or docs.
- When a spec's task plan lists test tasks, treat them as **skipped/out of scope** unless the
  maintainer explicitly asks otherwise for that session.

---

## Code Comments Policy — KEEP COMMENTS MINIMAL

**Rule: write self-explanatory code, not comments.** Prefer clear names and structure over prose.
Only add a comment when it carries information the code cannot:

- **Allowed**: a non-obvious *why* (rationale, a subtle invariant, a bug that a naive change would
  reintroduce), a disambiguation, a necessary technical note (units, fixed-point format, overflow
  reasoning, a hardware/spec quirk), or a short header explaining a module's purpose.
- **Not allowed**: comments that restate what the code already says (`// increment i`, `// return`,
  `// NULL check`), step-by-step narration above/inside a function, or per-line annotations.
- **No requirement/spec tags in code** (e.g. `// Req 3.1`, `// Decision 14`). Traceability lives in
  the spec/`AGENTS.md`, not as inline tags scattered through the implementation.
- Keep necessary comments **short** — a line or two. If a block needs a paragraph to be understood,
  prefer refactoring (extract a well-named helper) over explaining it.

This applies to new code and to edits of existing code. (Pre-existing third-party/port code under
`src/Dependency/` and inherited Waveshare comments are left as-is unless touched.)



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
- **"Setup: WSL interop"** → `Toolchain/wsl.sh` (restores running Windows `.exe` from WSL)

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
- **WSL interop broken under systemd (`Exec format error` on `.exe`)**: With `systemd=true`
  in `/etc/wsl.conf`, the `binfmt_misc` handler that lets Linux run Windows `.exe` files is
  registered by `systemd-binfmt.service` from `/etc/binfmt.d/WSLInterop.conf`. If that service
  is **masked** (a common leftover: `/etc/systemd/system/systemd-binfmt.service -> /dev/null`),
  the handler is never registered on boot and any `.exe` (e.g. `usbipd.exe`) fails with
  "cannot execute binary file: Exec format error". This is **not** a `wsl.conf` problem — interop
  does not need a `[interop]` entry, and `wsl.conf` is not being wiped. Fix: run the
  **"Setup: WSL interop"** task (`Toolchain/wsl.sh`), which ensures `WSLInterop.conf` exists and
  unmasks/restarts `systemd-binfmt.service` so the handler survives every reboot and
  `wsl --shutdown`. (`systemd-binfmt.service` is a static unit — it cannot be `systemctl enable`d
  and does not need to be; unmasking is what matters.) Verify with
  `cat /proc/sys/fs/binfmt_misc/WSLInterop`.

### Language

All code comments, documentation, and commit messages must be in **English** — this includes
source code, code comments, `AGENTS.md` and `README.md`. The AGENTS.md itself is the reference
for this rule.

**Chat/conversation with the user is always in Brazilian Portuguese (pt-BR)** — only the chat;
all artifacts that land in the repo (code, comments, docs, commit messages) stay in English.

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
| Setup: WSL interop | Restores running Windows `.exe` from WSL (unmasks systemd-binfmt) |

---

## Current Status (as of last session)

- **RP2040**: Builds and runs. SD card init + read confirmed working on a solid prototype.
- **ESP32-S3**: An ESP-IDF 5.3.0 environment is available locally and was used this session to do a
  full `esp32s3` build (isolated build dir, root `sdkconfig` left untouched) — `Canvas.c` compiled and
  `gui.ll.elf`/`.bin` linked, so the separate-compilation + curved-char changes **are now
  compile-tested on ESP32-S3** (earlier parity changes elsewhere may still predate this). SD path
  rewritten for parity but **not yet hardware-tested**.

Recent work:
- **Added `CanvasDrawCurvedText` to the Canvas module** (`src/lib/GUI/Canvas.c`, prototype in
  `Canvas.h`) — a thin loop over `CanvasDrawCurvedChar` that renders a whole string along the circle.
  Signature mirrors the curved-char primitive, leading with the text then the geometric placement:
  `void CanvasDrawCurvedText(const char *text, UWORD xCenter, UWORD yCenter, UWORD radius,
  UWORD startAngle, sFONT* font, UWORD foregroundColor, UWORD backgroundColor)`. It holds center,
  radius, font and colors constant and **advances the angle per glyph**:
  - **Per-glyph angular step from arc length, in integers.** Each glyph occupies `font->Width` pixels
    of arc; the angular step for a pixel arc `s` at radius `r` is `s/r` radians = `s/r * 180/PI`
    degrees. The conversion is done with integer math (no runtime float, matching Decision 15):
    `180/PI` is approximated as `57296/1000` and divided via the existing `CanvasRoundDivAway`
    helper. Error < 0.0014° over a full turn — negligible at whole-degree resolution.
  - **Cumulative arc, no drift.** The consumed arc length is accumulated (`arcPixels += width`) and
    each character's angle is computed from the running total, so per-character rounding does not
    drift across the string. The first glyph is centered at `startAngle`; subsequent glyphs advance
    clockwise (increasing angle = the tangent reading direction).
  - **Guards / edge cases**: `text == NULL` or `font == NULL` is a no-op; each character angle is
    normalized to 0..359 before being passed on (so a long string cannot overflow the `UWORD` angle);
    `radius == 0` keeps the offset at 0 (every glyph anchors at the center — degenerate but safe, no
    divide-by-zero); spaces advance the angle without drawing (handled by `CanvasDrawCurvedChar`).
  - **Resolution note**: `CanvasDrawCurvedChar` only accepts whole-degree angles (360-entry LUT), so
    spacing is quantized to 1° (≈ `radius·0.0175` px of jitter — imperceptible at typical radii). Sub-
    degree spacing would need a fractional-angle curved-char variant.
  - Builds and links clean on RP2040; **validated on hardware** — text follows the rim correctly.
    No build-system changes (lives in the shared `Canvas.c`).
- **Added `CanvasDrawCurvedChar` to the Canvas module** (`src/lib/GUI/Canvas.c`, prototype in
  `Canvas.h`) — a new drawing primitive that places a single ASCII character on a circle of a given
  `radius` around `(xCenter, yCenter)` at a whole-degree `startAngle`, with the glyph rotated so its
  baseline stays tangent to the circle border (text that follows the round panel's rim). Signature:
  `void CanvasDrawCurvedChar(const char ASCIIChar, UWORD xCenter, UWORD yCenter, UWORD radius,
  UWORD startAngle, sFONT* font, UWORD foregroundColor, UWORD backgroundColor)` — parameter order
  leads with the char then the geometric placement values so a future `CanvasDrawCurvedText` /
  `CanvasDrawCurvedNumber` can loop over characters by advancing `startAngle` (those helpers are
  out of scope here). It reuses the existing 1bpp `sFONT` glyphs (Font8/12/16/20/24), writes every
  pixel through `CanvasSetPixel`, and honors the `TRANSPARENT` color-key background (Decision 14).
  Implementation details (see Decision 15):
  - **No runtime floating point.** Trigonometry comes from two file-local `static const int32_t`
    Q16.16 lookup tables (`canvasCurvedCosTable[360]` / `canvasCurvedSinTable[360]`, `SCALE = 1<<16`)
    embedded as compile-time constant literals (generated offline). Two `static` helpers,
    `CanvasRoundDivAway` (round-half-away-from-zero integer division on an `int64_t` numerator) and
    `CanvasRotateGlyphOffset` (clockwise doubled-delta rotation about the glyph-box center), keep the
    whole transform in integers so both targets compute bit-for-bit identical pixels (Req 7.2/7.3).
  - **Guards / safety**: NULL `font` and out-of-range `ASCIIChar` (outside 0x20..0x7E) are no-ops;
    `startAngle` is normalized `% 360`; rotated targets are computed as signed `int32_t` and skipped
    when negative *before* the `UWORD` cast, so no negative wraps into `CanvasSetPixel`.
  - **Angle convention & tangent orientation**: 0° at three-o'clock, increasing clockwise in screen
    coords; `radius == 0` anchors exactly at the center. The glyph is oriented **tangent** to the
    circle, not radial: the placement angle positions the anchor, but the glyph rotation uses
    `orientAngle = (angle + 90) % 360` (the tangent direction `(-sin, cos)`). This is the key so that
    advancing `startAngle` sweeps a *readable* line of text along the border — a plain rotation by the
    placement angle would point each letter radially out of the center and not read along the arc
    (this was a real bug, fixed). Upright text therefore occurs at `startAngle == 270` (top of the
    circle, horizontal tangent); at `startAngle == 0` the glyph is rotated 90° (baseline runs vertically
    down the right side).
  - **Known nuance (Req 3.4)**: at `startAngle == 270` the glyph matches upright `CanvasDrawChar`
    *except* for a ±1-pixel center seam on fonts with an even Width or Height — an inherent
    consequence of round-half-away rotation about a half-integer geometric center (Req 3.3). All five
    bundled fonts have even Height; Font20 also has even Width. This was an accepted spec relaxation,
    not a bug.
  - **No build-system changes**: `Canvas.c` is already in `GUILL_COMMON_SRCS` (RP2040) and the ESP32
    `idf_component_register SRCS`. RP2040 **and** ESP32-S3 both compile and link the function from the
    single shared `Canvas.c` (verified — ESP32-S3 was actually compile-tested this session, not just
    parity-assumed; the symbol is present as global text in both object files).
  - **Spec**: the requirements, design and task plan live in `.kiro/specs/canvas-draw-curved-char/`.
- **Added `CanvasDrawPng(FIL *file)` to the Canvas module** (`src/lib/GUI/Canvas.c`, prototype in
  `Canvas.h`) for **flicker-free PNG rendering**. It is a faithful, step-by-step copy of
  `LCDRenderPng` (`LCD/1in28/LCDRenderer.c`) — same libpng decode pipeline, error handling
  (`setjmp`/`longjmp`, `volatile png_bytep rowPointers`), palette/non-palette handling, and RGB565
  bit math — with exactly two differences: (1) each decoded pixel is written into the in-RAM canvas
  buffer via `CanvasSetPixel(col, row, color)` instead of two `SPIWriteByte` calls, and (2) all
  LCD-panel-only operations are omitted (`LCDSetDisplayArea`, DC/CS `DigitalWrite`, the
  `lcdSelected` chip-select tracking, the CS restore in the error path, and the final
  `DriverSendCommand(0x29)`). Clipping is computed against the **canvas** dimensions
  (`canvas.Width`/`canvas.Height`), not the panel (`LCD.WIDTH`/`LCD.HEIGHT`). RGB565 is packed into
  a single `UWORD` with the identical high/low byte layout `CanvasSetPixel` (scale 65) stores
  big-endian, so the buffer bytes are bit-for-bit what `LCDRenderPng` would have streamed — and what
  `LCDRenderTexture` later blits.
  - **Why**: decoding straight to the panel updates the screen row-by-row with inflate latency
    between rows, producing a visible top-to-bottom scan/flicker. Decoding into a full-screen
    texture, drawing text over the same buffer, then doing a single `LCDRenderTexture` blit makes
    the update smooth.
  - **Caller flow** (`Sample.c`): allocate texture → `CanvasNewImage(...)` → `CanvasSetScale(65)` →
    mount/open SD → `CanvasDrawPng(&file)` → close/unmount → `CanvasDrawText(...)` →
    `LCDRenderTexture(texture)`. The canvas must be configured (scale 65, dims = `LCD.WIDTH`/
    `LCD.HEIGHT`) **before** `CanvasDrawPng` so clipping and the RGB565 byte layout are correct.
  - **libpng helpers**: `Canvas.c` defines its own **`static`** `PngCustomReadData` and
    `PngShowError` (copied verbatim from `LCDRenderer.c`). They are file-local so there is no link
    conflict with the non-static `PngCustomReadData` in `LCDRenderer.c` (different TUs, different
    linkage — well-defined in C). `Canvas.c` gained `<png.h>`, `"ff.h"`, `"Debug.h"` includes;
    `Canvas.h` gained `#include "ff.h"` (for the `FIL *` parameter type, mirroring `LCDRenderer.h`).
  - **`LCDRenderPng` is left byte-for-byte unchanged** — the direct-to-panel path still works exactly
    as before. Only `Canvas.c`, `Canvas.h`, and `Sample.c` were edited; no build-system changes
    (Canvas.c is already in `GUILL_COMMON_SRCS` and the ESP32 `idf_component_register SRCS`, and
    libpng/FatFS are already linked). RP2040 **builds, links, and is hardware-verified** (PNG renders
    flicker-free with the text overlay). ESP32 is parity-only (platform-agnostic, no `#ifdef`), not
    compile-tested here.
  - **Out of scope**: sub-area drawing (`CanvasDrawPngArea`) is deferred to a future spec.
- **Split the LCD layer into `LCD/1in28/LCDSetup` + `LCD/1in28/LCDRenderer`** (replaces the old
  `LCD/LCD_1in28.c/.h`, now deleted). `LCDSetup.c/.h` owns panel bring-up (`LCDInitialize()`:
  `DriverInitialize` + hardware reset, `LCDSetAttributes` for scan direction/dimensions, the
  GC9A01 register init sequence, and backlight) plus the `LCD_ATTRIBUTES LCD` global.
  `LCDRenderer.c/.h` owns drawing-to-panel (`LCDSetDisplayArea`, `LCDClear`, `LCDDisplayTexture`,
  `LCDDisplayTextureInArea`, `LCDDisplayTexturePoint`). Build wiring: `GUILL_COMMON_SRCS` lists
  both `.c`; the new headers live in the `LCD/1in28/` subfolder, so each platform's
  `PostExecutable.cmake` include path changed from `${PICO_CODE_LIB}/LCD` to
  `${PICO_CODE_LIB}/LCD/1in28`. RP2040 builds and runs. ESP32 wiring updated to match:
  `Platform/ESP32/CMakeLists.txt` now lists the two new `.c` in `SRCS` and `LCD/1in28` in
  `INCLUDE_DIRS` (parity-only, not compile-tested here).
- **`Driver.c/.h` gained backlight brightness control**: new `DriverSetBacklightBrightness(UINT
  brightness)` sets the backlight PWM channel level (0–99). `DriverInitialize` configures the PWM
  via `DriverBacklightPWMInitialize` (slice cached in the file-local `pwmBacklightBrightnessLevel`);
  `LCDSetup`'s `LCDInitialize()` calls `DriverSetBacklightBrightness(90)` after bring-up.
- **Migrated from unity build to standard separate compilation** (Decision 5 / §5 rewritten):
  each `.c` is now its own translation unit compiled to a `.o` and linked, instead of `.c` files
  being `#include`d into `Sample.c`. Motivation: under the unity build only `Sample.c` appeared in
  `compile_commands.json`, so clangd had no compile command for the other `.c` files and couldn't
  resolve their includes/types (e.g. `ff.h`, project types, `SHOWDEBUG` all flagged red in
  `PNGHelper.c`). Changes:
  - New shared header `src/lib/Types.h` holds the `UBYTE`/`UWORD`/`UDOUBLE` aliases (were `#define`
    macros inside each `HAL.c`); added `src/lib` to the include path on both platforms.
  - New public headers with prototypes: `HAL.h` (RP2040 + ESP32), `Driver.h`, `FileHelper.h`,
    `PNGHelper.h`. `Canvas.h`/`LCD_1in28.h` gained their prototypes + `extern` for the `canvas` /
    `LCD` globals.
  - `RTC.h` split into `RTC.h` (prototype) + `RTC.c` (definitions of `RTCInitialize()` + `get_fattime()`)
    so `get_fattime()` has exactly one definition (was defined in the header → would duplicate
    across TUs).
  - `Driver.c`: the helpers called from other TUs (`DriverHardwareReset`, `DriverSendCommand`,
    `DriverSendData8Bit`, `DriverSendCommandData8Bit`, `DriverSendData16Bit`) lost `static`;
    `slice_num` is now `static UDOUBLE` (file-local). All `#include "X.c"` were replaced by
    `#include "X.h"` in `Sample.c`, `PNGHelper.c`, `LCD_1in28.c`, `Canvas.c`, `FileHelper.c`.
  - Build: root `CMakeLists.txt` now lists the common TUs in `GUILL_COMMON_SRCS`; RP2040's
    `add_executable` adds them + `RP2040/HAL.c` + `RP2040/RTC.c`; ESP32's `idf_component_register
    SRCS` lists the same common TUs + `ESP32/HAL.c` + `ESP32/RTC.c`, with `src/lib` added to
    `INCLUDE_DIRS`. RP2040 **builds and links**; `compile_commands.json` now has an entry per `.c`
    and clangd reports no diagnostics on `PNGHelper.c`/`Sample.c`/`Driver.c`/`Canvas.c`. ESP32
    changes are **parity-only, not compile-tested** (no ESP32 build configured locally).
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
- SD pins / shared-shield design, LCD pin defines, `EPD_*`→`LCD_*`, backlight in `DriverInitialize`,
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

5. **Separate compilation (each `.c` is its own translation unit)**: The project compiles every
   `.c` independently to a `.o` and links them. Each module is a `.c`/`.h` pair; the header
   declares the module's public prototypes, types and `extern` globals (see §5). Add new `.c`
   files to the build source list (RP2040: `add_executable` via `GUILL_COMMON_SRCS` +
   platform sources; ESP32: `idf_component_register SRCS`). Shared scalar aliases
   (`UBYTE`/`UWORD`/`UDOUBLE`) live in `src/lib/Types.h`.

   > Superseded approach: the project originally used a **unity build** — `.c` files were
   > `#include`d directly (`#include "HAL.c"`) so the whole program was one translation unit.
   > It was abandoned because only `Sample.c` ended up in `compile_commands.json`, so clangd
   > couldn't resolve includes/types in any other `.c` (no IDE intelligence). The migration to
   > separate compilation fixed that; do not reintroduce the `#include "X.c"` pattern.

6. **LCD pin ownership**: All LCD GPIO definitions (`LCD_DC_PIN`, `LCD_CS_PIN`, `LCD_CLK_PIN`,
   `LCD_MOSI_PIN`, `LCD_RST_PIN`, `LCD_BL_PIN`) live exclusively in each platform's `HALConfig.h`.
   The `Driver.c` reads them via `#include "HALConfig.h"` — no pin numbers hardcoded in driver code.
   Prefix is `LCD_*` (not `EPD_*`). Backlight PWM is initialized inside `DriverInitialize` — never in
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

13. **PNG compositing into RAM via `CanvasDrawPng` (flicker-free render)**: Rendering a PNG
    straight to the panel with `LCDRenderPng` updates the screen row-by-row with inflate latency
    between rows, producing a visible top-to-bottom scan/flicker. `CanvasDrawPng` (in
    `GUI/Canvas.c`) decodes the PNG into the in-RAM canvas texture buffer via `CanvasSetPixel`
    instead, so a single `LCDRenderTexture` blit shows the finished image smoothly (and text/other
    drawing can be composited over the same buffer first). Design rules that MUST be preserved:
    - **Faithful copy of `LCDRenderPng`**: the libpng decode pipeline, error handling
      (`setjmp`/`longjmp`, `volatile png_bytep rowPointers`), palette/non-palette extraction, and
      RGB565 bit math are reproduced verbatim (including `SHOWDEBUG` traces/comments for retained
      steps). The *only* intended differences are the two below.
    - **Pixel destination**: the two `SPIWriteByte` calls are replaced by one
      `CanvasSetPixel(col, row, color)`. The `UWORD` is packed with the identical high/low byte
      layout `CanvasSetPixel` (scale 65) stores big-endian — so buffer bytes are bit-for-bit what
      the panel path would have streamed. The packing is host-endianness-independent integer math,
      giving identical buffer contents on RP2040 and ESP32.
    - **No panel ops**: omit `LCDSetDisplayArea`, DC/CS `DigitalWrite`, the `lcdSelected` tracking
      (and its CS restore in the error path), and the final `DriverSendCommand(0x29)`. The function
      never touches the panel and has no platform `#ifdef`.
    - **Clip to the canvas, not the panel**: `maxCol`/`maxRow` clamp to `canvas.Width`/
      `canvas.Height` (not `LCD.WIDTH`/`LCD.HEIGHT`). Caller must configure the canvas
      (`CanvasSetScale(65)`, dims = `LCD.WIDTH`/`LCD.HEIGHT`) **before** calling.
    - **Private helpers stay `static`**: `Canvas.c` has its own file-local `PngCustomReadData` /
      `PngShowError` (copies of the `LCDRenderer.c` versions). Keeping them `static` avoids a link
      clash with the non-static `PngCustomReadData` in `LCDRenderer.c` (different TUs/linkage —
      well-defined in C); do NOT promote either to a shared public symbol.
    - **`LCDRenderPng` stays byte-for-byte unchanged** — the direct-to-panel path must keep working.
      No build-system changes were needed (Canvas.c already builds on both targets; libpng/FatFS
      already linked). `Canvas.h` includes `"ff.h"` for the `FIL *` parameter type, mirroring
      `LCDRenderer.h`.
    - Sub-area drawing (`CanvasDrawPngArea`) is intentionally **deferred** to a future spec.

14. **Transparency via color-key (`TRANSPARENT`) — text/overlays over an image**: Once a PNG (or
    any content) is composited into the canvas buffer (Decision 13), we can draw text/shapes on
    top so that only the drawn pixels overwrite the image and the rest "shows through". RGB565 has
    no alpha channel, so this is done with a **color key** (chroma key), not real alpha:
    - **`TRANSPARENT` is a sentinel color**, defined in `Canvas.h` as `RGB_COLOR(255, 0, 255)` —
      magenta, `0xF81F`. It is the classic chroma-key color and equals `MAGENTA`; the trade-off is
      that **magenta cannot be used as a real drawable color** when relying on transparency.
    - **How it works**: `CanvasDrawChar` / `CanvasDrawTextCN` check `if (TRANSPARENT ==
      backgroundColor)`. When the caller passes `TRANSPARENT` as the background, only the glyph
      (foreground) pixels are written via `CanvasSetPixel`; the non-glyph pixels are skipped, so
      whatever is already in the buffer (e.g. the PNG) stays visible behind the text. Passing any
      other `backgroundColor` paints the glyph box opaquely as before. This replaced the old
      `FONT_BACKGROUND`-equals check.
    - **`RGB_COLOR(r, g, b)` MUST cast to `UWORD` (unsigned), not `short`**. RGB565 values above
      `0x7FFF` (red/magenta/white, etc.) are negative as a signed `short`; comparing a signed
      `short` constant against a `UWORD` (e.g. `TRANSPARENT == backgroundColor`) is *always false*
      after integer promotion (clang warns: "comparison … always false"), silently breaking the
      transparency check. The unsigned cast is what makes the color key actually match. All color
      macros (`WHITE`, `RED`, `TRANSPARENT`, …) go through `RGB_COLOR`, so they are all `UWORD`.
    - **At render time there is still no panel-level transparency**: `LCDRenderTexture` blits the
      whole buffer. The "transparency" is purely a *compositing-in-RAM* effect — only meaningful
      because the buffer already holds the background. If you ever want to overlay onto an image
      that is already on the panel *without* keeping it in RAM, that needs a different mechanism
      (per-pixel/run color-key skipping at blit time, or reading back the panel). That is the path
      we will need for **partial screen updates** (drawing a small changed region over an image
      already shown) — keep `TRANSPARENT` and `RGB_COLOR`'s unsigned semantics in mind when we
      build it, and a future `LCDRenderTexture`/`...InArea` color-key variant would skip
      `TRANSPARENT` pixels instead of streaming the full buffer.

15. **Integer-only curved-character rendering (`CanvasDrawCurvedChar`)**: The primitive that draws a
    rotated character on a circle (see Recent work) is built to be **cross-platform deterministic** —
    identical inputs must produce a bit-for-bit identical set of written pixels on RP2040 (software
    float) and ESP32-S3 (hardware FPU). Rules that MUST be preserved:
    - **No runtime floating point.** Calling `sinf`/`cosf` at runtime risks last-ULP differences
      between the two libm implementations that occasionally round to a different integer pixel.
      Instead, trigonometry is read from **Q16.16** integer lookup tables exposed by the
      `Helper/Trigonometry` module (`TrigCosQ16(UWORD deg)` / `TrigSinQ16(UWORD deg)`, normalized
      mod 360; `TRIG_Q16_SCALE = 1 << 16`). The tables are `static const` literals generated offline
      and embedded in `Trigonometry.c` (~2.8 KB rodata, one copy), so both targets link identical
      integers; do NOT regenerate them with `libm` at runtime. (The tables originally lived inside
      `Canvas.c`; they were extracted to `Helper/Trigonometry` since integer trig is generic and not
      Canvas-specific.)
    - **Tangent orientation (placement angle + 90).** The anchor is placed using the placement
      angle's `cosV`/`sinV`, but the glyph is rotated using a **separate** orientation angle
      `orientAngle = (angle + 90) % 360` (its own `cosR`/`sinR` from the same LUT). This aligns the
      glyph advance axis with the circle tangent `(-sin, cos)` so text reads along the rim; rotating
      by the placement angle alone would point letters radially out of the center (a real bug, fixed).
      Consequence: upright text is at `startAngle == 270` (top, horizontal tangent), not 0.
    - **Integer rotation with a single rounding step.** `CanvasRoundDivAway(int64_t, int32_t)` does
      round-half-away-from-zero division (64-bit numerator to avoid overflow on `radius * scaledTrig`
      and the doubled products). `CanvasRotateGlyphOffset` rotates a **doubled-delta** pair
      (`dx2 = 2*Column-(Width-1)`, `dy2 = 2*Page-(Height-1)`) by the supplied (tangent) trig and folds
      the `/2` (for the doubling) and `/TRIG_Q16_SCALE` (Q16.16) into one
      `CanvasRoundDivAway(.., 2*TRIG_Q16_SCALE)` call. Doubling makes the half-pixel box center exact
      in integers (Req 3.3).
    - **Signed-before-cast bounds check.** `CanvasSetPixel` takes `UWORD`; a negative rotated
      coordinate would wrap to a huge value and bypass the upper-bound clip. So `targetX`/`targetY`
      are `int32_t` and any negative component is skipped **before** the `UWORD` cast (Req 6.3). All
      writes still go through `CanvasSetPixel`, so its clip/rotate/flip and `WidthMemory`/`HeightMemory`
      guards apply.
    - **Forward (source-driven) mapping**, mirroring `CanvasDrawChar`'s glyph iteration, so the
      `startAngle == 270` case is (almost) equivalent to `CanvasDrawChar` and the opaque "rotated
      bounding box" is exactly the forward-mapped `Width x Height` set (Req 5.3). Known tradeoff:
      nearest-neighbor aliasing can leave 1-px gaps / double-maps at some angles — acceptable by
      definition here; a destination-driven inverse map is deferred unless visual quality requires it.
    - **Even-dimension seam (accepted relaxation).** At the upright angle (`startAngle == 270`),
      round-half-away rotation about a half-integer center (even Width/Height) skips the center axis,
      so the glyph differs from upright `CanvasDrawChar` by at most 1 px along that seam. Req 3.4 was
      relaxed to allow this ±1 tolerance rather than special-casing it (which would diverge from the
      pure-rotation model). All bundled fonts have even Height.
    - **Reuse / extension point**: the signature leads with `ASCIIChar` then the geometric placement
      values so a text helper can hold center/radius/font/colors constant and advance only
      `startAngle` per glyph. `CanvasDrawCurvedText` is **implemented** on exactly this pattern (see
      Recent work): step ≈ glyphAdvance / radius converted to whole degrees with integer math.
      `CanvasDrawCurvedNumber` is still out of scope.
    - **Two pre-existing Canvas quirks to keep in mind** (relevant when reasoning about scale-65
      pixel writes): production `CanvasClear` at scale 65 iterates `x` over `WidthByte` (the byte
      pitch) while addressing `x*2`, so it overruns the image region by ~one row; and `CanvasSetPixel`
      clips with `>` (not `>=`), so a coordinate equal to `canvas.Width`/`Height` is still written.
      `CanvasDrawCurvedChar` itself relies only on `CanvasSetPixel`'s clipping and is unaffected, but
      these are worth knowing before changing the buffer layout.
