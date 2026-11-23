/**
 * @file    Texture.hpp
 * @brief   SDL_Texture 的 RAII 封装（已上传显存的图像）。
 */

#pragma once

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <string_view>
#include <string>

/**
 * @brief SDL_Texture 的 RAII 封装：像素已上传显存，可直接用于绘制。
 *
 * @warning 必须先于创建它的 SDL_Renderer 析构，否则纹理句柄悬空。
 * @note 禁用拷贝：显存句柄独占所有权，复制会导致重复释放。
 */
class Texture {
public:
    /**
     * @brief 从 CPU 侧 Surface 创建纹理（上传显存）。
     * @param[in] renderer 目标渲染器（借用，不接管所有权）。
     * @param[in] surface  像素来源（借用，创建后即可销毁）。
     * @note 失败时内部句柄为 nullptr，需用 IsValid() 检查。
     */
    explicit Texture(SDL_Renderer* renderer,SDL_Surface* surface):texture_(SDL_CreateTextureFromSurface(renderer,surface)){}
    /**
     * @brief 一步式：直接从图片文件创建纹理。
     * @param[in] renderer 目标渲染器（借用）。
     * @param[in] path     图片路径，相对当前工作目录。
     * @note 省掉显式 Surface 中间步骤，失败时同样返回 nullptr。
     */
    explicit Texture(SDL_Renderer* renderer,std::string_view path):texture_(IMG_LoadTexture(renderer,std::string(path).c_str())){}
    /**
     * @brief 销毁纹理（释放显存）。
     */
    ~Texture(){if(texture_)SDL_DestroyTexture(texture_);};
    /**
     * @brief 禁止拷贝构造。
     */
    Texture(const Texture&) = delete;
    /**
     * @brief 禁止拷贝赋值。
     */
    Texture& operator=(const Texture&) = delete;
    /**
     * @brief 借用底层句柄。
     * @return SDL_Texture 指针；所有权仍归本类，调用方不得销毁。
     */
    SDL_Texture* Get() const {return texture_;}
    /**
     * @brief 查询纹理是否创建成功。
     * @return 成功返回 true。
     */
    bool IsValid() const {return texture_!=nullptr;}

    void Draw(SDL_Renderer* renderer,const SDL_FRect* src,const SDL_FRect& dst) const {
        if(texture_) SDL_RenderTexture(renderer, texture_, src, &dst);
    }
private:
    SDL_Texture* texture_=nullptr; ///< 显存纹理句柄；nullptr 表示创建失败

};
