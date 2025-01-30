#include "ttftext.h"

TTFText::TTFText(){

}

TTFText::~TTFText(){
    TTF_CloseFont(textFont);
    SDL_DestroyTexture(outTexture);
}

void TTFText::LoadFont(const char *path, int size) {
    textFont = TTF_OpenFont(path, size);
}

void TTFText::UpdateText(const SDL_Renderer *rend, const char *txt, int wrapLength) {
    if (outTexture != nullptr) SDL_DestroyTexture(outTexture);
    SDL_Surface *front = TTF_RenderUTF8_Blended_Wrapped(textFont, txt, forecolor, wrapLength);
    SDL_Surface *back = TTF_RenderUTF8_Blended_Wrapped(textFont, txt, backcolor, wrapLength);
    SDL_Rect end = {-1, -1, front->w, front->h};
    SDL_BlitSurface(front, nullptr, back, &end);
    outTexture = SDL_CreateTextureFromSurface(const_cast<SDL_Renderer *>(rend), back);
    coords.w = back->w;
    coords.h = back->h;
    SDL_FreeSurface(front);
    SDL_FreeSurface(back);
}

void TTFText::UpdateAlignment(int align) {
    TTF_SetFontWrappedAlign(textFont, align);
}

void TTFText::UpdateColor(SDL_Color fg, SDL_Color bg) {
    forecolor = fg;
    backcolor = bg;
}

void TTFText::UpdateStyle(int size, int style) {
    TTF_SetFontSize(textFont, size);
    TTF_SetFontStyle(textFont, style);
}

void TTFText::UpdatePosition(SDL_Point xy) {
    coords.x = xy.x;
    coords.y = xy.y;
}