#include "game.h"
#include "globals.h"


void InitGameParams(GameLogicParams *params) {
    static Player player;
    player.position = (Vector2){400, 300}; // Center of the screen
    player.radius = 20.0f;
    player.health = 10;

    static BulletManager bulletManager;
    bulletManager.bulletCount = 0; // Initialize bullet count
    bulletManager.lastShotTime = 0.0f; // Reset shot timer
    bulletManager.bulletCooldown = 0.8f; // Set bullet cooldown

    static PowerUp powerUp;
    powerUp.active = false;

    // Initialize enemies array
    for (int i = 0; i < MAX_ENEMIES; i++) {
        params->enemies[i] = (Enemy){0}; // Initialize each Enemy to zero
    }
    static int enemyCount = 0;
    static int hitEnemyIndex = -1; // Initialize to -1 (no hit)

    // Wave system variables
    static float waveTimer = 0.0f;
    static int currentWave = 1;

    Shader hitShader = LoadShader(0, "hit_shader.fs");    

    params->player = &player;
    params->bulletManager = &bulletManager;
    // params->enemies;
    // for (int i = 0; i < MAX_ENEMIES; i++) {
    //     params->enemies[i] = (Enemy){0}; // Initialize each Enemy to zero
    // }
    params->enemyCount = &enemyCount;
    params->powerUp = &powerUp;
    params->powerUpsCollected = &powerUpsCollected;
    params->enemiesShot = &enemiesShot;
    params->enemySpawnVar = &enemySpawnVar;
    params->deltaTime = 0.0f;
    params->waveTimer = &waveTimer;
    params->currentWave = &currentWave;
    params->hitEnemyIndex = &hitEnemyIndex;
    params->hitShader = hitShader;
    params->isGamePaused = false;
    TraceLog(LOG_DEBUG, "Init Game Params");
}

void GameLogic(GameLogicParams *params) {
    //
    /* Input Handling: Update player movement based on input. */
    //
    UpdatePlayer(params->player, params->deltaTime);

    //
    /* Update Game State: Update the state of the player, enemies, bullets, and power-ups. */
    //
    UpdateBullets(params->bulletManager, params->deltaTime);
    FireBullet(params->player, params->bulletManager, params->enemies, *(params->enemyCount), *(params->powerUpsCollected), 0.05f);
    UpdateEnemies(params);
    
    //
    /* Collision Detection: Check for collisions between bullets and enemies, and between the player and power-ups. */
    //
    // Collision Detection
    CheckBulletEnemyCollisions(params->bulletManager, params->enemies, params->enemyCount, params->enemiesShot, params->hitEnemyIndex);
    CheckPowerUpCollection(params->player, params->powerUp, params->powerUpsCollected, params->enemySpawnVar);

    // Spawn power-up if conditions are met
    if (!params->powerUp->active && (*(params->enemiesShot) != 0) && (*(params->enemiesShot) % 10 == 0)) {
        SpawnPowerUp(params->powerUp, params->player);
    }

    // Spawn new enemies based on the updated enemy spawn variable
    if (GetRandomValue(0, 100) < *(params->enemySpawnVar) && *(params->enemyCount) < MAX_ENEMIES) {
        SpawnEnemy(params);
    }

    // Wave system: update timer and end wave if needed
    *(params->waveTimer) += params->deltaTime;
    if (*(params->waveTimer) >= WAVE_DURATION) {
        *(params->enemyCount) = 0;
        params->player->health++;
        params->powerUp->active = false;
        (*(params->currentWave))++;
        *(params->waveTimer) = 0.0f;
    }

    // Check for Player death and restart game state if health <= 0
    if (params->player->health <= 0) {
        InitPlayer(params->player);
        InitBulletManager(params->bulletManager);
        *(params->enemyCount) = 0;
        *params->enemyCount = 0;
        *(params->powerUpsCollected) = 0;
        *(params->enemiesShot) = 0;
        params->powerUp->active = false;
        *(params->enemySpawnVar) = INITIAL_ENEMY_SPAWN_VAR;
        *(params->currentWave) = 1;
        *(params->waveTimer) = 0.0f;
        params->hitShader = LoadShader(0, "hit_shader.fs");
    }
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


void UpdatePlayer(Player *player, float deltaTime) {
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) player->position.y -= PLAYER_SPEED * deltaTime;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) player->position.y += PLAYER_SPEED * deltaTime;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) player->position.x -= PLAYER_SPEED * deltaTime;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) player->position.x += PLAYER_SPEED * deltaTime;

    // Clamp player position to stay within screen boundaries
    player->position.x = Clamp(player->position.x, player->radius, GetScreenWidth() - player->radius);
    player->position.y = Clamp(player->position.y, player->radius, GetScreenHeight() - player->radius);

    // // Auto-fire bullets
    // FireBullet(&player, &bulletManager);
}

void SpawnEnemy(GameLogicParams *params) {
    if (*(params->enemyCount) >= MAX_ENEMIES) {
        TraceLog(LOG_DEBUG, "Max enemies reached, cannot spawn more.");
        return; // Ensure we don't exceed the max enemies
    }
    Enemy newEnemy;
    newEnemy.radius = 15.0f;
    newEnemy.direction = (Vector2){0, 0};

    int edge = GetRandomValue(0, 3); // 0: top, 1: bottom, 2: left, 3: right
    switch (edge) {
        case 0: // Top
            newEnemy.position = (Vector2){GetRandomValue(0, GetScreenWidth()), 0};
            newEnemy.direction = (Vector2){0, 1}; // Move down
            break;
        case 1: // Bottom
            newEnemy.position = (Vector2){GetRandomValue(0, GetScreenWidth()), GetScreenHeight()};
            newEnemy.direction = (Vector2){0, -1}; // Move up
            break;
        case 2: // Left
            newEnemy.position = (Vector2){0, GetRandomValue(0, GetScreenHeight())};
            newEnemy.direction = (Vector2){1, 0}; // Move right
            break;
        case 3: // Right
            newEnemy.position = (Vector2){GetScreenWidth(), GetRandomValue(0, GetScreenHeight())};
            newEnemy.direction = (Vector2){-1, 0}; // Move left
            break;
    }
    params->enemies[*params->enemyCount] = newEnemy;
    (*(params->enemyCount))++;

    TraceLog(LOG_DEBUG, "Spawned enemy at position (%f, %f) with direction (%f, %f). Total enemies: %d",
        newEnemy.position.x, newEnemy.position.y, newEnemy.direction.x, newEnemy.direction.y, *(params->enemyCount));
}

void UpdateEnemies(GameLogicParams *params) {
    TraceLog(LOG_DEBUG, "Updating enemies. Current enemy count: %d", *(params->enemyCount));

    for (int i = 0; i < *(params->enemyCount); i++) {
        // Calculate direction vector from enemy to player
        Vector2 direction = (Vector2){params->player->position.x - params->enemies[i].position.x, params->player->position.y - params->enemies[i].position.y};

        // Normalize the direction vector
        direction = Vector2Normalize(direction);

        // Move the enemy towards the player
        params->enemies[i].position.x += direction.x * 100.0f * params->deltaTime; // Adjust speed as needed
        params->enemies[i].position.y += direction.y * 100.0f * params->deltaTime; // Adjust speed as needed

        // Clamp enemy position to stay within screen boundaries
        params->enemies[i].position.x = Clamp(params->enemies[i].position.x, params->enemies[i].radius, GetScreenWidth() - params->enemies[i].radius);
        params->enemies[i].position.y = Clamp(params->enemies[i].position.y, params->enemies[i].radius, GetScreenHeight() - params->enemies[i].radius);

        // Check for collision with player
        if (CheckCollision(params->player, &params->enemies[i])) {
            // Decrease player's health
            params->player->health--;
            
            // Remove enemy by shifting the rest of the array
            for (int j = i; j < *(params->enemyCount) - 1; j++) {
                params->enemies[j] = params->enemies[j + 1];
            }
            (*(params->enemyCount))--;
            i--; // Adjust index after removal
        }
    }

    // Spawn new enemies periodically
    if (GetRandomValue(0, 100) < *(params->enemySpawnVar) && *(params->enemyCount) < MAX_ENEMIES) {
        SpawnEnemy(params);
    }

    TraceLog(LOG_DEBUG, "Finished updating enemies. Current enemy count: %d", *(params->enemyCount));
}

bool CheckCollision(Player *player, Enemy *enemy) {
    return CheckCollisionCircles(player->position, player->radius, enemy->position, enemy->radius);
}

void UpdateBullets(BulletManager *bulletManager, float deltaTime) {
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

void FireBullet(Player *player, BulletManager *bulletManager, Enemy enemies[], int enemyCount, int powerUpsCollected, float fireRateIncrease)
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

void CheckBulletEnemyCollisions(BulletManager *bulletManager, Enemy enemies[], int *enemyCount, int *enemiesShot, int *hitEnemyIndex) {
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

                    // Store the index of the hit enemy
                    *hitEnemyIndex = j;

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

void SpawnPowerUp(PowerUp *powerUp, Player *player) {
    const float MIN_DISTANCE_FROM_PLAYER = 100.0f; // Minimum distance from player

    do {
        powerUp->position = (Vector2){GetRandomValue(50, GetScreenWidth() - 50), GetRandomValue(50, GetScreenHeight() - 50)};
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
    ClearBackground(m_colors[COLOR_DARK_GRAY]);
    DrawText("My Game Logo", GetScreenWidth() / 2 - MeasureText("My Game Logo", 20) / 2, GetScreenHeight() / 2 - 10, 20, m_colors[COLOR_WHITE]);
    DrawText("Created by Your Name", GetScreenWidth() / 2 - MeasureText("Created by Your Name", 20) / 2, GetScreenHeight() / 2 + 20, 20, m_colors[COLOR_WHITE]);
}

void DrawMainMenu() {
    ClearBackground(m_colors[COLOR_DARK_GRAY]);
    DrawText("Main Menu", GetScreenWidth() / 2 - MeasureText("Main Menu", 40) / 2, GetScreenHeight() / 2 - 40, 40, m_colors[COLOR_WHITE]);
    DrawText("1. Play Game", GetScreenWidth() / 2 - MeasureText("1. Play Game", 20) / 2, GetScreenHeight() / 2, 20, m_colors[COLOR_WHITE]);
    DrawText("2. Settings", GetScreenWidth() / 2 - MeasureText("2. Settings", 20) / 2, GetScreenHeight() / 2 + 30, 20, m_colors[COLOR_WHITE]);
    DrawText("3. Exit", GetScreenWidth() / 2 - MeasureText("3. Exit", 20) / 2, GetScreenHeight() / 2 + 60, 20, m_colors[COLOR_WHITE]);
}

void DrawGame(GameLogicParams *params) {
    ClearBackground(m_colors[COLOR_DARK_GRAY]);

    DrawCircleV(params->player->position, params->player->radius, m_colors[COLOR_BLUE]);
    DrawEnemies(params);
    DrawBullets(params->bulletManager);
    DrawText("Use WASD to move", 10, 10, 20, m_colors[COLOR_LIGHTER_GRAY]);

    // Draw player health at a fixed position
    char healthText[32];
    sprintf(healthText, "Health: %d", params->player->health);
    DrawText(healthText, 10, 40, 20, m_colors[COLOR_WHITE]);

    // Draw wave information centered at the top
    char waveText[32];
    sprintf(waveText, "Wave: %d", *(params->currentWave));
    int textWidth = MeasureText(waveText, 20);
    DrawText(waveText, (GetScreenWidth() - textWidth) / 2, 10, 20, m_colors[COLOR_WHITE]);

    // Optionally, display the remaining time for the current wave (in seconds)
    char timerText[32];
    sprintf(timerText, "Time: %d", (int)(WAVE_DURATION - *(params->waveTimer)));
    int timerTextWidth = MeasureText(timerText, 20);
    DrawText(timerText, (GetScreenWidth() - timerTextWidth) / 2, 40, 20, m_colors[COLOR_WHITE]);

    // Draw enemies killed
    char enemiesText[32];
    sprintf(enemiesText, "Enemies Killed: %d", *(params->enemiesShot));
    int enemiesTextWidth = MeasureText(enemiesText, 20);
    DrawText(enemiesText, (GetScreenWidth() - enemiesTextWidth) - 100, 10, 20, m_colors[COLOR_WHITE]);

    // Draw current enemy count
    char enemyCountText[32];
    sprintf(enemyCountText, "[debug] Enemies: %d", *(params->enemyCount));
    DrawText(enemyCountText, 10, 70, 16, m_colors[COLOR_WHITE]);
    
    if (params->powerUp->active) {
        DrawCircleV(params->powerUp->position, params->powerUp->radius, m_colors[COLOR_GREEN]); // Draw power-up
    }
}

void DrawBullets(BulletManager *bulletManager) {
    for (int i = 0; i < bulletManager->bulletCount; i++) {
        Bullet *bullet = &bulletManager->bullets[i];
        if (bullet->active) {
            DrawCircleV(bullet->position, bullet->radius, m_colors[COLOR_LIGHT_YELLOW]); // Draw bullet
        }
    }
}

void DrawEnemies(GameLogicParams *params) {
    for (int i = 0; i < *params->enemyCount; i++) {
        Enemy enemy = params->enemies[i];
        if (i == *params->hitEnemyIndex) {
            BeginShaderMode(params->hitShader);
        }
        DrawCircleV(enemy.position, enemy.radius, m_colors[COLOR_ORANGE_RED]);
        if (i == *params->hitEnemyIndex) {
            EndShaderMode();
        }
    }
}

void ExitGameplay(GameLogicParams *gameParams) {
    // Unload the shader
    UnloadShader(gameParams->hitShader);

    // Free any dynamically allocated resources if necessary
    // For example, if you have dynamically allocated memory for enemies or bullets, free them here

    // Reset game parameters if needed
    *(gameParams->hitEnemyIndex) = -1;
    *(gameParams->enemyCount) = 0;
    *(gameParams->enemyCount) = 0;
    *(gameParams->powerUpsCollected) = 0;
    *(gameParams->enemiesShot) = 0;
    gameParams->powerUp->active = false;
    *(gameParams->waveTimer) = 0.0f;
    *(gameParams->currentWave) = 1;
}

void DrawGameOver() {
    ClearBackground(m_colors[COLOR_DARK_GRAY]);
    DrawText("Game Over", GetScreenWidth() / 2 - MeasureText("Game Over", 40) / 2, GetScreenHeight() / 2 - 40, 40, m_colors[COLOR_WHITE]);
    DrawText("Press R to Restart or M to go to Main Menu", GetScreenWidth() / 2 - MeasureText("Press R to Restart or M to go to Main Menu", 20) / 2, GetScreenHeight() / 2, 20, DARKGRAY);
}

const char* SceneToString(Scene scene) {
    switch (scene) {
        case LOGO: return "LOGO";
        case MAIN_MENU: return "MAIN_MENU";
        case GAME: return "GAME";
        case GAME_OVER: return "GAME_OVER";
        default: return "UNKNOWN_SCENE";
    }
}
