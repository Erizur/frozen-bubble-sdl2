#ifndef MAINMENU_H
#define MAINMENU_H

#include <SDL2/SDL.h>
#include "menubutton.h"
#include <vector>
#include <tuple>

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
    SDL_Texture *bannerArtwork, *bannerCPU, *bannerLevel, *bannerSound;
    int banner_max;
    SDL_Rect fb_logo_rect, banner_rect;
    uint8_t active_button_index;
};

#endif // MAINMENU_H
