#include <stdio.h>
// #include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <stddef.h>

#include "raylib.h"
#include "raymath.h"

#define PLAYER_SPEED 200.0f
#define MAX_ENEMIES 1000
#define INITIAL_ENEMY_SPAWN_VAR 2 // Initial spawn chance for enemies
#define MAX_BULLETS 100
#define SHOOTING_RANGE 500.0f // Define the shooting range



typedef struct Player {
    Vector2 position;
    float radius;
    int health;
} Player;

typedef struct Enemy {
    Vector2 position;
    float radius;
    Vector2 direction;
} Enemy;

typedef struct Bullet {
    Vector2 position;
    Vector2 direction;
    float speed;
    float radius;
    bool active; // To check if the bullet is active
} Bullet;

typedef struct BulletManager {
    Bullet bullets[MAX_BULLETS]; // Array to hold bullets
    int bulletCount; // Current number of active bullets
    float lastShotTime; // Timer for shooting
    float bulletCooldown; // Time between shots
} BulletManager;

typedef struct PowerUp {
    Vector2 position;
    float radius;
    bool active;
} PowerUp;

// Define the palette enum
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

// Create an array of raylib Color structures
Color colors[16] = {
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

typedef enum {
    LOGO,
    MAIN_MENU,
    GAME,
    GAME_OVER
} GameState;

void InitPlayer(Player *player);
void InitBulletManager(BulletManager *bulletManager);
void InitEnemies(Enemy enemies[], int *enemyCount);
void SpawnEnemy(Enemy *enemy, int screenWidth, int screenHeight);
void UpdateEnemies(Enemy enemies[], int *enemyCount, Player *player, float deltaTime, int screenWidth, int screenHeight, int enemySpawnVar);
void DrawEnemies(Enemy enemies[], int enemyCount);
bool CheckCollision(Player *player, Enemy *enemy);
void UpdatePlayer(Player *player, float deltaTime, int screenWidth, int screenHeight);
void UpdateBullets(BulletManager *bulletManager, float deltaTime, int screenHeight);
void DrawBullets(BulletManager *bulletManager);
void FireBullet(Player *player, BulletManager *bulletManager, Enemy enemies[], int enemyCount, float screenWidth, float screenHeight, int powerUpsCollected, int *enemySpawnVar, float fireRateIncrease);
Enemy* FindClosestEnemy(Enemy enemies[], int enemyCount, Player *player);
void CheckBulletEnemyCollisions(BulletManager *bulletManager, Enemy enemies[], int *enemyCount, int *enemiesShot);
void SpawnPowerUp(PowerUp *powerUp, float screenWidth, float screenHeight, Player *player);
void CheckPowerUpCollection(Player *player, PowerUp *powerUp, int *powerUpsCollected, int *enemySpawnVar);
void DrawLogo();
void DrawMainMenu();
void DrawGame();
void DrawGameOver();

int main(void) {
    //
    /* Initialization: Set up the window and initialize game entities. */
    //
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Reverse Bullet Hell Survivor Roguelike");

    Player player;
    InitPlayer(&player);

    BulletManager bulletManager;
    InitBulletManager(&bulletManager);

    PowerUp powerUp; // Move power-up variable to main
    powerUp.active = false; // Initialize power-up as inactive

    int enemySpawnVar = INITIAL_ENEMY_SPAWN_VAR; // Variable to control enemy spawn rate
    int powerUpsCollected = 0; // Number of power-ups collected
    int enemiesShot = 0; // Track number of enemies shot
    float fireRateIncrease = 0.05f; // 5% increase in fire rate

    Enemy enemies[MAX_ENEMIES];
    int enemyCount;
    InitEnemies(enemies, &enemyCount);

    // Wave system variables
    float waveTimer = 0.0f;
    const float WAVE_DURATION = 30.0f;
    int currentWave = 1;

    GameState currentState = LOGO;
    float logoTimer = 0.0f;

    // SetTargetFPS(60); // Set the game to run at 60 frames-per-second

    //
    /* Game Loop: Continuously update and draw the game until the window is closed. */
    //
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        switch (currentState) {
            case LOGO:
                logoTimer += GetFrameTime();
                if (logoTimer >= 3.0f) { // Show logo for 3 seconds
                    currentState = MAIN_MENU;
                }
                break;
            case MAIN_MENU:
                if (IsKeyPressed(KEY_ONE)) {
                    currentState = GAME;
                }
                if (IsKeyPressed(KEY_TWO)) {
                    // Handle settings (not implemented)
                }
                if (IsKeyPressed(KEY_THREE)) {
                    CloseWindow(); // Exit the game
                }
                break;
            case GAME:
                // Game logic goes here
                // For demonstration, we'll simulate a game over after a key press
                if (IsKeyPressed(KEY_SPACE)) {
                    currentState = GAME_OVER;
                }
                break;
            case GAME_OVER:
                if (IsKeyPressed(KEY_R)) {
                    currentState = GAME; // Restart the game
                }
                if (IsKeyPressed(KEY_M)) {
                    currentState = MAIN_MENU; // Go to main menu
                }
                break;
        }

        // Draw
        switch (currentState) {
            case LOGO:
                DrawLogo();
                break;
            case MAIN_MENU:
                DrawMainMenu();
                break;
            case GAME:
                DrawGame();
                break;
            case GAME_OVER:
                DrawGameOver();
                break;
            }

        // //
        // /* Input Handling: Update player movement based on input. */
        // //
        // UpdatePlayer(&player, deltaTime, screenWidth, screenHeight);

        // //
        // /* Update Game State: Update the state of the player, enemies, bullets, and power-ups. */
        // //
        // UpdateBullets(&bulletManager, deltaTime, screenHeight);
        // FireBullet(&player, &bulletManager, enemies, enemyCount, screenWidth, screenHeight, powerUpsCollected, &enemySpawnVar, fireRateIncrease);
        // UpdateEnemies(enemies, &enemyCount, &player, deltaTime, screenWidth, screenHeight, enemySpawnVar);
        
        // //
        // /* Collision Detection: Check for collisions between bullets and enemies, and between the player and power-ups. */
        // //
        // CheckBulletEnemyCollisions(&bulletManager, enemies, &enemyCount, &enemiesShot); // Check for collisions
        // CheckPowerUpCollection(&player, &powerUp, &powerUpsCollected, &enemySpawnVar); // Check for power-up 

        // // Spawn power-up if conditions are met (e.g., every 10 enemies shot)
        // if (!powerUp.active && (enemiesShot != 0) && (enemiesShot % 10 == 0) * (powerUpsCollected + 1)) {
        //     SpawnPowerUp(&powerUp, screenWidth, screenHeight, &player);
        //     // enemiesShot = 0; // Reset enemies shot count after spawning power-up
        // }

        // // Spawn new enemies based on the updated enemy spawn variable
        // if (GetRandomValue(0, 100) < enemySpawnVar && enemyCount < MAX_ENEMIES) {
        //     SpawnEnemy(&enemies[enemyCount], screenWidth, screenHeight);
        //     enemyCount++;
        // }

        // // Wave system: update timer and end wave if needed
        // waveTimer += deltaTime;
        // if (waveTimer >= WAVE_DURATION)
        // {
        //     // End of wave: reset enemies, reward player and proceed to the next wave
        //     enemyCount = 0;
        //     player.health++; // Give player an extra health as a reward
        //     powerUp.active = false;  // Optionally, reset powerUp state or spawn a new one

        //     currentWave++;
        //     waveTimer = 0.0f;
        // }

        // // Check for Player death and restart game state if health <= 0
        // if (player.health <= 0)
        // {
        //     // Restart state: reinitialize all game elements
        //     InitPlayer(&player);
        //     InitBulletManager(&bulletManager);
        //     enemyCount = 0;
        //     InitEnemies(enemies, &enemyCount);
        //     powerUpsCollected = 0;
        //     enemiesShot = 0;
        //     powerUp.active = false;
        //     enemySpawnVar = INITIAL_ENEMY_SPAWN_VAR;
        //     // Optionally, reset wave system
        //     currentWave = 1;
        //     waveTimer = 0.0f;
        // }

        // //
        // /* Drawing: Render the player, enemies, bullets, and power-ups to the screen. */
        // //
        // BeginDrawing();
        // ClearBackground(BLACK);
        
        // DrawCircleV(player.position, player.radius, colors[COLOR_BLUE]);
        // DrawEnemies(enemies, enemyCount);
        // DrawBullets(&bulletManager);
        // DrawText("Use WASD to move", 10, 10, 20, colors[COLOR_LIGHTER_GRAY]);

        // // Draw player health at a fixed position
        // char healthText[32];
        // sprintf(healthText, "Health: %d", player.health);
        // DrawText(healthText, 10, 40, 20, colors[COLOR_WHITE]);

        // // Draw wave information centered at the top
        // char waveText[32];
        // sprintf(waveText, "Wave: %d", currentWave);
        // int textWidth = MeasureText(waveText, 20);
        // DrawText(waveText, (screenWidth - textWidth) / 2, 10, 20, colors[COLOR_WHITE]);

        // // Optionally, display the remaining time for the current wave (in seconds)
        // char timerText[32];
        // sprintf(timerText, "Time: %d", (int)(WAVE_DURATION - waveTimer));
        // int timerTextWidth = MeasureText(timerText, 20);
        // DrawText(timerText, (screenWidth - timerTextWidth) / 2, 40, 20, colors[COLOR_WHITE]);

        // // Draw wave information centered at the top
        // char enemiesText[32];
        // sprintf(waveText, "Enemies Killed: %d", enemiesShot);
        // int enemiesTextWidth = MeasureText(waveText, 20);
        // DrawText(waveText, (screenWidth - enemiesTextWidth) - 100, 10, 20, colors[COLOR_WHITE]);

        // if (powerUp.active) {
        //     DrawCircleV(powerUp.position, powerUp.radius, colors[COLOR_GREEN]); // Draw power-up
        // }

        EndDrawing();
    }

    //
    /* De-Initialization: Clean up resources and close the window. */
    //
    CloseWindow(); // Close window and OpenGL context

    return 0;
}


void InitPlayer(Player *player) {
    player->position = (Vector2){400, 300}; // Center of the screen
    player->radius = 20.0f;
    player->health = 10;
}

void InitBulletManager(BulletManager *bulletManager) {
    bulletManager->bulletCount = 0; // Initialize bullet count
    bulletManager->lastShotTime = 0.0f; // Reset shot timer
    bulletManager->bulletCooldown = 0.8f; // Set bullet cooldown
}

void InitEnemies(Enemy enemies[], int *enemyCount) {
    *enemyCount = 0; // Initialize enemy count
}

void UpdatePlayer(Player *player, float deltaTime, int screenWidth, int screenHeight) {
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) player->position.y -= PLAYER_SPEED * deltaTime;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) player->position.y += PLAYER_SPEED * deltaTime;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) player->position.x -= PLAYER_SPEED * deltaTime;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) player->position.x += PLAYER_SPEED * deltaTime;

    // Clamp player position to stay within screen boundaries
    player->position.x = Clamp(player->position.x, player->radius, screenWidth - player->radius);
    player->position.y = Clamp(player->position.y, player->radius, screenHeight - player->radius);

    // // Auto-fire bullets
    // FireBullet(&player, &bulletManager);
}

void SpawnEnemy(Enemy *enemy, int screenWidth, int screenHeight) {
    enemy->radius = 15.0f;
    enemy->direction = (Vector2){0, 0};

    int edge = GetRandomValue(0, 3); // 0: top, 1: bottom, 2: left, 3: right
    switch (edge) {
        case 0: // Top
            enemy->position = (Vector2){GetRandomValue(0, screenWidth), 0};
            enemy->direction = (Vector2){0, 1}; // Move down
            break;
        case 1: // Bottom
            enemy->position = (Vector2){GetRandomValue(0, screenWidth), screenHeight};
            enemy->direction = (Vector2){0, -1}; // Move up
            break;
        case 2: // Left
            enemy->position = (Vector2){0, GetRandomValue(0, screenHeight)};
            enemy->direction = (Vector2){1, 0}; // Move right
            break;
        case 3: // Right
            enemy->position = (Vector2){screenWidth, GetRandomValue(0, screenHeight)};
            enemy->direction = (Vector2){-1, 0}; // Move left
            break;
    }
}

void UpdateEnemies(Enemy enemies[], int *enemyCount, Player *player, float deltaTime, int screenWidth, int screenHeight, int enemySpawnVar) {
    for (int i = 0; i < *enemyCount; i++) {
        // Calculate direction vector from enemy to player
        Vector2 direction = (Vector2){player->position.x - enemies[i].position.x, player->position.y - enemies[i].position.y};

        // Normalize the direction vector
        direction = Vector2Normalize(direction);

        // Move the enemy towards the player
        enemies[i].position.x += direction.x * 100.0f * deltaTime; // Adjust speed as needed
        enemies[i].position.y += direction.y * 100.0f * deltaTime; // Adjust speed as needed

        // Clamp enemy position to stay within screen boundaries
        enemies[i].position.x = Clamp(enemies[i].position.x, enemies[i].radius, screenWidth - enemies[i].radius);
        enemies[i].position.y = Clamp(enemies[i].position.y, enemies[i].radius, screenHeight - enemies[i].radius);

        // Check for collision with player
        if (CheckCollision(player, &enemies[i])) {
            // Decrease player's health
            player->health--;
            
            // Remove enemy by shifting the rest of the array
            for (int j = i; j < *enemyCount - 1; j++) {
                enemies[j] = enemies[j + 1];
            }
            (*enemyCount)--;
            i--; // Adjust index after removal
        }
    }

    // Spawn new enemies periodically
    if (GetRandomValue(0, 100) < enemySpawnVar && *enemyCount < MAX_ENEMIES) {
        SpawnEnemy(&enemies[*enemyCount], screenWidth, screenHeight);
        (*enemyCount)++;
    }
}

void DrawEnemies(Enemy enemies[], int enemyCount) {
    for (int i = 0; i < enemyCount; i++) {
        DrawCircleV(enemies[i].position, enemies[i].radius, colors[COLOR_ORANGE_RED]);
    }
}

bool CheckCollision(Player *player, Enemy *enemy) {
    return CheckCollisionCircles(player->position, player->radius, enemy->position, enemy->radius);
}

void UpdateBullets(BulletManager *bulletManager, float deltaTime, int screenHeight) {
    for (int i = 0; i < bulletManager->bulletCount; i++) {
        Bullet *bullet = &bulletManager->bullets[i];
        if (bullet->active) {
            // Move the bullet
            bullet->position.x += bullet->direction.x * bullet->speed * deltaTime;
            bullet->position.y += bullet->direction.y * bullet->speed * deltaTime;

            // Check if the bullet is off-screen
            if (bullet->position.y < 0) {
                bullet->active = false; // Deactivate bullet
            }
        }
    }

    // Remove inactive bullets
    for (int i = 0; i < bulletManager->bulletCount; i++) {
        if (!bulletManager->bullets[i].active) {
            // Shift the rest of the array
            for (int j = i; j < bulletManager->bulletCount - 1; j++) {
                bulletManager->bullets[j] = bulletManager->bullets[j + 1];
            }
            bulletManager->bulletCount--; // Decrease bullet count
            i--; // Adjust index after removal
        }
    }
}

void FireBullet(Player *player, BulletManager *bulletManager, Enemy enemies[], int enemyCount, float screenWidth, float screenHeight, int powerUpsCollected, int *enemySpawnVar, float fireRateIncrease)
{
    bulletManager->lastShotTime += GetFrameTime();
    float effectiveBulletCooldown = bulletManager->bulletCooldown * (1.0f - (powerUpsCollected * fireRateIncrease));

    if (bulletManager->lastShotTime >= effectiveBulletCooldown) {
        // Find the closest enemy
        Enemy *closestEnemy = FindClosestEnemy(enemies, enemyCount, player);

        if (closestEnemy != NULL) {
            // Check for available bullet slot
            if (bulletManager->bulletCount < MAX_BULLETS) {
                Bullet *newBullet = &bulletManager->bullets[bulletManager->bulletCount++];
                newBullet->position = player->position; // Start at player's position

                // Calculate direction towards the closest enemy
                Vector2 direction = Vector2Subtract(closestEnemy->position, player->position);
                newBullet->direction = Vector2Normalize(direction); // Normalize the direction

                newBullet->speed = PLAYER_SPEED * 4; // Set bullet speed
                newBullet->radius = 5.0f; // Set bullet radius
                newBullet->active = true; // Mark bullet as active
                bulletManager->lastShotTime = 0.0f; // Reset shot timer
            }
        }
    }
}

void DrawBullets(BulletManager *bulletManager) {
    for (int i = 0; i < bulletManager->bulletCount; i++) {
        Bullet *bullet = &bulletManager->bullets[i];
        if (bullet->active) {
            DrawCircleV(bullet->position, bullet->radius, colors[COLOR_LIGHT_YELLOW]); // Draw bullet
        }
    }
}

Enemy* FindClosestEnemy(Enemy enemies[], int enemyCount, Player *player) {
    Enemy *closestEnemy = NULL;
    float closestDistance = SHOOTING_RANGE;

    for (int i = 0; i < enemyCount; i++) {
        float distance = Vector2Distance(player->position, enemies[i].position);
        if (distance < closestDistance) {
            closestDistance = distance;
            closestEnemy = &enemies[i];
        }
    }

    return closestEnemy; // Returns NULL if no enemy is within range
}

void CheckBulletEnemyCollisions(BulletManager *bulletManager, Enemy enemies[], int *enemyCount, int *enemiesShot) {
    for (int i = 0; i < bulletManager->bulletCount; i++) {
        Bullet *bullet = &bulletManager->bullets[i];
        if (bullet->active) {
            for (int j = 0; j < *enemyCount; j++) {
                Enemy *enemy = &enemies[j];
                if (CheckCollisionCircles(bullet->position, bullet->radius, enemy->position, enemy->radius)) {
                    // Collision detected
                    bullet->active = false; // Deactivate the bullet

                    // Increment enemiesShot
                    (*enemiesShot)++;

                    // Remove the enemy by shifting the rest of the array
                    for (int k = j; k < *enemyCount - 1; k++) {
                        enemies[k] = enemies[k + 1];
                    }
                    (*enemyCount)--; // Decrease enemy count
                    break; // Exit the inner loop since the bullet is now inactive
                }
            }
        }
    }
}

void SpawnPowerUp(PowerUp *powerUp, float screenWidth, float screenHeight, Player *player) {
    const float MIN_DISTANCE_FROM_PLAYER = 100.0f; // Minimum distance from player

    do {
        powerUp->position = (Vector2){GetRandomValue(50, screenWidth - 50), GetRandomValue(50, screenHeight - 50)};
    } while (Vector2Distance(powerUp->position, player->position) < MIN_DISTANCE_FROM_PLAYER);

    powerUp->radius = 15.0f; // Set power-up radius
    powerUp->active = true; // Activate power-up
}

void CheckPowerUpCollection(Player *player, PowerUp *powerUp, int *powerUpsCollected, int *enemySpawnVar) {
    if (powerUp->active && CheckCollisionCircles(player->position, player->radius, powerUp->position, powerUp->radius)) {
        (*powerUpsCollected)++; // Increase power-ups collected
        powerUp->active = false; // Deactivate power-up
        *enemySpawnVar += 1; // Increase enemy spawn variable
    }
}


void DrawLogo() {
    ClearBackground(colors[COLOR_DARK_GRAY]);
    DrawText("My Game Logo", GetScreenWidth() / 2 - MeasureText("My Game Logo", 20) / 2, GetScreenHeight() / 2 - 10, 20, colors[COLOR_WHITE]);
    DrawText("Created by Your Name", GetScreenWidth() / 2 - MeasureText("Created by Your Name", 20) / 2, GetScreenHeight() / 2 + 20, 20, colors[COLOR_WHITE]);
}

void DrawMainMenu() {
    ClearBackground(colors[COLOR_DARK_GRAY]);
    DrawText("Main Menu", GetScreenWidth() / 2 - MeasureText("Main Menu", 40) / 2, GetScreenHeight() / 2 - 40, 40, colors[COLOR_WHITE]);
    DrawText("1. Play Game", GetScreenWidth() / 2 - MeasureText("1. Play Game", 20) / 2, GetScreenHeight() / 2, 20, colors[COLOR_WHITE]);
    DrawText("2. Settings", GetScreenWidth() / 2 - MeasureText("2. Settings", 20) / 2, GetScreenHeight() / 2 + 30, 20, colors[COLOR_WHITE]);
    DrawText("3. Exit", GetScreenWidth() / 2 - MeasureText("3. Exit", 20) / 2, GetScreenHeight() / 2 + 60, 20, colors[COLOR_WHITE]);
}

void DrawGame() {
    ClearBackground(colors[COLOR_DARK_GRAY]);
    DrawText("Playing the Game!", GetScreenWidth() / 2 - MeasureText("Playing the Game!", 20) / 2, GetScreenHeight() / 2, 20, colors[COLOR_WHITE]);
}

void DrawGameOver() {
    ClearBackground(colors[COLOR_DARK_GRAY]);
    DrawText("Game Over", GetScreenWidth() / 2 - MeasureText("Game Over", 40) / 2, GetScreenHeight() / 2 - 40, 40, colors[COLOR_WHITE]);
    DrawText("Press R to Restart or M to go to Main Menu", GetScreenWidth() / 2 - MeasureText("Press R to Restart or M to go to Main Menu", 20) / 2, GetScreenHeight() / 2, 20, DARKGRAY);
}
