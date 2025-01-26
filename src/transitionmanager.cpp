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
    snapIn = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SDL_PIXELFORMAT_ARGB8888);
    snapOut = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SDL_PIXELFORMAT_ARGB8888);
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
    SDL_RenderReadPixels(rend, NULL, SURF_FORMAT, snapIn->pixels, snapIn->pitch);
}

void TransitionManager::TakeSnipOut(SDL_Renderer *rend) 
{
    SDL_RenderReadPixels(rend, NULL, SURF_FORMAT, snapOut->pixels, snapOut->pitch);
    effect(snapIn, snapOut, rend, transitionTexture);
}

