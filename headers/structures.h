#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_


/**
 * struct Vector2D - Defines an (x, y) point on the window
 * @x: x coordinate
 * @y: y coordinate
 *
 */
typedef struct Vector2D
{
	float x;
	float y;
} Vector2D;

/**
 * struct Player - Defines a player's position, direction
 * 	           & angle they are facing;
 * @pos: Position of player
 * @dir: Direction player is facing
 * @angle: Angle player is facing
 *
 */
typedef struct Player
{
	Vector2D pos;
	Vector2D dir;
	float angle;
} Player;


#endif
