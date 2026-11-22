#pragma once

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_surface.h>
#include<SDL3_image/SDL_image.h>
#include <iostream>
#include<string_view>
#include<string>

class Surface {
public:
    /**
     * @brief 构造
     */
    explicit Surface(std::string_view path): surface_(IMG_Load(std::string(path).c_str())){
        if(!surface_) std::cerr<<"IMG Load Failed:"<<path<<" - "<<SDL_GetError()<<"\n";
    }
    /**
     * @brief 析构
     */
    ~Surface(){if(surface_) SDL_DestroySurface(surface_);}

    /// 禁用拷贝构造
    /// eg: Surface a{调用构造函数初始化}
    /// Surface b = a; 不允许
    Surface(const Surface&) = delete;
    /// 禁用拷贝赋值
    /// eg: Surface a,b;
    /// b = a; 不允许
    Surface& operator=(const Surface&) = delete;
    /**
     * @brief 获取surface句柄
     */
    SDL_Surface* Get() const {return surface_;}
    /**
     * @brief 检查surface数据是否创建成功
     */
    bool IsValid() const {return surface_!=nullptr;}

private:
    SDL_Surface* surface_ = nullptr;
};
