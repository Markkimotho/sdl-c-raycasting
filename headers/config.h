#ifndef RAYCAST_CONFIG_H
#define RAYCAST_CONFIG_H

/* The renderer works at a stable logical resolution and SDL scales it to the
 * resizable desktop window. This keeps ray and HUD costs deterministic. */
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define LOGICAL_WIDTH 1280
#define LOGICAL_HEIGHT 720

#define MAP_WIDTH 16
#define MAP_HEIGHT 16
#define NUM_MAPS 3

#define MAX_WALL_TYPES 6
#define MAX_OBJECT_TEXTURES 5
#define MAX_ENTITIES 64

#define FIXED_TIMESTEP (1.0f / 120.0f)
#define MAX_FRAME_TIME 0.1f
#define MIN_RAY_DISTANCE 0.0001f

#define PLAYER_RADIUS 0.22f
#define WALK_SPEED 2.35f
#define SPRINT_SPEED 4.1f
#define KEYBOARD_TURN_SPEED 2.25f
#define MOUSE_SENSITIVITY 0.0022f

#define MAX_HEALTH 100.0f
#define MAX_STAMINA 100.0f
#define MAX_FLASHLIGHT_POWER 100.0f

#endif /* RAYCAST_CONFIG_H */
