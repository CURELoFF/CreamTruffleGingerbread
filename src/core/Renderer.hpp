#pragma once
#include "SDL3/SDL_render.h"
#include<SDL3/SDL.h>

class Renderer {
public:
    explicit Renderer(SDL_Window* target);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool IsValid() const;
    SDL_Renderer* Get() const {return renderer_;};

private:
    SDL_Renderer* renderer_ = nullptr;
};
