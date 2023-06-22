#include "../headers/global.h"

int map[10][10] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 1, 0, 1, 0, 0, 1},
	{1, 0, 0, 0, 1, 0, 1, 0, 0, 1},
	{1, 0, 0, 0, 1, 0, 1, 0, 0, 1},
	{1, 0, 0, 0, 1, 0, 1, 1, 1, 1},
	{1, 0, 0, 0, 1, 0, 1, 0, 0, 1},
	{1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

int tileSizeX = 20; 
int tileSizeY = 20;

int initialize(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("SDL Error! SDL Could Not Initialize: %s\n", SDL_GetError());
		return (FALSE);
	}

	window = SDL_CreateWindow("Maze Project",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			WINDOW_WIDTH,
			WINDOW_HEIGHT,
			0);

	if (window == NULL)
	{
	        printf("SDL Error! Window Could Not Be Created: %s\n", SDL_GetError());
	        SDL_Quit();
	        return (FALSE);
	}	

	Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	renderer = SDL_CreateRenderer(window, -1, render_flags);	

	if (renderer == NULL)
	{
	        printf("SDL Error!: SDL Could Not Create Renderer: %s\n", SDL_GetError());
	        SDL_DestroyWindow(window);
	        SDL_Quit();
	        return (FALSE);
	}

	return (TRUE);
}

void destroy()
{	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void setup()
{
	player.x = 30;
	player.y = 30;
}


void process_input()
{
	SDL_Event event;
	SDL_PollEvent(&event);

	if (event.type == SDL_QUIT)
	        game_is_running = FALSE;
        
	else if (event.type == SDL_KEYDOWN)
        {
		int newPlayerX = player.x;
		int newPlayerY = player.y;

	        switch (event.key.keysym.sym)
                {
	                case SDLK_ESCAPE:
	                        game_is_running = FALSE;
	                        break;
			case SDLK_w:
	                        newPlayerY -= 2;
	                        break;
			case SDLK_s:
	                        newPlayerY += 2;
	                        break;
	                case SDLK_a:
				newPlayerX -= 2;
	                        break;
	                case SDLK_d:
	                        newPlayerX += 2;
	                        break;
	        }

		//Check for collisions with the walls
		if (map[newPlayerY / tileSizeY][newPlayerX / tileSizeX] != 1)
		{
			player.x = newPlayerX;
			player.y = newPlayerY;
		}
        }
}

void drawMap()
{

    	for (int i = 0; i < 10; i++)
    	{
        	for (int j = 0; j < 10; j++)
        	{
            		int cellX = j * tileSizeX;
            		int cellY = i * tileSizeY;

            	SDL_Rect cellRect = {
                	cellX,
                	cellY,
                	tileSizeX,
                	tileSizeY
           	 };

            	if (map[i][j] == 1)
           	 {
                	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                	SDL_RenderFillRect(renderer, &cellRect);
           	 }
            	else if (map[i][j] == 0)
            	{
                	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                	SDL_RenderFillRect(renderer, &cellRect);
        	    }
	
       	 	}
    	}
}

void drawAvatar(SDL_Renderer* renderer, int centerX, int centerY, int radius)
{
	int x = radius;
	int y = 0;
	int radiusError = 1 - x;

	while (x >= y)
	{
		SDL_RenderDrawLine(renderer, centerX + x, centerY + y, centerX - x, centerY + y);
		SDL_RenderDrawLine(renderer, centerX + x, centerY - y, centerX - x, centerY - y);
		SDL_RenderDrawLine(renderer, centerX + y, centerY + x, centerX - y, centerY + x);
		SDL_RenderDrawLine(renderer, centerX + y, centerY - x, centerX - y, centerY - x);
		
		y++;
		if (radiusError < 0)
			radiusError += 2 * y + 1;
		else
		{
			x--;
			radiusError += 2 * (y - x) + 1;
		}

	}
}

void drawRay(SDL_Renderer *renderer, int startX, int startY, int endX, int endY)
{
	startX = player.x;
	startY = player.y;

	endX = FALSE;
	endY = FALSE;


}

void render()
{
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawMap();

     	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	int centerX = player.x;
	int centerY = player.y;
	int radius = 2;

	drawAvatar(renderer, centerX, centerY, radius);

        SDL_RenderPresent(renderer);
}

void update()
{
        //TODO: Updating the game
}


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

