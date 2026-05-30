// Game.h 修改后
#ifndef GAME_H
#define GAME_H

#include "Object.h"
#include "Scene.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>


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
    void update(float deltaTime);
    void render();
    void backgroundUpdate(float deltaTime);
    void backgroundRender();
    void quitGame(){ isRunning = false; }

    SDL_Window* getWindow() { return window; }
    SDL_Renderer* getRenderer() { return renderer; }
    int getWindowWidth() { return windowWidth; }  //获取窗口宽度
    int getWindowHeight() { return windowHeight; }   //获取窗口高度
    TTF_Font* getTitleFont() const { return titleFont; }
    TTF_Font* getTextFont() const { return textFont; }

private:
    Game();
    //删除拷贝和赋值构造函数
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    TTF_Font* titleFont;
    TTF_Font* textFont;


    bool isRunning = true;
    Scene* currentScene = nullptr;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    int windowWidth = 600;
    int windowHeight = 800;
    int FPS = 60;
    Uint32 frameTime;
    float deltaTime;

    Background nearStars;
    Background farStars;
};

#endif

