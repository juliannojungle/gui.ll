# Requirements Document

## Introduction

Add a `CanvasDrawPngToArea` method to the Canvas module that decodes a rectangular sub-area of a PNG file and composites it into the canvas texture at a specified target position. The method follows the same structure as `CanvasDrawPng` (same libpng pipeline, error handling, SHOWDEBUG traces, private helpers) with three key differences: optimized row reading (skip/stop), sub-area pixel extraction, and transparency-based color-key skipping.

## Glossary

- **Canvas**: The in-RAM drawing surface (RGB565 texture buffer) used for flicker-free compositing before panel blit
- **Canvas_Module**: The `Canvas.c` / `Canvas.h` module that implements the drawing API
- **CanvasDrawPngToArea**: The new method being specified: `void CanvasDrawPngToArea(FIL *file, UWORD xSource, UWORD ySource, UWORD width, UWORD height, UWORD xTarget, UWORD yTarget)`
- **Source_Area**: The rectangular region [xSource, ySource] to [xSource + width, ySource + height] within the PNG image
- **Target_Position**: The point [xTarget, yTarget] in the canvas texture where the top-left of the extracted sub-area is placed
- **TRANSPARENT**: The color-key sentinel `RGB_COLOR(255, 0, 255)` = `0xF81F` (magenta); pixels matching this value are skipped during compositing
- **Row_Skipping**: The optimization of reading (and discarding) PNG rows before ySource without decoding their pixels into the canvas
- **Row_Stopping**: The optimization of stopping PNG row decoding once ySource + height rows have been processed
- **PngCustomReadData**: The existing static helper in Canvas.c that reads PNG data from a FatFS FIL via libpng's custom I/O
- **PngShowError**: The existing static error callback in Canvas.c for libpng error reporting
- **RGB565**: 16-bit pixel format (5 red, 6 green, 5 blue) used by the canvas and the GC9A01 LCD panel

## Requirements

### Requirement 1: Faithful structural copy of CanvasDrawPng

**User Story:** As a developer, I want CanvasDrawPngToArea to follow the same code structure as CanvasDrawPng, so that the codebase remains consistent and maintainable.

#### Acceptance Criteria

1. THE Canvas_Module SHALL expose `CanvasDrawPngToArea` with the signature `void CanvasDrawPngToArea(FIL *file, UWORD xSource, UWORD ySource, UWORD width, UWORD height, UWORD xTarget, UWORD yTarget)`
2. THE Canvas_Module SHALL declare the `CanvasDrawPngToArea` prototype in `Canvas.h`
3. THE CanvasDrawPngToArea SHALL use the same libpng decode pipeline as CanvasDrawPng: `png_create_read_struct`, `png_create_info_struct`, `png_set_read_fn`, `setjmp`/`longjmp` error recovery, `png_read_info`, `png_get_IHDR`, row-by-row `png_read_rows`, and `png_destroy_read_struct`
4. THE CanvasDrawPngToArea SHALL use the same SHOWDEBUG traces as CanvasDrawPng for each corresponding step
5. THE CanvasDrawPngToArea SHALL declare `rowPointers` as `volatile png_bytep` and track the allocation for cleanup on longjmp, matching the existing error-recovery pattern
6. THE CanvasDrawPngToArea SHALL reuse the existing static helpers `PngCustomReadData` and `PngShowError` already defined in Canvas.c
7. THE CanvasDrawPngToArea SHALL handle both palette (`PNG_COLOR_TYPE_PALETTE`) and non-palette PNG color types using the same conditional logic as CanvasDrawPng
8. IF `file` is NULL, THEN THE CanvasDrawPngToArea SHALL return immediately without allocating any libpng resources
9. THE CanvasDrawPngToArea SHALL maintain full fidelity to the original CanvasDrawPng code: identical variable names, identical comments, identical formatting, identical numeric notation (binary `0b` literals, not hexadecimal), and identical code structure — the only differences allowed are those explicitly specified by the other requirements (row skipping/stopping, sub-area extraction, and transparency filtering)

### Requirement 2: Optimized PNG row reading

**User Story:** As a developer, I want CanvasDrawPngToArea to avoid processing rows outside the source area, so that decode time and CPU usage are minimized on resource-constrained MCUs.

#### Acceptance Criteria

1. WHEN the PNG row index is less than ySource, THE CanvasDrawPngToArea SHALL call `png_read_rows` for that row to advance the libpng decode state but SHALL NOT extract any pixels into the canvas buffer
2. WHEN the PNG row index reaches ySource + height, THE CanvasDrawPngToArea SHALL stop calling `png_read_rows` for subsequent rows and SHALL proceed directly to freeing libpng resources via `png_destroy_read_struct`
3. WHILE processing a row within the source area (row index in [ySource, ySource + height)), THE CanvasDrawPngToArea SHALL extract only the columns in the range [xSource, xSource + width) and SHALL NOT call CanvasSetPixel for columns outside that range
4. IF ySource is greater than or equal to the PNG image height, THEN THE CanvasDrawPngToArea SHALL produce no pixel output and SHALL free all libpng resources before returning
5. IF ySource + height exceeds the PNG image height, THEN THE CanvasDrawPngToArea SHALL process rows from ySource up to the last available PNG row and SHALL free all libpng resources before returning

### Requirement 3: Sub-area pixel extraction

**User Story:** As a developer, I want only the pixels within the defined source rectangle to be written to the canvas at the target position, so that I can composite sprite-like regions from larger PNG atlases.

#### Acceptance Criteria

1. WHILE decoding rows in the range [ySource, ySource + effectiveHeight), THE CanvasDrawPngToArea SHALL extract only pixels in columns [xSource, xSource + effectiveWidth) from each decoded row, where effectiveWidth = min(width, pngWidth - xSource) and effectiveHeight = min(height, pngHeight - ySource)
2. THE CanvasDrawPngToArea SHALL write extracted pixels to the canvas starting at position [xTarget, yTarget], mapping source pixel at (xSource + dx, ySource + dy) to canvas position (xTarget + dx, yTarget + dy), converting each pixel to RGB565 using the same bit layout as CanvasDrawPng
3. IF xSource + width exceeds the PNG image width, THEN THE CanvasDrawPngToArea SHALL reduce the extracted column count to (pngWidth - xSource), leaving canvas positions beyond that count unmodified
4. IF ySource + height exceeds the PNG image height, THEN THE CanvasDrawPngToArea SHALL reduce the extracted row count to (pngHeight - ySource), leaving canvas positions beyond that count unmodified
5. IF xTarget + effectiveWidth exceeds canvas.Width or yTarget + effectiveHeight exceeds canvas.Height, THEN THE CanvasDrawPngToArea SHALL skip writing any pixel whose target position (xTarget + dx, yTarget + dy) falls at or beyond canvas.Width or canvas.Height
6. IF xSource is greater than or equal to the PNG image width OR ySource is greater than or equal to the PNG image height, THEN THE CanvasDrawPngToArea SHALL write zero pixels to the canvas and return without error
7. IF libpng reports a decode error during row extraction, THEN THE CanvasDrawPngToArea SHALL free all allocated libpng resources and return without modifying canvas pixels beyond those already written before the error

### Requirement 4: Transparency color-key support

**User Story:** As a developer, I want pixels matching the TRANSPARENT color to be skipped during compositing, so that I can overlay PNG sub-areas onto existing canvas content without overwriting the background.

#### Acceptance Criteria

1. WHEN a decoded pixel produces the RGB565 value equal to TRANSPARENT (0xF81F), THE CanvasDrawPngToArea SHALL skip that pixel without calling CanvasSetPixel, leaving the existing canvas buffer content at that coordinate unchanged
2. WHEN a decoded pixel produces an RGB565 value different from TRANSPARENT (0xF81F), THE CanvasDrawPngToArea SHALL write that pixel to the canvas at the corresponding target coordinate via CanvasSetPixel
3. THE CanvasDrawPngToArea SHALL perform the TRANSPARENT comparison against the converted RGB565 value regardless of the canvas Scale setting

### Requirement 5: Cross-platform determinism

**User Story:** As a developer, I want CanvasDrawPngToArea to produce identical output on RP2040 and ESP32-S3, so that firmware behavior is consistent across targets.

#### Acceptance Criteria

1. THE CanvasDrawPngToArea SHALL use only integer arithmetic (no `float`, `double`, or floating-point library calls) for all computations including RGB565 conversion, row/column indexing, and bounds clamping, matching the bit-manipulation formula in CanvasDrawPng: `(UWORD)(((red & 0b11111000) | ((green & 0b11100000) >> 5)) << 8) | (UWORD)(((green & 0b00011100) << 3) | ((blue & 0b11111000) >> 3))`
2. THE CanvasDrawPngToArea SHALL contain no preprocessor conditional compilation directives (`#if`, `#ifdef`, `#ifndef`) that select platform-variant code paths
3. THE CanvasDrawPngToArea SHALL produce byte-for-byte identical canvas buffer contents on RP2040 and ESP32-S3 when given the same PNG file content and the same function arguments (xSource, ySource, width, height, xTarget, yTarget)

### Requirement 6: No build system changes

**User Story:** As a developer, I want the new method to require no build configuration changes, so that existing build workflows remain intact.

#### Acceptance Criteria

1. THE CanvasDrawPngToArea function definition SHALL reside in the existing Canvas.c translation unit with its prototype declared in Canvas.h, requiring no new source files to be added to GUILL_COMMON_SRCS or the ESP32 idf_component_register SRCS list
2. THE CanvasDrawPngToArea SHALL require no new library dependencies, include paths, or compiler flags beyond those already present for CanvasDrawPng (libpng, zlib, FatFS)
3. WHEN CanvasDrawPngToArea is added, THE build system SHALL produce a successful compile and link on both RP2040 (cmake + make) and ESP32 (idf.py) targets with zero modifications to CMakeLists.txt, PreExecutable.cmake, PostExecutable.cmake, or Platform/ESP32/CMakeLists.txt
