#include "frozenbubble.h"
#include "gamesettings.h"
#include <iostream>
#include "mainmenu.h"

FrozenBubble::FrozenBubble() : IsGameQuit(false)
{
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
}

uint8_t FrozenBubble::RunForEver()
{
    window = SDL_CreateWindow("Frozen-Bubble", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);

    if(!window) {
        IsGameQuit = true;
        std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
    }

    std::string icon_path = std::string(DATA_DIR) + "/gfx/pinguins/window_icon_penguin.bmp";
    SDL_Surface *icon = SDL_LoadBMP(icon_path.c_str());
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if(!renderer) {
        IsGameQuit = true;
        std::cout << "Failed to create renderer: " << SDL_GetError() << std::endl;
    }

    GameSettings* gameOptions = GameSettings::instance();
    gameOptions->ReadSettings();

    MainMenu main_menu(renderer);

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
            switch(e.type) {
                case SDL_WINDOWEVENT:
                    switch (e.window.event) {
                        case SDL_WINDOWEVENT_CLOSE:
                        {
                            IsGameQuit = true;
                            break;
                        }
                    }
                    break;
                case SDL_KEYDOWN:
                    if(e.key.repeat) break;
                    switch(e.key.keysym.sym) {
                        case SDLK_UP:
                        case SDLK_LEFT:
                            main_menu.up();
                            break;
                        case SDLK_DOWN:
                        case SDLK_RIGHT:
                            main_menu.down();
                            break;
                    }
                    break;
            }
        }
        // do magic
        // render
        SDL_RenderClear(renderer);
        main_menu.Render();
        SDL_RenderPresent(renderer);
        if(elapsed < frametime) {
            SDL_Delay(frametime - elapsed);
        }
    }
    //SDL_Quit(); causes a segfault, i don't know if this is intended
    return 0;
}
