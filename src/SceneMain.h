#ifndef SCENE_MAIN_H
#define SCENE_MAIN_H

#include "Scene.h"
#include "Object.h"
#include <list>
#include <random>
#include <vector>



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

    void keyboardControls(float deltaTime);
    void shootPlayer();
    void updateProjectiles(float deltaTime);      //玩家射击物
    void updateEnemyProjectiles(float deltaTime); //敌人射击物
    void renderPlayerProjectiles();
    void renderEnemyProjectiles();
    void spawEnemy();
    void updateEnemies(float deltaTime);
    void renderEnemies();
    void shootEnemy(Enemy* enemy);
    void checkCollisions(); //检查碰撞
    void createExplosion(float x, float y);
    void updateExplosions(float deltaTime);
    void renderExplosions();
    SDL_FPoint getDirection(Enemy* enemy);




private:
    Player player;
    Game &game;
    std::mt19937 gen; // 随机数生成器
    std::uniform_real_distribution<float> dis; // 随机数分布

    //创建每个物体的模板
    Enemy EnemyTemplate;
    ProjectilePlayer ProjectilePlayerTemplate;
    ProjectileEnemy ProjectileEnemyTemplate;
    Explosion ExplosionTemplate;

    //创建每个物体的容器
    std::list<Enemy*> Enemies;
    std::list<ProjectilePlayer*> ProjectilePlayers;
    std::list<ProjectileEnemy*> ProjectileEnemies;
    std::list<Explosion> Explosions;   // 存储爆炸效果（值语义）
    
    bool gameOver = false;


};

#endif