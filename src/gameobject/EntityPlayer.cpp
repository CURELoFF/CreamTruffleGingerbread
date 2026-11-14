#include"EntityPlayer.hpp"
#include "SDL3/SDL_rect.h"
#include <algorithm>

EntityPlayer::EntityPlayer(float worldW, float worldH)
    : worldW_(worldW), worldH_(worldH) {}

void EntityPlayer::Update(double delta){
    collisionRect_.BeginStep();   // 拍首契约：旗子复位（会写状态的对象第一行）

    float fdelta = static_cast<float>(delta);
    const bool* keys = SDL_GetKeyboardState(nullptr);
    float right=0,down=0;
    if(keys[SDL_SCANCODE_LEFT]) right  = -1;
    if(keys[SDL_SCANCODE_RIGHT]) right =  1;
    if(keys[SDL_SCANCODE_UP]) down     = -1;
    if(keys[SDL_SCANCODE_DOWN]) down   =  1;

    SDL_FRect& r = collisionRect_.Write();   // 唯一写通道（第一个写手=自动快照）, 快照当前值被修改前的值.
    const float step = MoveSpeed * fdelta;
    r.x = std::clamp(r.x + right*step, 0.f, worldW_ - r.w);
    r.y = std::clamp(r.y + down *step, 0.f, worldH_ - r.h);
}

void EntityPlayer::Draw(SDL_Renderer* renderer,float interpolation){
    SDL_FRect debug_nointerp = collisionRect_.Current();
    SDL_FRect shown = collisionRect_.Rendered(interpolation);   // 读通道：插值视图
    SDL_SetRenderDrawColor(renderer, 230, 120, 70, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &shown);
}
