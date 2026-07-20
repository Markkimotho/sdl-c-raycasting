#ifndef RAYCAST_STRUCTURES_H
#define RAYCAST_STRUCTURES_H

#include <SDL2/SDL.h>

#include "config.h"

typedef enum GameMode {
    GAME_MODE_TITLE = 0,
    GAME_MODE_PLAYING,
    GAME_MODE_PAUSED,
    GAME_MODE_WON,
    GAME_MODE_OVER
} GameMode;

typedef enum EntityType {
    ENTITY_BARREL = 0,
    ENTITY_PILLAR,
    ENTITY_POWER_CELL,
    ENTITY_EXIT,
    ENTITY_SENTINEL
} EntityType;

typedef enum ObjectTextureId {
    OBJECT_TEXTURE_BARREL = 0,
    OBJECT_TEXTURE_PILLAR,
    OBJECT_TEXTURE_LIGHT,
    OBJECT_TEXTURE_SENTINEL,
    OBJECT_TEXTURE_POWER_CELL
} ObjectTextureId;

typedef struct TextureAsset {
    SDL_Texture *texture;
    int width;
    int height;
    int contentLeft;
    int contentTop;
    int contentRight;
    int contentBottom;
} TextureAsset;

typedef struct Player {
    float x;
    float y;
    float dirX;
    float dirY;
    float planeX;
    float planeY;
    float radius;

    float health;
    float stamina;
    float flashlightPower;
    float bobPhase;
    float bobAmount;
    float damageFlash;
    float useAnimation;
    int flashlightOn;
    int sprintExhausted;
    int collectedCells;
    int totalCells;
} Player;

typedef struct Entity {
    float x;
    float y;
    float homeX;
    float homeY;
    float radius;
    float scale;
    float health;
    float animationTime;
    EntityType type;
    ObjectTextureId textureId;
    int active;
    int solid;
} Entity;

typedef struct GameState {
    Player player;
    Entity entities[MAX_ENTITIES];
    int entityCount;
    int selectedMap;
    int showMinimap;
    int requestQuit;
    float elapsedTime;
    float messageTimer;
    char message[96];
    GameMode mode;
} GameState;

typedef struct Instance {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TextureAsset wallTextures[MAX_WALL_TYPES];
    TextureAsset objectTextures[MAX_OBJECT_TEXTURES];
    TextureAsset povHands;
    SDL_Texture *flashlightMask;
    SDL_Texture *torchGlow;
    SDL_Texture *vignette;
    float zBuffer[LOGICAL_WIDTH];
    char basePath[1024];
    int isNight;
    float daylight;
    GameState game;
} Instance;

#endif /* RAYCAST_STRUCTURES_H */
