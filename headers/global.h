#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include "structures.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define MAP_WIDTH 16
#define MAP_HEIGHT 16
#define NUM_RAYS WINDOW_WIDTH
#define NUM_TEXTURES 2
#define FAIL 1
#define SUCCESS 0

int map[MAP_WIDTH][MAP_HEIGHT] = {
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
	{1,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1},
	{1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,0,0,0,0,0,1,0,0,1,0,0,0,1,0,1},
	{1,0,0,0,0,0,1,0,0,1,0,0,0,1,0,1},
	{1,0,0,0,0,0,1,0,0,1,0,0,0,1,0,1},
	{1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

int screenBuffer[WINDOW_WIDTH][NUM_RAYS];

/***** PROTOTYPES *****/
void setup(Player *player, int map[MAP_WIDTH][MAP_HEIGHT]);
void drawMap(Instance *instance, int map[MAP_WIDTH][MAP_HEIGHT]);
void drawScreen(Instance *instance, int map[MAP_WIDTH][MAP_HEIGHT]);
void movePlayer(Player *player, float deltaTime);
void rotatePlayer(Player *player, float deltaTime);
void castRays(Player *player, int map[MAP_WIDTH][MAP_HEIGHT]);

#endif
