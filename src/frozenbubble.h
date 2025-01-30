#ifndef FROZENBUBBLE_H
#define FROZENBUBBLE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>
#include <string>
#include <iostream>

#include "gamesettings.h"
#include "audiomixer.h"

#include "mainmenu.h"
#include "bubblegame.h"

#include "ttftext.h"

enum GameState {
    TitleScreen = 0,
    MainGame = 1,
    LevelEditor = 2,
    Netplay = 3
};

class FrozenBubble
{
public:
    int totalBubbles = 0;
    Uint32 startTime = 0;
    void CallGameQuit() { IsGameQuit = true; };
    void CallGamePause() { IsGamePause = !IsGamePause; };

    uint8_t RunForEver();
    GameState currentState = TitleScreen;

    FrozenBubble(const FrozenBubble& obj) = delete;
    static FrozenBubble* Instance();
    BubbleGame* bubbleGame() { return mainGame; };
private:
    int addictedTime = 0;
    bool IsGameQuit, IsGamePause = false;
    SDL_Window *window;
    SDL_Renderer *renderer;

    GameSettings *gameOptions;
    AudioMixer *audMixer;

    MainMenu *mainMenu;
    BubbleGame *mainGame;

    TTFText menuText;

    void HandleInput(SDL_Event *e);

    static FrozenBubble* ptrInstance;
    FrozenBubble();
    ~FrozenBubble();
};

#endif // FROZENBUBBLE_H
