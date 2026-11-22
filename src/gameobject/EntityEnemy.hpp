/**
 * @file    EntityEnemy.hpp
 * @brief   敌人实体（当前为纯色方块占位实现）。
 */

#pragma once

#include<SDL3/SDL.h>

#include"GameObject.hpp"

/**
 * @brief 敌人实体。
 *
 * 目前没有自主行为，只提供一块可被碰撞检测读取、可被瞬移的矩形；
 * 后续 D 阶段的敌人 AI 会在这里加入回合制行为。
 */
class EntityEnemy : public GameObject{
public:
    /**
     * @brief 碰撞/绘制矩形（逻辑态，不含插值）。
     * @return 当前矩形的位置与尺寸（像素）。
     */
    SDL_FRect GetCollisionRect() const {return collisionRect_;}
    /**
     * @brief 瞬移敌人到指定位置（碰撞后重随机用）。
     * @param[in] x 新左上角 x（像素）。
     * @param[in] y 新左上角 y（像素）。
     */
    void SetCollisionRectPos(float x,float y){collisionRect_.x = x;collisionRect_.y=y;}

    /**
     * @brief 敌人目前没有行为，留空待 AI 阶段实现。
     * @param[in] delta 固定步长（秒），当前未使用。
     */
    void Update(double delta) override;
    /**
     * @brief 以方块形式绘制自身。
     * @param[in] renderer      渲染目标（借用）。
     * @param[in] interpolation 当前版本不做插值，参数暂未使用。
     */
    void Draw(SDL_Renderer* renderer,float interpolation) override;
private:
    SDL_FRect collisionRect_{0,0,25.f,25.f}; ///< 位置与尺寸（像素），初始位于左上角
};
