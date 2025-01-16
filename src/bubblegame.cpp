#include "frozenbubble.h"
#include "bubblegame.h"
#include "audiomixer.h"

#include <algorithm>
#include <iterator>

inline int ranrange(int a, int b) { return a + rand() % ((b - a ) + 1); }

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

    onTopTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/on_top_next.png");
    miniOnTopTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/on_top_next-mini.png");
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

    std::array<std::vector<Bubble>, 13> *bubbleMap = &bubbleArrays[0].bubbleMap;

    for (size_t i = 0; i < level.size(); i++)
    {
        int smallerSep = level[i].size() % 2 == 0 ? 0 : bubbleSize / 2;
        for (size_t j = 0; j < level[i].size(); j++)
        {
            (*bubbleMap)[i].push_back(Bubble{level[i][j], {smallerSep + bubbleSize * ((int)j), initBubbleY * ((int)i)}});
        }
    }
    if(bubbleMap->size() % 2 == 0) {
        for (int i = 0; i < 3; i++)
        {
            int smallerSep = i % 2 == 0 ? 0 : bubbleSize / 2;
            for (int j = 0; j < (i % 2 == 0 ? 7 : 8); j++) (*bubbleMap)[10 + i].push_back({-1, {smallerSep + bubbleSize * j, initBubbleY * (10 + i)}});
        }
    } else {
        for (int i = 1; i < 4; i++)
        {
            int smallerSep = i % 2 == 0 ? 0 : bubbleSize / 2;
            for (int j = 0; j < (i % 2 == 0 ? 8 : 7); j++) (*bubbleMap)[10 + (i - 1)].push_back({-1, {smallerSep + bubbleSize * j, initBubbleY * (10 + i)}});
        }
    }
}

void BubbleGame::NewGame(SetupSettings setup) {
    audMixer = AudioMixer::Instance();
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);
    currentSettings = setup;

    //lowGfx = GameSettings::Instance()->gfxLevel() > 2 ? true : false;
    lowGfx = true;

    if (currentSettings.playerCount == 1){
        background = IMG_LoadTexture(rend, DATA_DIR "/gfx/back_one_player.png");
        bubbleArrays[0].penguinSprite.LoadPenguin(rend, (char*)"p1");
        bubbleArrays[0].shooterSprite = {lowGfx ? lowShooterTexture : shooterTexture, rend};
        bubbleArrays[0].bubbleOffset = {190, 51};
        audMixer->PlayMusic("main1p");
    }

    LoadLevelset(DATA_DIR "/data/levels");
    LoadLevel(1);

    FrozenBubble::Instance()->startTime = SDL_GetTicks();
    FrozenBubble::Instance()->currentState = MainGame;

    ChooseFirstBubble(bubbleArrays[0]);
}

void BubbleGame::UpdatePenguin(BubbleArray &bArray) {
    float &angle = bArray.shooterSprite.angle;
    Penguin &penguin = bArray.penguinSprite;

    bArray.shooterLeft = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT];
    bArray.shooterRight = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT];
    bArray.shooterCenter = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_DOWN];

    if(bArray.shooterAction == true) {
        if(penguin.curAnimation != 1) penguin.PlayAnimation(1);
        bArray.shooterAction = false;
        return;
    }

    if (bArray.shooterLeft || bArray.shooterRight || bArray.shooterCenter) {
        if (angle < 0.1) angle = 0.1;
        if (angle > M_PI-0.1) angle = M_PI-0.1;

        if (bArray.shooterLeft) {
            angle += lowGfx ? LAUNCHER_SPEED : -LAUNCHER_SPEED;
            if(penguin.curAnimation != 1 && (penguin.curAnimation > 7 || penguin.curAnimation < 2)) penguin.PlayAnimation(2);
        }
        else if (bArray.shooterRight) {
            angle -= lowGfx ? LAUNCHER_SPEED : -LAUNCHER_SPEED;
            if(penguin.curAnimation != 1 && (penguin.curAnimation > 7 || penguin.curAnimation < 2)) penguin.PlayAnimation(5);
        }
        else if (bArray.shooterCenter) {
            if (angle >= M_PI/2 - LAUNCHER_SPEED && angle <= M_PI/2 + LAUNCHER_SPEED) angle = M_PI/2;
            else angle += (angle < M_PI/2) ? LAUNCHER_SPEED : -LAUNCHER_SPEED;
        }

        penguin.sleeping = 0;
    }
    else if(!bArray.shooterLeft && !bArray.shooterRight || !bArray.shooterCenter) {
        penguin.sleeping++;
        if (penguin.sleeping > TIMEOUT_PENGUIN_SLEEP && (penguin.curAnimation > 9 || penguin.curAnimation < 8)) penguin.PlayAnimation(8);
    }

    if (!bArray.shooterLeft && penguin.curAnimation == 3) penguin.PlayAnimation(4);
    if (!bArray.shooterRight && penguin.curAnimation == 6) penguin.PlayAnimation(7);
}

// only called for a new game.
void BubbleGame::ChooseFirstBubble(BubbleArray &bArray) {
    std::vector<int> currentBubbles = bArray.remainingBubbles();
    bArray.curLaunch = currentBubbles[ranrange(1, currentBubbles.size()) - 1];
    bArray.nextBubble = currentBubbles[ranrange(1, currentBubbles.size()) - 1];
}

void BubbleGame::PickNextBubble(BubbleArray &bArray) {
    bArray.curLaunch = bArray.nextBubble;
    std::vector<int> currentBubbles = bArray.remainingBubbles();
    bArray.nextBubble = currentBubbles[ranrange(1, currentBubbles.size()) - 1];
}

void BubbleGame::Render() {
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);
    SDL_RenderCopy(rend, background, nullptr, nullptr);
    
    if(currentSettings.playerCount == 1) {
        BubbleArray &curArray = bubbleArrays[0];

        SDL_RenderCopy(rend, imgBubbles[curArray.curLaunch], nullptr, new SDL_Rect{640/2 - 16, 480 - 89, 32, 32});
        SDL_RenderCopy(rend, imgBubbles[curArray.nextBubble], nullptr, new SDL_Rect{640/2 - 16, 480 - 40, 32, 32});
        SDL_RenderCopy(rend, onTopTexture, nullptr, new SDL_Rect{640/2 - 19, 480 - 43, 39, 39});

        UpdatePenguin(curArray);
        if(!lowGfx) {
            curArray.penguinSprite.Render(new SDL_Rect{640/2 + 84, 480 - 60, 80, 60});
            curArray.shooterSprite.Render(new SDL_Rect{640/2 - 50, 480 - 123, 100, 100});
        } else {
            SDL_RenderCopy(rend, lowShooterTexture, nullptr, new SDL_Rect{(int)((640/2) + (LAUNCHER_DIAMETER * SDL_cos(curArray.shooterSprite.angle))), (int)((480 - 69) - (LAUNCHER_DIAMETER * SDL_sin(curArray.shooterSprite.angle))), 4, 4});
        }

        for (const std::vector<Bubble> &vecBubble : curArray.bubbleMap)
        {
            for (Bubble bubble : vecBubble) bubble.Render(rend, imgBubbles, &curArray.bubbleOffset);
        }
        
        SDL_RenderCopy(rend, compressorTexture, nullptr, new SDL_Rect{(640/2) - 128, -5, 252, 56});
    }
    else { //iterate until all penguins are rendered
        for (int i = 0; i < currentSettings.playerCount; i++) {
            BubbleArray &curArray = bubbleArrays[i];

            UpdatePenguin(curArray);
            if(!lowGfx) {
                curArray.penguinSprite.Render(new SDL_Rect{640/2 + 84, 480 - 60, 80, 60});
                curArray.shooterSprite.Render(new SDL_Rect{640/2 - 50, 480 - 123, 100, 100});
            } else {
                SDL_RenderCopy(rend, lowShooterTexture, nullptr, new SDL_Rect{(int)((640/2) + (LAUNCHER_DIAMETER * SDL_cos(curArray.shooterSprite.angle))), (int)((480 - 69) - (LAUNCHER_DIAMETER * SDL_sin(curArray.shooterSprite.angle))), 4, 4});
            }
        }
    }
}

void BubbleGame::HandleInput(SDL_Event *e) {
    switch(e->type) {
        case SDL_KEYDOWN:
            if(e->key.repeat) break;
            switch(e->key.keysym.sym) {
                case SDLK_UP:
                    bubbleArrays[0].shooterAction = true;
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