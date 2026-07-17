#ifndef __LCD_RENDERER_H
#define __LCD_RENDERER_H

#include <stdbool.h>
#include "ff.h"
#include "Types.h"

void LCDSetDisplayArea(UINT16 Xstart, UINT16 Ystart, UINT16 Xend, UINT16 Yend);
void LCDClear(UINT16 FillColor);
void LCDRenderTexture(UINT8 *texture);
void LCDRenderTextureInArea(UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd, UINT8 *texture);
void LCDRenderPoint(UINT16 x, UINT16 y, UINT16 color);
void LCDRenderPng(FIL *file);
bool LCDRenderShouldClose();
void LCDRenderClose();

#endif
