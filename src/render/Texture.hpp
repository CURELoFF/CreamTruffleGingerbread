#pragma once

#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <string_view>
#include <string>

class Texture {
public:
    /**
     * @brief 构造函数
     */
    explicit Texture(SDL_Renderer* renderer,SDL_Surface* surface):texture_(SDL_CreateTextureFromSurface(renderer,surface)){}
    /**
     * @brief 构造函数
     */
    explicit Texture(SDL_Renderer* renderer,std::string_view path):texture_(IMG_LoadTexture(renderer,std::string(path).c_str())){}
    /**
     * @brief 析构函数
     */
    ~Texture(){if(texture_)SDL_DestroyTexture(texture_);};
    /**
     * 禁用复制构造
     */
    Texture(const Texture&) = delete;
    /**
     * 禁用拷贝赋值
     */
    Texture& operator=(const Texture&) = delete;
    /**
     * @brief 获取Texture
     */
    SDL_Texture* Get() const {return texture_;}
    /**
     * @brief 检查Texture是否存在
     */
    bool IsValid() const {return texture_!=nullptr;}
private:
    SDL_Texture* texture_=nullptr;

};
