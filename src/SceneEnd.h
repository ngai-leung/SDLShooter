#ifndef SCENE_END_H
#define SCENE_END_H

#include "Scene.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

class SceneEnd : public Scene {
public:
    SceneEnd(int finalScore);
    ~SceneEnd();

    void init() override;
    void update(float deltaTime) override;
    void render() override;
    void clean() override;
    void handleEvent(SDL_Event* event) override;

private:
    int score;                      // 最终得分
    TTF_Font* font;                 // 用于显示文字的字体
    Mix_Music* bgm;                 // 结束场景背景音乐（可选）
    SDL_Texture* gameOverTexture;   // 缓存 "GAME OVER" 纹理
    SDL_Texture* scoreTexture;      // 缓存得分纹理
    SDL_Texture* hintTexture;       // 缓存提示文字纹理
    int gameOverWidth, gameOverHeight;
    int scoreWidth, scoreHeight;
    int hintWidth, hintHeight;

    void updateTextures();          // 更新得分纹理（当分数变化时调用）
};

#endif // SCENE_END_H