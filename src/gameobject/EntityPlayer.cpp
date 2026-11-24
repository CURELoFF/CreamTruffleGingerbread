/**
 * @file    EntityPlayer.cpp
 * @brief   EntityPlayer 的实现：键盘输入、位置裁剪、动画帧推进与贴图绘制。
 */

#include "EntityPlayer.hpp"
#include "SDL3/SDL_rect.h"
#include <SDL3/SDL_render.h>
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <utility>

EntityPlayer::EntityPlayer(float worldW, float worldH, SDL_Renderer* renderer, SheetDesc sheet)
    : worldW_(worldW), worldH_(worldH),
      sheet_(std::move(sheet)),                 // 清单按值接管：不共享、不悬空
      texture_(renderer, sheet_.texture) {      // 依赖 sheet_，所以必须声明在 sheet_ 之后
    // 贴图是硬依赖：加载不到就没有任何可画的东西，直接让启动失败
    if(!texture_.IsValid())
        throw std::runtime_error("cannot load texture: " + sheet_.texture);

    // 清单里的坐标必须真的落在图上：语法正确的 JSON 也可能整体读错方向
    const SDL_FPoint size = texture_.Size();

    // Debug: 将碰撞矩形设为贴图尺寸
    collisionRect_.Snap(SDL_FRect{0,0,static_cast<float>(sheet_.frameWidth),static_cast<float>(sheet_.frameHeight)});

    if(const std::string err = sheet_.ValidateFits(static_cast<int>(size.x), static_cast<int>(size.y)); !err.empty())
        throw std::runtime_error(err);

    texture_.SetNearestScale();                 // 像素画放大用最近邻，避免边缘糊掉

    const ClipDesc* idle = sheet_.Find(ClipNameOf(dir_));   // 初始朝向：朝下
    if(idle == nullptr)
        throw std::runtime_error(std::string("clip not found: ") + ClipNameOf(dir_));
    animator_.Play(idle);
}

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

    const bool moving = (right != 0.f || down != 0.f);

    // 朝向：水平优先（同时按两个方向时以左右为准），只在方向真的改变时切片段
    // 若每帧都调 Play()，动画会被反复复位，永远停在第 0 帧
    if(moving) {
        Direction next = dir_;
        if(right < 0.f)      next = Direction::Left;
        else if(right > 0.f) next = Direction::Right;
        else if(down  < 0.f) next = Direction::Up;
        else                 next = Direction::Down;

        if(next != dir_) {
            dir_ = next;
            animator_.Play(sheet_.Find(ClipNameOf(dir_)));
        }
    }

    SDL_FRect& r = collisionRect_.Write();   // 唯一写通道（第一个写手=自动快照，保存修改前的值）
    if(moving) {
        const float step = MoveSpeed * fdelta;
        // clamp 把矩形限制在世界内：上界要减自身宽高，否则右下角会越界
        r.x = std::clamp(r.x + right*step, 0.f, worldW_ - r.w);
        r.y = std::clamp(r.y + down *step, 0.f, worldH_ - r.h);
        animator_.Update(fdelta);   // 动画只在移动时推进：站着不该"原地踏步"
    } else {
        animator_.Reset();          // 站住：回到走路第 0 帧（本表没有独立 Idle 片段）
    }
}

void EntityPlayer::Draw(SDL_Renderer* renderer,float interpolation){
    SDL_FRect shown = collisionRect_.Rendered(interpolation);   // 读通道：插值视图
    SDL_SetRenderDrawColor(renderer, 230, 120, 70, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &shown);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // 只画当前帧那一格；清单或纹理缺失时退回纯色块
    if(texture_.IsValid() && animator_.HasClip()) {
        const SDL_FRect src = SrcRect(sheet_, animator_.Clip(), animator_.FrameIndex());

        // 整数倍放大 + 脚底对齐碰撞盒底边：非整数倍会让像素宽窄不一（有的 2px 有的 3px）
        constexpr float kScale = 1.0f;
        const float dw = static_cast<float>(sheet_.frameWidth)  * kScale;
        const float dh = static_cast<float>(sheet_.frameHeight) * kScale;
        const SDL_FRect dst{
            shown.x + (shown.w - dw) * 0.5f,   // 水平居中
            shown.y + shown.h - dh,            // 脚底贴住碰撞盒底边
            dw, dh
        };
        texture_.Draw(renderer, &src, dst);
    }
}
