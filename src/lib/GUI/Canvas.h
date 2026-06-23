#ifndef __CANVAS_H
#define __CANVAS_H

#include "Types.h"
#include "fonts.h"

typedef struct {
    UBYTE *Image;
    UWORD Width;
    UWORD Height;
    UWORD WidthMemory;
    UWORD HeightMemory;
    UWORD Color;
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

#define WHITE   0xFFFF
#define BLACK   0x0000
#define BLUE    0x001F
#define BRED    0XF81F
#define GRED    0XFFE0
#define GBLUE   0X07FF
#define RED     0xF800
#define MAGENTA 0xF81F
#define GREEN   0x07E0
#define CYAN    0x7FFF
#define YELLOW  0xFFE0
#define BROWN   0XBC40
#define BRRED   0XFC07
#define GRAY    0X8430

#define IMAGE_BACKGROUND WHITE
#define FONT_FOREGROUND  BLACK
#define FONT_BACKGROUND  WHITE

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
void CanvasNewImage(UBYTE *image, UWORD width, UWORD height, UWORD rotate, UWORD color);
void CanvasSelectImage(UBYTE *image);
void CanvasSetRotate(UWORD rotate);
void CanvasSetScale(UBYTE scale);
void CanvasSetMirroring(UBYTE mirror);
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
void CanvasDrawText(UWORD xStart, UWORD yStart, const char * text,
    sFONT* font, UWORD foregroundColor, UWORD backgroundColor);
void CanvasDrawTextCN(UWORD xStart, UWORD yStart, const char * text, cFONT* font,
    UWORD foregroundColor, UWORD backgroundColor);
void CanvasDrawNum(UWORD xPoint, UWORD yPoint, double number,
    sFONT* font, UWORD digit, UWORD foregroundColor, UWORD backgroundColor);
void CanvasDrawTime(UWORD xStart, UWORD yStart, DateTime *pTime, sFONT* font,
    UWORD foregroundColor, UWORD backgroundColor);
void CanvasDrawImage(const unsigned char *image, UWORD xStart, UWORD yStart, UWORD imageWidth, UWORD imageHeight);
void CanvasDrawImage1(const unsigned char *image, UWORD xStart, UWORD yStart, UWORD imageWidth, UWORD imageHeight);
void CanvasDrawBitmap(const unsigned char* imageBuffer);
void CanvasDrawBitmapBlock(const unsigned char* imageBuffer, UBYTE region);
void CanvasDrawBitmapToArea(unsigned char x, unsigned char y, const unsigned char *pBitmap,
    unsigned char areaWidth, unsigned char areaHeight);

#endif /* __CANVAS_H */