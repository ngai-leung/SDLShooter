#ifndef SCENE_TITLE_H
#define SCENE_TITLE_H

#include "Scene.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>

class SceneTitle : public Scene {
public:
    SceneTitle();
    ~SceneTitle();

    void init() override;
    void update(float deltaTime) override;
    void render() override;
    void clean() override;
    void handleEvent(SDL_Event* event) override;

private:
    Mix_Music* bgm = nullptr ;




    // 你可以添加标题场景所需的资源
    SDL_Texture* background = nullptr;
    SDL_Texture* titleText = nullptr;
    // 文本闪烁等效果
    Uint32 startTime = 0;
};

#endif // SCENE_TITLE_H