#include "mainmenu.h"
#include "audiomixer.h"

#include <SDL2/SDL_image.h>

inline int ranrange(int a, int b) { return a + std::rand() % (b + 1 - a); };

struct ButtonId {
    std::string buttonName;
    std::string iconName;
    int iconFrames;
};

SDL_Point GetSize(SDL_Texture *texture){
    SDL_Point size;
    SDL_QueryTexture(texture, NULL, NULL, &size.x, &size.y);
    return size;
}

MainMenu::MainMenu(const SDL_Renderer *renderer)
    : renderer(renderer), active_button_index(0)
{
    const ButtonId texts[] = {
        {"1pgame", "1pgame", 30}, 
        {"2pgame", "p1p2", 30}, 
        {"langame", "langame", 70}, 
        {"netgame", "netgame", 89}, 
        {"editor", "editor", 67}, 
        {"graphics", "graphics", 30}, 
        {"keys", "keys", 80}, 
        {"highscores", "highscore", 89}
    };
    uint32_t y_start = 14;
    for(const ButtonId button : texts) { // TODO: get rid of compiler warning
        buttons.push_back(MenuButton(89, y_start, button.buttonName, renderer, button.iconName, button.iconFrames));
        y_start += 56;
    }

    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);

    background = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/back_start.png");
    fbLogo = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/fblogo.png");
    fb_logo_rect.x = 400;
    fb_logo_rect.y = 15;
    fb_logo_rect.w = 190;
    fb_logo_rect.h = 119;

    bannerArtwork = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/banner_artwork.png");
    bannerCPU = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/banner_cpucontrol.png");
    bannerSound = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/banner_soundtrack.png");
    bannerLevel = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/banner_leveleditor.png");

    bannerFormulas[0] = BANNER_START;
    bannerFormulas[1] = BANNER_START + GetSize(bannerArtwork).x + BANNER_SPACING;
    bannerFormulas[2] = BANNER_START + GetSize(bannerArtwork).x + BANNER_SPACING
                        + GetSize(bannerSound).x + BANNER_SPACING;
    bannerFormulas[3] = BANNER_START + GetSize(bannerArtwork).x + BANNER_SPACING
                        + GetSize(bannerSound).x + BANNER_SPACING
                        + GetSize(bannerCPU).x + BANNER_SPACING;

    bannerMax = bannerFormulas[3] - (640 - (BANNER_MAXX - BANNER_MINX)) + BANNER_SPACING;
    banner_rect = {BANNER_MINX, BANNER_Y, (BANNER_MAXX - BANNER_MINX), 30};

    blinkGreenL = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/backgrnd-closedeye-left-green.png");
    blinkGreenR = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/backgrnd-closedeye-right-green.png");
    blink_green_left = {411, 385, GetSize(blinkGreenL).x, GetSize(blinkGreenL).y};
    blink_green_right = {434, 378, GetSize(blinkGreenR).x, GetSize(blinkGreenR).y};

    blinkPurpleL = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/backgrnd-closedeye-left-purple.png");
    blinkPurpleR = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/backgrnd-closedeye-right-purple.png");
    blink_purple_left = {522, 356, GetSize(blinkPurpleL).x, GetSize(blinkPurpleL).y};
    blink_purple_right = {535, 356, GetSize(blinkPurpleR).x, GetSize(blinkPurpleR).y};

    InitCandy();

    buttons[active_button_index].Activate();
    AudioMixer::instance()->PlayMusic("intro");
}

MainMenu::~MainMenu() {
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(fbLogo);
    buttons.clear();
}

void MainMenu::InitCandy() {
    candyOrig.LoadTextureData(const_cast<SDL_Renderer*>(renderer), DATA_DIR "/gfx/menu/fblogo.png");
    candyModif.LoadTextureData(const_cast<SDL_Renderer*>(renderer), DATA_DIR "/gfx/menu/fblogo.png");

    candyInit = true;
}

void MainMenu::Render(void) {
    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), background, nullptr, nullptr);
    
    for (MenuButton &button : buttons) {
        button.Render(renderer);
    }
    BannerRender();
    BlinkRender();
    CandyRender();
}

void MainMenu::BannerRender() {
    bannerCurpos = bannerCurpos != 0 ? bannerCurpos : 670;
    for(int i = 0; i < 4; i++) {
        int posX = bannerFormulas[i] - bannerCurpos;
        SDL_Texture *image = i == 0 ? bannerArtwork : (i == 1 ? bannerSound : (i == 2 ? bannerCPU : bannerLevel));
        SDL_Point size = GetSize(image);
        if (posX > bannerMax / 2) posX = bannerFormulas[i] - (bannerCurpos + bannerMax);

        if (posX < BANNER_MAXX && posX + size.x >= 0) {
            SDL_Rect iRect = {-posX, 0, std::min(size.x + posX, BANNER_MAXX - BANNER_MINX), size.y};
            SDL_Rect dRect = {iRect.x < 0 ? BANNER_MAXX - (-posX > -iRect.w ? -posX + iRect.w : 0): BANNER_MINX, BANNER_Y, 
                              iRect.x < 0 ? iRect.w - (-posX > -iRect.w ? posX : 0): iRect.w, size.y};
            SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), image, &iRect, &dRect);
        }
    }

    if(GameSettings::instance()->gfxLevel() > 2) return;
    if(bannerFU == 0) {
        bannerCurpos++;
        bannerFU = BANNER_SLOWDOWN;
    }
    else bannerFU--;
    if(bannerCurpos >= bannerMax) bannerCurpos = 1;
}

void MainMenu::BlinkRender() {
    if(GameSettings::instance()->gfxLevel() > 2) return;

    if (!waitGreen) {
        if(blinkGreen > 0) {
            blinkGreen--;
            if(!blinkGreen) {
                waitGreen = BLINK_FRAMES;
                if(ranrange(-1, 3) <= 1) blinkGreen = -(5 * BLINK_SLOWDOWN); 
            }
        }
        else if(blinkGreen < 0) {
            blinkGreen++;
            if(!blinkGreen) {
                waitGreen = BLINK_FRAMES;
                blinkGreen = 3 * BLINK_SLOWDOWN; 
            }
        }
        else {
            if(ranrange(0, 200) <= 1) {
                waitGreen = BLINK_FRAMES;
                blinkGreen = 3 * BLINK_SLOWDOWN;
            }
        }
    }
    else {
        waitGreen--;
        SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), blinkGreenL, NULL, &blink_green_left);
        SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), blinkGreenR, NULL, &blink_green_right);
    }
    
    if(!waitPurple) {
        if(blinkPurple > 0) {
            blinkPurple--;
            if(!blinkPurple) {
                waitPurple = BLINK_FRAMES;
                if(ranrange(-1, 3) <= 1) blinkPurple = -(5 * BLINK_SLOWDOWN); 
            }
        }
        else if(blinkPurple < 0) {
            blinkPurple++;
            if(!blinkPurple) {
                waitPurple = BLINK_FRAMES;
                blinkPurple = 3 * BLINK_SLOWDOWN; 
            }
        }
        else {
            if(ranrange(0, 200) <= 1) {
                waitPurple = BLINK_FRAMES;
                blinkPurple = 3 * BLINK_SLOWDOWN;
            }
        }
    }
    else {
        waitPurple--;
        SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), blinkPurpleL, NULL, &blink_purple_left);
        SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), blinkPurpleR, NULL, &blink_purple_right);
    }

}

void MainMenu::CandyRender() {
    if (!candyInit || GameSettings::instance()->gfxLevel() > 1) {
        SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), fbLogo, nullptr, &fb_logo_rect);
        return;
    }
    tilt_(candyModif.sfc, candyOrig.sfc, candyIndex);
    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), candyModif.OutputTexture(), nullptr, &fb_logo_rect);

    candyIndex++;
}

void MainMenu::press() {
    buttons[active_button_index].Pressed();

    AudioMixer::instance()->PlaySFX("menu_selected");
}

void MainMenu::down()
{
    buttons[active_button_index].Deactivate();
    if(active_button_index == (buttons.size() - 1)) {
        active_button_index = 0;
    } else {
        active_button_index++;
    }

    buttons[active_button_index].Activate();

    AudioMixer::instance()->PlaySFX("menu_change");
}

void MainMenu::up()
{
    buttons[active_button_index].Deactivate();

    if(active_button_index == 0) {
        active_button_index = buttons.size() - 1;
    } else {
        active_button_index--;
    }

    buttons[active_button_index].Activate();

    AudioMixer::instance()->PlaySFX("menu_change");
}
