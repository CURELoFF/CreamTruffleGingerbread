/**
 * @file    Collision.hpp
 * @brief   轴对齐包围盒（AABB）相交判定，header-only。
 */

#pragma once
#include<SDL3/SDL.h>

/**
 * @brief 判断两个 AABB 是否真正重叠。
 * @param[in] a 矩形 a。
 * @param[in] b 矩形 b。
 * @return 两矩形有面积重叠时返回 true。
 * @note 四个比较都用严格不等号，因此仅边缘贴合（如 A 右边界 == B 左边界）不算碰撞。
 */
inline bool Intersects(const SDL_FRect& a,const SDL_FRect& b) {
    return a.x < b.x + b.w
        && a.x + a.w > b.x
        && a.y < b.y + b.h
        && a.y + a.h > b.y;
}
