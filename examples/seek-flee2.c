/* Enemy SEEK or FLEE the Player */

#include <stdbool.h>
#include <float.h>
#include <stddef.h>

#include "raylib.h"
#include "raymath.h"

#define MAX_SPEED 100.0f

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

SteeringOutput Seek(KinematicCharacter* character, KinematicCharacter* target);
SteeringOutput Flee(KinematicCharacter* character, KinematicCharacter* target);
void UpdateKinematicCharacter(KinematicCharacter* character, SteeringOutput* steering, float deltaTime);
void DrawCharacterWithOrientation(KinematicCharacter* character, Color color);
void UpdateOrientation(KinematicCharacter* character);

int main(void) {
    SetTraceLogLevel( LOG_ALL );
    SetConfigFlags( FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Kinematic Seek and Flee");

    KinematicCharacter player = { .position = {400, 300}, .orientation = 0, .velocity = {0, 0}, .rotation = 0, .radius = 20.0f };
    KinematicCharacter enemy = { .position = {0, 0}, .orientation = 0, .velocity = {0, 0}, .rotation = 0, .radius = 20.0f };
    enemy.velocity = (Vector2){100, 0}; // Set initial speed for the enemy

    SteeringOutput steering = {0};
    EnemyState enemyState = IDLE;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            player.position = GetMousePosition();
        }

        // Update player movement
        if (IsKeyDown(KEY_UP)) player.velocity.y -= 200 * GetFrameTime();
        if (IsKeyDown(KEY_DOWN)) player.velocity.y += 200 * GetFrameTime();
        if (IsKeyDown(KEY_LEFT)) player.velocity.x -= 200 * GetFrameTime();
        if (IsKeyDown(KEY_RIGHT)) player.velocity.x += 200 * GetFrameTime();

        // Update orientation based on velocity
        UpdateOrientation(&player);

        // Update player position
        UpdateKinematicCharacter(&player, &(SteeringOutput){.linear = {0, 0}, .angular = 0}, GetFrameTime());

        // Check for behavior change
        if (IsKeyDown(KEY_F)) {
            enemyState = FLEEING;
        } else if (IsKeyDown(KEY_S)) {
            enemyState = SEEKING;
        } else if (IsKeyDown(KEY_SPACE)) {
            enemyState = IDLE;
        }

        switch (enemyState) {
            case IDLE:
                steering.linear = (Vector2){0, 0};
                break;
            case SEEKING:
                steering = Seek(&enemy, &player);
                break;
            case FLEEING:
                steering = Flee(&enemy, &player);
                break;
        }
        UpdateOrientation(&enemy);
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

        if (enemyState == IDLE)
            DrawText("Idle!", 10, 10, 20, RAYWHITE);
        if (enemyState == FLEEING)
            DrawText("Fleeing!", 10, 10, 20, RAYWHITE);
        if (enemyState == SEEKING)
            DrawText("Seeking!", 10, 10, 20, RAYWHITE);
        DrawText("Press S to make enemy Seek, F to Flee, Space to zero orientation", 10, 50, 20, RAYWHITE);

        EndDrawing();
    }
    CloseWindow(); // Close window and OpenGL context
    return 0;
}


SteeringOutput Seek(KinematicCharacter* character, KinematicCharacter* target) {
    SteeringOutput output = {0};

    // Get the direction to the target
    output.linear = Vector2Subtract(target->position, character->position);

    // Give full acceleration along this direction
    output.linear = Vector2Normalize(output.linear);
    output.linear = Vector2Scale(output.linear, MAX_SPEED); // Use a fixed speed for seeking

    output.angular = 0; // No angular change for seeking

    return output;
}

SteeringOutput Flee(KinematicCharacter* character, KinematicCharacter* target) {
    SteeringOutput output = {0};

    // Get the direction to the target
    output.linear = Vector2Subtract(character->position, target->position);

    // Give full acceleration along this direction
    output.linear = Vector2Normalize(output.linear);
    output.linear = Vector2Scale(output.linear, MAX_SPEED); // Use a fixed speed for seeking

    output.angular = 0; // No angular change for seeking

    return output;
}

void UpdateKinematicCharacter(KinematicCharacter* character, SteeringOutput* steering, float deltaTime) {
    // Update position and orientation
    character->position = Vector2Add(character->position, Vector2Scale(character->velocity, deltaTime));
    character->orientation += character->rotation * deltaTime;

    // Update velocity and rotation
    character->velocity = Vector2Add(character->velocity, Vector2Scale(steering->linear, deltaTime));
    character->rotation += steering->angular * deltaTime;

    // Check for speeding and clip
    float maxSpeed = 200.0f; // Set your desired max speed
    if (Vector2Length(character->velocity) > maxSpeed) {
        character->velocity = Vector2Normalize(character->velocity);
        character->velocity = Vector2Scale(character->velocity, maxSpeed);
    }
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

void UpdateOrientation(KinematicCharacter* character) {
    if (Vector2Length(character->velocity) > 0) {
        character->orientation = atan2(character->velocity.y, character->velocity.x);
    }
}