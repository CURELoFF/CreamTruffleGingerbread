/**
 * @file    Renderer.hpp
 * @brief   SDL_Renderer 的 RAII 封装（绘制指令的提交入口）。
 */

#pragma once
#include "SDL3/SDL_render.h"
#include<SDL3/SDL.h>

/**
 * @brief SDL_Renderer 的 RAII 封装。
 *
 * 构造时创建渲染器并开启垂直同步；析构时销毁。
 * @note 渲染器由某块窗口创建，必须**先于该窗口**析构；
 *       本项目中由 Application 的成员声明顺序保证。
 * @note 禁用拷贝：渲染器句柄独占所有权。
 */
class Renderer {
public:
    /**
     * @brief 为指定窗口创建渲染器，并开启 vsync（1 = 每帧同步一次）。
     * @param[in] target 目标窗口（借用，不接管所有权，但必须活得比本对象久）。
     * @note 失败时内部句柄为 nullptr，需用 IsValid() 检查。
     */
    explicit Renderer(SDL_Window* target);
    /**
     * @brief 销毁渲染器。
     */
    ~Renderer();

    /**
     * @brief 禁止拷贝构造。
     */
    Renderer(const Renderer&) = delete;
    /**
     * @brief 禁止拷贝赋值。
     */
    Renderer& operator=(const Renderer&) = delete;

    /**
     * @brief 查询渲染器是否创建成功。
     * @return 创建成功返回 true。
     */
    bool IsValid() const;
    /**
     * @brief 借用底层句柄。
     * @return SDL_Renderer 指针；所有权仍归本类，调用方不得销毁。
     */
    SDL_Renderer* Get() const {return renderer_;};

private:
    SDL_Renderer* renderer_ = nullptr; ///< 渲染器句柄；nullptr 表示创建失败
};
