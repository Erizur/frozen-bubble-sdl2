#include "frozenbubble.h"
#include "bubblegame.h"
#include "audiomixer.h"

#include <algorithm>
#include <iterator>

BubbleGame::BubbleGame(const SDL_Renderer *renderer) 
    : renderer(renderer)
{
    // We mostly just load images here. Everything else should be setup in NewGame() instead.
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);

    char path[256];
    for (int i = 1; i <= BUBBLE_STYLES; i++)
    {
        sprintf(path, DATA_DIR "/gfx/balls/bubble-%d.gif", i);
        imgBubbles[i - 1] = IMG_LoadTexture(rend, path);
        sprintf(path, DATA_DIR "/gfx/balls/bubble-colourblind-%d.gif", i);
        imgColorblindBubbles[i - 1] = IMG_LoadTexture(rend, path);
        sprintf(path, DATA_DIR "/gfx/balls/bubble-%d-mini.png", i);
        imgMiniBubbles[i - 1] = IMG_LoadTexture(rend, path);
        sprintf(path, DATA_DIR "/gfx/balls/bubble-colourblind-%d-mini.png", i);
        imgMiniColorblindBubbles[i - 1] = IMG_LoadTexture(rend, path);
    }

    for (int i = 0; i <= BUBBLE_STICKFC; i++) {
        sprintf(path, DATA_DIR "/gfx/balls/stick_effect_%d.png", i);
        imgBubbleStick[i] = IMG_LoadTexture(rend, path);
        sprintf(path, DATA_DIR "/gfx/balls/stick_effect_%d-mini.png", i);
        imgMiniBubbleStick[i] = IMG_LoadTexture(rend, path);
    }

    imgBubbleFrozen = IMG_LoadTexture(rend, DATA_DIR "/gfx/balls/bubble_lose.png");
    imgMiniBubbleFrozen = IMG_LoadTexture(rend, DATA_DIR "/gfx/balls/bubble_lose-mini.png");

    shooterTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/shooter.png");
    miniShooterTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/shooter-mini.png");
    lowShooterTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/shooter-lowgfx.png");
    
    compressorTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/compressor_main.png");
    sepCompressorTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/compressor_ext.png");
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

void BubbleGame::LoadLevelset(const char *path) {
    std::ifstream lvlSet(path);
    std::string curLine;

    loadedLevels.clear();
    if(lvlSet.is_open())
    {
        int idx = 0;
        std::string curChar;
        std::array<std::vector<int>, 10> level;
        std::vector<int> line;
        while(std::getline(lvlSet, curLine))
        {
            if (curLine.empty())
            {
                idx = 0;
                loadedLevels.push_back(level);
            }
            else {
                std::stringstream ss(curLine);
                while(std::getline(ss, curChar, ' '))
                {
                    if(curChar.empty()) continue;
                    else if(curChar == "-") line.push_back(-1);
                    else {
                        line.push_back(stoi(curChar));
                    }
                }

                level[idx] = line;
                line.clear();
                idx++;
            }
        }
    }
    else {
        SDL_LogError(1, "No such levelset (%s).", path);
    }
}

// singleplayer function, you generate random arrays for multiplayer
void BubbleGame::LoadLevel(int id){
    std::array<std::vector<int>, 10> level = loadedLevels[id - 1];
    int bubbleSize = 32;
    int initBubbleY = (int)(bubbleSize / 1.15);

    for (size_t i = 0; i < level.size(); i++)
    {
        int smallerSep = level[i].size() % 2 == 0 ? 0 : bubbleSize / 2;
        for (size_t j = 0; j < level[i].size(); j++)
        {
            bubbleArrays[0][i].push_back(Bubble{level[i][j], {smallerSep + bubbleSize * ((int)j), initBubbleY * ((int)i)}});
        }
    }
    if(bubbleArrays[0][9].size() % 2 == 0) {
        for (int i = 0; i < 3; i++)
        {
            int smallerSep = i % 2 == 0 ? 0 : bubbleSize / 2;
            for (int j = 0; j < (i % 2 == 0 ? 7 : 8); j++) bubbleArrays[0][10 + i].push_back({-1, {smallerSep + bubbleSize * j, initBubbleY * (10 + i)}});
        }
    } else {
        for (int i = 1; i < 4; i++)
        {
            int smallerSep = i % 2 == 0 ? 0 : bubbleSize / 2;
            for (int j = 0; j < (i % 2 == 0 ? 8 : 7); j++) bubbleArrays[0][10 + (i - 1)].push_back({-1, {smallerSep + bubbleSize * j, initBubbleY * (10 + i)}});
        }
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
        bubbleOffsets[0] = {190, 51};
        audMixer->PlayMusic("main1p");
    }

    LoadLevelset(DATA_DIR "/data/levels");
    LoadLevel(1);

    FrozenBubble::Instance()->startTime = SDL_GetTicks();
    FrozenBubble::Instance()->currentState = MainGame;
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
            angle += lowGfx ? LAUNCHER_SPEED : -LAUNCHER_SPEED;
            if(penguinSprites[id].curAnimation != 1 && (penguinSprites[id].curAnimation > 7 || penguinSprites[id].curAnimation < 2)) penguinSprites[id].PlayAnimation(2);
        }
        else if (shooterRight) {
            angle -= lowGfx ? LAUNCHER_SPEED : -LAUNCHER_SPEED;
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

void BubbleGame::ChooseFirstBubble(int id) {
    
}

void BubbleGame::PickNextBubble(int id) {

}

void BubbleGame::Render() {
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);
    SDL_RenderCopy(rend, background, nullptr, nullptr);
    
    if(currentSettings.playerCount == 1) {
        UpdatePenguin(0);
        if(!lowGfx) {
            penguinSprites[0].Render(new SDL_Rect{640/2 + 84, 480 - 60, 80, 60});
            shooterSprites[0].Render(new SDL_Rect{640/2 - 50, 480 - 123, 100, 100});
        } else {
            SDL_RenderCopy(rend, lowShooterTexture, nullptr, new SDL_Rect{(int)((640/2) + (LAUNCHER_DIAMETER * SDL_cos(shooterSprites[0].angle))), (int)((480 - 69) - (LAUNCHER_DIAMETER * SDL_sin(shooterSprites[0].angle))), 4, 4});
        }
        
        SDL_RenderCopy(rend, compressorTexture, nullptr, new SDL_Rect{(640/2) - 128, -5, 252, 56});
    }
    else { //iterate until all penguins are rendered
        for (int i = 0; i < currentSettings.playerCount; i++) {
            UpdatePenguin(i);
            if(!lowGfx) {
                penguinSprites[0].Render(new SDL_Rect{640/2 + 84, 480 - 60, 80, 60});
                shooterSprites[0].Render(new SDL_Rect{640/2 - 50, 480 - 123, 100, 100});
            } else {
                SDL_RenderCopy(rend, lowShooterTexture, nullptr, new SDL_Rect{(int)((640/2) + (LAUNCHER_DIAMETER * SDL_cos(shooterSprites[i].angle))), (int)((480 - 69) - (LAUNCHER_DIAMETER * SDL_sin(shooterSprites[i].angle))), 4, 4});
            }
        }
    }

    for (const std::vector<Bubble> &vecBubble : bubbleArrays[0])
    {
        for (Bubble bubble : vecBubble) bubble.Render(rend, imgBubbles, &bubbleOffsets[0]);
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