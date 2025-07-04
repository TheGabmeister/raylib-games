#include "raylib.h"
#include <math.h>

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 900

#define TANK_SIZE 32
#define TANK_SPEED 2.0f
#define TANK_ROT_SPEED 2.5f

#define BULLET_SPEED 5.0f
#define BULLET_RADIUS 4
#define MAX_BULLETS 3

#define MAX_OBSTACLES 6
#define OBSTACLE_SIZE 64

typedef struct {
    Rectangle rect;
} Obstacle;

typedef struct {
    Vector2 position;
    float rotation;
    Color color;
    int upKey, downKey, leftKey, rightKey, fireKey;
    int bulletCount;
    struct Bullet *bullets[MAX_BULLETS];
} Tank;

typedef struct Bullet {
    Vector2 position;
    float rotation;
    bool active;
} Bullet;

static Tank tank1, tank2;
static Bullet bullets1[MAX_BULLETS], bullets2[MAX_BULLETS];
static Obstacle obstacles[MAX_OBSTACLES];

static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);

static void UpdateTank(Tank *tank, Bullet *bullets[]);
static void DrawTank(Tank *tank);
static void UpdateBullets(Bullet *bullets[]);
static void DrawBullets(Bullet *bullets[]);

static void DrawObstacles(void);
static bool CheckTankObstacleCollision(Vector2 nextPos);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "tank");
    InitGame();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateGame();
        DrawGame();
    }

    UnloadGame();
    CloseWindow();
    return 0;
}

void InitGame(void)
{
    tank1.position = (Vector2){100, 100};
    tank1.rotation = 0;
    tank1.color = GREEN;
    tank1.upKey = KEY_W;
    tank1.downKey = KEY_S;
    tank1.leftKey = KEY_A;
    tank1.rightKey = KEY_D;
    tank1.fireKey = KEY_SPACE;
    tank1.bulletCount = 0;
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets1[i].active = false;
        tank1.bullets[i] = &bullets1[i];
    }

    tank2.position = (Vector2){SCREEN_WIDTH - 100, SCREEN_HEIGHT - 100};
    tank2.rotation = 180;
    tank2.color = YELLOW;
    tank2.upKey = KEY_UP;
    tank2.downKey = KEY_DOWN;
    tank2.leftKey = KEY_LEFT;
    tank2.rightKey = KEY_RIGHT;
    tank2.fireKey = KEY_ENTER;
    tank2.bulletCount = 0;
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets2[i].active = false;
        tank2.bullets[i] = &bullets2[i];
    }

    // Place obstacles at fixed positions (you can randomize or adjust as needed)
    obstacles[0].rect = (Rectangle){ SCREEN_WIDTH/2 - OBSTACLE_SIZE/2, 200, OBSTACLE_SIZE, OBSTACLE_SIZE };
    obstacles[1].rect = (Rectangle){ 100, 400, OBSTACLE_SIZE, OBSTACLE_SIZE };
    obstacles[2].rect = (Rectangle){ SCREEN_WIDTH - 100 - OBSTACLE_SIZE, 400, OBSTACLE_SIZE, OBSTACLE_SIZE };
    obstacles[3].rect = (Rectangle){ SCREEN_WIDTH/2 - OBSTACLE_SIZE/2, 600, OBSTACLE_SIZE, OBSTACLE_SIZE };
    obstacles[4].rect = (Rectangle){ 200, 700, OBSTACLE_SIZE, OBSTACLE_SIZE };
    obstacles[5].rect = (Rectangle){ SCREEN_WIDTH - 200 - OBSTACLE_SIZE, 700, OBSTACLE_SIZE, OBSTACLE_SIZE };

}

void UpdateGame(void)
{
    UpdateTank(&tank1, tank1.bullets);
    UpdateTank(&tank2, tank2.bullets);
    UpdateBullets(tank1.bullets);
    UpdateBullets(tank2.bullets);
}

void DrawGame(void)
{
    BeginDrawing();
    ClearBackground(DARKGRAY);

    DrawTank(&tank1);
    DrawTank(&tank2);
    DrawBullets(tank1.bullets);
    DrawBullets(tank2.bullets);
    DrawObstacles();

    EndDrawing();
}

void UnloadGame(void)
{
    // No dynamic resources to unload
}

void UpdateTank(Tank *tank, Bullet *bullets[])
{
    // Movement
    float nextRot = tank->rotation;
    if (IsKeyDown(tank->leftKey)) nextRot -= TANK_ROT_SPEED;
    if (IsKeyDown(tank->rightKey)) nextRot += TANK_ROT_SPEED;

    Vector2 forward = {
        sinf(DEG2RAD * nextRot),
        -cosf(DEG2RAD * nextRot)
    };

    Vector2 nextPos = tank->position;

    if (IsKeyDown(tank->upKey)) {
        Vector2 tryPos = {
            tank->position.x + forward.x * TANK_SPEED,
            tank->position.y + forward.y * TANK_SPEED
        };
        if (!CheckTankObstacleCollision(tryPos)) {
            nextPos = tryPos;
        }
    }
    if (IsKeyDown(tank->downKey)) {
        Vector2 tryPos = {
            tank->position.x - forward.x * TANK_SPEED,
            tank->position.y - forward.y * TANK_SPEED
        };
        if (!CheckTankObstacleCollision(tryPos)) {
            nextPos = tryPos;
        }
    }

    // Clamp to screen
    if (nextPos.x < TANK_SIZE/2) nextPos.x = TANK_SIZE/2;
    if (nextPos.x > SCREEN_WIDTH - TANK_SIZE/2) nextPos.x = SCREEN_WIDTH - TANK_SIZE/2;
    if (nextPos.y < TANK_SIZE/2) nextPos.y = TANK_SIZE/2;
    if (nextPos.y > SCREEN_HEIGHT - TANK_SIZE/2) nextPos.y = SCREEN_HEIGHT - TANK_SIZE/2;

    // Only update rotation if not colliding
    tank->rotation = nextRot;
    tank->position = nextPos;

    // Fire
    if (IsKeyPressed(tank->fireKey)) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i]->active) {
                bullets[i]->active = true;
                bullets[i]->position = (Vector2){
                    tank->position.x + forward.x * (TANK_SIZE/2 + BULLET_RADIUS),
                    tank->position.y + forward.y * (TANK_SIZE/2 + BULLET_RADIUS)
                };
                bullets[i]->rotation = tank->rotation;
                break;
            }
        }
    }
}

void DrawTank(Tank *tank)
{
    Vector2 center = tank->position;
    float rot = tank->rotation;

    // Draw body
    DrawRectanglePro(
        (Rectangle){center.x, center.y, TANK_SIZE, TANK_SIZE * 0.6f},
        (Vector2){TANK_SIZE/2, TANK_SIZE*0.3f},
        rot,
        tank->color
    );
    // Draw barrel
    Vector2 barrelEnd = {
        center.x + sinf(DEG2RAD * rot) * (TANK_SIZE/2),
        center.y - cosf(DEG2RAD * rot) * (TANK_SIZE/2)
    };
    DrawLineEx(center, barrelEnd, 6, BLACK);
}

void UpdateBullets(Bullet *bullets[])
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i]->active) {
            bullets[i]->position.x += sinf(DEG2RAD * bullets[i]->rotation) * BULLET_SPEED;
            bullets[i]->position.y -= cosf(DEG2RAD * bullets[i]->rotation) * BULLET_SPEED;

            // Out of bounds
            if (bullets[i]->position.x < 0 || bullets[i]->position.x > SCREEN_WIDTH ||
                bullets[i]->position.y < 0 || bullets[i]->position.y > SCREEN_HEIGHT) {
                bullets[i]->active = false;
            }
        }
    }
}

void DrawBullets(Bullet *bullets[])
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i]->active) {
            DrawCircleV(bullets[i]->position, BULLET_RADIUS, WHITE);
        }
    }
}

void DrawObstacles(void)
{
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        DrawRectangleRec(obstacles[i].rect, GRAY);
        DrawRectangleLinesEx(obstacles[i].rect, 2, BLACK);
    }
}

bool CheckTankObstacleCollision(Vector2 nextPos)
{
    Rectangle tankRect = {
        nextPos.x - TANK_SIZE/2,
        nextPos.y - TANK_SIZE*0.3f,
        TANK_SIZE,
        TANK_SIZE * 0.6f
    };
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (CheckCollisionRecs(tankRect, obstacles[i].rect)) {
            return true;
        }
    }
    return false;
}