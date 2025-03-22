#include "game.h"
#include "globals.h"

int main(void) {
    SetTraceLogLevel(LOG_ALL);
    //
    /* Initialization: Set up the window and initialize game entities. */
    //
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Reverse Bullet Hell Survivor Roguelike");

    SearchAndSetResourceDir("resources");

    // Player player;
    // InitPlayer(&player);

    // BulletManager bulletManager;
    // InitBulletManager(&bulletManager);

    // PowerUp powerUp; // Move power-up variable to main
    // powerUp.active = false; // Initialize power-up as inactive

    // Enemy enemies[MAX_ENEMIES] = {0};
    // int enemyCount;
    // InitEnemies(&enemyCount);
    // int hitEnemyIndex = -1; // Initialize to -1 (no hit)

    // // Wave system variables
    // float waveTimer = 0.0f;
    // int currentWave = 1;

    Scene currentScene = LOGO;
    float logoTimer = 0.0f;

    // Shader hitShader = LoadShader(0, "hit_shader.fs");

    // GameLogicParams gameLogicParams = {
    //     .player = &player,
    //     .bulletManager = &bulletManager,
    //     .enemies = {*enemies}, // Initialize enemies array
    //     .enemyCount = &enemyCount,
    //     .powerUp = &powerUp,
    //     .powerUpsCollected = &powerUpsCollected,
    //     .enemiesShot = &enemiesShot,
    //     .enemySpawnVar = &enemySpawnVar,
    //     .deltaTime = 0.0f,
    //     .waveTimer = &waveTimer,
    //     .currentWave = &currentWave,
    //     .hitEnemyIndex = &hitEnemyIndex,
    //     .hitShader = hitShader,
    //     .isGamePaused = false
    // };
    GameLogicParams gameLogicParams;
    InitGameParams(&gameLogicParams);

    // Player player;
    // player.position = (Vector2){400, 300}; // Center of the screen
    // player.radius = 20.0f;
    // player.health = 10;

    // BulletManager bulletManager;
    // bulletManager.bulletCount = 0; // Initialize bullet count
    // bulletManager.lastShotTime = 0.0f; // Reset shot timer
    // bulletManager.bulletCooldown = 0.8f; // Set bullet cooldown

    // PowerUp powerUp;
    // powerUp.active = false;

    // // Enemy enemies[MAX_ENEMIES];
    // // Initialize enemies array
    // // for (int i = 0; i < MAX_ENEMIES; i++) {
    // //     params->enemies[i] = (Enemy){0}; // Initialize each Enemy to zero
    // // }
    // int enemyCount = 0;
    // int hitEnemyIndex = -1; // Initialize to -1 (no hit)

    // // Wave system variables
    // float waveTimer = 0.0f;
    // int currentWave = 1;

    // Shader hitShader = LoadShader(0, "hit_shader.fs");    

    // gameLogicParams.player = &player;
    // gameLogicParams.bulletManager = &bulletManager;
    // gameLogicParams.enemyCount = &enemyCount;
    // gameLogicParams.powerUp = &powerUp;
    // gameLogicParams.powerUpsCollected = &powerUpsCollected;
    // gameLogicParams.enemiesShot = &enemiesShot;
    // gameLogicParams.enemySpawnVar = &enemySpawnVar;
    // gameLogicParams.deltaTime = 0.0f;
    // gameLogicParams.waveTimer = &waveTimer;
    // gameLogicParams.currentWave = &currentWave;
    // gameLogicParams.hitEnemyIndex = &hitEnemyIndex;
    // gameLogicParams.hitShader = hitShader;
    // gameLogicParams.isGamePaused = false;

    SetTargetFPS(60); // Set the game to run at 60 frames-per-second

    //
    /* Game Loop: Continuously update and draw the game until the window is closed. */
    //
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        gameLogicParams.deltaTime = deltaTime;
        

#ifdef DEV_MODE
        currentScene = GAME;
#endif
        switch (currentScene) {
            case LOGO:
                // TraceLog(LOG_DEBUG, "!!!!! Entering %s Scene", SceneToString(currentScene));
                logoTimer += GetFrameTime();
                if (logoTimer >= 3.0f) { // Show logo for 3 seconds
                    currentScene = MAIN_MENU;
                }
                break;
            case MAIN_MENU:
                // TraceLog(LOG_DEBUG, "!!!!! Entering %s Scene", SceneToString(currentScene));
                if (IsKeyPressed(KEY_ONE)) {
                    currentScene = GAME;
                }
                else if (IsKeyPressed(KEY_TWO)) {
                    // Handle settings (not implemented)
                }
                else if (IsKeyPressed(KEY_THREE)) {
                    CloseWindow(); // Exit the game
                }
                break;
            case GAME:
                // TraceLog(LOG_DEBUG, "!!!!! Entering %s Scene", SceneToString(currentScene));
               if (!gameLogicParams.isGamePaused) {
                    GameLogic(&gameLogicParams); // Only update game logic if not paused
                }
                // For demonstration, we'll simulate a game over after a key press
                if (IsKeyPressed(KEY_P)) {
                    gameLogicParams.isGamePaused = !gameLogicParams.isGamePaused;
                }
                else if (IsKeyPressed(KEY_SPACE)) {
                    // gameLogicParams.isGamePaused = true;
                    currentScene = GAME_OVER;
                }
                break;
            case GAME_OVER:
                ExitGameplay(&gameLogicParams);
                // TraceLog(LOG_DEBUG, "!!!!! Entering %s Scene", SceneToString(currentScene));
                if (IsKeyPressed(KEY_R)) {
                    // gameLogicParams.isGamePaused = false;
                    currentScene = GAME; // Restart the game
                    // Optionally, reinitialize game parameters here
                }
                else if (IsKeyPressed(KEY_M)) {
                    ExitGameplay(&gameLogicParams);
                    currentScene = MAIN_MENU; // Go to main menu
                }
                break;
            default: break;
        }

        BeginDrawing();
        switch (currentScene) {
            case LOGO:
                DrawLogo();
                break;
            case MAIN_MENU:
                DrawMainMenu();
                break;
            case GAME:
                DrawGame(&gameLogicParams);
                if (gameLogicParams.isGamePaused) {
                    DrawText("Game Paused", GetScreenWidth() / 2 - MeasureText("Game Paused", 20) / 2, GetScreenHeight() / 2 - 10, 20, RED);
                }
                break;
            case GAME_OVER:
                DrawGameOver();
                break;
            }
        EndDrawing();
    }

    //
    /* De-Initialization: Clean up resources and close the window. */
    //
    UnloadShader(gameLogicParams.hitShader);
    CloseWindow(); // Close window and OpenGL context

    return 0;
}
