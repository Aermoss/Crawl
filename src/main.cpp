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
        Vector3 position, size;
        Color color;

        Entity(Vector3 position, Vector3 size, Color color) : position(position), size(size), color(color) {}

        void Draw() const {
            DrawCube(this->position, this->size.x, this->size.y, this->size.z, this->color);
        }

        bool Collide(Entity* other) const {
            return CheckCollisionBoxes({
                { this->position.x - this->size.x / 2, this->position.y - this->size.y / 2, this->position.z - this->size.z / 2 },
                { this->position.x + this->size.x / 2, this->position.y + this->size.y / 2, this->position.z + this->size.z / 2 } }, {
                { other->position.x - other->size.x / 2, other->position.y - other->size.y / 2, other->position.z - other->size.z / 2 },
                { other->position.x + other->size.x / 2, other->position.y + other->size.y / 2, other->position.z + other->size.z / 2 }
            });
        }
};

class App {
    private:
        Camera3D camera = {
            { 0.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            60.0f, CAMERA_PERSPECTIVE
        };

        Font font;
        Entity* entity;
        std::vector<Entity*> obstacles;
        uint32_t width, height;
        Sound music;

        const float bpm = 120.0f;
        float speed, score, timeScale, targetX, sinceBeat = 0.0f;
        bool dead = false, started = false, hueState, paused;
        int hue, maxObstacles, passBeat = 7;

        void DrawTextCentered(const char* text, int offset, int size, Color color) {
            DrawText(text, width / 2 - MeasureText(text, size) / 2, height / 2 - size / 2 + offset, size, color);
        }

    public:
        App() {}
        ~App() {}

        void ChangeHue() {
            if (hue >= 360) hueState = false;
            if (hue <= 0) hueState = true;
            if (hueState) { hue++; }
            else { hue--; }
        }

        void Reset() {
            entity->position = {0.0f, 0.0f, 0.0f};
            entity->position.x = 0.0f, targetX = 0.0f;
            camera.position.x = entity->position.x, camera.target.x = camera.position.x, camera.target.y = 10.0f;
            speed = 0.15f, score = 0.0f, timeScale = 0.0f, sinceBeat = 0.0f;
            paused = false, dead = false, started = false, hueState = true;
            while ((hue = rand()) > RAND_MAX - (RAND_MAX - 5) % 6);
            maxObstacles = 20, passBeat = 7;
            PlaySound(music);
        }

        void Run() {
            InitWindow(0, 0, "Crawl");
            srand((unsigned) time(NULL));

            width = static_cast<uint32_t>(1600.0f / 1920.0f * GetMonitorWidth(0));
            height = static_cast<uint32_t>(900.0f / 1080.0f * GetMonitorHeight(0));

            SetTargetFPS(GetMonitorRefreshRate(0));
            SetWindowPosition(GetMonitorWidth(0) / 2 - width / 2, GetMonitorHeight(0) / 2 - height / 2);
            SetWindowSize(width, height);

            Image icon = LoadImage("res/icon.png");
            SetWindowIcon(icon);
            UnloadImage(icon);

            RenderTexture2D firstTarget = LoadRenderTexture(width, height);
            RenderTexture2D secondTarget = LoadRenderTexture(width, height);
            Shader bloomShader = LoadShader(0, "shaders/bloom.frag");
            Shader crtShader = LoadShader(0, "shaders/crt.frag");

            float size[2] = { (float) width, (float) height };
            SetShaderValue(bloomShader, GetShaderLocation(bloomShader, "size"), size, SHADER_UNIFORM_VEC2);
            SetShaderValue(crtShader, GetShaderLocation(crtShader, "size"), size, SHADER_UNIFORM_VEC2);

            auto start = std::chrono::high_resolution_clock::now();

            entity = new Entity(
                {0.0f, 0.0f, 0.0f},
                {2.0f, 2.0f, 2.0f}, RAYWHITE
            );

            Color backgroundColor = VIOLET;
            bool introState = true, deathState = false, startState = false;

            InitAudioDevice();
            music = LoadSound("res/music.wav"), sinceBeat = 0.0f;
            PlaySound(music);

            while (!WindowShouldClose()) {
                if (IsSoundPlaying(music))
                    sinceBeat += GetFrameTime();

                if (sinceBeat >= 60.0f / bpm) {
                    sinceBeat = sinceBeat - 60.0f / bpm;

                    if (passBeat != 0) { passBeat--; }
                    else {
                        backgroundColor = (started && dead) ? RED : ((!started && !dead) ? VIOLET : ColorFromHSV(hue, 1, 1));
                    }
                }

                if (deathState != dead || startState != started) {
                    startState = started, deathState = dead;
                    backgroundColor = BLACK;
                }

                if (introState) {
                    introState = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - start).count() < 1000.0f;
                    if (!introState) backgroundColor = BLACK;
                } else {
                    if (!(started && !dead && paused)) {
                        backgroundColor.r = lerp(backgroundColor.r, 0.0f, GetFrameTime() * 3.0f);
                        backgroundColor.g = lerp(backgroundColor.g, 0.0f, GetFrameTime() * 3.0f);
                        backgroundColor.b = lerp(backgroundColor.b, 0.0f, GetFrameTime() * 3.0f);
                    }
                }

                BeginTextureMode(firstTarget);
                ClearBackground(backgroundColor);

                if (introState) {
                    DrawTextCentered("AERMOSS", 0, 200, BLACK);
                } else {
                    if (!started) { Menu(); }
                    else if (!dead) { Game(); }
                    else { GameOver(); }
                }

                EndTextureMode();

                BeginTextureMode(secondTarget);
                BeginShaderMode(bloomShader);
                DrawTextureRec(firstTarget.texture, { 0, 0, (float) firstTarget.texture.width, (float) -firstTarget.texture.height }, { 0, 0 }, WHITE);
                EndShaderMode();
                EndTextureMode();

                BeginDrawing();
                BeginShaderMode(crtShader);
                DrawTextureRec(secondTarget.texture, { 0, 0, (float) secondTarget.texture.width, (float) -secondTarget.texture.height }, { 0, 0 }, WHITE);
                EndShaderMode();
                EndDrawing();

                if (IsKeyPressed(KEY_F1))
                    TakeScreenshot("screenshot.png");
            }

            for (Entity* entity : obstacles)
                delete entity;

            UnloadShader(bloomShader);
            UnloadShader(crtShader);

            UnloadRenderTexture(firstTarget);
            UnloadRenderTexture(secondTarget);

            UnloadSound(music);
            CloseAudioDevice();

            obstacles.clear();
            CloseWindow();
        }

        void Menu() {
            if (IsKeyDown(KEY_Q)) DrawText((std::to_string(GetFPS()) + " FPS").c_str(), 10, 10, 25, VIOLET);
            DrawTextCentered("CRAWL", 0, 100, VIOLET);
            DrawTextCentered("press space to start", 100 / 2 + 20 / 2, 20, LIGHTGRAY);

            if (GetKeyPressed() == KEY_SPACE) {
                Reset(), started = true;
                
                for (int i = 0; i < maxObstacles; i++) {
                    int n; while((n = rand()) > RAND_MAX - (RAND_MAX - 5) % 8); ChangeHue();
                    obstacles.push_back(new Entity({(float) ((n % 8) + 1) - 4, 0.0f, (i + 1) * -10.0f}, {2.0f, 2.0f, 2.0f}, ColorFromHSV(hue, 1, 1)));
                }
            }
        }

        void Game() {
            BeginMode3D(camera);
            entity->Draw();

            if (IsKeyPressed(KEY_P)) {
                paused = !paused;

                if (paused) {
                    PauseSound(music);
                } else {
                    ResumeSound(music);
                }
            }

            timeScale = lerp(timeScale, IsKeyDown(KEY_F) ? 25.0f : 75.0f, GetFrameTime() * 2.0f);
            
            if (!paused) {
                if (IsKeyDown(KEY_A)) targetX -= speed * GetFrameTime() * timeScale;
                if (IsKeyDown(KEY_D)) targetX += speed * GetFrameTime() * timeScale;
                if (targetX > 4.0f) targetX = 4.0f;
                if (targetX < -4.0f) targetX = -4.0f;

                speed += 0.0001f * GetFrameTime() * timeScale;
                score += speed * GetFrameTime() * timeScale;
                entity->position.x = lerp(entity->position.x, targetX, 0.2f * GetFrameTime() * timeScale);
                entity->position.z -= speed * GetFrameTime() * timeScale;
                camera.target.z = entity->position.z;
                camera.position.y = entity->position.y + 7.0f;
                camera.position.z = entity->position.z + 10.0f + speed;
                camera.position.x = lerp(camera.position.x, entity->position.x, 0.1f * GetFrameTime() * timeScale);
                camera.target.x = lerp(camera.target.x, camera.position.x, 0.2f * GetFrameTime() * timeScale);
                camera.target.y = lerp(camera.target.y, entity->position.y, 0.2f * GetFrameTime() * timeScale);

                if (camera.fovy < 150.0f)
                    camera.fovy = 60.0f + speed;
            } else {
                timeScale = 0.0f;
            }

            for (size_t i = 0; i < obstacles.size(); i++) {
                if (entity->Collide(obstacles[i])) {
                    dead = true; break;
                } if (obstacles[i]->position.z - entity->position.z > 10) {
                    delete obstacles[i];
                    obstacles.erase(obstacles.begin() + i);

                    int n; {
                        while ((n = rand()) > RAND_MAX - (RAND_MAX - 5) % 6);
                    } ChangeHue();

                    obstacles.push_back(new Entity({(float) ((n % 8) + 1) - 4, 0.0f, entity->position.z - (maxObstacles * 10.0f)}, {2.0f, 2.0f, 2.0f}, ColorFromHSV(hue, 1, 1)));
                } obstacles[i]->Draw();
            }

            DrawPlane({0.0f, -1.0f, entity->position.z}, {10, 500}, {5, 5, 5, 255});
            EndMode3D();

            if (paused) DrawText("paused", 10, height - 10 - 30 - 50, 50, ColorFromHSV(hue - maxObstacles, 1, 1));
            if (IsKeyDown(KEY_Q)) DrawText((std::to_string(GetFPS()) + " FPS").c_str(), 10, 10, 25, ColorFromHSV(hue - maxObstacles, 1, 1));
            DrawText((std::string("score: ") + std::to_string((int) score)).c_str(), 10, height - 10 - 30, 30, ColorFromHSV(hue - maxObstacles, 1, 1));
        }

        void GameOver() {
            if (IsKeyDown(KEY_Q)) DrawText((std::to_string(GetFPS()) + " FPS").c_str(), 10, 10, 25, RED);
            DrawText((std::string("score: ") + std::to_string((int) score)).c_str(), 10, height - 10 - 30, 30, LIGHTGRAY);
            DrawTextCentered("GAME OVER", 0, 100, RED);
            DrawTextCentered("press space to play again", 100 / 2 + 20 / 2, 20, LIGHTGRAY);

            if (GetKeyPressed() == KEY_SPACE) {
                for (Entity* entity : obstacles)
                    delete entity;

                obstacles.clear();
                Reset(), started = true;

                for (int i = 0; i < maxObstacles; i++) {
                    int n; {
                        while ((n = rand()) > RAND_MAX - (RAND_MAX - 5) % 6);
                    } ChangeHue();

                    obstacles.push_back(new Entity({(float) ((n % 8) + 1) - 4, 0.0f, (i + 1) * -10.0f}, {2.0f, 2.0f, 2.0f}, ColorFromHSV(hue, 1, 1)));
                }
            }
        }
};

int main(int argc, const char* argv[]) {
    App app;
    app.Run();
    return 0;
}