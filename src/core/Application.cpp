#include"Application.hpp"
#include"Collision.hpp"
#include "SDL3/SDL_rect.h"
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
    constexpr double STEP = 1.0/60.0; // 固定步长
    double acc = 0.0; //累加器

    SDL_FRect prev{50.f,50.f,100.f,100.f};
    SDL_FRect player{50.f,50.f,100.f,100.f};
    SDL_FRect entity{200.f,200.f,50.f,50.f};

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    while(running)
    {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) running=false;
        }
        // ======================  Update  ====================== //
        const double maxDeltaTime = 0.1;

        const Uint64 now = SDL_GetPerformanceCounter();
        acc += std::min(maxDeltaTime,static_cast<double>(now-last) / static_cast<double>(freq));
        // Use dt instead of deltaTime because dt is a const
        // double deltaTime = std::min(maxDeltaTime, static_cast<double>(now-last) / static_cast<double>(freq) );
        last = now;
        // 使用固定步长 , 每帧处理一次 -> 每步处理一次 .每秒帧数不固定,步数一般固定
        while(acc >= STEP) {
            prev = player;
            // std::cout<<"frame : "<< 1 / dt <<std::endl;
            const bool* keys = SDL_GetKeyboardState(nullptr);

            if(keys[SDL_SCANCODE_ESCAPE]) running=false;

            float playerMoveSpeed = 2048.f;
            float deltaPlayerMoveSpeed = playerMoveSpeed * static_cast<float>(STEP);
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

            acc -= STEP;
        }


        // ======================  Render  ====================== //
        // update script goes here.
        float interplation = static_cast<float>(acc / STEP);
        SDL_FRect display = player;
        display.x = prev.x + (player.x - prev.x)*interplation;
        display.y = prev.y + (player.y - prev.y)*interplation;

        SDL_SetRenderDrawColor(renderer_.Get(),32,32,48,SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer_.Get());
        SDL_SetRenderDrawColor(renderer_.Get(), 230, 120, 70, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer_.Get(), &display);
        SDL_SetRenderDrawColor(renderer_.Get(),127, 0, 9, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer_.Get(),&entity);
        SDL_RenderPresent(renderer_.Get());
    }
    std::cout<<"Application clean quit."<<std::endl;
}
