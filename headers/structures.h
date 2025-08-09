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

typedef struct Instance {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *wallTexture;
} Instance;

#endif /* _STRUCTURES_H_ */
