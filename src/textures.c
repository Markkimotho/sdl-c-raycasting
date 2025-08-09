#include "headers/structures.h"

void loadTextures()
{
	SDL_Surface* wallSurface = IMG_Load("colorstone.png");
	if (!wallSurface)
	{
		printf("Failed to load texture image\n");
		return FAIL;
	}

	wallTexture = SDL_CreateTextureFromSurface(renderer, wallSurface);
	SDL_FreeSurface(wallSurface);
}

void destroyTextures(Instance *instance)
{
	if (instance->wallTexture)
	{
		SDL_DestroyTexture(instance->wallTexture);
		instance->wallTexture = NULL;
	}
}
