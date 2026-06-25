#ifndef __LCD_RENDERER_H
#define __LCD_RENDERER_H

#include "ff.h"
#include "Types.h"

void LCDSetDisplayArea(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend);
void LCDClear(UWORD FillColor);
void LCDRenderTexture(UWORD *Image);
void LCDRenderTextureInArea(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd, UWORD *image);
void LCDRenderPoint(UWORD x, UWORD y, UWORD color);
void LCDRenderPng(FIL *file);

#endif
