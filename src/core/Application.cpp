#include"Application.hpp"
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_scancode.h"
#include<algorithm>

#include<iostream>
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

    SDL_FRect rect{100.f,100.f,200.f,200.f};

    while(running)
    {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) running=false;
        }
        // ======================  Update  ======================
        const bool* keys = SDL_GetKeyboardState(nullptr);

        if(keys[SDL_SCANCODE_ESCAPE]) running=false;

        float step = 8.f;
        if(keys[SDL_SCANCODE_LEFT]) rect.x -= step;
        if(keys[SDL_SCANCODE_RIGHT]) rect.x += step;
        if(keys[SDL_SCANCODE_UP]) rect.y -= step;
        if(keys[SDL_SCANCODE_DOWN]) rect.y += step;
        int winw=0,winh=0;
        SDL_GetWindowSize(window_.Get(),&winw,&winh);
        rect.x = std::clamp(rect.x,0.f,static_cast<float>(winw)-rect.w);
        rect.y = std::clamp(rect.y,0.f,static_cast<float>(winh)-rect.h);



        // ======================  Render  ======================
        // update script goes here.
        SDL_SetRenderDrawColor(renderer_.Get(),32,32,48,SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer_.Get());
        SDL_SetRenderDrawColor(renderer_.Get(), 230, 120, 70, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer_.Get(), &rect);
        SDL_RenderPresent(renderer_.Get());
    }
    std::cout<<"Application clean quit."<<std::endl;
}
