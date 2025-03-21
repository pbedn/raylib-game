#include <stdio.h>
#include "raylib.h"

#define MAX_BUNNIES 50000
#define MAX_BATCH_ELEMENTS 8192

typedef struct Bunny {
    Vector2 position;
    Vector2 speed;
    Color color;
} Bunny;

int main(void)
{
    SetTraceLogLevel(LOG_ALL);
    TraceLog(LOG_DEBUG, "Init");
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1920;
    int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - bunnymark");

    // Load bunny texture
    Texture2D texBunny = LoadTexture("resources/wabbit_alpha.png");

    Bunny bunnies[MAX_BUNNIES];
    int bunniesCount = 0; // Bunnies counter

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            // Create more bunnies
            for (int i = 0; i < 100; i++)
            {
                if (bunniesCount < MAX_BUNNIES)
                {
                    bunnies[bunniesCount].position = GetMousePosition();
                    bunnies[bunniesCount].speed.x = (float)GetRandomValue(-250, 250) / 60.0f;
                    bunnies[bunniesCount].speed.y = (float)GetRandomValue(-250, 250) / 60.0f;
                    bunnies[bunniesCount].color = (Color){ GetRandomValue(50, 240), 
                                                           GetRandomValue(80, 240), 
                                                           GetRandomValue(100, 240), 255 };

                    bunniesCount++;
                }
            }
        }

        // Update bunnies
        for (int i = 0; i < bunniesCount; i++)
        {
            bunnies[i].position.x += bunnies[i].speed.x;
            bunnies[i].position.y += bunnies[i].speed.y;

            if ((bunnies[i].position.x + texBunny.width / 2) > GetScreenWidth() || 
                (bunnies[i].position.x + texBunny.width / 2) < 0)
            {
                bunnies[i].speed.x *= -1;
            }
            if ((bunnies[i].position.y + texBunny.height / 2) > GetScreenHeight() || 
                (bunnies[i].position.y + texBunny.height / 2 - 40) < 0)
            {
                bunnies[i].speed.y *= -1;
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        for (int i = 0; i < bunniesCount; i++)
        {
            DrawTexture(texBunny, (int)bunnies[i].position.x, (int)bunnies[i].position.y, bunnies[i].color);
        }

        DrawRectangle(0, 0, screenWidth, 40, BLACK);
        // char text[256];
        // snprintf(text, sizeof(text), "bunnies %d", bunniesCount);
        // DrawText(text, 120, 10, 20, GREEN);
        // snprintf(text, sizeof(text), "batched draw calls: %d", 1 + bunniesCount / MAX_BATCH_ELEMENTS);
        // DrawText(text, 320, 10, 20, MAROON);
        DrawText(TextFormat("bunnies: %i", bunniesCount), 120, 10, 20, GREEN);
        DrawText(TextFormat("batched draw calls: %i", 1 + bunniesCount/MAX_BATCH_ELEMENTS), 320, 10, 20, MAROON);


        DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texBunny); // Unload bunny texture
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
