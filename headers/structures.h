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
    SDL_Texture *wallTextures[6]; /* index 0 unused, 1-5 are wall types */
    Sprite sprites[32];
    int numSprites;
    int isNight;  /* 1 for night, 0 for day */
} Instance;

#endif /* _STRUCTURES_H_ */
