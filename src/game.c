#include "../headers/global.h"


/**
 * drawMap - draws the 2D map on the screen
 * @void: Takes no parameter
 *
 * Return: None

 */

void drawMap(Instance *instance, int map[MAP_WIDTH][MAP_HEIGHT])
{
	/****** Draw the map ********/
	int mapSize = WINDOW_WIDTH / 8;
	int mapScale = mapSize / MAP_WIDTH;

	SDL_Rect mapRect = {
		WINDOW_WIDTH - mapSize,
		0,
		mapSize,
		mapSize
	};

	SDL_SetRenderDrawColor(instance->renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(instance->renderer, &mapRect);

	for (int x = 0; x < MAP_WIDTH; x++)
	{
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			SDL_Rect mapTileRect = {
				WINDOW_WIDTH - mapSize + x * mapScale,
				y * mapScale,
				mapScale,
				mapScale
			};
			if (map[x][y] == 1)
			{
				SDL_SetRenderDrawColor(instance->renderer, 255, 255, 255, 255);
				SDL_RenderFillRect(instance->renderer, &mapTileRect);
			}
			else if (map[x][y] == 0)
			{

				SDL_SetRenderDrawColor(instance->renderer, 0, 0, 0, 255);
				SDL_RenderFillRect(instance->renderer, &mapTileRect);
			}
		}
	}
}


/**
 * drawScreen - draws the 3D scene
 * @void: Takes no parameter
 *
 * Return: None
 */

void drawScreen(Instance *instance, int map[MAP_WIDTH][MAP_HEIGHT])
{
	SDL_SetRenderDrawColor(instance->renderer, 0, 0, 0, 255);
	SDL_RenderClear(instance->renderer);

	/***** Draw the 3D scene ******/

	for (int x = 0; x < WINDOW_WIDTH; x++)
	{
		for (int y = 0; y < WINDOW_HEIGHT; y++)
		{
			SDL_SetRenderDrawColor(renderer,
						(screenBuffer[x][y] & 0XFF0000) >> 16,
						(screenBuffer[x][y] & 0X00FF00) >> 8,
						screenBuffer[x][y] & 0X0000FF,
						255);
			SDL_RenderDrawPoint(renderer, x, y);
		}
	}
	drawMap(instance, map);
}



/**
 * movePlayer - Moving the player forwards or backwards
 * @deltaTime: The speed the player will move at
 *
 * Return: None
 *
 */

void movePlayer(Player player, float deltaTime)
{
	const float moveSpeed = deltaTime * 5.0;
	const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

	float newX = player.x;
	float newY = player.y;

	if (keyboardState[SDL_SCANCODE_UP] || keyboardState[SDL_SCANCODE_W])
	{
		newX += player.dirX * moveSpeed;
		newY += player.dirY * moveSpeed;
	}

	if (keyboardState[SDL_SCANCODE_DOWN] || keyboardState[SDL_SCANCODE_S])
	{
		newX -= player.dirX * moveSpeed;
		newY -= player.dirY * moveSpeed;
	}

	int mapX = (int)newX;
	int mapY = (int)newY;

	if (map[mapX][mapY] == 0)
	{
		player.x = newX;
		player.y = newY;
	}
	else
	{
		if (map[(int)newX][mapY] == 0)
			player.x = newX;
		if (map[mapX][(int)newY] == 0)
			player.y = newY;
	}
}

/**
 * rotatePlayer - Rotates the player Westwards or Eastwards
 * @void: Takes no parameter
 *
 * Return: None
 */

void rotatePlayer(Player player)
{

	const float rotSpeed = deltaTime * 3.0;
	const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

	if (keyboardState[SDL_SCANCODE_LEFT] || keyboardState[SDL_SCANCODE_A])
	{
		float oldDirX = player.dirX;

		player.dirX = player.dirX * cos(rotSpeed) - player.dirY * sin(rotSpeed);
		player.dirY = oldDirX * sin(rotSpeed) + player.dirY * cos(rotSpeed);

		float oldPlaneX = player.planeX;

		player.planeX = player.planeX * cos(rotSpeed) - player.planeY * sin(rotSpeed);
		player.planeY = oldPlaneX * sin(rotSpeed) + player.planeY * cos(rotSpeed);
	}

	if (keyboardState[SDL_SCANCODE_RIGHT] || keyboardState[SDL_SCANCODE_D])
	{
		float oldDirX = player.dirX;

		player.dirX = player.dirX * cos(-rotSpeed) - player.dirY * sin(-rotSpeed);
		player.dirY = oldDirX * sin(-rotSpeed) + player.dirY * cos(-rotSpeed);

		float oldPlaneX = player.planeX;

		player.planeX = player.planeX * cos(-rotSpeed) - player.planeY * sin(-rotSpeed);
		player.planeY = oldPlaneX * sin(-rotSpeed) + player.planeY * cos(-rotSpeed);

	}
}

/**
 * castRays - Casts the rays that will render the 3D display
 * @void: Takes no parameter
 *
 * Return: None
 */

void castRays(Player player, int map[MAP_WIDTH][MAP_HEIGHT])
{
	for (int x = 0; x < WINDOW_WIDTH; x++)
	{
		float cameraX = 2 * x / (float)WINDOW_WIDTH - 1;
		float rayDirX = player.dirX + player.planeX * cameraX;
		float rayDirY = player.dirY + player.planeY * cameraX;

		int mapX = (int)player.x;
		int mapY = (int)player.y;

		float sideDistX;
		float sideDistY;

		float deltaDistX = fabs(1 / rayDirX);
		float deltaDistY = fabs(1 / rayDirY);
		float perpWallDist;

		int stepX;
		int stepY;

		int hit = 0;
		int side;

		if (rayDirX < 0)
		{
			stepX = -1;
			sideDistX = (player.x - mapX) * deltaDistX;
		}
		else
		{
			stepX = 1;
			sideDistX = (mapX + 1.0 - player.x) * deltaDistX;
		}

		if (rayDirY < 0)
		{
			stepY = -1;
			sideDistY = (player.y - mapY) * deltaDistY;
		}
		else
		{
			stepY = 1;
			sideDistY = (mapY + 1.0 - player.y) * deltaDistY;
		}

		while (!hit)
		{
			if (sideDistX < sideDistY)
			{
				sideDistX += deltaDistX;
				mapX += stepX;
				side = 0;
			}
			else
			{
				sideDistY += deltaDistY;
				mapY += stepY;
				side = 1;
			}

			if (map[mapX][mapY] > 0)
				hit = 1;
		}

		if (side == 0)
			perpWallDist = (mapX - player.x + (1 - stepX) / 2) / rayDirX;
		else
			perpWallDist = (mapY - player.y + (1 - stepY) / 2) / rayDirY;

		int lineHeight = (int)(WINDOW_HEIGHT / perpWallDist);

		int drawStart = -lineHeight / 2 + WINDOW_HEIGHT / 2;

		if (drawStart < 0)
			drawStart = 0;

		int drawEnd = lineHeight / 2 + WINDOW_HEIGHT / 2;

		if (drawEnd >= WINDOW_HEIGHT)
			drawEnd = WINDOW_HEIGHT - 1;

		Uint32 wallColor = (side == 1) ? 0xFF0000FF : 0x0000FFFF;

		for (int y = 0; y < drawStart; y++)
		screenBuffer[x][y] = 0x000000;

		for (int y = drawStart; y <= drawEnd; y++)
			screenBuffer[x][y] = wallColor;

		for (int y = drawEnd + 1; y < WINDOW_HEIGHT; y++)
			screenBuffer[x][y] = 0x808080;
	}

}
