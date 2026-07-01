#ifndef __LCD_SETUP_H
#define __LCD_SETUP_H

#include "Types.h"

#define LCD_HEIGHT 240
#define LCD_WIDTH 240

#define HORIZONTAL 0
#define VERTICAL 1

typedef struct{
    UINT16 WIDTH;
    UINT16 HEIGHT;
    UINT8 SCAN_DIR;
}LCD_ATTRIBUTES;

extern LCD_ATTRIBUTES LCD;

int LCDInitialize();

#endif
