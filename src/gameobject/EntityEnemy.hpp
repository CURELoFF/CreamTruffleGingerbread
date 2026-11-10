#pragma once

#include<SDL3/SDL.h>

class EntityEnemy {
public:
    SDL_FRect GetCollisionRect() const {return collisionRect_;}
    void SetCollisionRectPos(float x,float y){collisionRect_.x = x;collisionRect_.y=y;}

    void Update(double delta);
    void Draw(SDL_Renderer* renderer,float interpolation);
private:
    SDL_FRect collisionRect_{0,0,25.f,25.f};
};
