#include "SceneMain.h"
#include "SDL.h"
#include "SDL_image.h"
#include "Game.h"
#include "SceneEnd.h"
#include <random>
#include <string>


SceneMain::SceneMain() : game(*Game::getInstance())
{
}

SceneMain::~SceneMain() {}

void SceneMain::update(float deltaTime) 
{
    // 更新道具效果
    Uint32 now = SDL_GetTicks();
    if (hasShield && now >= shieldEndTime) {
        hasShield = false;
        SDL_SetTextureColorMod(player.texture, 255, 255, 255);
    }
    if (timeSlowActive && now >= timeSlowEndTime) {
        timeSlowActive = false;
        for (auto enemy : Enemies) enemy->speed = originalEnemySpeed;
        for (auto bullet : ProjectileEnemies) bullet->speed = originalEnemyBulletSpeed;
    }
    
    keyboardControls(deltaTime);
    updateProjectiles(deltaTime);
    updateEnemyProjectiles(deltaTime);
    spawEnemy();
    updateEnemies(deltaTime);
    checkCollisions();

    // 如果玩家死亡，不再执行后续更新，直接切换场景
    if (shouldEnd) {
        Game::getInstance()->changeScene(new SceneEnd(score));
        return;
    }

    updateItems(deltaTime);
    updateExplosions(deltaTime);
}

void SceneMain::render()
{
    renderPlayerProjectiles();
    renderEnemyProjectiles();

    // 渲染玩家
    if (player.texture) {
        SDL_Rect playerRect = {
            static_cast<int>(player.position.x),
            static_cast<int>(player.position.y),
            player.width,
            player.height
        };
        SDL_RenderCopy(game.getRenderer(), player.texture, NULL, &playerRect);
    }

    renderEnemies();
    renderItems();
    renderExplosions();
    renderUI();
}

void SceneMain::handleEvent(SDL_Event *event) {}

void SceneMain::init()
{
    //读取并播放音乐
    bgm = Mix_LoadMUS("assets/music/03_Racing_Through_Asteroids_Loop.ogg");
    if (bgm == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load music: %s", Mix_GetError());
    }else {
    Mix_PlayMusic(bgm, -1);
    }

    //读取ui Health
    uiHealth  = IMG_LoadTexture(game.getRenderer(), "assets/image/Health UI Black.png");

    //载入字体
    scoreFont = TTF_OpenFont("assets/font/VonwaonBitmap-12px.ttf", 24);

    // 获取心形纹理原始尺寸
    SDL_QueryTexture(uiHealth, NULL, NULL, &heartWidth, &heartHeight);
    // 可选：若希望固定绘制大小（例如 32x32），可重新赋值
    // heartWidth = 32;
    // heartHeight = 32;

    //读取音效资源
    sounds["player_shoot"] = Mix_LoadWAV("assets/sound/laser_shoot4.wav");
    sounds["enemy_shoot"] = Mix_LoadWAV("assets/sound/xs_laser.wav");
    sounds["player_explosion"] = Mix_LoadWAV("assets/sound/explosion1.wav");
    sounds["enemy_explosion"] = Mix_LoadWAV("assets/sound/explosion3.wav");
    //sounds["hit"] = Mix_LoadWAV("assets/sound/eff11.wav");
    sounds["get_item"] = Mix_LoadWAV("assets/sound/eff5.wav");



    //随机生成敌机
    std::random_device rd; // 用于生成随机数种子
    gen = std::mt19937(rd());
    dis =std::uniform_real_distribution<float>(0.0f,1.0f);
    

    // 1. 加载玩家纹理
    player.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/SpaceShip.png");
    if (!player.texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to load player texture: %s", IMG_GetError());
        
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
    
    // 加载爆炸纹理
    ExplosionTemplate.texture = IMG_LoadTexture(game.getRenderer(), "assets/effect/explosion.png");
    if (!ExplosionTemplate.texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load explosion texture: %s", IMG_GetError());
        
    }
    int texW, texH;
    SDL_QueryTexture(ExplosionTemplate.texture, NULL, NULL, &texW, &texH);

    // 单行排列，总帧数假设为 8（根据你的实际图片修改）
    int totalFrames = texW / texH;   // 请改为你的爆炸图实际帧数
    ExplosionTemplate.frameWidth = texW / totalFrames;
    ExplosionTemplate.frameHeight = texH;
    ExplosionTemplate.totalFrames = totalFrames;
    // 每帧持续时间：例如 FPS=20 → 50ms
    ExplosionTemplate.frameDuration = 1000 / 20;   // 20帧/秒
    // 注意：如果你在 Explosion 结构体中有 FPS 字段，也可用 FPS 换算

    //初始化生命道具
    ItemLifeTemplate.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/bonus_life.png");
    SDL_QueryTexture(ItemLifeTemplate.texture, NULL, NULL,
                     &ItemLifeTemplate.width,
                     &ItemLifeTemplate.height);
    ItemLifeTemplate.width /= 4;
    ItemLifeTemplate.height /= 4;

    // 初始化护盾道具
    ItemShieldTemplate.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/bonus_shield.png");
    if (!ItemShieldTemplate.texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load shield texture: %s", IMG_GetError());
        // 可选：继续运行，只是无护盾道具
    }
    SDL_QueryTexture(ItemShieldTemplate.texture, NULL, NULL, &ItemShieldTemplate.width, &ItemShieldTemplate.height);
    ItemShieldTemplate.width /= 4;
    ItemShieldTemplate.height /= 4;

    // 初始化时间道具
    ItemTimeTemplate.texture = IMG_LoadTexture(game.getRenderer(), "assets/image/bonus_time.png");
    if (!ItemTimeTemplate.texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load time texture: %s", IMG_GetError());
    }
    SDL_QueryTexture(ItemTimeTemplate.texture, NULL, NULL, &ItemTimeTemplate.width, &ItemTimeTemplate.height);
    ItemTimeTemplate.width /= 4;
    ItemTimeTemplate.height /= 4;

}

void SceneMain::clean()
{
    //清理音效容器
    for (auto &sound : sounds) {
        if(sound.second != nullptr) {
            Mix_FreeChunk(sound.second);
        }
    }
    sounds.clear();

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
    
    //清理道具
    for(auto &item : items) {
        if(item != nullptr) {
            delete item;
        }
    }
    items.clear();

    //清理ui
    if(uiHealth != nullptr){
        SDL_DestroyTexture(uiHealth);
    }

    //清理字体
    if (scoreFont != nullptr) {
        TTF_CloseFont(scoreFont);
    }
        



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

    //销毁道具纹理
    if (ItemLifeTemplate.texture) {
        SDL_DestroyTexture(ItemLifeTemplate.texture);
        ItemLifeTemplate.texture = nullptr;
    }

    if (ItemShieldTemplate.texture) {
    SDL_DestroyTexture(ItemShieldTemplate.texture);
    ItemShieldTemplate.texture = nullptr;
    }

    if (ItemTimeTemplate.texture) {
        SDL_DestroyTexture(ItemTimeTemplate.texture);
        ItemTimeTemplate.texture = nullptr;
    }

    //清理音乐资源
    if (bgm != nullptr) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgm);
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
    Mix_PlayChannel(-1, sounds["player_shoot"], 0);

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
            //Mix_PlayChannel(-1,sounds["hit"],0);
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
    enemy->speed = EnemyTemplate.speed;
    if (timeSlowActive) {
        enemy->speed /= 2;
        }
    int maxX = game.getWindowWidth() - enemy->width;
    enemy->position.x = (maxX > 0) ? (dis(gen) * maxX) : 0;
    enemy->position.y = -enemy->height;
    Enemies.push_back(enemy);
}


void SceneMain::updateEnemies(float deltaTime)
{
    auto currTime = SDL_GetTicks();
    for (auto it = Enemies.begin(); it != Enemies.end(); ) {
        Enemy* enemy = *it;
        enemy->position.y += enemy->speed * deltaTime;

        if (enemy->position.y > game.getWindowHeight()) {
            delete enemy;
            it = Enemies.erase(it);
        } else {
            if (currTime - enemy->lastShootTime >= enemy->coolDown) {
                shootEnemy(enemy);
                enemy->lastShootTime = currTime;
            }
            ++it;   // 注意：必须放在 else 分支的最后
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
    //Mix_PlayChannel(-1, sounds["enemy_shoot"], 0);
    if (timeSlowActive) {
        projectile->speed /= 2;
        }

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
                enemy->currentHealth -= proj->damage;
                hit = true;
                if (enemy->currentHealth <= 0) {
                    float centerX = enemy->position.x + enemy->width / 2.0f;
                    float centerY = enemy->position.y + enemy->height / 2.0f;
                    createExplosion(centerX, centerY, false);
                    dropItem(centerX, centerY);
                    score += 10;
                    delete enemy;
                    eit = Enemies.erase(eit);
                } else {
                    ++eit;
                }
                break;
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
            if (!hasShield) {
                player.currentHealth -= proj->damage;
            }
            delete proj;
            pit = ProjectileEnemies.erase(pit);
            if (player.currentHealth <= 0) {
                shouldEnd = true;
                return;
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
            float centerX = enemy->position.x + enemy->width / 2.0f;
            float centerY = enemy->position.y + enemy->height / 2.0f;
            createExplosion(centerX, centerY, false);
            if (!hasShield) {
                player.currentHealth -= 33;
            }
            delete enemy;
            eit = Enemies.erase(eit);
            if (player.currentHealth <= 0) {
                shouldEnd = true;
                return;
            }
        } else {
            ++eit;
        }
    }

    // 4. 玩家 vs 物品
    for (auto it = items.begin(); it != items.end(); ) {
        Item* item = *it;
        SDL_Rect itemRect = {
            static_cast<int>(item->position.x),
            static_cast<int>(item->position.y),
            item->width,
            item->height
        };
        if (SDL_HasIntersection(&playerRect, &itemRect)) {
            applyItemEffect(item);
            delete item;
            it = items.erase(it);
        } else {
            ++it;
        }
    }
}


    void SceneMain::createExplosion(float x, float y, bool isPlayer)
{
    Explosion exp;
    exp.texture = ExplosionTemplate.texture;
    exp.frameWidth = ExplosionTemplate.frameWidth;
    exp.frameHeight = ExplosionTemplate.frameHeight;
    exp.totalFrames = ExplosionTemplate.totalFrames;
    exp.currentFrame = 0;
    exp.startTime = SDL_GetTicks();
    exp.frameDuration = ExplosionTemplate.frameDuration;
    // 将输入的中心点转换为左上角坐标
    exp.position.x = x - exp.frameWidth / 2.0f;
    exp.position.y = y - exp.frameHeight / 2.0f;
    Explosions.push_back(exp);

    // 播放对应的爆炸音效
    if (isPlayer) {
        if (sounds["player_explosion"]) {
            Mix_PlayChannel(-1, sounds["player_explosion"], 0);
            }
        }  else {
        if (sounds["enemy_explosion"]) {
            Mix_PlayChannel(-1, sounds["enemy_explosion"], 0);
            }
        }
}

void SceneMain::updateExplosions(float deltaTime)
{
    Uint32 now = SDL_GetTicks();
    for (auto it = Explosions.begin(); it != Explosions.end(); ) {
        Uint32 elapsed = now - it->startTime;
        int frameIndex = elapsed / it->frameDuration;
        if (frameIndex >= it->totalFrames) {
            it = Explosions.erase(it);   // 动画结束，移除
        } else {
            it->currentFrame = frameIndex;
            ++it;
        }
    }
}

void SceneMain::renderExplosions()
{
    for (const auto& exp : Explosions) {
        SDL_Rect srcRect = {
            exp.currentFrame * exp.frameWidth,   // 水平偏移
            0,
            exp.frameWidth,
            exp.frameHeight
        };
        SDL_Rect dstRect = {
            static_cast<int>(exp.position.x),
            static_cast<int>(exp.position.y),
            exp.frameWidth,
            exp.frameHeight
        };
        SDL_RenderCopy(game.getRenderer(), exp.texture, &srcRect, &dstRect);
    }
}

void SceneMain::dropItem(float x, float y)
{
    if (dis(gen) > 0.2f) return;

    int r = rand() % 3;
    Item* templateItem = nullptr;
    switch (r) {
        case 0: templateItem = &ItemLifeTemplate; break;
        case 1: templateItem = &ItemShieldTemplate; break;
        case 2: templateItem = &ItemTimeTemplate; break;
        default: return;
    }
    if (!templateItem->texture) return;

    Item* item = new Item();
    item->texture = templateItem->texture;
    item->width = templateItem->width;
    item->height = templateItem->height;
    item->position.x = x - item->width / 2.0f;
    item->position.y = y - item->height / 2.0f;

    // 随机方向（角度 0~360°）
    float angle = (static_cast<float>(rand()) / RAND_MAX) * 2.0f * M_PI;
    item->direction.x = cosf(angle);
    item->direction.y = sinf(angle);

    // 确保速度不为零
    if (fabs(item->direction.x) < 0.01f) item->direction.x = (rand() % 2 == 0) ? 0.01f : -0.01f;
    if (fabs(item->direction.y) < 0.01f) item->direction.y = (rand() % 2 == 0) ? 0.01f : -0.01f;

    // 归一化（确保单位向量）
    float len = sqrtf(item->direction.x * item->direction.x + item->direction.y * item->direction.y);
    item->direction.x /= len;
    item->direction.y /= len;

    item->speed = 100.0f;                 // 恒定速度
    item->type = (ItemType)r;
    item->bounceCount = 0;

    items.push_back(item);
}

void SceneMain::updateItems(float deltaTime)
{
    const float DAMP = 0.9f;      // 每次反弹速度保留 90%
    const float MIN_SPEED = 0.05f;

    for (auto it = items.begin(); it != items.end(); ) {
        Item* item = *it;

        // 移动
        item->position.x += item->direction.x * item->speed * deltaTime;
        item->position.y += item->direction.y * item->speed * deltaTime;

        // 左右边界碰撞（反弹 + 能量损失）
        int left = 0;
        int right = game.getWindowWidth() - item->width;
        if (item->position.x < left) {
            item->position.x = left;
            item->direction.x = -item->direction.x * DAMP;
            item->bounceCount++;
        } else if (item->position.x > right) {
            item->position.x = right;
            item->direction.x = -item->direction.x * DAMP;
            item->bounceCount++;
        }

        // 上下边界碰撞（反弹 + 能量损失）
        int top = 0;
        int bottom = game.getWindowHeight() - item->height;
        if (item->position.y < top) {
            item->position.y = top;
            item->direction.y = -item->direction.y * DAMP;
            item->bounceCount++;
        } else if (item->position.y > bottom) {
            item->position.y = bottom;
            item->direction.y = -item->direction.y * DAMP;
            item->bounceCount++;
        }

        // 速度过小时停止（避免无限抖动）
        if (fabs(item->direction.x) < MIN_SPEED) item->direction.x = 0;
        if (fabs(item->direction.y) < MIN_SPEED) item->direction.y = 0;

        // 碰撞次数达到 3 次，删除物品
        if (item->bounceCount >= 3) {
            delete item;
            it = items.erase(it);
            continue;
        }

        ++it;
    }
    
}

void SceneMain::renderItems()
{
        for (auto item : items) {
        SDL_Rect dstRect = {
            static_cast<int>(item->position.x),
            static_cast<int>(item->position.y),
            item->width,
            item->height
        };
        SDL_RenderCopy(game.getRenderer(), item->texture, NULL, &dstRect);
    }
}

void SceneMain::applyItemEffect(Item* item)
{
    Uint32 now = SDL_GetTicks();
    switch (item->type) {
        case ItemType::Life:
            player.currentHealth += 33;
            if (player.currentHealth > 100) player.currentHealth = 100;
            break;
        case ItemType::Shield:
            // 护盾持续5秒，期间无敌
            hasShield = true;
            shieldEndTime = now + 5000;  // 5秒
            // 设置纹理颜色为金色
            SDL_SetTextureColorMod(player.texture, 255, 215, 0);
            break;
        case ItemType::Time:
            // 时间减速：敌人和敌人子弹速度减半，持续5秒
            if (!timeSlowActive) {
                originalEnemySpeed = EnemyTemplate.speed;
                originalEnemyBulletSpeed = ProjectileEnemyTemplate.speed;
            }
            timeSlowActive = true;
            timeSlowEndTime = now + 5000;
            // 立即应用减速
            for (auto enemy : Enemies) {
                enemy->speed = originalEnemySpeed / 2;
            }
            for (auto bullet : ProjectileEnemies) {
                bullet->speed = originalEnemyBulletSpeed / 2;
            }
            break;
    }
    Mix_PlayChannel(1, sounds["get_item"], 0);
}

void SceneMain::renderUI()
{
    const int startX = 20;
    const int startY = 20;
    const int spacing = 10;
    const int maxHealth = 100;
    const int heartCount = 3;

    // 每颗心对应的生命值（向上取整） 100/3 = 33.33 → 34
    const int perHeart = (maxHealth + heartCount - 1) / heartCount; // 34
    int activeHearts = (player.currentHealth + perHeart - 1) / perHeart;
    if (activeHearts > heartCount) activeHearts = heartCount;
    if (activeHearts < 0) activeHearts = 0;

    for (int i = 0; i < heartCount; ++i) {
        if (i < activeHearts) {
            SDL_SetTextureColorMod(uiHealth, 255, 80, 80);   // 红色
        } else {
            SDL_SetTextureColorMod(uiHealth, 80, 80, 80);    // 灰色
        }
        SDL_Rect dstRect = {
            startX + i * (heartWidth + spacing),
            startY,
            heartWidth,
            heartHeight
        };
        SDL_RenderCopy(game.getRenderer(), uiHealth, NULL, &dstRect);
    }
    SDL_SetTextureColorMod(uiHealth, 255, 255, 255);

        // 2. 渲染得分文本
    if (scoreFont != nullptr) {
        std::string scoreText = "Score: " + std::to_string(score);
        SDL_Color color = { 255, 255, 255, 255 }; // 白色
        SDL_Surface* surface = TTF_RenderText_Solid(scoreFont, scoreText.c_str(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(game.getRenderer(), surface);
            int textWidth = surface->w;
            int textHeight = surface->h;
            SDL_Rect dstRect = {
                game.getWindowWidth() - textWidth - 20,
                20,
                textWidth,
                textHeight
            };
            SDL_RenderCopy(game.getRenderer(), texture, NULL, &dstRect);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }
    }
}