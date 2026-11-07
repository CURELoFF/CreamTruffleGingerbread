#include <SDL3/SDL.h>

#include <cstdio>
#include <iostream>

int main() {
    std::cout<<"Program Start"<<std::endl;
    // --- 1. Report which SDL version we linked against ---
    // SDL3 packs the version into a single int:
    //   major * 1_000_000 + minor * 1_000 + patch
    const int version = SDL_GetVersion();
    std::printf(
        "SDL3 runtime version: %d.%d.%d (packed: %d)\n",
        version / 1'000'000,
        (version / 1'000) % 1'000,
        version % 1'000,
        version
    );

    // --- 2. Initialize the VIDEO subsystem ---
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        // SDL3 functions return bool: false signals failure.
        // SDL_GetError() returns a description of the last error.
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return EXIT_FAILURE;
    }
    std::cout << "SDL_Init(VIDEO): OK\n";

    // --- 3. Clean shutdown ---
    SDL_Quit();
    std::cout << "SDL_Quit: done.\n";
    return EXIT_SUCCESS;
}
