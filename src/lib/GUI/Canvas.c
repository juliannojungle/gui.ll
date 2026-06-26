#include "Canvas.h"
#include <png.h>
#include "Debug.h"

Canvas canvas;

void CanvasNewImage(UBYTE *image, UWORD width, UWORD height, UWORD rotate) {
    canvas.Image = NULL;
    canvas.Image = image;

    canvas.WidthMemory = width;
    canvas.HeightMemory = height;
    canvas.Scale = 2;

    canvas.WidthByte = (width % 8 == 0)? (width / 8 ): (width / 8 + 1);
    canvas.HeightByte = height;

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
        SHOWDEBUG("Set image Rotate %d\r\n", rotate);
        canvas.Rotate = rotate;
    }
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
    }
}

void CanvasFlipTexture(UBYTE flipDirection) {
    if (flipDirection == FLIP_NONE || flipDirection == FLIP_HORIZONTAL ||
        flipDirection == FLIP_VERTICAL || flipDirection == FLIP_ORIGIN) {
        canvas.Flip = flipDirection;
    }
}

void CanvasSetPixel(UWORD xPoint, UWORD yPoint, UWORD color) {
    if (xPoint > canvas.Width || yPoint > canvas.Height) {
        SHOWDEBUG("Exceeding texture boundaries\r\n");
        return;
    }

    UWORD x, y;

    switch (canvas.Rotate) {
        case 0:
            x = xPoint;
            y = yPoint;
            break;
        case 90:
            x = canvas.WidthMemory - yPoint - 1;
            y = xPoint;
            break;
        case 180:
            x = canvas.WidthMemory - xPoint - 1;
            y = canvas.HeightMemory - yPoint - 1;
            break;
        case 270:
            x = yPoint;
            y = canvas.HeightMemory - xPoint - 1;
            break;
        default: return;
    }

    switch (canvas.Flip) {
        case FLIP_NONE:
            break;
        case FLIP_HORIZONTAL:
            x = canvas.WidthMemory - x - 1;
            break;
        case FLIP_VERTICAL:
            y = canvas.HeightMemory - y - 1;
            break;
        case FLIP_ORIGIN:
            x = canvas.WidthMemory - x - 1;
            y = canvas.HeightMemory - y - 1;
            break;
        default: return;
    }

    if (x > canvas.WidthMemory || y > canvas.HeightMemory) {
        SHOWDEBUG("Exceeding texture memory boundaries\r\n");
        return;
    }

    if (canvas.Scale == 2) {
        UDOUBLE addr = x / 8 + y * canvas.WidthByte;
        UBYTE rData = canvas.Image[addr];
        if ((color & 0xff) == BLACK)
            canvas.Image[addr] = rData & ~(0x80 >> (x % 8));
        else
            canvas.Image[addr] = rData | (0x80 >> (x % 8));
    } else if (canvas.Scale == 4) {
        UDOUBLE addr = x / 4 + y * canvas.WidthByte;
        color = color % 4; //Guaranteed color scale is 4  --- 0~3
        UBYTE rData = canvas.Image[addr];
        rData = rData & (~(0xC0 >> ((x % 4) * 2)));
        canvas.Image[addr] = rData | ((color << 6) >> ((x % 4) * 2));
    } else if (canvas.Scale == 16) {
        UDOUBLE addr = x / 2 + y * canvas.WidthByte;
        UBYTE rData = canvas.Image[addr];
        color = color % 16;
        rData = rData & (~(0xf0 >> ((x % 2) * 4)));
        canvas.Image[addr] = rData | ((color << 4) >> ((x % 2) * 4));
    } else if (canvas.Scale == 65) {
        UDOUBLE addr = x * 2 + y * canvas.WidthByte;
        canvas.Image[addr] = 0xff & (color >> 8);
        canvas.Image[addr + 1] = 0xff & color;
    }
}

void CanvasClear(UWORD color) {
    if (canvas.Scale == 2 || canvas.Scale == 4) {
        for (UWORD y = 0; y < canvas.HeightByte; y++) {
            for (UWORD x = 0; x < canvas.WidthByte; x++) {//8 pixel =  1 byte
                UDOUBLE Addr = x + y * canvas.WidthByte;
                canvas.Image[Addr] = color;
            }
        }
    } else if (canvas.Scale == 16) {
        for (UWORD y = 0; y < canvas.HeightByte; y++) {
            for (UWORD x = 0; x < canvas.WidthByte; x++ ) {//8 pixel =  1 byte
                UDOUBLE Addr = x + y * canvas.WidthByte;
                color = color & 0x0f;
                canvas.Image[Addr] = (color << 4) | color;
            }
        }
    } else if (canvas.Scale == 65) {
        for (UWORD y = 0; y < canvas.HeightByte; y++) {
            for (UWORD x = 0; x < canvas.WidthByte; x++) {//8 pixel =  1 byte
                UDOUBLE Addr = x * 2 + y * canvas.WidthByte;
                canvas.Image[Addr] = 0xff & (color >> 8);
                canvas.Image[Addr+1] = 0xff & color;
            }
        }
    }
}

void CanvasClearArea(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd, UWORD color) {
    UWORD x, y;
    for (y = yStart; y < yEnd; y++) {
        for (x = xStart; x < xEnd; x++) {//8 pixel =  1 byte
            CanvasSetPixel(x, y, color);
        }
    }
}

void CanvasDrawPoint(UWORD xPoint, UWORD yPoint, UWORD color,
    PixelSize pixelSize, PixelFillStyle pixelFillStyle) {
    if (xPoint > canvas.Width || yPoint > canvas.Height) {
        SHOWDEBUG("Exceeding boundaries: x:%d, y:%d, width:%d, height:%d\r\n ", xPoint, yPoint, canvas.Width, canvas.Height);
        return;
    }

    int16_t XDir_Num , YDir_Num;
    if (pixelFillStyle == PIXEL_FILL_STYLE_AROUND) {
        for (XDir_Num = 0; XDir_Num < 2 * pixelSize - 1; XDir_Num++) {
            for (YDir_Num = 0; YDir_Num < 2 * pixelSize - 1; YDir_Num++) {
                if (xPoint + XDir_Num - pixelSize < 0 || yPoint + YDir_Num - pixelSize < 0)
                    break;
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
        SHOWDEBUG("Line exceeds boundaries\r\n");
        return;
    }

    UWORD xPoint = xStart;
    UWORD yPoint = yStart;
    int dx = (int)xEnd - (int)xStart >= 0 ? xEnd - xStart : xStart - xEnd;
    int dy = (int)yEnd - (int)yStart <= 0 ? yEnd - yStart : yStart - yEnd;

    // Increment direction, 1 is positive, -1 is counter;
    int xIncDirection = xStart < xEnd ? 1 : -1;
    int yIncDirection = yStart < yEnd ? 1 : -1;

    //Cumulative error
    int esp = dx + dy;
    char dottedLen = 0;

    for (;;) {
        dottedLen++;
        if (lineStyle == LINE_STYLE_DOTTED && dottedLen % 3 == 0) {
            if(color)
                CanvasDrawPoint(xPoint, yPoint, BLACK, pixelSize, DEFAULT_PIXEL_FILL_STYLE);
            else
                CanvasDrawPoint(xPoint, yPoint, WHITE, pixelSize, DEFAULT_PIXEL_FILL_STYLE);
            dottedLen = 0;
        } else {
            CanvasDrawPoint(xPoint, yPoint, color, pixelSize, DEFAULT_PIXEL_FILL_STYLE);
        }

        if (2 * esp >= dy) {
            if (xPoint == xEnd)
                break;
            esp += dy;
            xPoint += xIncDirection;
        }

        if (2 * esp <= dx) {
            if (yPoint == yEnd)
                break;
            esp += dx;
            yPoint += yIncDirection;
        }
    }
}

void CanvasDrawRectangle(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd,
    UWORD color, PixelSize lineWidth, DrawFillStyle rectangleFillStyle) {
    if (xStart > canvas.Width || yStart > canvas.Height ||
        xEnd > canvas.Width || yEnd > canvas.Height) {
        SHOWDEBUG("Rectangle exceeds texture boundaries\r\n");
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
        SHOWDEBUG("Circle exceeds texture boundaries\r\n");
        return;
    }

    //Draw a circle from(0, R) as a starting point
    int16_t xCurrent, yCurrent;
    xCurrent = 0;
    yCurrent = radius;

    //Cumulative error
    int16_t esp = 3 - (radius << 1 );

    int16_t sCountY;
    if (circleFillStyle == DRAW_FILL_STYLE_FULL) {
        while (xCurrent <= yCurrent ) { //Realistic circles
            for (sCountY = xCurrent; sCountY <= yCurrent; sCountY ++ ) {
                CanvasDrawPoint(xCenter + xCurrent, yCenter + sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//1
                CanvasDrawPoint(xCenter - xCurrent, yCenter + sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//2
                CanvasDrawPoint(xCenter - sCountY, yCenter + xCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//3
                CanvasDrawPoint(xCenter - sCountY, yCenter - xCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//4
                CanvasDrawPoint(xCenter - xCurrent, yCenter - sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//5
                CanvasDrawPoint(xCenter + xCurrent, yCenter - sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//6
                CanvasDrawPoint(xCenter + sCountY, yCenter - xCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//7
                CanvasDrawPoint(xCenter + sCountY, yCenter + xCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
            }

            if (esp < 0 )
                esp += 4 * xCurrent + 6;
            else {
                esp += 10 + 4 * (xCurrent - yCurrent );
                yCurrent --;
            }

            xCurrent ++;
        }
    } else {
        while (xCurrent <= yCurrent ) {
            CanvasDrawPoint(xCenter + xCurrent, yCenter + yCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//1
            CanvasDrawPoint(xCenter - xCurrent, yCenter + yCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//2
            CanvasDrawPoint(xCenter - yCurrent, yCenter + xCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//3
            CanvasDrawPoint(xCenter - yCurrent, yCenter - xCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//4
            CanvasDrawPoint(xCenter - xCurrent, yCenter - yCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//5
            CanvasDrawPoint(xCenter + xCurrent, yCenter - yCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//6
            CanvasDrawPoint(xCenter + yCurrent, yCenter - xCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//7
            CanvasDrawPoint(xCenter + yCurrent, yCenter + xCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//0

            if (esp < 0 )
                esp += 4 * xCurrent + 6;
            else {
                esp += 10 + 4 * (xCurrent - yCurrent );
                yCurrent --;
            }
            xCurrent ++;
        }
    }
}

void CanvasDrawChar(UWORD xPoint, UWORD yPoint, const char ASCIIChar,
    sFONT* font, UWORD foregroundColor, UWORD backgroundColor) {
    UWORD Page, Column;

    if (xPoint > canvas.Width || yPoint > canvas.Height) {
        SHOWDEBUG("Char exceeds texture boundaries\r\n");
        return;
    }

    uint32_t charOffset = (ASCIIChar - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &font->table[charOffset];

    for (Page = 0; Page < font->Height; Page ++ ) {
        for (Column = 0; Column < font->Width; Column ++ ) {
            if (FONT_BACKGROUND == backgroundColor) {
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
        SHOWDEBUG("Text exceeds texture boundaries\r\n");
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
        text++; //The next character of the address
        xPoint += font->Width; //The next word of the abscissa increases the font of the broadband
    }
}

void CanvasDrawTextCN(UWORD xStart, UWORD yStart, const char * text, cFONT* font,
    UWORD foregroundColor, UWORD backgroundColor) {
    const char* pText = text;
    int x = xStart, y = yStart;
    int i, j, num;

    /* Send the string character by character on EPD */
    while (*pText != 0) {
        if (*pText <= 0x7F) {  //ASCII < 126
            for (num = 0; num < font->size; num++) {
                if (*pText== font->table[num].index[0]) {
                    const char* ptr = &font->table[num].matrix[0];

                    for (j = 0; j < font->Height; j++) {
                        for (i = 0; i < font->Width; i++) {
                            if (FONT_BACKGROUND == backgroundColor) {
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
            for (num = 0; num < font->size; num++) {
                if ((*pText== font->table[num].index[0]) && (*(pText + 1) == font->table[num].index[1])) {
                    const char* ptr = &font->table[num].matrix[0];

                    for (j = 0; j < font->Height; j++) {
                        for (i = 0; i < font->Width; i++) {
                            if (FONT_BACKGROUND == backgroundColor) {
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

            pText += 2; /* Point on the next character */
            x += font->Width; /* Decrement the column position by 16 */
        }
    }
}

#define ARRAY_LEN 255
void CanvasDrawNum(UWORD xPoint, UWORD yPoint, double number,
    sFONT* font, UWORD digit, UWORD foregroundColor, UWORD backgroundColor) {
    int16_t numberBit = 0, textBit = 0;
    uint8_t textArray[ARRAY_LEN] = {0}, numberArray[ARRAY_LEN] = {0};
    uint8_t *pStr = textArray;
    int temp = number;
    float decimals;
    uint8_t i;

    if (xPoint > canvas.Width || yPoint > canvas.Height) {
        SHOWDEBUG("Number exceeds texture boundaries\r\n");
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
    UWORD dX = font->Width;
    CanvasDrawChar(xStart, yStart, value[pTime->Hour / 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + dX, yStart, value[pTime->Hour % 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + dX  + dX / 4 + dX / 2, yStart, ':', font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + dX * 2 + dX / 2, yStart, value[pTime->Min / 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + dX * 3 + dX / 2, yStart, value[pTime->Min % 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + dX * 4 + dX / 2 - dX / 4, yStart, ':', font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + dX * 5, yStart, value[pTime->Sec / 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(xStart + dX * 6, yStart, value[pTime->Sec % 10], font, foregroundColor, backgroundColor);
}

void CanvasDrawImage(const unsigned char *image, UWORD xStart, UWORD yStart, UWORD imageWidth, UWORD imageHeight) {
    int i, j;
    for (j = 0; j < imageHeight; j++) {
        for (i = 0; i < imageWidth; i++) {
            if (xStart + i < canvas.HeightMemory && yStart + j < canvas.WidthMemory) //Exceeded part does not display
                CanvasSetPixel(xStart + i, yStart + j, (*(image + j * imageWidth * 2 + i * 2 + 1)) << 8 | (*(image + j * imageWidth * 2 + i * 2)));
                // j*imageWidth*2 = Y offset
                // i*2 = X offset
        }
    }
}

void CanvasDrawBitmap(const unsigned char* imageBuffer) {
    UWORD x, y;
    UDOUBLE addr = 0;

    for (y = 0; y < canvas.HeightByte; y++) {
        for (x = 0; x < canvas.WidthByte; x++) {//8 pixel =  1 byte
            addr = x + y * canvas.WidthByte;
            canvas.Image[addr] = (unsigned char)imageBuffer[addr];
        }
    }
}

void CanvasDrawBitmapBlock(const unsigned char* imageBuffer, UBYTE region) {
    UWORD x, y;
    UDOUBLE addr = 0;

    for (y = 0; y < canvas.HeightByte; y++) {
        for (x = 0; x < canvas.WidthByte; x++) {//8 pixel =  1 byte
            addr = x + y * canvas.WidthByte ;
            canvas.Image[addr] = \
            (unsigned char)imageBuffer[addr+ (canvas.HeightByte)*canvas.WidthByte*(region - 1)];
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

static void PngCustomReadData(png_structrp pngPointer, png_bytep data, size_t length) {
    SHOWDEBUG(".");
    UINT bytesRead;
    f_read((FIL*)png_get_io_ptr(pngPointer), data, length, &bytesRead);
}

static void PngShowError(png_structp pngPointer, const char *message) {
    SHOWDEBUG("Error from libpng: %s\n", message);
}

void CanvasDrawPng(FIL *file) {
    SHOWDEBUG("Creating read structure\n");
    png_structp pngPointer = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, PngShowError, NULL);

    if (pngPointer == NULL) {
        SHOWDEBUG("png_create_read_struct error\n");
        return;
    }

    SHOWDEBUG("Allocating memory for image information\n");
    png_infop infoPointer = png_create_info_struct(pngPointer);

    if (infoPointer == NULL) {
        SHOWDEBUG("png_create_info_struct error\n");
        png_destroy_read_struct(&pngPointer, NULL, NULL);
        return;
    }

    SHOWDEBUG("Setting up the custom read function\n");
    png_set_read_fn(pngPointer, file, PngCustomReadData);

    /* Declared volatile before setjmp so their values survive a longjmp (C99 7.13.2.1) and can be cleaned up. */
    volatile png_bytep rowPointers = NULL;
    SHOWDEBUG("Setting up LongJump\n");

    if (setjmp(png_jmpbuf(pngPointer)) != 0) {
        /* Any libpng error from here on jumps back to this point. */
        SHOWDEBUG("LongJump from a libpng error. Cleaning up.\n");

        if (rowPointers != NULL)
            png_free(pngPointer, rowPointers);

        png_destroy_read_struct(&pngPointer, &infoPointer, NULL);
        return;
    }

    SHOWDEBUG("Reading info\n");
    /* png_read_info gets information about PNG file before the first IDAT (image data chunk). REQUIRED. */
    png_read_info(pngPointer, infoPointer);

    SHOWDEBUG("\nParsing image info\n");
    png_uint_32 width, height;
    int bitDepth, colorType, interlaceType;
    png_get_IHDR(pngPointer, infoPointer, &width, &height, &bitDepth, &colorType, &interlaceType, NULL, NULL);
    SHOWDEBUG("PNG info: width: %d, height: %d, bit_depth: %d\n", width, height, bitDepth);

    int col, row;
    int maxCol = width > canvas.Width ? canvas.Width : width; /* won't write outside canvas. */
    int maxRow = height > canvas.Height ? canvas.Height : height;

    int num_palette = 0;
    png_colorp palette = NULL;

    if (colorType == PNG_COLOR_TYPE_PALETTE)
        png_get_PLTE(pngPointer, infoPointer, &palette, &num_palette);

    for (row = 0; row < maxRow; row++) {
        png_bytep rowBuffer = (png_bytep)png_malloc(pngPointer, png_get_rowbytes(pngPointer, infoPointer));
        rowPointers = rowBuffer; // track for cleanup before any call that may longjmp
        png_read_rows(pngPointer, &rowBuffer, NULL, 1);

        for (col = 0; col < maxCol; col++) {
            png_byte red, green, blue;

            if ((colorType == PNG_COLOR_TYPE_PALETTE) && (palette != NULL)) {
                red = palette[rowPointers[col]].red;
                green = palette[rowPointers[col]].green;
                blue = palette[rowPointers[col]].blue;
            } else {
                /* if the image is paletted but we don't have a palette, display as grayscale using palette index. */
                png_bytep pixel = &rowPointers[col];
                red = *(pixel++);
                green = *(pixel++);
                blue = *(pixel++);
            }

            /* The LCD uses RGB565 16-bits format: RRRRRGGG GGGBBBBB */
            UWORD color = (UWORD)(((red & 0b11111000) | ((green & 0b11100000) >> 5)) << 8)
                | (UWORD)(((green & 0b00011100) << 3) | ((blue & 0b11111000) >> 3));
            CanvasSetPixel(col, row, color);
        }

        png_free(pngPointer, rowBuffer);
        rowPointers = NULL;
    }

    png_destroy_read_struct(&pngPointer, &infoPointer, NULL);
}
