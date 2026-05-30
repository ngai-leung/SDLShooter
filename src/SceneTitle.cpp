#include "SceneTitle.h"
#include "Game.h"
#include "SceneMain.h"
#include <SDL_image.h>
#include <string>

SceneTitle::SceneTitle() : bgm(nullptr)   // 初始化指针
{
}

SceneTitle::~SceneTitle()
{
    clean();
}

void SceneTitle::init()
{
    // 加载背景音乐（可选）
    bgm = Mix_LoadMUS("assets/music/06_Battle_in_Space_Intro.ogg");
    if (!bgm) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load title music! %s\n", Mix_GetError());
    } else {
        Mix_PlayMusic(bgm, -1);
    }

    // 可以加载背景图片等资源
    // background = IMG_LoadTexture(Game::getInstance()->getRenderer(), "assets/image/title_bg.png");
    startTime = SDL_GetTicks();
}

void SceneTitle::update(float deltaTime)
{
    // 可添加简单动画，如文字闪烁或背景移动
    // 这里留空，仅保持帧率稳定
}

void SceneTitle::render()
{
    SDL_Renderer* renderer = Game::getInstance()->getRenderer();

    // 使用 Game 中加载的字体显示标题文字
    TTF_Font* titleFont = Game::getInstance()->getTitleFont();
    TTF_Font* textFont  = Game::getInstance()->getTextFont();

    if (titleFont && textFont) {
        SDL_Color white = {255, 255, 255, 255};

        // 标题文字 "SPACE SHOOTER"（不闪烁，保持不变）
        SDL_Surface* titleSurf = TTF_RenderText_Solid(titleFont, "SPACE SHOOTER", white);
        if (titleSurf) {
            SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
            int w = titleSurf->w, h = titleSurf->h;
            SDL_Rect dst = {(Game::getInstance()->getWindowWidth() - w) / 2, 200, w, h};
            SDL_RenderCopy(renderer, titleTex, NULL, &dst);
            SDL_FreeSurface(titleSurf);
            SDL_DestroyTexture(titleTex);
        }

        // 提示文字 "Press ENTER to start"（闪烁效果）
        // 计算闪烁的 alpha 值（周期约 1 秒）
        Uint32 now = SDL_GetTicks();
        // 方法一：平滑正弦波
        // float alpha = (sin(now * 0.006) + 1) / 2.0f * 255;
        // 方法二：简单快速闪烁（每 0.5 秒切换）
        Uint8 alpha = (now % 1000) < 500 ? 255 : 80;

        SDL_Surface* infoSurf = TTF_RenderText_Solid(textFont, "Press ENTER to start", white);
        if (infoSurf) {
            SDL_Texture* infoTex = SDL_CreateTextureFromSurface(renderer, infoSurf);
            // 设置纹理透明度
            SDL_SetTextureAlphaMod(infoTex, alpha);
            int w = infoSurf->w, h = infoSurf->h;
            SDL_Rect dst = {(Game::getInstance()->getWindowWidth() - w) / 2, 500, w, h};
            SDL_RenderCopy(renderer, infoTex, NULL, &dst);
            SDL_FreeSurface(infoSurf);
            SDL_DestroyTexture(infoTex);
        }
    }
}

void SceneTitle::clean()
{
    if (bgm) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgm);
        bgm = nullptr;
    }
    if (background) {
        SDL_DestroyTexture(background);
        background = nullptr;
    }
    // titleText 等资源也需释放
}

void SceneTitle::handleEvent(SDL_Event* event)
{
    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN) {
        // 停止当前音乐（可选）
        Mix_HaltMusic();
        // 切换到主游戏场景
        Game::getInstance()->changeScene(new SceneMain());
    }
}