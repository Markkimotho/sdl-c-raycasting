#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "structures.h"

#define WINDOW_WIDTH 1480
#define WINDOW_HEIGHT 820
#define MAP_WIDTH 16
#define MAP_HEIGHT 16

/* map is [row][col] => map[y][x] */
static const int map[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,3,3,3,3,3,3,3,3,3,0,0,1},
    {1,0,2,0,4,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,4,0,5,0,5,0,5,0,5,0,0,1},
    {1,0,2,0,4,0,5,0,5,0,5,0,5,0,0,1},
    {1,0,2,0,4,0,5,0,5,0,5,0,5,0,0,1},
    {1,0,2,0,4,0,5,0,5,0,5,0,5,0,0,1},
    {1,0,2,0,4,0,5,0,5,0,5,0,5,0,0,1},
    {1,0,2,0,4,0,5,0,5,0,5,0,5,0,0,1},
    {1,0,2,0,4,0,5,0,5,0,5,0,5,0,0,1},
    {1,0,2,0,4,0,5,0,5,0,5,0,5,0,0,1},
    {1,0,2,0,4,0,5,0,5,0,5,0,5,0,0,1},
    {1,0,0,0,3,3,3,0,4,4,4,0,5,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

extern int screenBuffer[WINDOW_WIDTH][WINDOW_HEIGHT];

void drawMap(Instance *instance, Player *player);
void castRays(Instance *instance, Player *player);
void drawScreen(Instance *instance, Player *player);
void movePlayer(Player *player, const Uint8 *keystate, float deltaTime);
void rotatePlayer(Player *player, const Uint8 *keystate, float deltaTime);


/* --- Prototypes for init/textures --- */
int initializeSDL(Instance *instance);
void cleanupSDL(Instance *instance);
int loadTextures(Instance *instance);
void destroyTextures(Instance *instance);
// int loadSprites(Instance *instance);

#endif /* _GLOBAL_H_ */
