/**
 * @file    Window.hpp
 * @brief   SDL_Window 的 RAII 封装：创建窗口并独占其所有权。
 */

#pragma once
#include<SDL3/SDL.h>
#include<string_view>

/**
 * @brief SDL_Window 的 RAII 封装。
 *
 * 构造即创建窗口，析构即销毁窗口；禁止拷贝，保证"一个窗口句柄只有一个主人"。
 * @note 必须在 SDL 视频子系统初始化之后创建（本项目由 Subsystem 保证）。
 */
class Window {
    public:
        /**
         * @brief 创建窗口。
         * @param[in] title  窗口标题（借用，不保存指针）。
         * @param[in] width  客户区宽度（像素）。
         * @param[in] height 客户区高度（像素）。
         * @note 失败时内部句柄为 nullptr，需用 IsValid() 检查。
         */
        explicit Window(std::string_view title,int width,int height);
        /**
         * @brief 禁止拷贝构造。
         * @note 句柄独占所有权；复制会导致两次 SDL_DestroyWindow。
         */
        Window(const Window&) = delete;
        /**
         * @brief 禁止拷贝赋值。
         */
        Window& operator=(const Window&) = delete;
        /**
         * @brief 销毁窗口（仅当创建成功时）。
         */
        ~Window();
        /**
         * @brief 查询窗口是否创建成功。
         * @return 创建成功返回 true。
         */
        bool IsValid() const;
        /**
         * @brief 借用底层句柄。
         * @return SDL_Window 指针；所有权仍归本类，调用方不得销毁。
         */
        SDL_Window* Get() const {return window_;}
    private:
        SDL_Window* window_ = nullptr; ///< 窗口句柄；nullptr 表示创建失败
};

// ---- 讲义（实现细节，不属于接口文档） ----
// explicit 禁用的是"拷贝列表初始化"，即 Window w = {"t",2,3}; 不再合法；
// 仍接受 Window w{"t",2,3}; 与 Window w("t",2,3); 两种写法。
// 禁止拷贝的原因：所有权归属必须唯一，避免双重释放（double free）。
