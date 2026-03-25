#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../headers/structures.h"
#include "../headers/global.h"

int loadTextures(Instance *instance)
{
    if (!instance || !instance->renderer) {
        fprintf(stderr, "Error: loadTextures called with NULL instance or renderer\n");
        return -1;
    }

    const char *textureFile = "assets/colorstone.png";
    
    instance->wallTexture = NULL;

    SDL_Surface *surf = IMG_Load(textureFile);
    if (!surf) {
        fprintf(stderr, "Error: IMG_Load failed for '%s': %s\n", textureFile, IMG_GetError());
        destroyTextures(instance);
        return -1;
    }

    instance->wallTexture = SDL_CreateTextureFromSurface(instance->renderer, surf);
    SDL_FreeSurface(surf);
    
    if (!instance->wallTexture) {
        fprintf(stderr, "Error: SDL_CreateTextureFromSurface failed for '%s': %s\n",
                textureFile, SDL_GetError());
        destroyTextures(instance);
        return -1;
    }
    
    return 0;
}

// int loadSprites(Instance *instance)
// {
//     const char *spriteFiles[] = {
//         "assets/barrel.png",
//         "assets/pillar.png"
//     };
//     srand((unsigned)time(NULL));
//     instance->numSprites = 0;

//     for (int i = 0; i < 6; i++) {
//         float rx = 1 + rand() % (MAP_WIDTH - 2);
//         float ry = 1 + rand() % (MAP_HEIGHT - 2);
//         if (map[(int)ry][(int)rx] == 0) {
//             int texIndex = rand() % 2;
//             SDL_Surface *surf = IMG_Load(spriteFiles[texIndex]);
//             if (!surf) continue;
//             Sprite *spr = &instance->sprites[instance->numSprites++];
//             spr->texture = SDL_CreateTextureFromSurface(instance->renderer, surf);
//             SDL_FreeSurface(surf);
//             spr->x = rx + 0.5f;
//             spr->y = ry + 0.5f;
//         }
//     }
//     return 0;
// }

void destroyTextures(Instance *instance)
{
    if (!instance) {
        fprintf(stderr, "Warning: destroyTextures called with NULL instance\n");
        return;
    }

    if (instance->wallTexture) {
        SDL_DestroyTexture(instance->wallTexture);
        instance->wallTexture = NULL;
    }

    /* Destroy sprite textures */
    if (instance->numSprites < 0 || instance->numSprites > MAX_SPRITE_COUNT) {
        fprintf(stderr, "Warning: Invalid sprite count (%d), clamping to 0\n", instance->numSprites);
        instance->numSprites = 0;
    }

    for (int i = 0; i < instance->numSprites; i++) {
        if (instance->sprites[i].texture) {
            SDL_DestroyTexture(instance->sprites[i].texture);
            instance->sprites[i].texture = NULL;
        }
    }
}
