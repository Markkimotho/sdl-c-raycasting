#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <time.h>

#define WINDOW_WIDTH 1480
#define WINDOW_HEIGHT 820
#define MAP_WIDTH 16
#define MAP_HEIGHT 16
#define NUM_RAYS WINDOW_WIDTH

void initializeSDL();
void cleanupSDL();
void loadTextures();
void destroyTextures();
void castRays();
void drawMap();
void drawScreen();
void movePlayer(float deltaTime);
void rotatePlayer(float deltaTime);

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* wallTexture = NULL;

/**
 * initializeSDL - initializes SDL window and renderer
 *
 * Return: None
 *
 */
void initializeSDL()
{
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
                printf("SDL ERROR! SDL could not initilize: %s\n", SDL_GetError());
                return;
        }
        window = SDL_CreateWindow("Raycasting Engine",
                        SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED,
                        WINDOW_WIDTH,
                        WINDOW_HEIGHT,
                        0);

        if (window == NULL)
        {
                printf("SDL ERROR! SDL could not create window: %s\n", SDL_GetError());
                SDL_Quit();
                return;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == NULL)
        {
                printf("SDL ERROR! SDL could not create renderer: %s\n", SDL_GetError());
                SDL_DestroyWindow(window);
                SDL_Quit();
                return;
        }
}

/**
 * cleanupSDL - frees up resources from SDL
 *
 * Return: None
 *
 */
void cleanupSDL()
{
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
}

/**
 * Player - A structure that defines a player's position on the maze
 * @x: X-position on the maze
 * @y: Y-position on the maze
 * @dirX: The X-direction the player is facing
 * @dirY: The Y-direction the player is facing
 * @planeX: The X-plane the player is facing
 * @planeY: The Y-plane the player is facing
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

Player player;

int map[MAP_WIDTH][MAP_HEIGHT] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,1,1,1,1,1,1,1,1,0,0,1},
        {1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
        {1,0,0,0,1,1,1,0,1,1,1,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

/**
 * loadTextures - Loads the texture that we are rendering
 * 
 * Return: None
 *
 */
void loadTextures()
{
        SDL_Surface* surface = IMG_Load("assets/colorstone.png");
        if (!surface) {
                printf("Failed to load texture image: redbrick.png\n");
                return;
        }
        wallTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

}

/**
 * destroyTextures - Cleans up the texture resources
 *
 * Return: None
 */
void destroyTextures()
{
        if (wallTexture)
        {
                SDL_DestroyTexture(wallTexture);
                wallTexture = NULL;
        }

}

/**
 * castRays - Performs the raycasting and rendering of the 3D scene
 * 
 * Return: None
 *
 */
void castRays()
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
                } else
                {
                        stepX = 1;
                        sideDistX = (mapX + 1.0 - player.x) * deltaDistX;
                }

                if (rayDirY < 0)
                {
                        stepY = -1;
                        sideDistY = (player.y - mapY) * deltaDistY;
                } else
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
                        } else
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

                // Calculate the texture coordinates
                int texWidth, texHeight;
                SDL_QueryTexture(wallTexture, NULL, NULL, &texWidth, &texHeight);
                float wallX;
                if (side == 0)
                        wallX = player.y + perpWallDist * rayDirY;
                else
                        wallX = player.x + perpWallDist * rayDirX;
                wallX -= floor(wallX);

                // Calculate the texture column to draw
                int texColumn = (int)(wallX * texWidth);
                if ((side == 0 && rayDirX > 0) || (side == 1 && rayDirY < 0))
                        texColumn = texWidth - texColumn - 1;

                // Draw the wall
                SDL_Rect wallRect = {
                        x,
                        drawStart,
                        1,
                        drawEnd - drawStart + 1
                };
                SDL_Rect texRect = {
                        texColumn,
                        0,
                        1,
                        texHeight
                };

                SDL_RenderCopy(renderer, wallTexture, &texRect, &wallRect);
        }

}

/*
 * movePlayer - Performs forward and backward movement of the player
 * 
 * @deltaTime: Speed of player movement
 * 
 * Return: None
 *
 */
void movePlayer(float deltaTime)
{
        const float moveSpeed = deltaTime * 5.0;

        const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

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
        } else
        {
                if (map[(int)newX][mapY] == 0)
                        player.x = newX;

                if (map[mapX][(int)newY] == 0)
                        player.y = newY;
        }

        rotatePlayer(deltaTime);
}


/**
 * rotatePlayer - Performs left or right rotation
 *
 * @deltaTime: Speed of player rotation
 *
 * Return: None
 *
 */
void rotatePlayer(float deltaTime)
{
        const float rotSpeed = deltaTime * 3.0;

        const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

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
 * drawMap - Draws the 2D map on the screen
 * 
 * Return: None
 *
 */
void drawMap()
{
        int miniMapSize = WINDOW_WIDTH / 6;
        int miniMapScale = miniMapSize / MAP_WIDTH;
        SDL_Rect miniMapRect = {
                WINDOW_WIDTH - miniMapSize,
                0,
                miniMapSize,
                miniMapSize
        };

        for (int y = 0; y < MAP_HEIGHT; y++)
        {
                for (int x = 0; x < MAP_WIDTH; x++)
                {
                        SDL_Rect miniMapTileRect = {
                                WINDOW_WIDTH - miniMapSize + x * miniMapScale,
                                y * miniMapScale,
                                miniMapScale,
                                miniMapScale
                        };

                        if (map[y][x] == 1)
                        {
                                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                                SDL_RenderFillRect(renderer, &miniMapTileRect);
                        } else if (map[y][x] == 0)
                        {
                                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                SDL_RenderFillRect(renderer, &miniMapTileRect);
                        }
                }
        }

        SDL_Rect playerRect = {
                WINDOW_WIDTH - miniMapSize + (int)(player.y * miniMapScale) - 2,
                (int)(player.x * miniMapScale) - 2,
                4,
                4
        };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &playerRect);

        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        int lineX = WINDOW_WIDTH - miniMapSize + (int)(player.y * miniMapScale);
        int lineY = (int)(player.x * miniMapScale);
        int lineEndX = lineX + (int)(player.dirY * miniMapScale * 1);
        int lineEndY = lineY + (int)(player.dirX * miniMapScale * 1);
        SDL_RenderDrawLine(renderer, lineX, lineY, lineEndX, lineEndY);

        SDL_RenderPresent(renderer);
}

/**
 * drawScreen - For rendering objects of the screen
 *
 * Return: None 
 *
 */
void drawScreen()
{
        // Draw sky (ceiling)
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255); // Set the sky color to sky blue (RGB: 135, 206, 235)
        SDL_Rect ceilingRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT / 2};
        SDL_RenderFillRect(renderer, &ceilingRect);

        // Draw floor
        SDL_SetRenderDrawColor(renderer, 169, 169, 169, 255); // Set the floor color to gray (RGB: 169, 169, 169)
        SDL_Rect floorRect = {0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT / 2};
        SDL_RenderFillRect(renderer, &floorRect);

        castRays();
        drawMap();

        SDL_RenderPresent(renderer);
}


/**
 * main - Entry point to our program
 * 
 * @argc: The number of CLI arguments 
 * @argv: An array containing the CLI arguments
 *
 * Return: 0 if succesful; 1 if not
 *
 */
int main(int argc, char* args[])
{
        initializeSDL();

        player.x = 3.5;
        player.y = 3.5;
        player.dirX = -1.0;
        player.dirY = 0.0;
        player.planeX = 0.0;
        player.planeY = 0.66;

        loadTextures();

        SDL_Event event;
        int quit = 0;
        while (!quit)
        {
                while (SDL_PollEvent(&event) != 0)
                {
                        if (event.type == SDL_QUIT)
                                quit = 1;
                        else if (event.type == SDL_KEYDOWN)
                        {
                                SDL_Keycode keycode = event.key.keysym.sym;
                                if (keycode == SDLK_ESCAPE)
                                        quit = 1;
                        }
                }

                float deltaTime = 0.016;  // Adjust the value here to control player movement speed
                movePlayer(deltaTime);
                drawScreen();
        }

        destroyTextures();
        cleanupSDL();

        return 0;
}


