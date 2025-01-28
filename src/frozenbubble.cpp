#include "frozenbubble.h"

FrozenBubble *FrozenBubble::ptrInstance = NULL;

const char *formatTime(int time){
    int h = int(time/3600.0);
    int m = int((time-h*3600)/60.0);
    int s = int((time-h*3600)-(m*60));

    char *fm = new char[128];
    if (h > 0) sprintf(fm, "%dh ", h);
    if (m > 0) {
        if (h > 0) sprintf(fm + strlen(fm), "%02dm ", m);
        else sprintf(fm, "%dm ", m);
    }
    if (s > 0) {
        if (m > 0) sprintf(fm + strlen(fm), "%02ds", s);
        else sprintf(fm, "%ds", s); 
    }
    return fm;
}

FrozenBubble *FrozenBubble::Instance()
{
    if(ptrInstance == NULL)
        ptrInstance = new FrozenBubble();
    return ptrInstance;
}

FrozenBubble::FrozenBubble() {
    gameOptions = GameSettings::Instance();
    gameOptions->ReadSettings();

    SDL_Point resolution = gameOptions->curResolution();
    Uint32 fullscreen = gameOptions->fullscreenMode() == true ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;

    window = SDL_CreateWindow("Frozen-Bubble", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, resolution.x, resolution.y, fullscreen);
    if(gameOptions->linearScaling) SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

    if(!window) {
        IsGameQuit = true;
        std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
    }

    SDL_Surface *icon = SDL_LoadBMP(DATA_DIR "/gfx/pinguins/window_icon_penguin.bmp");
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, 640, 480);

    if(!renderer) {
        IsGameQuit = true;
        std::cout << "Failed to create renderer: " << SDL_GetError() << std::endl;
    }

    audMixer = AudioMixer::Instance();

    init_effects((char*)DATA_DIR);
    mainMenu = new MainMenu(renderer);
    mainGame = new BubbleGame(renderer);
}

FrozenBubble::~FrozenBubble() {
    if(renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if(window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    audMixer->Dispose();
    gameOptions->Dispose();
}

uint8_t FrozenBubble::RunForEver()
{
    // on init, try playing one of these songs depending on the current state:
    if(currentState == TitleScreen) audMixer->PlayMusic("intro");
    //else if (currentState == MainGame) mainGame->NewGame({false, 1, false});

    float framerate = 60;
    float frametime = 1/framerate * 1000;

    unsigned int ticks, lasttick = 0;
    float elapsed = 0;

    while(!IsGameQuit) {
        lasttick = ticks;
        ticks = SDL_GetTicks();
        elapsed = ticks - lasttick;

        // handle input
        SDL_Event e;
        while (SDL_PollEvent (&e)) {
            HandleInput(&e);
        }

        // render
        if(!IsGamePause) {
            SDL_RenderClear(renderer);
            if (currentState == TitleScreen) mainMenu->Render();
            else if (currentState == MainGame) mainGame->Render();
            SDL_RenderPresent(renderer);
        }
        else {
            if (currentState == MainGame){
                SDL_RenderClear(renderer);
                mainGame->RenderPaused();
                SDL_RenderPresent(renderer);
            }
        }
        if(elapsed < frametime) {
            SDL_Delay(frametime - elapsed);
        }
    }
    if (startTime != 0) addictedTime += SDL_GetTicks() - startTime;
    if(addictedTime != 0) printf("Addicted for %s, %d bubbles were launched.", formatTime(addictedTime/1000), totalBubbles);
    //SDL_Quit(); causes a segfault, i don't know if this is intended
    return 0;
}

void FrozenBubble::HandleInput(SDL_Event *e) {
    switch(e->type) {
        case SDL_WINDOWEVENT:
            switch (e->window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                {
                    IsGameQuit = true;
                    break;
                }
            }
            break;
        case SDL_KEYDOWN:
            if(e->key.repeat) break;
            switch(e->key.keysym.sym) {
                case SDLK_F12:
                    gameOptions->SetValue("GFX:Fullscreen", "");
                    SDL_SetWindowFullscreen(window, gameOptions->fullscreenMode() == true ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                    break;
                case SDLK_PAUSE:
                    CallGamePause();
                    if (currentState == MainGame) {
                        if (!mainGame->playedPause) mainGame->playedPause = false;
                    }
                    break;
            }
            break;
    }

    if (IsGamePause) return;
    if(currentState == TitleScreen) mainMenu->HandleInput(e);
    if(currentState == MainGame) mainGame->HandleInput(e);
}
