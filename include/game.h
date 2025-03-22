#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include "raylib.h"
#include "raymath.h"
#include "resource_dir.h"
#include "globals.h"

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

typedef struct {
    Player *player;
    BulletManager *bulletManager;
    Enemy enemies[MAX_ENEMIES];
    int *enemyCount;
    PowerUp *powerUp;
    int *powerUpsCollected;
    int *enemiesShot;
    int *enemySpawnVar;
    float deltaTime;
    float *waveTimer;
    int *currentWave;
    int *hitEnemyIndex;
    Shader hitShader;
    bool isGamePaused;
} GameLogicParams;

typedef enum {
    LOGO,
    MAIN_MENU,
    GAME,
    GAME_OVER
} Scene;


const char* SceneToString(Scene scene);
void GameLogic(GameLogicParams *params);
void InitPlayer(Player *player);
void InitBulletManager(BulletManager *bulletManager);
void InitEnemies(int *enemyCount);
void SpawnEnemy(Enemy *enemy);
void UpdateEnemies(Enemy enemies[], int *enemyCount, Player *player, float deltaTime, int enemySpawnVar);
void DrawEnemies(Enemy enemies[], int enemyCount, Shader hitShader, int hitEnemyIndex);
bool CheckCollision(Player *player, Enemy *enemy);
void UpdatePlayer(Player *player, float deltaTime);
void UpdateBullets(BulletManager *bulletManager, float deltaTime);
void DrawBullets(BulletManager *bulletManager);
void FireBullet(Player *player, BulletManager *bulletManager, Enemy enemies[], int enemyCount, int powerUpsCollected, float fireRateIncrease);
Enemy* FindClosestEnemy(Enemy enemies[], int enemyCount, Player *player);
void CheckBulletEnemyCollisions(BulletManager *bulletManager, Enemy enemies[], int *enemyCount, int *enemiesShot, int *hitEnemyIndex);
void SpawnPowerUp(PowerUp *powerUp, Player *player);
void CheckPowerUpCollection(Player *player, PowerUp *powerUp, int *powerUpsCollected, int *enemySpawnVar);
void DrawLogo();
void DrawMainMenu();
void DrawGame(GameLogicParams *params);
void DrawGameOver();
void ExitGameplay(GameLogicParams *gameParams);