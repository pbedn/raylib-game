#include "raylib.h"
#include "raymath.h"

// #include "TracyC.h"

#define TRACY_ENABLE
#define PLAYER_SPEED 200.0f
#define MAX_ENEMIES 1000

typedef struct Player {
    Vector2 position;
    float radius;
} Player;

typedef struct Enemy {
    Vector2 position;
    float radius;
    Vector2 direction;
} Enemy;

void InitPlayer(Player *player);
void InitEnemies(Enemy enemies[], int *enemyCount);
void SpawnEnemy(Enemy *enemy, int screenWidth, int screenHeight);
void UpdateEnemies(Enemy enemies[], int *enemyCount, Player *player, float deltaTime, int screenWidth, int screenHeight);
void DrawEnemies(Enemy enemies[], int enemyCount);
bool CheckCollision(Player *player, Enemy *enemy);
void UpdatePlayer(Player *player, float deltaTime, int screenWidth, int screenHeight);

int main(void) {
    // TracyCAlloc(x, y);

    // Initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Vampire Survivors Clone");

    Player player;
    InitPlayer(&player);

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
        UpdateEnemies(enemies, &enemyCount, &player, deltaTime, screenWidth, screenHeight);

        // Drawing Section
        BeginDrawing();
        ClearBackground(BLACK);
        
        DrawCircleV(player.position, player.radius, BLUE);
        DrawEnemies(enemies, enemyCount);
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

