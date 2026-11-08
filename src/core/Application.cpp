#include"Application.hpp"

#include<iostream>
//#include<utility>

Application::Application(std::string_view title,int width,int height) : subsystem_(SDL_INIT_VIDEO),window_(title,width,height){}

bool Application::IsValid() const {
    return subsystem_.IsValid() && window_.IsValid();
}

void Application::Run() {
    bool running = true;
    SDL_Event event;

    while(running)
    {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) running=false;
        }
        // update script goes here.
    }
    std::cout<<"Application clean quit."<<std::endl;
}
