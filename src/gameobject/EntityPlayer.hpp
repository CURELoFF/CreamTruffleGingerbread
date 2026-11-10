#pragma once

#include<SDL3/SDL.h>

class EntityPlayer {
public:
    SDL_FRect GetCollisionRect() const{return collisionRect_;}
    void SetCollisionRectPos(float x,float y){collisionRect_.x=x;collisionRect_.y=y;}

    void Update(double delta);
    void Draw(SDL_Renderer* renderer,float interpolation);

    float MoveSpeed = 1024.f;
private:
    SDL_FRect collisionRect_{50.f,50.f,100.f,100.f};
};
