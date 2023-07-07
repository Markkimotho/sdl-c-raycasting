#include <SDL2/SDL.h>
#include <stdio.h>
#include "../headers/structures.h"
#include "../headers/global.h"

/**
 * initializeSDL - Initializes SDL; Starts up a window, & renderer
 *
 * @instance: SDL instance
 *
 * Return: (0) on success & (1) on failure
 */

int initializeSDL(Instance *instance)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL ERROR! SDL could not initilize: %s\n", SDL_GetError());
		return (FAIL);
	}

	instance->window = SDL_CreateWindow("Maze",
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				WINDOW_WIDTH,
				WINDOW_HEIGHT,
				0);
	if (instance->window == NULL)
	{
		printf("SDL ERROR! SDL could not create window: %s\n", SDL_GetError());
		SDL_Quit();
		return (FAIL);
	}

	Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

	instance->renderer = SDL_CreateRenderer(instance->window, -1, render_flags);

	if (instance->renderer == NULL)
	{
		printf("SDL ERROR! SDL could not create renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(instance->window);
		SDL_Quit();
		return (FAIL);
	}

	return (SUCCESS);
}

/**
 * cleanupSDL - It cleans up resources befores exiting
 *
 * @instance: SDL instance
 *
 * Return: Nothing
 */

void cleanupSDL(Instance *instance)
{
	SDL_DestroyRenderer(instance->renderer);
	SDL_DestroyWindow(instance->window);
	SDL_Quit();
}
