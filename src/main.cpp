#include <SDL3/SDL.h>

#include <cstdio>
#include <iostream>

int main() {
    // --- 1. Initialize the VIDEO subsystem ---
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return EXIT_FAILURE;
    }

    // --- 2. Create the main window ---
    // SDL3 signature: title, width, height, SDL_WindowFlags.
    // (SDL2 took an x/y position too; SDL3 centers windows by default.)
    SDL_Window* window = SDL_CreateWindow(
        "CreamTruffleGingerbread",                                  // title
        1600, 900,                                                   // size in pixels
        SDL_WINDOW_RESIZABLE                                        // flags: user can resize
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        SDL_Quit();
        return EXIT_FAILURE;
    }
    std::cout << "Window created. Close it (or press Alt+F4) to quit.\n";

    // --- 3. The event loop: the heart of every game ---
    bool running = true;
    SDL_Event event;

    while (running) {
        // Drain all pending events this frame.
        // SDL3 uses SDL_PollEvent(out SDL_Event*) returning true while events remain.
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {   // user clicked the X / Alt+F4
                running = false;
            }
        }
        // (Later: update -> render goes here, once per frame.)
    }

    // --- 4. Clean shutdown, in reverse order of creation ---
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << "Exited cleanly.\n";
    return EXIT_SUCCESS;
}
