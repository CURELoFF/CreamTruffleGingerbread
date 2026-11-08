#include "Renderer.hpp"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

Renderer::Renderer(SDL_Window* target) {
    renderer_ = SDL_CreateRenderer(target, nullptr);
    SDL_SetRenderVSync(renderer_, 1);
}

Renderer::~Renderer() {
    if(renderer_) SDL_DestroyRenderer(renderer_);
}

bool Renderer::IsValid() const {
    return renderer_!=nullptr;
}
