#ifndef Object_h
#define Object_h
#include <SDL.h>



struct Player{
    SDL_Texture* texture = nullptr;  //材质
    SDL_FPoint position = {0,0};     //初始化位置
    int width = 0;                   
    int height = 0; 
    int speed = 300;
     Uint32 coolDown =100;
     Uint32 lastShootTime = 0;     
};

struct Enemy{
    SDL_Texture* texture = nullptr;
    SDL_FPoint position = {0,0};
    int width = 0 ;
    int height = 0 ;
    int speed = 100;
    Uint32 coolDown =1000;     //冷却时间
    Uint32 lastShootTime = 0; //记录上次射击时间
};

struct ProjectilePlayer{
    SDL_Texture* texture = nullptr;
    SDL_FPoint position = {0,0};
    int width = 0 ;
    int height = 0 ;
    int speed = 600;
};

struct ProjectileEnemy{
    
    SDL_Texture* texture = nullptr;
    SDL_FPoint position = {0,0};
    SDL_FPoint direction = {0,0};
    int width = 0 ;
    int height = 0 ;
    int speed = 300;
};









#endif /* Object_h */