#include "game.h"
#include "globals.h"


void GameLogic(GameLogicParams *params) {
    //
    /* Input Handling: Update player movement based on input. */
    //
    UpdatePlayer(params->player, params->deltaTime, params->screenWidth, params->screenHeight);

    //
    /* Update Game State: Update the state of the player, enemies, bullets, and power-ups. */
    //
    UpdateBullets(params->bulletManager, params->deltaTime);
    FireBullet(params->player, params->bulletManager, params->enemies, *(params->enemyCount), *(params->powerUpsCollected), 0.05f);
    UpdateEnemies(params->enemies, params->enemyCount, params->player, params->deltaTime, params->screenWidth, params->screenHeight, *(params->enemySpawnVar));
    
    //
    /* Collision Detection: Check for collisions between bullets and enemies, and between the player and power-ups. */
    //
    // Collision Detection
    CheckBulletEnemyCollisions(params->bulletManager, params->enemies, params->enemyCount, params->enemiesShot, params->hitEnemyIndex);
    CheckPowerUpCollection(params->player, params->powerUp, params->powerUpsCollected, params->enemySpawnVar);

    // Spawn power-up if conditions are met
    if (!params->powerUp->active && (*(params->enemiesShot) != 0) && (*(params->enemiesShot) % 10 == 0)) {
        SpawnPowerUp(params->powerUp, params->screenWidth, params->screenHeight, params->player);
    }

    // Spawn new enemies based on the updated enemy spawn variable
    if (GetRandomValue(0, 100) < *(params->enemySpawnVar) && *(params->enemyCount) < MAX_ENEMIES) {
        SpawnEnemy(&params->enemies[*(params->enemyCount)], params->screenWidth, params->screenHeight);
        (*(params->enemyCount))++;
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
        InitEnemies(params->enemyCount);
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

void InitEnemies(int *enemyCount) {
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

void DrawEnemies(Enemy enemies[], int enemyCount, Shader hitShader, int hitEnemyIndex) {
    for (int i = 0; i < enemyCount; i++) {
        if (i == hitEnemyIndex) {
            BeginShaderMode(hitShader);
            DrawCircleV(enemies[i].position, enemies[i].radius, m_colors[COLOR_ORANGE_RED]);
            EndShaderMode();
        } else {
            DrawCircleV(enemies[i].position, enemies[i].radius, m_colors[COLOR_ORANGE_RED]);
        }
    }
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

void DrawBullets(BulletManager *bulletManager) {
    for (int i = 0; i < bulletManager->bulletCount; i++) {
        Bullet *bullet = &bulletManager->bullets[i];
        if (bullet->active) {
            DrawCircleV(bullet->position, bullet->radius, m_colors[COLOR_LIGHT_YELLOW]); // Draw bullet
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
    DrawEnemies(params->enemies, *(params->enemyCount), params->hitShader, *(params->hitEnemyIndex));
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
    DrawText(waveText, (params->screenWidth - textWidth) / 2, 10, 20, m_colors[COLOR_WHITE]);

    // Optionally, display the remaining time for the current wave (in seconds)
    char timerText[32];
    sprintf(timerText, "Time: %d", (int)(WAVE_DURATION - *(params->waveTimer)));
    int timerTextWidth = MeasureText(timerText, 20);
    DrawText(timerText, (params->screenWidth - timerTextWidth) / 2, 40, 20, m_colors[COLOR_WHITE]);

    // Draw enemies killed
    char enemiesText[32];
    sprintf(enemiesText, "Enemies Killed: %d", *(params->enemiesShot));
    int enemiesTextWidth = MeasureText(enemiesText, 20);
    DrawText(enemiesText, (params->screenWidth - enemiesTextWidth) - 100, 10, 20, m_colors[COLOR_WHITE]);

    if (params->powerUp->active) {
        DrawCircleV(params->powerUp->position, params->powerUp->radius, m_colors[COLOR_GREEN]); // Draw power-up
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
    InitEnemies(gameParams->enemyCount);
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
