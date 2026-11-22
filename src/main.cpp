/**
 * @file    main.cpp
 * @brief   程序入口：创建 Application 并运行主循环。
 */

#include <SDL3/SDL.h>
#include <cstdlib>

#include "core/Application.hpp"

#include <iostream>

/**
 * @brief 启动游戏并运行主循环。
 * @return 正常退出返回 EXIT_SUCCESS；启动失败返回 EXIT_FAILURE。
 * @note 进程退出码用 EXIT_* 而不是魔数 0/1，方便外部脚本判断。
 */
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
