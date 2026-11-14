#pragma once

#include<SDL3/SDL.h>

class GameObject {
public:
    /// 确保在析构时派生类会执行自己的析构函数
    virtual ~GameObject() = default;

    /// = 0 表示"纯虚",派生必须实现
    /**
     * @brief Update
     */
    virtual void Update(double delta) = 0;
    /**
     * @brief Draw
     */
    virtual void Draw(SDL_Renderer* renderer,float interpolation) = 0;
};
