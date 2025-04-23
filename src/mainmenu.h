#ifndef MAINMENU_H
#define MAINMENU_H

#include <SDL2/SDL.h>
#include <vector>

#include "menubutton.h"
#include "shaderstuff.h"
#include "ttftext.h"

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

#define SP_OPT 4

class MainMenu final
{
public:
    MainMenu(const SDL_Renderer *renderer);
    MainMenu(const MainMenu&) = delete;
    ~MainMenu();
    void Render(void);
    void RefreshCandy();
    void HandleInput(SDL_Event *e);
    void SetupNewGame(int mode);
    void ShowPanel(int which);
    void ReturnToMenu();
private:
    const SDL_Renderer *renderer;
    std::vector<MenuButton> buttons;
    SDL_Texture *background;

    //candy
    SDL_Rect fb_logo_rect, candy_fb_rect;
    SDL_Texture *fbLogo;
    TextureEx candyOrig, candyModif, logoMask;
    int candyIndex = 0;
    int candyMethod = -1;
    bool candyInit = false;
    void InitCandy();
    
    //banner
    SDL_Rect banner_rect;
    SDL_Texture *bannerArtwork, *bannerCPU, *bannerLevel, *bannerSound;
    SDL_Texture *blinkGreenL, *blinkGreenR, *blinkPurpleL, *blinkPurpleR;
    int bannerFU = BANNER_SLOWDOWN;
    int bannerFormulas[4];
    int bannerMax = 0, bannerCurpos = 0;

    //blink
    SDL_Rect blink_green_left, blink_green_right, blink_purple_left, blink_purple_right;
    int blinkGreen = 0, blinkPurple = 0, waitGreen = 0, waitPurple = 0;
    int blinkUpdate = BLINK_FRAMES;
    bool canUpdateBlink;

    //rest
    uint8_t active_button_index;

    void press();
    void up();
    void down();

    void BlinkRender();
    void BannerRender();
    void CandyRender();

    TTFText panelText;

    //singleplayer panel
    SDL_Texture *singlePanelBG;
    SDL_Texture *singleButtonAct, *singleButtonIdle;
    SDL_Surface *activeSPButtons[SP_OPT];
    SDL_Surface *overlookSfc = nullptr;
    SDL_Texture *idleSPButtons[SP_OPT];
    int activeSPIdx = 0, overlookIndex = 0;
    bool showingSPPanel = false, showing2PPanel = false;
    const struct spPanelEntry {
        std::string option;
        int pivot;
    } spOptions[SP_OPT] = {{"play_all_levels", 90}, {"pick_start_level", 135}, {"play_random_levels", 82}, {"multiplayer_training", 105}};
    
    SDL_Rect voidPanelRct = {(640/2) - (341/2), (480/2) - (280/2), 341, 280};
    void SPPanelRender();
    void TPPanelRender();

    //Options panel render
    bool showingOptPanel = false, awaitKp = false, runDelay = false;
    int delayTime;
    SDL_Keycode lastOptInput = SDLK_UNKNOWN;
    SDL_Texture *voidPanelBG;
    void OptPanelRender();

    //game setup defines
    bool chainReaction = false;
    int selectedMode;
};

#endif // MAINMENU_H
