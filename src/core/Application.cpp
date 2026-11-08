#include"Application.hpp"
#include"Collision.hpp"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_timer.h"

#include<algorithm>
#include<iostream>
#include<cstdlib>
#include<ctime>
//#include<utility>

Application::Application(std::string_view title,int width,int height) :
    subsystem_(SDL_INIT_VIDEO),
    window_(title,width,height),
    renderer_(window_.Get()){}

bool Application::IsValid() const {
    return subsystem_.IsValid() && window_.IsValid();
}

void Application::Run() {
    bool running = true;
    SDL_Event event;

    const Uint64 freq = SDL_GetPerformanceFrequency(); // 每秒 tick 数（机器常数）
    Uint64 last = SDL_GetPerformanceCounter(); // 记录上次运行到现在的时间间隔 并初始化值

    SDL_FRect player{50.f,50.f,100.f,100.f};
    SDL_FRect entity{200.f,200.f,50.f,50.f};

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    while(running)
    {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) running=false;
        }
        // ======================  Update  ====================== //
        const Uint64 now = SDL_GetPerformanceCounter();
        // Use dt instead of deltaTime because dt is a const
        double deltaTime = static_cast<double>(now-last) / static_cast<double>(freq);
        last = now;
        deltaTime = std::min(deltaTime,1.); // 限制最久时间间隔
        // Use this instead of deltaTime
        const double dt = deltaTime;
        // std::cout<<"frame : "<< 1 / dt <<std::endl;
        const bool* keys = SDL_GetKeyboardState(nullptr);

        if(keys[SDL_SCANCODE_ESCAPE]) running=false;

        float playerMoveSpeed = 64.f;
        float deltaPlayerMoveSpeed = playerMoveSpeed * static_cast<float>(dt);
        if(keys[SDL_SCANCODE_LEFT]) player.x -= deltaPlayerMoveSpeed;
        if(keys[SDL_SCANCODE_RIGHT]) player.x += deltaPlayerMoveSpeed;
        if(keys[SDL_SCANCODE_UP]) player.y -= deltaPlayerMoveSpeed;
        if(keys[SDL_SCANCODE_DOWN]) player.y += deltaPlayerMoveSpeed;
        int winw=0,winh=0;
        SDL_GetWindowSize(window_.Get(),&winw,&winh);
        player.x = std::clamp(player.x,0.f,static_cast<float>(winw)-player.w);
        player.y = std::clamp(player.y,0.f,static_cast<float>(winh)-player.h);

        // Collision check
        if(Intersects(player, entity)) {
            std::cout<<"Hit at "<<player.x<<","<<player.y<<std::endl;
            entity.x = static_cast<float>(std::rand() % (winw - static_cast<int>(entity.w)));
            entity.y = static_cast<float>(std::rand() % (winh - static_cast<int>(entity.h)));
        }

        // ======================  Render  ====================== //
        // update script goes here.
        SDL_SetRenderDrawColor(renderer_.Get(),32,32,48,SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer_.Get());
        SDL_SetRenderDrawColor(renderer_.Get(), 230, 120, 70, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer_.Get(), &player);
        SDL_SetRenderDrawColor(renderer_.Get(),127, 0, 9, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer_.Get(),&entity);
        SDL_RenderPresent(renderer_.Get());
    }
    std::cout<<"Application clean quit."<<std::endl;
}
