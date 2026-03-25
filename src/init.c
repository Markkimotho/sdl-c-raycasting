#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/structures.h"
#include "../headers/global.h"

int initializeSDL(Instance *instance)
{
    if (!instance) {
        fprintf(stderr, "Error: initializeSDL called with NULL instance\n");
        return -1;
    }

    /* Zero-initialize instance */
    memset(instance, 0, sizeof(Instance));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "Error: SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        fprintf(stderr, "Error: IMG_Init failed: %s\n", IMG_GetError());
        SDL_Quit();
        return -1;
    }

    instance->window = SDL_CreateWindow("Raycasting Engine",
                        SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED,
                        WINDOW_WIDTH,
                        WINDOW_HEIGHT,
                        SDL_WINDOW_SHOWN);
    if (!instance->window) {
        fprintf(stderr, "Error: SDL_CreateWindow failed: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    instance->renderer = SDL_CreateRenderer(instance->window, -1, 
                                            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!instance->renderer) {
        fprintf(stderr, "Error: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(instance->window);
        instance->window = NULL;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    /* Set reasonable default draw color */
    SDL_SetRenderDrawColor(instance->renderer, 0, 0, 0, 255);

    return 0;
}

void cleanupSDL(Instance *instance)
{
    if (!instance) {
        fprintf(stderr, "Warning: cleanupSDL called with NULL instance\n");
        return;
    }

    /* Destroy renderer first */
    if (instance->renderer) {
        SDL_DestroyRenderer(instance->renderer);
        instance->renderer = NULL;
    }

    /* Then destroy window */
    if (instance->window) {
        SDL_DestroyWindow(instance->window);
        instance->window = NULL;
    }

    /* Cleanup SDL libraries */
    IMG_Quit();
    SDL_Quit();
}
