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


typedef struct KinematicCharacter {
    Vector2 position;   // Position of the character
    float orientation;  // Orientation in radians
    Vector2 velocity;   // Current velocity
    float rotation;     // Current rotation speed
    float radius;       // Radius for drawing (circle)
} KinematicCharacter;

typedef struct SteeringOutput {
    Vector2 linear;     // Linear acceleration
    float angular;      // Angular acceleration
} SteeringOutput;

typedef enum {
    IDLE,
    SEEKING,
    FLEEING
} EnemyState;

SteeringOutput Seek(KinematicCharacter* character, Vector2 target);
SteeringOutput Flee(KinematicCharacter* character, Vector2 target);
void UpdateKinematicCharacter(KinematicCharacter* character, SteeringOutput* steering, float deltaTime);
void DrawCharacterWithOrientation(KinematicCharacter* character, Color color);

int main(void) {
    SetTraceLogLevel( LOG_ALL );
    SetConfigFlags( FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Reverse Bullet Hell Survivor Roguelike");

    KinematicCharacter player = { .position = {400, 300}, .orientation = 0, .velocity = {0, 0}, .rotation = 0, .radius = 20.0f };
    KinematicCharacter enemy = { .position = {0, 0}, .orientation = 0, .velocity = {0, 0}, .rotation = 0, .radius = 20.0f };
    enemy.velocity = (Vector2){100, 0}; // Set initial speed for the enemy

    SteeringOutput steering = {0};
    EnemyState enemyState = IDLE;

    float acceleration = 1000.0f; // Adjust this value to control how quickly the player slows down
    float baseDeceleration = 4000.0f; // Base deceleration factor

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Update player movement
        if (IsKeyDown(KEY_UP)) player.velocity.y -= acceleration * GetFrameTime();
        else if (IsKeyDown(KEY_DOWN)) player.velocity.y += acceleration * GetFrameTime();
        else {
            // Apply deceleration when no vertical keys are pressed
            if (player.velocity.y > 0) {
                player.velocity.y -= baseDeceleration * GetFrameTime() * (player.velocity.y / acceleration); // Scale by current speed
                if (player.velocity.y < 0) player.velocity.y = 0; // Prevent reversing direction
            } else if (player.velocity.y < 0) {
                player.velocity.y += baseDeceleration * GetFrameTime() * (-player.velocity.y / acceleration); // Scale by current speed
                if (player.velocity.y > 0) player.velocity.y = 0; // Prevent reversing direction
            }
        }
        if (IsKeyDown(KEY_LEFT)) player.velocity.x -= acceleration * GetFrameTime();
        else if (IsKeyDown(KEY_RIGHT)) player.velocity.x += acceleration * GetFrameTime();
        else {
            // Apply deceleration when no horizontal keys are pressed
            if (player.velocity.x > 0) {
                player.velocity.x -= baseDeceleration * GetFrameTime() * (player.velocity.x / acceleration); // Scale by current speed
                if (player.velocity.x < 0) player.velocity.x = 0; // Prevent reversing direction
            } else if (player.velocity.x < 0) {
                player.velocity.x += baseDeceleration * GetFrameTime() * (-player.velocity.x / acceleration); // Scale by current speed
                if (player.velocity.x > 0) player.velocity.x = 0; // Prevent reversing direction
            }
        }

        // Update player position
        UpdateKinematicCharacter(&player, &(SteeringOutput){.linear = {0, 0}, .angular = 0}, GetFrameTime());

        // Check for behavior change
        if (IsKeyPressed(KEY_F)) {
            enemyState = FLEEING;
        } else if (IsKeyPressed(KEY_S)) {
            enemyState = SEEKING;
        }

        switch (enemyState) {
            case IDLE:
                break;
            case SEEKING:
                steering = Seek(&enemy, player.position);
                // enemy.velocity = Vector2Add(enemy.velocity, steering.linear);
                break;
            case FLEEING:
                steering = Flee(&enemy, player.position);
                break;
        }
        // Update enemy behavior
        UpdateKinematicCharacter(&enemy, &steering, GetFrameTime());


        // Draw
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw the player
        DrawCharacterWithOrientation(&player, BLUE);

        // Draw the enemy
        DrawCharacterWithOrientation(&enemy, RED);

        // Optionally, draw the target position for the enemy
        DrawCircleV(player.position, 5, GREEN); // Draw a small circle at the player's position

        EndDrawing();
    }

    //
    /* De-Initialization: Clean up resources and close the window. */
    //
    CloseWindow(); // Close window and OpenGL context

    return 0;
}


SteeringOutput Seek(KinematicCharacter* character, Vector2 target) {
    SteeringOutput output = {0};

    Vector2 desiredVelocity = Vector2Subtract(target, character->position);
    float distance = Vector2Length(desiredVelocity);
    
    if (distance > 0) {
        desiredVelocity = Vector2Normalize(desiredVelocity);
        desiredVelocity = Vector2Scale(desiredVelocity, character->velocity.x); // Use character's speed
    }

    output.linear = Vector2Subtract(desiredVelocity, character->velocity);
    output.angular = 0; // No angular change for seeking

    return output;
}

SteeringOutput Flee(KinematicCharacter* character, Vector2 target) {
    SteeringOutput output = {0};

    Vector2 desiredVelocity = Vector2Subtract(character->position, target);
    float distance = Vector2Length(desiredVelocity);
    
    if (distance > 0) {
        desiredVelocity = Vector2Normalize(desiredVelocity);
        desiredVelocity = Vector2Scale(desiredVelocity, character->velocity.x); // Use character's speed
    }

    output.linear = Vector2Subtract(desiredVelocity, character->velocity);
    output.angular = 0; // No angular change for fleeing

    return output;
}

void UpdateKinematicCharacter(KinematicCharacter* character, SteeringOutput* steering, float deltaTime) {
    // Update linear velocity
    character->velocity = Vector2Add(character->velocity, Vector2Scale(steering->linear, deltaTime));
    
    // Update position
    character->position = Vector2Add(character->position, Vector2Scale(character->velocity, deltaTime));
    
    // Update orientation
    character->rotation += steering->angular * deltaTime;
    character->orientation += character->rotation * deltaTime;
}

void DrawCharacterWithOrientation(KinematicCharacter* character, Color color) {
    // Draw the character as a circle
    DrawCircleV(character->position, character->radius, color);

    // Calculate the end point of the orientation line
    Vector2 orientationEnd = {
        character->position.x + cos(character->orientation) * character->radius * 1.5f,
        character->position.y + sin(character->orientation) * character->radius * 1.5f
    };

    // Draw the orientation line
    DrawLineV(character->position, orientationEnd, color);
}
