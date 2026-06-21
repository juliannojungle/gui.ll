#include "GUI_Paint.h"
#include "HAL.c"
// #include "Debug.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h> //memset()
#include <math.h>

Canvas canvas;

/******************************************************************************
function: Create Image
parameter:
    image   :   Pointer to the image cache
    width   :   The width of the picture
    Height  :   The height of the picture
    Color   :   Whether the picture is inverted
******************************************************************************/
void CanvasNewImage(UBYTE *image, UWORD Width, UWORD Height, UWORD Rotate, UWORD Color) {
    canvas.Image = NULL;
    canvas.Image = image;

    canvas.WidthMemory = Width;
    canvas.HeightMemory = Height;
    canvas.Color = Color;
    canvas.Scale = 2;

    canvas.WidthByte = (Width % 8 == 0)? (Width / 8 ): (Width / 8 + 1);
    canvas.HeightByte = Height;
//    printf("WidthByte = %d, HeightByte = %d\r\n", canvas.WidthByte, canvas.HeightByte);
//    printf(" LCD_WIDTH / 8 = %d\r\n",  122 / 8);

    canvas.Rotate = Rotate;
    canvas.Flip = FLIP_NONE;

    if (Rotate == ROTATE_0 || Rotate == ROTATE_180) {
        canvas.Width = Width;
        canvas.Height = Height;
    } else {
        canvas.Width = Height;
        canvas.Height = Width;
    }
}

/******************************************************************************
function: Select Image
parameter:
    image : Pointer to the image cache
******************************************************************************/
void CanvasSelectImage(UBYTE *image) {
    canvas.Image = image;
}

/******************************************************************************
function: Select Image Rotate
parameter:
    Rotate : 0,90,180,270
******************************************************************************/
void CanvasSetRotate(UWORD Rotate) {
    if (Rotate == ROTATE_0 || Rotate == ROTATE_90 || Rotate == ROTATE_180 || Rotate == ROTATE_270) {
        // Debug("Set image Rotate %d\r\n", Rotate);
        canvas.Rotate = Rotate;
    } //else {
        // Debug("rotate = 0, 90, 180, 270\r\n");
    // }
}

void CanvasSetScale(UBYTE scale) {
    switch (scale) {
        case 2: {
            canvas.Scale = scale;
            canvas.WidthByte = (canvas.WidthMemory % 8 == 0) ? (canvas.WidthMemory / 8 ) : (canvas.WidthMemory / 8 + 1);
        } break;
        case 4: {
            canvas.Scale = scale;
            canvas.WidthByte = (canvas.WidthMemory % 4 == 0) ? (canvas.WidthMemory / 4 ) : (canvas.WidthMemory / 4 + 1);
        } break;
        case 16: {
            canvas.Scale = scale;
            canvas.WidthByte = (canvas.WidthMemory % 2 == 0) ? (canvas.WidthMemory / 2) : (canvas.WidthMemory / 2 + 1);
        } break;
        case 65: {
            canvas.Scale = scale;
            canvas.WidthByte = canvas.WidthMemory * 2;
        } break;
        default: {
            // Debug("Set Scale Input parameter error\r\n");
            // Debug("Scale Only support: 2 4 16 65\r\n");
        }
    }
}

/******************************************************************************
function:    Select Image mirror
parameter:
    mirror   :Not mirror,Horizontal mirror,Vertical mirror,Origin mirror
******************************************************************************/
void CanvasSetMirroring(UBYTE mirror) {
    if (mirror == FLIP_NONE || mirror == FLIP_HORIZONTAL ||
        mirror == FLIP_VERTICAL || mirror == FLIP_ORIGIN) {
        // Debug("mirror image x:%s, y:%s\r\n", (mirror & 0x01) ? "mirror" : "none", ((mirror >> 1) & 0x01) ? "mirror" : "none");
        canvas.Flip = mirror;
    }// else {
        // Debug("mirror should be FLIP_NONE, FLIP_HORIZONTAL, \
        // FLIP_VERTICAL or FLIP_ORIGIN\r\n");
    // }
}

/******************************************************************************
function: Draw Pixels
parameter:
    Xpoint : At point X
    Ypoint : At point Y
    Color  : Painted colors
******************************************************************************/
void CanvasSetPixel(UWORD Xpoint, UWORD Ypoint, UWORD Color) {
    if (Xpoint > canvas.Width || Ypoint > canvas.Height) {
        // Debug("Exceeding display boundaries\r\n");
        return;
    }

    UWORD X, Y;

    switch (canvas.Rotate) {
        case 0:
            X = Xpoint;
            Y = Ypoint;
            break;
        case 90:
            X = canvas.WidthMemory - Ypoint - 1;
            Y = Xpoint;
            break;
        case 180:
            X = canvas.WidthMemory - Xpoint - 1;
            Y = canvas.HeightMemory - Ypoint - 1;
            break;
        case 270:
            X = Ypoint;
            Y = canvas.HeightMemory - Xpoint - 1;
            break;
        default: return;
    }

    switch (canvas.Flip) {
        case FLIP_NONE:
            break;
        case FLIP_HORIZONTAL:
            X = canvas.WidthMemory - X - 1;
            break;
        case FLIP_VERTICAL:
            Y = canvas.HeightMemory - Y - 1;
            break;
        case FLIP_ORIGIN:
            X = canvas.WidthMemory - X - 1;
            Y = canvas.HeightMemory - Y - 1;
            break;
        default: return;
    }

    if (X > canvas.WidthMemory || Y > canvas.HeightMemory) {
        // Debug("Exceeding display boundaries\r\n");
        return;
    }

    if (canvas.Scale == 2) {
        UDOUBLE Addr = X / 8 + Y * canvas.WidthByte;
        UBYTE Rdata = canvas.Image[Addr];
        if (Color &0xff == BLACK)
            canvas.Image[Addr] = Rdata & ~(0x80 >> (X % 8));
        else
            canvas.Image[Addr] = Rdata | (0x80 >> (X % 8));
    } else if (canvas.Scale == 4) {
        UDOUBLE Addr = X / 4 + Y * canvas.WidthByte;
        Color = Color % 4; //Guaranteed color scale is 4  --- 0~3
        UBYTE Rdata = canvas.Image[Addr];

        Rdata = Rdata & (~(0xC0 >> ((X % 4) * 2)));
        canvas.Image[Addr] = Rdata | ((Color << 6) >> ((X % 4) * 2));
    } else if (canvas.Scale == 16) {
        UDOUBLE Addr = X / 2 + Y * canvas.WidthByte;
        UBYTE Rdata = canvas.Image[Addr];
        Color = Color % 16;
        Rdata = Rdata & (~(0xf0 >> ((X % 2) * 4)));
        canvas.Image[Addr] = Rdata | ((Color << 4) >> ((X % 2) * 4));
    } else if (canvas.Scale == 65) {
        UDOUBLE Addr = X * 2 + Y * canvas.WidthByte;
        canvas.Image[Addr] = 0xff & (Color >> 8);
        canvas.Image[Addr + 1] = 0xff & Color;
    }
}

/******************************************************************************
function: Clear the color of the picture
parameter:
    Color : Painted colors
******************************************************************************/
void CanvasClear(UWORD Color) {
    if (canvas.Scale == 2 || canvas.Scale == 4) {
        for (UWORD Y = 0; Y < canvas.HeightByte; Y++) {
            for (UWORD X = 0; X < canvas.WidthByte; X++) {//8 pixel =  1 byte
                UDOUBLE Addr = X + Y * canvas.WidthByte;
                canvas.Image[Addr] = Color;
            }
        }
    } else if (canvas.Scale == 16) {
        for (UWORD Y = 0; Y < canvas.HeightByte; Y++) {
            for (UWORD X = 0; X < canvas.WidthByte; X++ ) {//8 pixel =  1 byte
                UDOUBLE Addr = X + Y * canvas.WidthByte;
                Color = Color & 0x0f;
                canvas.Image[Addr] = (Color << 4) | Color;
            }
        }
    } else if (canvas.Scale == 65) {
        for (UWORD Y = 0; Y < canvas.HeightByte; Y++) {
            for (UWORD X = 0; X < canvas.WidthByte; X++) {//8 pixel =  1 byte
                UDOUBLE Addr = X * 2 + Y * canvas.WidthByte;
                canvas.Image[Addr] = 0xff & (Color >> 8);
                canvas.Image[Addr+1] = 0xff & Color;
            }
        }
    }
}

/******************************************************************************
function: Clear the color of a window
parameter:
    Xstart : x starting point
    Ystart : Y starting point
    Xend   : x end point
    Yend   : y end point
    Color  : Painted colors
******************************************************************************/
void CanvasClearArea(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color) {
    UWORD X, Y;
    for (Y = Ystart; Y < Yend; Y++) {
        for (X = Xstart; X < Xend; X++) {//8 pixel =  1 byte
            CanvasSetPixel(X, Y, Color);
        }
    }
}

/******************************************************************************
function: Draw Point(Xpoint, Ypoint) Fill the color
parameter:
    Xpoint        : The Xpoint coordinate of the point
    Ypoint        : The Ypoint coordinate of the point
    Color        : Painted color
    Dot_Pixel    : point size
    Dot_Style    : point Style
******************************************************************************/
void CanvasDrawPoint(UWORD Xpoint, UWORD Ypoint, UWORD Color,
    PixelSize Dot_Pixel, PixelFillStyle Dot_Style) {
    if (Xpoint > canvas.Width || Ypoint > canvas.Height) {
        // Debug("CanvasDrawPoint Input exceeds the normal display range\r\n");
        // printf("Xpoint = %d , canvas.Width = %d  \r\n ",Xpoint,canvas.Width);
        // printf("Ypoint = %d , canvas.Height = %d  \r\n ",Ypoint,canvas.Height);
        return;
    }

    int16_t XDir_Num , YDir_Num;
    if (Dot_Style == PIXEL_FILL_STYLE_AROUND) {
        for (XDir_Num = 0; XDir_Num < 2 * Dot_Pixel - 1; XDir_Num++) {
            for (YDir_Num = 0; YDir_Num < 2 * Dot_Pixel - 1; YDir_Num++) {
                if (Xpoint + XDir_Num - Dot_Pixel < 0 || Ypoint + YDir_Num - Dot_Pixel < 0)
                    break;
                // // printf("x = %d, y = %d\r\n", Xpoint + XDir_Num - Dot_Pixel, Ypoint + YDir_Num - Dot_Pixel);
                CanvasSetPixel(Xpoint + XDir_Num - Dot_Pixel, Ypoint + YDir_Num - Dot_Pixel, Color);
            }
        }
    } else {
        for (XDir_Num = 0; XDir_Num <  Dot_Pixel; XDir_Num++) {
            for (YDir_Num = 0; YDir_Num <  Dot_Pixel; YDir_Num++) {
                CanvasSetPixel(Xpoint + XDir_Num - 1, Ypoint + YDir_Num - 1, Color);
            }
        }
    }
}

/******************************************************************************
function: Draw a line of arbitrary slope
parameter:
    Xstart ：Starting Xpoint point coordinates
    Ystart ：Starting Xpoint point coordinates
    Xend   ：End point Xpoint coordinate
    Yend   ：End point Ypoint coordinate
    Color  ：The color of the line segment
    Line_width : Line width
    Line_Style: Solid and dotted lines
******************************************************************************/
void CanvasDrawLine(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend,
    UWORD Color, PixelSize Line_width, LineStyle Line_Style) {
    if (Xstart > canvas.Width || Ystart > canvas.Height ||
        Xend > canvas.Width || Yend > canvas.Height) {
        // Debug("CanvasDrawLine Input exceeds the normal display range\r\n");
        return;
    }

    UWORD Xpoint = Xstart;
    UWORD Ypoint = Ystart;
    int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
    int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;

    // Increment direction, 1 is positive, -1 is counter;
    int XAddway = Xstart < Xend ? 1 : -1;
    int YAddway = Ystart < Yend ? 1 : -1;

    //Cumulative error
    int Esp = dx + dy;
    char Dotted_Len = 0;

    for (;;) {
        Dotted_Len++;
        //Painted dotted line, 2 point is really virtual
        if (Line_Style == LINE_STYLE_DOTTED && Dotted_Len % 3 == 0) {
            //Debug("LINE_DOTTED\r\n");
            if(Color)
                CanvasDrawPoint(Xpoint, Ypoint, BLACK, Line_width, DEFAULT_PIXEL_FILL_STYLE);
            else
                CanvasDrawPoint(Xpoint, Ypoint, WHITE, Line_width, DEFAULT_PIXEL_FILL_STYLE);
            Dotted_Len = 0;
        } else {
            CanvasDrawPoint(Xpoint, Ypoint, Color, Line_width, DEFAULT_PIXEL_FILL_STYLE);
        }

        if (2 * Esp >= dy) {
            if (Xpoint == Xend)
                break;
            Esp += dy;
            Xpoint += XAddway;
        }

        if (2 * Esp <= dx) {
            if (Ypoint == Yend)
                break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

/******************************************************************************
function: Draw a rectangle
parameter:
    Xstart ：Rectangular  Starting Xpoint point coordinates
    Ystart ：Rectangular  Starting Xpoint point coordinates
    Xend   ：Rectangular  End point Xpoint coordinate
    Yend   ：Rectangular  End point Ypoint coordinate
    Color  ：The color of the Rectangular segment
    Line_width: Line width
    Draw_Fill : Whether to fill the inside of the rectangle
******************************************************************************/
void CanvasDrawRectangle(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend,
    UWORD Color, PixelSize Line_width, DrawFillStyle Draw_Fill) {
    if (Xstart > canvas.Width || Ystart > canvas.Height ||
        Xend > canvas.Width || Yend > canvas.Height) {
        // Debug("Input exceeds the normal display range\r\n");
        return;
    }

    if (Draw_Fill) {
        UWORD Ypoint;
        for(Ypoint = Ystart; Ypoint < Yend; Ypoint++) {
            CanvasDrawLine(Xstart, Ypoint, Xend, Ypoint, Color , Line_width, LINE_STYLE_SOLID);
        }
    } else {
        CanvasDrawLine(Xstart, Ystart, Xend, Ystart, Color, Line_width, LINE_STYLE_SOLID);
        CanvasDrawLine(Xstart, Ystart, Xstart, Yend, Color, Line_width, LINE_STYLE_SOLID);
        CanvasDrawLine(Xend, Yend, Xend, Ystart, Color, Line_width, LINE_STYLE_SOLID);
        CanvasDrawLine(Xend, Yend, Xstart, Yend, Color, Line_width, LINE_STYLE_SOLID);
    }
}

/******************************************************************************
function: Use the 8-point method to draw a circle of the
            specified size at the specified position->
parameter:
    X_Center  ：Center X coordinate
    Y_Center  ：Center Y coordinate
    Radius    ：circle Radius
    Color     ：The color of the ：circle segment
    Line_width: Line width
    Draw_Fill : Whether to fill the inside of the Circle
******************************************************************************/
void CanvasDrawCircle(UWORD X_Center, UWORD Y_Center, UWORD Radius,
    UWORD Color, PixelSize Line_width, DrawFillStyle Draw_Fill) {
    if (X_Center > canvas.Width || Y_Center >= canvas.Height) {
        // Debug("CanvasDrawCircle Input exceeds the normal display range\r\n");
        return;
    }

    //Draw a circle from(0, R) as a starting point
    int16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;

    //Cumulative error,judge the next point of the logo
    int16_t Esp = 3 - (Radius << 1 );

    int16_t sCountY;
    if (Draw_Fill == DRAW_FILL_STYLE_FULL) {
        while (XCurrent <= YCurrent ) { //Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ ) {
                CanvasDrawPoint(X_Center + XCurrent, Y_Center + sCountY, Color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//1
                CanvasDrawPoint(X_Center - XCurrent, Y_Center + sCountY, Color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//2
                CanvasDrawPoint(X_Center - sCountY, Y_Center + XCurrent, Color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//3
                CanvasDrawPoint(X_Center - sCountY, Y_Center - XCurrent, Color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//4
                CanvasDrawPoint(X_Center - XCurrent, Y_Center - sCountY, Color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//5
                CanvasDrawPoint(X_Center + XCurrent, Y_Center - sCountY, Color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//6
                CanvasDrawPoint(X_Center + sCountY, Y_Center - XCurrent, Color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//7
                CanvasDrawPoint(X_Center + sCountY, Y_Center + XCurrent, Color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
            }

            if (Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }

            XCurrent ++;
        }
    } else { //Draw a hollow circle
        while (XCurrent <= YCurrent ) {
            CanvasDrawPoint(X_Center + XCurrent, Y_Center + YCurrent, Color, Line_width, DEFAULT_PIXEL_FILL_STYLE);//1
            CanvasDrawPoint(X_Center - XCurrent, Y_Center + YCurrent, Color, Line_width, DEFAULT_PIXEL_FILL_STYLE);//2
            CanvasDrawPoint(X_Center - YCurrent, Y_Center + XCurrent, Color, Line_width, DEFAULT_PIXEL_FILL_STYLE);//3
            CanvasDrawPoint(X_Center - YCurrent, Y_Center - XCurrent, Color, Line_width, DEFAULT_PIXEL_FILL_STYLE);//4
            CanvasDrawPoint(X_Center - XCurrent, Y_Center - YCurrent, Color, Line_width, DEFAULT_PIXEL_FILL_STYLE);//5
            CanvasDrawPoint(X_Center + XCurrent, Y_Center - YCurrent, Color, Line_width, DEFAULT_PIXEL_FILL_STYLE);//6
            CanvasDrawPoint(X_Center + YCurrent, Y_Center - XCurrent, Color, Line_width, DEFAULT_PIXEL_FILL_STYLE);//7
            CanvasDrawPoint(X_Center + YCurrent, Y_Center + XCurrent, Color, Line_width, DEFAULT_PIXEL_FILL_STYLE);//0

            if (Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    }
}

/******************************************************************************
function: Show English characters
parameter:
    Xpoint           ：X coordinate
    Ypoint           ：Y coordinate
    Acsii_Char       ：To display the English characters
    Font             ：A structure pointer that displays a character size
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
void CanvasDrawChar(UWORD Xpoint, UWORD Ypoint, const char Acsii_Char,
    sFONT* Font, UWORD Color_Foreground, UWORD Color_Background) {
    UWORD Page, Column;

    if (Xpoint > canvas.Width || Ypoint > canvas.Height) {
        // Debug("CanvasDrawChar Input exceeds the normal display range\r\n");
        return;
    }

    uint32_t Char_Offset = (Acsii_Char - ' ') * Font->Height * (Font->Width / 8 + (Font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &Font->table[Char_Offset];

    for (Page = 0; Page < Font->Height; Page ++ ) {
        for (Column = 0; Column < Font->Width; Column ++ ) {
            //To determine whether the font background color and screen background color is consistent
            if (FONT_BACKGROUND == Color_Background) { //this process is to speed up the scan
                if (*ptr & (0x80 >> (Column % 8)))
                    CanvasSetPixel(Xpoint + Column, Ypoint + Page, Color_Foreground);
                    // CanvasDrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
            } else {
                if (*ptr & (0x80 >> (Column % 8))) {
                    CanvasSetPixel(Xpoint + Column, Ypoint + Page, Color_Foreground);
                    // CanvasDrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
                } else {
                    CanvasSetPixel(Xpoint + Column, Ypoint + Page, Color_Background);
                    // CanvasDrawPoint(Xpoint + Column, Ypoint + Page, Color_Background, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
                }
            }

            //One pixel is 8 bits
            if (Column % 8 == 7)
                ptr++;
        }// Write a line

        if (Font->Width % 8 != 0)
            ptr++;
    }// Write all
}

/******************************************************************************
function:    Display the string
parameter:
    Xstart           ：X coordinate
    Ystart           ：Y coordinate
    pString          ：The first address of the English string to be displayed
    Font             ：A structure pointer that displays a character size
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
void CanvasDrawStringEN(UWORD Xstart, UWORD Ystart, const char * pString,
    sFONT* Font, UWORD Color_Foreground, UWORD Color_Background) {
    UWORD Xpoint = Xstart;
    UWORD Ypoint = Ystart;

    if (Xstart > canvas.Width || Ystart > canvas.Height) {
        // Debug("CanvasDrawStringEN Input exceeds the normal display range\r\n");
        return;
    }

    while (* pString != '\0') {
        //if X direction filled , reposition to(Xstart,Ypoint),Ypoint is Y direction plus the Height of the character
        if ((Xpoint + Font->Width ) > canvas.Width ) {
            Xpoint = Xstart;
            Ypoint += Font->Height;
        }

        // If the Y direction is full, reposition to(Xstart, Ystart)
        if ((Ypoint  + Font->Height ) > canvas.Height ) {
            Xpoint = Xstart;
            Ypoint = Ystart;
        }

        CanvasDrawChar(Xpoint, Ypoint, * pString, Font, Color_Foreground, Color_Background);

        //The next character of the address
        pString++;

        //The next word of the abscissa increases the font of the broadband
        Xpoint += Font->Width;
    }
}


/******************************************************************************
function: Display the string
parameter:
    Xstart  ：X coordinate
    Ystart  ：Y coordinate
    pString ：The first address of the Chinese string and English
              string to be displayed
    Font    ：A structure pointer that displays a character size
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
void CanvasDrawStringCN(UWORD Xstart, UWORD Ystart, const char * pString, cFONT* font,
    UWORD Color_Foreground, UWORD Color_Background) {
    const char* p_text = pString;
    int x = Xstart, y = Ystart;
    int i, j, Num;

    /* Send the string character by character on EPD */
    while (*p_text != 0) {
        if (*p_text <= 0x7F) {  //ASCII < 126
            for (Num = 0; Num < font->size; Num++) {
                if (*p_text== font->table[Num].index[0]) {
                    const char* ptr = &font->table[Num].matrix[0];

                    for (j = 0; j < font->Height; j++) {
                        for (i = 0; i < font->Width; i++) {
                            if (FONT_BACKGROUND == Color_Background) { //this process is to speed up the scan
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(x + i, y + j, Color_Foreground);
                                    // CanvasDrawPoint(x + i, y + j, Color_Foreground, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
                                }
                            } else {
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(x + i, y + j, Color_Foreground);
                                    // CanvasDrawPoint(x + i, y + j, Color_Foreground, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
                                } else {
                                    CanvasSetPixel(x + i, y + j, Color_Background);
                                    // CanvasDrawPoint(x + i, y + j, Color_Background, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
                                }
                            }

                            if (i % 8 == 7) {
                                ptr++;
                            }
                        }

                        if (font->Width % 8 != 0) {
                            ptr++;
                        }
                    }

                    break;
                }
            }

            /* Point on the next character */
            p_text += 1;
            /* Decrement the column position by 16 */
            x += font->ASCII_Width;
        } else {        //Chinese
            for (Num = 0; Num < font->size; Num++) {
                if ((*p_text== font->table[Num].index[0]) && (*(p_text + 1) == font->table[Num].index[1])) {
                    const char* ptr = &font->table[Num].matrix[0];

                    for (j = 0; j < font->Height; j++) {
                        for (i = 0; i < font->Width; i++) {
                            if (FONT_BACKGROUND == Color_Background) { //this process is to speed up the scan
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(x + i, y + j, Color_Foreground);
                                    // CanvasDrawPoint(x + i, y + j, Color_Foreground, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
                                }
                            } else {
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(x + i, y + j, Color_Foreground);
                                    // CanvasDrawPoint(x + i, y + j, Color_Foreground, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
                                } else {
                                    CanvasSetPixel(x + i, y + j, Color_Background);
                                    // CanvasDrawPoint(x + i, y + j, Color_Background, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
                                }
                            }

                            if (i % 8 == 7) {
                                ptr++;
                            }
                        }

                        if (font->Width % 8 != 0) {
                            ptr++;
                        }
                    }

                    break;
                }
            }

            /* Point on the next character */
            p_text += 2;
            /* Decrement the column position by 16 */
            x += font->Width;
        }
    }
}

/******************************************************************************
function:    Display nummber
parameter:
    Xstart           ：X coordinate
    Ystart           : Y coordinate
    Nummber          : The number displayed
    Font             ：A structure pointer that displays a character size
    Digit                         : Fractional width
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
#define  ARRAY_LEN 255
void CanvasDrawNum(UWORD Xpoint, UWORD Ypoint, double Nummber,
    sFONT* Font, UWORD Digit,UWORD Color_Foreground, UWORD Color_Background) {
    int16_t Num_Bit = 0, Str_Bit = 0;
    uint8_t Str_Array[ARRAY_LEN] = {0}, Num_Array[ARRAY_LEN] = {0};
    uint8_t *pStr = Str_Array;
    int temp = Nummber;
    float decimals;
    uint8_t i;

    if (Xpoint > canvas.Width || Ypoint > canvas.Height) {
        // Debug("Paint_DisNum Input exceeds the normal display range\r\n");
        return;
    }

    if (Digit > 0) {
        decimals = Nummber - temp;

        for(i = Digit; i > 0; i--) {
            decimals*=10;
        }

        temp = decimals;

        //Converts a number to a string
        for(i=Digit; i>0; i--) {
            Num_Array[Num_Bit] = temp % 10 + '0';
            Num_Bit++;
            temp /= 10;
        }

        Num_Array[Num_Bit] = '.';
        Num_Bit++;
    }

    temp = Nummber;

    //Converts a number to a string
    while (temp) {
        Num_Array[Num_Bit] = temp % 10 + '0';
        Num_Bit++;
        temp /= 10;
    }

    //The string is inverted
    while (Num_Bit > 0) {
        Str_Array[Str_Bit] = Num_Array[Num_Bit - 1];
        Str_Bit ++;
        Num_Bit --;
    }

    //show
    CanvasDrawStringEN(Xpoint, Ypoint, (const char*)pStr, Font, Color_Foreground, Color_Background);
}

/******************************************************************************
function:    Display time
parameter:
    Xstart           ：X coordinate
    Ystart           : Y coordinate
    pTime            : Time-related structures
    Font             ：A structure pointer that displays a character size
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
void CanvasDrawTime(UWORD Xstart, UWORD Ystart, DateTime *pTime, sFONT* Font,
    UWORD Color_Foreground, UWORD Color_Background) {
    uint8_t value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    UWORD Dx = Font->Width;

    //Write data into the cache
    CanvasDrawChar(Xstart, Ystart, value[pTime->Hour / 10], Font, Color_Foreground, Color_Background);
    CanvasDrawChar(Xstart + Dx, Ystart, value[pTime->Hour % 10], Font, Color_Foreground, Color_Background);
    CanvasDrawChar(Xstart + Dx  + Dx / 4 + Dx / 2, Ystart, ':', Font, Color_Foreground, Color_Background);
    CanvasDrawChar(Xstart + Dx * 2 + Dx / 2, Ystart, value[pTime->Min / 10], Font, Color_Foreground, Color_Background);
    CanvasDrawChar(Xstart + Dx * 3 + Dx / 2, Ystart, value[pTime->Min % 10], Font, Color_Foreground, Color_Background);
    CanvasDrawChar(Xstart + Dx * 4 + Dx / 2 - Dx / 4, Ystart, ':', Font, Color_Foreground, Color_Background);
    CanvasDrawChar(Xstart + Dx * 5, Ystart, value[pTime->Sec / 10], Font, Color_Foreground, Color_Background);
    CanvasDrawChar(Xstart + Dx * 6, Ystart, value[pTime->Sec % 10], Font, Color_Foreground, Color_Background);
}

void CanvasDrawImage(const unsigned char *image, UWORD xStart, UWORD yStart, UWORD W_Image, UWORD H_Image) {
    int i, j;
    for (j = 0; j < H_Image; j++) {
        for (i = 0; i < W_Image; i++) {
            if (xStart + i < canvas.WidthMemory && yStart + j < canvas.HeightMemory) //Exceeded part does not display
                CanvasSetPixel(xStart + i, yStart + j, (*(image + j * W_Image * 2 + i * 2 + 1)) << 8 | (*(image + j * W_Image * 2 + i * 2)));
            //Using arrays is a property of sequential storage, accessing the original array by algorithm
            //j*W_Image*2                Y offset
            //i*2                     X offset
        }
    }
}

void CanvasDrawImage1(const unsigned char *image, UWORD xStart, UWORD yStart, UWORD W_Image, UWORD H_Image) {
    int i, j;
    for (j = 0; j < H_Image; j++) {
        for (i = 0; i < W_Image; i++) {
            if (xStart + i < canvas.HeightMemory  &&  yStart + j < canvas.WidthMemory) //Exceeded part does not display
                CanvasSetPixel(xStart + i, yStart + j, (*(image + j * W_Image * 2 + i * 2 + 1)) << 8 | (*(image + j * W_Image * 2 + i * 2)));
            //Using arrays is a property of sequential storage, accessing the original array by algorithm
            //j*W_Image*2                Y offset
            //i*2                     X offset
        }
    }
}

/******************************************************************************
function:    Display monochrome bitmap
parameter:
    image_buffer ：A picture data converted to a bitmap
info:
    Use a computer to convert the image into a corresponding array,
    and then embed the array directly into Imagedata.cpp as a .c file.
******************************************************************************/
void CanvasDrawBitmap(const unsigned char* image_buffer) {
    UWORD x, y;
    UDOUBLE Addr = 0;

    for (y = 0; y < canvas.HeightByte; y++) {
        for (x = 0; x < canvas.WidthByte; x++) {//8 pixel =  1 byte
            Addr = x + y * canvas.WidthByte;
            canvas.Image[Addr] = (unsigned char)image_buffer[Addr];
        }
    }
}

void CanvasDrawBitmapBlock(const unsigned char* image_buffer, UBYTE Region) {
    UWORD x, y;
    UDOUBLE Addr = 0;

    for (y = 0; y < canvas.HeightByte; y++) {
        for (x = 0; x < canvas.WidthByte; x++) {//8 pixel =  1 byte
            Addr = x + y * canvas.WidthByte ;
            canvas.Image[Addr] = \
            (unsigned char)image_buffer[Addr+ (canvas.HeightByte)*canvas.WidthByte*(Region - 1)];
        }
    }
}

 void CanvasDrawBitmapToArea(unsigned char x,unsigned char y,const unsigned char *pBmp,
    unsigned char chWidth,unsigned char chHeight) {
    uint16_t i, j, byteWidth = (chWidth + 7) / 8;

    for (j = 0; j < chHeight; j ++) {
        for (i = 0; i < chWidth; i ++ ) {
            if (*(pBmp + j * byteWidth + i / 8) & (128 >> (i & 7))) {
                CanvasSetPixel(x + i, y + j, 0xffff);
            }
        }
    }
}
