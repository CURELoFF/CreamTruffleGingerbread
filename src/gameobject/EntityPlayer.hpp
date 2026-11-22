/**
 * @file    EntityPlayer.hpp
 * @brief   玩家实体：位置由 InterpProperty 持有，绘制时叠加贴图。
 */

#pragma once

#include <SDL3/SDL.h>

#include "GameObject.hpp"
#include "../animation/InterpProperty.hpp"
#include "../render/Texture.hpp"
#include "../render/Surface.hpp"

/**
 * @brief 玩家实体：位置的唯一权威 = InterpProperty<SDL_FRect>。
 *
 * 逻辑读写走 Write()/Current()，渲染读取走 Rendered(alpha)，
 * 两条通道共用同一份状态，避免"碰撞用一份坐标、画图用另一份"的错位。
 * @note 本类持有 Texture，生命周期必须短于传入的 SDL_Renderer。
 */
class EntityPlayer : public GameObject {
public:
    /**
     * @brief 构造玩家：注入世界边界与渲染资源。
     * @param[in] worldW   世界（窗口）宽（像素），用于右边界裁剪。
     * @param[in] worldH   世界（窗口）高（像素），用于下边界裁剪。
     * @param[in] renderer 渲染器（借用，用于创建纹理）。
     * @param[in] surface  已加载的像素数据（借用，仅在构造期读取）。
     */
    EntityPlayer(float worldW, float worldH,SDL_Renderer* renderer,SDL_Surface* surface);

    /**
     * @brief 碰撞矩形的逻辑态（碰撞判定用这个，不用插值视图）。
     * @return 当前拍的矩形。
     */
    SDL_FRect GetCollisionRect() const {return collisionRect_.Current();}

    /**
     * @brief 读键盘并推进一拍位置，结果被世界边界裁剪。
     * @param[in] delta 固定步长（秒）。
     * @note 第一行调用 BeginStep() 是 InterpProperty 的拍首契约。
     */
    void Update(double delta) override;
    /**
     * @brief 先画占位色块，再叠加玩家贴图。
     * @param[in] renderer      渲染目标（借用）。
     * @param[in] interpolation 插值系数 ∈ [0,1)，用于平滑显示位置。
     */
    void Draw(SDL_Renderer* renderer,float interpolation) override;

    float MoveSpeed = 1024.f; ///< 移动速度（像素/秒）
private:
    InterpProperty<SDL_FRect> collisionRect_{SDL_FRect{50.f,50.f,100.f,100.f}}; ///< 位置唯一权威（prev/curr 两拍）
    Texture texture_; ///< 玩家贴图；必须晚于 Renderer 析构

    /* 临时变量 */
    float worldW_ = 0.f;   ///< 世界宽度（像素），构造注入
    float worldH_ = 0.f;   ///< 世界高度（像素），构造注入

};
