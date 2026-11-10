#include"EntityPlayer.hpp"
#include <algorithm>

void EntityPlayer::Update(double delta){
    float fdelta = static_cast<float>(delta);
    const bool* keys = SDL_GetKeyboardState(nullptr);
    float right=0,down=0;
    if(keys[SDL_SCANCODE_LEFT]) right  = -1;
    if(keys[SDL_SCANCODE_RIGHT]) right =  1;
    if(keys[SDL_SCANCODE_UP]) down     = -1;
    if(keys[SDL_SCANCODE_DOWN]) down   =  1;
    float targetx,targety;
    targetx = std::clamp(GetCollisionRect().x+right*fdelta*MoveSpeed,0.f,1600.f-GetCollisionRect().w);
    targety = std::clamp(GetCollisionRect().y+down*fdelta*MoveSpeed,0.f,900.f-GetCollisionRect().h);
    SetCollisionRectPos(targetx,targety);
}
void EntityPlayer::Draw(SDL_Renderer* renderer,float interpolation){
    SDL_FRect nointerp = GetCollisionRect();
    SDL_SetRenderDrawColor(renderer, 230, 120, 70, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &nointerp);
}
