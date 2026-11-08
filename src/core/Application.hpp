/**
 * @file Application.hpp
 * @brief 应用程序封装
 * @author me
 * @date 2026-11-7
 * @version ALPHA
 */

#pragma once
#include<SDL3/SDL.h>
#include<string_view>

#include "Window.hpp"
// --- 子系统 RAII：太小，不值得单独文件，与 Application 同住 ---
class Subsystem {
public:
    explicit Subsystem(SDL_InitFlags flags) : ok_(SDL_Init(flags)) {}
    ~Subsystem() {SDL_Quit();}

    Subsystem(const Subsystem&) = delete;
    Subsystem& operator=(const Subsystem&) = delete;

    bool IsValid() const {return ok_;}
private:
    bool ok_ = false;
};
// --- 框架骨架：拥有子系统与窗口，并提供主循环 ---
/**
 *
 */
class Application {
public:
    /**
     * @brief 构造应用程序(SDL+Window)
     * @param[in] title 标题
     * @param[in] width 窗口宽
     * @param[in] height 窗口高
     */
    explicit Application(std::string_view title,int width,int height);
    /**
     * @brief 返回窗口是否正常运行
     */
    bool IsValid() const;
    /**
     * 运行应用程序
     */
    void Run();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
private:
    Subsystem subsystem_;
    Window window_;
};
