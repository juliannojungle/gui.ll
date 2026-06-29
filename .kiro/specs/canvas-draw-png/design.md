# Design Document

## Overview

`CanvasDrawPng` is a new drawing-layer function added to the Canvas module
(`src/lib/GUI/Canvas.c`, prototype in `src/lib/GUI/Canvas.h`). It decodes a PNG from an open
FatFS file handle and composites the result into the in-RAM canvas buffer (`canvas.Image`) via
`CanvasSetPixel`, instead of streaming the decoded pixels straight to the LCD panel over SPI.

The function is a faithful, step-by-step copy of the existing `LCDRenderPng`
(`src/lib/LCD/1in28/LCDRenderer.c`). The libpng decode pipeline, error handling, palette handling
and RGB565 bit math are reproduced unchanged. The **only** behavioral differences are:

1. **Pixel destination** — the two `SPIWriteByte` calls that push the high/low RGB565 bytes to the
   panel are replaced by a single `CanvasSetPixel(col, row, value)` call writing the same 16-bit
   value into the canvas buffer.
2. **No panel operations** — `LCDSetDisplayArea`, the `DigitalWrite(LCD_DC_PIN/LCD_CS_PIN, …)`
   chip-select toggling, the `lcdSelected` chip-select tracking, and the final
   `DriverSendCommand(0x29)` display-on command are removed, since the function never touches the
   panel.
3. **Clip bounds** — clipping is computed against the canvas dimensions (`canvas.Width` /
   `canvas.Height`) rather than the panel dimensions (`LCD.WIDTH` / `LCD.HEIGHT`). This is a
   deliberate, documented deviation (Requirement 6).

Intended caller flow (in `Sample.c`):

```c
CanvasNewImage((UINT8 *)texture, LCD.WIDTH, LCD.HEIGHT, ROTATE_0, BLACK);
CanvasSetScale(65);
if (MountSdCard() && SelectActiveDrive() && OpenFile(&file, "01.png")) {
    CanvasDrawPng(&file);   /* decode into the texture buffer */
    CloseFile(&file);
}
UnMountSdCard();
CanvasDrawText(30, 110, "Hello, World!", &Font20, WHITE, BLACK);
LCDRenderTexture(texture);  /* single, flicker-free bulk blit */
```

The motivation is flicker-free rendering: `LCDRenderPng` updates the panel row-by-row with inflate
latency between rows (visible scan). Decoding into RAM first, then doing a single
`LCDRenderTexture` blit, produces a smooth update.

Out of scope: `CanvasDrawPngArea` (sub-area drawing) is deferred to a future spec.

## Architecture

```
                Sample.c (app_entry)
                      │  CanvasDrawPng(&file)
                      ▼
   ┌─────────────────────────────────────────────┐
   │ Canvas module  (src/lib/GUI/Canvas.c/.h)     │
   │                                              │
   │  CanvasDrawPng(FIL *file)                    │
   │    ├─ libpng decode pipeline (== LCDRenderPng)│
   │    ├─ per pixel: build RGB565                 │
   │    └─ CanvasSetPixel(col, row, rgb565) ───────┼──► canvas.Image (RAM buffer)
   │                                              │
   │  (private) PngCustomReadData / PngShowError   │
   └─────────────────────────────────────────────┘
                      │
                      ▼
        libpng  ◄── f_read ──►  FatFS (FIL *)
```

`CanvasDrawPng` lives entirely in the platform-agnostic Canvas module (no `#ifdef`), so it builds
identically for RP2040 and ESP32 (Requirement 10). The existing `LCDRenderPng` in
`LCDRenderer.c` is left completely untouched (Requirement 9): the direct-to-panel path keeps
working exactly as before, and the new RAM-compositing path is independent.

## Components and Interfaces

### 1. Public function and prototype

Definition in `Canvas.c`:

```c
void CanvasDrawPng(FIL *file);
```

Prototype added to `Canvas.h`, alongside the other `Canvas*` drawing-API prototypes (e.g. next to
`CanvasDrawImage` / `CanvasDrawBitmap`):

```c
void CanvasDrawPng(FIL *file);
```

The name follows the project convention (PascalCase, `Canvas` module prefix) — Requirement 1.

### 2. New includes in Canvas.c and Canvas.h

`Canvas.c` currently includes only `Canvas.h`, `<stdint.h>`, `<stdlib.h>`, `<string.h>`. To host
the PNG decode it must additionally include:

- `<png.h>` — libpng API (`png_*` functions/types).
- `"ff.h"` — the FatFS `FIL` type used by the parameter and by the read callback (`f_read`).
- `"Debug.h"` — the `SHOWDEBUG` macro used by the replicated traces (Requirement 11).

`Canvas.h` gains the `CanvasDrawPng` prototype, which references `FIL *`. Therefore `Canvas.h` must
make the `FIL` type visible. **Decision: include `"ff.h"` in `Canvas.h`.** Rationale:

- `FIL` is a `typedef struct { … } FIL;` in `ff.h`; it cannot be cleanly forward-declared as a
  named typedef without redefining it (a redefinition error against `ff.h`). A pointer-only use
  still needs the typedef name in scope.
- There is precedent: `LCDRenderer.h` already includes `"ff.h"` for the same reason
  (`LCDRenderPng(FIL *file)`).

This pulls FatFS declarations into any translation unit that includes `Canvas.h`. That is
acceptable and consistent with the existing renderer header; no new linkage is introduced.

### 3. PNG helper functions (read callback + error handler) — design decision

`LCDRenderPng` relies on two helpers that today live in `LCDRenderer.c`:

- `PngCustomReadData(png_structrp, png_bytep, size_t)` — the libpng read callback; **non-static**
  in `LCDRenderer.c` (external linkage), but **not** declared in `LCDRenderer.h`.
- `PngShowError(png_structp, const char *)` — the libpng error handler; **static** (file-local) in
  `LCDRenderer.c`.

`CanvasDrawPng` needs equivalents. Two options were considered:

| Option | Approach | Trade-off |
|--------|----------|-----------|
| (a) **Duplicate as private statics in Canvas.c** | Define `static` `PngCustomReadData` and `PngShowError` inside `Canvas.c`, identical to the originals. | Tiny code duplication (two short functions). Keeps `LCDRenderer.c` untouched (Req 9). Follows §5 (private helpers are `static`). No shared/public symbol for an internal concern. |
| (b) **Share via a header** | Promote both to non-static, declare them in a header, include it from `Canvas.c`. | Avoids duplication, but forces `PngShowError` to lose `static` and exposes two helpers as public symbols — touching `LCDRenderer.c` and contradicting §5 (these are implementation details, not module API). |

**Decision: Option (a).** Define both helpers as `static` inside `Canvas.c` with the same names and
bodies as in `LCDRenderer.c`. This satisfies the "faithful copy" intent (Requirement 2), the
"leave `LCDRenderPng` unchanged" requirement (Requirement 9), and the §5 discipline (private
helpers stay `static`, not in the header).

> Linkage note: `Canvas.c`'s `static PngCustomReadData` (internal linkage) and `LCDRenderer.c`'s
> non-static `PngCustomReadData` (external linkage) live in different translation units and refer
> to different entities — this is well-defined in C (§6.2.2 only forbids mixing internal/external
> linkage for the same identifier *within one* translation unit). No link-time conflict arises.

Replicated helper bodies in `Canvas.c`:

```c
static void PngCustomReadData(png_structrp pngPointer, png_bytep data, size_t length) {
    SHOWDEBUG(".");
    UINT bytesRead;
    f_read((FIL*)png_get_io_ptr(pngPointer), data, length, &bytesRead);
}

static void PngShowError(png_structp pngPointer, const char *message) {
    SHOWDEBUG("Error from libpng: %s\n", message);
}
```

## Detailed Design

### Step-by-step mapping: LCDRenderPng → CanvasDrawPng

The table below maps every step of `LCDRenderPng` to its `CanvasDrawPng` counterpart. "Same"
means reproduced verbatim (including `SHOWDEBUG` traces and comments, per Requirement 11).

| `LCDRenderPng` step | `CanvasDrawPng` | Req |
|---------------------|-----------------|-----|
| `png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, PngShowError, NULL)` + null check → return | Same | 2.1, 7.1 |
| `png_create_info_struct` + null check → `png_destroy_read_struct(&p, NULL, NULL)`; return | Same | 2.2, 7.2 |
| `png_set_read_fn(pngPointer, file, PngCustomReadData)` | Same | 2.3 |
| `volatile png_bytep rowPointers = NULL;` | Same | 7.4 |
| `volatile bool lcdSelected = false;` | **Removed** (no chip-select state) | 4.4 |
| `setjmp(png_jmpbuf(pngPointer))` error block: `png_free(rowPointers)` if non-null; **`if (lcdSelected) DigitalWrite(LCD_CS_PIN, 1)`**; `png_destroy_read_struct(&p,&i,NULL)`; return | Same **minus** the CS restoration line | 7.3, 7.5, 4.6 |
| `png_read_info` | Same | 2.4 |
| `png_get_IHDR(...)` (width, height, bitDepth, colorType, interlaceType) | Same | 2.4 |
| `maxCol = width > LCD.WIDTH ? LCD.WIDTH : width;` | `maxCol = width > canvas.Width ? canvas.Width : width;` | 6.1, 6.3 |
| `maxRow = height > LCD.HEIGHT ? LCD.HEIGHT : height;` | `maxRow = height > canvas.Height ? canvas.Height : height;` | 6.2, 6.4 |
| `LCDSetDisplayArea(0,0,maxCol,maxRow)` | **Removed** | 4.1 |
| `DigitalWrite(LCD_DC_PIN,1); DigitalWrite(LCD_CS_PIN,0); lcdSelected=true;` | **Removed** | 4.2, 4.3, 4.4 |
| `png_get_PLTE` when `colorType == PNG_COLOR_TYPE_PALETTE` | Same (read palette once before the row loop) | 5.1 |
| Row loop: `png_malloc(png_get_rowbytes)`, `rowPointers = rowBuffer`, `png_read_rows(...,1)` | Same | 2.5 |
| Per-pixel palette / non-palette / null-palette extraction | Same | 5.2, 5.3, 5.4 |
| Two `SPIWriteByte(...)` building RGB565 | **Replaced** by `CanvasSetPixel(col, row, rgb565)` | 3.1, 3.2, 3.3 |
| `png_free(rowBuffer); rowPointers = NULL;` | Same | 2.6 |
| `DigitalWrite(LCD_CS_PIN,1); lcdSelected=false;` + "LCD SPI released" trace | **Removed** | 4.3, 4.4, 11.3 |
| `DriverSendCommand(0x29)` | **Removed** | 4.5 |
| `png_destroy_read_struct(&pngPointer, &infoPointer, NULL)` | Same | 8.1 |

### RGB565 value construction and byte order (Requirement 3, 10)

`LCDRenderPng` sends two bytes per pixel:

```c
SPIWriteByte((red & 0b11111000) | ((green & 0b11100000) >> 5));        /* high byte */
SPIWriteByte(((green & 0b00011100) << 3) | ((blue & 0b11111000) >> 3)); /* low  byte */
```

`CanvasSetPixel` under scale 65 stores a 16-bit `color` big-endian (high byte first):

```c
canvas.Image[Addr]     = 0xff & (color >> 8);   /* high byte */
canvas.Image[Addr + 1] = 0xff & color;          /* low  byte */
```

So to write the **identical two bytes** into the buffer, `CanvasDrawPng` packs the same high/low
bytes into a single `UINT16` and passes it to `CanvasSetPixel`:

```c
UINT16 color = (UINT16)(((red & 0b11111000) | ((green & 0b11100000) >> 5)) << 8)
            | (UINT16)(((green & 0b00011100) << 3) | ((blue & 0b11111000) >> 3));
CanvasSetPixel(col, row, color);
```

This guarantees the bytes deposited in `canvas.Image` are bit-for-bit what `LCDRenderPng` would
have streamed, and exactly what `LCDRenderTexture` later blits — so the composited image is
correct. The packing is pure integer math (no endianness assumptions on the build host), giving
identical buffer contents on RP2040 and ESP32 (Requirement 10.3).

> Coordinate mapping: `CanvasSetPixel(col, row, …)` maps source pixel `(col, row)` to canvas
> `(col, row)` and additionally honors `canvas.Rotate` / `canvas.Flip`. For the parity scenario
> (`ROTATE_0`, `FLIP_NONE`) this is a direct 1:1 placement; rotation/flip support is a free bonus
> of routing through `CanvasSetPixel` and is not exercised by this spec.

### Reference skeleton of CanvasDrawPng

```c
void CanvasDrawPng(FIL *file) {
    SHOWDEBUG("Creating read structure\n");
    png_structp pngPointer = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, PngShowError, NULL);

    if (pngPointer == NULL) {
        SHOWDEBUG("png_create_read_struct error\n");
        return;
    }

    SHOWDEBUG("Allocating memory for image information\n");
    png_infop infoPointer = png_create_info_struct(pngPointer);

    if (infoPointer == NULL) {
        SHOWDEBUG("png_create_info_struct error\n");
        png_destroy_read_struct(&pngPointer, NULL, NULL);
        return;
    }

    SHOWDEBUG("Setting up the custom read function\n");
    png_set_read_fn(pngPointer, file, PngCustomReadData);

    /* Declared volatile before setjmp so their values survive a longjmp (C99 7.13.2.1) and can be cleaned up. */
    volatile png_bytep rowPointers = NULL;
    SHOWDEBUG("Setting up LongJump\n");

    if (setjmp(png_jmpbuf(pngPointer)) != 0) {
        /* Any libpng error from here on jumps back to this point. */
        SHOWDEBUG("LongJump from a libpng error. Cleaning up.\n");

        if (rowPointers != NULL)
            png_free(pngPointer, rowPointers);

        png_destroy_read_struct(&pngPointer, &infoPointer, NULL);
        return;
    }

    SHOWDEBUG("Reading info\n");
    /* png_read_info gets information about PNG file before the first IDAT (image data chunk). REQUIRED. */
    png_read_info(pngPointer, infoPointer);

    SHOWDEBUG("\nParsing image info\n");
    png_uint_32 width, height;
    int bitDepth, colorType, interlaceType;
    png_get_IHDR(pngPointer, infoPointer, &width, &height, &bitDepth, &colorType, &interlaceType, NULL, NULL);
    SHOWDEBUG("PNG info: width: %d, height: %d, bit_depth: %d\n", width, height, bitDepth);

    int col, row;
    int maxCol = width > canvas.Width ? canvas.Width : width;    /* won't write outside canvas. */
    int maxRow = height > canvas.Height ? canvas.Height : height;

    int num_palette = 0;
    png_colorp palette = NULL;

    if (colorType == PNG_COLOR_TYPE_PALETTE)
        png_get_PLTE(pngPointer, infoPointer, &palette, &num_palette);

    for (row = 0; row < maxRow; row++) {
        png_bytep rowBuffer = (png_bytep)png_malloc(pngPointer, png_get_rowbytes(pngPointer, infoPointer));
        rowPointers = rowBuffer; // track for cleanup before any call that may longjmp
        png_read_rows(pngPointer, &rowBuffer, NULL, 1);

        for (col = 0; col < maxCol; col++) {
            png_byte red, green, blue;

            if ((colorType == PNG_COLOR_TYPE_PALETTE) && (palette != NULL)) {
                red = palette[rowPointers[col]].red;
                green = palette[rowPointers[col]].green;
                blue = palette[rowPointers[col]].blue;
            } else {
                /* if the image is paletted but we don't have a palette, display as grayscale using palette index. */
                png_bytep pixel = &rowPointers[col];
                red = *(pixel++);
                green = *(pixel++);
                blue = *(pixel++);
            }

            /* The LCD uses RGB565 16-bits format: RRRRRGGG GGGBBBBB */
            UINT16 color = (UINT16)(((red & 0b11111000) | ((green & 0b11100000) >> 5)) << 8)
                        | (UINT16)(((green & 0b00011100) << 3) | ((blue & 0b11111000) >> 3));
            CanvasSetPixel(col, row, color);
        }

        png_free(pngPointer, rowBuffer);
        rowPointers = NULL;
    }

    png_destroy_read_struct(&pngPointer, &infoPointer, NULL);
}
```

> This skeleton is the design intent, not final code; the implementation task will produce the
> exact source. Note the removed `lcdSelected` declaration and the removed CS line in the longjmp
> block, the canvas-based `maxCol`/`maxRow`, and the `CanvasSetPixel` replacement — everything else
> mirrors `LCDRenderPng` verbatim, including traces and comments (Req 11).

### Diagnostic parity (Requirement 11)

All `SHOWDEBUG` traces and source comments tied to **retained** decode steps are reproduced
verbatim: "Creating read structure", "png_create_read_struct error", "Allocating memory for image
information", "png_create_info_struct error", "Setting up the custom read function", the
volatile/longjmp comment, "Setting up LongJump", "LongJump from a libpng error. Cleaning up.", the
longjmp comment, "Reading info" + its comment, "Parsing image info", "PNG info: …", the
`png_read_info` comment, the `maxCol` clip comment (reworded "won't write outside canvas"), the
"track for cleanup" comment, the null-palette comment, and the RGB565 format comment.

`SHOWDEBUG`/comments tied to **omitted** panel operations are dropped with them: "LCD SPI chip
select" and "LCD SPI released" (Requirement 11.3, 4.x).

## Data Models

`CanvasDrawPng` introduces no new persistent data structures; it reads from existing types and
writes into the existing canvas surface. The relevant models are:

### Canvas surface (existing — `Canvas` struct, `Canvas.h`)

Only a subset of fields is consumed by `CanvasDrawPng` (read-only here; writes go through
`CanvasSetPixel`):

| Field | Type | Role in CanvasDrawPng |
|-------|------|------------------------|
| `canvas.Image` | `UINT8 *` | Destination buffer; written byte-pairs per pixel via `CanvasSetPixel`. |
| `canvas.Width` | `UINT16` | Column clip bound (`Clip_Width = min(pngWidth, canvas.Width)`). |
| `canvas.Height` | `UINT16` | Row clip bound (`Clip_Height = min(pngHeight, canvas.Height)`). |
| `canvas.Scale` | `UINT16` | Must be 65 (RGB565) for the byte layout below; set by the caller via `CanvasSetScale(65)`. |
| `canvas.Rotate`, `canvas.Flip` | `UINT16` | Applied inside `CanvasSetPixel`; `ROTATE_0`/`FLIP_NONE` for the parity scenario. |

### Pixel formats

- **Decoded source pixel** (from libpng row buffer): either a 1-byte palette index resolved to an
  `{red, green, blue}` triple via `png_colorp palette`, or three consecutive bytes
  `{red, green, blue}` read directly from the row.
- **RGB565 (`UINT16`)** — packed 16-bit value, layout `RRRRR GGGGGG BBBBB`:

  ```
  bit:  15 14 13 12 11 | 10  9  8  7  6  5 |  4  3  2  1  0
        R4 R3 R2 R1 R0 | G5 G4 G3 G2 G1 G0 | B4 B3 B2 B1 B0
  high byte = (R & 0xF8) | (G >> 5)        low byte = ((G & 0x1C) << 3) | (B >> 3)
  ```

- **Canvas buffer storage** — `CanvasSetPixel` (scale 65) writes `canvas.Image[Addr] = color >> 8`
  (high byte) then `canvas.Image[Addr+1] = color & 0xFF` (low byte): big-endian, matching the byte
  order `LCDRenderTexture` streams to the panel.

### File handle (existing — FatFS `FIL`)

Passed by the caller as `FIL *`, positioned at the start of an open PNG. Consumed only indirectly
through the libpng read callback `PngCustomReadData`, which calls `f_read` on it. `CanvasDrawPng`
does not open, seek, or close the file (the caller owns its lifecycle).

## Correctness Properties

### Property 1: Byte-for-byte parity with the legacy path
For any pixel `(col, row)` within bounds, the two bytes `CanvasDrawPng` deposits into
`canvas.Image` equal the two bytes `LCDRenderPng` would have sent over SPI for the same decoded
`{red, green, blue}`.
**Validates: Requirements 3.1, 3.3**

### Property 2: No out-of-bounds writes
`CanvasSetPixel` is never invoked for `col >= canvas.Width` or `row >= canvas.Height`; writes are
confined to `[0, Clip_Width) × [0, Clip_Height)`.
**Validates: Requirements 6.3, 6.4, 6.5, 3.4**

### Property 3: No panel side effects
Executing `CanvasDrawPng` performs no SPI transfer, no GPIO/CS toggling, and no LCD command — the
panel state is identical before and after the call.
**Validates: Requirements 4.1, 4.2, 4.3, 4.5, 3.2**

### Property 4: No resource leak on any path
Every successful or failed execution ends with all libpng structures destroyed and every allocated
row buffer freed; repeated calls retain no state.
**Validates: Requirements 7.5, 8.1, 8.3**

### Property 5: Decode logic invariance
The libpng call sequence, palette handling and clipping decisions are identical to `LCDRenderPng`
except for the documented clip-bound source (canvas vs panel) and the pixel destination.
**Validates: Requirements 2.1, 2.4, 2.5, 5.2, 5.3, 5.4**

### Property 6: Platform determinism
For identical input PNG and identical canvas configuration, the resulting `canvas.Image` bytes are
identical on RP2040 and ESP32.
**Validates: Requirements 10.3**

### Property 7: Reference path preserved
`LCDRenderPng` remains byte-for-byte unchanged and continues to render directly to the panel.
**Validates: Requirements 9.2, 9.3**

## Error Handling

`CanvasDrawPng` reproduces `LCDRenderPng`'s error model exactly (Requirement 7):

- `png_create_read_struct` returns NULL → log + `return` (no resources allocated). (7.1)
- `png_create_info_struct` returns NULL → `png_destroy_read_struct(&pngPointer, NULL, NULL)` +
  `return`. (7.2)
- `setjmp(png_jmpbuf(...))` is established after both structures exist and before any libpng call
  that reads image data. (7.3)
- `rowPointers` is `volatile`, initialized NULL before the error point, so its latest value
  survives `longjmp` and reaches the cleanup path. (7.4)
- On a libpng error after the jump point: free the outstanding row buffer (when non-null), destroy
  read+info structures, return — no resources left allocated. The CS-restoration line present in
  `LCDRenderPng` is omitted (4.6). (7.5)
- Success path: `png_destroy_read_struct` exactly once before return; the decode is considered
  successful regardless of destruction outcome; each invocation owns and releases its own
  structures (no state retained between calls). (8.1, 8.2, 8.3)

## Build / Wiring

No build-system changes are required:

- `Canvas.c` is already compiled on both platforms — listed in `GUILL_COMMON_SRCS` (RP2040
  `add_executable`) and in the ESP32 `idf_component_register SRCS`.
- libpng and FatFS are already built and linked (RP2040 `PostExecutable.cmake`; ESP32 component),
  because `LCDRenderer.c` already uses `<png.h>` and `ff.h`. `Canvas.c` simply adds the same
  includes.
- The only file edits are: `Canvas.c` (new includes + two static helpers + `CanvasDrawPng`) and
  `Canvas.h` (`#include "ff.h"` + the prototype). `LCDRenderer.c`/`.h` are untouched (Requirement
  9).

## Cross-Platform Considerations (Requirement 10)

`CanvasDrawPng` contains no platform-specific code, no `#ifdef`, and no direct platform API calls
(it uses only libpng, FatFS via the read callback, and `CanvasSetPixel`). The RGB565 packing is
host-endianness-independent integer math, and `CanvasSetPixel` writes the buffer in a fixed
big-endian order on every target, so the resulting `canvas.Image` bytes are identical on RP2040 and
ESP32 for the same input PNG. Build succeeds on both targets with the shared source.

## Testing Strategy

Given the embedded target and the existing project practice (no unit-test harness; RP2040 is the
hardware-validated platform), verification is:

1. **Build verification (both targets where configured).** Compile RP2040 (primary) and confirm no
   errors/warnings introduced; confirm `compile_commands.json` gains a `CanvasDrawPng` entry and
   clangd reports no diagnostics on `Canvas.c`. ESP32 wiring is parity-only (Requirement 10.2).
2. **Visual / behavioral check on RP2040 hardware** using the `Sample.c` flow above:
   - The PNG appears composited in the texture and is shown by the single `LCDRenderTexture` blit
     with **no top-to-bottom scan/flicker** (the motivating outcome).
   - Text drawn after `CanvasDrawPng` overlays the image correctly.
3. **Parity sanity** — the on-buffer result of `CanvasDrawPng` + `LCDRenderTexture` is visually the
   same image as the legacy `LCDRenderPng` direct path for the same file.
4. **Clipping** — a PNG larger than the canvas is clipped to `canvas.Width`/`canvas.Height` with no
   out-of-bounds writes (Requirement 6); a smaller PNG fills only its area.
5. **Regression** — `LCDRenderPng` still renders directly to the panel unchanged (Requirement 9).

## Requirements Traceability

| Requirement | Addressed by |
|-------------|--------------|
| 1 — Entry point | Public function + `Canvas.h` prototype; `void CanvasDrawPng(FIL *)`; PascalCase/`Canvas` prefix. |
| 2 — Decode pipeline | Step-by-step mapping table; verbatim libpng call sequence. |
| 3 — Pixels to buffer | RGB565 packing + `CanvasSetPixel`; no SPI calls; identical bit math; canvas-bound clipping. |
| 4 — Omit panel ops | Removed `LCDSetDisplayArea`, DC/CS toggling, `lcdSelected`, `DriverSendCommand(0x29)`, CS restore in error path. |
| 5 — Palette handling | `png_get_PLTE` once; palette / non-palette / null-palette extraction reproduced. |
| 6 — Clipping | `maxCol`/`maxRow` clamp to `canvas.Width`/`canvas.Height`; glossary deviation documented. |
| 7 — Error handling | Error Handling section; null checks, setjmp, volatile `rowPointers`, longjmp cleanup. |
| 8 — Success cleanup | Single `png_destroy_read_struct` on success; per-call resource ownership. |
| 9 — LCDRenderPng unchanged | Helpers duplicated in `Canvas.c`; only `Canvas.c`/`Canvas.h` edited. |
| 10 — Cross-platform parity | Platform-agnostic, no `#ifdef`; endianness-independent packing; identical buffer bytes. |
| 11 — Logging/comments parity | Diagnostic parity section; retained traces verbatim, panel-tied traces omitted. |
