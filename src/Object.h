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

struct ProjectilePlayer{
    SDL_Texture* texture = nullptr;
    SDL_FPoint position = {0,0};
    int width = 0 ;
    int height = 0 ;
    int speed = 600;
};









#endif /* Object_h */