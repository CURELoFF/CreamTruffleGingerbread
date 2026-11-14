#pragma once

#include <SDL3/SDL_rect.h>    // SDL_FRect
#include <SDL3/SDL_render.h>  // SDL_Renderer

#include"GameObject.hpp"
#include"../animation/InterpProperty.hpp"

/**
 * @brief 玩家实体：位置的唯一权威 = InterpProperty<SDL_FRect>
 */
class EntityPlayer : public GameObject {
public:
    /**
     * @brief 出生时注入环境（窗口尺寸）——1600/900 魔数退役
     * @param[in] worldW 世界（窗口）宽
     * @param[in] worldH 世界（窗口）高
     */
    EntityPlayer(float worldW, float worldH);

    /**
     * @brief 碰撞矩形的逻辑态（碰撞判定用这个，不用插值视图）
     */
    SDL_FRect GetCollisionRect() const {return collisionRect_.Current();}

    void Update(double delta) override;
    void Draw(SDL_Renderer* renderer,float interpolation) override;

    float MoveSpeed = 1024.f;
private:
    InterpProperty<SDL_FRect> collisionRect_{SDL_FRect{50.f,50.f,100.f,100.f}};
    float worldW_ = 0.f;   // 环境边界（构造注入）
    float worldH_ = 0.f;
};
