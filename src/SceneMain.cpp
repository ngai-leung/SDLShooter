#include "SceneMain.h"
#include "SDL.h"
#include "SDL_image.h"
#include "Game.h"

SceneMain::SceneMain() : game(*Game::getInstance())
{
}

SceneMain::~SceneMain() {}

void SceneMain::update() {}

void SceneMain::render()
{
    if (!player.texture) return;

    SDL_Rect playerRect = {
        static_cast<int>(player.position.x),
        static_cast<int>(player.position.y),
        player.width,
        player.height
    };
    SDL_RenderCopy(game.getRenderer(), player.texture, NULL, &playerRect);
}

void SceneMain::handleEvent(SDL_Event *event) {}

void SceneMain::init()
{
    player.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/SpaceShip.png");
    if (!player.texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to load texture: %s", IMG_GetError());
        return;
    }

    SDL_QueryTexture(player.texture, NULL, NULL, &player.width, &player.height);
    player.width /= 4;
    player.height /= 4;

    // 居中显示（如果想放底部改 y 即可）
    player.position.x = (game.getWindowWidth()  - player.width)  / 2.0f;
    player.position.y = static_cast<float>(game.getWindowHeight() - player.height - 20); // 20 是距离底部的距离
}

void SceneMain::clean()
{
    if (player.texture) {
        SDL_DestroyTexture(player.texture);
        player.texture = nullptr;
    }
}