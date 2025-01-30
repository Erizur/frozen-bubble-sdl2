#ifndef PANGOTEXT_H
#define PANGOTEXT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>

#include "gamesettings.h"

#define WINDOW_W 640
#define WINDOW_H 480
#define SURF_FORMAT SDL_PIXELFORMAT_ARGB8888

class TTFText final
{
public:
    void LoadFont(const char *path, int size);

    void AddText(SDL_Renderer *rend, const char *txt);
    void RemoveText(SDL_Renderer *rend, const char *txt);
    void Dispose();
private:
    TTFText();
    ~TTFText();

    TTF_Font *textFont = nullptr;
    SDL_Surface *sfc = nullptr;
    SDL_Texture *outTexture = nullptr;
};

#endif //TRANSITIONMANAGER_H