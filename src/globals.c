#include "globals.h"


// Define global variables
Color m_colors[16] = {
    (Color){34, 35, 35, 255},      // COLOR_DARK_GRAY
    (Color){67, 67, 79, 255},      // COLOR_GRAY
    (Color){96, 96, 112, 255},     // COLOR_LIGHT_GRAY
    (Color){126, 126, 143, 255},   // COLOR_LIGHTER_GRAY
    (Color){194, 194, 209, 255},   // COLOR_WHITE
    (Color){140, 63, 93, 255},     // COLOR_DARK_RED
    (Color){186, 97, 86, 255},     // COLOR_RED
    (Color){235, 86, 75, 255},     // COLOR_ORANGE_RED
    (Color){255, 181, 181, 255},   // COLOR_PINK
    (Color){60, 163, 112, 255},    // COLOR_DARK_GREEN
    (Color){93, 222, 135, 255},    // COLOR_GREEN
    (Color){100, 118, 232, 255},   // COLOR_BLUE
    (Color){134, 167, 237, 255},   // COLOR_LIGHT_BLUE
    (Color){242, 166, 94, 255},    // COLOR_ORANGE
    (Color){255, 228, 120, 255},   // COLOR_YELLOW
    (Color){255, 255, 235, 255}    // COLOR_LIGHT_YELLOW
};

int enemySpawnVar = INITIAL_ENEMY_SPAWN_VAR; // Variable to control enemy spawn rate
int powerUpsCollected = 0; // Number of power-ups collected
int enemiesShot = 0; // Track number of enemies shot
float fireRateIncrease = 0.05f; // 5% increase in fire rate
