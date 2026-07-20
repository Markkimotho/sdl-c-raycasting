#ifndef RAYCAST_MAPS_H
#define RAYCAST_MAPS_H

#include "structures.h"

typedef struct EntitySpawn {
    EntityType type;
    float x;
    float y;
    float scale;
} EntitySpawn;

typedef struct MapOption {
    const char *name;
    const int (*layout)[MAP_WIDTH];
    float spawnX;
    float spawnY;
    float dirX;
    float dirY;
    int startsAtNight;
    const EntitySpawn *spawns;
    int spawnCount;
} MapOption;

extern int map[MAP_HEIGHT][MAP_WIDTH];
extern const MapOption mapOptions[NUM_MAPS];

int loadMapPreset(int mapIndex);
int mapCellIsOpen(int x, int y);

#endif /* RAYCAST_MAPS_H */
