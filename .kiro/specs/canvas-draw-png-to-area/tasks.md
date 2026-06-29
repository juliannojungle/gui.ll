# Implementation Plan: CanvasDrawPngToArea

## Overview

Add `CanvasDrawPngToArea` to the Canvas module — a faithful structural copy of `CanvasDrawPng` with three localized behavioral additions: row skip/stop optimization, sub-area column extraction, and transparency color-key filtering. The implementation touches three files: `Canvas.h` (prototype), `Canvas.c` (function definition), and `AGENTS.md` (documentation).

## Tasks

- [x] 1. Add prototype and implement CanvasDrawPngToArea
  - [x] 1.1 Add the CanvasDrawPngToArea prototype to Canvas.h
    - Add `void CanvasDrawPngToArea(FIL *file, UINT16 xSource, UINT16 ySource, UINT16 width, UINT16 height, UINT16 xTarget, UINT16 yTarget);` after the existing `CanvasDrawPng` prototype
    - _Requirements: 1.1, 1.2, 6.1_

  - [x] 1.2 Implement CanvasDrawPngToArea in Canvas.c
    - Place the function after `CanvasDrawPng` in Canvas.c
    - Start with a faithful copy of the entire `CanvasDrawPng` function body: same libpng pipeline (`png_create_read_struct`, `png_create_info_struct`, `png_set_read_fn`, `setjmp`/`longjmp`, `png_read_info`, `png_get_IHDR`, `png_read_rows`, `png_destroy_read_struct`), same variable names, same SHOWDEBUG traces, same comments, same `volatile png_bytep rowPointers` pattern, same palette/non-palette conditional, same RGB565 bit math with binary `0b` notation
    - Add NULL check for `file` parameter — return immediately before any allocations
    - Add effective dimension clamping before the row loop: `effectiveWidth = min(width, pngWidth - xSource)`, `effectiveHeight = min(height, pngHeight - ySource)`, `maxCol = min(effectiveWidth, canvas.Width - xTarget)`, `maxRow = effectiveHeight`; if `xSource >= pngWidth` or `ySource >= pngHeight`, set effective dimensions to zero (proceeds to cleanup with no pixel writes)
    - Behavioral addition 1 — Row skipping/stopping: rows before `ySource` are decoded via `png_read_rows` (to advance DEFLATE state) but pixels are NOT extracted; once `ySource + effectiveHeight` rows are reached, stop calling `png_read_rows` and proceed to `png_destroy_read_struct`
    - Behavioral addition 2 — Sub-area column extraction: within active rows, iterate only columns `[xSource, xSource + maxCol)` and write to canvas at `(xTarget + dx, yTarget + dy)` via `CanvasSetPixel`
    - Behavioral addition 3 — Transparency filtering: after computing the RGB565 `color` value, compare against `TRANSPARENT` (`0xF81F`); if equal, skip the `CanvasSetPixel` call, leaving existing canvas content unchanged
    - Maintain full code fidelity: identical variable names, comments, formatting, binary `0b` notation — NOT hex
    - _Requirements: 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.1, 2.2, 2.3, 2.4, 2.5, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 4.1, 4.2, 4.3, 5.1, 5.2, 5.3, 6.1, 6.2, 6.3_

- [x] 2. Checkpoint - Ensure clean build
  - Ensure the project builds with zero warnings on both RP2040 (cmake + make) and ESP32 (idf.py). Ask the user if questions arise.

- [x] 3. Update project documentation
  - [x] 3.1 Update AGENTS.md with CanvasDrawPngToArea documentation
    - Add a bullet under "Recent work" documenting `CanvasDrawPngToArea`: signature, purpose (sub-area extraction from PNG atlases with transparency), the three behavioral additions over `CanvasDrawPng`, the faithful-copy design rule, and that no build-system changes were needed
    - _Requirements: 1.1, 6.1_

- [x] 4. Final checkpoint - Verify on hardware
  - Ensure all builds pass, ask the user if questions arise. Verification is done by building firmware and running on hardware (load a PNG spritesheet to SD, call CanvasDrawPngToArea with various source rectangles, verify correct sub-area appears at the target position on the LCD with transparency working).

## Notes

- No tests are written — the project has an explicit "DO NOT WRITE TESTS" policy. Verification is done by building firmware and running on the target hardware.
- This is essentially a single function addition. The implementation is a faithful copy of CanvasDrawPng with three localized behavioral additions — identical variable names, comments, formatting, and binary `0b` notation must be preserved.
- No build system changes needed — Canvas.c is already in the build on both platforms (RP2040 and ESP32-S3).
- The existing static helpers `PngCustomReadData` and `PngShowError` are reused as-is.

## Task Dependency Graph

```json
{
  "waves": [
    { "id": 0, "tasks": ["1.1"] },
    { "id": 1, "tasks": ["1.2"] },
    { "id": 2, "tasks": ["3.1"] }
  ]
}
```
