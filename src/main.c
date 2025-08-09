#include <stdio.h>
#include <math.h>
#include <time.h>
#include "SDL2/SDL.h"
#include "../headers/structures.h"
#include "../headers/global.h"

/* Keep implementation-only in their own .c files (init.c, textures.c).
   main.c only calls them via prototypes declared in headers/global.h */

int screenBuffer[WINDOW_WIDTH][WINDOW_HEIGHT]; /* legacy / fallback */

void castRays(Instance *instance, Player *player)
{
    for (int x = 0; x < WINDOW_WIDTH; x++) {
        float cameraX = 2 * x / (float)WINDOW_WIDTH - 1;
        float rayDirX = player->dirX + player->planeX * cameraX;
        float rayDirY = player->dirY + player->planeY * cameraX;

        int mapX = (int)player->x;
        int mapY = (int)player->y;

        float sideDistX, sideDistY;
        float deltaDistX = fabsf(1.0f / rayDirX);
        float deltaDistY = fabsf(1.0f / rayDirY);
        float perpWallDist;

        int stepX, stepY;
        int hit = 0, side = 0;

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (player->x - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0f - player->x) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (player->y - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0f - player->y) * deltaDistY;
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

            if (mapY < 0 || mapY >= MAP_HEIGHT || mapX < 0 || mapX >= MAP_WIDTH) {
                hit = 1;
                break;
            }

            if (map[mapY][mapX] > 0) hit = 1;
        }

        if (side == 0)
            perpWallDist = (mapX - player->x + (1 - stepX) / 2.0f) / rayDirX;
        else
            perpWallDist = (mapY - player->y + (1 - stepY) / 2.0f) / rayDirY;

        if (perpWallDist <= 0.0f) perpWallDist = 0.0001f;

        int lineHeight = (int)(WINDOW_HEIGHT / perpWallDist);
        int drawStart = -lineHeight / 2 + WINDOW_HEIGHT / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + WINDOW_HEIGHT / 2;
        if (drawEnd >= WINDOW_HEIGHT) drawEnd = WINDOW_HEIGHT - 1;

        /* textured walls */
        int texW = 0, texH = 0;
        if (instance->wallTexture)
            SDL_QueryTexture(instance->wallTexture, NULL, NULL, &texW, &texH);

        float wallX = 0.0f;
        if (side == 0)
            wallX = player->y + perpWallDist * rayDirY;
        else
            wallX = player->x + perpWallDist * rayDirX;
        wallX -= floorf(wallX);

        int texX = 0;
        if (texW > 0) {
            texX = (int)(wallX * (float)texW);
            if (side == 0 && rayDirX > 0) texX = texW - texX - 1;
            if (side == 1 && rayDirY < 0) texX = texW - texX - 1;
        }

        SDL_Rect dst = { x, drawStart, 1, drawEnd - drawStart + 1 };
        SDL_Rect src = { texX, 0, 1, texH };

        if (instance->wallTexture)
            SDL_RenderCopy(instance->renderer, instance->wallTexture, &src, &dst);
    }
}

void drawMap(Instance *instance, Player *player)
{
    int miniMapSize = WINDOW_WIDTH / 6;
    int miniMapScale = miniMapSize / MAP_WIDTH;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            SDL_Rect r = {
                WINDOW_WIDTH - miniMapSize + x * miniMapScale,
                y * miniMapScale,
                miniMapScale,
                miniMapScale
            };
            if (map[y][x] == 1) {
                SDL_SetRenderDrawColor(instance->renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(instance->renderer, &r);
            } else {
                SDL_SetRenderDrawColor(instance->renderer, 0, 0, 0, 255);
                SDL_RenderFillRect(instance->renderer, &r);
            }
        }
    }

    SDL_Rect pr = {
        WINDOW_WIDTH - miniMapSize + (int)(player->x * miniMapScale) - 2,
        (int)(player->y * miniMapScale) - 2,
        4, 4
    };
    SDL_SetRenderDrawColor(instance->renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(instance->renderer, &pr);
}

void drawScreen(Instance *instance, Player *player)
{
    /* ceiling */
    SDL_SetRenderDrawColor(instance->renderer, 135, 206, 235, 255);
    SDL_Rect ceiling = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT / 2};
    SDL_RenderFillRect(instance->renderer, &ceiling);

    /* floor */
    SDL_SetRenderDrawColor(instance->renderer, 169, 169, 169, 255);
    SDL_Rect floor = {0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT / 2};
    SDL_RenderFillRect(instance->renderer, &floor);

    castRays(instance, player);
    drawMap(instance, player);

    SDL_RenderPresent(instance->renderer);
}

void movePlayer(Player *player, float deltaTime)
{
    const float moveSpeed = deltaTime * 5.0f;
    const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

    float newX = player->x;
    float newY = player->y;

    if (keyboardState[SDL_SCANCODE_UP] || keyboardState[SDL_SCANCODE_W]) {
        newX += player->dirX * moveSpeed;
        newY += player->dirY * moveSpeed;
    }
    if (keyboardState[SDL_SCANCODE_DOWN] || keyboardState[SDL_SCANCODE_S]) {
        newX -= player->dirX * moveSpeed;
        newY -= player->dirY * moveSpeed;
    }

    int mapX = (int)newX;
    int mapY = (int)newY;

    if (mapY >= 0 && mapY < MAP_HEIGHT && mapX >= 0 && mapX < MAP_WIDTH && map[mapY][mapX] == 0) {
        player->x = newX;
        player->y = newY;
    } else {
        if ((int)newY >= 0 && (int)newY < MAP_HEIGHT && map[(int)newY][mapX] == 0) player->y = newY;
        if ((int)newX >= 0 && (int)newX < MAP_WIDTH && map[mapY][(int)newX] == 0) player->x = newX;
    }
}

void rotatePlayer(Player *player, float deltaTime)
{
    const float rotSpeed = deltaTime * 3.0f;
    const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

    if (keyboardState[SDL_SCANCODE_LEFT] || keyboardState[SDL_SCANCODE_A]) {
        float oldDirX = player->dirX;
        player->dirX = player->dirX * cosf(rotSpeed) - player->dirY * sinf(rotSpeed);
        player->dirY = oldDirX * sinf(rotSpeed) + player->dirY * cosf(rotSpeed);
        float oldPlaneX = player->planeX;
        player->planeX = player->planeX * cosf(rotSpeed) - player->planeY * sinf(rotSpeed);
        player->planeY = oldPlaneX * sinf(rotSpeed) + player->planeY * cosf(rotSpeed);
    }

    if (keyboardState[SDL_SCANCODE_RIGHT] || keyboardState[SDL_SCANCODE_D]) {
        float oldDirX = player->dirX;
        player->dirX = player->dirX * cosf(-rotSpeed) - player->dirY * sinf(-rotSpeed);
        player->dirY = oldDirX * sinf(-rotSpeed) + player->dirY * cosf(-rotSpeed);
        float oldPlaneX = player->planeX;
        player->planeX = player->planeX * cosf(-rotSpeed) - player->planeY * sinf(-rotSpeed);
        player->planeY = oldPlaneX * sinf(-rotSpeed) + player->planeY * cosf(-rotSpeed);
    }
}

int main(int argc, char **argv)
{
	(void)argc; (void)argv; /* unused parameters */
    Instance instance;
    Player player;

    if (initializeSDL(&instance) != 0) {
        return 1;
    }

    if (loadTextures(&instance) != 0) {
        cleanupSDL(&instance);
        return 1;
    }

    /* starting player */
    player.x = 3.5f;
    player.y = 3.5f;
    player.dirX = -1.0f;
    player.dirY = 0.0f;
    player.planeX = 0.0f;
    player.planeY = 0.66f;

    SDL_Event event;
    int quit = 0;
    Uint32 last = SDL_GetTicks();

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = 1;
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
        }

        Uint32 now = SDL_GetTicks();
        float deltaTime = (now - last) / 1000.0f;
        if (deltaTime > 0.05f) deltaTime = 0.05f; /* clamp */
        last = now;

        movePlayer(&player, deltaTime);
        rotatePlayer(&player, deltaTime);

        drawScreen(&instance, &player);

        SDL_Delay(1);
    }

    destroyTextures(&instance);
    cleanupSDL(&instance);
    return 0;
}
