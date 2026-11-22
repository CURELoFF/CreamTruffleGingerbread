/**
 * @file    Window.cpp
 * @brief   Window 的实现：构造、析构与状态查询。
 */

#include"Window.hpp"

Window::Window(std::string_view title,int width,int height) {
    window_ = SDL_CreateWindow(title.data(),width,height,0);
}

Window::~Window() {
    // 先判空：构造失败时句柄为 nullptr，SDL_DestroyWindow(nullptr) 虽安全但不该依赖
    if(window_) SDL_DestroyWindow(window_);
}

bool Window::IsValid() const {
    return window_ != nullptr;
}
