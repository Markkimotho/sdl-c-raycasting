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

	cleanupSDL(&instance);

	return (SUCCESS);
}
