#include "mainmenu.h"
#include "audiomixer.h"
#include "frozenbubble.h"
#include "transitionmanager.h"

#include <SDL2/SDL_image.h>

inline int ranrange(int a, int b) { return a + rand() % ((b - a ) + 1); }

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
    for(size_t i = 0; i < std::size(texts); i++) { // TODO: get rid of compiler warning
        buttons.push_back(MenuButton(89, y_start, texts[i].buttonName, renderer, texts[i].iconName, texts[i].iconFrames));
        y_start += 56;
    }

    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);

    background = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/back_start.png");
    fbLogo = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/fblogo.png");
    fb_logo_rect.x = 400;
    fb_logo_rect.y = 15;
    fb_logo_rect.w = 190;
    fb_logo_rect.h = 119;
    candy_fb_rect = SDL_Rect(fb_logo_rect);

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

    for (int i = 0; i < SP_OPT; i++) {
        std::string idlePath = std::string(DATA_DIR) + "/gfx/menu/txt_" + spOptions[i].option + "_outlined_text.png";
        std::string activePath = std::string(DATA_DIR) + "/gfx/menu/txt_" + spOptions[i].option + "_text.png";
        idleSPButtons[i] = IMG_LoadTexture(rend, idlePath.c_str());
        activeSPButtons[i] = IMG_Load(activePath.c_str());
    }
    singlePanelBG = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/1p_panel.png");
    singleButtonAct = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/txt_menu_1p_over.png");
    singleButtonIdle = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/txt_menu_1p_off.png");

    voidPanelBG = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/void_panel.png");

    InitCandy();

    buttons[active_button_index].Activate();

    panelText.LoadFont(DATA_DIR "/gfx/DroidSans.ttf", 15);
    panelText.UpdateAlignment(TTF_WRAPPED_ALIGN_CENTER);
    panelText.UpdateColor({255, 255, 255, 255}, {0, 0, 0, 255});
}

MainMenu::~MainMenu() {
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(fbLogo);
    buttons.clear();
}

void MainMenu::InitCandy() {
    candyOrig.LoadTextureData(const_cast<SDL_Renderer*>(renderer), DATA_DIR "/gfx/menu/fblogo.png");
    if(candyMethod == -1) candyMethod = ranrange(0, 8);
    else {
        int a = ranrange(0, 8);
        while (a == candyMethod) a = ranrange(0, 8);
        candyMethod = a;
    }

    SDL_Rect tmpRct;
    if (candyMethod == 3) { // stretch
        candy_fb_rect.x -= (int)(fb_logo_rect.w * 0.05);
        candy_fb_rect.y -= (int)(fb_logo_rect.h * 0.05);
        tmpRct = {(int)(fb_logo_rect.w * 0.05), (int)(fb_logo_rect.h * 0.05), (int)(fb_logo_rect.w * 1.1), (int)(fb_logo_rect.h * 1.1)};
        candyModif.LoadEmptyAndApply(&tmpRct, const_cast<SDL_Renderer*>(renderer), DATA_DIR "/gfx/menu/fblogo.png");
        SDL_FreeSurface(candyOrig.sfc);
        candyOrig.LoadFromSurface(candyModif.sfc, const_cast<SDL_Renderer*>(renderer));
        candy_fb_rect.w = candyOrig.sfc->w;
        candy_fb_rect.h = candyOrig.sfc->h;
    }
    else if (candyMethod == 4) { // tilt
        candy_fb_rect.x -= (int)(fb_logo_rect.w * 0.05);
        candy_fb_rect.y -= (int)(fb_logo_rect.h * 0.025);
        tmpRct = {(int)(fb_logo_rect.w * 0.05), (int)(fb_logo_rect.h * 0.025), (int)(fb_logo_rect.w * 1.1), (int)(fb_logo_rect.h * 1.05)};
        candyModif.LoadEmptyAndApply(&tmpRct, const_cast<SDL_Renderer*>(renderer), DATA_DIR "/gfx/menu/fblogo.png");
        SDL_FreeSurface(candyOrig.sfc);
        candyOrig.LoadFromSurface(candyModif.sfc, const_cast<SDL_Renderer*>(renderer));
        candy_fb_rect.w = candyOrig.sfc->w;
        candy_fb_rect.h = candyOrig.sfc->h;
    }
    else if (candyMethod == 5) {
        candyModif.LoadTextureData(const_cast<SDL_Renderer*>(renderer), DATA_DIR "/gfx/menu/fblogo.png");
        logoMask.LoadTextureData(const_cast<SDL_Renderer*>(renderer), DATA_DIR "/gfx/menu/fblogo-mask.png"); // points
    }
    else if (candyMethod == 8) { //snow
        candy_fb_rect.x -= (int)(fb_logo_rect.w * 0.05);
        tmpRct = {(int)(fb_logo_rect.w * 0.05), candy_fb_rect.y, (int)(fb_logo_rect.w * 1.1), fb_logo_rect.h + candy_fb_rect.y};
        candyModif.LoadEmptyAndApply(&tmpRct, const_cast<SDL_Renderer*>(renderer), DATA_DIR "/gfx/menu/fblogo.png");
        SDL_FreeSurface(candyOrig.sfc);
        candyOrig.LoadFromSurface(candyModif.sfc, const_cast<SDL_Renderer*>(renderer));
        candy_fb_rect.y = 0;
        candy_fb_rect.w = candyOrig.sfc->w;
        candy_fb_rect.h = candyOrig.sfc->h;
    }
    else candyModif.LoadTextureData(const_cast<SDL_Renderer*>(renderer), DATA_DIR "/gfx/menu/fblogo.png");

    candyInit = true;
}

void MainMenu::RefreshCandy(){
    candy_fb_rect = SDL_Rect(fb_logo_rect);
    InitCandy();
}

void MainMenu::HandleInput(SDL_Event *e){
    switch(e->type) {
        case SDL_KEYDOWN:
            if(e->key.repeat) break;
            if (awaitKp && (showingOptPanel || showing2PPanel) && e->key.keysym.sym != SDLK_ESCAPE) {
                AudioMixer::Instance()->PlaySFX("typewriter");
                lastOptInput = e->key.keysym.sym;
                awaitKp = false;
                break;
            }
            switch(e->key.keysym.sym) {
                case SDLK_UP:
                case SDLK_LEFT:
                    up();
                    break;
                case SDLK_DOWN:
                case SDLK_RIGHT:
                    down();
                    break;
                case SDLK_RETURN:
                    press();
                    break;
                case SDLK_n:
                    if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL] == SDL_PRESSED) RefreshCandy();
                    break;
                case SDLK_ESCAPE:
                    if (showingSPPanel) {
                        AudioMixer::Instance()->PlaySFX("cancel");
                        showingSPPanel = false;
                        break;
                    }
                    if (showingOptPanel || showing2PPanel) {
                        AudioMixer::Instance()->PlaySFX("cancel");
                        showingOptPanel = showing2PPanel = false;
                        awaitKp = false;
                        break;
                    }
                    FrozenBubble::Instance()->CallGameQuit();
                    break;
                case SDLK_F11: // mute / unpause audio
                    if(AudioMixer::Instance()->IsHalted() == true) {
                        AudioMixer::Instance()->MuteAll(true);
                        AudioMixer::Instance()->PlayMusic("intro");
                    }
                    else AudioMixer::Instance()->MuteAll();
                    break;
            }
            break;
    }
}

void MainMenu::Render(void) {
    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), background, nullptr, nullptr);
    
    for (MenuButton &button : buttons) {
        button.Render(renderer);
    }
    BannerRender();
    BlinkRender();
    CandyRender();
    SPPanelRender();
    TPPanelRender();
    OptPanelRender();
}

void MainMenu::BannerRender() {
    bannerCurpos = bannerCurpos != 0 ? bannerCurpos : 670;
    for(size_t i = 0; i < std::size(bannerFormulas); i++) {
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

    if(GameSettings::Instance()->gfxLevel() > 2) return;
    if(bannerFU == 0) {
        bannerCurpos++;
        bannerFU = BANNER_SLOWDOWN;
    }
    else bannerFU--;
    if(bannerCurpos >= bannerMax) bannerCurpos = 1;
}

void MainMenu::BlinkRender() {
    if(GameSettings::Instance()->gfxLevel() > 2) return;

    if (waitGreen <= 0) {
        if(blinkGreen > 0) {
            blinkGreen--;
            if(!blinkGreen) {
                waitGreen = BLINK_FRAMES;
                if(ranrange(0, 3) <= 1) blinkGreen = -(5 * BLINK_SLOWDOWN); 
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
    
    if(waitPurple <= 0) {
        if(blinkPurple > 0) {
            blinkPurple--;
            if(!blinkPurple) {
                waitPurple = BLINK_FRAMES;
                if(ranrange(0, 3) <= 1) blinkPurple = -(5 * BLINK_SLOWDOWN); 
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
    if (!candyInit || GameSettings::Instance()->gfxLevel() > 1) {
        SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), fbLogo, nullptr, &fb_logo_rect);
        return;
    }

    if (candyMethod == 0)       rotate_bilinear_(candyModif.sfc, candyOrig.sfc, SDL_sin(candyIndex/40.0)/10.0);
    else if(candyMethod == 1)   flipflop_(candyModif.sfc, candyOrig.sfc, candyIndex);
    else if(candyMethod == 2)   enlighten_(candyModif.sfc, candyOrig.sfc, candyIndex);
    else if(candyMethod == 3)   stretch_(candyModif.sfc, candyOrig.sfc, candyIndex);
    else if(candyMethod == 4)   tilt_(candyModif.sfc, candyOrig.sfc, candyIndex);
    else if(candyMethod == 5)   points_(candyModif.sfc, candyOrig.sfc, logoMask.sfc);
    else if(candyMethod == 6)   waterize_(candyModif.sfc, candyOrig.sfc, candyIndex);
    else if(candyMethod == 7)   brokentv_(candyModif.sfc, candyOrig.sfc, candyIndex);
    else if(candyMethod == 8)   snow_(candyModif.sfc, candyOrig.sfc);

    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), candyModif.OutputTexture(), nullptr, &candy_fb_rect);
    candyIndex++;
}

void restartOverlook(SDL_Surface *overlookSfc, int &overlookIndex){
    if(GameSettings::Instance()->gfxLevel() > 2) return;
    overlook_init_(overlookSfc);
    overlookIndex = 0;
}

void MainMenu::SPPanelRender() {
    if (!showingSPPanel) return;

    if(overlookSfc == nullptr) {
        overlookSfc = SDL_CreateRGBSurfaceWithFormat(0, activeSPButtons[0]->w, activeSPButtons[0]->h, 32, SURF_FORMAT);
        overlook_init_(overlookSfc);
    }

    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), singlePanelBG, nullptr, &voidPanelRct);
    for (int i = 0; i < SP_OPT; i++){
        int w, h;
        SDL_QueryTexture(idleSPButtons[i], nullptr, nullptr, &w, &h);
        SDL_Rect entryRct = {(640/2)-(298/2), ((480/2)-90)+(41 * (i + 1)), 298, 37};
        SDL_Rect subRct = {(640/2)-(298/2), ((480/2)-90)+(41 * (i + 1)), w, h};
        if(i == activeSPIdx) {
            if (GameSettings::Instance()->gfxLevel() <= 2) {
                overlook_(overlookSfc, activeSPButtons[i], overlookIndex, spOptions[i].pivot);
                SDL_Rect miniRct = {(640/2)-(298/2), ((480/2)-90)+(41 * (i + 1)), overlookSfc->w, overlookSfc->h};
                SDL_Texture *miniOverlook = SDL_CreateTextureFromSurface(const_cast<SDL_Renderer*>(renderer), overlookSfc);
                
                SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), singleButtonAct, nullptr, &entryRct);
                SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), miniOverlook, nullptr, &miniRct);
                SDL_DestroyTexture(miniOverlook);

                overlookIndex++;
                if (overlookIndex >= 70) overlookIndex = 0;
            }
            else SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), singleButtonAct, nullptr, &entryRct);
        }
        else SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), singleButtonIdle, nullptr, &entryRct);
        SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), idleSPButtons[i], nullptr, &subRct);
    }

    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), panelText.Texture(), nullptr, panelText.Coords());
}

void MainMenu::TPPanelRender() {
    if (!showing2PPanel) return;

    if(awaitKp == false && lastOptInput != SDLK_UNKNOWN && !runDelay) { // we got our response
        chainReaction = lastOptInput == SDLK_y ? true : false;

        char pnltxt[256];
        sprintf(pnltxt, "2-player game\n\n\nEnable chain reaction?\n\n\nY or N?:        %s\n\n\n\n\nEnjoy the game!", SDL_GetKeyName(lastOptInput));
        panelText.UpdateText(const_cast<SDL_Renderer *>(renderer), pnltxt, 0);
        panelText.UpdatePosition({(640/2) - (panelText.Coords()->w / 2), (480/2) - 120});

        delayTime = 120;
        runDelay = true;
    }

    if (runDelay){
        if (delayTime == 0) SetupNewGame(selectedMode);
        else delayTime--;
    }

    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), voidPanelBG, nullptr, &voidPanelRct);
    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), panelText.Texture(), nullptr, panelText.Coords());
}

void MainMenu::OptPanelRender() {
    if (!showingOptPanel) return;

    if(awaitKp == false && lastOptInput != SDLK_UNKNOWN && !runDelay) { // we got our response
        chainReaction = lastOptInput == SDLK_y ? true : false;

        char pnltxt[256];
        sprintf(pnltxt, "Random level\n\n\nEnable chain reaction?\n\n\nY or N?:        %s\n\n\n\n\nEnjoy the game!", SDL_GetKeyName(lastOptInput));
        panelText.UpdateText(const_cast<SDL_Renderer *>(renderer), pnltxt, 0);
        panelText.UpdatePosition({(640/2) - (panelText.Coords()->w / 2), (480/2) - 120});

        delayTime = 120;
        runDelay = true;
    }

    if (runDelay){
        if (delayTime == 0) SetupNewGame(selectedMode);
        else delayTime--;
    }

    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), voidPanelBG, nullptr, &voidPanelRct);
    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), panelText.Texture(), nullptr, panelText.Coords());
}

void MainMenu::press() {
    if (showingOptPanel || showing2PPanel) return;
    AudioMixer::Instance()->PlaySFX("menu_selected");

    if (showingSPPanel) {
        if (activeSPIdx == 0) SetupNewGame(1);
        if (activeSPIdx == 2) ShowPanel(1);
        return;
    }

    buttons[active_button_index].Pressed(this);
}

void MainMenu::down()
{
    if (showingOptPanel || showing2PPanel) return;
    AudioMixer::Instance()->PlaySFX("menu_change");

    if (showingSPPanel) {
        if (activeSPIdx == SP_OPT - 1) activeSPIdx = 0;
        else activeSPIdx++;
        restartOverlook(overlookSfc, overlookIndex);
        return;
    }

    buttons[active_button_index].Deactivate();
    if(active_button_index == (buttons.size() - 1)) {
        active_button_index = 0;
    } else {
        active_button_index++;
    }

    buttons[active_button_index].Activate();
}

void MainMenu::up()
{
    if (showingOptPanel || showing2PPanel) return;
    AudioMixer::Instance()->PlaySFX("menu_change");

    if (showingSPPanel) {
        if (activeSPIdx == 0) activeSPIdx = SP_OPT - 1;
        else activeSPIdx--;
        restartOverlook(overlookSfc, overlookIndex);
        return;
    }

    buttons[active_button_index].Deactivate();

    if(active_button_index == 0) {
        active_button_index = buttons.size() - 1;
    } else {
        active_button_index--;
    }

    buttons[active_button_index].Activate();
}

void MainMenu::ShowPanel(int which) {
    switch (which){
        case 0: // singleplayer menu
            showingSPPanel = true;
            panelText.UpdateText(const_cast<SDL_Renderer *>(renderer), "Start 1-player game menu", 0);
            panelText.UpdatePosition({(640/2) - (panelText.Coords()->w / 2), (480/2) - 120});
            break;
        case 1: // random level
            showingSPPanel = false;
            showingOptPanel = awaitKp = true;
            panelText.UpdateText(const_cast<SDL_Renderer *>(renderer), "Random level\n\n\nEnable chain reaction?\n\n\nY or N?:          \n", 0);
            panelText.UpdatePosition({(640/2) - (panelText.Coords()->w / 2), (480/2) - 120});
            selectedMode = 3;
            break;
        case 2: // 2p menu
            showing2PPanel = awaitKp = true;
            panelText.UpdateText(const_cast<SDL_Renderer *>(renderer), "2-player game\n\n\nEnable chain reaction?\n\n\nY or N?:          \n", 0);
            panelText.UpdatePosition({(640/2) - (panelText.Coords()->w / 2), (480/2) - 120});
            selectedMode = 2;
            break;
        case 6:
            HighscoreManager::Instance()->ShowScoreScreen(0);
            break;
        default:
            break;
    }
}

void MainMenu::SetupNewGame(int mode) {
    TransitionManager::Instance()->DoSnipIn(const_cast<SDL_Renderer*>(renderer));
    switch(mode){
        case 1:
            FrozenBubble::Instance()->bubbleGame()->NewGame({chainReaction, 1, false});
            break;
        case 2:
            FrozenBubble::Instance()->bubbleGame()->NewGame({chainReaction, 2, false, true});
            break;
        case 3:
            FrozenBubble::Instance()->bubbleGame()->NewGame({chainReaction, 1, false, true});
            break;
        default:
            break;
    }
}

void MainMenu::ReturnToMenu() {
    AudioMixer::Instance()->PlayMusic("intro");
    FrozenBubble::Instance()->currentState = TitleScreen;
    candyIndex = 0;
    bannerCurpos = 0;
    showingSPPanel = false;
    showing2PPanel = false;
    showingOptPanel = false;
    awaitKp = false;
    selectedMode = 0;
    runDelay = false;
}
