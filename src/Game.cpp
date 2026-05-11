#include "Game.h"
#include "SceneMain.h"
#include <SDL.h>
#include <SDL_image.h>

Game::Game()
{
}

Game::~Game()
{
    clean();
}

void Game::run()
{
    // 初始化 deltaTime（防止第一帧之前未定义）
    deltaTime = frameTime / 1000.0f;

    while (isRunning)
    {
        Uint32 frameStart = SDL_GetTicks();

        handleEvents();               // 无参数
        update(deltaTime);
        render();

        Uint32 frameEnd = SDL_GetTicks();
        Uint32 elapsed = frameEnd - frameStart;

        if (elapsed < frameTime) {
            SDL_Delay(frameTime - elapsed);
        }
        // 使用固定时间步长（帧率稳定）
        deltaTime = frameTime / 1000.0f;
    }
}

void Game::init()
{
    frameTime = static_cast<Uint32>(1000.0f / FPS);  // 仍然 16，但意图更清晰
    // 初始化 SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL could not initialize! SDL_Error: %s", SDL_GetError());
        isRunning = false;
        return;   // ★ 必须提前返回
    }

    // 创建窗口
    window = SDL_CreateWindow("SDL Shooter",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              windowWidth, windowHeight,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Window could not be created! SDL_Error: %s", SDL_GetError());
        isRunning = false;
        return;
    }

    // 创建渲染器
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Renderer could not be created! SDL_Error: %s", SDL_GetError());
        isRunning = false;
        return;
    }


    // 初始化 SDL_image
    if( IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_image could not initialize! SDL_image Error: %s", IMG_GetError());
        isRunning = false;
        return;
    }


    // 创建并初始化场景
    currentScene = new SceneMain();
    currentScene->init();    // ★ 别忘了调用 init
}

void Game::clean()
{
    // 先销毁场景
    if (currentScene) {
        currentScene->clean();
        delete currentScene;
        currentScene = nullptr;
    }

    // 安全销毁 SDL_image 资源
    IMG_Quit();



    // 安全销毁 SDL 资源（必须判空）
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

void Game::changeScene(Scene* scene)
{
    if (currentScene) {
        currentScene->clean();
        delete currentScene;
    }
    currentScene = scene;
    currentScene->init();
}

void Game::handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        // 将事件传递给当前场景处理
        if (currentScene) {
            currentScene->handleEvent(&event);
        }
    }
}

void Game::update(float deltaTime)
{
    if (currentScene) {
        currentScene->update(deltaTime);
    }
}

void Game::render()
{
    SDL_RenderClear(renderer);

    if (currentScene) {
        currentScene->render();
    }

    SDL_RenderPresent(renderer);
}