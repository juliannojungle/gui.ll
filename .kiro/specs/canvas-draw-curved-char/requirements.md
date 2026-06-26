# Requirements Document

## Introduction

This feature adds a new drawing primitive, `CanvasDrawCurvedChar`, to the Canvas module
(`src/lib/GUI/Canvas.c` and `src/lib/GUI/Canvas.h`). The GC9A01 panel is a round 240x240
display, so text placed near the rim reads best when it follows the circular border. This
primitive draws a single ASCII character positioned on a circle of a given radius around a
center point, at a given angle, with the glyph rotated so it stays tangent to the circle's
border. The signature is designed so that future `CanvasDrawCurvedText` and
`CanvasDrawCurvedNumber` functions can reuse this primitive by advancing the angle per glyph.

The primitive reuses the existing `sFONT` font set (Font8/Font12/Font16/Font20/Font24), writes
pixels through the existing `CanvasSetPixel` API, and honors the `TRANSPARENT` color-key
background convention already used by `CanvasDrawChar` (Decision 14). It is platform-agnostic
(RP2040 and ESP32-S3) with no platform `#ifdef`.

## Glossary

- **Canvas**: The in-RAM drawing module (`Canvas.c`/`Canvas.h`) that owns the texture buffer and
  exposes the `Canvas*` drawing API. The system under specification.
- **CanvasDrawCurvedChar**: The new function being specified. Draws one ASCII character on a
  circle, rotated tangent to the circle border.
- **Glyph**: The bitmap representation of a single character within an `sFONT` font table.
- **sFONT**: The existing single-byte font struct (`table`, `Width`, `Height`) defined in
  `src/lib/GUI/Fonts/fonts.h`. The supported instances are Font8, Font12, Font16, Font20, Font24.
- **xCenter / yCenter**: The canvas coordinates of the circle center around which the character
  is placed.
- **radius**: The distance in pixels from the circle center to the anchor point where the
  character is placed.
- **startAngle**: The angular position in whole degrees, in the range 0 to 359, at which the
  character is placed on the circle.
- **Glyph anchor**: The point on the circle (computed from xCenter, yCenter, radius, startAngle)
  that the geometric center of the glyph bounding box maps to.
- **Tangent orientation**: The glyph rotation such that the character baseline is perpendicular
  to the radius line at the placement angle, so the text reads along the circle border.
- **foregroundColor**: The RGB565 color used for glyph (set) pixels.
- **backgroundColor**: The RGB565 color used for non-glyph pixels, or the `TRANSPARENT` sentinel
  to skip non-glyph pixels.
- **TRANSPARENT**: The chroma-key sentinel color (`RGB_COLOR(255, 0, 255)`, magenta) defined in
  `Canvas.h`; when passed as the background, only glyph pixels are written (Decision 14).
- **CanvasSetPixel**: The existing Canvas function that writes a single color value to one
  axis-aligned canvas coordinate and applies clipping, rotation and flip.
- **Nearest-neighbor rotation**: The raster rotation method that maps each destination pixel to
  the closest source glyph pixel by rounding rotated coordinates to the nearest integer.

## Requirements

### Requirement 1: Public API and signature

**User Story:** As an embedded developer, I want a `CanvasDrawCurvedChar` function with a stable
public signature, so that I can draw a single character on the round display and later build
curved text and number helpers on top of the same primitive.

#### Acceptance Criteria

1. THE Canvas SHALL declare a `CanvasDrawCurvedChar` function prototype in `Canvas.h` and define
   the function in `Canvas.c`.
2. THE Canvas SHALL accept the following parameters for `CanvasDrawCurvedChar` with these types:
   the ASCII character as a `const char`, the circle center X coordinate as a `UWORD`, the circle
   center Y coordinate as a `UWORD`, the radius as a `UWORD`, the start angle as a `UWORD`, a
   pointer to an `sFONT` font, the foreground color as a `UWORD` RGB565 value, and the background
   color as a `UWORD` RGB565 value.
3. THE Canvas SHALL order the `CanvasDrawCurvedChar` parameters as ASCII character, center X
   coordinate, center Y coordinate, radius, start angle, font pointer, foreground color, and
   background color, so that a caller can advance the start angle to position consecutive
   characters.
4. THE Canvas SHALL name all `CanvasDrawCurvedChar` parameters in camelCase.
5. THE Canvas SHALL define `CanvasDrawCurvedChar` with a `void` return type, consistent with the
   existing `CanvasDrawChar` function.

### Requirement 2: Circular placement

**User Story:** As an embedded developer, I want a character placed at a specified radius and
angle around a center point, so that I can position text along the round display border.

#### Acceptance Criteria

1. WHEN `CanvasDrawCurvedChar` is called with a center, radius, and start angle, THE Canvas SHALL
   convert the start angle from degrees to radians and compute the glyph anchor position on the
   circle as `xCenter + radius * cos(angle)` and `yCenter + radius * sin(angle)`, rounding each
   result to the nearest integer.
2. THE Canvas SHALL interpret the start angle in whole degrees with 0 degrees at the
   three-o'clock position and increasing clockwise in screen coordinates.
3. WHEN the start angle is outside the range 0 to 359, THE Canvas SHALL normalize the angle into
   the range 0 to 359 inclusive by modulo-360 reduction (for example, -90 normalizes to 270)
   before computing the placement position.
4. THE Canvas SHALL use the geometric center of the glyph bounding box, computed from the `sFONT`
   `Width` and `Height` members, as the glyph-local point that maps to the glyph anchor.
5. WHEN the radius is 0, THE Canvas SHALL place the glyph anchor at the center point
   (xCenter, yCenter).

### Requirement 3: Tangent rotation

**User Story:** As an embedded developer, I want the character rotated so it sits tangent to the
circle border, so that the text follows the curve and reads naturally along the rim.

#### Acceptance Criteria

1. WHEN `CanvasDrawCurvedChar` renders a glyph, THE Canvas SHALL rotate each glyph pixel about the
   glyph anchor computed per Requirement 2 by a rotation angle equal to the start angle, expressed
   in whole degrees and measured clockwise using the same angle convention as Requirement 2, so
   that the glyph baseline is tangent to the circle at the start angle.
2. THE Canvas SHALL use, as the glyph-local point that coincides with the glyph anchor before
   rotation, the same glyph-box reference origin that `CanvasDrawChar` uses for the same character
   and font, so that placement is consistent between the two functions.
3. WHEN `CanvasDrawCurvedChar` maps a rotated glyph pixel to a canvas coordinate, THE Canvas SHALL
   round each rotated coordinate component to the nearest integer, rounding halves away from zero,
   before writing the pixel.
4. WHEN the start angle is 0 degrees, THE Canvas SHALL apply a rotation of 0 degrees and produce a
   glyph whose orientation is identical, ignoring translation, to the upright `CanvasDrawChar`
   output for the same character and font. WHERE the font has an even `Width` or `Height`, THE
   Canvas MAY produce angle-0 output that differs from `CanvasDrawChar` by at most 1 pixel along
   the center seam, as an inherent consequence of round-half-away rotation about the glyph
   geometric center (Requirement 3.3).

### Requirement 4: Font support

**User Story:** As an embedded developer, I want curved characters to use the existing fonts, so
that curved text matches the rest of the UI without new font assets.

#### Acceptance Criteria

1. WHEN `CanvasDrawCurvedChar` renders a glyph, THE Canvas SHALL index the glyph from the `sFONT`
   table using the first printable ASCII character (space, 0x20) as the table origin, matching the
   indexing used by `CanvasDrawChar`.
2. THE Canvas SHALL support the Font8, Font12, Font16, Font20, and Font24 `sFONT` instances for
   `CanvasDrawCurvedChar`.
3. THE Canvas SHALL read glyph dimensions from the `sFONT` `Width` and `Height` members and SHALL
   NOT use any hardcoded glyph dimension.
4. IF the font pointer is NULL, THEN THE Canvas SHALL skip drawing and leave the canvas buffer
   unchanged, without accessing any glyph data.
5. IF the ASCII character is outside the range space (0x20) through tilde (0x7E) inclusive, THEN
   THE Canvas SHALL skip drawing that character and leave the canvas buffer unchanged.

### Requirement 5: Color and transparency

**User Story:** As an embedded developer, I want curved characters to support an opaque
background or a transparent background, so that I can draw text over a rendered image the same
way the existing character API allows.

#### Acceptance Criteria

1. WHEN `CanvasDrawCurvedChar` renders a glyph, THE Canvas SHALL write each glyph (set) pixel
   through `CanvasSetPixel` using the foreground color value as an RGB565 color.
2. IF the background color equals `TRANSPARENT`, THEN THE Canvas SHALL write only the glyph (set)
   pixels through `CanvasSetPixel` and SHALL skip every non-glyph pixel, leaving the existing
   canvas buffer content at the skipped coordinates unchanged.
3. IF the background color does not equal `TRANSPARENT`, THEN THE Canvas SHALL write every
   non-glyph pixel of the rotated glyph bounding box through `CanvasSetPixel` using the background
   color value, where the rotated glyph bounding box is the set of canvas coordinates produced by
   applying the tangent rotation to the glyph's `Width` by `Height` rectangle.
4. WHILE the background color does not equal `TRANSPARENT`, THE Canvas SHALL write the background
   color to non-glyph pixels of the current character's rotated glyph bounding box even where that
   bounding box overlaps a previously drawn character's pixels, so that the most recently drawn
   character determines the final value of any overlapping coordinate.

### Requirement 6: Bounds safety

**User Story:** As an embedded developer, I want curved drawing to stay within the texture, so
that placing a character near the border never corrupts memory outside the canvas buffer.

#### Acceptance Criteria

1. THE Canvas SHALL write every pixel produced by `CanvasDrawCurvedChar` through `CanvasSetPixel`,
   so that the existing clipping, rotation, and flip handling is applied to each pixel.
2. IF a rotated glyph pixel maps to a target column greater than `canvas.Width` or a target row
   greater than `canvas.Height`, THEN THE Canvas SHALL skip that pixel and continue rendering the
   remaining glyph pixels, leaving the canvas buffer unmodified for the skipped pixel.
3. IF the rotation transform produces a negative intermediate column or row coordinate before
   translation, THEN THE Canvas SHALL treat that pixel as out of bounds and skip it, so that no
   negative value is converted to an unsigned `UWORD` coordinate and passed to `CanvasSetPixel`.
4. WHEN `CanvasDrawCurvedChar` finishes rendering a character at any placement, including
   placements adjacent to or beyond the canvas border, THE Canvas SHALL write only within the
   canvas buffer bounded by `canvas.WidthMemory` and `canvas.HeightMemory` and SHALL NOT write to
   any address outside that buffer.

### Requirement 7: Platform-agnostic implementation

**User Story:** As a maintainer, I want the curved-character code to compile and behave
identically on both target platforms, so that the single codebase stays portable.

#### Acceptance Criteria

1. THE Canvas SHALL implement `CanvasDrawCurvedChar` from a single shared source file without any
   platform-specific code paths, including `#ifdef` branches, conditional compilation on platform
   macros, function pointers, or virtual methods used for platform selection or optimization.
2. WHEN `CanvasDrawCurvedChar` is called with identical inputs (character, center, radius, start
   angle, font, foreground color, and background color) on the RP2040 and the ESP32-S3 targets,
   THE Canvas SHALL write a bit-for-bit identical set of canvas pixel coordinates and per-pixel
   color values on both targets.
3. WHEN `CanvasDrawCurvedChar` computes the glyph anchor position and the tangent rotation, THE
   Canvas SHALL use a trigonometric computation method that yields identical integer pixel
   coordinates for identical inputs on the RP2040 and the ESP32-S3 targets, independent of each
   target's floating-point hardware.
4. WHEN the project is built for the RP2040 target and for the ESP32-S3 target, THE Canvas SHALL
   compile and link `CanvasDrawCurvedChar` from the same shared source file without errors and
   without target-specific source variants.
