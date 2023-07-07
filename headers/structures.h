#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

/**
 * struct Player - Structure to represent a player position and direction
 * @x: (x) position of player on the 2D map
 * @y: (y) position of player on the 2D map
 * @dirX: (x) facing direction of player
 * @dirY: (y) facing direction of player
 * @planeX: (x) plane
 * @planeY: (y) plane
 *
 */

typedef struct Player
{
	float x;
	float y;
	float dirX;
	float dirY;
	float planeX;
	float planeY;
} Player;

/**
 * struct Init
 *
 *
 *
 *
 */

typedef struct Instance
{
	SDL_Window *window;
	SDL_Renderer *renderer;
} Instance;


int initializeSDL(Instance *instance);
void cleanupSDL(Instance *instance);


#endif
