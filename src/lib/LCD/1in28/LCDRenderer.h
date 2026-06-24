#ifndef __LCD_RENDERER_H
#define __LCD_RENDERER_H

#include "Types.h"

void LCDSetDisplayArea(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend);
void LCDClear(UWORD FillColor);
void LCDDisplayTexture(UWORD *Image);
void LCDDisplayTextureInArea(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd, UWORD *image);
void LCDDisplayTexturePoint(UWORD x, UWORD y, UWORD color);

#endif
