#include <stdio.h>
#include <string.h>

#include "../headers/maps.h"

int map[MAP_HEIGHT][MAP_WIDTH];

static const int mapClassic[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,2,2,2,0,3,3,3,3,0,4,4,0,1},
    {1,0,2,0,0,0,0,3,0,0,0,0,4,0,0,1},
    {1,0,2,0,5,5,5,3,0,5,5,0,4,0,5,1},
    {1,0,2,0,5,0,5,3,0,5,0,0,4,0,5,1},
    {1,0,2,0,5,0,5,3,0,5,0,3,4,0,5,1},
    {1,0,2,0,5,5,5,3,0,5,5,3,4,0,5,1},
    {1,0,2,0,0,0,0,3,0,0,0,3,4,0,0,1},
    {1,0,2,2,2,2,2,3,3,0,0,3,4,4,4,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,2,3,3,4,4,5,5,0,0,0,0,0,1},
    {1,0,2,0,3,0,4,0,5,0,0,0,0,0,0,1},
    {1,0,2,3,3,4,4,5,5,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

static const int mapArena[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,2,2,2,2,2,2,2,2,2,2,2,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,2,2,2,2,2,2,2,2,2,2,2,2,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

static const int mapDungeon[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,4,0,0,0,0,4,0,0,0,0,1},
    {1,0,3,3,0,4,0,3,3,0,4,0,3,3,0,1},
    {1,0,3,3,0,4,0,3,3,0,4,0,3,3,0,1},
    {1,0,3,3,0,5,0,3,3,0,5,0,3,3,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,2,0,2,0,2,2,0,2,0,2,2,0,1},
    {1,0,2,2,0,2,0,2,2,0,2,0,2,2,0,1},
    {1,0,2,2,0,2,0,2,2,0,2,0,2,2,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,5,5,0,3,0,5,5,0,3,0,5,5,0,1},
    {1,0,5,5,0,3,0,5,5,0,3,0,5,5,0,1},
    {1,0,5,5,0,4,0,5,5,0,4,0,5,5,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

static const EntitySpawn classicSpawns[] = {
    {ENTITY_BARREL,     6.5f,  1.5f, 0.58f},
    {ENTITY_BARREL,    10.5f, 10.5f, 0.58f},
    {ENTITY_PILLAR,    13.5f,  3.5f, 0.95f},
    {ENTITY_POWER_CELL,14.5f,  3.5f, 0.42f},
    {ENTITY_POWER_CELL, 9.5f, 10.5f, 0.42f},
    {ENTITY_POWER_CELL, 1.5f, 14.5f, 0.42f},
    {ENTITY_SENTINEL,  12.5f, 10.5f, 0.92f},
    {ENTITY_SENTINEL,  10.5f, 13.5f, 0.92f},
    {ENTITY_EXIT,      14.5f, 14.5f, 0.80f}
};

static const EntitySpawn arenaSpawns[] = {
    {ENTITY_BARREL,      5.5f,  5.5f, 0.60f},
    {ENTITY_BARREL,     10.5f, 10.5f, 0.60f},
    {ENTITY_PILLAR,      8.5f,  5.5f, 1.00f},
    {ENTITY_POWER_CELL,  4.5f,  4.5f, 0.42f},
    {ENTITY_POWER_CELL, 11.5f,  4.5f, 0.42f},
    {ENTITY_POWER_CELL,  4.5f, 11.5f, 0.42f},
    {ENTITY_SENTINEL,    8.5f, 11.5f, 0.94f},
    {ENTITY_SENTINEL,   11.5f,  8.5f, 0.94f},
    {ENTITY_EXIT,       11.5f, 11.5f, 0.82f}
};

static const EntitySpawn dungeonSpawns[] = {
    {ENTITY_BARREL,      4.5f,  5.5f, 0.58f},
    {ENTITY_BARREL,     12.5f,  9.5f, 0.58f},
    {ENTITY_PILLAR,      9.5f, 13.5f, 0.95f},
    {ENTITY_POWER_CELL,  6.5f,  5.5f, 0.42f},
    {ENTITY_POWER_CELL, 14.5f,  9.5f, 0.42f},
    {ENTITY_POWER_CELL,  6.5f, 13.5f, 0.42f},
    {ENTITY_SENTINEL,    8.5f,  5.5f, 0.92f},
    {ENTITY_SENTINEL,    7.5f,  9.5f, 0.92f},
    {ENTITY_EXIT,       14.5f, 14.5f, 0.80f}
};

#define ARRAY_COUNT(values) ((int)(sizeof(values) / sizeof((values)[0])))

const MapOption mapOptions[NUM_MAPS] = {
    {"CLASSIC MAZE", mapClassic, 1.5f, 1.5f, 1.0f, 0.0f, 1,
     classicSpawns, ARRAY_COUNT(classicSpawns)},
    {"INNER YARD", mapArena, 8.0f, 8.0f, 0.0f, 1.0f, 0,
     arenaSpawns, ARRAY_COUNT(arenaSpawns)},
    {"THE DUNGEON", mapDungeon, 2.5f, 1.5f, 1.0f, 0.0f, 1,
     dungeonSpawns, ARRAY_COUNT(dungeonSpawns)}
};

int mapCellIsOpen(int x, int y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return 0;
    }
    return map[y][x] == 0;
}

int loadMapPreset(int mapIndex)
{
    if (mapIndex < 0 || mapIndex >= NUM_MAPS) {
        fprintf(stderr, "Invalid map index: %d\n", mapIndex);
        return -1;
    }

    memcpy(map, mapOptions[mapIndex].layout, sizeof(map));
    return 0;
}
