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
    updateProjectiles(deltaTime);
}

void SceneMain::render()
{
    // 先渲染子弹
    renderPlayerProjectiles();

    // 再渲染玩家
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
    // 1. 加载玩家纹理
    player.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/SpaceShip.png");
    if (!player.texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to load player texture: %s", IMG_GetError());
        return;
    }
    SDL_QueryTexture(player.texture, NULL, NULL, &player.width, &player.height);
    player.width /= 4;
    player.height /= 4;

    // 设置玩家初始位置（底部居中）
    player.position.x = (game.getWindowWidth() - player.width) / 2.0f;
    player.position.y = static_cast<float>(game.getWindowHeight() - player.height - 20);

    // 2. 加载子弹模板纹理
    ProjectilePlayerTemplate.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/laser-1.png");
    if (!ProjectilePlayerTemplate.texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to load projectile texture: %s", IMG_GetError());
        // 不影响玩家，但子弹将无法使用
        return;
    }
    SDL_QueryTexture(ProjectilePlayerTemplate.texture, NULL, NULL,
                     &ProjectilePlayerTemplate.width,
                     &ProjectilePlayerTemplate.height);
    ProjectilePlayerTemplate.width /= 4;
    ProjectilePlayerTemplate.height /= 4;
}

void SceneMain::clean()
{
    // 清理活跃子弹（只删除对象，纹理由模板统一销毁）
    for (auto projectile : ProjectilePlayers) {
        delete projectile;
    }
    ProjectilePlayers.clear();

    // 销毁玩家纹理
    if (player.texture) {
        SDL_DestroyTexture(player.texture);
        player.texture = nullptr;
    }

    // 销毁子弹模板纹理
    if (ProjectilePlayerTemplate.texture) {
        SDL_DestroyTexture(ProjectilePlayerTemplate.texture);
        ProjectilePlayerTemplate.texture = nullptr;
    }
}

void SceneMain::keyboardControls(float deltaTime)
{
    auto keyboardState = SDL_GetKeyboardState(NULL);

    if (keyboardState[SDL_SCANCODE_W]) {
        player.position.y -= deltaTime * player.speed;
    }
    if (keyboardState[SDL_SCANCODE_S]) {
        player.position.y += deltaTime * player.speed;
    }
    if (keyboardState[SDL_SCANCODE_A]) {
        player.position.x -= deltaTime * player.speed;
    }
    if (keyboardState[SDL_SCANCODE_D]) {
        player.position.x += deltaTime * player.speed;
    }

    // 边界限制
    if (player.position.x < 0) player.position.x = 0;
    if (player.position.y < 0) player.position.y = 0;
    if (player.position.x > game.getWindowWidth() - player.width)
        player.position.x = game.getWindowWidth() - player.width;
    if (player.position.y > game.getWindowHeight() - player.height)
        player.position.y = game.getWindowHeight() - player.height;

    // 射击（空格键）
    if (keyboardState[SDL_SCANCODE_SPACE]) {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - player.lastShootTime >= player.coolDown) {
            player.lastShootTime = currentTime;
            shootPlayer();
        }
    }
}

void SceneMain::shootPlayer()
{
    if (!ProjectilePlayerTemplate.texture) return;  // 模板无效则不发射

    auto projectile = new ProjectilePlayer;
    projectile->texture = ProjectilePlayerTemplate.texture;   // 共享纹理
    projectile->width = ProjectilePlayerTemplate.width;
    projectile->height = ProjectilePlayerTemplate.height;
    projectile->speed = ProjectilePlayerTemplate.speed;

    // 子弹初始位置：玩家上方中央
    projectile->position.x = player.position.x + player.width / 2.0f - projectile->width / 2.0f;
    projectile->position.y = player.position.y - projectile->height;

    ProjectilePlayers.push_back(projectile);
}

void SceneMain::updateProjectiles(float deltaTime)
{
    for (auto it = ProjectilePlayers.begin(); it != ProjectilePlayers.end(); ) {
        auto projectile = *it;
        projectile->position.y -= projectile->speed * deltaTime;

        // 飞出屏幕顶部则销毁
        if (projectile->position.y + projectile->height < 0) {
            delete projectile;
            it = ProjectilePlayers.erase(it);
        } else {
            ++it;
        }
    }
}

void SceneMain::renderPlayerProjectiles()
{
    for (auto projectile : ProjectilePlayers) {
        SDL_Rect projectileRect = {
            static_cast<int>(projectile->position.x),
            static_cast<int>(projectile->position.y),
            projectile->width,
            projectile->height
        };
        SDL_RenderCopy(game.getRenderer(), projectile->texture, NULL, &projectileRect);
    }
}