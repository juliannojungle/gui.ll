# Requirements Document

## Introduction

This feature adds a new drawing-layer function, `CanvasDrawPng`, to the Canvas module
(`src/lib/GUI/Canvas.c`, prototype in `src/lib/GUI/Canvas.h`). `CanvasDrawPng` decodes a PNG
file from an open FatFS file handle and composites the decoded image into the in-RAM canvas
texture buffer, instead of streaming the decoded pixels directly to the LCD panel over SPI.

The function is a faithful, command-by-command copy of the existing `LCDRenderPng`
(`src/lib/LCD/1in28/LCDRenderer.c`). The **only** behavioral difference is the destination of
each decoded pixel: where `LCDRenderPng` writes the two RGB565 bytes to the panel via
`SPIWriteByte`, `CanvasDrawPng` writes the pixel into the canvas buffer via `CanvasSetPixel`.
Because `CanvasDrawPng` never touches the panel, the LCD-panel-specific operations in
`LCDRenderPng` (display-area setup, chip-select toggling, chip-select tracking, and the final
display-on command) are not part of `CanvasDrawPng`.

The motivation is flicker-free rendering: decoding a PNG directly to the panel updates the
screen row-by-row with inflate latency between rows, producing a visible scan/flicker. By
decoding into a pre-allocated full-screen texture, drawing text over the same buffer, and then
performing a single bulk blit via `LCDRenderTexture`, the on-screen update is smooth.

This spec covers only full-canvas PNG compositing. Drawing a PNG into a sub-area
(`CanvasDrawPngArea`) is explicitly deferred to a future spec.

## Glossary

- **Canvas_Module**: The drawing-layer translation unit `src/lib/GUI/Canvas.c` together with its
  public interface `src/lib/GUI/Canvas.h`. Owns the global `canvas` surface state.
- **CanvasDrawPng**: The new function defined in this spec. Decodes a PNG from an open file handle
  into the canvas texture buffer.
- **LCDRenderPng**: The existing reference function in `src/lib/LCD/1in28/LCDRenderer.c` that
  decodes a PNG and streams it to the LCD panel over SPI. Serves as the source of truth for the
  decode logic.
- **CanvasSetPixel**: The existing Canvas_Module function `CanvasSetPixel(xPoint, yPoint, color)`
  that writes one RGB565 pixel into the canvas buffer at the given column/row.
- **Canvas_Buffer**: The pre-allocated, full-screen RGB565 texture buffer referenced by the global
  `canvas` surface (`canvas.Image`), written by `CanvasSetPixel` in big-endian (high byte first)
  memory order under scale 65.
- **RGB565**: The 16-bit color format `RRRRRGGG GGGBBBBB` produced by the same bit math used in
  `LCDRenderPng`.
- **File_Handle**: An open FatFS `FIL *` positioned at the start of a PNG file, supplied by the
  caller.
- **libpng**: The third-party PNG decode library (`src/Dependency/libpng`) used unchanged.
- **PngCustomReadData**: The existing custom libpng read callback that reads bytes from the
  File_Handle via `f_read`.
- **Clip_Width**: The lesser of the PNG width and the canvas width (`canvas.Width`).
- **Clip_Height**: The lesser of the PNG height and the canvas height (`canvas.Height`).

## Requirements

### Requirement 1: Provide the CanvasDrawPng entry point

**User Story:** As a firmware developer, I want a `CanvasDrawPng` function in the Canvas module, so
that I can decode a PNG into the in-RAM texture buffer before a single bulk blit to the panel.

#### Acceptance Criteria

1. THE Canvas_Module SHALL define a function `CanvasDrawPng` that accepts exactly one parameter, a
   `FIL *` file handle named in camelCase, and that has return type `void` (returns no value on
   every code path, including error paths).
2. THE Canvas_Module SHALL declare the `CanvasDrawPng` prototype in `Canvas.h` with a signature
   identical to its definition in `Canvas.c` (return type `void`, one `FIL *` parameter).
3. THE `CanvasDrawPng` name SHALL follow the project naming convention of PascalCase with the
   `Canvas` module prefix.
4. WHEN `CanvasDrawPng` is invoked with a File_Handle referencing a decodable PNG, THE
   Canvas_Module SHALL convert each decoded pixel to RGB565 and write it into the Canvas_Buffer
   through `CanvasSetPixel`, clamping the written region to at most Clip_Width columns and
   Clip_Height rows so that no pixel is written outside the canvas bounds.
5. IF allocation of the libpng read or info resources fails, or a libpng decoding error occurs
   while reading the file, THEN THE Canvas_Module SHALL release every decoder resource it has
   allocated and return without a value, leaving any canvas pixels written before the failure
   unchanged.

### Requirement 2: Faithfully replicate the libpng decode pipeline

**User Story:** As a maintainer, I want `CanvasDrawPng` to mirror `LCDRenderPng` line by line, so
that the decode behavior stays identical and predictable across both functions.

#### Acceptance Criteria

1. WHEN `CanvasDrawPng` begins, THE CanvasDrawPng SHALL create the libpng read structure using
   `png_create_read_struct` with the same version string and the same `PngShowError` error
   handler used by LCDRenderPng.
2. WHEN the read structure has been created, THE CanvasDrawPng SHALL allocate the image info
   structure using `png_create_info_struct`.
3. THE CanvasDrawPng SHALL register the custom read callback `PngCustomReadData` against the
   File_Handle using `png_set_read_fn`.
4. WHEN the read and info structures have been created, THE CanvasDrawPng SHALL read PNG header
   information using `png_read_info` followed by `png_get_IHDR` to obtain width, height, bit
   depth, color type, and interlace type.
5. WHILE iterating over image rows, THE CanvasDrawPng SHALL allocate each row buffer with
   `png_malloc` sized by `png_get_rowbytes` and read exactly one row per iteration with
   `png_read_rows`.
6. WHEN a row has been processed, THE CanvasDrawPng SHALL free that row buffer with `png_free`.

### Requirement 3: Write decoded pixels to the canvas buffer instead of the panel

**User Story:** As a firmware developer, I want each decoded pixel written into the canvas texture
buffer, so that the PNG is composited in RAM rather than scanned onto the panel.

#### Acceptance Criteria

1. WHEN a decoded pixel at source column C and source row R has been converted to an RGB565 value,
   THE CanvasDrawPng SHALL write that pixel into the Canvas_Buffer by calling `CanvasSetPixel` with
   column C, row R, and the single 16-bit RGB565 value, so that the decoded pixel at source
   position (C, R) maps to canvas coordinate (C, R).
2. THE CanvasDrawPng SHALL NOT call `SPIWriteByte` or any other SPI function for any purpose,
   including pixel-data transfer.
3. WHEN converting a decoded pixel to RGB565, THE CanvasDrawPng SHALL produce a single 16-bit value
   in the format `RRRRRGGG GGGBBBBB` using the identical red, green, and blue masks and shifts
   applied by LCDRenderPng, such that for any given (red, green, blue) input the resulting value is
   bit-for-bit identical to the value LCDRenderPng produces for that same input.
4. IF a decoded pixel's source column is greater than or equal to the canvas width, or its source
   row is greater than or equal to the canvas height, THEN THE CanvasDrawPng SHALL NOT call
   `CanvasSetPixel` for that pixel, limiting writes to columns 0 through (Clip_Width − 1) and rows
   0 through (Clip_Height − 1).

### Requirement 4: Omit LCD-panel-specific operations

**User Story:** As a maintainer, I want `CanvasDrawPng` to exclude every panel-only operation, so
that the function has no dependency on the LCD panel and produces no on-screen side effects.

#### Acceptance Criteria

1. THE CanvasDrawPng SHALL NOT call `LCDSetDisplayArea`.
2. THE CanvasDrawPng SHALL NOT call `DigitalWrite` on `LCD_DC_PIN`.
3. THE CanvasDrawPng SHALL NOT call `DigitalWrite` on `LCD_CS_PIN`.
4. THE CanvasDrawPng SHALL NOT maintain any LCD chip-select state variable or flag (such as
   `lcdSelected`).
5. THE CanvasDrawPng SHALL NOT send the display-on command `DriverSendCommand(0x29)`.
6. IF a libpng decoding error occurs during execution, THEN THE CanvasDrawPng SHALL NOT perform any
   chip-select restoration via `DigitalWrite` on `LCD_CS_PIN`.

### Requirement 5: Handle palette and non-palette color types

**User Story:** As a firmware developer, I want palette and direct-color PNGs handled the same way
as in `LCDRenderPng`, so that both color types decode correctly into the canvas.

#### Acceptance Criteria

1. WHEN the PNG color type is `PNG_COLOR_TYPE_PALETTE`, THE CanvasDrawPng SHALL read the palette
   once with `png_get_PLTE` before iterating over image rows.
2. IF the PNG color type is `PNG_COLOR_TYPE_PALETTE` AND the palette pointer is non-null, THEN THE
   CanvasDrawPng SHALL resolve each pixel's red, green, and blue from the palette entry indexed by
   the single unsigned byte read from the row buffer at that pixel's column position.
3. IF the PNG color type is not `PNG_COLOR_TYPE_PALETTE`, THEN THE CanvasDrawPng SHALL read three
   consecutive unsigned bytes from the row buffer in red-then-green-then-blue order, starting at
   that pixel's byte offset.
4. IF the PNG color type is `PNG_COLOR_TYPE_PALETTE` AND the palette pointer is null, THEN THE
   CanvasDrawPng SHALL read three consecutive unsigned bytes from the row buffer in
   red-then-green-then-blue order, starting at that pixel's byte offset, matching the LCDRenderPng
   fallback.

### Requirement 6: Clip the image to the canvas dimensions

**User Story:** As a firmware developer, I want oversized PNGs clipped to the display dimensions,
so that decoding never writes outside the canvas bounds.

#### Acceptance Criteria

1. WHEN CanvasDrawPng decodes a PNG, THE CanvasDrawPng SHALL set Clip_Width to the smaller of the
   decoded PNG width and `canvas.Width`, where `canvas.Width` is the canvas width in pixels.
2. WHEN CanvasDrawPng decodes a PNG, THE CanvasDrawPng SHALL set Clip_Height to the smaller of the
   decoded PNG height and `canvas.Height`, where `canvas.Height` is the canvas height in pixels.
3. WHEN CanvasDrawPng processes a PNG, THE CanvasDrawPng SHALL limit the processed column count to
   Clip_Width.
4. WHEN CanvasDrawPng processes a PNG, THE CanvasDrawPng SHALL limit the processed row count to
   Clip_Height.
5. THE CanvasDrawPng SHALL NOT write to any column index outside the range 0 to Clip_Width minus 1,
   nor to any row index outside the range 0 to Clip_Height minus 1.

### Requirement 7: Handle libpng errors with cleanup

**User Story:** As a maintainer, I want `CanvasDrawPng` to recover from libpng errors without
leaking memory, so that a malformed PNG cannot corrupt or exhaust RAM.

#### Acceptance Criteria

1. IF `png_create_read_struct` returns null, THEN THE CanvasDrawPng SHALL return to its caller
   without reading image data and without leaving any decoder resources allocated.
2. IF `png_create_info_struct` returns null, THEN THE CanvasDrawPng SHALL destroy the read
   structure with `png_destroy_read_struct` and return to its caller without reading image data.
3. THE CanvasDrawPng SHALL establish a libpng error return point using `setjmp` on the libpng jump
   buffer after both the read and info structures are created and before any libpng call that reads
   image data.
4. THE CanvasDrawPng SHALL declare the row-buffer tracking variable as volatile, initialize it to
   null before the error return point is established, and ensure its most recent value survives a
   libpng `longjmp` and is reachable by the cleanup path.
5. IF a libpng error occurs after the error return point is established, THEN THE CanvasDrawPng
   SHALL free the outstanding row buffer with `png_free` when the row-buffer tracking variable is
   non-null, destroy the read and info structures with `png_destroy_read_struct`, and return with
   no decoder resources left allocated.

### Requirement 8: Release decode resources on the success path

**User Story:** As a maintainer, I want decode resources released after a successful decode, so
that repeated calls do not leak memory.

#### Acceptance Criteria

1. WHEN all rows have been decoded successfully, THE CanvasDrawPng SHALL destroy the read and info
   structures with `png_destroy_read_struct` exactly once before returning to the caller.
2. THE CanvasDrawPng SHALL treat a successfully decoded image as a successful operation regardless
   of the outcome of resource destruction.
3. WHEN CanvasDrawPng is invoked repeatedly, THE CanvasDrawPng SHALL allocate and release its own
   decoder structures on each invocation and SHALL retain no decoder structures from a prior
   invocation.

### Requirement 9: Leave LCDRenderPng unchanged

**User Story:** As a maintainer, I want the existing `LCDRenderPng` left untouched, so that the
direct-to-panel rendering path keeps working exactly as before.

#### Acceptance Criteria

1. THE Canvas_Module SHALL define the new `CanvasDrawPng` function in `src/lib/GUI/Canvas.c`.
2. THE Canvas_Module SHALL leave the existing `LCDRenderPng` function in
   `src/lib/LCD/1in28/LCDRenderer.c` byte-for-byte identical to its state before `CanvasDrawPng`
   was added, including its `void LCDRenderPng(FIL *file)` signature.
3. WHEN `LCDRenderPng` is invoked after `CanvasDrawPng` has been added, THE LCDRenderPng SHALL
   stream the decoded PNG pixels to the LCD panel over SPI, producing panel output identical to its
   behavior before the addition.

### Requirement 10: Preserve cross-platform parity

**User Story:** As a maintainer, I want `CanvasDrawPng` to build on both targets, so that the
single codebase stays consistent across RP2040 and ESP32.

#### Acceptance Criteria

1. THE CanvasDrawPng SHALL reside in the platform-agnostic Canvas_Module and SHALL contain no
   platform-specific preprocessor branches and no direct platform API calls.
2. WHEN the Canvas_Module source is compiled unchanged for the RP2040 target and for the ESP32
   target, THE build SHALL complete with zero compilation errors and zero linker errors on both
   targets.
3. WHEN the same input PNG is decoded by CanvasDrawPng on the RP2040 target and on the ESP32
   target, THE CanvasDrawPng SHALL write byte-for-byte identical RGB565 values to the Canvas_Buffer,
   such that every byte at every corresponding buffer offset is equal between the two targets.
4. IF CanvasDrawPng fails to decode or rejects an input PNG on one target, THEN THE CanvasDrawPng
   SHALL fail to decode or reject the same input PNG on the other target with an identical error
   indication and SHALL leave the Canvas_Buffer unmodified on both targets.

### Requirement 11: Replicate diagnostic logging and comments

**User Story:** As a maintainer, I want `CanvasDrawPng` to carry the same `SHOWDEBUG` traces and
source comments as `LCDRenderPng`, so that the two functions read identically and produce the same
diagnostic output for the decode steps they share.

#### Acceptance Criteria

1. THE CanvasDrawPng SHALL replicate verbatim each `SHOWDEBUG` call from LCDRenderPng that is
   associated with a decode step retained by CanvasDrawPng (read-structure creation, info
   allocation, read-function setup, longjmp setup, info reading, image-info parsing, and per-read
   progress).
2. THE CanvasDrawPng SHALL replicate verbatim each source comment from LCDRenderPng that is
   associated with a decode step retained by CanvasDrawPng.
3. WHERE a `SHOWDEBUG` call or comment in LCDRenderPng is associated with an LCD-panel operation
   omitted under Requirement 4 (for example the chip-select select/release traces), THE
   CanvasDrawPng SHALL omit that `SHOWDEBUG` call or comment together with the operation it
   describes.
