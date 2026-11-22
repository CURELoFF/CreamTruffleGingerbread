/**
 * @file    Renderer.cpp
 * @brief   Renderer 的实现：创建渲染器、开启 vsync、销毁。
 */

#include "Renderer.hpp"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

Renderer::Renderer(SDL_Window* target) {
    renderer_ = SDL_CreateRenderer(target, nullptr);
    // vsync = 1：Present 时阻塞到下一次屏幕刷新，天然把帧率钉在显示器刷新率上
    SDL_SetRenderVSync(renderer_, 1);
}

Renderer::~Renderer() {
    // 用 if 守卫：创建失败时 renderer_ 为 nullptr
    if(renderer_) SDL_DestroyRenderer(renderer_);
}

bool Renderer::IsValid() const {
    return renderer_!=nullptr;
}
