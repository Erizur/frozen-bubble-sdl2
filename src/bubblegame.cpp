#include "frozenbubble.h"
#include "bubblegame.h"
#include "audiomixer.h"

BubbleGame::BubbleGame(const SDL_Renderer *renderer) 
    : renderer(renderer)
{
    // We mostly don't do anything here. Everything should be setup in NewGame() instead.
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);

    char path[256];
    for (int i = 0; i < BUBBLE_STYLES; i++)
    {
        sprintf(path, DATA_DIR "/gfx/balls/bubble-%d.gif", i);
        imgBubbles[i] = IMG_LoadTexture(rend, path);
        sprintf(path, DATA_DIR "/gfx/balls/bubble-colourblind-%d.gif", i);
        imgColorblindBubbles[i] = IMG_LoadTexture(rend, path);
        sprintf(path, DATA_DIR "/gfx/balls/bubble-%d-mini.png", i);
        imgMiniBubbles[i] = IMG_LoadTexture(rend, path);
        sprintf(path, DATA_DIR "/gfx/balls/bubble-colourblind-%d-mini.png", i);
        imgMiniColorblindBubbles[i] = IMG_LoadTexture(rend, path);
    }

    shooterTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/shooter.png");
    miniShooterTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/shooter-mini.png");
    lowShooterTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/shooter-lowgfx.png");
}

BubbleGame::~BubbleGame() {
    SDL_DestroyTexture(background);
    for (int i = 0; i < BUBBLE_STYLES; i++)  {
        SDL_DestroyTexture(imgBubbles[i]);
        SDL_DestroyTexture(imgColorblindBubbles[i]);
        SDL_DestroyTexture(imgMiniBubbles[i]);
        SDL_DestroyTexture(imgMiniColorblindBubbles[i]);
    }
}

void BubbleGame::NewGame(SetupSettings setup) {
    audMixer = AudioMixer::Instance();
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);
    currentSettings = setup;

    lowGfx = GameSettings::Instance()->gfxLevel() > 2 ? true : false;

    if (currentSettings.playerCount == 1){
        background = IMG_LoadTexture(rend, DATA_DIR "/gfx/back_one_player.png");
        penguinSprites[0].LoadPenguin(rend, (char*)"p1");
        shooterSprites[0] = {lowGfx ? lowShooterTexture : shooterTexture, rend};
        audMixer->PlayMusic("main1p");
    }

    FrozenBubble::Instance()->startTime = SDL_GetTicks();
}

void BubbleGame::UpdatePenguin(int id) {
    float &angle = shooterSprites[id].angle;

    shooterLeft = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT];
    shooterRight = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT];
    shooterCenter = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_DOWN];

    if(shooterAction == true) {
        if(penguinSprites[id].curAnimation != 1) penguinSprites[id].PlayAnimation(1);
        shooterAction = false;
        return;
    }

    if (shooterLeft || shooterRight || shooterCenter) {
        if (angle < 0.1) angle = 0.1;
        if (angle > M_PI-0.1) angle = M_PI-0.1;

        if (shooterLeft) {
            angle -= LAUNCHER_SPEED;
            if(penguinSprites[id].curAnimation != 1 && (penguinSprites[id].curAnimation > 7 || penguinSprites[id].curAnimation < 2)) penguinSprites[id].PlayAnimation(2);
        }
        else if (shooterRight) {
            angle += LAUNCHER_SPEED;
            if(penguinSprites[id].curAnimation != 1 && (penguinSprites[id].curAnimation > 7 || penguinSprites[id].curAnimation < 2)) penguinSprites[id].PlayAnimation(5);
        }
        else if (shooterCenter) {
            if (angle >= M_PI/2 - LAUNCHER_SPEED && angle <= M_PI/2 + LAUNCHER_SPEED) angle = M_PI/2;
            else angle += (angle < M_PI/2) ? LAUNCHER_SPEED : -LAUNCHER_SPEED;
        }
    }

    if (!shooterLeft && penguinSprites[id].curAnimation == 3) penguinSprites[id].PlayAnimation(4);
    if (!shooterRight && penguinSprites[id].curAnimation == 6) penguinSprites[id].PlayAnimation(7);
}

void BubbleGame::Render() {
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);
    SDL_RenderCopy(rend, background, nullptr, nullptr);

    if(currentSettings.playerCount == 1) {
        UpdatePenguin(0);
        penguinSprites[0].Render(new SDL_Rect{640/2 + 84, 480 - 60, 80, 60});
        shooterSprites[0].Render(new SDL_Rect{640/2 - 50, 480 - 123, 100, 100});
    }
    else { //iterate until all penguins are rendered
        for (int i = 0; i < currentSettings.playerCount; i++) {
            UpdatePenguin(i);
            penguinSprites[i].Render(new SDL_Rect{640/2 + 84, 480 - 60, 80, 60});
            shooterSprites[i].Render(new SDL_Rect{640/2 - 50, 480 - 123, 100, 100});
        }
    }
}

void BubbleGame::HandleInput(SDL_Event *e) {
    switch(e->type) {
        case SDL_KEYDOWN:
            if(e->key.repeat) break;
            switch(e->key.keysym.sym) {
                case SDLK_UP:
                    shooterAction = true;
                    break;
                case SDLK_PAUSE:
                    while(1) {
                        if (SDL_PollEvent(e)) {
                            if(e->type == SDL_KEYDOWN) break;
                            else if (e->type == SDL_QUIT) {
                                FrozenBubble::Instance()->CallGameQuit();
                                break;
                            }
                        }
                    }
                    break;
                case SDLK_ESCAPE:
                    FrozenBubble::Instance()->CallGameQuit();
                    break;
                case SDLK_F11: // mute / unpause audio
                    if(audMixer->IsHalted() == true) {
                        audMixer->MuteAll(true);
                        audMixer->PlayMusic("main1p");
                    }
                    else audMixer->MuteAll();
                    break;
            }
            break;
    }
}