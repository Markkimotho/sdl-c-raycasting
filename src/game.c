#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../headers/global.h"

static float clampFloat(float value, float minimum, float maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static void setMouseCapture(int enabled)
{
    SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE);
    SDL_ShowCursor(enabled ? SDL_DISABLE : SDL_ENABLE);
}

static void setMessage(GameState *game, const char *message, float duration)
{
    if (!game || !message) return;
    snprintf(game->message, sizeof(game->message), "%s", message);
    game->messageTimer = duration;
}

static int circleTouchesWalls(float x, float y, float radius)
{
    int minX = (int)floorf(x - radius);
    int maxX = (int)floorf(x + radius);
    int minY = (int)floorf(y - radius);
    int maxY = (int)floorf(y + radius);

    for (int cellY = minY; cellY <= maxY; ++cellY) {
        for (int cellX = minX; cellX <= maxX; ++cellX) {
            if (mapCellIsOpen(cellX, cellY)) continue;

            float nearestX = clampFloat(x, (float)cellX, (float)cellX + 1.0f);
            float nearestY = clampFloat(y, (float)cellY, (float)cellY + 1.0f);
            float dx = x - nearestX;
            float dy = y - nearestY;
            if (dx * dx + dy * dy < radius * radius) {
                return 1;
            }
        }
    }
    return 0;
}

static int circleTouchesEntities(const GameState *game, float x, float y,
                                 float radius, int ignoredEntity)
{
    for (int i = 0; i < game->entityCount; ++i) {
        const Entity *entity = &game->entities[i];
        if (i == ignoredEntity || !entity->active || !entity->solid) continue;

        float dx = x - entity->x;
        float dy = y - entity->y;
        float combinedRadius = radius + entity->radius;
        if (dx * dx + dy * dy < combinedRadius * combinedRadius) {
            return 1;
        }
    }
    return 0;
}

static int playerPositionIsFree(const GameState *game, float x, float y)
{
    const Player *player = &game->player;
    return !circleTouchesWalls(x, y, player->radius) &&
           !circleTouchesEntities(game, x, y, player->radius, -1);
}

static int hasLineOfSight(float fromX, float fromY, float toX, float toY)
{
    float dx = toX - fromX;
    float dy = toY - fromY;
    float distance = sqrtf(dx * dx + dy * dy);
    int steps = (int)(distance / 0.08f);

    if (steps <= 1) return 1;
    for (int i = 1; i < steps; ++i) {
        float amount = (float)i / (float)steps;
        int cellX = (int)floorf(fromX + dx * amount);
        int cellY = (int)floorf(fromY + dy * amount);
        if (!mapCellIsOpen(cellX, cellY)) return 0;
    }
    return 1;
}

int gameFindInteractable(const GameState *game)
{
    const Player *player;
    float nearestDistance = 1.35f;
    int nearestIndex = -1;

    if (!game) return -1;
    player = &game->player;
    for (int i = 0; i < game->entityCount; ++i) {
        const Entity *entity = &game->entities[i];
        float dx;
        float dy;
        float distance;
        float facing;

        if (!entity->active ||
            (entity->type != ENTITY_POWER_CELL && entity->type != ENTITY_EXIT)) {
            continue;
        }
        dx = entity->x - player->x;
        dy = entity->y - player->y;
        distance = sqrtf(dx * dx + dy * dy);
        if (distance >= nearestDistance) continue;
        facing = distance <= 0.001f ? 1.0f :
                 (dx * player->dirX + dy * player->dirY) / distance;
        if (facing < 0.25f) continue;
        if (!hasLineOfSight(player->x, player->y, entity->x, entity->y)) continue;

        nearestDistance = distance;
        nearestIndex = i;
    }
    return nearestIndex;
}

static ObjectTextureId textureForEntity(EntityType type)
{
    switch (type) {
        case ENTITY_BARREL: return OBJECT_TEXTURE_BARREL;
        case ENTITY_PILLAR: return OBJECT_TEXTURE_PILLAR;
        case ENTITY_POWER_CELL: return OBJECT_TEXTURE_POWER_CELL;
        case ENTITY_EXIT: return OBJECT_TEXTURE_LIGHT;
        case ENTITY_SENTINEL: return OBJECT_TEXTURE_SENTINEL;
        default: return OBJECT_TEXTURE_BARREL;
    }
}

static void configureEntity(Entity *entity, const EntitySpawn *spawn)
{
    memset(entity, 0, sizeof(*entity));
    entity->x = spawn->x;
    entity->y = spawn->y;
    entity->homeX = spawn->x;
    entity->homeY = spawn->y;
    entity->scale = spawn->scale;
    entity->type = spawn->type;
    entity->textureId = textureForEntity(spawn->type);
    entity->health = 100.0f;
    entity->active = 1;

    switch (spawn->type) {
        case ENTITY_BARREL:
            entity->radius = 0.24f;
            entity->solid = 1;
            break;
        case ENTITY_PILLAR:
            entity->radius = 0.29f;
            entity->solid = 1;
            break;
        case ENTITY_SENTINEL:
            entity->radius = 0.24f;
            entity->solid = 1;
            break;
        case ENTITY_POWER_CELL:
        case ENTITY_EXIT:
        default:
            entity->radius = 0.16f;
            entity->solid = 0;
            break;
    }
}

static void interactWithWorld(Instance *instance)
{
    GameState *game = &instance->game;
    Player *player = &game->player;
    int entityIndex = gameFindInteractable(game);
    Entity *nearest = entityIndex >= 0 ? &game->entities[entityIndex] : NULL;

    player->useAnimation = 1.0f;
    if (!nearest) {
        setMessage(game, "NOTHING TO USE", 1.2f);
        return;
    }

    if (nearest->type == ENTITY_POWER_CELL) {
        nearest->active = 0;
        player->collectedCells++;
        player->flashlightPower = clampFloat(player->flashlightPower + 32.0f,
                                             0.0f, MAX_FLASHLIGHT_POWER);
        setMessage(game, "POWER CELL RECOVERED", 2.2f);
        return;
    }

    if (player->collectedCells < player->totalCells) {
        char message[96];
        int remaining = player->totalCells - player->collectedCells;
        snprintf(message, sizeof(message), "EXTRACTION LOCKED - %d CELL%s MISSING",
                 remaining, remaining == 1 ? "" : "S");
        setMessage(game, message, 2.4f);
        return;
    }

    game->mode = GAME_MODE_WON;
    setMouseCapture(0);
}

void gameRotatePlayer(Player *player, float radians)
{
    float sine;
    float cosine;
    float oldX;

    if (!player || fabsf(radians) < 0.000001f) return;
    sine = sinf(radians);
    cosine = cosf(radians);

    oldX = player->dirX;
    player->dirX = player->dirX * cosine - player->dirY * sine;
    player->dirY = oldX * sine + player->dirY * cosine;

    oldX = player->planeX;
    player->planeX = player->planeX * cosine - player->planeY * sine;
    player->planeY = oldX * sine + player->planeY * cosine;
}

int gameStart(Instance *instance, int mapIndex)
{
    GameState *game;
    const MapOption *option;
    int requestQuit;

    if (!instance || mapIndex < 0 || mapIndex >= NUM_MAPS) return -1;
    if (loadMapPreset(mapIndex) != 0) return -1;

    game = &instance->game;
    requestQuit = game->requestQuit;
    memset(game, 0, sizeof(*game));
    game->requestQuit = requestQuit;
    game->mode = GAME_MODE_PLAYING;
    game->selectedMap = mapIndex;
    game->showMinimap = 1;
    option = &mapOptions[mapIndex];

    game->player.x = option->spawnX;
    game->player.y = option->spawnY;
    game->player.dirX = option->dirX;
    game->player.dirY = option->dirY;
    game->player.planeX = -option->dirY * 0.66f;
    game->player.planeY = option->dirX * 0.66f;
    game->player.radius = PLAYER_RADIUS;
    game->player.health = MAX_HEALTH;
    game->player.stamina = MAX_STAMINA;
    game->player.flashlightPower = option->startsAtNight ? 78.0f : 100.0f;
    game->player.flashlightOn = option->startsAtNight;

    for (int i = 0; i < option->spawnCount && i < MAX_ENTITIES; ++i) {
        const EntitySpawn *spawn = &option->spawns[i];
        if (!mapCellIsOpen((int)spawn->x, (int)spawn->y)) {
            fprintf(stderr, "Ignoring entity on blocked tile at %.1f, %.1f\n",
                    spawn->x, spawn->y);
            continue;
        }
        configureEntity(&game->entities[game->entityCount], spawn);
        if (spawn->type == ENTITY_POWER_CELL) game->player.totalCells++;
        game->entityCount++;
    }

    if (circleTouchesWalls(game->player.x, game->player.y, game->player.radius)) {
        fprintf(stderr, "Map '%s' has an invalid player spawn\n", option->name);
        gameReturnToTitle(instance);
        return -1;
    }

    instance->isNight = option->startsAtNight;
    instance->daylight = instance->isNight ? 0.0f : 1.0f;
    setMessage(game, "RECOVER THE CELLS - REACH EXTRACTION", 4.0f);
    setMouseCapture(1);
    return 0;
}

void gameReturnToTitle(Instance *instance)
{
    int selectedMap;
    int requestQuit;

    if (!instance) return;
    selectedMap = instance->game.selectedMap;
    requestQuit = instance->game.requestQuit;
    memset(&instance->game, 0, sizeof(instance->game));
    instance->game.requestQuit = requestQuit;
    instance->game.selectedMap = selectedMap >= 0 && selectedMap < NUM_MAPS ? selectedMap : 0;
    instance->game.mode = GAME_MODE_TITLE;
    setMouseCapture(0);
}

void gameHandleEvent(Instance *instance, const SDL_Event *event)
{
    GameState *game;
    SDL_Keycode key;

    if (!instance || !event) return;
    game = &instance->game;

    if (event->type == SDL_QUIT) {
        game->requestQuit = 1;
        return;
    }
    if (event->type == SDL_WINDOWEVENT &&
        event->window.event == SDL_WINDOWEVENT_FOCUS_LOST &&
        game->mode == GAME_MODE_PLAYING) {
        game->mode = GAME_MODE_PAUSED;
        setMouseCapture(0);
        return;
    }
    if (event->type == SDL_MOUSEMOTION && game->mode == GAME_MODE_PLAYING) {
        gameRotatePlayer(&game->player,
                         (float)event->motion.xrel * MOUSE_SENSITIVITY);
        return;
    }
    if (event->type != SDL_KEYDOWN || event->key.repeat) return;

    key = event->key.keysym.sym;
    switch (game->mode) {
        case GAME_MODE_TITLE:
            if (key == SDLK_ESCAPE || key == SDLK_q) {
                game->requestQuit = 1;
            } else if (key == SDLK_UP || key == SDLK_LEFT) {
                game->selectedMap = (game->selectedMap - 1 + NUM_MAPS) % NUM_MAPS;
            } else if (key == SDLK_DOWN || key == SDLK_RIGHT) {
                game->selectedMap = (game->selectedMap + 1) % NUM_MAPS;
            } else if (key >= SDLK_1 && key <= SDLK_3) {
                game->selectedMap = (int)(key - SDLK_1);
                gameStart(instance, game->selectedMap);
            } else if (key == SDLK_RETURN || key == SDLK_SPACE) {
                gameStart(instance, game->selectedMap);
            }
            break;

        case GAME_MODE_PLAYING:
            if (key == SDLK_ESCAPE) {
                game->mode = GAME_MODE_PAUSED;
                setMouseCapture(0);
            } else if (key == SDLK_f) {
                if (game->player.flashlightPower > 0.0f) {
                    game->player.flashlightOn = !game->player.flashlightOn;
                    setMessage(game, game->player.flashlightOn ?
                               "TORCH ON" : "TORCH OFF", 1.0f);
                } else {
                    game->player.flashlightOn = 0;
                    setMessage(game, "TORCH BATTERY EMPTY", 1.8f);
                }
            } else if (key == SDLK_m) {
                game->showMinimap = !game->showMinimap;
            } else if (key == SDLK_e || key == SDLK_SPACE) {
                interactWithWorld(instance);
            } else if (key == SDLK_n) {
                instance->isNight = !instance->isNight;
                setMessage(game, instance->isNight ?
                           "NIGHT FALLING" : "DAWN BREAKING", 1.6f);
            } else if (key == SDLK_r) {
                gameStart(instance, game->selectedMap);
            }
            break;

        case GAME_MODE_PAUSED:
            if (key == SDLK_ESCAPE || key == SDLK_RETURN) {
                game->mode = GAME_MODE_PLAYING;
                setMouseCapture(1);
            } else if (key == SDLK_q) {
                gameReturnToTitle(instance);
            } else if (key == SDLK_r) {
                gameStart(instance, game->selectedMap);
            }
            break;

        case GAME_MODE_WON:
        case GAME_MODE_OVER:
            if (key == SDLK_RETURN || key == SDLK_ESCAPE) {
                gameReturnToTitle(instance);
            } else if (key == SDLK_r) {
                gameStart(instance, game->selectedMap);
            }
            break;
    }
}

static void updateMovement(GameState *game, const Uint8 *keys, float deltaTime)
{
    Player *player = &game->player;
    float forward = 0.0f;
    float strafe = 0.0f;
    float length;
    float speed;
    float moveX;
    float moveY;
    float oldX;
    float oldY;
    float distanceMoved;
    int wantsSprint;
    int sprinting;

    if (keys[SDL_SCANCODE_W]) forward += 1.0f;
    if (keys[SDL_SCANCODE_S]) forward -= 1.0f;
    if (keys[SDL_SCANCODE_D]) strafe += 1.0f;
    if (keys[SDL_SCANCODE_A]) strafe -= 1.0f;

    length = sqrtf(forward * forward + strafe * strafe);
    if (length > 1.0f) {
        forward /= length;
        strafe /= length;
    }

    wantsSprint = (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT]) &&
                  forward > 0.0f;
    if (player->stamina <= 0.5f) player->sprintExhausted = 1;
    if (player->stamina >= 30.0f) player->sprintExhausted = 0;
    sprinting = wantsSprint && !player->sprintExhausted;
    speed = sprinting ? SPRINT_SPEED : WALK_SPEED;

    moveX = (player->dirX * forward - player->dirY * strafe) * speed * deltaTime;
    moveY = (player->dirY * forward + player->dirX * strafe) * speed * deltaTime;
    oldX = player->x;
    oldY = player->y;

    if (fabsf(moveX) > 0.0f && playerPositionIsFree(game, player->x + moveX, player->y)) {
        player->x += moveX;
    }
    if (fabsf(moveY) > 0.0f && playerPositionIsFree(game, player->x, player->y + moveY)) {
        player->y += moveY;
    }

    distanceMoved = sqrtf((player->x - oldX) * (player->x - oldX) +
                          (player->y - oldY) * (player->y - oldY));
    if (sprinting && distanceMoved > 0.0001f) {
        player->stamina -= 23.0f * deltaTime;
    } else {
        player->stamina += 16.0f * deltaTime;
    }
    player->stamina = clampFloat(player->stamina, 0.0f, MAX_STAMINA);

    if (distanceMoved > 0.0001f) {
        float bobSpeed = sprinting ? 13.0f : 8.5f;
        player->bobPhase += bobSpeed * deltaTime;
        player->bobAmount += (1.0f - player->bobAmount) * 10.0f * deltaTime;
    } else {
        player->bobAmount += (0.0f - player->bobAmount) * 8.0f * deltaTime;
    }

    if (keys[SDL_SCANCODE_LEFT]) {
        gameRotatePlayer(player, -KEYBOARD_TURN_SPEED * deltaTime);
    }
    if (keys[SDL_SCANCODE_RIGHT]) {
        gameRotatePlayer(player, KEYBOARD_TURN_SPEED * deltaTime);
    }
}

static int sentinelIsLit(const Instance *instance, const Entity *entity, float distance)
{
    const Player *player = &instance->game.player;
    float dx;
    float dy;
    float dot;

    if (!player->flashlightOn || player->flashlightPower <= 0.0f || distance > 8.0f) {
        return 0;
    }
    dx = (entity->x - player->x) / distance;
    dy = (entity->y - player->y) / distance;
    dot = dx * player->dirX + dy * player->dirY;
    return dot > 0.93f &&
           hasLineOfSight(player->x, player->y, entity->x, entity->y);
}

static void moveSentinel(GameState *game, int entityIndex, float moveX, float moveY)
{
    Entity *entity = &game->entities[entityIndex];
    float nextX = entity->x + moveX;
    float nextY = entity->y + moveY;
    float playerClearance = entity->radius + game->player.radius + 0.08f;
    float playerDx;
    float playerDy;

    playerDx = nextX - game->player.x;
    playerDy = entity->y - game->player.y;
    if (playerDx * playerDx + playerDy * playerDy >= playerClearance * playerClearance &&
        !circleTouchesWalls(nextX, entity->y, entity->radius) &&
        !circleTouchesEntities(game, nextX, entity->y, entity->radius, entityIndex)) {
        entity->x = nextX;
    }
    playerDx = entity->x - game->player.x;
    playerDy = nextY - game->player.y;
    if (playerDx * playerDx + playerDy * playerDy >= playerClearance * playerClearance &&
        !circleTouchesWalls(entity->x, nextY, entity->radius) &&
        !circleTouchesEntities(game, entity->x, nextY, entity->radius, entityIndex)) {
        entity->y = nextY;
    }
}

static void updateSentinels(Instance *instance, float deltaTime)
{
    GameState *game = &instance->game;
    Player *player = &game->player;

    for (int i = 0; i < game->entityCount; ++i) {
        Entity *entity = &game->entities[i];
        float dx;
        float dy;
        float distance;
        int lit;

        if (!entity->active || entity->type != ENTITY_SENTINEL) continue;
        entity->animationTime += deltaTime;
        dx = player->x - entity->x;
        dy = player->y - entity->y;
        distance = sqrtf(dx * dx + dy * dy);
        if (distance < 0.001f) distance = 0.001f;
        lit = sentinelIsLit(instance, entity, distance);

        if (lit) {
            entity->health -= 30.0f * deltaTime;
            moveSentinel(game, i, -dx / distance * 0.34f * deltaTime,
                         -dy / distance * 0.34f * deltaTime);
            if (entity->health <= 0.0f) {
                entity->active = 0;
                setMessage(game, "SENTINEL NEUTRALIZED", 2.0f);
                continue;
            }
        } else if (distance < 8.5f &&
                   hasLineOfSight(entity->x, entity->y, player->x, player->y)) {
            moveSentinel(game, i, dx / distance * 0.62f * deltaTime,
                         dy / distance * 0.62f * deltaTime);
        }

        dx = player->x - entity->x;
        dy = player->y - entity->y;
        distance = sqrtf(dx * dx + dy * dy);
        if (distance < player->radius + entity->radius + 0.16f && !lit) {
            player->health -= 22.0f * deltaTime;
            player->damageFlash = 1.0f;
        }
    }
}

void gameUpdate(Instance *instance, const Uint8 *keys, float deltaTime)
{
    GameState *game;
    Player *player;

    if (!instance || !keys || instance->game.mode != GAME_MODE_PLAYING) return;
    game = &instance->game;
    player = &game->player;
    game->elapsedTime += deltaTime;

    {
        float targetDaylight = instance->isNight ? 0.0f : 1.0f;
        float transition = 0.38f * deltaTime;
        if (instance->daylight < targetDaylight) {
            instance->daylight = clampFloat(instance->daylight + transition,
                                            0.0f, targetDaylight);
        } else if (instance->daylight > targetDaylight) {
            instance->daylight = clampFloat(instance->daylight - transition,
                                            targetDaylight, 1.0f);
        }
    }

    updateMovement(game, keys, deltaTime);
    updateSentinels(instance, deltaTime);

    if (player->flashlightOn) {
        float drain = instance->isNight ? 0.78f : 0.48f;
        player->flashlightPower -= drain * deltaTime;
        if (player->flashlightPower <= 0.0f) {
            player->flashlightPower = 0.0f;
            player->flashlightOn = 0;
            setMessage(game, "TORCH BATTERY DEPLETED", 2.5f);
        }
    }

    if (player->damageFlash > 0.0f) {
        player->damageFlash = clampFloat(player->damageFlash - 2.7f * deltaTime, 0.0f, 1.0f);
    }
    if (player->useAnimation > 0.0f) {
        player->useAnimation = clampFloat(player->useAnimation - 2.8f * deltaTime, 0.0f, 1.0f);
    }
    if (game->messageTimer > 0.0f) {
        game->messageTimer -= deltaTime;
        if (game->messageTimer <= 0.0f) game->message[0] = '\0';
    }

    player->health = clampFloat(player->health, 0.0f, MAX_HEALTH);
    if (player->health <= 0.0f) {
        game->mode = GAME_MODE_OVER;
        setMouseCapture(0);
    }
}
