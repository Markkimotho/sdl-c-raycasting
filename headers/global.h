#ifndef RAYCAST_GLOBAL_H
#define RAYCAST_GLOBAL_H

#include "maps.h"
#include "structures.h"

int initializeSDL(Instance *instance);
void cleanupSDL(Instance *instance);

int loadTextures(Instance *instance);
void destroyTextures(Instance *instance);

int gameStart(Instance *instance, int mapIndex);
void gameReturnToTitle(Instance *instance);
void gameHandleEvent(Instance *instance, const SDL_Event *event);
void gameUpdate(Instance *instance, const Uint8 *keys, float deltaTime);
void gameRotatePlayer(Player *player, float radians);
int gameFindInteractable(const GameState *game);

void renderDraw(Instance *instance);
void renderFrame(Instance *instance);

#endif /* RAYCAST_GLOBAL_H */
