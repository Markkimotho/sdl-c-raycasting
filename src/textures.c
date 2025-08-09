#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <stdio.h>
#include "../headers/structures.h"

int loadTextures(Instance *instance)
{
    SDL_Surface *surf = IMG_Load("assets/wall2.png");
    if (!surf) {
        fprintf(stderr, "IMG_Load failed: %s\n", IMG_GetError());
        return -1;
    }
    instance->wallTexture = SDL_CreateTextureFromSurface(instance->renderer, surf);
    SDL_FreeSurface(surf);
    if (!instance->wallTexture) {
        fprintf(stderr, "SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        return -1;
    }
    return 0;
}

void destroyTextures(Instance *instance)
{
    if (instance->wallTexture) {
        SDL_DestroyTexture(instance->wallTexture);
        instance->wallTexture = NULL;
    }
}
