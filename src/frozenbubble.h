#ifndef FROZENBUBBLE_H
#define FROZENBUBBLE_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <string>
#include <iostream>

#include "gamesettings.h"
#include "audiomixer.h"

#include "mainmenu.h"

enum GameState {
    TitleScreen = 0,
    BubbleGame = 1,
    LevelEditor = 2,
    Netplay = 3
};

class FrozenBubble
{
public:

    uint8_t RunForEver();
    GameState currentState = TitleScreen;

    FrozenBubble(const FrozenBubble& obj) = delete;
    static FrozenBubble* Instance();
private:
    bool IsGameQuit;
    SDL_Window *window;
    SDL_Renderer *renderer;

    GameSettings *gameOptions;
    AudioMixer *audMixer;

    MainMenu *mainMenu;

    void HandleInput(SDL_Event *e);

    static FrozenBubble* ptrInstance;
    FrozenBubble();
    ~FrozenBubble();
};

#endif // FROZENBUBBLE_H
