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

    Scene currentScene = LOGO;
    float logoTimer = 0.0f;

    GameLogicParams gameLogicParams;
    InitGameParams(&gameLogicParams);

    SetTargetFPS(60);
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
                logoTimer += GetFrameTime();
                if (logoTimer >= 3.0f) { // Show logo for 3 seconds
                    currentScene = MAIN_MENU;
                }
                break;
            case MAIN_MENU:
                if (IsKeyPressed(KEY_ONE)) {
                    currentScene = GAME;
                }
                else if (IsKeyPressed(KEY_TWO)) {
                }
                else if (IsKeyPressed(KEY_THREE)) {
                    CloseWindow();
                }
                break;
            case GAME:
               if (!gameLogicParams.isGamePaused) {
                    GameLogic(&gameLogicParams); // Only update game logic if not paused
                }
                if (IsKeyPressed(KEY_P)) {
                    gameLogicParams.isGamePaused = !gameLogicParams.isGamePaused;
                }
                else if (IsKeyPressed(KEY_SPACE)) {
                    currentScene = GAME_OVER;
                }
                break;
            case GAME_OVER:
                ExitGameplay(&gameLogicParams);
                if (IsKeyPressed(KEY_R)) {
                    currentScene = GAME; // Restart the game
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
    CloseWindow(); // Close window and OpenGL context

    return 0;
}
