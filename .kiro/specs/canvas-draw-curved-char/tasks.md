# Implementation Plan: CanvasDrawCurvedChar

## Overview

`CanvasDrawCurvedChar` is a single new drawing primitive added to the existing Canvas module
(`src/lib/GUI/Canvas.c`, prototype in `src/lib/GUI/Canvas.h`). There are no new source files and no
build-system changes: `Canvas.c` is already in `GUILL_COMMON_SRCS` (root `CMakeLists.txt`) and in the
ESP32 `idf_component_register SRCS`.

The work is built incrementally inside `Canvas.c`/`Canvas.h`: first the public signature and input
guards, then the precomputed Q16.16 integer trig table, then the two file-local geometry helpers, then
the glyph rendering loop that wires them together through `CanvasSetPixel`. Because every implementation
step edits the same `Canvas.c`, the steps are sequential. Property-based testing is the primary
verification strategy (host build, Theft, in-RAM scale-65 canvas, guard-padded buffer, all five fonts),
backed by targeted unit/example tests and cross-platform build/static checks.

Implementation language: **C** (the module is already C; the design's pseudocode maps directly onto the
existing `Canvas.c` style).

## Tasks

- [x] 1. Add public API signature and input guards
  - [x] 1.1 Declare the prototype and implement the guard/no-op skeleton
    - Add the `CanvasDrawCurvedChar` prototype in `Canvas.h` next to `CanvasDrawChar`, with the exact
      parameter order, `UINT16`/`const char`/`sFONT*` types, camelCase names, and `void` return type.
    - Add the definition in `Canvas.c` with the early-return guards: return immediately if `font` is
      NULL, then return if `ASCIIChar` is outside the inclusive range 0x20..0x7E, then normalize the
      angle with `angle = startAngle % 360` (leave the render body as a stub for now so the module still
      compiles and links into the existing RP2040 build).
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5, 4.4, 4.5, 2.3_

- [x] 2. Generate and embed the fixed-point trigonometry lookup table
  - [x] 2.1 Embed the Q16.16 cos/sin tables produced by an offline host generator
    - Write a small standalone host generator (e.g. a throwaway C/Python script kept out of the firmware
      build) that emits `round(cos(deg * PI/180) * 65536)` and `round(sin(deg * PI/180) * 65536)` for
      `deg` in 0..359.
    - Paste its output into `Canvas.c` as two `static const int32_t canvasCurvedCosTable[360]` and
      `canvasCurvedSinTable[360]` literal arrays, plus a `SCALE` (`1 << 16`) constant. No runtime
      `<math.h>` call is introduced; the tables are compile-time constant literals so both targets link
      identical integers.
    - _Requirements: 7.3, 7.2_

- [x] 3. Implement the file-local geometry helpers
  - [x] 3.1 Implement `CanvasRoundDivAway`
    - Add `static int32_t CanvasRoundDivAway(int64_t numerator, int32_t denominator)` that performs
      round-half-away-from-zero integer division, using a 64-bit numerator so `radius * scaledTrig` and
      the doubled rotation products cannot overflow.
    - _Requirements: 3.3, 2.1_
  - [x] 3.2 Implement `CanvasRotateGlyphOffset`
    - Add `static void CanvasRotateGlyphOffset(int32_t dx2, int32_t dy2, int32_t cosV, int32_t sinV,
      int32_t *outX, int32_t *outY)` that computes the clockwise rotation `rx = dx2*cosV - dy2*sinV`,
      `ry = dx2*sinV + dy2*cosV` and divides each by `2 * SCALE` through `CanvasRoundDivAway` (the `/2`
      folds in the doubled-delta scaling), returning the signed integer offset components.
    - _Requirements: 3.1, 3.3, 2.4_

- [x] 4. Implement the glyph rendering loop and wire everything together
  - [x] 4.1 Implement the `CanvasDrawCurvedChar` body
    - Look up `cosV = canvasCurvedCosTable[angle]` / `sinV = canvasCurvedSinTable[angle]`, compute the
      anchor `anchorX = xCenter + CanvasRoundDivAway((int64_t)radius * cosV, SCALE)` and the matching
      `anchorY` (radius 0 yields the exact center).
    - Read `width`/`height` from `font->Width`/`font->Height`, compute the glyph table offset from
      `(ASCIIChar - ' ')` exactly as `CanvasDrawChar` does (1bpp, MSB-first, byte-padded rows), and
      iterate every source cell `(Column, Page)` over the full `Width x Height` box.
    - For each cell compute `dx2 = 2*Column - (width-1)`, `dy2 = 2*Page - (height-1)`, rotate via
      `CanvasRotateGlyphOffset`, add the anchor to get signed `targetX`/`targetY`, skip the pixel if
      either is negative (before any `UINT16` cast), then write the set bit with `foregroundColor`, write
      unset bits with `backgroundColor`, or skip unset bits when `backgroundColor == TRANSPARENT`. All
      writes go through `CanvasSetPixel`.
    - _Requirements: 2.1, 2.2, 2.4, 2.5, 3.1, 3.2, 3.3, 3.4, 4.1, 4.2, 4.3, 5.1, 5.2, 5.3, 5.4, 6.1, 6.2, 6.3, 6.4_

- [x] 5. Checkpoint - core implementation builds
  - Build the RP2040 target and ensure the module compiles and links. Ask the user if questions arise.

- [x] 6. Set up the host test harness and validate correctness properties
  - [x] 6.1 Stand up the host property-based test harness
    - Create a host-only test build (separate from the firmware build) that compiles `Canvas.c` against
      an in-RAM canvas configured at scale 65 (RGB565), with `canvas.Rotate = ROTATE_0` /
      `canvas.Flip = FLIP_NONE` for the geometric properties.
    - Integrate an established C property-based testing library (Theft or comparable) — do NOT hand-roll
      the generator/shrinker framework. Provide a guard-padded (canary) canvas buffer for memory-safety
      checks, font fixtures for Font8/Font12/Font16/Font20/Font24, and a way to observe the exact
      `(x, y, color)` writes performed by `CanvasSetPixel`. Configure each property to run 100+ iterations.
    - _Requirements: 4.2, 4.3, 7.2_
  - [x]* 6.2 Property test 1 - glyph anchor placement
    - **Property 1: Glyph anchor placement**
    - Tag: `Feature: canvas-draw-curved-char, Property 1: Glyph anchor placement`
    - **Validates: Requirements 2.1, 2.5**
  - [x]* 6.3 Property test 2 - angle normalization invariance
    - **Property 2: Angle normalization invariance**
    - Tag: `Feature: canvas-draw-curved-char, Property 2: Angle normalization invariance`
    - **Validates: Requirements 2.3**
  - [x]* 6.4 Property test 3 - angle-0 equivalence with CanvasDrawChar
    - **Property 3: Angle-0 equivalence with CanvasDrawChar (model-based)**
    - Tag: `Feature: canvas-draw-curved-char, Property 3: Angle-0 equivalence with CanvasDrawChar`
    - **Validates: Requirements 3.4, 3.2, 4.1**
  - [-]* 6.5 Property test 4 - rotation preserves the glyph foreground
    - **Property 4: Rotation preserves the glyph foreground**
    - Tag: `Feature: canvas-draw-curved-char, Property 4: Rotation preserves the glyph foreground`
    - **Validates: Requirements 3.1, 3.3, 2.4**
  - [-]* 6.6 Property test 5 - transparent background writes only glyph pixels
    - **Property 5: Transparent background writes only glyph pixels**
    - Tag: `Feature: canvas-draw-curved-char, Property 5: Transparent background writes only glyph pixels`
    - **Validates: Requirements 5.1, 5.2**
  - [-]* 6.7 Property test 6 - opaque background fills the rotated bounding box
    - **Property 6: Opaque background fills the rotated bounding box**
    - Tag: `Feature: canvas-draw-curved-char, Property 6: Opaque background fills the rotated bounding box`
    - **Validates: Requirements 5.3, 5.4**
  - [-]* 6.8 Property test 7 - bounds and memory safety
    - **Property 7: Bounds and memory safety** (assert against the guard-padded buffer canaries)
    - Tag: `Feature: canvas-draw-curved-char, Property 7: Bounds and memory safety`
    - **Validates: Requirements 6.1, 6.2, 6.3, 6.4**
  - [-]* 6.9 Property test 8 - invalid inputs are a no-op
    - **Property 8: Invalid inputs are a no-op** (NULL font and out-of-range chars leave buffer unchanged)
    - Tag: `Feature: canvas-draw-curved-char, Property 8: Invalid inputs are a no-op`
    - **Validates: Requirements 4.4, 4.5**
  - [-]* 6.10 Property test 9 - deterministic, float-free results
    - **Property 9: Deterministic, float-free results**
    - Tag: `Feature: canvas-draw-curved-char, Property 9: Deterministic, float-free results`
    - **Validates: Requirements 7.2, 7.3**
  - [-]* 6.11 Unit test - directional convention
    - Angles 0, 90, 180, 270 place the anchor to the right, below, left, and above the center respectively.
    - _Requirements: 2.2_
  - [-]* 6.12 Unit test - half-away-from-zero rounding
    - Inputs that land exactly on `.5` offsets round away from zero.
    - _Requirements: 3.3_
  - [-]* 6.13 Unit test - per-font smoke
    - One example per supported font (Font8/12/16/20/24) confirming pixels are written.
    - _Requirements: 4.2, 4.3_
  - [-]* 6.14 Unit test - opaque overlap last-writer-wins
    - Two overlapping opaque characters; the overlap holds the second character's value.
    - _Requirements: 5.4_

- [x] 7. Cross-platform build and static checks
  - [-]* 7.1 Static / structural checks for the platform-agnostic constraint
    - Grep/inspect around `CanvasDrawCurvedChar` to confirm no `#ifdef`, platform macros, function
      pointers, or runtime floating-point operations (trig comes only from the integer LUT).
    - _Requirements: 7.1_
  - [x] 7.2 Cross-platform compile/link verification
    - Build the RP2040 target (cmake + make) and confirm `CanvasDrawCurvedChar` compiles and links from
      the single `Canvas.c`. Verify the ESP32-S3 wiring for parity (function is in the shared
      `Canvas.c`, no target-specific variant); note that ESP32-S3 is parity-only and not locally
      compile-tested in this environment, so flag if a local ESP-IDF build cannot be run.
    - _Requirements: 7.4_

- [x] 8. Final checkpoint - ensure all tests pass
  - Ensure the host property/unit tests pass and the RP2040 build is clean. Ask the user if questions arise.

## Notes

- Tasks marked with `*` are optional (test-related) and can be skipped for a faster MVP; core
  implementation tasks (1.1, 2.1, 3.1, 3.2, 4.1, 6.1, 7.2) are never optional.
- All implementation sub-tasks edit the same `Canvas.c`/`Canvas.h`, so they are scheduled in separate
  waves to avoid write conflicts.
- Property tests map 1:1 to the nine design properties; each is tagged
  `Feature: canvas-draw-curved-char, Property N: ...` and runs 100+ iterations against all five fonts.
- Cross-platform bit-for-bit identity (Requirement 7.2) is argued by construction (integer-only math +
  constant table) and surrogate-tested by Property 9; ESP32-S3 is parity-only and not locally
  compile-tested.

## Task Dependency Graph

```json
{
  "waves": [
    { "id": 0, "tasks": ["1.1"] },
    { "id": 1, "tasks": ["2.1"] },
    { "id": 2, "tasks": ["3.1"] },
    { "id": 3, "tasks": ["3.2"] },
    { "id": 4, "tasks": ["4.1"] },
    { "id": 5, "tasks": ["6.1"] },
    { "id": 6, "tasks": ["6.2", "6.3", "6.4", "6.5", "6.6", "6.7", "6.8", "6.9", "6.10", "6.11", "6.12", "6.13", "6.14"] },
    { "id": 7, "tasks": ["7.1", "7.2"] }
  ]
}
```
