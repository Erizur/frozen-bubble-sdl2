#include "frozenbubble.h"
#include "bubblegame.h"
#include "audiomixer.h"

BubbleGame::BubbleGame(const SDL_Renderer *renderer) 
    : renderer(renderer)
{
    // we mostly don't do anything here. Everything should be setup in NewGame()
    char path[256];
    for (int i = 0; i < BUBBLE_STYLES; i++)
    {
        sprintf(path, DATA_DIR "/gfx/balls/bubble-%d.gif", i);
        imgBubbles[i] = IMG_LoadTexture(const_cast<SDL_Renderer*>(renderer), path);
        sprintf(path, DATA_DIR "/gfx/balls/bubble-colourblind-%d.gif", i);
        imgColorblindBubbles[i] = IMG_LoadTexture(const_cast<SDL_Renderer*>(renderer), path);
        sprintf(path, DATA_DIR "/gfx/balls/bubble-%d-mini.png", i);
        imgMiniBubbles[i] = IMG_LoadTexture(const_cast<SDL_Renderer*>(renderer), path);
        sprintf(path, DATA_DIR "/gfx/balls/bubble-colourblind-%d-mini.png", i);
        imgMiniColorblindBubbles[i] = IMG_LoadTexture(const_cast<SDL_Renderer*>(renderer), path);
    }
}

BubbleGame::~BubbleGame() {
    SDL_DestroyTexture(background);
}

void BubbleGame::NewGame(SetupSettings setup) {
    AudioMixer *audMixer = AudioMixer::Instance();
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);

    if (setup.playerCount == 1){
        background = IMG_LoadTexture(rend, DATA_DIR "/gfx/back_one_player.png");
        penguinSprites[0].LoadPenguin(rend, "p1");
        audMixer->PlayMusic("main1p");
    }

    FrozenBubble::Instance()->startTime = SDL_GetTicks();
    penguinSprites[0].PlayAnimation(8);
}

void BubbleGame::Render() {
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);

    SDL_RenderCopy(rend, background, nullptr, nullptr);
    SDL_RenderCopy(rend, penguinSprites[0].CurrentFrame(), nullptr, new SDL_Rect{40, 40, 80, 60});
}