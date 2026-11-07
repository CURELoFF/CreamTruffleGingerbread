#include <SDL3/SDL.h>
#include <cstdlib>
#include "SDL3/SDL_error.h"
#include "core/Application.hpp"

#include <cstdio>
#include <iostream>

int main() {
    Application app("C++ Toolchained Game",1600,900);
    if(!app.IsValid()) {
        std::cerr<<"Startup failed "<<SDL_GetError()<<std::endl;
        return EXIT_FAILURE;
    }
    std::cout<<"Application is running. Close the window to quit.\n";
    app.Run();
    return EXIT_SUCCESS;
}
