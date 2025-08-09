#include <stdio.h>
#include <math.h>
#include <time.h>
#include "SDL2/SDL.h"
#include "../headers/structures.h"
#include "../headers/global.h"

#define MOVE_SPEED 5.0f
#define ROT_SPEED 3.0f

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

            if (map[mapY][mapX] > 0) hit = 1; // wallType stored in map
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

        int texW = 0, texH = 0;
        int wallType = map[mapY][mapX];
        SDL_Texture *tex = NULL;

        if (wallType > 0 && wallType < 6)  // since 6 textures total, 0 unused
            tex = instance->wallTextures[wallType];

        if (tex)
            SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);

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

        if (tex)
            SDL_RenderCopy(instance->renderer, tex, &src, &dst);
    }
}

void drawSprites(Instance *instance, Player *player)
{
    // Sort sprites back to front (simple bubble sort)
    for (int i = 0; i < instance->numSprites - 1; i++) {
        for (int j = i + 1; j < instance->numSprites; j++) {
            float di = (player->x - instance->sprites[i].x)*(player->x - instance->sprites[i].x) +
                       (player->y - instance->sprites[i].y)*(player->y - instance->sprites[i].y);
            float dj = (player->x - instance->sprites[j].x)*(player->x - instance->sprites[j].x) +
                       (player->y - instance->sprites[j].y)*(player->y - instance->sprites[j].y);
            if (dj > di) {
                Sprite tmp = instance->sprites[i];
                instance->sprites[i] = instance->sprites[j];
                instance->sprites[j] = tmp;
            }
        }
    }

    for (int i = 0; i < instance->numSprites; i++) {
        float spriteX = instance->sprites[i].x - player->x;
        float spriteY = instance->sprites[i].y - player->y;

        float invDet = 1.0f / (player->planeX * player->dirY - player->dirX * player->planeY);
        float transformX = invDet * (player->dirY * spriteX - player->dirX * spriteY);
        float transformY = invDet * (-player->planeY * spriteX + player->planeX * spriteY);

        if (transformY <= 0) continue; // sprite behind player

        int spriteScreenX = (int)((WINDOW_WIDTH / 2) * (1 + transformX / transformY));
        int spriteHeight = abs((int)(WINDOW_HEIGHT / transformY));
        int drawStartY = -spriteHeight / 2 + WINDOW_HEIGHT / 2;
        if (drawStartY < 0) drawStartY = 0;
        int drawEndY = spriteHeight / 2 + WINDOW_HEIGHT / 2;
        if (drawEndY >= WINDOW_HEIGHT) drawEndY = WINDOW_HEIGHT - 1;

        int spriteWidth = abs((int)(WINDOW_HEIGHT / transformY));
        int drawStartX = -spriteWidth / 2 + spriteScreenX;
        if (drawStartX < 0) drawStartX = 0;
        int drawEndX = spriteWidth / 2 + spriteScreenX;
        if (drawEndX >= WINDOW_WIDTH) drawEndX = WINDOW_WIDTH - 1;

        SDL_Rect dst = { drawStartX, drawStartY, spriteWidth, spriteHeight };
        SDL_RenderCopy(instance->renderer, instance->sprites[i].texture, NULL, &dst);
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
            if (map[y][x] > 0) {
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
    // Clear with ceiling color
    SDL_SetRenderDrawColor(instance->renderer, 135, 206, 235, 255);
    SDL_Rect ceiling = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT / 2};
    SDL_RenderFillRect(instance->renderer, &ceiling);

    // Draw floor color
    SDL_SetRenderDrawColor(instance->renderer, 169, 169, 169, 255);
    SDL_Rect floor = {0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT / 2};
    SDL_RenderFillRect(instance->renderer, &floor);

    castRays(instance, player);
    drawSprites(instance, player);
    drawMap(instance, player);

    SDL_RenderPresent(instance->renderer);
}

void movePlayer(Player *player, const Uint8 *keystate, float deltaTime)
{
    const float moveSpeed = deltaTime * MOVE_SPEED;

    float newX = player->x;
    float newY = player->y;

    if (keystate[SDL_SCANCODE_W]) {
        newX += player->dirX * moveSpeed;
        newY += player->dirY * moveSpeed;
    }
    if (keystate[SDL_SCANCODE_S]) {
        newX -= player->dirX * moveSpeed;
        newY -= player->dirY * moveSpeed;
    }
    if (keystate[SDL_SCANCODE_A]) {
        newX -= player->planeX * moveSpeed;
        newY -= player->planeY * moveSpeed;
    }
    if (keystate[SDL_SCANCODE_D]) {
        newX += player->planeX * moveSpeed;
        newY += player->planeY * moveSpeed;
    }

    if (map[(int)newY][(int)newX] == 0) {
        player->x = newX;
        player->y = newY;
    }
}

void rotatePlayer(Player *player, const Uint8 *keystate, float deltaTime)
{
    const float rotSpeed = deltaTime * ROT_SPEED;

    if (keystate[SDL_SCANCODE_LEFT]) {
        float oldDirX = player->dirX;
        player->dirX = player->dirX * cosf(rotSpeed) - player->dirY * sinf(rotSpeed);
        player->dirY = oldDirX * sinf(rotSpeed) + player->dirY * cosf(rotSpeed);

        float oldPlaneX = player->planeX;
        player->planeX = player->planeX * cosf(rotSpeed) - player->planeY * sinf(rotSpeed);
        player->planeY = oldPlaneX * sinf(rotSpeed) + player->planeY * cosf(rotSpeed);
    }

    if (keystate[SDL_SCANCODE_RIGHT]) {
        float oldDirX = player->dirX;
        player->dirX = player->dirX * cosf(-rotSpeed) - player->dirY * sinf(-rotSpeed);
        player->dirY = oldDirX * sinf(-rotSpeed) + player->dirY * cosf(-rotSpeed);

        float oldPlaneX = player->planeX;
        player->planeX = player->planeX * cosf(-rotSpeed) - player->planeY * sinf(-rotSpeed);
        player->planeY = oldPlaneX * sinf(-rotSpeed) + player->planeY * cosf(-rotSpeed);
    }
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    Instance instance = {0}; // zero-init
    Player player;

    if (initializeSDL(&instance) != 0) return 1;

    if (loadTextures(&instance) != 0) {
        cleanupSDL(&instance);
        return 1;
    }

    // if (loadSprites(&instance) != 0) {
    //     fprintf(stderr, "Failed to load sprites\n");
    //     destroyTextures(&instance);
    //     cleanupSDL(&instance);
    //     return 1;
    // }

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
        if (deltaTime > 0.05f) deltaTime = 0.05f;
        last = now;

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        movePlayer(&player, keystate, deltaTime);
        rotatePlayer(&player, keystate, deltaTime);

        drawScreen(&instance, &player);

        SDL_Delay(1);
    }

    destroyTextures(&instance);
    cleanupSDL(&instance);
    return 0;
}