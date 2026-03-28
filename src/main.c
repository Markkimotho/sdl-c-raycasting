#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "SDL2/SDL.h"
#include "../headers/structures.h"
#include "../headers/global.h"

#define MOVE_SPEED 5.0f
#define ROT_SPEED 3.0f

/* Global mutable map (copied from selected preset) */
int map[MAP_HEIGHT][MAP_WIDTH];

int screenBuffer[WINDOW_WIDTH][WINDOW_HEIGHT]; /* legacy / fallback */

/* ===== Robustness Helper Functions ===== */

/**
 * Check if a position is within map bounds and walkable
 */
static int isWalkablePosition(float x, float y) {
    int mapX = (int)x;
    int mapY = (int)y;
    
    if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
        return 0;
    }
    return map[mapY][mapX] == 0;
}

/**
 * Check if a position is safe (account for collision buffer)
 */
static int isSafePosition(float x, float y) {
    /* Check center and diagonal corners for more flexible but safe collision detection */
    return isWalkablePosition(x, y) && 
           isWalkablePosition(x + PLAYER_COLLISION_BUFFER, y + PLAYER_COLLISION_BUFFER) &&
           isWalkablePosition(x - PLAYER_COLLISION_BUFFER, y - PLAYER_COLLISION_BUFFER);
}

/**
 * Copy map from preset into mutable global map
 */
static void loadMapPreset(int mapIndex) {
    if (mapIndex < 0 || mapIndex >= NUM_MAPS) {
        fprintf(stderr, "Error: Invalid map index %d\n", mapIndex);
        return;
    }
    memcpy(map, mapOptions[mapIndex].layout, sizeof(map));
}

/**
 * Find a valid spawn position near the desired location
 */
static int findValidSpawn(float *outX, float *outY, float desiredX, float desiredY) {
    if (isSafePosition(desiredX, desiredY)) {
        *outX = desiredX;
        *outY = desiredY;
        return 1;
    }
    
    /* Try nearby positions in expanding circles */
    float searchRadius = 0.5f;
    float step = 0.2f;
    int maxAttempts = 50;
    
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        for (float angle = 0; angle < 6.28f; angle += 0.785f) {  /* ~8 directions */
            float testX = desiredX + searchRadius * cosf(angle);
            float testY = desiredY + searchRadius * sinf(angle);
            
            if (isSafePosition(testX, testY)) {
                *outX = testX;
                *outY = testY;
                fprintf(stderr, "Found valid spawn at (%.2f, %.2f)\n", testX, testY);
                return 1;
            }
        }
        searchRadius += step;
    }
    
    return 0;  /* No valid spawn found */
}

/**
 * Generate sprites procedurally on empty floor tiles
 */
static void generateSprites(Instance *instance) {
    if (!instance) return;
    
    srand((unsigned)time(NULL));
    instance->numSprites = 0;
    
    int spriteCount = 8 + (rand() % 5);  /* 8-12 sprites */
    if (spriteCount > MAX_SPRITE_COUNT) spriteCount = MAX_SPRITE_COUNT;
    
    int attempts = 0;
    int maxAttempts = 200;
    
    while (instance->numSprites < spriteCount && attempts < maxAttempts) {
        attempts++;
        float rx = 1.5f + (rand() % (MAP_WIDTH - 3));
        float ry = 1.5f + (rand() % (MAP_HEIGHT - 3));
        
        if (isWalkablePosition(rx, ry)) {
            instance->sprites[instance->numSprites].x = rx;
            instance->sprites[instance->numSprites].y = ry;
            /* Assign a random texture (simplified - won't load, but structure is valid) */
            instance->sprites[instance->numSprites].texture = NULL;
            instance->numSprites++;
        }
    }
    
    fprintf(stderr, "Generated %d sprites\n", instance->numSprites);
}

/**
 * Safely check wall type at map position with bounds validation
 */
static int getWallType(int mapX, int mapY) {
    if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
        return 1;  // Treat out-of-bounds as wall
    }
    return map[mapY][mapX];
}

void castRays(Instance *instance, Player *player)
{
    if (!instance || !player) {
        fprintf(stderr, "Error: castRays called with NULL pointer\n");
        return;
    }

    for (int x = 0; x < WINDOW_WIDTH; x++) {
        float cameraX = 2 * x / (float)WINDOW_WIDTH - 1;
        float rayDirX = player->dirX + player->planeX * cameraX;
        float rayDirY = player->dirY + player->planeY * cameraX;

        /* Prevent division by zero */
        if (fabsf(rayDirX) < MIN_DISTANCE && fabsf(rayDirY) < MIN_DISTANCE) {
            continue;
        }

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

        /* Walk the ray through the map to find wall */
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

            /* Check bounds and hit */
            if (mapY < 0 || mapY >= MAP_HEIGHT || mapX < 0 || mapX >= MAP_WIDTH) {
                hit = 1;
                break;
            }

            if (map[mapY][mapX] > 0) hit = 1;
        }

        /* Calculate perpendicular distance */
        if (side == 0)
            perpWallDist = (mapX - player->x + (1 - stepX) / 2.0f) / rayDirX;
        else
            perpWallDist = (mapY - player->y + (1 - stepY) / 2.0f) / rayDirY;

        if (perpWallDist <= MIN_DISTANCE) perpWallDist = MIN_DISTANCE;

        int lineHeight = (int)(WINDOW_HEIGHT / perpWallDist);
        int drawStart = -lineHeight / 2 + WINDOW_HEIGHT / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + WINDOW_HEIGHT / 2;
        if (drawEnd >= WINDOW_HEIGHT) drawEnd = WINDOW_HEIGHT - 1;

        /* Validate and fetch texture based on wall type */
        int texW = 0, texH = 0;
        int wallType = 0;
        if (mapY >= 0 && mapY < MAP_HEIGHT && mapX >= 0 && mapX < MAP_WIDTH)
            wallType = map[mapY][mapX];
        SDL_Texture *tex = NULL;
        if (wallType > 0 && wallType < MAX_WALL_TYPES)
            tex = instance->wallTextures[wallType];

        if (tex) {
            SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);
        }

        float wallX = 0.0f;
        if (side == 0)
            wallX = player->y + perpWallDist * rayDirY;
        else
            wallX = player->x + perpWallDist * rayDirX;
        wallX -= floorf(wallX);

        int texX = 0;
        if (texW > 0) {
            texX = (int)(wallX * (float)texW);
            if (texX < 0) texX = 0;
            if (texX >= texW) texX = texW - 1;
            
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
    if (!instance || !player || instance->numSprites <= 0) {
        return;
    }

    if (instance->numSprites > MAX_SPRITE_COUNT) {
        fprintf(stderr, "Warning: sprite count exceeds maximum (%d > %d)\n",
                instance->numSprites, MAX_SPRITE_COUNT);
        instance->numSprites = MAX_SPRITE_COUNT;
    }

    /* Sort sprites back to front (simple bubble sort) */
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

    /* Render sprites */
    for (int i = 0; i < instance->numSprites; i++) {
        if (!instance->sprites[i].texture) {
            continue;
        }

        float spriteX = instance->sprites[i].x - player->x;
        float spriteY = instance->sprites[i].y - player->y;

        float detInv = player->planeX * player->dirY - player->dirX * player->planeY;
        if (fabsf(detInv) < MIN_DISTANCE) {
            continue;  /* Degenerate camera plane */
        }
        
        float invDet = 1.0f / detInv;
        float transformX = invDet * (player->dirY * spriteX - player->dirX * spriteY);
        float transformY = invDet * (-player->planeY * spriteX + player->planeX * spriteY);

        if (transformY <= MIN_DISTANCE) continue; /* Sprite behind player or too close */

        int spriteScreenX = (int)((WINDOW_WIDTH / 2.0f) * (1.0f + transformX / transformY));
        int spriteHeight = (int)fabs(WINDOW_HEIGHT / transformY);
        
        /* Clamp height to prevent overflow */
        if (spriteHeight > WINDOW_HEIGHT * 2) spriteHeight = WINDOW_HEIGHT * 2;
        
        int drawStartY = -spriteHeight / 2 + WINDOW_HEIGHT / 2;
        if (drawStartY < 0) drawStartY = 0;
        int drawEndY = spriteHeight / 2 + WINDOW_HEIGHT / 2;
        if (drawEndY >= WINDOW_HEIGHT) drawEndY = WINDOW_HEIGHT - 1;

        int spriteWidth = (int)fabs(WINDOW_HEIGHT / transformY);
        if (spriteWidth > WINDOW_WIDTH * 2) spriteWidth = WINDOW_WIDTH * 2;
        
        int drawStartX = -spriteWidth / 2 + spriteScreenX;
        if (drawStartX < 0) drawStartX = 0;
        int drawEndX = spriteWidth / 2 + spriteScreenX;
        if (drawEndX >= WINDOW_WIDTH) drawEndX = WINDOW_WIDTH - 1;

        if (drawStartX >= WINDOW_WIDTH || drawEndX < 0 ||
            drawStartY >= WINDOW_HEIGHT || drawEndY < 0) {
            continue;  /* Sprite off-screen */
        }

        SDL_Rect dst = { drawStartX, drawStartY, 
                        drawEndX - drawStartX, drawEndY - drawStartY };
        
        if (dst.w > 0 && dst.h > 0) {
            SDL_RenderCopy(instance->renderer, instance->sprites[i].texture, NULL, &dst);
        }
    }
}

/* ===== Sky Rendering Functions ===== */

/**
 * Draw a realistic day sky with natural gradient
 */
void drawDaySky(Instance *instance)
{
    if (!instance || !instance->renderer) return;
    
    int height = WINDOW_HEIGHT / 2;
    
    /* Realistic sky gradient: deep blue at zenith fading to warm white at horizon */
    for (int y = 0; y < height; y++) {
        float ratio = (float)y / height;
        /* Use smooth cubic easing for natural light falloff */
        float eased = ratio * ratio;
        
        /* Zenith: deep sky blue (60, 130, 210) -> Horizon: warm haze (200, 210, 220) */
        Uint8 r = (Uint8)(60  + 140 * eased);
        Uint8 g = (Uint8)(130 +  80 * eased);
        Uint8 b = (Uint8)(210 +  10 * eased);
        
        SDL_SetRenderDrawColor(instance->renderer, r, g, b, 255);
        SDL_RenderDrawLine(instance->renderer, 0, y, WINDOW_WIDTH, y);
    }
}

/**
 * Draw a starry night sky with natural gradient
 */
void drawNightSky(Instance *instance)
{
    if (!instance || !instance->renderer) return;
    
    int height = WINDOW_HEIGHT / 2;
    
    /* Night sky gradient: dark at zenith to slightly lighter at horizon */
    for (int y = 0; y < height; y++) {
        float ratio = (float)y / height;
        float eased = ratio * ratio;
        
        /* Zenith: near black (5, 8, 25) -> Horizon: dark blue-gray (25, 30, 55) */
        Uint8 r = (Uint8)(5  + 20 * eased);
        Uint8 g = (Uint8)(8  + 22 * eased);
        Uint8 b = (Uint8)(25 + 30 * eased);
        
        SDL_SetRenderDrawColor(instance->renderer, r, g, b, 255);
        SDL_RenderDrawLine(instance->renderer, 0, y, WINDOW_WIDTH, y);
    }
    
    /* Draw stars */
    srand(42);  /* Deterministic stars */
    int numStars = 200;
    for (int i = 0; i < numStars; i++) {
        int starX = rand() % WINDOW_WIDTH;
        int starY = rand() % (height - 20);  /* Stars across full sky except near horizon */
        int brightness = 120 + (rand() % 135);
        
        /* Dimmer stars near horizon for realism */
        float heightRatio = (float)starY / height;
        brightness = (int)(brightness * (1.0f - heightRatio * 0.5f));
        
        SDL_SetRenderDrawColor(instance->renderer, brightness, brightness, brightness, 255);
        SDL_RenderDrawPoint(instance->renderer, starX, starY);
        
        /* Occasional brighter/larger stars */
        if (rand() % 8 == 0) {
            SDL_RenderDrawPoint(instance->renderer, starX + 1, starY);
            SDL_RenderDrawPoint(instance->renderer, starX, starY + 1);
            SDL_RenderDrawPoint(instance->renderer, starX + 1, starY + 1);
        }
        /* Rare very bright stars with cross pattern */
        if (rand() % 30 == 0) {
            SDL_SetRenderDrawColor(instance->renderer, 255, 255, 240, 255);
            SDL_RenderDrawPoint(instance->renderer, starX - 1, starY);
            SDL_RenderDrawPoint(instance->renderer, starX + 2, starY);
            SDL_RenderDrawPoint(instance->renderer, starX, starY - 1);
            SDL_RenderDrawPoint(instance->renderer, starX, starY + 2);
        }
    }
}

void drawMap(Instance *instance, Player *player)
{
    if (!instance || !player || !instance->renderer) {
        return;
    }

    int miniMapSize = WINDOW_WIDTH / 6;
    if (miniMapSize <= 0) miniMapSize = 1;
    
    int miniMapScale = miniMapSize / MAP_WIDTH;
    if (miniMapScale <= 0) miniMapScale = 1;

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
    if (!instance || !instance->renderer || !player) {
        fprintf(stderr, "Error: drawScreen called with NULL pointer\n");
        return;
    }

    /* Draw sky (either day or night) */
    if (instance->isNight) {
        drawNightSky(instance);
    } else {
        drawDaySky(instance);
    }

    /* Draw floor color */
    Uint8 floorColor = instance->isNight ? 50 : 169;  /* Darker floor at night */
    SDL_SetRenderDrawColor(instance->renderer, floorColor, floorColor, floorColor, 255);
    SDL_Rect floor = {0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT / 2};
    SDL_RenderFillRect(instance->renderer, &floor);

    castRays(instance, player);
    drawSprites(instance, player);
    drawMap(instance, player);

    SDL_RenderPresent(instance->renderer);
}

void movePlayer(Player *player, const Uint8 *keystate, float deltaTime)
{
    if (!player || !keystate) {
        fprintf(stderr, "Error: movePlayer called with NULL pointer\n");
        return;
    }

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

    /* Use safe collision check with buffer */
    if (isSafePosition(newX, newY)) {
        player->x = newX;
        player->y = newY;
    }
}

void rotatePlayer(Player *player, const Uint8 *keystate, float deltaTime)
{
    if (!player || !keystate) {
        fprintf(stderr, "Error: rotatePlayer called with NULL pointer\n");
        return;
    }

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

/**
 * Interactive map selection menu
 */
int selectMapInteractive(Instance *instance)
{
    if (!instance || !instance->renderer || !instance->window) {
        fprintf(stderr, "Error: Invalid SDL instance\n");
        return 0;
    }

    int selectedMap = 0;
    int running = 1;
    SDL_Event event;

    printf("=== MAP SELECTION ===\n");
    for (int i = 0; i < NUM_MAPS; i++) {
        printf("%d. %s\n", i + 1, mapOptions[i].name);
    }
    printf("\nUse UP/DOWN arrows to select, ENTER to confirm, ESCAPE to use default\n\n");

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return -1;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        selectedMap = (selectedMap - 1 + NUM_MAPS) % NUM_MAPS;
                        printf("Selected: %s\n", mapOptions[selectedMap].name);
                        break;
                    case SDLK_DOWN:
                        selectedMap = (selectedMap + 1) % NUM_MAPS;
                        printf("Selected: %s\n", mapOptions[selectedMap].name);
                        break;
                    case SDLK_RETURN:
                        running = 0;
                        break;
                    case SDLK_ESCAPE:
                        selectedMap = 0;
                        running = 0;
                        break;
                    default:
                        break;
                }
            }
        }

        /* Draw menu screen */
        SDL_SetRenderDrawColor(instance->renderer, 0, 0, 0, 255);
        SDL_RenderClear(instance->renderer);

        /* Simple text-based menu (rendered via SDL fill rects for visual feedback) */
        int menuY = 100;
        int itemHeight = 80;
        
        for (int i = 0; i < NUM_MAPS; i++) {
            SDL_Rect rect = {
                WINDOW_WIDTH / 2 - 150,
                menuY + i * itemHeight,
                300,
                70
            };
            
            if (i == selectedMap) {
                SDL_SetRenderDrawColor(instance->renderer, 100, 200, 100, 255);
            } else {
                SDL_SetRenderDrawColor(instance->renderer, 50, 50, 50, 255);
            }
            SDL_RenderFillRect(instance->renderer, &rect);
            
            SDL_SetRenderDrawColor(instance->renderer, 200, 200, 200, 255);
            SDL_RenderDrawRect(instance->renderer, &rect);
        }

        SDL_RenderPresent(instance->renderer);
        SDL_Delay(16);
    }

    /* Now select day/night (0 = day, 1 = night) */
    int selectedTime = 0;
    running = 1;

    printf("\n=== TIME OF DAY ===\n");
    printf("1. Day (cloudy sky)\n");
    printf("2. Night (starry sky)\n");
    printf("\nUse LEFT/RIGHT arrows to select, ENTER to confirm, ESCAPE to use Day\n\n");

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return -1;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        selectedTime = 0;
                        printf("Selected: Day\n");
                        break;
                    case SDLK_RIGHT:
                        selectedTime = 1;
                        printf("Selected: Night\n");
                        break;
                    case SDLK_RETURN:
                        running = 0;
                        break;
                    case SDLK_ESCAPE:
                        selectedTime = 0;
                        running = 0;
                        break;
                    default:
                        break;
                }
            }
        }

        /* Draw time selection screen */
        SDL_SetRenderDrawColor(instance->renderer, 0, 0, 0, 255);
        SDL_RenderClear(instance->renderer);

        int timeY = WINDOW_HEIGHT / 2 - 100;
        int timeHeight = 100;

        /* Day option */
        SDL_Rect dayRect = {
            WINDOW_WIDTH / 4 - 100,
            timeY,
            200,
            timeHeight
        };
        SDL_SetRenderDrawColor(instance->renderer, 
                             selectedTime == 0 ? 100 : 50, 200, 100, 255);
        SDL_RenderFillRect(instance->renderer, &dayRect);
        SDL_SetRenderDrawColor(instance->renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(instance->renderer, &dayRect);

        /* Night option */
        SDL_Rect nightRect = {
            3 * WINDOW_WIDTH / 4 - 100,
            timeY,
            200,
            timeHeight
        };
        SDL_SetRenderDrawColor(instance->renderer, 
                             selectedTime == 1 ? 100 : 50, 50, 150, 255);
        SDL_RenderFillRect(instance->renderer, &nightRect);
        SDL_SetRenderDrawColor(instance->renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(instance->renderer, &nightRect);

        SDL_RenderPresent(instance->renderer);
        SDL_Delay(16);
    }

    instance->isNight = selectedTime;
    return selectedMap;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    Instance instance = {0}; /* zero-init */
    Player player;

    if (initializeSDL(&instance) != 0) {
        fprintf(stderr, "Failed to initialize SDL\n");
        return 1;
    }

    if (loadTextures(&instance) != 0) {
        fprintf(stderr, "Failed to load textures\n");
        destroyTextures(&instance);
        cleanupSDL(&instance);
        return 1;
    }

    /* Interactive map selection */
    int selectedMapIndex = selectMapInteractive(&instance);
    if (selectedMapIndex < 0) {
        fprintf(stderr, "User cancelled map selection\n");
        destroyTextures(&instance);
        cleanupSDL(&instance);
        return 0;
    }
    
    /* Load selected map */
    loadMapPreset(selectedMapIndex);
    fprintf(stderr, "Loaded map: %s\n", mapOptions[selectedMapIndex].name);

    /* Initialize player position from map spawn point */
    player.x = mapOptions[selectedMapIndex].spawnX;
    player.y = mapOptions[selectedMapIndex].spawnY;
    player.dirX = -1.0f;
    player.dirY = 0.0f;
    player.planeX = 0.0f;
    player.planeY = 0.66f;

    /* Validate and find valid spawn position if needed */
    if (!findValidSpawn(&player.x, &player.y, player.x, player.y)) {
        fprintf(stderr, "Error: Could not find valid spawn position for map\n");
        destroyTextures(&instance);
        cleanupSDL(&instance);
        return 1;
    }

    /* Generate sprites for this map */
    generateSprites(&instance);

    SDL_Event event;
    int quit = 0;
    Uint32 frameStart = SDL_GetTicks();
    Uint32 lastFrame = frameStart;

    while (!quit) {
        frameStart = SDL_GetTicks();

        /* Handle events */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                quit = 1;
            }
        }

        /* Calculate delta time */
        Uint32 now = SDL_GetTicks();
        float deltaTime = (now - lastFrame) / 1000.0f;
        
        /* Clamp deltaTime to prevent large jumps (e.g., when window is dragged) */
        if (deltaTime > MAX_DELTA_TIME) {
            deltaTime = MAX_DELTA_TIME;
        }
        if (deltaTime < 0.001f) {
            deltaTime = 0.001f;
        }
        lastFrame = now;

        /* Get keyboard state and update player */
        const Uint8 *keystate = SDL_GetKeyboardState(NULL);
        if (!keystate) {
            fprintf(stderr, "Error: Failed to get keyboard state\n");
            break;
        }

        movePlayer(&player, keystate, deltaTime);
        rotatePlayer(&player, keystate, deltaTime);

        /* Render frame */
        drawScreen(&instance, &player);

        /* Frame rate limiting - maintain TARGET_FPS */
        Uint32 frameElapsed = SDL_GetTicks() - frameStart;
        if (frameElapsed < FRAME_DELAY_MS) {
            SDL_Delay(FRAME_DELAY_MS - frameElapsed);
        }
    }

    /* Cleanup */
    destroyTextures(&instance);
    cleanupSDL(&instance);
    
    return 0;
}
