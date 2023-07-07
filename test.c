#include <SDL2/SDL.h>
#include <math.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define MAP_WIDTH 16
#define MAP_HEIGHT 16
#define NUM_RAYS WINDOW_WIDTH

void rotatePlayer(float deltaTime);

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

void initializeSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Raycasting Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void cleanupSDL() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

typedef struct {
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
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,1,1,1,0,1,1,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};


int screenBuffer[WINDOW_WIDTH][NUM_RAYS];

void castRays() {
    for (int x = 0; x < WINDOW_WIDTH; x++) {
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

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (player.x - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - player.x) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (player.y - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - player.y) * deltaDistY;
        }

        while (!hit) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
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

        // Choose a wall color based on the side (red for side 1, blue for side 0)
        Uint32 wallColor = (side == 1) ? 0x00008080 : 0x0000FFFF;

        for (int y = 0; y < drawStart; y++)
            screenBuffer[x][y] = 0x000000; // Sky color

        for (int y = drawStart; y <= drawEnd; y++)
            screenBuffer[x][y] = wallColor;

        for (int y = drawEnd + 1; y < WINDOW_HEIGHT; y++)
            screenBuffer[x][y] = 0x808080; // Ground color
    }
}



void movePlayer(float deltaTime) {
    const float moveSpeed = deltaTime * 5.0;

    const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

    // Calculate the new player position based on movement keys
    float newX = player.x;
    float newY = player.y;

    // Move forward
    if (keyboardState[SDL_SCANCODE_UP] || keyboardState[SDL_SCANCODE_W]) {
        newX += player.dirX * moveSpeed;
        newY += player.dirY * moveSpeed;
    }

    // Move backward
    if (keyboardState[SDL_SCANCODE_DOWN] || keyboardState[SDL_SCANCODE_S]) {
        newX -= player.dirX * moveSpeed;
        newY -= player.dirY * moveSpeed;
    }

    // Check if the new position collides with a wall
    int mapX = (int)newX;
    int mapY = (int)newY;

    if (map[mapX][mapY] == 0) {
        player.x = newX;
        player.y = newY;
    } else {
        // Slide along the wall

        // Slide in X direction
        if (map[(int)newX][mapY] == 0)
            player.x = newX;

        // Slide in Y direction
        if (map[mapX][(int)newY] == 0)
            player.y = newY;
    }

    rotatePlayer(deltaTime);
}

void rotatePlayer(float deltaTime){

    const float rotSpeed = deltaTime * 3.0;

    const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
    // Rotate left
    if (keyboardState[SDL_SCANCODE_LEFT] || keyboardState[SDL_SCANCODE_A]) {
        float oldDirX = player.dirX;
        player.dirX = player.dirX * cos(rotSpeed) - player.dirY * sin(rotSpeed);
        player.dirY = oldDirX * sin(rotSpeed) + player.dirY * cos(rotSpeed);
        float oldPlaneX = player.planeX;
        player.planeX = player.planeX * cos(rotSpeed) - player.planeY * sin(rotSpeed);
        player.planeY = oldPlaneX * sin(rotSpeed) + player.planeY * cos(rotSpeed);
    }

    // Rotate right
    if (keyboardState[SDL_SCANCODE_RIGHT] || keyboardState[SDL_SCANCODE_D]) {
        float oldDirX = player.dirX;
        player.dirX = player.dirX * cos(-rotSpeed) - player.dirY * sin(-rotSpeed);
        player.dirY = oldDirX * sin(-rotSpeed) + player.dirY * cos(-rotSpeed);
        float oldPlaneX = player.planeX;
        player.planeX = player.planeX * cos(-rotSpeed) - player.planeY * sin(-rotSpeed);
        player.planeY = oldPlaneX * sin(-rotSpeed) + player.planeY * cos(-rotSpeed);
    }
}

void drawMap() {
    // Draw the mini-map
    int miniMapSize = WINDOW_WIDTH / 8; // Adjust the size of the mini-map
    int miniMapScale = miniMapSize / MAP_WIDTH;
    SDL_Rect miniMapRect = {
        WINDOW_WIDTH - miniMapSize,
        0,
        miniMapSize,
        miniMapSize
    };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &miniMapRect);

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            SDL_Rect miniMapTileRect = {
                WINDOW_WIDTH - miniMapSize + x * miniMapScale,
                y * miniMapScale,
                miniMapScale,
                miniMapScale
            };

            if (map[y][x] == 1) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &miniMapTileRect);
            }
            else if (map[y][x] == 0) {
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

    // Draw a line indicating the direction of movement
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    int lineX = WINDOW_WIDTH - miniMapSize + (int)(player.y * miniMapScale);
    int lineY = (int)(player.x * miniMapScale);
    int lineEndX = lineX + (int)(player.dirY * miniMapScale * 1);
    int lineEndY = lineY + (int)(player.dirX * miniMapScale * 1);
    SDL_RenderDrawLine(renderer, lineX, lineY, lineEndX, lineEndY);

    SDL_RenderPresent(renderer);
}

void drawScreen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    //Draw the 3D scene
    for (int x = 0; x < WINDOW_WIDTH; x++) {
        for (int y = 0; y < WINDOW_HEIGHT; y++) {
            SDL_SetRenderDrawColor(renderer,
                                   (screenBuffer[x][y] & 0xFF0000) >> 16,
                                   (screenBuffer[x][y] & 0x00FF00) >> 8,
                                   screenBuffer[x][y] & 0x0000FF,
                                   255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

    drawMap();
}

int main(int argc, char* args[]) {
    initializeSDL();
    float speed = 0.07;
    
    player.x = 3.5;
    player.y = 3.5;
    player.dirX = -1.0;
    player.dirY = 0.0;
    player.planeX = 0.0;
    player.planeY = 0.66;

    SDL_Event event;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT)
                quit = 1;
	    else if (event.type == SDL_KEYDOWN)
	    {
		    SDL_Keycode keycode = event.key.keysym.sym;
		    if (keycode == SDLK_ESCAPE)
			    quit = 1;
	    }
        }

        drawScreen();
        castRays();
        movePlayer(speed); // Adjust the value here to control player movement speed
        rotatePlayer(speed); // Adjust the value here to control player rotation speed
    }

    cleanupSDL();

    return 0;
}
