#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "structures.h"
#include "maps.h"

#define WINDOW_WIDTH 1480
#define WINDOW_HEIGHT 820

/* Robustness & Performance Constants */
#define TARGET_FPS 60
#define FRAME_DELAY_MS (1000 / TARGET_FPS)  // ~16.67ms per frame
#define MAX_DELTA_TIME 0.05f                 // Clamp deltaTime to prevent wall-clipping
#define MIN_DISTANCE 0.0001f                 // Minimum wall distance to prevent division issues
#define MAX_SPRITE_COUNT 32
#define MAX_WALL_TYPES 6
#define PLAYER_COLLISION_BUFFER 0.1f         // Distance to maintain from walls

/* map is [row][col] => map[y][x] */
extern int map[MAP_HEIGHT][MAP_WIDTH];

extern int screenBuffer[WINDOW_WIDTH][WINDOW_HEIGHT];

void drawMap(Instance *instance, Player *player);
void castRays(Instance *instance, Player *player);
void drawScreen(Instance *instance, Player *player);
void movePlayer(Player *player, const Uint8 *keystate, float deltaTime);
void rotatePlayer(Player *player, const Uint8 *keystate, float deltaTime);
int selectMapInteractive(Instance *instance);


/* --- Prototypes for init/textures --- */
int initializeSDL(Instance *instance);
void cleanupSDL(Instance *instance);
int loadTextures(Instance *instance);
void destroyTextures(Instance *instance);
// int loadSprites(Instance *instance);

#endif /* _GLOBAL_H_ */
