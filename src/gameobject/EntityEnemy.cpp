/**
 * @file    EntityEnemy.cpp
 * @brief   EntityEnemy 的实现：占位色块绘制。
 */

#include"EntityEnemy.hpp"

void EntityEnemy::Update(double delta){

}
void EntityEnemy::Draw(SDL_Renderer* renderer,float interpolation){
    SDL_FRect nointerp = GetCollisionRect();

    // 敌人位置是"瞬移"而非连续运动，所以直接画逻辑态，不做插值
    SDL_SetRenderDrawColor(renderer,127, 0, 9, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer,&nointerp);
}
