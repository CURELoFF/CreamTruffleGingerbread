#include<algorithm>
#include<iostream>
#include<cstdlib>
#include<ctime>
#include<vector>
//#include<utility>

#include"Application.hpp"
#include"Collision.hpp"
#include"../gameobject/EntityPlayer.hpp"
#include"../gameobject/EntityEnemy.hpp"

Application::Application(std::string_view title,int width,int height) :
    subsystem_(SDL_INIT_VIDEO),
    window_(title,width,height),
    renderer_(window_.Get()){}

bool Application::IsValid() const {
    return subsystem_.IsValid() && window_.IsValid();
}

void Application::Run() {
    // Aliases
    int init_win_w,init_win_h;
    SDL_GetWindowSize(window_.Get(), &init_win_w, &init_win_h);
    // Application
    bool running = true;
    SDL_Event event;
    // Update
    const Uint64 freq = SDL_GetPerformanceFrequency(); // 每秒 tick 数（机器常数）
    Uint64 last = SDL_GetPerformanceCounter(); // 记录上次运行到现在的时间间隔 并初始化值
    constexpr double STEP = 1.0/60.0; // 固定步长
    double acc = 0.0; //累加器
    // Init random seed.
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Init instances
    EntityPlayer player{};
    EntityEnemy enemy{};

    while(running)
    {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) running=false;
        }
        // ======================  Update  ====================== //
        const double maxDeltaTime = 0.1;

        const Uint64 now = SDL_GetPerformanceCounter();
        acc += std::min(maxDeltaTime,static_cast<double>(now-last) / static_cast<double>(freq));
        last = now;
        // 使用固定步长 , 每帧处理一次 -> 每步处理一次 .每秒帧数不固定,步数一般固定
        while(acc >= STEP) {
            int winw=0,winh=0;
            SDL_GetWindowSize(window_.Get(),&winw,&winh);

            // std::cout<<"frame : "<< 1 / dt <<std::endl;
            const bool* keys = SDL_GetKeyboardState(nullptr);

            if(keys[SDL_SCANCODE_ESCAPE]) running=false;
            // 顺序目前不影响，因为在碰撞检测时这些实例必将全都计算完毕
            player.Update(STEP);
            enemy.Update(STEP);
            // Collision check
            if(Intersects(player.GetCollisionRect(), enemy.GetCollisionRect())) {
                // if hit, random enemy's position.
                std::cout<<"Hit at "<<player.GetCollisionRect().x<<","<<player.GetCollisionRect().y<<std::endl;
                 enemy.SetCollisionRectPos(static_cast<float>(std::rand()%1500),static_cast<float>(std::rand()%800));
            }

            acc -= STEP;
        }


        // ======================  Render  ====================== //
        // update script goes here.
        float interpolation = static_cast<float>(acc / STEP);
        SDL_SetRenderDrawColor(renderer_.Get(),32,32,48,SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer_.Get());
        player.Draw(renderer_.Get(), interpolation);
        enemy.Draw(renderer_.Get(), interpolation);
        SDL_RenderPresent(renderer_.Get());
    }
    std::cout<<"Application clean quit."<<std::endl;
}
