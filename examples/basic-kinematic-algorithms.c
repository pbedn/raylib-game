/* Kinematick seek, flee, arriving, wandering

Kinematic movement algorithms use static data (position and orientation, no velocities) and
output a desired velocity.

*/

#include <stdbool.h>
#include <float.h>
#include <stddef.h>

#include "raylib.h"
#include "raymath.h"

#define MAX_SPEED 100.0f

typedef struct {
    Vector2 position;   // Position of the character
    float orientation;  // Orientation in radians
} KinematicCharacter;

typedef struct {
    Vector2 velocity;
    float rotation;
} KinematicSteeringOutput;

typedef struct {
    KinematicCharacter* character; // The character seeking
    KinematicCharacter* target;    // The target to seek
    float maxSpeed;                // Maximum speed
} KinematicSeek;

typedef struct {
    KinematicCharacter* character; // The character arriving
    KinematicCharacter* target;     // The target to arrive at
    float maxSpeed;                 // Maximum speed
    float radius;                   // Satisfaction radius
    float timeToTarget;             // Time to target constant
} KinematicArrive;

typedef struct {
    KinematicCharacter* character; // The character wandering
    float maxSpeed;                // Maximum speed
    float maxRotation;             // Maximum rotation speed
} KinematicWander;

typedef enum {
    IDLE,
    SEEKING,
    FLEEING,
    ARRIVING,
    WANDERING
} EnemyState;

KinematicSteeringOutput GetArrive(KinematicArrive* kinematicArrive);
KinematicSteeringOutput GetSteering(KinematicSeek* kinematicSeek, bool isFleeing);
KinematicSteeringOutput GetWander(KinematicWander* kinematicWander);
float newOrientation(float currentOrientation, Vector2 velocity);
float randomBinomial();

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Kinematic Seek and Flee");

    KinematicCharacter character = { .position = {400, 300}, .orientation = 0 };
    KinematicCharacter target = { .position = {200, 200}, .orientation = 0 };

    KinematicSeek kinematicSeek = { .character = &character, .target = &target, .maxSpeed = 200.0f };

    KinematicArrive kinematicArrive = { .character = &character, .target = &target, .maxSpeed = 200.0f, .radius = 50.0f, .timeToTarget = 0.25f };

    KinematicWander kinematicWander = { .character = &character, .maxSpeed = 200.0f, .maxRotation = 0.1f };
    
    EnemyState enemyState = IDLE;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            character.position = GetMousePosition();
        }

        // Check for behavior change
        if (IsKeyDown(KEY_F)) {
            enemyState = FLEEING;
        } else if (IsKeyDown(KEY_S)) {
            enemyState = SEEKING;
        } else if (IsKeyDown(KEY_A)) {
            enemyState = ARRIVING;
        } else if (IsKeyDown(KEY_W)) {
            enemyState = WANDERING;
        } else if (IsKeyDown(KEY_SPACE)) {
            enemyState = IDLE;
        }

        KinematicSteeringOutput steering;

        // Determine behavior based on target state
        switch (enemyState) {
            case IDLE:
                steering.velocity = (Vector2){0, 0}; // No movement when idle
                break;
            case SEEKING:
                steering = GetSteering(&kinematicSeek, false);
                break;
            case FLEEING:
                steering = GetSteering(&kinematicSeek, true);
                break;
            case ARRIVING:
                steering = GetArrive(&kinematicArrive);
                break;
            case WANDERING:
                steering = GetWander(&kinematicWander);
                break;
            }

        // Update character position based on steering output
        character.position = Vector2Add(character.position, Vector2Scale(steering.velocity, GetFrameTime()));

        BeginDrawing();
        ClearBackground(BLACK);
        DrawCircleV(character.position, 20, BLUE); // Draw character
        DrawCircleV(target.position, 20, RED);  // Draw target

        if (enemyState == IDLE)
            DrawText("Idle!", 10, 10, 20, RAYWHITE);
        if (enemyState == FLEEING)
            DrawText("Fleeing!", 10, 10, 20, RAYWHITE);
        if (enemyState == SEEKING)
            DrawText("Seeking!", 10, 10, 20, RAYWHITE);
        DrawText("Press S to make target Seek, F to Flee, Space to Stop", 10, 50, 20, RAYWHITE);
        DrawText("Press A to make character arriving", 10, 100, 20, RAYWHITE);
        EndDrawing();
    }
    CloseWindow(); // Close window and OpenGL context
    return 0;
}

KinematicSteeringOutput GetSteering(KinematicSeek* kinematicSeek, bool isFleeing) {
    KinematicSteeringOutput result = {0};

    Vector2 direction;
    if (isFleeing) {
        direction = Vector2Subtract(kinematicSeek->character->position, kinematicSeek->target->position);
    } else {
        direction = Vector2Subtract(kinematicSeek->target->position, kinematicSeek->character->position);
    }

    // Calculate the velocity along this direction at full speed
    float distance = Vector2Length(direction);
    if (distance > 0) {
        direction = Vector2Normalize(direction);
        result.velocity = Vector2Scale(direction, kinematicSeek->maxSpeed);
    } else {
        result.velocity = (Vector2){0, 0}; // No movement if the character is at the target
    }

    // Face in the direction we want to move
    kinematicSeek->character->orientation = newOrientation(kinematicSeek->character->orientation, result.velocity);
    result.rotation = 0; // No rotation change

    return result;
}

KinematicSteeringOutput GetFlee(KinematicSeek* kinematicSeek) {
    KinematicSteeringOutput result = {0};

    // Get the direction away from the target
    Vector2 direction = Vector2Subtract(kinematicSeek->character->position, kinematicSeek->target->position);
    
    // Calculate the velocity away from the target at full speed
    float distance = Vector2Length(direction);
    if (distance > 0) {
        direction = Vector2Normalize(direction);
        result.velocity = Vector2Scale(direction, kinematicSeek->maxSpeed);
    } else {
        result.velocity = (Vector2){0, 0}; // No movement if the character is at the target
    }

    // Face in the direction we want to move
    kinematicSeek->character->orientation = atan2(result.velocity.y, result.velocity.x);
    result.rotation = 0; // No rotation change

    return result;
}

KinematicSteeringOutput GetArrive(KinematicArrive* kinematicArrive) {
    KinematicSteeringOutput result = {0};

    // Get the direction to the target
    result.velocity = Vector2Subtract(kinematicArrive->target->position, kinematicArrive->character->position);

    // Check if we’re within the radius
    if (Vector2Length(result.velocity) < kinematicArrive->radius) {
        return (KinematicSteeringOutput){0}; // Request no steering
    }

    // We need to move to our target, we’d like to get there in timeToTarget seconds
    result.velocity = Vector2Scale(result.velocity, 1.0f / kinematicArrive->timeToTarget);

    // If this is too fast, clip it to the max speed
    if (Vector2Length(result.velocity) > kinematicArrive->maxSpeed) {
        result.velocity = Vector2Normalize(result.velocity);
        result.velocity = Vector2Scale(result.velocity, kinematicArrive->maxSpeed);
    }

    // Face in the direction we want to move
    kinematicArrive->character->orientation = newOrientation(kinematicArrive->character->orientation, result.velocity);
    result.rotation = 0; // No rotation change

    return result;
}

// Function to calculate new orientation (you need to implement this)
float newOrientation(float currentOrientation, Vector2 velocity) {
    // Calculate the new orientation based on the velocity vector
    return atan2(velocity.y, velocity.x);
}

KinematicSteeringOutput GetWander(KinematicWander* kinematicWander) {
    KinematicSteeringOutput result = {0};

    // Get velocity from the vector form of the orientation
    result.velocity = Vector2Scale((Vector2){cos(kinematicWander->character->orientation), sin(kinematicWander->character->orientation)}, kinematicWander->maxSpeed);

    // Change our orientation randomly
    result.rotation = randomBinomial() * kinematicWander->maxRotation;

    return result;
}

float randomBinomial() {
    // Generate a random float between -1 and 1, with values around zero being more likely
    return (GetRandomValue(0, 100) / 100.0f) - (GetRandomValue(0, 100) / 100.0f);
}
