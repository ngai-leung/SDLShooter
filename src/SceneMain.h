#ifndef SCENE_MAIN_H
#define SCENE_MAIN_H

#include "Scene.h"
#include "Object.h"
#include <list>
#include <random>
#include <map>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>



class Game;   // ★ 正确前向声明，不要用小写 game

class SceneMain : public Scene {
public:
    SceneMain();
    ~SceneMain();

    void update(float deltaTime) override;
    void render() override;
    void handleEvent(SDL_Event* event) override;
    void init() override;
    void clean() override;

private:
    Player player;
    Game &game;
    Mix_Music* bgm ;
    SDL_Texture*uiHealth;
    TTF_Font* scoreFont;
    int score = 0;

    int heartWidth = 0;   
    int heartHeight = 0;  
    std::mt19937 gen; // 随机数生成器
    std::uniform_real_distribution<float> dis; // 随机数分布

    //创建每个物体的模板
    Enemy EnemyTemplate;
    ProjectilePlayer ProjectilePlayerTemplate;
    ProjectileEnemy ProjectileEnemyTemplate;
    Explosion ExplosionTemplate;
    Item ItemLifeTemplate;
    Item ItemShieldTemplate;
    Item ItemTimeTemplate;


    //创建每个物体的容器
    std::list<Enemy*> Enemies;
    std::list<ProjectilePlayer*> ProjectilePlayers;
    std::list<ProjectileEnemy*> ProjectileEnemies;
    std::list<Explosion> Explosions;
    std::list<Item*> items;   // 存储爆炸效果
    std::map<std::string, Mix_Chunk*> sounds; // 存储音效
    

    bool hasShield = false;            // 是否拥有护盾
    Uint32 shieldEndTime = 0;          // 护盾结束时间（毫秒）
    bool timeSlowActive = false;       // 时间减速是否激活
    Uint32 timeSlowEndTime = 0;        // 减速结束时间
    float originalEnemySpeed = 100;    // 记录敌人原始速度（用于恢复）
    float originalEnemyBulletSpeed = 300; // 原始子弹速度
    bool shouldEnd = false; // 是否应该结束游戏

    //渲染更新
    void renderPlayerProjectiles();
    void renderEnemyProjectiles();
    void renderEnemies();
    void renderExplosions();
    void renderItems();                            // 渲染物品
    void renderUI();                               // 渲染UI
    

    //更新相关
    void updateProjectiles(float deltaTime);      //玩家射击物
    void updateEnemyProjectiles(float deltaTime); //敌人射击物
    void updateEnemies(float deltaTime);
    void updateExplosions(float deltaTime);
    void updateItems(float deltaTime);             // 更新所有物品
    
    //其他
    void keyboardControls(float deltaTime);
    void shootPlayer();
    void spawEnemy();
    void shootEnemy(Enemy* enemy);
    void checkCollisions(); //检查碰撞
    void createExplosion(float x, float y, bool isPlayer);
    void dropItem(float x, float y);               // 在敌机位置掉落物品
    void applyItemEffect(Item* item);              // 玩家拾取物品的效果
    SDL_FPoint getDirection(Enemy* enemy);
};

#endif