#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "LCDSetup.h"

LCD_ATTRIBUTES LCD;

SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;

int LCDInitialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    sdlWindow = SDL_CreateWindow("LCD Simulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        LCD_WIDTH, LCD_HEIGHT, SDL_WINDOW_SHOWN);
    if (!sdlWindow) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!sdlRenderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    sdlTexture = SDL_CreateTexture(sdlRenderer,
        SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, LCD_WIDTH, LCD_HEIGHT);
    if (!sdlTexture) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(sdlRenderer);
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    LCD.WIDTH = LCD_WIDTH;
    LCD.HEIGHT = LCD_HEIGHT;
    LCD.SCAN_DIR = HORIZONTAL;

    return EXIT_SUCCESS;
}
