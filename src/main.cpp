#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

#include <raylib/raylib.h>

class Entity {
    public:
        Vector3 position;
        Vector3 size;
        Color color;

        Entity(Vector3 position, Vector3 size, Color color) : position(position), size(size), color(color) {}

        void draw(bool wires = false, Color wire_color = BLACK) {
            DrawCube(position, size.x, size.y, size.z, color);

            if (wires) {
                DrawCubeWires(position, size.x, size.y, size.z, wire_color);
            }
        }

        bool collide(Entity entity) {
            return CheckCollisionBoxes(
                {
                    {
                        position.x - size.x / 2,
                        position.y - size.y / 2,
                        position.z - size.z / 2
                    }, {
                        position.x + size.x / 2,
                        position.y + size.y / 2,
                        position.z + size.z / 2
                    },
                },
                {
                    {
                        entity.position.x - entity.size.x / 2,
                        entity.position.y - entity.size.y / 2,
                        entity.position.z - entity.size.z / 2
                    },
                    {
                        entity.position.x + entity.size.x / 2,
                        entity.position.y + entity.size.y / 2,
                        entity.position.z + entity.size.z / 2
                    }
                }
            );
        }
};

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

int main(int argc, const char* argv[]) {
    InitWindow(1200, 600, "Crawl");
    SetTargetFPS(75);

    Camera3D camera = {
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        60.0f, CAMERA_PERSPECTIVE
    };

    Entity entity(
        {0.0f, 0.0f, 0.0f},
        {2.0f, 2.0f, 2.0f}, RED
    );

    std::vector <Entity*> obstacles;

    float speed = 0.1f;

    int max_obstacles = 20;
    float score = 0.0f;

    bool dead = false;
    bool started = false;

    while (!WindowShouldClose()) {
        if (!started) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("CRAWL", 425, 250, 100, VIOLET);
            DrawText("press space to start", 480, 350, 20, GRAY);

            if (GetKeyPressed() == KEY_SPACE) {
                for (int i = 0; i < max_obstacles; i++) {
                    srand(i);
                    srand((unsigned) time(0) * rand());
                    obstacles.push_back(new Entity({(float) ((rand() % 8) + 1) - 4, 0.0f, (i + 1) * -10.0f}, {2.0f, 2.0f, 2.0f}, DARKGRAY));
                }

                started = true;
            }

            EndDrawing();
        } else if (!dead) {
            BeginDrawing();
            ClearBackground(LIGHTGRAY);
            BeginMode3D(camera);

            if (IsKeyDown(KEY_A))
                entity.position.x -= speed;

            if (IsKeyDown(KEY_D))
                entity.position.x += speed;

            if (entity.position.x > 4.0f)
                entity.position.x = 4.0f;

            if (entity.position.x < -4.0f)
                entity.position.x = -4.0f;

            speed += 0.00005f;
            score += speed;

            for (int i = 0; i < obstacles.size(); i++) {
                obstacles[i]->draw();

                if (entity.collide(*obstacles[i])) {
                    dead = true;
                    break;
                }

                if (obstacles[i]->position.z - entity.position.z > 10) {
                    delete obstacles[i];
                    obstacles.erase(obstacles.begin() + i);
                    srand((unsigned) time(0) * time(0));
                    obstacles.push_back(new Entity({(float) ((rand() % 8) + 1) - 4, 0.0f, entity.position.z - (max_obstacles * 10.0f)}, {2.0f, 2.0f, 2.0f}, DARKGRAY));
                }
            }

            entity.draw();

            entity.position.z -= speed;
            camera.target.z = entity.position.z;
            camera.position.x = 0.0f;
            camera.position.y = entity.position.y + 7.0f;
            camera.position.z = entity.position.z + 10.0f + speed;

            camera.position.x = lerp(camera.position.x, entity.position.x, speed);
            camera.target.x = camera.position.x / 2;
            camera.fovy = 60.0f + speed;

            if (camera.fovy > 150.0f)
                camera.fovy = 150.0f;

            DrawPlane({0.0f, -1.0f, entity.position.z}, {10, 500}, RAYWHITE);
            EndMode3D();
            DrawFPS(10, 10);
            DrawText((std::string("score: ") + std::to_string((int) score)).c_str(), 10, 570, 20, BLUE);
            EndDrawing();
        } else {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText((std::string("score: ") + std::to_string((int) score)).c_str(), 10, 560, 30, BLUE);
            DrawText("GAME OVER", 280, 250, 100, RED);
            DrawText("press space to play again", 450, 350, 20, GRAY);

            if (GetKeyPressed() == KEY_SPACE) {
                for (int i = 0; i < obstacles.size(); i++) {
                    delete obstacles[i];
                }

                obstacles.clear();

                for (int i = 0; i < max_obstacles; i++) {
                    srand(i);
                    srand((unsigned) time(0) * rand());
                    obstacles.push_back(new Entity({(float) ((rand() % 8) + 1) - 4, 0.0f, (i + 1) * -10.0f}, {2.0f, 2.0f, 2.0f}, DARKGRAY));
                }

                entity.position = {0.0f, 0.0f, 0.0f};
                score = 0.0f;
                speed = 0.1f;
                dead = false;
            }
            
            EndDrawing();
        }
    }

    for (int i = 0; i < obstacles.size(); i++) {
        delete obstacles[i];
    }

    obstacles.clear();
    
    CloseWindow();
    return 0;
}