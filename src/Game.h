// Game.h 修改后
#ifndef GAME_H
#define GAME_H

#include "Scene.h"
#include <SDL.h>

class Game
{
public:
    static Game* getInstance(){
        static Game instance;
        return &instance;
    }
    ~Game();
    void run();
    void init();
    void clean();
    void changeScene(Scene* scene);

    void handleEvents();
    void update();
    void render();

    SDL_Window* getWindow() { return window; }
    SDL_Renderer* getRenderer() { return renderer; }
    int getWindowWidth() { return windowWidth; }  //获取窗口宽度
    int getWindowHeight() { return windowHeight; }   //获取窗口高度

private:
    Game();
    //删除拷贝和赋值构造函数
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    bool isRunning = true;
    Scene* currentScene = nullptr;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    int windowWidth = 600;
    int windowHeight = 800;
};

#endif

