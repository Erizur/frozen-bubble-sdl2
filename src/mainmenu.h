#ifndef MAINMENU_H
#define MAINMENU_H

#include <SDL2/SDL.h>
#include "menubutton.h"
#include "shaderstuff.h"

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

#define BLINK_FRAMES 5
#define BLINK_SLOWDOWN 30

class MainMenu final
{
public:
    MainMenu(const SDL_Renderer *renderer);
    MainMenu(const MainMenu&) = delete;
    ~MainMenu();
    void Render(void);
    void press();
    void up();
    void down();
private:
    const SDL_Renderer *renderer;
    std::vector<MenuButton> buttons;
    SDL_Texture *background;

    //candy
    SDL_Texture *fbLogo;
    TextureEx candyOrig, candyModif, logoMask;
    int candyIndex = 0;
    bool candyInit = false;
    void InitCandy();
    
    //banner
    SDL_Texture *bannerArtwork, *bannerCPU, *bannerLevel, *bannerSound;
    SDL_Texture *blinkGreenL, *blinkGreenR, *blinkPurpleL, *blinkPurpleR;
    int bannerFU = BANNER_SLOWDOWN;
    int bannerFormulas[4];
    int bannerMax, bannerCurpos;

    //blink
    int blinkGreen, blinkPurple, waitGreen = 0, waitPurple = 0;
    int blinkUpdate = BLINK_FRAMES;
    bool canUpdateBlink;

    //rest
    SDL_Rect fb_logo_rect, banner_rect, blink_green_left, blink_green_right, blink_purple_left, blink_purple_right;
    uint8_t active_button_index;

    void BlinkRender();
    void BannerRender();
    void CandyRender();
};

#endif // MAINMENU_H
