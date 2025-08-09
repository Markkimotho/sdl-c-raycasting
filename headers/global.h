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
    {1,0,1,0,1,1,1,1,1,1,1,1,1,0,0,1},
    {1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,1,1,1,0,1,1,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

extern int screenBuffer[WINDOW_WIDTH][WINDOW_HEIGHT]; /* optional */

void drawMap(Instance *instance, Player *player);
void castRays(Instance *instance, Player *player);
void drawScreen(Instance *instance, Player *player);
void movePlayer(Player *player, float deltaTime);
void rotatePlayer(Player *player, float deltaTime);

/* --- Prototypes for init/textures (added) --- */
int initializeSDL(Instance *instance);
void cleanupSDL(Instance *instance);
int loadTextures(Instance *instance);
void destroyTextures(Instance *instance);

#endif /* _GLOBAL_H_ */
