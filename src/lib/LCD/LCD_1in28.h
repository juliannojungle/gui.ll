/*****************************************************************************
* | File        :   LCD_1in28.h
* | Author      :   Waveshare team, Julianno F. C. Silva (@juliannojungle)
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master
*                and enhance portability
*----------------
* | This version:   V1.0
* | Date        :   2023-03-26
* | Info        :   Basic version
*
******************************************************************************/
#ifndef __LCD_1IN28_H
#define __LCD_1IN28_H

#define LCD_1IN28_HEIGHT 240
#define LCD_1IN28_WIDTH 240

#define HORIZONTAL 0
#define VERTICAL 1

typedef struct{
    UWORD WIDTH;
    UWORD HEIGHT;
    UBYTE SCAN_DIR;
}LCD_ATTRIBUTES;

#endif
