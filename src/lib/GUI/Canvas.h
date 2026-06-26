#ifndef __CANVAS_H
#define __CANVAS_H

#include "Types.h"
#include "fonts.h"
#include "ff.h"

typedef struct {
    UBYTE *Image;
    UWORD Width;
    UWORD Height;
    UWORD WidthMemory;
    UWORD HeightMemory;
    UWORD Rotate;
    UWORD Flip;
    UWORD WidthByte;
    UWORD HeightByte;
    UWORD Scale;
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
    (UWORD)(((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | ((b >> 3) & 0x001F))

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
    UWORD	Year;
    UBYTE Month;
    UBYTE Day;
    UBYTE Hour;
    UBYTE Min;
    UBYTE Sec;
} DateTime;

/* Drawing API — definitions in Canvas.c */
void CanvasNewImage(UBYTE *image, UWORD width, UWORD height, UWORD rotate);
void CanvasSelectImage(UBYTE *image);
void CanvasSetRotate(UWORD rotate);
void CanvasSetScale(UBYTE scale);
void CanvasFlipTexture(UBYTE mirror);
void CanvasSetPixel(UWORD xPoint, UWORD yPoint, UWORD color);
void CanvasClear(UWORD color);
void CanvasClearArea(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd, UWORD color);
void CanvasDrawPoint(UWORD xPoint, UWORD yPoint, UWORD color,
    PixelSize pixelSize, PixelFillStyle pixelFillStyle);
void CanvasDrawLine(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd,
    UWORD color, PixelSize pixelSize, LineStyle lineStyle);
void CanvasDrawRectangle(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd,
    UWORD color, PixelSize lineWidth, DrawFillStyle rectangleFillStyle);
void CanvasDrawCircle(UWORD xCenter, UWORD yCenter, UWORD radius,
    UWORD color, PixelSize lineWidth, DrawFillStyle circleFillStyle);
void CanvasDrawChar(UWORD xPoint, UWORD yPoint, const char ASCIIChar,
    sFONT* font, UWORD foregroundColor, UWORD backgroundColor);
void CanvasDrawCurvedChar(const char ASCIIChar, UWORD xCenter, UWORD yCenter,
    UWORD radius, UWORD startAngle, TextOrientation orientation, sFONT* font,
    UWORD foregroundColor, UWORD backgroundColor);
void CanvasDrawCurvedText(const char *text, UWORD xCenter, UWORD yCenter,
    UWORD radius, UWORD startAngle, TextOrientation orientation, sFONT* font,
    UWORD foregroundColor, UWORD backgroundColor);
void CanvasDrawText(UWORD xStart, UWORD yStart, const char * text,
    sFONT* font, UWORD foregroundColor, UWORD backgroundColor);
void CanvasDrawTextCN(UWORD xStart, UWORD yStart, const char * text, cFONT* font,
    UWORD foregroundColor, UWORD backgroundColor);
void CanvasDrawNum(UWORD xPoint, UWORD yPoint, double number,
    sFONT* font, UWORD digit, UWORD foregroundColor, UWORD backgroundColor);
void CanvasDrawTime(UWORD xStart, UWORD yStart, DateTime *pTime, sFONT* font,
    UWORD foregroundColor, UWORD backgroundColor);
void CanvasDrawPng(FIL *file);
void CanvasDrawImage(const unsigned char *image, UWORD xStart, UWORD yStart, UWORD imageWidth, UWORD imageHeight);
void CanvasDrawBitmap(const unsigned char* imageBuffer);
void CanvasDrawBitmapBlock(const unsigned char* imageBuffer, UBYTE region);
void CanvasDrawBitmapToArea(unsigned char x, unsigned char y, const unsigned char *pBitmap,
    unsigned char areaWidth, unsigned char areaHeight);

#endif /* __CANVAS_H */