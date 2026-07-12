#include <SDL2/SDL.h>
#include <png.h>
#include "LCDRenderer.h"
#include "LCDSetup.h"

extern SDL_Window *sdlWindow;
extern SDL_Renderer *sdlRenderer;
extern SDL_Texture *sdlTexture;

void LCDSetDisplayArea(UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd)
{
}

void LCDClear(UINT16 fillColor)
{
    UINT16 texture[LCD.WIDTH * LCD.HEIGHT];

    for (int i = 0; i < LCD.WIDTH * LCD.HEIGHT; i++) {
        texture[i] = fillColor;
    }

    SDL_UpdateTexture(sdlTexture, NULL, texture, LCD.WIDTH * sizeof(UINT16));
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
}

void LCDRenderTexture(UINT8 *texture)
{
    UINT16 swapped[LCD.WIDTH * LCD.HEIGHT];

    for (int i = 0; i < LCD.WIDTH * LCD.HEIGHT; i++) {
        UINT32 addr = i * 2;
        swapped[i] = (texture[addr] << 8) | texture[addr + 1];
    }

    SDL_UpdateTexture(sdlTexture, NULL, swapped, LCD.WIDTH * sizeof(UINT16));
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
}

void LCDRenderTextureInArea(UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd, UINT8 *texture)
{
    UINT16 regionWidth = xEnd - xStart;
    UINT16 regionHeight = yEnd - yStart - 1;
    UINT16 swapped[regionWidth * regionHeight];
    int idx = 0;

    for (UINT16 row = yStart; row < yEnd - 1; row++) {
        UINT32 baseAddr = (xStart + row * LCD.WIDTH) * 2;
        for (UINT16 col = 0; col < regionWidth; col++) {
            UINT32 addr = baseAddr + col * 2;
            swapped[idx++] = (texture[addr] << 8) | texture[addr + 1];
        }
    }

    SDL_Rect rect = { xStart, yStart, regionWidth, regionHeight };
    SDL_UpdateTexture(sdlTexture, &rect, swapped, regionWidth * sizeof(UINT16));
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
}

void LCDRenderPoint(UINT16 x, UINT16 y, UINT16 color)
{
    UINT16 swapped = (color << 8) | (color >> 8);
    SDL_Rect rect = { x, y, 1, 1 };
    SDL_UpdateTexture(sdlTexture, &rect, &swapped, sizeof(UINT16));
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
}

void LCDRenderPng(FIL *file)
{
}

SDLState sdlState;
bool LCDRenderShouldClose()
{
    if (sdlState.ShouldQuit)
        return true;

    SDL_Event event;

    if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
        sdlState.ShouldQuit = true;

    return sdlState.ShouldQuit;
}

void LCDRenderClose()
{
    SDL_DestroyTexture(sdlTexture);
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit(); // shutdown all SDL subsystems
}