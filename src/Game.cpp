#include "Game.h"
#include "SceneMain.h"
#include "SceneTitle.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

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

    // 初始化 SDL_mixer
    int flags = MIX_INIT_MP3 | MIX_INIT_OGG;
    int initted = Mix_Init(flags);
    if ((initted & flags) != flags) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_mixer could not initialize! Required: %d, Got: %d, Error: %s", flags, initted, Mix_GetError());
        isRunning = false;
        return;
    }

    //打开音频设备
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_mixer could not initialize! SDL_mixer Error: %s", Mix_GetError());
        isRunning = false;
        return;
    }
    Mix_AllocateChannels(32);           // 分配 32 个音频通道
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4); // 设置音量
    Mix_Volume(-1, MIX_MAX_VOLUME / 8); // 设置音效音量

    // 初始化 SDL_ttf
    if (TTF_Init() == -1) {

        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_ttf could not initialize! SDL_ttf Error: %s", TTF_GetError());
        isRunning = false;
        return;
    }

    //初始化背景卷轴
    nearStars.texture = IMG_LoadTexture(renderer, "assets/image/Stars-A.png");
    SDL_QueryTexture(nearStars.texture, nullptr, nullptr, &nearStars.width, &nearStars.height);
    farStars.texture = IMG_LoadTexture(renderer, "assets/image/Stars-B.png");
    SDL_QueryTexture(farStars.texture, nullptr, nullptr, &farStars.width, &farStars.height);
    farStars.speed = 20;

    //载入字体
    titleFont = TTF_OpenFont("assets/font/VonwaonBitmap-16px.ttf", 64);
    textFont = TTF_OpenFont("assets/font/VonwaonBitmap-16px.ttf", 32);
    if(titleFont == nullptr || textFont == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_ttf could not initialize! SDL_ttf Error: %s", TTF_GetError());
        isRunning = false;
        return;
    }

    // 创建并初始化场景
    currentScene = new SceneTitle();
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
    //释放字体
    if(titleFont != nullptr) {
        TTF_CloseFont(titleFont);
    }
    if(textFont != nullptr) {
        TTF_CloseFont(textFont);
    }
    if(nearStars.texture != nullptr) {
        SDL_DestroyTexture(nearStars.texture);
    }
    if(farStars.texture != nullptr) {
        SDL_DestroyTexture(farStars.texture);
    }


    // 安全销毁 SDL_image 资源
    IMG_Quit();

    // 安全销毁 SDL_mixer 资源
    Mix_CloseAudio();
    Mix_Quit();

    //清理 SDL_ttf 资源
    TTF_Quit();
    


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

void Game::changeScene(Scene* scene) {
    SDL_Log("changeScene called");
    if (currentScene) {
        SDL_Log("Cleaning old scene");
        currentScene->clean();
        delete currentScene;
        SDL_Log("Old scene deleted");
    }
    currentScene = scene;
    SDL_Log("Initializing new scene");
    currentScene->init();
    SDL_Log("New scene initialized");
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
    backgroundUpdate(deltaTime);
    if (currentScene) {
        currentScene->update(deltaTime);
    }
}

void Game::render()
{
    SDL_RenderClear(renderer);

    //渲染星空背景
    backgroundRender();

    if (currentScene) {
        currentScene->render();
    }

    SDL_RenderPresent(renderer);
}

void Game::backgroundUpdate(float deltaTime)
{
    // 近处星星（速度快）
    nearStars.offset += nearStars.speed * deltaTime;
    if (nearStars.offset >= 0) {
        nearStars.offset = -nearStars.height;
    }

    // 远处星星（速度慢）
    farStars.offset += farStars.speed * deltaTime;
    if (farStars.offset >= 0) {
        farStars.offset = -farStars.height;
    }
}

void Game::backgroundRender()
{
    // 渲染远处星星（速度慢，滚动慢）
    for (int y = farStars.offset; y < windowHeight; y += farStars.height) {
        for (int x = 0; x < windowWidth; x += farStars.width) {
            SDL_Rect dstRect = { x, y, farStars.width, farStars.height };
            SDL_RenderCopy(renderer, farStars.texture, nullptr, &dstRect);
        }
    }

    // 渲染近处星星（速度快，滚动快）
    for (int y = nearStars.offset; y < windowHeight; y += nearStars.height) {
        for (int x = 0; x < windowWidth; x += nearStars.width) {
            SDL_Rect dstRect = { x, y, nearStars.width, nearStars.height };
            SDL_RenderCopy(renderer, nearStars.texture, nullptr, &dstRect);
        }
    }
}
