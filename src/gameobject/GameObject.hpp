/**
 * @file    GameObject.hpp
 * @brief   所有游戏实体的抽象基类（纯接口）。
 */

#pragma once

#include<SDL3/SDL.h>

/**
 * @brief 游戏实体抽象基类。
 *
 * 只规定"每拍更新"与"每帧绘制"两件事，不持有任何状态；
 * 具体实体以 std::unique_ptr<GameObject> 的形式统一存放和遍历。
 * @note 虚析构：通过基类指针删除派生对象时，必须执行派生类自己的析构。
 */
class GameObject {
public:
    /**
     * @brief 虚析构：保证 delete 基类指针时派生类析构被调用。
     */
    virtual ~GameObject() = default;

    /**
     * @brief 推进一拍的逻辑（固定步长下调用）。
     * @param[in] delta 固定步长（秒），本项目中恒为 1/60。
     * @note 纯虚（= 0）：基类无法定义"更新"的语义，派生类必须实现。
     */
    virtual void Update(double delta) = 0;
    /**
     * @brief 绘制当前状态（每个渲染帧调用一次）。
     * @param[in] renderer      渲染目标（借用，不接管所有权）。
     * @param[in] interpolation 插值系数 ∈ [0,1)，用于在两拍之间平滑显示位置。
     */
    virtual void Draw(SDL_Renderer* renderer,float interpolation) = 0;
};
