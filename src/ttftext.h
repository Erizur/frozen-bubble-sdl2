#ifndef TTFTEXT_H
#define TTFTEXT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>

#define WINDOW_W 640
#define WINDOW_H 480
#define SURF_FORMAT SDL_PIXELFORMAT_ARGB8888

class TTFText final
{
public:
    void LoadFont(const char *path, int size);

    void UpdateText(const SDL_Renderer *rend, const char *txt, int wrapLength);
    void UpdateAlignment(int align);
    void UpdateColor(SDL_Color fg, SDL_Color bg);
    void UpdateStyle(int size, int style);
    void UpdatePosition(SDL_Point xy);

    SDL_Rect *Coords() { return &coords; };
    SDL_Texture *Texture() { return outTexture; };
    
    TTFText();
    ~TTFText();
private:
    SDL_Rect coords;
    SDL_Color forecolor, backcolor;

    TTF_Font *textFont = nullptr;
    SDL_Texture *outTexture = nullptr;
};

#endif //TTFTEXT_H