/**
 * @file    Surface.hpp
 * @brief   SDL_Surface 的 RAII 封装（CPU 侧像素缓冲区）。
 */

#pragma once

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_surface.h>
#include<SDL3_image/SDL_image.h>
#include <iostream>
#include<string_view>
#include<string>

/**
 * @brief SDL_Surface 的 RAII 封装：图像解码后的 CPU 内存像素。
 *
 * 构造时用 SDL_image 从文件解码，析构时释放。
 * @note 使用流程：Surface 解码 → Texture 上传显存 → Surface 可以立刻销毁。
 * @note 禁用拷贝：像素缓冲区独占所有权，复制会导致重复释放。
 */
class Surface {
public:
    /**
     * @brief 从图片文件解码到 CPU 内存。
     * @param[in] path 图片路径，相对当前工作目录。
     * @note 失败时内部句柄为 nullptr 并打印错误；需用 IsValid() 检查。
     * @note std::string(path) 只是为拿到 null 结尾的 c_str()，临时对象在本表达式内有效。
     */
    explicit Surface(std::string_view path): surface_(IMG_Load(std::string(path).c_str())){
        if(!surface_) std::cerr<<"IMG Load Failed:"<<path<<" - "<<SDL_GetError()<<"\n";
    }
    /**
     * @brief 释放像素缓冲区。
     */
    ~Surface(){if(surface_) SDL_DestroySurface(surface_);}

    /// 禁用拷贝构造：Surface b = a; 不允许（所有权不明确）
    Surface(const Surface&) = delete;
    /// 禁用拷贝赋值：b = a; 不允许
    Surface& operator=(const Surface&) = delete;
    /**
     * @brief 借用底层句柄。
     * @return SDL_Surface 指针；所有权仍归本类，调用方不得销毁。
     */
    SDL_Surface* Get() const {return surface_;}
    /**
     * @brief 查询解码是否成功。
     * @return 成功返回 true。
     */
    bool IsValid() const {return surface_!=nullptr;}

private:
    SDL_Surface* surface_ = nullptr; ///< 像素缓冲区句柄；nullptr 表示解码失败
};
