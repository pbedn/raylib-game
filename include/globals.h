#ifndef GLOBALS_H
#define GLOBALS_H

#include "raylib.h" // Include raylib if needed

#define PLAYER_SPEED 200.0f
#define MAX_ENEMIES 1000
#define INITIAL_ENEMY_SPAWN_VAR 2 // Initial spawn chance for enemies
#define MAX_BULLETS 100
#define SHOOTING_RANGE 500.0f // Define the shooting range
#define WAVE_DURATION 30.0f

#define DEV_MODE

// Define enums
enum palette {
    COLOR_DARK_GRAY,
    COLOR_GRAY,
    COLOR_LIGHT_GRAY,
    COLOR_LIGHTER_GRAY,
    COLOR_WHITE,
    COLOR_DARK_RED,
    COLOR_RED,
    COLOR_ORANGE_RED,
    COLOR_PINK,
    COLOR_DARK_GREEN,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_LIGHT_BLUE,
    COLOR_ORANGE,
    COLOR_YELLOW,
    COLOR_LIGHT_YELLOW,
};

// Declare global variables
extern Color m_colors[]; // Declaration of the color array
extern int enemySpawnVar; // Variable to control enemy spawn rate
extern int powerUpsCollected; // Number of power-ups collected
extern int enemiesShot; // Track number of enemies shot
extern float fireRateIncrease; // 5% increase in fire rate

#endif // GLOBALS_H
