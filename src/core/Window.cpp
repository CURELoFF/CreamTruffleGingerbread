#include"Window.hpp"

Window::Window(std::string_view title,int width,int height) {
    window_ = SDL_CreateWindow(title.data(),width,height,0);
}

Window::~Window() {
    if(window_) SDL_DestroyWindow(window_);
}

bool Window::IsValid() const {
    return window_ != nullptr;
}
