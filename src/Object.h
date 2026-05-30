#ifndef Object_h
#define Object_h
#include <SDL.h>

enum class ItemType {
    Life,
    Shield,
    Time
};

struct Player{
    SDL_Texture* texture = nullptr;  //材质
    SDL_FPoint position = {0,0};     //初始化位置
    int width = 0;                   
    int height = 0; 
    int speed = 300;
    int currentHealth = 99;        //当前生命值
     Uint32 coolDown =100;
     Uint32 lastShootTime = 0;     
};

struct Enemy{
    SDL_Texture* texture = nullptr;
    SDL_FPoint position = {0,0};
    int width = 0 ;
    int height = 0 ;
    int speed = 100;
    int currentHealth = 1;      //当前生命值
    Uint32 coolDown =1000;     //冷却时间
    Uint32 lastShootTime = 0;  //记录上次射击时间
};

struct ProjectilePlayer{
    SDL_Texture* texture = nullptr;
    SDL_FPoint position = {0,0};
    int width = 0 ;
    int height = 0 ;
    int speed = 600;
    int damage = 1;          //伤害值
};

struct ProjectileEnemy{
    
    SDL_Texture* texture = nullptr;
    SDL_FPoint position = {0,0};
    SDL_FPoint direction = {0,0};
    int width = 0 ;
    int height = 0 ;
    int speed = 300;
    int damage = 33;          //伤害值
};


// 爆炸效果（动画，单行纹理）
struct Explosion {
    SDL_Texture* texture = nullptr;   // 共享纹理
    SDL_FPoint position = {0, 0};     // 绘制位置（左上角）
    int frameWidth = 0;               // 每帧宽度
    int frameHeight = 0;              // 每帧高度
    int totalFrames = 0;              // 总帧数
    int currentFrame = 0;             // 当前帧索引（由时间计算）
    Uint32 startTime = 0;             // 动画开始时间（毫秒）
    Uint32 frameDuration = 0;         // 每帧持续时间（毫秒）
};

struct Item {
    SDL_Texture* texture = nullptr;
    SDL_FPoint position = {0, 0};
    SDL_FPoint direction = {0, 0};
    int width = 0;
    int height = 0;
    ItemType type = ItemType::Life;
    float speed = 10.0f;
    int bounceCount = 0;      // 已碰撞次数（用于左右碰壁）
};

struct Background {
    SDL_Texture* texture = nullptr;
    SDL_FPoint position = {0, 0};
    int width = 0;
    int height = 0;
    int speed = 30;
    float offset = 0.0f; // 用于实现滚动效果

};





#endif /* Object_h */