#include "SceneMain.h"
#include "SDL.h"
#include "SDL_image.h"
#include "Game.h"

SceneMain::SceneMain() : game(*Game::getInstance())
{
}

SceneMain::~SceneMain() {}

void SceneMain::update(float deltaTime) 
{
    keyboardControls(deltaTime);
}

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


//实现键盘控制飞机移动
/**
 * 处理键盘控制函数
 * 根据按下的WASD键来控制玩家位置
 */
void SceneMain::keyboardControls(float deltaTime)
{
    auto keyboardState = SDL_GetKeyboardState(NULL);
    if(keyboardState[SDL_SCANCODE_W]){
        player.position.y -= deltaTime * player.speed;
    }
    if(keyboardState[SDL_SCANCODE_S]){
        player.position.y += deltaTime * player.speed;
    }
    if(keyboardState[SDL_SCANCODE_A]){
        player.position.x -= deltaTime * player.speed;
    }
    if(keyboardState[SDL_SCANCODE_D]){
        player.position.x += deltaTime * player.speed;
    }
    //限制玩家飞机在屏幕内移动
    if(player.position.x < 0) player.position.x = 0;
    if(player.position.y < 0) player.position.y = 0;
    if(player.position.x > game.getWindowWidth() - player.width) player.position.x = game.getWindowWidth() - player.width;
    if(player.position.y > game.getWindowHeight() - player.height) player.position.y = game.getWindowHeight() - player.height;

}
