// Game.h 修改后
#ifndef GAME_H
#define GAME_H

#include "Scene.h"
#include <SDL.h>

class Game
{
public:
    Game();
    ~Game();
    void run();
    void init();
    void clean();
    void changeScene(Scene* scene);

    void handleEvents();
    void update();
    void render();

private:
    bool isRunning = true;
    Scene* currentScene = nullptr;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    int windowWidth = 600;
    int windowHeight = 800;
};

#endif

