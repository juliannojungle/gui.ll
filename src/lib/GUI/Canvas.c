#include "Canvas.h"
#include "HAL.c"
#include <stdint.h>
#include <stdlib.h>
#include <string.h> //memset()
#include <math.h>

Canvas canvas;

void CanvasNewImage(UBYTE *image, UWORD width, UWORD height, UWORD rotate, UWORD color) {
    canvas.Image = NULL;
    canvas.Image = image;

    canvas.WidthMemory = width;
    canvas.HeightMemory = height;
    canvas.Color = color;
    canvas.Scale = 2;

    canvas.WidthByte = (width % 8 == 0)? (width / 8 ): (width / 8 + 1);
    canvas.HeightByte = height;
//    printf("WidthByte = %d, HeightByte = %d\r\n", canvas.WidthByte, canvas.HeightByte);
//    printf(" LCD_WIDTH / 8 = %d\r\n",  122 / 8);

    canvas.Rotate = rotate;
    canvas.Flip = FLIP_NONE;

    if (rotate == ROTATE_0 || rotate == ROTATE_180) {
        canvas.Width = width;
        canvas.Height = height;
    } else {
        canvas.Width = height;
        canvas.Height = width;
    }
}

void CanvasSelectImage(UBYTE *image) {
    canvas.Image = image;
}

void CanvasSetRotate(UWORD rotate) {
    if (rotate == ROTATE_0 || rotate == ROTATE_90 || rotate == ROTATE_180 || rotate == ROTATE_270) {
        // Debug("Set image Rotate %d\r\n", rotate);
        canvas.Rotate = rotate;
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

void CanvasSetPixel(UWORD xPoint, UWORD yPoint, UWORD color) {
    if (xPoint > canvas.Width || yPoint > canvas.Height) {
        // Debug("Exceeding display boundaries\r\n");
        return;
    }

    UWORD X, Y;

    switch (canvas.Rotate) {
        case 0:
            X = xPoint;
            Y = yPoint;
            break;
        case 90:
            X = canvas.WidthMemory - yPoint - 1;
            Y = xPoint;
            break;
        case 180:
            X = canvas.WidthMemory - xPoint - 1;
            Y = canvas.HeightMemory - yPoint - 1;
            break;
        case 270:
            X = yPoint;
            Y = canvas.HeightMemory - xPoint - 1;
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
        if (color &0xff == BLACK)
            canvas.Image[Addr] = Rdata & ~(0x80 >> (X % 8));
        else
            canvas.Image[Addr] = Rdata | (0x80 >> (X % 8));
    } else if (canvas.Scale == 4) {
        UDOUBLE Addr = X / 4 + Y * canvas.WidthByte;
        color = color % 4; //Guaranteed color scale is 4  --- 0~3
        UBYTE Rdata = canvas.Image[Addr];

        Rdata = Rdata & (~(0xC0 >> ((X % 4) * 2)));
        canvas.Image[Addr] = Rdata | ((color << 6) >> ((X % 4) * 2));
    } else if (canvas.Scale == 16) {
        UDOUBLE Addr = X / 2 + Y * canvas.WidthByte;
        UBYTE Rdata = canvas.Image[Addr];
        color = color % 16;
        Rdata = Rdata & (~(0xf0 >> ((X % 2) * 4)));
        canvas.Image[Addr] = Rdata | ((color << 4) >> ((X % 2) * 4));
    } else if (canvas.Scale == 65) {
        UDOUBLE Addr = X * 2 + Y * canvas.WidthByte;
        canvas.Image[Addr] = 0xff & (color >> 8);
        canvas.Image[Addr + 1] = 0xff & color;
    }
}

void CanvasClear(UWORD color) {
    if (canvas.Scale == 2 || canvas.Scale == 4) {
        for (UWORD Y = 0; Y < canvas.HeightByte; Y++) {
            for (UWORD X = 0; X < canvas.WidthByte; X++) {//8 pixel =  1 byte
                UDOUBLE Addr = X + Y * canvas.WidthByte;
                canvas.Image[Addr] = color;
            }
        }
    } else if (canvas.Scale == 16) {
        for (UWORD Y = 0; Y < canvas.HeightByte; Y++) {
            for (UWORD X = 0; X < canvas.WidthByte; X++ ) {//8 pixel =  1 byte
                UDOUBLE Addr = X + Y * canvas.WidthByte;
                color = color & 0x0f;
                canvas.Image[Addr] = (color << 4) | color;
            }
        }
    } else if (canvas.Scale == 65) {
        for (UWORD Y = 0; Y < canvas.HeightByte; Y++) {
            for (UWORD X = 0; X < canvas.WidthByte; X++) {//8 pixel =  1 byte
                UDOUBLE Addr = X * 2 + Y * canvas.WidthByte;
                canvas.Image[Addr] = 0xff & (color >> 8);
                canvas.Image[Addr+1] = 0xff & color;
            }
        }
    }
}

void CanvasClearArea(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd, UWORD color) {
    UWORD X, Y;
    for (Y = yStart; Y < yEnd; Y++) {
        for (X = xStart; X < xEnd; X++) {//8 pixel =  1 byte
            CanvasSetPixel(X, Y, color);
        }
    }
}

void CanvasDrawPoint(UWORD xPoint, UWORD yPoint, UWORD color,
    PixelSize pixelSize, PixelFillStyle pixelFillStyle) {
    if (xPoint > canvas.Width || yPoint > canvas.Height) {
        // Debug("CanvasDrawPoint Input exceeds the normal display range\r\n");
        // printf("xPoint = %d , canvas.Width = %d  \r\n ",xPoint,canvas.Width);
        // printf("yPoint = %d , canvas.Height = %d  \r\n ",yPoint,canvas.Height);
        return;
    }

    int16_t XDir_Num , YDir_Num;
    if (pixelFillStyle == PIXEL_FILL_STYLE_AROUND) {
        for (XDir_Num = 0; XDir_Num < 2 * pixelSize - 1; XDir_Num++) {
            for (YDir_Num = 0; YDir_Num < 2 * pixelSize - 1; YDir_Num++) {
                if (xPoint + XDir_Num - pixelSize < 0 || yPoint + YDir_Num - pixelSize < 0)
                    break;
                // // printf("x = %d, y = %d\r\n", xPoint + XDir_Num - pixelSize, yPoint + YDir_Num - pixelSize);
                CanvasSetPixel(xPoint + XDir_Num - pixelSize, yPoint + YDir_Num - pixelSize, color);
            }
        }
    } else {
        for (XDir_Num = 0; XDir_Num <  pixelSize; XDir_Num++) {
            for (YDir_Num = 0; YDir_Num <  pixelSize; YDir_Num++) {
                CanvasSetPixel(xPoint + XDir_Num - 1, yPoint + YDir_Num - 1, color);
            }
        }
    }
}

void CanvasDrawLine(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd,
    UWORD color, PixelSize pixelSize, LineStyle lineStyle) {
    if (xStart > canvas.Width || yStart > canvas.Height ||
        xEnd > canvas.Width || yEnd > canvas.Height) {
        // Debug("CanvasDrawLine Input exceeds the normal display range\r\n");
        return;
    }

    UWORD xPoint = xStart;
    UWORD yPoint = yStart;
    int dx = (int)xEnd - (int)xStart >= 0 ? xEnd - xStart : xStart - xEnd;
    int dy = (int)yEnd - (int)yStart <= 0 ? yEnd - yStart : yStart - yEnd;

    // Increment direction, 1 is positive, -1 is counter;
    int XAddway = xStart < xEnd ? 1 : -1;
    int YAddway = yStart < yEnd ? 1 : -1;

    //Cumulative error
    int Esp = dx + dy;
    char Dotted_Len = 0;

    for (;;) {
        Dotted_Len++;
        //Painted dotted line, 2 point is really virtual
        if (lineStyle == LINE_STYLE_DOTTED && Dotted_Len % 3 == 0) {
            //Debug("LINE_DOTTED\r\n");
            if(color)
                CanvasDrawPoint(xPoint, yPoint, BLACK, pixelSize, DEFAULT_PIXEL_FILL_STYLE);
            else
                CanvasDrawPoint(xPoint, yPoint, WHITE, pixelSize, DEFAULT_PIXEL_FILL_STYLE);
            Dotted_Len = 0;
        } else {
            CanvasDrawPoint(xPoint, yPoint, color, pixelSize, DEFAULT_PIXEL_FILL_STYLE);
        }

        if (2 * Esp >= dy) {
            if (xPoint == xEnd)
                break;
            Esp += dy;
            xPoint += XAddway;
        }

        if (2 * Esp <= dx) {
            if (yPoint == yEnd)
                break;
            Esp += dx;
            yPoint += YAddway;
        }
    }
}

void CanvasDrawRectangle(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd,
    UWORD color, PixelSize lineWidth, DrawFillStyle rectangleFillStyle) {
    if (xStart > canvas.Width || yStart > canvas.Height ||
        xEnd > canvas.Width || yEnd > canvas.Height) {
        // Debug("Input exceeds the normal display range\r\n");
        return;
    }

    if (rectangleFillStyle) {
        UWORD yPoint;
        for(yPoint = yStart; yPoint < yEnd; yPoint++) {
            CanvasDrawLine(xStart, yPoint, xEnd, yPoint, color , lineWidth, LINE_STYLE_SOLID);
        }
    } else {
        CanvasDrawLine(xStart, yStart, xEnd, yStart, color, lineWidth, LINE_STYLE_SOLID);
        CanvasDrawLine(xStart, yStart, xStart, yEnd, color, lineWidth, LINE_STYLE_SOLID);
        CanvasDrawLine(xEnd, yEnd, xEnd, yStart, color, lineWidth, LINE_STYLE_SOLID);
        CanvasDrawLine(xEnd, yEnd, xStart, yEnd, color, lineWidth, LINE_STYLE_SOLID);
    }
}

void CanvasDrawCircle(UWORD xCenter, UWORD yCenter, UWORD radius,
    UWORD color, PixelSize lineWidth, DrawFillStyle circleFillStyle) {
    if (xCenter > canvas.Width || yCenter >= canvas.Height) {
        // Debug("CanvasDrawCircle Input exceeds the normal display range\r\n");
        return;
    }

    //Draw a circle from(0, R) as a starting point
    int16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = radius;

    //Cumulative error,judge the next point of the logo
    int16_t Esp = 3 - (radius << 1 );

    int16_t sCountY;
    if (circleFillStyle == DRAW_FILL_STYLE_FULL) {
        while (XCurrent <= YCurrent ) { //Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ ) {
                CanvasDrawPoint(xCenter + XCurrent, yCenter + sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//1
                CanvasDrawPoint(xCenter - XCurrent, yCenter + sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//2
                CanvasDrawPoint(xCenter - sCountY, yCenter + XCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//3
                CanvasDrawPoint(xCenter - sCountY, yCenter - XCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//4
                CanvasDrawPoint(xCenter - XCurrent, yCenter - sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//5
                CanvasDrawPoint(xCenter + XCurrent, yCenter - sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//6
                CanvasDrawPoint(xCenter + sCountY, yCenter - XCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//7
                CanvasDrawPoint(xCenter + sCountY, yCenter + XCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
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
            CanvasDrawPoint(xCenter + XCurrent, yCenter + YCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//1
            CanvasDrawPoint(xCenter - XCurrent, yCenter + YCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//2
            CanvasDrawPoint(xCenter - YCurrent, yCenter + XCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//3
            CanvasDrawPoint(xCenter - YCurrent, yCenter - XCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//4
            CanvasDrawPoint(xCenter - XCurrent, yCenter - YCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//5
            CanvasDrawPoint(xCenter + XCurrent, yCenter - YCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//6
            CanvasDrawPoint(xCenter + YCurrent, yCenter - XCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//7
            CanvasDrawPoint(xCenter + YCurrent, yCenter + XCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//0

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

void CanvasDrawChar(UWORD xPoint, UWORD yPoint, const char ASCIIChar,
    sFONT* font, UWORD foregroundColor, UWORD backgroundColor) {
    UWORD Page, Column;

    if (xPoint > canvas.Width || yPoint > canvas.Height) {
        // Debug("CanvasDrawChar Input exceeds the normal display range\r\n");
        return;
    }

    uint32_t Char_Offset = (ASCIIChar - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &font->table[Char_Offset];

    for (Page = 0; Page < font->Height; Page ++ ) {
        for (Column = 0; Column < font->Width; Column ++ ) {
            //To determine whether the font background color and screen background color is consistent
            if (FONT_BACKGROUND == backgroundColor) { //this process is to speed up the scan
                if (*ptr & (0x80 >> (Column % 8)))
                    CanvasSetPixel(xPoint + Column, yPoint + Page, foregroundColor);
            } else {
                if (*ptr & (0x80 >> (Column % 8))) {
                    CanvasSetPixel(xPoint + Column, yPoint + Page, foregroundColor);
                } else {
                    CanvasSetPixel(xPoint + Column, yPoint + Page, backgroundColor);
                }
            }

            //One pixel is 8 bits
            if (Column % 8 == 7)
                ptr++;
        }// Write a line

        if (font->Width % 8 != 0)
            ptr++;
    }// Write all
}

void CanvasDrawText(UWORD xStart, UWORD yStart, const char * text,
    sFONT* font, UWORD foregroundColor, UWORD backgroundColor) {
    UWORD xPoint = xStart;
    UWORD yPoint = yStart;

    if (xStart > canvas.Width || yStart > canvas.Height) {
        // Debug("CanvasDrawText Input exceeds the normal display range\r\n");
        return;
    }

    while (* text != '\0') {
        //if X direction filled , reposition to(xStart,yPoint),yPoint is Y direction plus the Height of the character
        if ((xPoint + font->Width ) > canvas.Width ) {
            xPoint = xStart;
            yPoint += font->Height;
        }

        // If the Y direction is full, reposition to(xStart, yStart)
        if ((yPoint  + font->Height ) > canvas.Height ) {
            xPoint = xStart;
            yPoint = yStart;
        }

        CanvasDrawChar(xPoint, yPoint, * text, font, foregroundColor, backgroundColor);

        //The next character of the address
        text++;

        //The next word of the abscissa increases the font of the broadband
        xPoint += font->Width;
    }
}

void CanvasDrawTextCN(UWORD xStart, UWORD yStart, const char * text, cFONT* font,
    UWORD foregroundColor, UWORD backgroundColor) {
    const char* pText = text;
    int x = xStart, y = yStart;
    int i, j, Num;

    /* Send the string character by character on EPD */
    while (*pText != 0) {
        if (*pText <= 0x7F) {  //ASCII < 126
            for (Num = 0; Num < font->size; Num++) {
                if (*pText== font->table[Num].index[0]) {
                    const char* ptr = &font->table[Num].matrix[0];

                    for (j = 0; j < font->Height; j++) {
                        for (i = 0; i < font->Width; i++) {
                            if (FONT_BACKGROUND == backgroundColor) { //this process is to speed up the scan
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(x + i, y + j, foregroundColor);
                                }
                            } else {
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(x + i, y + j, foregroundColor);
                                } else {
                                    CanvasSetPixel(x + i, y + j, backgroundColor);
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
            pText += 1;
            /* Decrement the column position by 16 */
            x += font->ASCII_Width;
        } else {        //Chinese
            for (Num = 0; Num < font->size; Num++) {
                if ((*pText== font->table[Num].index[0]) && (*(pText + 1) == font->table[Num].index[1])) {
                    const char* ptr = &font->table[Num].matrix[0];

                    for (j = 0; j < font->Height; j++) {
                        for (i = 0; i < font->Width; i++) {
                            if (FONT_BACKGROUND == backgroundColor) { //this process is to speed up the scan
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(x + i, y + j, foregroundColor);
                                }
                            } else {
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(x + i, y + j, foregroundColor);
                                } else {
                                    CanvasSetPixel(x + i, y + j, backgroundColor);
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
            pText += 2;
            /* Decrement the column position by 16 */
            x += font->Width;
        }
    }
}

#define  ARRAY_LEN 255
void CanvasDrawNum(UWORD xPoint, UWORD yPoint, double number,
    sFONT* font, UWORD digit, UWORD foregroundColor, UWORD backgroundColor) {
    int16_t numberBit = 0, textBit = 0;
    uint8_t textArray[ARRAY_LEN] = {0}, numberArray[ARRAY_LEN] = {0};
    uint8_t *pStr = textArray;
    int temp = number;
    float decimals;
    uint8_t i;

    if (xPoint > canvas.Width || yPoint > canvas.Height) {
        // Debug("Paint_DisNum Input exceeds the normal display range\r\n");
        return;
    }

    if (digit > 0) {
        decimals = number - temp;

        for(i = digit; i > 0; i--) {
            decimals*=10;
        }

        temp = decimals;

        //Converts a number to a string
        for(i=digit; i>0; i--) {
            numberArray[numberBit] = temp % 10 + '0';
            numberBit++;
            temp /= 10;
        }

        numberArray[numberBit] = '.';
        numberBit++;
    }

    temp = number;

    //Converts a number to a string
    while (temp) {
        numberArray[numberBit] = temp % 10 + '0';
        numberBit++;
        temp /= 10;
    }

    //The string is inverted
    while (numberBit > 0) {
        textArray[textBit] = numberArray[numberBit - 1];
        textBit ++;
        numberBit --;
    }

    CanvasDrawText(xPoint, yPoint, (const char*)pStr, font, foregroundColor, backgroundColor);
}

void CanvasDrawTime(UWORD xStart, UWORD yStart, DateTime *pTime, sFONT* font,
    UWORD foregroundColor, UWORD backgroundColor) {
    uint8_t value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    UWORD Dx = font->Width;
    CanvasDrawChar(xStart, yStart, value[pTime->Hour / 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + Dx, yStart, value[pTime->Hour % 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + Dx  + Dx / 4 + Dx / 2, yStart, ':', font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + Dx * 2 + Dx / 2, yStart, value[pTime->Min / 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + Dx * 3 + Dx / 2, yStart, value[pTime->Min % 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + Dx * 4 + Dx / 2 - Dx / 4, yStart, ':', font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + Dx * 5, yStart, value[pTime->Sec / 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + Dx * 6, yStart, value[pTime->Sec % 10], font, foregroundColor, backgroundColor);
}

void CanvasDrawImage(const unsigned char *image, UWORD xStart, UWORD yStart, UWORD imageWidth, UWORD imageHeight) {
    int i, j;
    for (j = 0; j < imageWidth; j++) {
        for (i = 0; i < imageHeight; i++) {
            if (xStart + i < canvas.WidthMemory && yStart + j < canvas.HeightMemory) //Exceeded part does not display
                CanvasSetPixel(xStart + i, yStart + j, (*(image + j * imageHeight * 2 + i * 2 + 1)) << 8 | (*(image + j * imageHeight * 2 + i * 2)));
            //Using arrays is a property of sequential storage, accessing the original array by algorithm
            //j*imageHeight*2                Y offset
            //i*2                     X offset
        }
    }
}

void CanvasDrawImage1(const unsigned char *image, UWORD xStart, UWORD yStart, UWORD imageWidth, UWORD imageHeight) {
    int i, j;
    for (j = 0; j < imageHeight; j++) {
        for (i = 0; i < imageWidth; i++) {
            if (xStart + i < canvas.HeightMemory  &&  yStart + j < canvas.WidthMemory) //Exceeded part does not display
                CanvasSetPixel(xStart + i, yStart + j, (*(image + j * imageWidth * 2 + i * 2 + 1)) << 8 | (*(image + j * imageWidth * 2 + i * 2)));
            //Using arrays is a property of sequential storage, accessing the original array by algorithm
            //j*imageWidth*2                Y offset
            //i*2                     X offset
        }
    }
}

void CanvasDrawBitmap(const unsigned char* imageBuffer) {
    UWORD x, y;
    UDOUBLE Addr = 0;

    for (y = 0; y < canvas.HeightByte; y++) {
        for (x = 0; x < canvas.WidthByte; x++) {//8 pixel =  1 byte
            Addr = x + y * canvas.WidthByte;
            canvas.Image[Addr] = (unsigned char)imageBuffer[Addr];
        }
    }
}

void CanvasDrawBitmapBlock(const unsigned char* imageBuffer, UBYTE region) {
    UWORD x, y;
    UDOUBLE Addr = 0;

    for (y = 0; y < canvas.HeightByte; y++) {
        for (x = 0; x < canvas.WidthByte; x++) {//8 pixel =  1 byte
            Addr = x + y * canvas.WidthByte ;
            canvas.Image[Addr] = \
            (unsigned char)imageBuffer[Addr+ (canvas.HeightByte)*canvas.WidthByte*(region - 1)];
        }
    }
}

 void CanvasDrawBitmapToArea(unsigned char x,unsigned char y,const unsigned char *pBitmap,
    unsigned char areaWidth,unsigned char areaHeight) {
    uint16_t i, j, byteWidth = (areaWidth + 7) / 8;

    for (j = 0; j < areaHeight; j ++) {
        for (i = 0; i < areaWidth; i ++ ) {
            if (*(pBitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
                CanvasSetPixel(x + i, y + j, 0xffff);
            }
        }
    }
}
