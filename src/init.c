#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <stdio.h>
#include "../headers/structures.h"
#include "../headers/global.h"

int initializeSDL(Instance *instance)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError());
        SDL_Quit();
        return -1;
    }

    instance->window = SDL_CreateWindow("Raycasting Engine",
                        SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED,
                        WINDOW_WIDTH,
                        WINDOW_HEIGHT,
                        0);
    if (!instance->window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    instance->renderer = SDL_CreateRenderer(instance->window, -1, SDL_RENDERER_ACCELERATED);
    if (!instance->renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(instance->window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    instance->wallTexture = NULL;
    return 0;
}

void cleanupSDL(Instance *instance)
{
    if (instance->wallTexture) {
        SDL_DestroyTexture(instance->wallTexture);
        instance->wallTexture = NULL;
    }
    if (instance->renderer) SDL_DestroyRenderer(instance->renderer);
    if (instance->window) SDL_DestroyWindow(instance->window);
    IMG_Quit();
    SDL_Quit();
}
