#include <string.h>
#include <SDL2/SDL.h>
#include <png.h>
#include "LCDRenderer.h"
#include "LCDSetup.h"

/* Defined in LCDSetup.c */
extern volatile UINT8 framebuffer[];
extern volatile bool hasNewFrame;
extern volatile bool sdlReady;
extern volatile bool shouldClose;

void LCDSetDisplayArea(UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd)
{
    (void)xStart; (void)yStart; (void)xEnd; (void)yEnd;
}

void LCDClear(UINT16 fillColor)
{
    for (int i = 0; i < LCD.WIDTH * LCD.HEIGHT; i++) {
        UINT32 addr = i * 2;
        framebuffer[addr] = (fillColor >> 8) & 0xFF;
        framebuffer[addr + 1] = fillColor & 0xFF;
    }
    hasNewFrame = true;
}

void LCDRenderTexture(UINT8 *texture)
{
    memcpy((void *)framebuffer, texture, LCD.WIDTH * LCD.HEIGHT * 2);
    hasNewFrame = true;
}

void LCDRenderTextureInArea(UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd, UINT8 *texture)
{
    UINT16 regionWidth = xEnd - xStart;

    for (UINT16 row = yStart; row < yEnd - 1; row++) {
        UINT32 dstAddr = (xStart + row * LCD.WIDTH) * 2;
        UINT32 srcAddr = ((row - yStart) * regionWidth) * 2;
        memcpy((void *)&framebuffer[dstAddr], &texture[srcAddr], regionWidth * 2);
    }
    hasNewFrame = true;
}

void LCDRenderPoint(UINT16 x, UINT16 y, UINT16 color)
{
    UINT32 addr = (x + y * LCD.WIDTH) * 2;
    framebuffer[addr] = (color >> 8) & 0xFF;
    framebuffer[addr + 1] = color & 0xFF;
    hasNewFrame = true;
}

void LCDRenderPng(FIL *file)
{
    (void)file;
}

bool LCDRenderShouldClose()
{
    return shouldClose;
}

void LCDRenderClose()
{
    /* Wait for SDL to be destroyed before quitting */
    while (sdlReady) {
        SDL_Delay(1);
    }

    SDL_Quit();
}
