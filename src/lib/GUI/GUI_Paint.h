#ifndef __GUI_CANVAS_H
#define __GUI_CANVAS_H

#include "HAL.c"
#include "../Fonts/fonts.h"

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
    PIXEL_FILL_STYLE_AROUND  = 1,   // dot pixel 1 x 1
    PIXEL_FILL_STYLE_RIGHTUP  ,     // dot pixel 2 X 2
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
    UWORD	Year;  //0000
    UBYTE Month; //1 - 12
    UBYTE Day;   //1 - 30
    UBYTE Hour;  //0 - 23
    UBYTE Min;   //0 - 59
    UBYTE Sec;   //0 - 59
} DateTime;

#endif