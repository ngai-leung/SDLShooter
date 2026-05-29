#include "SceneMain.h"
#include "SDL.h"
#include "SDL_image.h"
#include "Game.h"
#include <random>

SceneMain::SceneMain() : game(*Game::getInstance())
{
}

SceneMain::~SceneMain() {}

void SceneMain::update(float deltaTime) 
{
    keyboardControls(deltaTime);
    updateProjectiles(deltaTime);
    updateEnemyProjectiles(deltaTime); //更新敌机子弹
    spawEnemy(); //生成敌机
    updateEnemies(deltaTime); //更新敌机
    checkCollisions();//检查碰撞
}

void SceneMain::render()
{
    // 先渲染子弹
    renderPlayerProjectiles();

    //渲染敌机子弹
    renderEnemyProjectiles();

    // 再渲染玩家
    if (!player.texture) return;

    SDL_Rect playerRect = {
        static_cast<int>(player.position.x),
        static_cast<int>(player.position.y),
        player.width,
        player.height
    };
    SDL_RenderCopy(game.getRenderer(), player.texture, NULL, &playerRect);

    // 最后渲染敌机
    renderEnemies();
}

void SceneMain::handleEvent(SDL_Event *event) {}

void SceneMain::init()
{
    //随机生成敌机
    std::random_device rd; // 用于生成随机数种子
    gen = std::mt19937(rd());
    dis =std::uniform_real_distribution<float>(0.0f,1.0f);
    

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

    //初始化敌机
    EnemyTemplate.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/insect-2.png");
    //加载敌机纹理
    if (!EnemyTemplate.texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to load enemy texture: %s", IMG_GetError());
        // 不影响玩家，但子弹将无法使用
        return;
    }

    SDL_QueryTexture(EnemyTemplate.texture, NULL, NULL,
                     &EnemyTemplate.width,
                     &EnemyTemplate.height);
    EnemyTemplate.width /= 4;
    EnemyTemplate.height /= 4;

    //初始化敌机射击物模版
    ProjectileEnemyTemplate.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/bullet-1.png");
    SDL_QueryTexture(ProjectileEnemyTemplate.texture, NULL, NULL,
                     &ProjectileEnemyTemplate.width,
                     &ProjectileEnemyTemplate.height);
    ProjectileEnemyTemplate.width /= 4;
    ProjectileEnemyTemplate.height /=4;                 

}

void SceneMain::clean()
{
    // 清理活跃子弹（只删除对象，纹理由模板统一销毁）
    for (auto projectile : ProjectilePlayers) {
        delete projectile;
    }
    ProjectilePlayers.clear();


    // 清理活跃敌机
    for (auto enemy : Enemies) {
        delete enemy;
    }
    Enemies.clear();

    // 清理活跃敌机射击物
    for (auto projectile : ProjectileEnemies) {
        delete projectile;
    }
    ProjectileEnemies.clear();
 

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

    // 销毁敌机模板纹理
    if (EnemyTemplate.texture) {
        SDL_DestroyTexture(EnemyTemplate.texture);
        EnemyTemplate.texture = nullptr;
    }

    // 销毁敌机射击物模版纹理
    if (ProjectileEnemyTemplate.texture) {
        SDL_DestroyTexture(ProjectileEnemyTemplate.texture);
        ProjectileEnemyTemplate.texture = nullptr;
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

void SceneMain::updateEnemyProjectiles(float deltaTime)
{
    for (auto it = ProjectileEnemies.begin(); it != ProjectileEnemies.end(); ) {
        auto projectile = *it;
        // 根据方向向量移动
        projectile->position.x += projectile->direction.x * projectile->speed * deltaTime;
        projectile->position.y += projectile->direction.y * projectile->speed * deltaTime;

        // 飞出屏幕任意边界则销毁（简单处理：超出窗口范围）
        if (projectile->position.x + projectile->width < 0 ||
            projectile->position.x > game.getWindowWidth() ||
            projectile->position.y + projectile->height < 0 ||
            projectile->position.y > game.getWindowHeight()) {
            delete projectile;
            it = ProjectileEnemies.erase(it);
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

void SceneMain::renderEnemyProjectiles()
{
    for (auto projectile : ProjectileEnemies) {
        SDL_Rect destRect = {
            static_cast<int>(projectile->position.x),
            static_cast<int>(projectile->position.y),
            projectile->width,
            projectile->height
        };

        // 计算旋转角度（弧度转度数，SDL 中使用度数）
        // direction 向量 (dx, dy) 指向子弹运动方向
        // 若子弹图片默认朝上（0° 指向屏幕顶部），则需计算 atan2(dy, dx) 再减去 90°
        float angle = atan2(projectile->direction.y, projectile->direction.x) * 180.0f / M_PI + 90.0f;
        // 注意：SDL_RenderCopyEx 的 angle 是顺时针旋转的度数，正数表示顺时针。

        SDL_Point center = { projectile->width / 2, projectile->height / 2 };
        SDL_RenderCopyEx(game.getRenderer(),
                         projectile->texture,
                         NULL,
                         &destRect,
                         angle,
                         &center,
                         SDL_FLIP_NONE);
    }
}

void SceneMain::spawEnemy()
{
    if(dis(gen) > 1/60.0f){ return; }
    Enemy* enemy = new Enemy(EnemyTemplate);
    int maxX = game.getWindowWidth() - enemy->width;
    enemy->position.x = (maxX > 0) ? (dis(gen) * maxX) : 0;
    enemy->position.y = -enemy->height;
    Enemies.push_back(enemy);
}

void SceneMain::updateEnemies(float deltaTime)
{
    auto currTime = SDL_GetTicks(); //获取当前时间
    for (auto it = Enemies.begin(); it != Enemies.end(); ) {
        auto enemy = *it;
        enemy->position.y += enemy->speed * deltaTime;

        // 飞出屏幕顶部则销毁
        if (enemy->position.y > game.getWindowHeight()) {
            delete enemy;
            it = Enemies.erase(it);
        } else {
            if (currTime - enemy->lastShootTime >= enemy->coolDown) {
                shootEnemy(enemy);
                enemy->lastShootTime = currTime;
            }
            ++it;
        }
    }
}

void SceneMain::renderEnemies()
{
    for (auto enemy : Enemies) {
        SDL_Rect enemyRect = {
            static_cast<int>(enemy->position.x),
            static_cast<int>(enemy->position.y),
            enemy->width,
            enemy->height
        };
        SDL_RenderCopy(game.getRenderer(), enemy->texture, NULL, &enemyRect);
    }
}

void SceneMain::shootEnemy(Enemy* enemy)
{
    if (!ProjectileEnemyTemplate.texture) return;

    auto projectile = new ProjectileEnemy;
    // 复制模板属性
    projectile->texture = ProjectileEnemyTemplate.texture;
    projectile->width  = ProjectileEnemyTemplate.width;
    projectile->height = ProjectileEnemyTemplate.height;
    projectile->speed  = ProjectileEnemyTemplate.speed;  // 需要确保模板有有效值

    // 计算子弹出生位置（敌机中心）
    projectile->position.x = enemy->position.x + enemy->width/2.0f - projectile->width/2.0f;
    projectile->position.y = enemy->position.y + enemy->height/2.0f - projectile->height/2.0f;

    // 计算方向
    SDL_FPoint dir = getDirection(enemy);
    // 将方向存入 direction 供更新使用
    projectile->direction = dir;

    ProjectileEnemies.push_back(projectile);
}

SDL_FPoint SceneMain::getDirection(Enemy* enemy)
{
    // 计算从敌机中心指向玩家中心的向量
    float dx = (player.position.x + player.width / 2.0f) - (enemy->position.x + enemy->width / 2.0f);
    float dy = (player.position.y + player.height / 2.0f) - (enemy->position.y + enemy->height / 2.0f);
    
    float length = sqrtf(dx * dx + dy * dy);
    if (length < 1e-6f) {  // 避免除零，若重合则默认向下
        return {0.0f, 1.0f};
    }
    
    return {dx / length, dy / length};
}

    void SceneMain::checkCollisions()
{
    // 1. 玩家子弹 vs 敌机
    for (auto pit = ProjectilePlayers.begin(); pit != ProjectilePlayers.end(); ) {
        ProjectilePlayer* proj = *pit;
        SDL_Rect projRect = {
            static_cast<int>(proj->position.x),
            static_cast<int>(proj->position.y),
            proj->width,
            proj->height
        };
        bool hit = false;

        for (auto eit = Enemies.begin(); eit != Enemies.end(); ) {
            Enemy* enemy = *eit;
            SDL_Rect enemyRect = {
                static_cast<int>(enemy->position.x),
                static_cast<int>(enemy->position.y),
                enemy->width,
                enemy->height
            };
            if (SDL_HasIntersection(&projRect, &enemyRect)) {
                // 子弹击中敌机
                enemy->currentHealth -= proj->damage;
                hit = true;  // 子弹命中，无论敌机是否死亡，子弹都消失
                if (enemy->currentHealth <= 0) {
                    // 敌机死亡，从列表中移除并删除
                    delete enemy;
                    eit = Enemies.erase(eit);
                } else {
                    ++eit;
                }
                break; // 一颗子弹只击中一个敌人
            } else {
                ++eit;
            }
        }

        if (hit) {
            delete proj;
            pit = ProjectilePlayers.erase(pit);
        } else {
            ++pit;
        }
    }

    // 2. 敌机子弹 vs 玩家
    for (auto pit = ProjectileEnemies.begin(); pit != ProjectileEnemies.end(); ) {
        ProjectileEnemy* proj = *pit;
        SDL_Rect projRect = {
            static_cast<int>(proj->position.x),
            static_cast<int>(proj->position.y),
            proj->width,
            proj->height
        };
        SDL_Rect playerRect = {
            static_cast<int>(player.position.x),
            static_cast<int>(player.position.y),
            player.width,
            player.height
        };
        if (SDL_HasIntersection(&projRect, &playerRect)) {
            player.currentHealth -= proj->damage;
            delete proj;
            pit = ProjectileEnemies.erase(pit);
            // 如果玩家死亡，可以设置游戏结束标志（后面实现）
            if (player.currentHealth <= 0) {
                // 例如：game.quitGame();  但需要先提供 Game::quitGame()
                // 暂时只打印日志，后续完善
                SDL_Log("Player died!");
            }
        } else {
            ++pit;
        }
    }

    // 3. 玩家 vs 敌机（直接碰撞）
    SDL_Rect playerRect = {
        static_cast<int>(player.position.x),
        static_cast<int>(player.position.y),
        player.width,
        player.height
    };
    for (auto eit = Enemies.begin(); eit != Enemies.end(); ) {
        Enemy* enemy = *eit;
        SDL_Rect enemyRect = {
            static_cast<int>(enemy->position.x),
            static_cast<int>(enemy->position.y),
            enemy->width,
            enemy->height
        };
        if (SDL_HasIntersection(&playerRect, &enemyRect)) {
            player.currentHealth -= 10;   // 碰撞伤害可自行调整
            delete enemy;
            eit = Enemies.erase(eit);
            if (player.currentHealth <= 0) {
                SDL_Log("Player died by collision!");
            }
        } else {
            ++eit;
        }
    }
}

