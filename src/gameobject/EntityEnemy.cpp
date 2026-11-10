#include"EntityEnemy.hpp"

void EntityEnemy::Update(double delta){

}
void EntityEnemy::Draw(SDL_Renderer* renderer,float interpolation){
    SDL_FRect nointerp = GetCollisionRect();

    SDL_SetRenderDrawColor(renderer,127, 0, 9, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer,&nointerp);
}
