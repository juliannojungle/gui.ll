#include "Canvas.h"
#include <png.h>
#include <stdlib.h>
#include "Debug.h"
#include "Trigonometry.h"

Canvas canvas = { ROTATE_0, FLIP_NONE };

Texture CanvasNewTexture(UINT16 width, UINT16 height) {
    Texture texture;
    UINT32 imageSize = height * width * 2;
    texture.Data = (UINT8 *) malloc(imageSize);

    texture.WidthMemory = width;
    texture.HeightMemory = height;
    texture.WidthByte = width * 2;
    texture.HeightByte = height;

    if (canvas.Rotate == ROTATE_0 || canvas.Rotate == ROTATE_180) {
        texture.Width = width;
        texture.Height = height;
    } else {
        texture.Width = height;
        texture.Height = width;
    }

    return texture;
}

void CanvasSetRotate(UINT16 rotate) {
    if (rotate == ROTATE_0 || rotate == ROTATE_90 || rotate == ROTATE_180 || rotate == ROTATE_270) {
        SHOWDEBUG("Set canvas rotate %d\r\n", rotate);
        canvas.Rotate = rotate;
    }
}

void CanvasSetFlip(UINT8 flipDirection) {
    if (flipDirection == FLIP_NONE || flipDirection == FLIP_HORIZONTAL ||
        flipDirection == FLIP_VERTICAL || flipDirection == FLIP_ORIGIN) {
        SHOWDEBUG("Set canvas flip %d\r\n", flipDirection);
        canvas.Flip = flipDirection;
    }
}

void CanvasSetPixel(Texture texture, UINT16 xPoint, UINT16 yPoint, UINT16 color) {
    if (xPoint > texture.Width || yPoint > texture.Height) {
        SHOWDEBUG("Exceeding texture boundaries\r\n");
        return;
    }

    UINT16 x, y;

    switch (canvas.Rotate) {
        case ROTATE_0:
            x = xPoint;
            y = yPoint;
            break;
        case ROTATE_90:
            x = texture.WidthMemory - yPoint - 1;
            y = xPoint;
            break;
        case ROTATE_180:
            x = texture.WidthMemory - xPoint - 1;
            y = texture.HeightMemory - yPoint - 1;
            break;
        case ROTATE_270:
            x = yPoint;
            y = texture.HeightMemory - xPoint - 1;
            break;
        default: return;
    }

    switch (canvas.Flip) {
        case FLIP_NONE:
            break;
        case FLIP_HORIZONTAL:
            x = texture.WidthMemory - x - 1;
            break;
        case FLIP_VERTICAL:
            y = texture.HeightMemory - y - 1;
            break;
        case FLIP_ORIGIN:
            x = texture.WidthMemory - x - 1;
            y = texture.HeightMemory - y - 1;
            break;
        default: return;
    }

    if (x > texture.WidthMemory || y > texture.HeightMemory) {
        SHOWDEBUG("Exceeding texture memory boundaries\r\n");
        return;
    }

    UINT32 addr = x * 2 + y * texture.WidthByte;
    texture.Data[addr] = 0xff & (color >> 8);
    texture.Data[addr + 1] = 0xff & color;
}

void CanvasClear(Texture texture, UINT16 color) {
    for (UINT16 y = 0; y < texture.HeightByte; y++) {
        for (UINT16 x = 0; x < texture.WidthByte; x++) {
            UINT32 Addr = x * 2 + y * texture.WidthByte;
            texture.Data[Addr] = 0xff & (color >> 8);
            texture.Data[Addr + 1] = 0xff & color;
        }
    }
}

void CanvasClearArea(Texture texture, UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd, UINT16 color) {
    UINT16 x, y;
    for (y = yStart; y < yEnd; y++) {
        for (x = xStart; x < xEnd; x++) {//8 pixel =  1 byte
            CanvasSetPixel(texture, x, y, color);
        }
    }
}

void CanvasDrawPoint(Texture texture, UINT16 xPoint, UINT16 yPoint, UINT16 color,
    PixelSize pixelSize, PixelFillStyle pixelFillStyle) {
    if (xPoint > texture.Width || yPoint > texture.Height) {
        SHOWDEBUG("Exceeding boundaries: x:%d, y:%d, width:%d, height:%d\r\n ", xPoint, yPoint, texture.Width, texture.Height);
        return;
    }

    int16_t XDir_Num , YDir_Num;
    if (pixelFillStyle == PIXEL_FILL_STYLE_AROUND) {
        for (XDir_Num = 0; XDir_Num < 2 * pixelSize - 1; XDir_Num++) {
            for (YDir_Num = 0; YDir_Num < 2 * pixelSize - 1; YDir_Num++) {
                if (xPoint + XDir_Num - pixelSize < 0 || yPoint + YDir_Num - pixelSize < 0)
                    break;
                CanvasSetPixel(texture, xPoint + XDir_Num - pixelSize, yPoint + YDir_Num - pixelSize, color);
            }
        }
    } else {
        for (XDir_Num = 0; XDir_Num <  pixelSize; XDir_Num++) {
            for (YDir_Num = 0; YDir_Num <  pixelSize; YDir_Num++) {
                CanvasSetPixel(texture, xPoint + XDir_Num - 1, yPoint + YDir_Num - 1, color);
            }
        }
    }
}

void CanvasDrawLine(Texture texture, UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd,
    UINT16 color, PixelSize pixelSize, LineStyle lineStyle) {
    if (xStart > texture.Width || yStart > texture.Height ||
        xEnd > texture.Width || yEnd > texture.Height) {
        SHOWDEBUG("Line exceeds boundaries\r\n");
        return;
    }

    UINT16 xPoint = xStart;
    UINT16 yPoint = yStart;
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
                CanvasDrawPoint(texture, xPoint, yPoint, BLACK, pixelSize, DEFAULT_PIXEL_FILL_STYLE);
            else
                CanvasDrawPoint(texture, xPoint, yPoint, WHITE, pixelSize, DEFAULT_PIXEL_FILL_STYLE);
            dottedLen = 0;
        } else {
            CanvasDrawPoint(texture, xPoint, yPoint, color, pixelSize, DEFAULT_PIXEL_FILL_STYLE);
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

void CanvasDrawRectangle(Texture texture, UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd,
    UINT16 color, PixelSize lineWidth, DrawFillStyle rectangleFillStyle) {
    if (xStart > texture.Width || yStart > texture.Height ||
        xEnd > texture.Width || yEnd > texture.Height) {
        SHOWDEBUG("Rectangle exceeds texture boundaries\r\n");
        return;
    }

    if (rectangleFillStyle) {
        UINT16 yPoint;
        for(yPoint = yStart; yPoint < yEnd; yPoint++) {
            CanvasDrawLine(texture, xStart, yPoint, xEnd, yPoint, color , lineWidth, LINE_STYLE_SOLID);
        }
    } else {
        CanvasDrawLine(texture, xStart, yStart, xEnd, yStart, color, lineWidth, LINE_STYLE_SOLID);
        CanvasDrawLine(texture, xStart, yStart, xStart, yEnd, color, lineWidth, LINE_STYLE_SOLID);
        CanvasDrawLine(texture, xEnd, yEnd, xEnd, yStart, color, lineWidth, LINE_STYLE_SOLID);
        CanvasDrawLine(texture, xEnd, yEnd, xStart, yEnd, color, lineWidth, LINE_STYLE_SOLID);
    }
}

void CanvasDrawCircle(Texture texture, UINT16 xCenter, UINT16 yCenter, UINT16 radius,
    UINT16 color, PixelSize lineWidth, DrawFillStyle circleFillStyle) {
    if (xCenter > texture.Width || yCenter >= texture.Height) {
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
                CanvasDrawPoint(texture, xCenter + xCurrent, yCenter + sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//1
                CanvasDrawPoint(texture, xCenter - xCurrent, yCenter + sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//2
                CanvasDrawPoint(texture, xCenter - sCountY, yCenter + xCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//3
                CanvasDrawPoint(texture, xCenter - sCountY, yCenter - xCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//4
                CanvasDrawPoint(texture, xCenter - xCurrent, yCenter - sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//5
                CanvasDrawPoint(texture, xCenter + xCurrent, yCenter - sCountY, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//6
                CanvasDrawPoint(texture, xCenter + sCountY, yCenter - xCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);//7
                CanvasDrawPoint(texture, xCenter + sCountY, yCenter + xCurrent, color, DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_FILL_STYLE);
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
            CanvasDrawPoint(texture, xCenter + xCurrent, yCenter + yCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//1
            CanvasDrawPoint(texture, xCenter - xCurrent, yCenter + yCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//2
            CanvasDrawPoint(texture, xCenter - yCurrent, yCenter + xCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//3
            CanvasDrawPoint(texture, xCenter - yCurrent, yCenter - xCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//4
            CanvasDrawPoint(texture, xCenter - xCurrent, yCenter - yCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//5
            CanvasDrawPoint(texture, xCenter + xCurrent, yCenter - yCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//6
            CanvasDrawPoint(texture, xCenter + yCurrent, yCenter - xCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//7
            CanvasDrawPoint(texture, xCenter + yCurrent, yCenter + xCurrent, color, lineWidth, DEFAULT_PIXEL_FILL_STYLE);//0

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

void CanvasDrawChar(Texture texture, UINT16 xPoint, UINT16 yPoint, const char ASCIIChar,
    sFONT* font, UINT16 foregroundColor, UINT16 backgroundColor) {
    UINT16 Page, Column;

    if (xPoint > texture.Width || yPoint > texture.Height) {
        SHOWDEBUG("Char exceeds texture boundaries\r\n");
        return;
    }

    uint32_t charOffset = (ASCIIChar - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &font->table[charOffset];

    for (Page = 0; Page < font->Height; Page ++ ) {
        for (Column = 0; Column < font->Width; Column ++ ) {
            if (TRANSPARENT == backgroundColor) {
                if (*ptr & (0x80 >> (Column % 8)))
                    CanvasSetPixel(texture, xPoint + Column, yPoint + Page, foregroundColor);
            } else {
                if (*ptr & (0x80 >> (Column % 8))) {
                    CanvasSetPixel(texture, xPoint + Column, yPoint + Page, foregroundColor);
                } else {
                    CanvasSetPixel(texture, xPoint + Column, yPoint + Page, backgroundColor);
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

/* Round-half-away-from-zero division; int64 numerator avoids overflow on the
 * radius*trig and doubled-rotation products. */
static int32_t CanvasRoundDivAway(int64_t numerator, int32_t denominator) {
    int64_t half = denominator / 2;
    if (numerator >= 0)
        return (int32_t)((numerator + half) / denominator);
    return (int32_t)((numerator - half) / denominator);
}

/* Rotate a doubled-delta glyph cell (dx2 = 2*Column-(W-1), dy2 = 2*Page-(H-1)) by
 * the Q16.16 trig cosV/sinV. Dividing by 2*TRIG_Q16_SCALE folds out both the
 * doubling (/2) and the Q16.16 scale. */
static void CanvasRotateGlyphOffset(int32_t dx2, int32_t dy2, int32_t cosV,
    int32_t sinV, int32_t *outX, int32_t *outY) {
    int64_t rx = (int64_t)dx2 * cosV - (int64_t)dy2 * sinV;
    int64_t ry = (int64_t)dx2 * sinV + (int64_t)dy2 * cosV;
    *outX = CanvasRoundDivAway(rx, 2 * TRIG_Q16_SCALE);
    *outY = CanvasRoundDivAway(ry, 2 * TRIG_Q16_SCALE);
}

void CanvasDrawCurvedChar(Texture texture, const char ASCIIChar, UINT16 xCenter, UINT16 yCenter,
    UINT16 radius, UINT16 startAngle, TextOrientation orientation, sFONT* font,
    UINT16 foregroundColor, UINT16 backgroundColor) {
    if (font == NULL)
        return;

    if (ASCIIChar < 0x20 || ASCIIChar > 0x7E)
        return;

    UINT16 angle = startAngle % 360;
    int32_t cosV = TrigCosQ16(angle);
    int32_t sinV = TrigSinQ16(angle);

    /* Glyph orientation is tangent to the circle, not radial: rotating by the
     * placement angle alone would point each letter out of the center. INWARDS uses
     * the tangent (angle + 90, upright at the top of the circle); OUTWARDS flips it
     * 180 degrees so the text faces the other way. */
    UINT16 orientAngle = (angle + (orientation == TEXT_ORIENTATION_OUTWARDS ? 270 : 90)) % 360;
    int32_t cosR = TrigCosQ16(orientAngle);
    int32_t sinR = TrigSinQ16(orientAngle);

    int32_t anchorX = (int32_t)xCenter + CanvasRoundDivAway((int64_t)radius * cosV, TRIG_Q16_SCALE);
    int32_t anchorY = (int32_t)yCenter + CanvasRoundDivAway((int64_t)radius * sinV, TRIG_Q16_SCALE);

    UINT16 width = font->Width;
    UINT16 height = font->Height;

    /* Glyph addressing mirrors CanvasDrawChar: 1bpp, MSB-first, byte-padded rows. */
    uint32_t charOffset = (ASCIIChar - ' ') * height * (width / 8 + (width % 8 ? 1 : 0));
    const unsigned char *ptr = &font->table[charOffset];

    UINT16 Page, Column;
    for (Page = 0; Page < height; Page++) {
        for (Column = 0; Column < width; Column++) {
            int bitSet = (*ptr & (0x80 >> (Column % 8))) != 0;

            int32_t dx2 = 2 * (int32_t)Column - ((int32_t)width - 1);
            int32_t dy2 = 2 * (int32_t)Page - ((int32_t)height - 1);

            int32_t offX, offY;
            CanvasRotateGlyphOffset(dx2, dy2, cosR, sinR, &offX, &offY);

            /* Skip negatives before the UINT16 cast so they don't wrap past the clip. */
            int32_t targetX = anchorX + offX;
            int32_t targetY = anchorY + offY;

            if (targetX >= 0 && targetY >= 0) {
                if (bitSet)
                    CanvasSetPixel(texture, (UINT16)targetX, (UINT16)targetY, foregroundColor);
                else if (backgroundColor != TRANSPARENT)
                    CanvasSetPixel(texture, (UINT16)targetX, (UINT16)targetY, backgroundColor);
            }

            if (Column % 8 == 7)
                ptr++;
        }

        if (width % 8 != 0)
            ptr++;
    }
}

void CanvasDrawCurvedText(Texture texture, const char *text, UINT16 xCenter, UINT16 yCenter,
    UINT16 radius, UINT16 startAngle, TextOrientation orientation, sFONT* font,
    UINT16 foregroundColor, UINT16 backgroundColor) {
    if (text == NULL || font == NULL)
        return;

    UINT16 width = font->Width;
    int32_t arcPixels = 0;

    for (const char *p = text; *p != '\0'; p++) {
        /* Angular step from arc length: deg = arcPixels/radius * 180/PI, in integers
         * (180/PI ~= 57296/1000) to stay float-free. Accumulating arcPixels avoids
         * per-character rounding drift; radius 0 stacks every glyph at the center.
         * INWARDS advances clockwise (+), OUTWARDS counter-clockwise (-). */
        int32_t offsetDeg = 0;
        if (radius != 0)
            offsetDeg = CanvasRoundDivAway((int64_t)arcPixels * 57296, (int32_t)radius * 1000);
        if (orientation == TEXT_ORIENTATION_OUTWARDS)
            offsetDeg = -offsetDeg;

        int32_t totalAngle = (int32_t)startAngle + offsetDeg;
        UINT16 charAngle = (UINT16)(((totalAngle % 360) + 360) % 360);

        CanvasDrawCurvedChar(texture, *p, xCenter, yCenter, radius, charAngle, orientation, font,
            foregroundColor, backgroundColor);

        arcPixels += (int32_t)width;
    }
}

void CanvasDrawText(Texture texture, UINT16 xStart, UINT16 yStart, const char * text,
    sFONT* font, UINT16 foregroundColor, UINT16 backgroundColor) {
    UINT16 xPoint = xStart;
    UINT16 yPoint = yStart;

    if (xStart > texture.Width || yStart > texture.Height) {
        SHOWDEBUG("Text exceeds texture boundaries\r\n");
        return;
    }

    while (* text != '\0') {
        //if X direction filled , reposition to(xStart,yPoint),yPoint is Y direction plus the Height of the character
        if ((xPoint + font->Width ) > texture.Width ) {
            xPoint = xStart;
            yPoint += font->Height;
        }

        // If the Y direction is full, reposition to(xStart, yStart)
        if ((yPoint  + font->Height ) > texture.Height ) {
            xPoint = xStart;
            yPoint = yStart;
        }

        CanvasDrawChar(texture, xPoint, yPoint, * text, font, foregroundColor, backgroundColor);
        text++; //The next character of the address
        xPoint += font->Width; //The next word of the abscissa increases the font of the broadband
    }
}

void CanvasDrawTextCN(Texture texture, UINT16 xStart, UINT16 yStart, const char * text, cFONT* font,
    UINT16 foregroundColor, UINT16 backgroundColor) {
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
                            if (TRANSPARENT == backgroundColor) {
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(texture, x + i, y + j, foregroundColor);
                                }
                            } else {
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(texture, x + i, y + j, foregroundColor);
                                } else {
                                    CanvasSetPixel(texture, x + i, y + j, backgroundColor);
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
                            if (TRANSPARENT == backgroundColor) {
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(texture, x + i, y + j, foregroundColor);
                                }
                            } else {
                                if (*ptr & (0x80 >> (i % 8))) {
                                    CanvasSetPixel(texture, x + i, y + j, foregroundColor);
                                } else {
                                    CanvasSetPixel(texture, x + i, y + j, backgroundColor);
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
void CanvasDrawNum(Texture texture, UINT16 xPoint, UINT16 yPoint, double number,
    sFONT* font, UINT16 digit, UINT16 foregroundColor, UINT16 backgroundColor) {
    int16_t numberBit = 0, textBit = 0;
    uint8_t textArray[ARRAY_LEN] = {0}, numberArray[ARRAY_LEN] = {0};
    uint8_t *pStr = textArray;
    int temp = number;
    float decimals;
    uint8_t i;

    if (xPoint > texture.Width || yPoint > texture.Height) {
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

    CanvasDrawText(texture, xPoint, yPoint, (const char*)pStr, font, foregroundColor, backgroundColor);
}

void CanvasDrawTime(Texture texture, UINT16 xStart, UINT16 yStart, DateTime *pTime, sFONT* font,
    UINT16 foregroundColor, UINT16 backgroundColor) {
    uint8_t value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    UINT16 dX = font->Width;
    CanvasDrawChar(texture, xStart, yStart, value[pTime->Hour / 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(texture, xStart + dX, yStart, value[pTime->Hour % 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(texture, xStart + dX  + dX / 4 + dX / 2, yStart, ':', font, foregroundColor, backgroundColor);
    CanvasDrawChar(texture, xStart + dX * 2 + dX / 2, yStart, value[pTime->Min / 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(texture, xStart + dX * 3 + dX / 2, yStart, value[pTime->Min % 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(texture, xStart + dX * 4 + dX / 2 - dX / 4, yStart, ':', font, foregroundColor, backgroundColor);
    CanvasDrawChar(texture, xStart + dX * 5, yStart, value[pTime->Sec / 10], font, foregroundColor, backgroundColor);
    CanvasDrawChar(texture, xStart + dX * 6, yStart, value[pTime->Sec % 10], font, foregroundColor, backgroundColor);
}

void CanvasDrawTexture(Texture texture, UINT16 xStart, UINT16 yStart, UINT16 imageWidth, UINT16 imageHeight) {
    int i, j;
    for (j = 0; j < imageHeight; j++) {
        for (i = 0; i < imageWidth; i++) {
            if (xStart + i < texture.HeightMemory && yStart + j < texture.WidthMemory) //Exceeded part does not display
                CanvasSetPixel(texture, xStart + i, yStart + j, (*(texture.Data + j * imageWidth * 2 + i * 2 + 1)) << 8 | (*(texture.Data + j * imageWidth * 2 + i * 2)));
                // j*imageWidth*2 = Y offset
                // i*2 = X offset
        }
    }
}

void CanvasDrawBitmap(Texture texture, const unsigned char* bitmap) {
    UINT16 x, y;
    UINT32 addr = 0;

    for (y = 0; y < texture.HeightByte; y++) {
        for (x = 0; x < texture.WidthByte; x++) {//8 pixel =  1 byte
            addr = x + y * texture.WidthByte;
            texture.Data[addr] = (unsigned char)bitmap[addr];
        }
    }
}

void CanvasDrawBitmapBlock(Texture texture, const unsigned char* bitmap, UINT8 region) {
    UINT16 x, y;
    UINT32 addr = 0;

    for (y = 0; y < texture.HeightByte; y++) {
        for (x = 0; x < texture.WidthByte; x++) {//8 pixel =  1 byte
            addr = x + y * texture.WidthByte ;
            texture.Data[addr] = \
            (unsigned char)bitmap[addr+ (texture.HeightByte)*texture.WidthByte*(region - 1)];
        }
    }
}

void CanvasDrawBitmapToArea(Texture texture, UINT16 x, UINT16 y, const unsigned char* bitmap,
    UINT16 width, UINT16 height) {
    uint16_t i, j, byteWidth = (width + 7) / 8;

    for (j = 0; j < height; j ++) {
        for (i = 0; i < width; i ++ ) {
            if (*(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
                CanvasSetPixel(texture, x + i, y + j, 0xffff);
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

void CanvasDrawPng(Texture texture, FIL *file) {
    SHOWDEBUG("Creating read structure\n");
    png_structp pngPointer = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, PngShowError, NULL);

    if (pngPointer == NULL) {
        SHOWDEBUG("png_create_read_struct error\n");
        return;
    }

    SHOWDEBUG("Allocating memory for texture information\n");
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
    /* png_read_info gets information about PNG file before the first IDAT (texture data chunk). REQUIRED. */
    png_read_info(pngPointer, infoPointer);

    SHOWDEBUG("\nParsing texture info\n");
    png_uint_32 width, height;
    int bitDepth, colorType, interlaceType;
    png_get_IHDR(pngPointer, infoPointer, &width, &height, &bitDepth, &colorType, &interlaceType, NULL, NULL);
    SHOWDEBUG("PNG info: width: %d, height: %d, bit_depth: %d\n", width, height, bitDepth);

    int col, row;
    int maxCol = width > texture.Width ? texture.Width : width; /* won't write outside texture. */
    int maxRow = height > texture.Height ? texture.Height : height;

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
                /* if the texture is paletted but we don't have a palette, display as grayscale using palette index. */
                png_bytep pixel = &rowPointers[col];
                red = *(pixel++);
                green = *(pixel++);
                blue = *(pixel++);
            }

            /* The LCD uses RGB565 16-bits format: RRRRRGGG GGGBBBBB */
            UINT16 color = (UINT16)(((red & 0b11111000) | ((green & 0b11100000) >> 5)) << 8)
                | (UINT16)(((green & 0b00011100) << 3) | ((blue & 0b11111000) >> 3));
            CanvasSetPixel(texture, col, row, color);
        }

        png_free(pngPointer, rowBuffer);
        rowPointers = NULL;
    }

    png_destroy_read_struct(&pngPointer, &infoPointer, NULL);
}

void CanvasDrawPngToArea(Texture texture, FIL *file, UINT16 xSource, UINT16 ySource, UINT16 width, UINT16 height, UINT16 xTarget, UINT16 yTarget) {
    if (file == NULL)
        return;

    SHOWDEBUG("Creating read structure\n");
    png_structp pngPointer = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, PngShowError, NULL);

    if (pngPointer == NULL) {
        SHOWDEBUG("png_create_read_struct error\n");
        return;
    }

    SHOWDEBUG("Allocating memory for texture information\n");
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
    /* png_read_info gets information about PNG file before the first IDAT (texture data chunk). REQUIRED. */
    png_read_info(pngPointer, infoPointer);

    SHOWDEBUG("\nParsing texture info\n");
    png_uint_32 pngWidth, pngHeight;
    int bitDepth, colorType, interlaceType;
    png_get_IHDR(pngPointer, infoPointer, &pngWidth, &pngHeight, &bitDepth, &colorType, &interlaceType, NULL, NULL);
    SHOWDEBUG("PNG info: width: %d, height: %d, bit_depth: %d\n", pngWidth, pngHeight, bitDepth);

    int effectiveWidth = width > pngWidth - xSource ? pngWidth - xSource : width;
    int effectiveHeight = height > pngHeight - ySource ? pngHeight - ySource : height;
    if (xSource >= pngWidth || ySource >= pngHeight) {
        effectiveWidth = 0;
        effectiveHeight = 0;
    }
    int maxCol = effectiveWidth > (int)(texture.Width - xTarget) ? (int)(texture.Width - xTarget) : effectiveWidth;
    int maxRow = effectiveHeight;

    int col, row;

    int num_palette = 0;
    png_colorp palette = NULL;

    if (colorType == PNG_COLOR_TYPE_PALETTE)
        png_get_PLTE(pngPointer, infoPointer, &palette, &num_palette);

    for (row = 0; row < (int)pngHeight; row++) {
        if (row >= (int)ySource + maxRow)
            break;

        png_bytep rowBuffer = (png_bytep)png_malloc(pngPointer, png_get_rowbytes(pngPointer, infoPointer));
        rowPointers = rowBuffer; // track for cleanup before any call that may longjmp
        png_read_rows(pngPointer, &rowBuffer, NULL, 1);

        if (row < (int)ySource) {
            png_free(pngPointer, rowBuffer);
            rowPointers = NULL;
            continue;
        }

        for (col = xSource; col < (int)xSource + maxCol; col++) {
            png_byte red, green, blue;

            if ((colorType == PNG_COLOR_TYPE_PALETTE) && (palette != NULL)) {
                red = palette[rowPointers[col]].red;
                green = palette[rowPointers[col]].green;
                blue = palette[rowPointers[col]].blue;
            } else {
                /* if the texture is paletted but we don't have a palette, display as grayscale using palette index. */
                png_bytep pixel = &rowPointers[col];
                red = *(pixel++);
                green = *(pixel++);
                blue = *(pixel++);
            }

            /* The LCD uses RGB565 16-bits format: RRRRRGGG GGGBBBBB */
            UINT16 color = (UINT16)(((red & 0b11111000) | ((green & 0b11100000) >> 5)) << 8)
                | (UINT16)(((green & 0b00011100) << 3) | ((blue & 0b11111000) >> 3));

            if (color != TRANSPARENT)
                CanvasSetPixel(texture, xTarget + col - xSource, yTarget + row - ySource, color);
        }

        png_free(pngPointer, rowBuffer);
        rowPointers = NULL;
    }

    png_destroy_read_struct(&pngPointer, &infoPointer, NULL);
}
