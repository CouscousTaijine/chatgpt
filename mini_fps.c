#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include <stdio.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAX_BOTS 24
#define BOT_SIZE 1.0f
#define BOT_SPEED 2.2f
#define ARENA_HALF_SIZE 45.0f
#define FIRE_COOLDOWN 0.14f
#define MATCH_TIME 90.0f

typedef struct Bot {
    Vector3 pos;
    float hp;
    float respawnTimer;
} Bot;

static float frand_range(float min, float max) {
    return min + ((float)GetRandomValue(0, 10000) / 10000.0f) * (max - min);
}

static Vector3 random_bot_position(void) {
    return (Vector3){ frand_range(-ARENA_HALF_SIZE, ARENA_HALF_SIZE), BOT_SIZE * 0.5f, frand_range(-ARENA_HALF_SIZE, ARENA_HALF_SIZE) };
}

static void spawn_bot(Bot *bot) {
    bot->pos = random_bot_position();
    bot->hp = 100.0f;
    bot->respawnTimer = 0.0f;
}

static int try_shoot_bot(Bot bots[], int count, Vector3 origin, Vector3 direction, float maxDistance) {
    float bestDist = maxDistance;
    int hit = -1;

    for (int i = 0; i < count; i++) {
        if (bots[i].hp <= 0.0f) continue;

        Vector3 toBot = Vector3Subtract(bots[i].pos, origin);
        float forwardDist = Vector3DotProduct(toBot, direction);
        if (forwardDist < 0.0f || forwardDist > maxDistance) continue;

        Vector3 closestPoint = Vector3Add(origin, Vector3Scale(direction, forwardDist));
        float perpDist = Vector3Distance(closestPoint, bots[i].pos);

        if (perpDist <= BOT_SIZE * 0.9f && forwardDist < bestDist) {
            bestDist = forwardDist;
            hit = i;
        }
    }

    return hit;
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Mini FPS C - bots 3D");
    SetTargetFPS(144);
    DisableCursor();

    Camera3D cam = {0};
    cam.position = (Vector3){0.0f, 1.8f, 0.0f};
    cam.target = (Vector3){0.0f, 1.8f, 1.0f};
    cam.up = (Vector3){0.0f, 1.0f, 0.0f};
    cam.fovy = 75.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    Bot bots[MAX_BOTS] = {0};
    for (int i = 0; i < MAX_BOTS; i++) spawn_bot(&bots[i]);

    float yaw = 0.0f;
    float pitch = 0.0f;
    float fireTimer = 0.0f;
    float timeLeft = MATCH_TIME;
    int score = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        timeLeft -= dt;
        if (timeLeft <= 0.0f) break;

        fireTimer -= dt;

        Vector2 mouseDelta = GetMouseDelta();
        yaw += mouseDelta.x * 0.0025f;
        pitch -= mouseDelta.y * 0.0025f;
        if (pitch > 1.45f) pitch = 1.45f;
        if (pitch < -1.45f) pitch = -1.45f;

        Vector3 forward = {
            cosf(pitch) * sinf(yaw),
            sinf(pitch),
            cosf(pitch) * cosf(yaw)
        };
        forward = Vector3Normalize(forward);

        Vector3 right = Vector3Normalize((Vector3){ forward.z, 0.0f, -forward.x });
        Vector3 move = {0};

        if (IsKeyDown(KEY_W)) move = Vector3Add(move, forward);
        if (IsKeyDown(KEY_S)) move = Vector3Subtract(move, forward);
        if (IsKeyDown(KEY_D)) move = Vector3Add(move, right);
        if (IsKeyDown(KEY_A)) move = Vector3Subtract(move, right);

        move.y = 0.0f;
        if (Vector3Length(move) > 0.01f) move = Vector3Scale(Vector3Normalize(move), 7.0f * dt);
        cam.position = Vector3Add(cam.position, move);

        if (cam.position.x > ARENA_HALF_SIZE) cam.position.x = ARENA_HALF_SIZE;
        if (cam.position.x < -ARENA_HALF_SIZE) cam.position.x = -ARENA_HALF_SIZE;
        if (cam.position.z > ARENA_HALF_SIZE) cam.position.z = ARENA_HALF_SIZE;
        if (cam.position.z < -ARENA_HALF_SIZE) cam.position.z = -ARENA_HALF_SIZE;

        cam.target = Vector3Add(cam.position, forward);

        for (int i = 0; i < MAX_BOTS; i++) {
            if (bots[i].hp <= 0.0f) {
                bots[i].respawnTimer -= dt;
                if (bots[i].respawnTimer <= 0.0f) spawn_bot(&bots[i]);
                continue;
            }

            Vector3 toPlayer = Vector3Subtract(cam.position, bots[i].pos);
            toPlayer.y = 0.0f;
            float dist = Vector3Length(toPlayer);

            if (dist > 0.05f) {
                Vector3 dir = Vector3Scale(Vector3Normalize(toPlayer), BOT_SPEED * dt);
                bots[i].pos = Vector3Add(bots[i].pos, dir);
            }
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && fireTimer <= 0.0f) {
            fireTimer = FIRE_COOLDOWN;
            int hit = try_shoot_bot(bots, MAX_BOTS, cam.position, forward, 50.0f);
            if (hit >= 0) {
                bots[hit].hp -= 40.0f;
                if (bots[hit].hp <= 0.0f) {
                    bots[hit].hp = 0.0f;
                    bots[hit].respawnTimer = frand_range(1.2f, 3.0f);
                    score++;
                }
            }
        }

        BeginDrawing();
        ClearBackground((Color){14, 18, 28, 255});

        BeginMode3D(cam);

        DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){ARENA_HALF_SIZE * 2.0f, ARENA_HALF_SIZE * 2.0f}, (Color){30, 42, 34, 255});

        DrawCubeWires((Vector3){0.0f, 1.0f, 0.0f}, ARENA_HALF_SIZE * 2.0f, 2.0f, ARENA_HALF_SIZE * 2.0f, (Color){80, 80, 95, 140});

        for (int i = 0; i < MAX_BOTS; i++) {
            if (bots[i].hp <= 0.0f) continue;
            Color c = (bots[i].hp > 50.0f) ? RED : ORANGE;
            DrawCube(bots[i].pos, BOT_SIZE, BOT_SIZE, BOT_SIZE, c);
            DrawCubeWires(bots[i].pos, BOT_SIZE, BOT_SIZE, BOT_SIZE, BLACK);
        }

        EndMode3D();

        DrawLine(SCREEN_WIDTH / 2 - 8, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 + 8, SCREEN_HEIGHT / 2, RAYWHITE);
        DrawLine(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 8, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 8, RAYWHITE);

        DrawRectangle(10, 10, 320, 84, (Color){0, 0, 0, 130});
        DrawText(TextFormat("SCORE: %d", score), 24, 24, 24, LIME);
        DrawText(TextFormat("BOTS: %d", MAX_BOTS), 24, 50, 20, RAYWHITE);
        DrawText(TextFormat("TEMPS: %.0f", timeLeft), 24, 72, 20, YELLOW);

        DrawText("WASD + souris | clic gauche pour tirer | ESC pour quitter", 20, SCREEN_HEIGHT - 34, 20, LIGHTGRAY);

        EndDrawing();
    }

    EnableCursor();
    CloseWindow();

    return 0;
}
