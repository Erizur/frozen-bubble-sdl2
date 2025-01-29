#ifndef PANGOTEXT_H
#define PANGOTEXT_H

#include <SDL2/SDL.h>
#include <SDL2_pango.h>
#include <iostream>
#include <string>

#include "gamesettings.h"

#define WINDOW_W 640
#define WINDOW_H 480
#define SURF_FORMAT SDL_PIXELFORMAT_ARGB8888

class PangoText final
{
public:
    void AddText(SDL_Renderer *rend, const char *txt);
    void RemoveText(SDL_Renderer *rend, const char *txt);
    void Dispose();
private:
    PangoText();
    ~PangoText();

    SDL_Surface *sfc = nullptr;
    SDL_Texture *outTexture = nullptr;
};

#endif //TRANSITIONMANAGER_H