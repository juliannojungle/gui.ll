#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include "LCDSetup.h"

LCD_ATTRIBUTES LCD;

SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;

/* Shared framebuffer: render functions write here, render thread reads it. */
volatile UINT8 framebuffer[LCD_WIDTH * LCD_HEIGHT * 2];
volatile bool hasNewFrame;
volatile bool shouldClose;
volatile bool sdlReady;

static void *RenderThreadFunc(void *arg)
{
    (void)arg;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        shouldClose = true;
        sdlReady = true;
        return NULL;
    }

    sdlWindow = SDL_CreateWindow("LCD Simulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        LCD_WIDTH, LCD_HEIGHT, SDL_WINDOW_SHOWN);
    if (!sdlWindow) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        shouldClose = true;
        sdlReady = true;
        return NULL;
    }

    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!sdlRenderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
        shouldClose = true;
        sdlReady = true;
        return NULL;
    }

    sdlTexture = SDL_CreateTexture(sdlRenderer,
        SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, LCD_WIDTH, LCD_HEIGHT);
    if (!sdlTexture) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(sdlRenderer);
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
        shouldClose = true;
        sdlReady = true;
        return NULL;
    }

    sdlReady = true;

    /* Render loop */
    while (!shouldClose) {
        /* Render new frame if available */
        if (hasNewFrame) {
            hasNewFrame = false;

            UINT16 swapped[LCD_WIDTH * LCD_HEIGHT];
            for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
                UINT32 addr = i * 2;
                swapped[i] = (framebuffer[addr] << 8) | framebuffer[addr + 1];
            }

            SDL_UpdateTexture(sdlTexture, NULL, swapped, LCD_WIDTH * sizeof(UINT16));
            SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
            SDL_RenderPresent(sdlRenderer);
        }

        /* Pump and poll events */
        SDL_PumpEvents();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldClose = true;
            }
        }

        SDL_Delay(1);
    }

    /* Cleanup */
    SDL_DestroyTexture(sdlTexture);
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
    sdlReady = false;

    return NULL;
}

int LCDInitialize()
{
    LCD.WIDTH = LCD_WIDTH;
    LCD.HEIGHT = LCD_HEIGHT;
    LCD.SCAN_DIR = HORIZONTAL;

    hasNewFrame = false;
    shouldClose = false;
    sdlReady = false;

    pthread_t renderThread;
    if (pthread_create(&renderThread, NULL, RenderThreadFunc, NULL) != 0) {
        fprintf(stderr, "Failed to create render thread\n");
        return EXIT_FAILURE;
    }
    pthread_detach(renderThread);

    /* Wait for SDL to be initialized before returning */
    while (!sdlReady) {
        SDL_Delay(1);
    }

    return shouldClose ? EXIT_FAILURE : EXIT_SUCCESS;
}
