/**
 * @file    EntityPlayer.cpp
 * @brief   EntityPlayer 的实现：键盘输入、位置裁剪与贴图绘制。
 */

#include"EntityPlayer.hpp"
#include "SDL3/SDL_rect.h"
#include <SDL3/SDL_render.h>
#include <algorithm>
#include <cstdlib>

EntityPlayer::EntityPlayer(float worldW, float worldH,SDL_Renderer* renderer,SDL_Surface* surface)
    : worldW_(worldW), worldH_(worldH), texture_(renderer,surface){}

void EntityPlayer::Update(double delta){
    collisionRect_.BeginStep();   // 拍首契约：旗子复位（会写状态的对象第一行）

    float fdelta = static_cast<float>(delta);
    const bool* keys = SDL_GetKeyboardState(nullptr);
    // 先算方向再乘速度：方向取 ±1，因此斜向不会比直线更快（未做归一化，斜向仍是 1.41 倍）
    float right=0,down=0;
    if(keys[SDL_SCANCODE_LEFT]) right  = -1;
    if(keys[SDL_SCANCODE_RIGHT]) right =  1;
    if(keys[SDL_SCANCODE_UP]) down     = -1;
    if(keys[SDL_SCANCODE_DOWN]) down   =  1;

    SDL_FRect& r = collisionRect_.Write();   // 唯一写通道（第一个写手=自动快照，保存修改前的值）
    const float step = MoveSpeed * fdelta;
    // clamp 把矩形限制在世界内：上界要减自身宽高，否则右下角会越界
    r.x = std::clamp(r.x + right*step, 0.f, worldW_ - r.w);
    r.y = std::clamp(r.y + down *step, 0.f, worldH_ - r.h);
}

void EntityPlayer::Draw(SDL_Renderer* renderer,float interpolation){
    SDL_FRect debug_nointerp = collisionRect_.Current();
    SDL_FRect shown = collisionRect_.Rendered(interpolation);   // 读通道：插值视图
    SDL_SetRenderDrawColor(renderer, 230, 120, 70, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &shown);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // 贴图以整幅矩形为目标区域；失败时 IsValid 为假，退回只画色块
    if(texture_.IsValid())
        SDL_RenderTexture(renderer, texture_.Get(), nullptr, &shown );
}
