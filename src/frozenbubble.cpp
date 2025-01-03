#include "frozenbubble.h"

FrozenBubble *FrozenBubble::ptrInstance = NULL;

FrozenBubble *FrozenBubble::Instance()
{
    if(ptrInstance == NULL)
        ptrInstance = new FrozenBubble();
    return ptrInstance;
}

FrozenBubble::FrozenBubble() {
    window = SDL_CreateWindow("Frozen-Bubble", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);

    if(!window) {
        IsGameQuit = true;
        std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
    }

    SDL_Surface *icon = SDL_LoadBMP(DATA_DIR "/gfx/pinguins/window_icon_penguin.bmp");
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if(!renderer) {
        IsGameQuit = true;
        std::cout << "Failed to create renderer: " << SDL_GetError() << std::endl;
    }

    gameOptions = GameSettings::Instance();
    gameOptions->ReadSettings();

    audMixer = AudioMixer::Instance();

    init_effects(DATA_DIR);
    mainMenu = new MainMenu(renderer);
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
        // do magic
        // render
        SDL_RenderClear(renderer);
        if (currentState == TitleScreen) mainMenu->Render();
        SDL_RenderPresent(renderer);
        if(elapsed < frametime) {
            SDL_Delay(frametime - elapsed);
        }
    }
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
    }

    if(currentState == TitleScreen) {
        switch(e->type) {
            case SDL_KEYDOWN:
                if(e->key.repeat) break;
                switch(e->key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_LEFT:
                        mainMenu->up();
                        break;
                    case SDLK_DOWN:
                    case SDLK_RIGHT:
                        mainMenu->down();
                        break;
                    case SDLK_RETURN:
                        mainMenu->press();
                        break;
                    case SDLK_n:
                        if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL] == SDL_PRESSED) mainMenu->RefreshCandy();
                        break;
                    case SDLK_PAUSE:
                        while(1) {
                            if (SDL_PollEvent(e)) {
                                if(e->type == SDL_KEYDOWN) break;
                                else if (e->type == SDL_QUIT) {
                                    IsGameQuit = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case SDLK_ESCAPE:
                        IsGameQuit = true;
                        break;
                    case SDLK_F11: // mute / unpause audio
                        if(audMixer->IsHalted() == true) {
                            audMixer->MuteAll(true);
                            audMixer->PlayMusic("intro");
                        }
                        else audMixer->MuteAll();
                        break;
                }
                break;
        }
    }
}
