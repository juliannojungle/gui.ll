#ifndef __CANVAS_H
#define __CANVAS_H

#include "Types.h"
#include "fonts.h"
#include "ff.h"

typedef struct {
    UINT8 *Texture;
    UINT16 Width;
    UINT16 Height;
    UINT16 WidthMemory;
    UINT16 HeightMemory;
    UINT16 Rotate;
    UINT16 Flip;
    UINT16 WidthByte;
    UINT16 HeightByte;
    UINT16 Scale;
} Canvas;
extern Canvas canvas;

typedef enum {
    ROTATE_0 = 0,
    ROTATE_90 = 90,
    ROTATE_180 = 180,
    ROTATE_270 = 270
} Rotate;

typedef enum {
    FLIP_NONE  = 0x00,
    FLIP_HORIZONTAL = 0x01,
    FLIP_VERTICAL = 0x02,
    FLIP_ORIGIN = 0x03,
} Flip;

/* Facing of curved text: INWARDS reads with the baseline on the inner side of the
 * arc and advances clockwise; OUTWARDS flips the glyph and advances counter-clockwise. */
typedef enum {
    TEXT_ORIENTATION_INWARDS = 0,
    TEXT_ORIENTATION_OUTWARDS,
} TextOrientation;

/* converts RGB to RGB565 */
#define RGB_COLOR(r, g, b) \
    (UINT16)(((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | ((b >> 3) & 0x001F))

#define LIGHT_GRAY  RGB_COLOR(200, 200, 200)
#define GRAY        RGB_COLOR(130, 130, 130)
#define DARK_GRAY   RGB_COLOR(80, 80, 80)
#define GOLD        RGB_COLOR(255, 203, 0)
#define ORANGE      RGB_COLOR(255, 161, 0)
#define PINK        RGB_COLOR(255, 109, 194)
#define RED         RGB_COLOR(230, 41, 55)
#define MAROON      RGB_COLOR(190, 33, 55)
#define GREEN       RGB_COLOR(0, 228, 48)
#define LIME        RGB_COLOR(0, 158, 47)
#define DARK_GREEN  RGB_COLOR(0, 117, 44)
#define SKY_BLUE    RGB_COLOR(102, 191, 255)
#define BLUE        RGB_COLOR(0, 121, 241)
#define DARK_BLUE   RGB_COLOR(0, 82, 172)
#define PURPLE      RGB_COLOR(200, 122, 255)
#define VIOLET      RGB_COLOR(135, 60, 190)
#define DARK_PURPLE RGB_COLOR(112, 31, 126)
#define BEIGE       RGB_COLOR(211, 176, 131)
#define BROWN       RGB_COLOR(127, 106, 79)
#define DARK_BROWN  RGB_COLOR(76, 63, 47)
#define WHITE       RGB_COLOR(255, 255, 255)
#define BLACK       RGB_COLOR(0, 0, 0)
#define CYAN        RGB_COLOR(0, 255, 255)
#define MAGENTA     RGB_COLOR(255, 0, 255)
#define YELLOW      RGB_COLOR(255, 255, 0)
/* TRANSPARENT=MAGENTA: classic chroma-key color */
#define TRANSPARENT RGB_COLOR(255, 0, 255)

typedef enum {
    PIXEL_SIZE_1X1  = 1, // 1 x 1
    PIXEL_SIZE_2X2  ,    // 2 X 2
    PIXEL_SIZE_3X3  ,    // 3 X 3
    PIXEL_SIZE_4X4  ,    // 4 X 4
    PIXEL_SIZE_5X5  ,    // 5 X 5
    PIXEL_SIZE_6X6  ,    // 6 X 6
    PIXEL_SIZE_7X7  ,    // 7 X 7
    PIXEL_SIZE_8X8  ,    // 8 X 8
} PixelSize;
#define DEFAULT_PIXEL_SIZE PIXEL_SIZE_1X1

typedef enum {
    PIXEL_FILL_STYLE_AROUND  = 1,
    PIXEL_FILL_STYLE_RIGHTUP  ,
} PixelFillStyle;
#define DEFAULT_PIXEL_FILL_STYLE PIXEL_FILL_STYLE_AROUND

typedef enum {
    LINE_STYLE_SOLID = 0,
    LINE_STYLE_DOTTED,
} LineStyle;

typedef enum {
    DRAW_FILL_STYLE_EMPTY = 0,
    DRAW_FILL_STYLE_FULL,
} DrawFillStyle;

typedef struct {
    UINT16	Year;
    UINT8 Month;
    UINT8 Day;
    UINT8 Hour;
    UINT8 Min;
    UINT8 Sec;
} DateTime;

/* Drawing API — definitions in Canvas.c */
void CanvasNewTexture(UINT8 *texture, UINT16 width, UINT16 height, UINT16 rotate);
void CanvasSelectTexture(UINT8 *texture);
void CanvasSetRotate(UINT16 rotate);
void CanvasSetScale(UINT8 scale);
void CanvasFlipTexture(UINT8 mirror);
void CanvasSetPixel(UINT16 xPoint, UINT16 yPoint, UINT16 color);
void CanvasClear(UINT16 color);
void CanvasClearArea(UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd, UINT16 color);
void CanvasDrawPoint(UINT16 xPoint, UINT16 yPoint, UINT16 color,
    PixelSize pixelSize, PixelFillStyle pixelFillStyle);
void CanvasDrawLine(UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd,
    UINT16 color, PixelSize pixelSize, LineStyle lineStyle);
void CanvasDrawRectangle(UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd,
    UINT16 color, PixelSize lineWidth, DrawFillStyle rectangleFillStyle);
void CanvasDrawCircle(UINT16 xCenter, UINT16 yCenter, UINT16 radius,
    UINT16 color, PixelSize lineWidth, DrawFillStyle circleFillStyle);
void CanvasDrawChar(UINT16 xPoint, UINT16 yPoint, const char ASCIIChar,
    sFONT* font, UINT16 foregroundColor, UINT16 backgroundColor);
void CanvasDrawCurvedChar(const char ASCIIChar, UINT16 xCenter, UINT16 yCenter,
    UINT16 radius, UINT16 startAngle, TextOrientation orientation, sFONT* font,
    UINT16 foregroundColor, UINT16 backgroundColor);
void CanvasDrawCurvedText(const char *text, UINT16 xCenter, UINT16 yCenter,
    UINT16 radius, UINT16 startAngle, TextOrientation orientation, sFONT* font,
    UINT16 foregroundColor, UINT16 backgroundColor);
void CanvasDrawText(UINT16 xStart, UINT16 yStart, const char * text,
    sFONT* font, UINT16 foregroundColor, UINT16 backgroundColor);
void CanvasDrawTextCN(UINT16 xStart, UINT16 yStart, const char * text, cFONT* font,
    UINT16 foregroundColor, UINT16 backgroundColor);
void CanvasDrawNum(UINT16 xPoint, UINT16 yPoint, double number,
    sFONT* font, UINT16 digit, UINT16 foregroundColor, UINT16 backgroundColor);
void CanvasDrawTime(UINT16 xStart, UINT16 yStart, DateTime *pTime, sFONT* font,
    UINT16 foregroundColor, UINT16 backgroundColor);
void CanvasDrawPng(FIL *file);
void CanvasDrawPngToArea(FIL *file, UINT16 xSource, UINT16 ySource, UINT16 width, UINT16 height, UINT16 xTarget, UINT16 yTarget);
void CanvasDrawTexture(const unsigned char *texture, UINT16 xStart, UINT16 yStart, UINT16 imageWidth, UINT16 imageHeight);
void CanvasDrawBitmap(const unsigned char* imageBuffer);
void CanvasDrawBitmapBlock(const unsigned char* imageBuffer, UINT8 region);
void CanvasDrawBitmapToArea(UINT16 x, UINT16 y, const unsigned char *pBitmap,
    UINT16 areaWidth, UINT16 areaHeight);

#endif /* __CANVAS_H */