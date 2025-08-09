#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include <SDL2/SDL.h>

typedef struct Player {
    float x;
    float y;
    float dirX;
    float dirY;
    float planeX;
    float planeY;
} Player;

typedef struct Sprite {
    float x, y;
    SDL_Texture *texture;
} Sprite;

typedef struct Instance {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *wallTextures[6]; // 0 unused
    Sprite sprites[32];
    int numSprites;
} Instance;

#endif /* _STRUCTURES_H_ */
