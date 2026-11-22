/**
 * @file    Application.hpp
 * @brief   应用骨架：持有 SDL 子系统、窗口与渲染器，并驱动主循环。
 */

#pragma once
#include<SDL3/SDL.h>
#include<string_view>

#include "Window.hpp"
#include "Renderer.hpp"

// --- 子系统 RAII：太小，不值得单独文件，与 Application 同住 ---
/**
 * @brief SDL 子系统的 RAII 守门人：Init 即构造，Quit 即析构。
 *
 * @note SDL_Quit 会关闭全部子系统，因此全进程只应存在一个 Subsystem 实例。
 * @note 禁用拷贝：避免析构两次导致重复 Quit。
 */
class Subsystem {
public:
    /**
     * @brief 初始化指定子系统。
     * @param[in] flags SDL_InitFlags 的按位或组合（如 SDL_INIT_VIDEO）。
     */
    explicit Subsystem(SDL_InitFlags flags) : ok_(SDL_Init(flags)) {}
    /** @brief 关闭 SDL（全部子系统）。 */
    ~Subsystem() {SDL_Quit();}

    /**
     * @brief 禁止拷贝构造。
     */
    Subsystem(const Subsystem&) = delete;
    /**
     * @brief 禁止拷贝赋值。
     */
    Subsystem& operator=(const Subsystem&) = delete;

    /**
     * @brief 查询子系统初始化是否成功。
     * @return 成功返回 true。
     */
    bool IsValid() const {return ok_;}
private:
    bool ok_ = false; ///< SDL_Init 的返回值；0 表示全部成功
};

// --- 框架骨架：拥有子系统与窗口，并提供主循环 ---
/**
 * @brief 框架骨架：拥有子系统、窗口与渲染器，并驱动主循环。
 *
 * 成员声明顺序 = 构造顺序；析构按逆序进行，
 * 因此 renderer_ 先于 window_ 销毁（渲染器依赖窗口）。
 * @note 禁用拷贝：三个成员都是独占所有权的资源，不可复制。
 */
class Application {
public:
    /**
     * @brief 构造应用程序（SDL 子系统 + 窗口 + 渲染器）。
     * @param[in] title  窗口标题。
     * @param[in] width  窗口宽度（像素）。
     * @param[in] height 窗口高度（像素）。
     */
    explicit Application(std::string_view title,int width,int height);
    /**
     * @brief 查询初始化是否成功。
     * @return 子系统与窗口都创建成功时返回 true。
     */
    bool IsValid() const;
    /**
     * @brief 运行主循环，直到关闭窗口或按 ESC。
     * @note 内部为"固定步长逻辑 + 每帧插值渲染"结构。
     */
    void Run();

    /**
     * @brief 禁止拷贝构造。
     */
    Application(const Application&) = delete;
    /**
     * @brief 禁止拷贝赋值。
     */
    Application& operator=(const Application&) = delete;
private:
    Subsystem subsystem_; ///< SDL 生命周期守门人，必须最先构造、最后析构
    Window window_;       ///< 窗口，渲染器依赖它
    Renderer renderer_;   ///< 渲染器，必须早于 window_ 销毁
};
