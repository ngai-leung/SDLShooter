#ifndef SCENE_MAIN_H
#define SCENE_MAIN_H

#include "Scene.h"
#include "Object.h"
#include <list>

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
    void updateProjectiles(float deltaTime);
    void renderPlayerProjectiles();

private:
    Player player;
    Game &game;

    //创建每个物体的模板
    ProjectilePlayer ProjectilePlayerTemplate;

    std::list<ProjectilePlayer*> ProjectilePlayers;

};

#endif