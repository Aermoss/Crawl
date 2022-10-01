#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <chrono>

#include <raylib/raylib.h>

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

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

        bool collide(Entity* entity) {
            return CheckCollisionBoxes(
                {
                    {
                        position.x - size.x / 2,
                        position.y - size.y / 2,
                        position.z - size.z / 2
                    },
                    {
                        position.x + size.x / 2,
                        position.y + size.y / 2,
                        position.z + size.z / 2
                    },
                },
                {
                    {
                        entity->position.x - entity->size.x / 2,
                        entity->position.y - entity->size.y / 2,
                        entity->position.z - entity->size.z / 2
                    },
                    {
                        entity->position.x + entity->size.x / 2,
                        entity->position.y + entity->size.y / 2,
                        entity->position.z + entity->size.z / 2
                    }
                }
            );
        }
};

class App {
    public:
        App() {}
        ~App() {}

        Camera3D camera = {
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            60.0f, CAMERA_PERSPECTIVE
        };

        Font font;
        Entity* entity;
        std::vector <Entity*> obstacles;

        float speed;
        int max_obstacles;
        float score;
        bool dead;
        bool started;
        bool hue_state;
        bool paused;
        int hue;

        void change_hue() {
            if (hue_state) hue++;
            else hue--;
            if (hue == 360) hue_state = false;
            if (hue == 0) hue_state = true;
        }

        void reset() {
            entity->position = {0.0f, 0.0f, 0.0f};
            camera.position.x = entity->position.x;
            camera.target.x = camera.position.x;
            camera.target.y = 10.0f;
            speed = 0.15f;
            max_obstacles = 20;
            paused = false;
            score = 0;
            dead = false;
            started = false;
            hue_state = true;
            while ((hue = rand()) > RAND_MAX - (RAND_MAX - 5) % 360);
        }

        void run() {
            InitWindow(1200, 600, "Crawl");
            SetTargetFPS(75);
            srand((unsigned) time(NULL));
            intro();

            entity = new Entity(
                {0.0f, 0.0f, 0.0f},
                {2.0f, 2.0f, 2.0f}, RAYWHITE
            );

            while (!WindowShouldClose()) {
                update();
            }

            for (int i = 0; i < obstacles.size(); i++) {
                delete obstacles[i];
            }

            obstacles.clear();
            CloseWindow();          
        }

        void intro() {
            auto start = std::chrono::high_resolution_clock::now();

            while (std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start).count() < 1000) {
                BeginDrawing();
                ClearBackground({16, 16, 16, 255});
                DrawText("AERMOSS", 350, 250, 96, LIGHTGRAY);
                EndDrawing();
            }
        }

        void update() {
            if (!started) menu();
            else if (!dead) game();
            else game_over();
        }

        void menu() {
            BeginDrawing();
            ClearBackground({21, 21, 21, 255});
            if (IsKeyDown(KEY_Q)) DrawText((std::to_string(GetFPS()) + " FPS").c_str(), 10, 10, 25, VIOLET);
            DrawText("CRAWL", 425, 250, 100, VIOLET);
            DrawText("press space to start", 480, 350, 20, LIGHTGRAY);

            if (GetKeyPressed() == KEY_SPACE) {
                reset(); started = true;
                
                for (int i = 0; i < max_obstacles; i++) {
                    int n; while((n = rand()) > RAND_MAX - (RAND_MAX - 5) % 8); change_hue();
                    obstacles.push_back(new Entity({(float) ((n % 8) + 1) - 4, 0.0f, (i + 1) * -10.0f}, {2.0f, 2.0f, 2.0f}, ColorFromHSV(hue, 1, 1)));
                }
            }

            EndDrawing();
        }

        void game() {
            BeginDrawing();
            ClearBackground({21, 21, 21, 255});
            // DrawRectangleGradientV(0, 0, GetRenderWidth(), GetRenderHeight(), ColorFromHSV(hue, 1, 0.5f), {21, 21, 21, 255});
            BeginMode3D(camera);
            entity->draw();

            if (IsKeyPressed(KEY_P)) {
                if (paused) paused = false;
                else paused = true;
            }

            if (!paused) {
                if (IsKeyDown(KEY_A)) entity->position.x -= speed;
                if (IsKeyDown(KEY_D)) entity->position.x += speed;
                if (entity->position.x > 4.0f) entity->position.x = 4.0f;
                if (entity->position.x < -4.0f) entity->position.x = -4.0f;

                speed += 0.0001f; score += speed;
                entity->position.z -= speed;
                camera.target.z = entity->position.z;
                camera.position.y = entity->position.y + 7.0f;
                camera.position.z = entity->position.z + 10.0f + speed;
                camera.position.x = lerp(camera.position.x, entity->position.x, 0.1f);
                camera.target.x = lerp(camera.target.x, camera.position.x, 0.2f);
                camera.target.y = lerp(camera.target.y, entity->position.y, 0.2f);

                if (camera.fovy < 150.0f)
                    camera.fovy = 60.0f + speed;
            }

            for (int i = 0; i < obstacles.size(); i++) {
                if (entity->collide(obstacles[i])) {
                    dead = true; break;
                }

                if (obstacles[i]->position.z - entity->position.z > 10) {
                    delete obstacles[i]; obstacles.erase(obstacles.begin() + i);
                    int n; while((n = rand()) > RAND_MAX - (RAND_MAX - 5) % 6); change_hue();
                    obstacles.push_back(new Entity({(float) ((n % 8) + 1) - 4, 0.0f, entity->position.z - (max_obstacles * 10.0f)}, {2.0f, 2.0f, 2.0f}, ColorFromHSV(hue, 1, 1)));
                }

                obstacles[i]->draw();
            }

            DrawPlane({0.0f, -1.0f, entity->position.z}, {10, 500}, {16, 16, 16, 255});
            EndMode3D();
            if (paused) DrawText("paused", 10, 500, 50, ColorFromHSV(hue - max_obstacles, 1, 1));
            if (IsKeyDown(KEY_Q)) DrawText((std::to_string(GetFPS()) + " FPS").c_str(), 10, 10, 25, ColorFromHSV(hue - max_obstacles, 1, 1));
            DrawText((std::string("score: ") + std::to_string((int) score)).c_str(), 10, 570, 20, ColorFromHSV(hue - max_obstacles, 1, 1));
            EndDrawing();
        }

        void game_over() {
            BeginDrawing();
            ClearBackground({21, 21, 21, 255});
            if (IsKeyDown(KEY_Q)) DrawText((std::to_string(GetFPS()) + " FPS").c_str(), 10, 10, 25, RED);
            DrawText((std::string("score: ") + std::to_string((int) score)).c_str(), 10, 560, 30, LIGHTGRAY);
            DrawText("GAME OVER", 280, 250, 100, RED);
            DrawText("press space to play again", 450, 350, 20, LIGHTGRAY);

            if (GetKeyPressed() == KEY_SPACE) {
                for (int i = 0; i < obstacles.size(); i++) {
                    delete obstacles[i];
                }

                obstacles.clear();
                reset(); started = true;

                for (int i = 0; i < max_obstacles; i++) {
                    int n; while((n = rand()) > RAND_MAX - (RAND_MAX - 5) % 6); change_hue();
                    obstacles.push_back(new Entity({(float) ((n % 8) + 1) - 4, 0.0f, (i + 1) * -10.0f}, {2.0f, 2.0f, 2.0f}, ColorFromHSV(hue, 1, 1)));
                }
            }
            
            EndDrawing();
        }
};

int main(int argc, const char* argv[]) {
    App app;
    app.run();
    return 0;
}