#include "transitionmanager.h"
#include "shaderstuff.h"

TransitionManager *TransitionManager::ptrInstance = NULL;

TransitionManager *TransitionManager::Instance()
{
    if(ptrInstance == NULL)
        ptrInstance = new TransitionManager();
    return ptrInstance;
}

TransitionManager::TransitionManager()
{
    gameSettings = GameSettings::Instance();
    snapIn = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SURF_FORMAT);
    snapOut = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SURF_FORMAT);
}

TransitionManager::~TransitionManager(){
    SDL_FreeSurface(snapIn);
    SDL_FreeSurface(snapOut);
}

void TransitionManager::Dispose(){
    this->~TransitionManager();
}

void TransitionManager::DoSnipIn(SDL_Renderer *rend) 
{
    float w = 0, h = 0;
    SDL_RenderGetScale(rend, &w, &h);
    SDL_Rect dstSize = {0, 0, 640, 480};
    SDL_Surface *sfc = SDL_CreateRGBSurfaceWithFormat(0, 640 * w, 480 * h, 32, SURF_FORMAT);
    SDL_RenderReadPixels(rend, NULL, SURF_FORMAT, sfc->pixels, sfc->pitch);
    SDL_BlitScaled(sfc, NULL, snapIn, &dstSize);
    SDL_FreeSurface(sfc);
}

void TransitionManager::TakeSnipOut(SDL_Renderer *rend) 
{
    float w = 0, h = 0;
    SDL_RenderGetScale(rend, &w, &h);
    SDL_Rect dstSize = {0, 0, 640, 480};
    SDL_Surface *sfc = SDL_CreateRGBSurfaceWithFormat(0, 640 * w, 480 * h, 32, SURF_FORMAT);
    SDL_RenderReadPixels(rend, NULL, SURF_FORMAT, sfc->pixels, sfc->pitch);
    SDL_BlitScaled(sfc, NULL, snapOut, &dstSize);
    SDL_FreeSurface(sfc);
    effect(snapIn, snapOut, rend, transitionTexture);
}

