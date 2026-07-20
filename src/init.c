#include <stdio.h>
#include <string.h>

#include <SDL2/SDL_image.h>

#include "../headers/global.h"

static SDL_Renderer *createRendererWithFallbacks(SDL_Window *window)
{
    static const Uint32 flags[] = {
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
        SDL_RENDERER_ACCELERATED,
        SDL_RENDERER_SOFTWARE
    };

    for (size_t i = 0; i < sizeof(flags) / sizeof(flags[0]); ++i) {
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, flags[i]);
        if (renderer) return renderer;
    }
    return NULL;
}

int initializeSDL(Instance *instance)
{
    char *basePath;
    int imageFlags = IMG_INIT_PNG;

    if (!instance) return -1;
    memset(instance, 0, sizeof(*instance));

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "0");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return -1;
    }
    if ((IMG_Init(imageFlags) & imageFlags) != imageFlags) {
        fprintf(stderr, "SDL_image initialization failed: %s\n", IMG_GetError());
        SDL_Quit();
        return -1;
    }

    basePath = SDL_GetBasePath();
    if (basePath) {
        snprintf(instance->basePath, sizeof(instance->basePath), "%s", basePath);
        SDL_free(basePath);
    }

    instance->window = SDL_CreateWindow(
        "Bunker Run - Raycasting Survival",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!instance->window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return -1;
    }
    SDL_SetWindowMinimumSize(instance->window, 640, 360);

    instance->renderer = createRendererWithFallbacks(instance->window);
    if (!instance->renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(instance->window);
        instance->window = NULL;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }
    if (SDL_RenderSetLogicalSize(instance->renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT) != 0) {
        fprintf(stderr, "Unable to set logical render size: %s\n", SDL_GetError());
        cleanupSDL(instance);
        return -1;
    }
    SDL_SetRenderDrawBlendMode(instance->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(instance->renderer, 0, 0, 0, 255);
    return 0;
}

void cleanupSDL(Instance *instance)
{
    if (!instance) return;
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_ShowCursor(SDL_ENABLE);

    if (instance->renderer) {
        SDL_DestroyRenderer(instance->renderer);
        instance->renderer = NULL;
    }
    if (instance->window) {
        SDL_DestroyWindow(instance->window);
        instance->window = NULL;
    }
    IMG_Quit();
    SDL_Quit();
}
