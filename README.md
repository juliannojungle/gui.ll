# gui.ll (GUI Low Level)

> ⚠️ **This project is under active development. The documentation is growing along the project as it's a work-in-progress.**

A lightweight, bare-metal GUI library for embedded systems. Draws visual elements directly on a GC9A01 round LCD (240×240) and renders PNG images streamed from an SD card — all with minimal memory footprint, no OS, no heap allocations in the critical path.

Currently supports **RP2040** (Raspberry Pi Pico) and **ESP32** from a single codebase.

---

## ✨ Features

- 🎨 **Direct LCD rendering** — pixels, lines, circles, rectangles, and text written straight to the display without a framebuffer
- 🖼️ **PNG from SD card** — PNG files decoded on-the-fly from an SD card and streamed directly to the LCD, row by row
- 🔌 **Multi-platform** — same application code compiles for RP2040 and ESP32; platform differences are fully encapsulated in the HAL layer
- ⚡ **Bare-metal** — no RTOS, no dynamic memory allocator required, runs on the metal
- 📦 **Self-contained** — zlib, libpng, and FatFS are bundled as submodules; no package manager needed
- 🔧 **CMake-native** — integrates cleanly as a CMake component into any parent project

---

## 🖥️ Supported Platforms

| Platform | RP2040 | ESP32 |
|---|---|---|
| Pinout |<img alt="RP2040 LCD 1.28" src="Documentation\Image\RP2040_LCD_1_28.png" width="300px">|<img alt="ESP32-S3 LCD 1.28" src="Documentation\Image\ESP32_S3_LCD_1_28.png" width="300px">|
| Device | [RP2040-LCD-1.28](https://www.waveshare.com/wiki/RP2040-LCD-1.28) | [ESP32-S3-LCD-1.28](https://www.waveshare.com/wiki/ESP32-S3-LCD-1.28) |
| Toolchain | arm-none-eabi-gcc + Pico SDK | xtensa-esp32-elf-gcc + ESP-IDF |
| Output | `.uf2` | `.bin` |

---

## 🛒 B.O.M. (Bill Of Materials)

* One of the target devices:
  * [RP2040-LCD-1.28](https://www.waveshare.com/wiki/RP2040-LCD-1.28)
  * [ESP32-S3-LCD-1.28](https://www.waveshare.com/wiki/ESP32-S3-LCD-1.28)
* MicroSD to SD card adapter (for easy soldering)
* MicroSD card
* Some nice wires (30 AWG recommended)
* 2 x 10K resistors (for pull-up)
* 1 x 10uF capacitor (for decoupling)
* *(Optional)* 2 x [1.27mm 2x20Pin header](https://ae01.alicdn.com/kf/H874df6c6359144e497abad3f5e2a03c6b.jpg_220x220.jpg) (for connecting SD to target device)

---

## 🔌 Wiring

> The below table shows connections in a way that you can create a header connector/shield board to swap between the target devices.\
> ⚠️ *Connect it to the RP2040 **or** to the ESP32. You're **NOT** supposed to wire RP2040 and ESP32 together.*

| RP2040     | ESP32       | R1(10K) | R2(10K) | C1(10uF) | SDCARD           | LCD*       |
|------------|-------------|---------|---------|----------|------------------|------------|
| GP0 (MISO) | GP46 (MISO) |         |  MISO   |          | pin7 (DAT0/MISO) |            |
| GP1 (CS)   | GP45 (CS)   |   CS    |         |          | pin1 (DAT3/CS)   |            |
| GP2 (SCK)  | GP42 (SCK)  |         |         |          | pin5 (CLK/SCK)   |            |
| GP3 (MOSI) | GP41 (MOSI) |         |         |          | pin2 (CMD/MOSI)  |            |
| GP5        | GP39        |         |         |          | pin3 (VSS1/GND)  |            |
| GND        | GND         |         |         |   GND    | pin6 (VSS2/GND)  | GND        |
| 3V3        | 3V3         |   3V3   |  3V3    |   3V3    | pin4 (VDD/3V3)   | VCC (3V3)  |
| GP8        | GP8         |         |         |          |                  | DC         |
| GP9        | GP9         |         |         |          |                  | CS         |
| GP10       | GP10        |         |         |          |                  | CLK        |
| GP11       | GP11        |         |         |          |                  | DIN (MOSI) |
| GP12       | GP12        |         |         |          |                  | RST        |
| GP25       | GP40        |         |         |          |                  | BL         |

<sup>* The LCD connection is internally made on these devices, GP pins just for reference.</sup>

---

## 📚 LCD Driver Datasheet (reference)

The GC9A01A LCD driver datasheet is versioned in `Documentation/` in two complementary forms:

| File | Role |
|---|---|
| [`Documentation/GC9A01A.md`](Documentation/GC9A01A.md) | **Primary / quick data source.** Markdown conversion (mostly used for quick ai insights) — search here first for registers, command opcodes, parameter bit fields, defaults, memory layout and timings. Section 6 (commands) is a faithful reproduction with bit-layout tables and verbatim Description/Restriction. |
| [`Documentation/GC9A01A.pdf`](Documentation/GC9A01A.pdf) | **Secondary source for disambiguation.** The original 192-page PDF — consult it to resolve any doubt, and for graphical waveform/timing diagrams (not extracted into the markdown). |

> Figures and diagrams are described in text in the markdown, not embedded as images. When
> precision matters, the PDF is the ground truth.

---

## 📋 Software Requirements

- **Host**: Linux or Windows 10 PC with [WSL 2 (Windows Subsystem Linux)](https://learn.microsoft.com/pt-br/windows/wsl/install) - Ubuntu is recommended — code edited on Windows, compiled inside WSL
- **IDE**: [Kiro](https://kiro.dev) or [VSCode](https://code.visualstudio.com/)
- **RP2040**: arm-none-eabi-gcc, Pico SDK (installed via `Setup: RP2040 toolchain`)
- **ESP32**: ESP-IDF ≥ 5.x, xtensa toolchain, espflash (installed via `Setup: ESP32 toolchain`)
- **CMake** ≥ 3.16

> The preferred IDE is **[Kiro](https://kiro.dev)**, which provides agent-assisted development with project-aware context via `AGENTS.md`. The project also works fully with **VS Code** — all tasks and settings are configured in `.vscode/`.

---

## 🏗️ Architecture

```
gui.ll/
├── src/
│   ├── Sample.c                    # Entry point — app_entry()
│   ├── lib/
│   │   ├── Helper/
│   │   │   ├── FileHelper.c        # SD card mount / open / close (FatFS)
│   │   │   └── Trigonometry.c      # Integer Q16.16 cos/sin LUT (used by curved text)
│   │   ├── Platform/
│   │   │   ├── RP2040/             # HAL, SPI, DiskIO, RTC (Pico SDK)
│   │   │   └── ESP32/              # HAL, SPI, DiskIO, RTC (ESP-IDF)
│   │   ├── Driver/GC9A01/          # LCD driver
│   │   ├── LCD/1in28/              # Display layer: LCDSetup (init) + LCDRenderer (blit)
│   │   ├── GUI/                    # Canvas / drawing primitives (Canvas.c/.h)
│   │   └── Fonts/                  # Bitmap font data
│   └── Dependency/
│       ├── fatfs/                  # ChaN FatFS
│       ├── libpng/                 # libpng
│       └── zlib/                   # zlib (libpng dependency)
├── Documentation/
│   ├── GC9A01A.pdf                 # LCD driver datasheet (original PDF)
│   ├── GC9A01A.md                  # Datasheet as markdown (primary data source)
│   └── Image/                      # Pinout reference images
└── Toolchain/
    ├── RP2040/Setup.sh
    └── ESP32/Setup.sh
```

### Platform abstraction

All hardware-specific code lives under `src/lib/Platform/<PLATFORM>/`. The application layer (`FileHelper`, `Sample.c`) calls abstract functions — `DigitalWrite`, `SPIWriteByte`, `Delay`, etc. — defined in each platform's `HAL.c`. CMake `include_directories` points to the active platform folder at build time, so `#include "HAL.h"` resolves to the correct implementation with no `#ifdef` scattered through application code.

### Entry point

```c
void app_entry(void) { /* your application code */ }

#ifdef ESP_PLATFORM
void app_main(void) { app_entry(); }  // ESP-IDF entry
#else
int main(void) { app_entry(); return 0; }  // RP2040 / standard entry
#endif
```

---

## 🎨 Drawing Primitives

The `Canvas` module writes directly to the LCD without requiring a full framebuffer in RAM.

```c
// Canvas setup
CanvasNewImage(buffer, 240, 240, ROTATE_0);
CanvasClear(BLACK);

// Geometry
CanvasDrawPoint(120, 120, RED, PIXEL_SIZE_2X2, PIXEL_FILL_STYLE_AROUND);
CanvasDrawLine(0, 0, 240, 240, WHITE, PIXEL_SIZE_1X1, LINE_STYLE_SOLID);
CanvasDrawRectangle(10, 10, 100, 80, BLUE, PIXEL_SIZE_1X1, DRAW_FILL_STYLE_EMPTY);
CanvasDrawCircle(120, 120, 60, GREEN, PIXEL_SIZE_1X1, DRAW_FILL_STYLE_FULL);

// Text
CanvasDrawText(20, 20, "Hello!", &Font16, WHITE, BLACK);
CanvasDrawNum(20, 50, 3.14, &Font12, 2, YELLOW, BLACK);
CanvasDrawTime(10, 200, &dateTime, &Font12, WHITE, BLACK);

// Curved text — placed on a circle (radius, startAngle in whole degrees, 0° = 3 o'clock,
// increasing clockwise) with the glyph kept tangent to the border. A single character or a
// whole string (which advances the angle per glyph automatically):
CanvasDrawCurvedChar('A', 120, 120, 100, 270, TEXT_ORIENTATION_INWARDS, &Font20, WHITE, TRANSPARENT);
CanvasDrawCurvedText("Hello, World!", 120, 120, 100, 250, TEXT_ORIENTATION_OUTWARDS, &Font20, WHITE, TRANSPARENT);
```

Available colors: `WHITE`, `BLACK`, `RED`, `GREEN`, `BLUE`, `YELLOW`, `CYAN`, `MAGENTA`, `GRAY`, and more.
Pixel sizes: `PIXEL_SIZE_1X1` through `PIXEL_SIZE_8X8`.
Line styles: `LINE_STYLE_SOLID`, `LINE_STYLE_DOTTED`.
Fill modes: `DRAW_FILL_STYLE_EMPTY`, `DRAW_FILL_STYLE_FULL`.
Curved-text orientation: `TEXT_ORIENTATION_INWARDS` (advances clockwise) or `TEXT_ORIENTATION_OUTWARDS` (glyph flipped, advances counter-clockwise) — ideal for labels following the round display's rim. Pass `TRANSPARENT` as the background to draw only the glyph pixels over an existing image.

---

## 🖼️ PNG Rendering from SD Card

PNG files are decoded with libpng into the canvas RAM buffer, then blitted to the display in a single
transfer — so the image appears instantly without visible row-by-row scanning. Memory usage is bounded
to a single decoded row at a time; the full image never needs to fit in RAM beyond the canvas texture.

```c
UINT8 texture[240 * 240 * 2]; // RGB565 framebuffer
FIL file;

CanvasNewImage(texture, 240, 240, ROTATE_0);
CanvasSetScale(65);

if (MountSdCard()
    && SelectActiveDrive()
    && OpenFile(&file, "image.png")) {
    CanvasDrawPng(&file);
    CloseFile(&file);
}

UnMountSdCard();

// Optionally draw text/shapes over the decoded image before blitting:
CanvasDrawText(20, 20, "Hello!", &Font16, WHITE, TRANSPARENT);

LCDRenderTexture(texture);
```

---

## 📐 Using gui.ll in Your Own Project

The `CMakeLists.txt` at the root is a functional standalone example. To integrate gui.ll as a component in your own CMake project, include the platform-specific fragments directly:

```cmake
set(PLATFORM_NAME "RP2040")  # or "ESP32"

# RP2040 only — include before add_executable
include(path/to/gui.ll/src/lib/Platform/RP2040/PreExecutable.cmake)

add_executable(my_app src/main.c)

# RP2040 only — include after add_executable
include(path/to/gui.ll/src/lib/Platform/RP2040/PostExecutable.cmake)
```

For ESP32, set `EXTRA_COMPONENT_DIRS` to point at `src/lib/Platform/ESP32` before calling `project()`:

```cmake
set(EXTRA_COMPONENT_DIRS "path/to/gui.ll/src/lib/Platform/ESP32")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(my_app)
```

Your application entry point mirrors `Sample.c` — implement `app_entry()` and let the platform macro route `main` or `app_main` automatically.

---

## 🚀 Getting Started

### 1. Clone with submodules

```bash
git clone --recurse-submodules --shallow-submodules https://github.com/juliannojungle/gui.ll.git
```

This single command clones the repository and all bundled dependencies (FatFS, libpng, zlib) in one step. No separate dependency installation needed.

### 2. Set up the toolchain

Before building for the first time, install the required toolchain for your target platform. The recommended way is via the built-in tasks.

**In Kiro or VS Code** — open the Command Palette (`Ctrl+Shift+P`) and select **Tasks: Run Task**, then choose:

- `Setup: RP2040 toolchain in WSL` — installs arm-none-eabi-gcc and Pico SDK
- `Setup: ESP32 toolchain in WSL` — installs ESP-IDF, xtensa toolchain, Rust, and espflash

These scripts are idempotent — safe to run again if you need to repair an installation.

### 3. Build

Once the toolchain is ready, build using the tasks:

**In Kiro or VS Code** — open the Command Palette (`Ctrl+Shift+P`) → **Tasks: Run Task**:

| Task | Description |
|---|---|
| `Build: Full RP2040` | Clean CMake configure + make for RP2040 |
| `Build: Full ESP32` | Clean idf.py build for ESP32 |
| `Build: Incremental` | Auto-detects platform, runs `make` or `ninja` |
| `Copy UF2 to Windows` | Copies `.uf2` to `C:\temp` for drag-and-drop flashing |

Alternatively, trigger the default build task directly with `Ctrl+Shift+B` (runs **Build: Incremental**).

---

## 📄 License

See [LICENSE](LICENSE).

---
<sup>[@juliannojungle](https://github.com/juliannojungle), 2026</sup>
