#include <float.h>
#include <stddef.h>

#include "raylib.h"
#include "raymath.h"

// #include "TracyC.h"

#define TRACY_ENABLE
#define PLAYER_SPEED 200.0f
#define MAX_ENEMIES 1000
#define MAX_BULLETS 100
#define SHOOTING_RANGE 500.0f // Define the shooting range

typedef struct Player {
    Vector2 position;
    float radius;
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

void InitPlayer(Player *player);
void InitBulletManager(BulletManager *bulletManager);
void InitEnemies(Enemy enemies[], int *enemyCount);
void SpawnEnemy(Enemy *enemy, int screenWidth, int screenHeight);
void UpdateEnemies(Enemy enemies[], int *enemyCount, Player *player, float deltaTime, int screenWidth, int screenHeight);
void DrawEnemies(Enemy enemies[], int enemyCount);
bool CheckCollision(Player *player, Enemy *enemy);
void UpdatePlayer(Player *player, float deltaTime, int screenWidth, int screenHeight);
void UpdateBullets(BulletManager *bulletManager, float deltaTime, int screenHeight);
void DrawBullets(BulletManager *bulletManager);
void FireBullet(Player *player, BulletManager *bulletManager, Enemy enemies[], int enemyCount);
Enemy* FindClosestEnemy(Enemy enemies[], int enemyCount, Player *player);


int main(void) {
    // TracyCAlloc(x, y);

    // Initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Vampire Survivors Clone");

    Player player;
    InitPlayer(&player);

    Bullet bullets[MAX_BULLETS]; // Array to hold bullets
    BulletManager bulletManager; // Declare bullet manager
    InitBulletManager(&bulletManager); // Initialize bullet manager

    

    Enemy enemies[MAX_ENEMIES]; // Static Allocation
    // Enemy *enemies = malloc(sizeof(Enemy) * MAX_ENEMIES); // Dynamic Allocation
    int enemyCount;
    InitEnemies(enemies, &enemyCount);

    SetTargetFPS(60); // Set the game to run at 60 frames-per-second

    // Main game loop
    while (!WindowShouldClose()) {

        // Update
        float deltaTime = GetFrameTime();
        UpdatePlayer(&player, deltaTime, screenWidth, screenHeight);
        UpdateBullets(&bulletManager, deltaTime, screenHeight);
        FireBullet(&player, &bulletManager, enemies, enemyCount);
        UpdateEnemies(enemies, &enemyCount, &player, deltaTime, screenWidth, screenHeight);

        // Drawing Section
        BeginDrawing();
        ClearBackground(BLACK);
        
        DrawCircleV(player.position, player.radius, BLUE);
        DrawEnemies(enemies, enemyCount);
        DrawBullets(&bulletManager);
        DrawText("Use WASD to move", 10, 10, 20, DARKGRAY);
        
        EndDrawing();
    }

    // De-Initialization
    // free(enemies);
    CloseWindow(); // Close window and OpenGL context

    return 0;
}


void InitPlayer(Player *player) {
    player->position = (Vector2){400, 300}; // Center of the screen
    player->radius = 20.0f;
}

void InitBulletManager(BulletManager *bulletManager) {
    bulletManager->bulletCount = 0; // Initialize bullet count
    bulletManager->lastShotTime = 0.0f; // Reset shot timer
    bulletManager->bulletCooldown = 0.2f; // Set bullet cooldown
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

void UpdateEnemies(Enemy enemies[], int *enemyCount, Player *player, float deltaTime, int screenWidth, int screenHeight) {
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
            // Remove enemy by shifting the rest of the array
            for (int j = i; j < *enemyCount - 1; j++) {
                enemies[j] = enemies[j + 1];
            }
            (*enemyCount)--;
            i--; // Adjust index after removal
        }
    }

    // Spawn new enemies periodically
    if (GetRandomValue(0, 100) < 2 && *enemyCount < MAX_ENEMIES) {
        SpawnEnemy(&enemies[*enemyCount], screenWidth, screenHeight);
        (*enemyCount)++;
    }
}

void DrawEnemies(Enemy enemies[], int enemyCount) {
    for (int i = 0; i < enemyCount; i++) {
        DrawCircleV(enemies[i].position, enemies[i].radius, RED);
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

void FireBullet(Player *player, BulletManager *bulletManager, Enemy enemies[], int enemyCount) {
    bulletManager->lastShotTime += GetFrameTime();
    if (bulletManager->lastShotTime >= bulletManager->bulletCooldown) {
        // Find the closest enemy
        Enemy *closestEnemy = FindClosestEnemy(enemies, enemyCount, player);

        if (closestEnemy != NULL) {
            // Check for available bullet slot
            if (bulletManager->bulletCount < MAX_BULLETS) {
                Bullet *newBullet = &bulletManager->bullets[bulletManager->bulletCount++];
                newBullet->position = player->position; // Start at player's position

                // newBullet->direction = (Vector2){0, -1}; // Fire upwards

                // Calculate direction towards the closest enemy
                Vector2 direction = Vector2Subtract(closestEnemy->position, player->position);
                newBullet->direction = Vector2Normalize(direction); // Normalize the direction

                newBullet->speed = 400.0f; // Set bullet speed
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
            DrawCircleV(bullet->position, bullet->radius, YELLOW); // Draw bullet
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
