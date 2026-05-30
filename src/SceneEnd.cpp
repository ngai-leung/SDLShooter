#include "SceneEnd.h"
#include "Game.h"
#include "SceneMain.h"
#include "SceneTitle.h"
#include <string>

SceneEnd::SceneEnd(int finalScore) : score(finalScore), font(nullptr), bgm(nullptr),
    gameOverTexture(nullptr), scoreTexture(nullptr), hintTexture(nullptr) {
}

SceneEnd::~SceneEnd() {
    clean();
}

void SceneEnd::init() {
    // 加载字体（复用 Game 中已加载的字体，或单独加载）
    font = TTF_OpenFont("assets/font/VonwaonBitmap-16px.ttf", 24);
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load font for SceneEnd: %s", TTF_GetError());
        // 可以尝试使用系统默认字体路径，若失败则继续（文字不显示）
    }

    // 加载背景音乐（可选）
    bgm = Mix_LoadMUS("assets/music/game_over.ogg");
    if (bgm) {
        Mix_PlayMusic(bgm, -1);
    } else {
        // 没有音乐文件也没关系，不播放即可
    }

    // 创建纹理
    updateTextures();
}

void SceneEnd::updateTextures() {
    SDL_Renderer* renderer = Game::getInstance()->getRenderer();

    if (gameOverTexture) SDL_DestroyTexture(gameOverTexture);
    if (scoreTexture) SDL_DestroyTexture(scoreTexture);
    if (hintTexture) SDL_DestroyTexture(hintTexture);

    SDL_Color red = {255, 0, 0, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, "GAME OVER", red);
    if (surface) {
        gameOverTexture = SDL_CreateTextureFromSurface(renderer, surface);
        gameOverWidth = surface->w;
        gameOverHeight = surface->h;
        SDL_FreeSurface(surface);
    }

    SDL_Color white = {255, 255, 255, 255};

    std::string scoreText = "Score: " + std::to_string(score);
    surface = TTF_RenderText_Solid(font, scoreText.c_str(), white);
    if (surface) {
        scoreTexture = SDL_CreateTextureFromSurface(renderer, surface);
        scoreWidth = surface->w;
        scoreHeight = surface->h;
        SDL_FreeSurface(surface);
    }

    // 只显示 R 键提示
    surface = TTF_RenderText_Solid(font, "Press R to Return to Title", white);
    if (surface) {
        hintTexture = SDL_CreateTextureFromSurface(renderer, surface);
        hintWidth = surface->w;
        hintHeight = surface->h;
        SDL_FreeSurface(surface);
    }
}

void SceneEnd::update(float deltaTime) {
    // 可以添加简单的动画效果（如文字缩放或颜色渐变），这里留空
}

void SceneEnd::render() {
    SDL_Renderer* renderer = Game::getInstance()->getRenderer();
    int winW = Game::getInstance()->getWindowWidth();
    int winH = Game::getInstance()->getWindowHeight();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (gameOverTexture) {
        SDL_Rect dst = { (winW - gameOverWidth) / 2, 180, gameOverWidth, gameOverHeight };
        SDL_RenderCopy(renderer, gameOverTexture, NULL, &dst);
    }
    if (scoreTexture) {
        SDL_Rect dst = { (winW - scoreWidth) / 2, 280, scoreWidth, scoreHeight };
        SDL_RenderCopy(renderer, scoreTexture, NULL, &dst);
    }
    if (hintTexture) {
        SDL_Rect dst = { (winW - hintWidth) / 2, 450, hintWidth, hintHeight };
        SDL_RenderCopy(renderer, hintTexture, NULL, &dst);
    }
}

void SceneEnd::clean() {
    if (gameOverTexture) SDL_DestroyTexture(gameOverTexture);
    if (scoreTexture) SDL_DestroyTexture(scoreTexture);
    if (hintTexture) SDL_DestroyTexture(hintTexture);
    if (font) TTF_CloseFont(font);
    if (bgm) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgm);
    }
}

void SceneEnd::handleEvent(SDL_Event* event) {
    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_r) {
        Game::getInstance()->changeScene(new SceneTitle());
    }
}