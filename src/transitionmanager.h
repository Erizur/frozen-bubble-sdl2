#ifndef TRANSITIONMANAGER_H
#define TRANSITIONMANAGER_H

#include <SDL2/SDL.h>
#include <iostream>
#include <string>

#include "gamesettings.h"

#define WINDOW_W 640
#define WINDOW_H 480
#define SURF_FORMAT SDL_PIXELFORMAT_ARGB8888

class TransitionManager final
{
public:
    void DoSnipIn(SDL_Renderer *rend);
    void TakeSnipOut(SDL_Renderer *rend);

    TransitionManager(const TransitionManager& obj) = delete;
    void Dispose();
    static TransitionManager* Instance();
private:
    TransitionManager();
    ~TransitionManager();
    static TransitionManager* ptrInstance;
    GameSettings* gameSettings;

    SDL_Surface *snapIn = nullptr, *snapOut = nullptr;
    SDL_Texture *transitionTexture = nullptr;
    bool stopRendering = true;
};

#endif //TRANSITIONMANAGER_H