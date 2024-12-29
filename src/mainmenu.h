#ifndef MAINMENU_H
#define MAINMENU_H

#include <SDL2/SDL.h>
#include "menubutton.h"
#include <vector>
#include <tuple>

#pragma region "banner_defines"
#define BANNER_START 1000
#define BANNER_SPACING 80
#define BANNER_MINX 304
#define BANNER_MAXX 596
#define BANNER_Y 243
#define BANNER_SLOWDOWN 1
#pragma endregion

class MainMenu final
{
public:
    MainMenu(const SDL_Renderer *renderer);
    MainMenu(const MainMenu&) = delete;
    ~MainMenu();
    void BannerRender();
    void Render(void);
    void press();
    void up();
    void down();
private:
    const SDL_Renderer *renderer;
    std::vector<MenuButton> buttons;
    SDL_Texture *background;
    SDL_Texture *fb_logo;
    //banner
    SDL_Texture *bannerArtwork, *bannerCPU, *bannerLevel, *bannerSound;
    SDL_Texture *bannerMix;
    int bannerFU = BANNER_SLOWDOWN;
    int bannerFormulas[4];
    int bannerMax;
    int bannerCurpos;
    //rest
    SDL_Rect fb_logo_rect, banner_rect;
    uint8_t active_button_index;
};

#endif // MAINMENU_H
