#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <SDL2/SDL.h>
#include "structures.h"

#define TRUE 1
#define FALSE 0

#define WINDOW_WIDTH 1260
#define WINDOW_HEIGHT 720
#define MAP_WIDTH 30
#define MAP_HEIGHT 30

#define FPS 30
#define FRAME_TARGET_TIME (1000 / FPS)

Vector2D player;
int game_is_running = FALSE;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;


/*********** Function Prototypes **************/
int initialize(void);
void destroy(void);
void setup(void);
void process_input(void);
void drawMap(void);
void drawAvatar(SDL_Renderer* renderer, int centerX, int centerY, int radius);
void render(void);
void update(void);

#endif
