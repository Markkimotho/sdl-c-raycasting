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

    const char *files[MAX_WALL_TYPES] = {
        NULL,                       /* Index 0 unused */
        "assets/colorstone.png",
        "assets/redbrick.png",
        "assets/purplestone.png",
        "assets/wood.png",
        "assets/rocky.png"
    };

    for (int i = 0; i < MAX_WALL_TYPES; i++)
        instance->wallTextures[i] = NULL;

    for (int i = 1; i < MAX_WALL_TYPES; i++) {
        SDL_Surface *surf = IMG_Load(files[i]);
        if (!surf) {
            fprintf(stderr, "Error: IMG_Load failed for '%s': %s\n", files[i], IMG_GetError());
            destroyTextures(instance);
            return -1;
        }
        instance->wallTextures[i] = SDL_CreateTextureFromSurface(instance->renderer, surf);
        SDL_FreeSurface(surf);
        if (!instance->wallTextures[i]) {
            fprintf(stderr, "Error: SDL_CreateTextureFromSurface failed for '%s': %s\n",
                    files[i], SDL_GetError());
            destroyTextures(instance);
            return -1;
        }
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

    for (int i = 0; i < MAX_WALL_TYPES; i++) {
        if (instance->wallTextures[i]) {
            SDL_DestroyTexture(instance->wallTextures[i]);
            instance->wallTextures[i] = NULL;
        }
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
