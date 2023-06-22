#include <stdio.h>
#include <SDL2/SDL.h>
#include "../headers/global.h"


/**
 * main - The entry point to our program
 * @void: Takes no parameters
 *
 * Return: TRUE  - if succesful
 *         FALSE - if NOT successful
 */

int main(void)
{
	game_is_running = initialize();

	if (!game_is_running)
	{
		printf("Failed to initialize the game.\n");
		return (FALSE);
	}

	setup();

	/* Game Loop */
	while (game_is_running)
	{
		process_input();
		update();
		render();
	}

	destroy();

	return (TRUE);
}
