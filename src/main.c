#include <stdio.h>
#include "../headers/global.h"




/**
 * main - Entry point into the program
 *
 * @argv: The number of arguments the program takes on the CLI
 * @args: An array that holds these arguments from CLI
 *
 * Return: (SUCCESS) on success & (FAIL) on failure
 *
 */

int main(int argv, char *args[])
{
	int game_is_running = SUCCESS;
	float speed = 0.05;
	Instance instance;
	
	initializeSDL(&instance);
	loadTextures();

	setup();

	SDL_Event event;

	/** Game Loop */
	while (game_is_running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				game_is_running = FAIL;
			else if (event.type == SDL_KEYDOWN)
			{
				SDL_Keycode keycode = event.key.keysym.sym;

				if (keycode == SDLK_ESCAPE)
					game_is_running = FAIL;
			}
		}


	drawScreen();
	castRays();
	movePlayer(speed);
	rotatePlayer(speed);
	}
	
	destroyTextures();
	cleanupSDL(&instance);

	return (SUCCESS);
}

/**
 * setup - Function that sets the player's position
 * @void: Takes no parameter
 *
 * Return: None
 *
 */

void setup(Player *player, int map[MAP_WIDTH][MAP_HEIGHT])
{
	player->x = 3.5;
	player->y = 3.5;
	player->dirX = -1.0;
	player->dirY = 0.0;
	player->planeX = 0.0;
	player->planeY = 0.66;
}
