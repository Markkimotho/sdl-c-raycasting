#include <math.h>
#include <stdio.h>

#include "../headers/global.h"
#include "../headers/ui.h"

typedef struct SpriteOrder {
    int index;
    float cameraDepth;
} SpriteOrder;

static int clampInt(int value, int minimum, int maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static float clampFloat(float value, float minimum, float maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static float lerpFloat(float from, float to, float amount)
{
    return from + (to - from) * amount;
}

static float smoothStep(float edge0, float edge1, float value)
{
    float amount;

    if (fabsf(edge1 - edge0) < MIN_RAY_DISTANCE) return value >= edge1 ? 1.0f : 0.0f;
    amount = clampFloat((value - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return amount * amount * (3.0f - 2.0f * amount);
}

static float torchStrength(const Player *player)
{
    float power;

    if (!player->flashlightOn || player->flashlightPower <= 0.0f) return 0.0f;
    power = clampFloat(player->flashlightPower / MAX_FLASHLIGHT_POWER, 0.0f, 1.0f);
    return 0.42f + sqrtf(power) * 0.58f;
}

static float torchBeam(float cameraX, float distance, const Player *player)
{
    float cone = 1.0f - smoothStep(0.17f, 0.64f, fabsf(cameraX));
    float range = 1.0f / (1.0f + 0.055f * distance * distance);

    return cone * range * torchStrength(player);
}

static int viewHorizon(const Player *player)
{
    float bob = sinf(player->bobPhase * 2.0f) * 2.0f * player->bobAmount;
    return LOGICAL_HEIGHT / 2 + (int)bob;
}

static void drawPanel(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color fill,
                      SDL_Color border)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(renderer, &rect);
}

static void drawFilledCircle(SDL_Renderer *renderer, int centerX, int centerY,
                             int radius, SDL_Color color)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int y = -radius; y <= radius; ++y) {
        int halfWidth = (int)sqrtf((float)(radius * radius - y * y));
        SDL_RenderDrawLine(renderer, centerX - halfWidth, centerY + y,
                          centerX + halfWidth, centerY + y);
    }
}

static void drawCelestialBody(SDL_Renderer *renderer, int x, int y, int radius,
                              SDL_Color color, Uint8 visibility)
{
    if (visibility == 0) return;
    for (int halo = 4; halo >= 1; --halo) {
        SDL_Color glow = color;
        glow.a = (Uint8)((unsigned int)visibility * (unsigned int)(5 - halo) / 38U);
        drawFilledCircle(renderer, x, y, radius + halo * 6, glow);
    }
    color.a = visibility;
    drawFilledCircle(renderer, x, y, radius, color);
}

static void drawBackground(Instance *instance)
{
    SDL_Renderer *renderer = instance->renderer;
    int horizon = viewHorizon(&instance->game.player);
    float daylight = clampFloat(instance->daylight, 0.0f, 1.0f);
    float night = 1.0f - daylight;
    float twilight = 4.0f * daylight * night;

    for (int y = 0; y < horizon; ++y) {
        float ratio = horizon > 0 ? (float)y / (float)horizon : 0.0f;
        float horizonWarmth = twilight * ratio * ratio;
        float nightR = lerpFloat(3.0f, 20.0f, ratio);
        float nightG = lerpFloat(7.0f, 27.0f, ratio);
        float nightB = lerpFloat(17.0f, 47.0f, ratio);
        float dayR = lerpFloat(38.0f, 174.0f, ratio);
        float dayG = lerpFloat(88.0f, 199.0f, ratio);
        float dayB = lerpFloat(154.0f, 220.0f, ratio);
        Uint8 r = (Uint8)clampFloat(lerpFloat(nightR, dayR, daylight) +
                                         horizonWarmth * 42.0f, 0.0f, 255.0f);
        Uint8 g = (Uint8)clampFloat(lerpFloat(nightG, dayG, daylight) +
                                         horizonWarmth * 13.0f, 0.0f, 255.0f);
        Uint8 b = (Uint8)clampFloat(lerpFloat(nightB, dayB, daylight) -
                                         horizonWarmth * 18.0f, 0.0f, 255.0f);
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, LOGICAL_WIDTH - 1, y);
    }

    if (night > 0.02f) {
        Uint32 state = 0x6d2b79f5U;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        for (int i = 0; i < 110; ++i) {
            int x;
            int y;
            Uint8 value;
            state = state * 1664525U + 1013904223U;
            x = (int)(state % LOGICAL_WIDTH);
            state = state * 1664525U + 1013904223U;
            y = (int)(state % (Uint32)(horizon > 24 ? horizon - 20 : 1));
            value = (Uint8)(115U + (state >> 25));
            SDL_SetRenderDrawColor(renderer, value, value,
                                   (Uint8)(value + (255 - value) / 3),
                                   (Uint8)(night * (i % 11 == 0 ? 245.0f : 180.0f)));
            SDL_RenderDrawPoint(renderer, x, y);
            if (i % 17 == 0) SDL_RenderDrawPoint(renderer, x + 1, y);
        }
    }

    drawCelestialBody(renderer, LOGICAL_WIDTH * 4 / 5, horizon / 4, 20,
                      (SDL_Color){255, 246, 199, 255},
                      (Uint8)(daylight * 255.0f));
    drawCelestialBody(renderer, LOGICAL_WIDTH / 5, horizon / 3, 15,
                      (SDL_Color){194, 215, 235, 255},
                      (Uint8)(night * 230.0f));

    for (int y = horizon; y < LOGICAL_HEIGHT; ++y) {
        float ratio = (float)(y - horizon) /
                      (float)(LOGICAL_HEIGHT - horizon > 0 ? LOGICAL_HEIGHT - horizon : 1);
        float shade = 1.0f - ratio * 0.52f;
        float nightBase = lerpFloat(33.0f, 16.0f, ratio);
        float dayBase = lerpFloat(104.0f, 49.0f, ratio);
        float base = lerpFloat(nightBase, dayBase, daylight);
        Uint8 r = (Uint8)(base * shade);
        Uint8 g = (Uint8)((base + lerpFloat(7.0f, 9.0f, daylight)) * shade);
        Uint8 b = (Uint8)((base + lerpFloat(17.0f, 2.0f, daylight)) * shade);
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, LOGICAL_WIDTH - 1, y);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (int band = 1; band <= 9; ++band) {
        float depth = (float)band / 9.0f;
        int y = horizon + (int)((LOGICAL_HEIGHT - horizon) * depth * depth);
        Uint8 alpha = (Uint8)((31.0f + daylight * 23.0f) * (1.0f - depth) + 6.0f);
        SDL_SetRenderDrawColor(renderer,
                               (Uint8)lerpFloat(85.0f, 188.0f, daylight),
                               (Uint8)lerpFloat(101.0f, 175.0f, daylight),
                               (Uint8)lerpFloat(126.0f, 146.0f, daylight), alpha);
        SDL_RenderDrawLine(renderer, 0, y, LOGICAL_WIDTH - 1, y);
    }
}

static void castWalls(Instance *instance)
{
    Player *player = &instance->game.player;
    SDL_Renderer *renderer = instance->renderer;
    int horizon = viewHorizon(player);

    for (int screenX = 0; screenX < LOGICAL_WIDTH; ++screenX) {
        float cameraX = 2.0f * (float)screenX / (float)LOGICAL_WIDTH - 1.0f;
        float rayDirX = player->dirX + player->planeX * cameraX;
        float rayDirY = player->dirY + player->planeY * cameraX;
        int mapX = (int)floorf(player->x);
        int mapY = (int)floorf(player->y);
        float deltaDistX = fabsf(rayDirX) < MIN_RAY_DISTANCE ? 1.0e30f : fabsf(1.0f / rayDirX);
        float deltaDistY = fabsf(rayDirY) < MIN_RAY_DISTANCE ? 1.0e30f : fabsf(1.0f / rayDirY);
        float sideDistX;
        float sideDistY;
        float distance;
        int stepX;
        int stepY;
        int wallType = 1;
        int side = 0;
        int hit = 0;

        if (rayDirX < 0.0f) {
            stepX = -1;
            sideDistX = (player->x - (float)mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = ((float)mapX + 1.0f - player->x) * deltaDistX;
        }
        if (rayDirY < 0.0f) {
            stepY = -1;
            sideDistY = (player->y - (float)mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = ((float)mapY + 1.0f - player->y) * deltaDistY;
        }

        for (int guard = 0; !hit && guard < MAP_WIDTH * MAP_HEIGHT * 2; ++guard) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
                hit = 1;
                wallType = 1;
            } else if (map[mapY][mapX] > 0) {
                hit = 1;
                wallType = map[mapY][mapX];
            }
        }

        if (side == 0) {
            distance = ((float)mapX - player->x + (1.0f - (float)stepX) * 0.5f) / rayDirX;
        } else {
            distance = ((float)mapY - player->y + (1.0f - (float)stepY) * 0.5f) / rayDirY;
        }
        if (!isfinite(distance) || distance < MIN_RAY_DISTANCE) distance = MIN_RAY_DISTANCE;
        instance->zBuffer[screenX] = distance;

        if (wallType <= 0 || wallType >= MAX_WALL_TYPES) wallType = 1;
        TextureAsset *asset = &instance->wallTextures[wallType];
        if (!asset->texture || asset->width <= 0 || asset->height <= 0) continue;

        int lineHeight = (int)((float)LOGICAL_HEIGHT / distance);
        int originalStart = horizon - lineHeight / 2;
        int originalEnd = horizon + lineHeight / 2;
        int drawStart = clampInt(originalStart, 0, LOGICAL_HEIGHT - 1);
        int drawEnd = clampInt(originalEnd, 0, LOGICAL_HEIGHT - 1);
        float wallX = side == 0 ? player->y + distance * rayDirY :
                                  player->x + distance * rayDirX;
        int textureX;
        int textureY;
        int textureHeight;
        float daylight;
        float nightAmbient;
        float dayDiffuse;
        float beam;
        float sideFactor;
        float redLight;
        float greenLight;
        float blueLight;
        Uint8 red;
        Uint8 green;
        Uint8 blue;

        wallX -= floorf(wallX);
        textureX = clampInt((int)(wallX * asset->width), 0, asset->width - 1);
        if (side == 0 && rayDirX > 0.0f) textureX = asset->width - textureX - 1;
        if (side == 1 && rayDirY < 0.0f) textureX = asset->width - textureX - 1;

        textureY = lineHeight > 0 ?
            (drawStart - originalStart) * asset->height / lineHeight : 0;
        textureHeight = lineHeight > 0 ?
            (drawEnd - drawStart + 1) * asset->height / lineHeight : asset->height;
        textureY = clampInt(textureY, 0, asset->height - 1);
        textureHeight = clampInt(textureHeight, 1, asset->height - textureY);

        daylight = clampFloat(instance->daylight, 0.0f, 1.0f);
        sideFactor = side ? 0.76f : 1.0f;
        dayDiffuse = clampFloat(1.06f / (1.0f + distance * 0.05f), 0.34f, 1.0f) *
                     sideFactor;
        nightAmbient = (0.055f + 0.18f / (1.0f + distance * 0.16f)) *
                       (side ? 0.82f : 1.0f);
        beam = torchBeam(cameraX, distance, player) *
               lerpFloat(0.12f, 1.0f, 1.0f - daylight);
        redLight = lerpFloat(nightAmbient * 0.70f, dayDiffuse, daylight) + beam;
        greenLight = lerpFloat(nightAmbient * 0.84f, dayDiffuse * 0.98f,
                               daylight) + beam * 0.88f;
        blueLight = lerpFloat(nightAmbient * 1.18f, dayDiffuse * 0.93f,
                              daylight) + beam * 0.66f;
        red = (Uint8)(255.0f * clampFloat(redLight, 0.025f, 1.0f));
        green = (Uint8)(255.0f * clampFloat(greenLight, 0.03f, 1.0f));
        blue = (Uint8)(255.0f * clampFloat(blueLight, 0.04f, 1.0f));
        SDL_SetTextureColorMod(asset->texture, red, green, blue);

        SDL_Rect source = {textureX, textureY, 1, textureHeight};
        SDL_Rect destination = {screenX, drawStart, 1, drawEnd - drawStart + 1};
        SDL_RenderCopy(renderer, asset->texture, &source, &destination);
    }

    for (int i = 1; i < MAX_WALL_TYPES; ++i) {
        if (instance->wallTextures[i].texture) {
            SDL_SetTextureColorMod(instance->wallTextures[i].texture, 255, 255, 255);
        }
    }
}

static void sortSprites(const GameState *game, SpriteOrder *order, int *count)
{
    const Player *player = &game->player;
    float determinant = player->planeX * player->dirY - player->dirX * player->planeY;

    *count = 0;
    if (fabsf(determinant) < MIN_RAY_DISTANCE) return;
    for (int i = 0; i < game->entityCount; ++i) {
        const Entity *entity = &game->entities[i];
        if (!entity->active) continue;
        float dx = entity->x - player->x;
        float dy = entity->y - player->y;
        float inverse = 1.0f / determinant;
        float cameraDepth = inverse * (-player->planeY * dx + player->planeX * dy);
        SpriteOrder next = {i, cameraDepth};
        int position = *count;
        while (position > 0 && order[position - 1].cameraDepth < next.cameraDepth) {
            order[position] = order[position - 1];
            position--;
        }
        order[position] = next;
        (*count)++;
    }
}

static int entityUsesFloorAnchor(EntityType type)
{
    return type != ENTITY_EXIT;
}

static void drawEntityShadow(Instance *instance, float depth, int centerX,
                             int groundY, int spriteWidth, int spriteHeight,
                             const TextureAsset *asset)
{
    float visibleWidth = (float)(asset->contentRight - asset->contentLeft + 1) /
                         (float)asset->width;
    int width = clampInt((int)(spriteWidth * visibleWidth * 0.72f), 3,
                         LOGICAL_WIDTH * 2);
    int height = clampInt((int)(spriteHeight * 0.045f), 2, 18);
    int startX = clampInt(centerX - width / 2, 0, LOGICAL_WIDTH - 1);
    int endX = clampInt(centerX + width / 2, 0, LOGICAL_WIDTH - 1);
    Uint8 alpha = (Uint8)lerpFloat(48.0f, 92.0f,
                                  clampFloat(instance->daylight, 0.0f, 1.0f));

    if (groundY + height < 0 || groundY - height >= LOGICAL_HEIGHT) return;
    SDL_SetRenderDrawBlendMode(instance->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(instance->renderer, 0, 0, 0, alpha);
    for (int x = startX; x <= endX; ++x) {
        float normalized = ((float)x - (float)centerX) / ((float)width * 0.5f);
        float profile;
        int halfHeight;
        int top;
        int bottom;

        if (depth >= instance->zBuffer[x] || fabsf(normalized) > 1.0f) continue;
        profile = sqrtf(clampFloat(1.0f - normalized * normalized, 0.0f, 1.0f));
        halfHeight = (int)(height * 0.5f * profile);
        top = clampInt(groundY - halfHeight, 0, LOGICAL_HEIGHT - 1);
        bottom = clampInt(groundY + halfHeight, 0, LOGICAL_HEIGHT - 1);
        if (bottom >= top) SDL_RenderDrawLine(instance->renderer, x, top, x, bottom);
    }
}

static void drawSprites(Instance *instance)
{
    GameState *game = &instance->game;
    Player *player = &game->player;
    SpriteOrder order[MAX_ENTITIES];
    int spriteCount;
    int horizon = viewHorizon(player);
    float determinant = player->planeX * player->dirY - player->dirX * player->planeY;

    if (fabsf(determinant) < MIN_RAY_DISTANCE) return;
    sortSprites(game, order, &spriteCount);

    for (int sorted = 0; sorted < spriteCount; ++sorted) {
        Entity *entity = &game->entities[order[sorted].index];
        TextureAsset *asset = &instance->objectTextures[entity->textureId];
        float spriteX = entity->x - player->x;
        float spriteY = entity->y - player->y;
        float inverse = 1.0f / determinant;
        float transformX = inverse * (player->dirY * spriteX - player->dirX * spriteY);
        float transformY = inverse * (-player->planeY * spriteX + player->planeX * spriteY);
        float scale = entity->scale;

        if (!asset->texture || transformY <= MIN_RAY_DISTANCE) continue;
        if (entity->type == ENTITY_POWER_CELL) {
            scale *= 1.0f + sinf(game->elapsedTime * 3.2f + entity->x) * 0.045f;
        }

        int spriteScreenX = (int)((LOGICAL_WIDTH * 0.5f) *
                            (1.0f + transformX / transformY));
        int spriteHeight = (int)fabsf((float)LOGICAL_HEIGHT / transformY * scale);
        int spriteWidth = spriteHeight;
        if (spriteHeight > LOGICAL_HEIGHT * 8) spriteHeight = LOGICAL_HEIGHT * 8;
        if (spriteWidth > LOGICAL_HEIGHT * 8) spriteWidth = LOGICAL_HEIGHT * 8;
        int floorScreenY = horizon + (int)((LOGICAL_HEIGHT * 0.5f) / transformY);
        int ceilingScreenY = horizon - (int)((LOGICAL_HEIGHT * 0.5f) / transformY);
        float visibleBottom = (float)(asset->contentBottom + 1) / (float)asset->height;
        float visibleTop = (float)asset->contentTop / (float)asset->height;
        int originalStartY = entityUsesFloorAnchor(entity->type) ?
            floorScreenY - (int)lroundf(visibleBottom * spriteHeight) :
            ceilingScreenY - (int)lroundf(visibleTop * spriteHeight);
        int originalEndY = originalStartY + spriteHeight - 1;
        int drawStartY = clampInt(originalStartY, 0, LOGICAL_HEIGHT - 1);
        int drawEndY = clampInt(originalEndY, 0, LOGICAL_HEIGHT - 1);
        int originalStartX = spriteScreenX - spriteWidth / 2;
        int originalEndX = originalStartX + spriteWidth - 1;
        int drawStartX;
        int drawEndX;
        int sourceY;
        int sourceHeight;
        float daylight;
        float nightAmbient;
        float dayDiffuse;
        float beam;
        float redLight;
        float greenLight;
        float blueLight;
        float emission = 0.0f;
        Uint8 baseR = 255;
        Uint8 baseG = 255;
        Uint8 baseB = 255;
        Uint8 tintR;
        Uint8 tintG;
        Uint8 tintB;

        if (spriteHeight <= 0 || spriteWidth <= 0 || originalEndX < 0 ||
            originalStartX >= LOGICAL_WIDTH || originalEndY < 0 ||
            originalStartY >= LOGICAL_HEIGHT) continue;
        drawStartX = clampInt(originalStartX, 0, LOGICAL_WIDTH - 1);
        drawEndX = clampInt(originalEndX, 0, LOGICAL_WIDTH - 1);
        if (drawEndX < drawStartX ||
            drawEndY < drawStartY) continue;

        sourceY = (drawStartY - originalStartY) * asset->height / spriteHeight;
        sourceHeight = (drawEndY - drawStartY + 1) * asset->height / spriteHeight;
        sourceY = clampInt(sourceY, 0, asset->height - 1);
        sourceHeight = clampInt(sourceHeight, 1, asset->height - sourceY);

        if (entityUsesFloorAnchor(entity->type)) {
            drawEntityShadow(instance, transformY, spriteScreenX, floorScreenY,
                             spriteWidth, spriteHeight, asset);
        }

        daylight = clampFloat(instance->daylight, 0.0f, 1.0f);
        dayDiffuse = clampFloat(1.04f / (1.0f + transformY * 0.045f),
                                0.38f, 1.0f);
        nightAmbient = 0.06f + 0.20f / (1.0f + transformY * 0.14f);
        beam = torchBeam(transformX / transformY, transformY, player) *
               lerpFloat(0.14f, 1.0f, 1.0f - daylight);
        redLight = lerpFloat(nightAmbient * 0.70f, dayDiffuse, daylight) + beam;
        greenLight = lerpFloat(nightAmbient * 0.84f, dayDiffuse * 0.98f,
                               daylight) + beam * 0.88f;
        blueLight = lerpFloat(nightAmbient * 1.18f, dayDiffuse * 0.94f,
                              daylight) + beam * 0.66f;

        if (entity->type == ENTITY_EXIT) {
            if (player->collectedCells < player->totalCells) {
                baseR = 255;
                baseG = 92;
                baseB = 82;
            } else {
                baseR = 98;
                baseG = 255;
                baseB = 170;
            }
            emission = 0.56f;
        } else if (entity->type == ENTITY_POWER_CELL) {
            baseR = 255;
            baseG = 210;
            baseB = 112;
            emission = 0.32f;
        } else if (entity->type == ENTITY_SENTINEL && entity->health < 100.0f) {
            baseR = 255;
            baseG = (Uint8)clampFloat(130.0f + entity->health * 1.1f,
                                      0.0f, 255.0f);
            baseB = (Uint8)clampFloat(115.0f + entity->health * 1.2f,
                                      0.0f, 255.0f);
        }
        tintR = (Uint8)(baseR * clampFloat(redLight + emission, 0.03f, 1.0f));
        tintG = (Uint8)(baseG * clampFloat(greenLight + emission, 0.03f, 1.0f));
        tintB = (Uint8)(baseB * clampFloat(blueLight + emission, 0.04f, 1.0f));
        SDL_SetTextureColorMod(asset->texture, tintR, tintG, tintB);

        for (int stripe = drawStartX; stripe <= drawEndX; ++stripe) {
            int sourceX;
            if (transformY >= instance->zBuffer[stripe]) continue;
            sourceX = (stripe - originalStartX) * asset->width / spriteWidth;
            sourceX = clampInt(sourceX, 0, asset->width - 1);
            SDL_Rect source = {sourceX, sourceY, 1, sourceHeight};
            SDL_Rect destination = {stripe, drawStartY, 1, drawEndY - drawStartY + 1};
            SDL_RenderCopy(instance->renderer, asset->texture, &source, &destination);
        }
        SDL_SetTextureColorMod(asset->texture, 255, 255, 255);
    }
}

static void drawLighting(Instance *instance)
{
    Player *player = &instance->game.player;
    SDL_Rect screen = {0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT};
    float daylight = clampFloat(instance->daylight, 0.0f, 1.0f);
    float night = 1.0f - daylight;
    float powerRatio = clampFloat(player->flashlightPower /
                                  MAX_FLASHLIGHT_POWER, 0.0f, 1.0f);
    float flicker = 1.0f;
    int torchOn = player->flashlightOn && player->flashlightPower > 0.0f;

    if (torchOn && powerRatio < 0.18f) {
        float pulse = sinf(instance->game.elapsedTime * 43.0f) *
                      sinf(instance->game.elapsedTime * 17.0f + 0.8f);
        flicker = 0.76f + 0.24f * (pulse * 0.5f + 0.5f);
    }

    if (night > 0.01f) {
        if (torchOn) {
            SDL_SetTextureAlphaMod(instance->flashlightMask,
                                   (Uint8)(night * 255.0f));
            SDL_RenderCopy(instance->renderer, instance->flashlightMask, NULL, &screen);
            if (powerRatio < 0.28f) {
                Uint8 failingLightAlpha = (Uint8)(night * (0.28f - powerRatio) /
                                                   0.28f * 68.0f);
                SDL_SetRenderDrawBlendMode(instance->renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(instance->renderer, 0, 2, 7, failingLightAlpha);
                SDL_RenderFillRect(instance->renderer, &screen);
            }
        } else {
            SDL_SetRenderDrawBlendMode(instance->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(instance->renderer, 0, 3, 11,
                                   (Uint8)(night * 225.0f));
            SDL_RenderFillRect(instance->renderer, &screen);
        }
    }

    if (torchOn) {
        float glowAlpha = lerpFloat(58.0f, 220.0f, night) *
                          torchStrength(player) * flicker;
        SDL_SetTextureAlphaMod(instance->torchGlow,
                               (Uint8)clampFloat(glowAlpha, 0.0f, 255.0f));
        SDL_RenderCopy(instance->renderer, instance->torchGlow, NULL, &screen);
    }

    SDL_SetTextureAlphaMod(instance->vignette,
                           (Uint8)lerpFloat(184.0f, 255.0f, night));
    SDL_RenderCopy(instance->renderer, instance->vignette, NULL, &screen);

    if (player->damageFlash > 0.0f) {
        SDL_SetRenderDrawBlendMode(instance->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(instance->renderer, 160, 4, 2,
                               (Uint8)(player->damageFlash * 115.0f));
        SDL_RenderFillRect(instance->renderer, &screen);
    }
}

static void drawViewModel(Instance *instance)
{
    Player *player = &instance->game.player;
    TextureAsset *hands = &instance->povHands;
    int size = 590;
    float sway = sinf(player->bobPhase) * 5.0f * player->bobAmount;
    float bob = fabsf(cosf(player->bobPhase * 2.0f)) * 6.0f * player->bobAmount;
    float use = sinf((1.0f - player->useAnimation) * 3.14159265f) * player->useAnimation;
    SDL_Rect destination = {
        (LOGICAL_WIDTH - size) / 2 + 50 + (int)sway,
        100 + (int)bob - (int)(use * 22.0f),
        size,
        size
    };

    if (!hands->texture) return;
    if (player->flashlightOn && player->flashlightPower > 0.0f) {
        SDL_SetTextureColorMod(hands->texture, 255, 255, 255);
    } else {
        Uint8 ambient = (Uint8)lerpFloat(145.0f, 226.0f,
                                        clampFloat(instance->daylight, 0.0f, 1.0f));
        SDL_SetTextureColorMod(hands->texture, ambient, ambient,
                               (Uint8)clampInt((int)ambient + 8, 0, 255));
    }
    SDL_RenderCopyEx(instance->renderer, hands->texture, NULL, &destination,
                     sway * 0.22f, NULL, SDL_FLIP_NONE);
    SDL_SetTextureColorMod(hands->texture, 255, 255, 255);
}

static void drawBar(SDL_Renderer *renderer, int x, int y, int width, int height,
                    float amount, SDL_Color fill)
{
    SDL_Rect background = {x, y, width, height};
    SDL_Rect foreground = {x + 2, y + 2,
        (int)((width - 4) * clampFloat(amount, 0.0f, 1.0f)), height - 4};
    SDL_SetRenderDrawColor(renderer, 4, 7, 10, 210);
    SDL_RenderFillRect(renderer, &background);
    SDL_SetRenderDrawColor(renderer, 195, 205, 210, 180);
    SDL_RenderDrawRect(renderer, &background);
    SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(renderer, &foreground);
}

static void drawMinimap(Instance *instance)
{
    const GameState *game = &instance->game;
    const Player *player = &game->player;
    const int scale = 7;
    const int size = MAP_WIDTH * scale;
    const int startX = LOGICAL_WIDTH - size - 17;
    const int startY = 17;
    SDL_Rect panel = {startX - 7, startY - 7, size + 14, size + 14};

    drawPanel(instance->renderer, panel, (SDL_Color){3, 8, 11, 185},
              (SDL_Color){103, 144, 150, 210});
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            SDL_Rect cell = {startX + x * scale, startY + y * scale, scale, scale};
            if (map[y][x] > 0) SDL_SetRenderDrawColor(instance->renderer, 95, 106, 105, 230);
            else SDL_SetRenderDrawColor(instance->renderer, 11, 19, 20, 210);
            SDL_RenderFillRect(instance->renderer, &cell);
        }
    }

    for (int i = 0; i < game->entityCount; ++i) {
        const Entity *entity = &game->entities[i];
        SDL_Rect marker;
        if (!entity->active) continue;
        marker.x = startX + (int)(entity->x * scale) - 1;
        marker.y = startY + (int)(entity->y * scale) - 1;
        marker.w = 3;
        marker.h = 3;
        if (entity->type == ENTITY_POWER_CELL) SDL_SetRenderDrawColor(instance->renderer, 255, 196, 55, 255);
        else if (entity->type == ENTITY_EXIT) SDL_SetRenderDrawColor(instance->renderer, 52, 241, 145, 255);
        else if (entity->type == ENTITY_SENTINEL) SDL_SetRenderDrawColor(instance->renderer, 210, 53, 45, 255);
        else continue;
        SDL_RenderFillRect(instance->renderer, &marker);
    }

    int playerX = startX + (int)(player->x * scale);
    int playerY = startY + (int)(player->y * scale);
    SDL_SetRenderDrawColor(instance->renderer, 238, 246, 247, 255);
    SDL_Rect playerMarker = {playerX - 2, playerY - 2, 5, 5};
    SDL_RenderFillRect(instance->renderer, &playerMarker);
    SDL_RenderDrawLine(instance->renderer, playerX, playerY,
                       playerX + (int)(player->dirX * 10.0f),
                       playerY + (int)(player->dirY * 10.0f));
}

static int playerCanInteract(const GameState *game)
{
    return gameFindInteractable(game) >= 0;
}

static void drawHud(Instance *instance)
{
    GameState *game = &instance->game;
    Player *player = &game->player;
    SDL_Color white = {229, 238, 235, 255};
    SDL_Color muted = {150, 171, 172, 255};
    char text[96];
    SDL_Rect hudPanel = {14, LOGICAL_HEIGHT - 104, 225, 90};

    drawPanel(instance->renderer, hudPanel, (SDL_Color){3, 7, 9, 184},
              (SDL_Color){78, 113, 115, 205});
    uiDrawText(instance->renderer, 24, LOGICAL_HEIGHT - 94, 2, "HEALTH", muted);
    drawBar(instance->renderer, 24, LOGICAL_HEIGHT - 76, 198, 12,
            player->health / MAX_HEALTH, (SDL_Color){190, 48, 39, 255});
    uiDrawText(instance->renderer, 24, LOGICAL_HEIGHT - 58, 2, "STAMINA", muted);
    drawBar(instance->renderer, 24, LOGICAL_HEIGHT - 40, 92, 10,
            player->stamina / MAX_STAMINA, (SDL_Color){72, 175, 139, 255});
    uiDrawText(instance->renderer, 128, LOGICAL_HEIGHT - 58, 2, "BATTERY", muted);
    drawBar(instance->renderer, 128, LOGICAL_HEIGHT - 40, 94, 10,
            player->flashlightPower / MAX_FLASHLIGHT_POWER,
            (SDL_Color){226, 176, 62, 255});

    snprintf(text, sizeof(text), "CELLS %d/%d", player->collectedCells, player->totalCells);
    uiDrawText(instance->renderer, 18, 18, 3, text, white);
    uiDrawText(instance->renderer, 18, 46, 2,
               player->collectedCells == player->totalCells ?
               "OBJECTIVE: REACH EXTRACTION" : "OBJECTIVE: RECOVER POWER CELLS",
               (SDL_Color){222, 177, 73, 255});
    if (player->flashlightPower <= 0.0f) {
        snprintf(text, sizeof(text), "%s  [F] TORCH EMPTY",
                 instance->isNight ? "NIGHT" : "DAY");
    } else {
        snprintf(text, sizeof(text), "%s  [F] TORCH %s",
                 instance->isNight ? "NIGHT" : "DAY",
                 player->flashlightOn ? "ON" : "OFF");
    }
    uiDrawText(instance->renderer, 18, 70, 2, text,
               player->flashlightOn && player->flashlightPower > 0.0f ?
               (SDL_Color){247, 195, 91, 255} : (SDL_Color){136, 164, 178, 255});

    SDL_SetRenderDrawBlendMode(instance->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(instance->renderer, 234, 244, 237, 210);
    SDL_RenderDrawLine(instance->renderer, LOGICAL_WIDTH / 2 - 7, LOGICAL_HEIGHT / 2,
                       LOGICAL_WIDTH / 2 - 2, LOGICAL_HEIGHT / 2);
    SDL_RenderDrawLine(instance->renderer, LOGICAL_WIDTH / 2 + 2, LOGICAL_HEIGHT / 2,
                       LOGICAL_WIDTH / 2 + 7, LOGICAL_HEIGHT / 2);
    SDL_RenderDrawLine(instance->renderer, LOGICAL_WIDTH / 2, LOGICAL_HEIGHT / 2 - 7,
                       LOGICAL_WIDTH / 2, LOGICAL_HEIGHT / 2 - 2);
    SDL_RenderDrawLine(instance->renderer, LOGICAL_WIDTH / 2, LOGICAL_HEIGHT / 2 + 2,
                       LOGICAL_WIDTH / 2, LOGICAL_HEIGHT / 2 + 7);

    if (game->showMinimap) drawMinimap(instance);
    if (playerCanInteract(game)) {
        uiDrawTextCentered(instance->renderer, LOGICAL_WIDTH / 2,
                           LOGICAL_HEIGHT - 118, 2, "[E] INTERACT", white);
    }
    if (game->messageTimer > 0.0f && game->message[0] != '\0') {
        SDL_Rect messagePanel = {
            LOGICAL_WIDTH / 2 - uiTextWidth(game->message, 2) / 2 - 12,
            78, uiTextWidth(game->message, 2) + 24, 30
        };
        drawPanel(instance->renderer, messagePanel, (SDL_Color){2, 5, 7, 196},
                  (SDL_Color){143, 168, 164, 205});
        uiDrawTextCentered(instance->renderer, LOGICAL_WIDTH / 2, 86, 2,
                           game->message, white);
    }
}

static void drawWorld(Instance *instance)
{
    drawBackground(instance);
    castWalls(instance);
    drawSprites(instance);
    drawLighting(instance);
    drawViewModel(instance);
    drawHud(instance);
}

static void drawTitle(Instance *instance)
{
    SDL_Renderer *renderer = instance->renderer;
    GameState *game = &instance->game;
    SDL_Color white = {230, 237, 233, 255};
    SDL_Color amber = {227, 174, 67, 255};

    for (int y = 0; y < LOGICAL_HEIGHT; ++y) {
        float ratio = (float)y / LOGICAL_HEIGHT;
        SDL_SetRenderDrawColor(renderer,
            (Uint8)(4 + ratio * 9), (Uint8)(8 + ratio * 11),
            (Uint8)(12 + ratio * 13), 255);
        SDL_RenderDrawLine(renderer, 0, y, LOGICAL_WIDTH - 1, y);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 75, 112, 105, 35);
    for (int x = -LOGICAL_HEIGHT; x < LOGICAL_WIDTH; x += 38) {
        SDL_RenderDrawLine(renderer, x, LOGICAL_HEIGHT, x + LOGICAL_HEIGHT, 0);
    }

    uiDrawTextCentered(renderer, LOGICAL_WIDTH / 2, 88, 7, "BUNKER RUN", amber);
    uiDrawTextCentered(renderer, LOGICAL_WIDTH / 2, 152, 2,
                       "A RAYCASTING SURVIVAL MISSION", white);
    uiDrawTextCentered(renderer, LOGICAL_WIDTH / 2, 202, 2,
                       "SELECT AN INFILTRATION ZONE", (SDL_Color){139, 165, 163, 255});

    for (int i = 0; i < NUM_MAPS; ++i) {
        SDL_Rect option = {LOGICAL_WIDTH / 2 - 205, 238 + i * 55, 410, 42};
        int selected = i == game->selectedMap;
        drawPanel(renderer, option,
                  selected ? (SDL_Color){47, 67, 60, 238} : (SDL_Color){8, 15, 17, 218},
                  selected ? amber : (SDL_Color){66, 91, 91, 220});
        char label[80];
        snprintf(label, sizeof(label), "%d  %s%s", i + 1, mapOptions[i].name,
                 mapOptions[i].startsAtNight ? "  [NIGHT]" : "  [DAY]");
        uiDrawTextCentered(renderer, LOGICAL_WIDTH / 2, 251 + i * 55, 2,
                           label, selected ? white : (SDL_Color){136, 154, 153, 255});
    }

    uiDrawTextCentered(renderer, LOGICAL_WIDTH / 2, 430, 2,
                       "ENTER TO DEPLOY   ARROWS TO SELECT", amber);
    uiDrawTextCentered(renderer, LOGICAL_WIDTH / 2, 496, 2,
                       "WASD MOVE   MOUSE LOOK   SHIFT SPRINT   E USE", white);
    uiDrawTextCentered(renderer, LOGICAL_WIDTH / 2, 526, 2,
                       "F TORCH   M MAP   N DAY/NIGHT   ESC PAUSE", white);
    uiDrawTextCentered(renderer, LOGICAL_WIDTH / 2, 626, 1,
                       "COLLECT EVERY POWER CELL. USE LIGHT TO STOP THE SENTINELS.",
                       (SDL_Color){132, 153, 152, 255});
}

static void drawModeOverlay(Instance *instance)
{
    GameMode mode = instance->game.mode;
    SDL_Color white = {235, 241, 237, 255};
    SDL_Color accent = mode == GAME_MODE_OVER ?
        (SDL_Color){224, 69, 58, 255} : (SDL_Color){226, 177, 73, 255};
    const char *title = mode == GAME_MODE_PAUSED ? "PAUSED" :
                        mode == GAME_MODE_WON ? "EXTRACTION COMPLETE" : "MISSION FAILED";
    const char *hint = mode == GAME_MODE_PAUSED ?
        "ESC RESUME   R RESTART   Q TITLE" : "R RESTART   ENTER TITLE";
    SDL_Rect shade = {0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT};
    SDL_Rect panel = {LOGICAL_WIDTH / 2 - 245, LOGICAL_HEIGHT / 2 - 72, 490, 144};

    SDL_SetRenderDrawBlendMode(instance->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(instance->renderer, 0, 2, 4, 165);
    SDL_RenderFillRect(instance->renderer, &shade);
    drawPanel(instance->renderer, panel, (SDL_Color){5, 10, 12, 238},
              (SDL_Color){121, 144, 141, 230});
    uiDrawTextCentered(instance->renderer, LOGICAL_WIDTH / 2,
                       LOGICAL_HEIGHT / 2 - 43, 4, title, accent);
    uiDrawTextCentered(instance->renderer, LOGICAL_WIDTH / 2,
                       LOGICAL_HEIGHT / 2 + 23, 2, hint, white);
}

void renderDraw(Instance *instance)
{
    if (!instance || !instance->renderer) return;
    SDL_SetRenderDrawColor(instance->renderer, 0, 0, 0, 255);
    SDL_RenderClear(instance->renderer);

    if (instance->game.mode == GAME_MODE_TITLE) {
        drawTitle(instance);
    } else {
        drawWorld(instance);
        if (instance->game.mode == GAME_MODE_PAUSED ||
            instance->game.mode == GAME_MODE_WON ||
            instance->game.mode == GAME_MODE_OVER) {
            drawModeOverlay(instance);
        }
    }
}

void renderFrame(Instance *instance)
{
    if (!instance || !instance->renderer) return;
    renderDraw(instance);
    SDL_RenderPresent(instance->renderer);
}
