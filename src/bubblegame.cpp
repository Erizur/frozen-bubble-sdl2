#include "frozenbubble.h"
#include "bubblegame.h"
#include "audiomixer.h"
#include "transitionmanager.h"

#include <fstream>
#include <sstream>

#include <cmath>
#include <algorithm>

inline int ranrange(int a, int b) { return a + rand() % ((b - a ) + 1); }
inline float ranrange(float b) { return (rand()) / (static_cast <float> (RAND_MAX/b)); }

struct SingleBubble {
    int assignedArray; // assigned board to use
    int bubbleId; // id to use bubble image
    SDL_Point pos; // current position, top left aligned
    SDL_Point oldpos; // old position, useful for collision
    float direction = PI/2.0f; // angle
    bool falling = false; // is falling from the top
    bool launching = false; // is launched from shooter
    int leftLimit, rightLimit, topLimit; // limit before bouncing
    bool lowGfx = false; // running on lowgfx
    int bubbleSize = 32; // bubble size (change when creating for small variants)
    float speedX = 0, speedY = 0, genSpeed = 0; // used for falling bubbles
    bool chainExists = false; // enable chain reaction animation
    SDL_Point chainDest = {}; //where to land when chain reacting
    bool exploding = false; // if bubble is exploding animation
    bool shouldClear = false; // if the bubble should be deleted now
    int waitForFall = 0; // frames to wait before falling
    SDL_Rect rect = {}; // rendering rect

    void CopyBubbleProperties(Bubble *prop) {
        bubbleId = (*prop).bubbleId;
        pos = (*prop).pos;
    }

    void GenerateFreeFall(bool explode = false, int waitTime = 0) {
        speedX = (ranrange(3) - 1.5) / (bubbleSize >= 32 ? 1 : 2);
        speedY = (-ranrange(4) - 2) / (bubbleSize >= 32 ? 1 : 2);
        if (!explode) {
            falling = true;
            waitForFall = waitTime;
        }
        else exploding = true;
    }

    bool IsCollision(Bubble *bubble) {
        if (bubble->bubbleId == -1) return false;
        if (pos.y < topLimit) return true; // end if out of bounds ontop
        double distanceCollision = pow(bubbleSize * 0.82, 2);
        double xs = pow(bubble->pos.x - pos.x, 2);
        if (xs > distanceCollision) return false;
        return (xs + pow(bubble->pos.y - pos.y, 2)) < distanceCollision; 
    }

    void UpdatePosition() {
        if (launching) {
            oldpos = pos;
            if (!lowGfx) pos.x -= ((float)BUBBLE_SPEED) * cosf(direction);
            else pos.x += ((float)BUBBLE_SPEED) * cosf(direction);
            pos.y -= ((float)BUBBLE_SPEED) * sinf(direction);
            if (pos.x < leftLimit) {
                AudioMixer::Instance()->PlaySFX("rebound");
                pos.x = 2.0f * leftLimit - pos.x;
                direction -= 2.0f * (direction-PI/2.0f);
            }
            if (pos.x > rightLimit - bubbleSize) {
                AudioMixer::Instance()->PlaySFX("rebound");
                pos.x = 2.0f * (rightLimit - bubbleSize) - pos.x;
                direction += 2.0f * (PI/2.0f-direction);
            }
        }
        else if (falling) {
            if (waitForFall > 0) {
                waitForFall--;
            }
            else {
                if (!chainExists) {
                    pos.y += genSpeed * 0.5;
                    genSpeed += FREEFALL_CONSTANT * 0.5;
                }
                else {
                    // not implemented
                }
                if(pos.y > 470) shouldClear = true;
            }
        }
        else if (exploding) {
            pos.x += speedX * 0.5;
            pos.y += speedY * 0.5;
            speedY += FREEFALL_CONSTANT * 0.5;
            if(pos.y > 470) shouldClear = true;
        }
    }

    void Render(SDL_Renderer *rend, SDL_Texture *bubbles[]) {
        if (bubbleId == -1) return;
        rect.x = pos.x;
        rect.y = pos.y;
        rect.w = rect.h = bubbleSize;
        SDL_RenderCopy(rend, bubbles[bubbleId], nullptr, &rect);
    };
};

std::vector<SingleBubble> singleBubbles;

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

    for (int i = 0; i < 35; i++) {
        sprintf(path, DATA_DIR "/gfx/pause_%04d.png", i);
        pausePenguin[i] = IMG_LoadTexture(rend, path);
    }

    imgBubbleFrozen = IMG_LoadTexture(rend, DATA_DIR "/gfx/balls/bubble_lose.png");
    imgMiniBubbleFrozen = IMG_LoadTexture(rend, DATA_DIR "/gfx/balls/bubble_lose-mini.png");

    imgBubblePrelight = IMG_LoadTexture(rend, DATA_DIR "/gfx/balls/bubble_prelight.png");
    imgMiniBubblePrelight = IMG_LoadTexture(rend, DATA_DIR "/gfx/balls/bubble_prelight-mini.png");

    shooterTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/shooter.png");
    miniShooterTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/shooter-mini.png");
    lowShooterTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/shooter-lowgfx.png");
    
    compressorTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/compressor_main.png");
    sepCompressorTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/compressor_ext.png");

    onTopTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/on_top_next.png");
    miniOnTopTexture = IMG_LoadTexture(rend, DATA_DIR "/gfx/on_top_next-mini.png");

    dotTexture[0] = IMG_LoadTexture(rend, DATA_DIR "/gfx/dot_green.png");
    dotTexture[1] = IMG_LoadTexture(rend, DATA_DIR "/gfx/dot_red.png");

    soloStatePanels[0] = IMG_LoadTexture(rend, DATA_DIR "/gfx/lose_panel.png");
    soloStatePanels[1] = IMG_LoadTexture(rend, DATA_DIR "/gfx/win_panel_1player.png");

    pauseBackground = IMG_LoadTexture(rend, DATA_DIR "/gfx/back_paused.png");

    inGameText.LoadFont(DATA_DIR "/gfx/DroidSans.ttf", 20);
    inGameText.UpdateAlignment(TTF_WRAPPED_ALIGN_CENTER);
    inGameText.UpdateColor({255, 255, 255, 255}, {0, 0, 0, 255});
}

BubbleGame::~BubbleGame() {
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(pauseBackground);
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

    SDL_Point &offset = bubbleArrays[0].bubbleOffset;
    std::array<std::vector<Bubble>, 13> &bubbleMap = bubbleArrays[0].bubbleMap;

    for (size_t i = 0; i < level.size(); i++)
    {
        int smallerSep = level[i].size() % 2 == 0 ? 0 : bubbleSize / 2;
        for (size_t j = 0; j < level[i].size(); j++)
        {
            bubbleMap[i].push_back(Bubble{level[i][j], {(smallerSep + bubbleSize * ((int)j)) + offset.x, (initBubbleY * ((int)i)) + offset.y}});
        }
    }
    if(bubbleMap[9].size() % 2 == 0) {
        for (int i = 0; i < 3; i++)
        {
            int smallerSep = i % 2 == 0 ? 0 : bubbleSize / 2;
            for (int j = 0; j < (i % 2 == 0 ? 7 : 8); j++) bubbleMap[10 + i].push_back({-1, {(smallerSep + bubbleSize * j) + offset.x, (initBubbleY * (10 + i)) + offset.y}});
        }
    } else {
        for (int i = 1; i < 4; i++)
        {
            int smallerSep = i % 2 == 0 ? bubbleSize / 2 : 0;
            for (int j = 0; j < (i % 2 == 0 ? 7 : 8); j++) bubbleMap[10 + (i - 1)].push_back({-1, {(smallerSep + bubbleSize * j) + offset.x, (initBubbleY * (9 + i)) + offset.y}});
        }
    }

    char lvnm[64];
    sprintf(lvnm, "Level %i", id);
    inGameText.UpdateText(renderer, lvnm, 0);
    inGameText.UpdatePosition({75 - (inGameText.Coords()->w / 2), 105});
}

void BubbleGame::RandomLevel(BubbleArray &bArray){
    int bubbleSize = 32;
    int initBubbleY = (int)(bubbleSize / 1.15);

    SDL_Point &offset = bArray.bubbleOffset;
    std::array<std::vector<Bubble>, 13> &bubbleMap = bArray.bubbleMap;

    int r = ranrange(0,1);
    int untilend = (13 / 2) - 1;
    for (size_t i = 0; i < bArray.bubbleMap.size(); i++)
    {
        int size = (i + r) % 2 == 0 ? 8 : 7;
        int smallerSep = (i + r) % 2 == 0 ? 0 : bubbleSize / 2;
        for (int j = 0; j < size; j++)
        {
            bubbleMap[i].push_back(Bubble{(int)i < untilend ? ranrange(0, 7) : -1, {(smallerSep + bubbleSize * ((int)j)) + offset.x, (initBubbleY * ((int)i)) + offset.y}});
        }
    }

    inGameText.UpdateText(renderer, "Random level", 0);
    inGameText.UpdatePosition({75 - (inGameText.Coords()->w / 2), 105});
}

void SetupGameMetrics(BubbleArray *bArray, int playerCount, bool lowGfx){
    switch (playerCount) {
        case 2:
            if (lowGfx) bArray[0].lGfxShooterRct.w = bArray[0].lGfxShooterRct.h = 2;
            bArray[0].curLaunchRct = {SCREEN_CENTER_X - 16, 480 - 89, 32, 32};
            bArray[0].nextBubbleRct = {SCREEN_CENTER_X - 16, 480 - 40, 32, 32};
            bArray[0].onTopRct = {SCREEN_CENTER_X - 19, 480 - 43, 39, 39};
            bArray[0].frozenBottomRct = {SCREEN_CENTER_X - 18, 480 - 42, 34, 48};
            break;
        case 1:
        default:
            if (lowGfx) bArray[0].lGfxShooterRct.w = bArray[0].lGfxShooterRct.h = 2;
            bArray[0].compressorRct = {SCREEN_CENTER_X - 128, -5 + (28 * bArray[0].numSeparators), 252, 56};
            bArray[0].curLaunchRct = {SCREEN_CENTER_X - 16, 480 - 89, 32, 32};
            bArray[0].nextBubbleRct = {SCREEN_CENTER_X - 16, 480 - 40, 32, 32};
            bArray[0].onTopRct = {SCREEN_CENTER_X - 19, 480 - 43, 39, 39};
            bArray[0].frozenBottomRct = {SCREEN_CENTER_X - 18, 480 - 42, 34, 48};
            break;
    }
}

void BubbleGame::NewGame(SetupSettings setup) {
    audMixer = AudioMixer::Instance();
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);
    currentSettings = setup;

    lowGfx = GameSettings::Instance()->gfxLevel() > 2;
    char path[256];

    if (currentSettings.playerCount == 1){
        background = IMG_LoadTexture(rend, DATA_DIR "/gfx/back_one_player.png");
        bubbleArrays[0].penguinSprite.LoadPenguin(rend, (char*)"p1", {SCREEN_CENTER_X + 84, 480 - 60, 80, 60});
        bubbleArrays[0].shooterSprite = {lowGfx ? lowShooterTexture : shooterTexture, rend};
        bubbleArrays[0].shooterSprite.rect = {SCREEN_CENTER_X - 50, 480 - 123, 100, 100};
        bubbleArrays[0].bubbleOffset = {190, 51};
        bubbleArrays[0].leftLimit = SCREEN_CENTER_X - 128;
        bubbleArrays[0].rightLimit = SCREEN_CENTER_X + 128;
        bubbleArrays[0].topLimit = 51;
        bubbleArrays[0].hurryRct = {SCREEN_CENTER_X - 122, 480 - 214, 244, 102};
        bubbleArrays[0].numSeparators = 0;
        bubbleArrays[0].playerAssigned = 0;
        sprintf(path, DATA_DIR "/gfx/hurry_%s.png", "p1");
        bubbleArrays[0].hurryTexture = IMG_LoadTexture(rend, path);
        audMixer->PlayMusic("main1p");
        SetupGameMetrics(bubbleArrays[0], currentSettings.playerCount, lowGfx);
    }

    if (!currentSettings.randomLevels) {
        LoadLevelset(DATA_DIR "/data/levels");
        LoadLevel(curLevel);
    }
    else {
        for (int i = 0; i < currentSettings.playerCount; i++) RandomLevel(bubbleArrays[i]);
    }

    FrozenBubble::Instance()->startTime = SDL_GetTicks();
    FrozenBubble::Instance()->currentState = MainGame;

    ChooseFirstBubble(bubbleArrays[0]);
}

void RemoveArray(BubbleArray &bArray) {
    for (size_t i = 0; i < bArray.bubbleMap.size(); i++) {
        bArray.bubbleMap[i].clear();
    }
}

void BubbleGame::ReloadGame(int level) {
    if (level >= (int)loadedLevels.size() && !currentSettings.randomLevels){
        QuitToTitle();
        return;
    }

    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);

    TransitionManager::Instance()->DoSnipIn(rend);
    firstRenderDone = false;

    gameFinish = gameWon = gameLost = false;

    switch (currentSettings.playerCount) {
        case 2:

            break;
        case 1:
            RemoveArray(bubbleArrays[0]);
            bubbleArrays[0].penguinSprite.PlayAnimation(0);
            bubbleArrays[0].shooterSprite.angle = PI/2.0f;
            bubbleArrays[0].bubbleOffset = {190, 51};
            bubbleArrays[0].topLimit = 51;
            bubbleArrays[0].numSeparators = 0;
            bubbleArrays[0].turnsToCompress = 9;
            bubbleArrays[0].dangerZone = 12;
            SetupGameMetrics(bubbleArrays[0], currentSettings.playerCount, lowGfx);
            break;
    }

    if (!currentSettings.randomLevels) {
        LoadLevel(level);
        for (int i = 0; i < currentSettings.playerCount; i++) ChooseFirstBubble(bubbleArrays[i]);
    }
    else {
        for (int i = 0; i < currentSettings.playerCount; i++) {
            RandomLevel(bubbleArrays[i]);
            ChooseFirstBubble(bubbleArrays[i]);
        }
    }
    
}

void BubbleGame::LaunchBubble(BubbleArray &bArray) {
    audMixer->PlaySFX("launch");
    singleBubbles.push_back({bArray.playerAssigned, bArray.curLaunch, {640/2 - 19, 480 - 89}, {}, bArray.shooterSprite.angle, false, true, bArray.leftLimit, bArray.rightLimit, bArray.topLimit, lowGfx});
    PickNextBubble(bArray);
    FrozenBubble::Instance()->totalBubbles++;
    bArray.hurryTimer = 0;
}

void BubbleGame::UpdatePenguin(BubbleArray &bArray) {
    if (gameFinish) return;

    if (bArray.hurryTimer >= TIME_HURRY_WARN) {
        if (bArray.warnTimer <= HURRY_WARN_FC / 2){
            if(bArray.warnTimer == 0) audMixer->PlaySFX("hurry");
            SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), bArray.hurryTexture, nullptr, &bArray.hurryRct);
        }
        bArray.warnTimer++;
        if (bArray.warnTimer > HURRY_WARN_FC) {
            bArray.warnTimer = 0;
        }
        if (bArray.hurryTimer >= TIME_HURRY_MAX) {
            bArray.shooterAction = true;
        }
    }
    bArray.hurryTimer++;

    float &angle = bArray.shooterSprite.angle;
    Penguin &penguin = bArray.penguinSprite;

    bArray.shooterLeft = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT];
    bArray.shooterRight = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT];
    bArray.shooterCenter = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_DOWN];

    if(bArray.shooterAction == true && bArray.newShoot == true) {
        penguin.sleeping = 0;
        if(penguin.curAnimation != 1) penguin.PlayAnimation(1);
        LaunchBubble(bArray);
        bArray.shooterAction = false;
        bArray.newShoot = false;
        return;
    }

    if (angle < 0.1) angle = 0.1;
    if (angle > PI-0.1) angle = PI-0.1;

    if (bArray.shooterLeft || bArray.shooterRight || bArray.shooterCenter) {
        if (bArray.shooterLeft) {
            angle += lowGfx ? LAUNCHER_SPEED : -LAUNCHER_SPEED;
            if(penguin.curAnimation != 1 && (penguin.curAnimation > 7 || penguin.curAnimation < 2)) penguin.PlayAnimation(2);
        }
        else if (bArray.shooterRight) {
            angle -= lowGfx ? LAUNCHER_SPEED : -LAUNCHER_SPEED;
            if(penguin.curAnimation != 1 && (penguin.curAnimation > 7 || penguin.curAnimation < 2)) penguin.PlayAnimation(5);
        }
        else if (bArray.shooterCenter) {
            if (angle >= PI/2.0f - LAUNCHER_SPEED && angle <= PI/2.0f + LAUNCHER_SPEED) angle = PI/2.0f;
            else angle += (angle < PI/2.0f) ? LAUNCHER_SPEED : -LAUNCHER_SPEED;
        }

        penguin.sleeping = 0;
    }
    else if(!bArray.shooterLeft && !bArray.shooterRight && !bArray.shooterCenter) {
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

void GetClosestFreeCell(SingleBubble &sBubble, BubbleArray &bArray, int *row, int *col) {
    int xpos = (sBubble.oldpos.x + sBubble.pos.x) / 2;
    int ypos = (sBubble.oldpos.y + sBubble.pos.y) / 2;
    float closestDistance = 9999;
    for (size_t i = 0; i < bArray.bubbleMap.size(); i++)
    {
        for (size_t j = 0; j < bArray.bubbleMap[i].size(); j++)
        {
            Bubble &bubble = bArray.bubbleMap[i][j];
            if (bubble.bubbleId != -1) continue; // skip if already filled
            float distance = sqrt(pow(bubble.pos.x - xpos, 2) + pow(bubble.pos.y - ypos, 2));
            if (distance < closestDistance) {
                closestDistance = distance;
                *row = (int)i;
                *col = (int)j;
            }
        }
    }
}

void BubbleGame::UpdateSingleBubbles(int id) {
    BubbleArray *bArray = &bubbleArrays[id];
    for (size_t i = 0; i < singleBubbles.size(); i++) {
        SingleBubble &sBubble = singleBubbles[i];
        if (sBubble.shouldClear) continue;
        if (sBubble.assignedArray != bArray->playerAssigned) continue; // if not from current array, ignore!
        sBubble.UpdatePosition();
        if (sBubble.launching != true) continue; // if not being launched, ignore collision!
        for (const std::vector<Bubble> &vecBubble : bArray->bubbleMap) {
            for (Bubble bubble : vecBubble) {
                if (sBubble.IsCollision(&bubble)) {
                    int row, col;
                    GetClosestFreeCell(sBubble, *bArray, &row, &col);
                    bArray->PlacePlayerBubble(sBubble.bubbleId, row, col);
                    bArray->newShoot = true;
                    audMixer->PlaySFX("stick");
                    sBubble.shouldClear = true;
                    CheckPossibleDestroy(*bArray);
                    CheckGameState(*bArray);
                    goto STOP_ITER;
                }
            };
        }
        STOP_ITER:
        continue;
    }
    singleBubbles.erase(std::remove_if(singleBubbles.begin(), singleBubbles.end(), [](const SingleBubble &s){ return s.shouldClear; }), singleBubbles.end());
}

bool IsTileNotGrouped(BubbleArray &bArray, std::vector<Bubble*> *bubbleCount, int row, int col) {
    bool validRow = (((size_t)row <= bArray.bubbleMap.size() - 1) && row >= 0) ? true : false;
    if (validRow == false) return false;
    bool validCol = (((size_t)col <= bArray.bubbleMap[row].size() - 1) && col >= 0) ? true : false;
    if (validCol == false) return false;

    if (std::count((*bubbleCount).begin(), (*bubbleCount).end(), &bArray.bubbleMap[row][col]) > 0) return false;
    if (bArray.bubbleMap[row][col].bubbleId != (*bubbleCount)[0]->bubbleId) return false; // if it doesnt match what were looking for, ignore!
    return true;
}

void GetGroupedCount(BubbleArray &bArray, std::vector<Bubble*> *bubbleCount, int row, int col, int *curStack) {
    for (size_t i = 0; i < bArray.bubbleMap.size(); i++) {
        for (size_t j = 0; j < bArray.bubbleMap[i].size(); j++) {
            if (i == (size_t)row && j == (size_t)col) { //we are where we left from.
                for (int k = -1; k < 2; k++) {
                    for (int l = -1; l < 2; l++) {
                        if(k != 0){
                            if((i > 0 && (k > 0 && (size_t)i >= bArray.bubbleMap.size() - 1) == false) || (i == 0 && k > 0)) {
                                if(bArray.bubbleMap[i].size() > bArray.bubbleMap[i + k].size()){ if (l > 0) continue; }
                                else { if (l < 0) continue; }
                            }
                        }
                        if (IsTileNotGrouped(bArray, bubbleCount, i + k, j + l)) {
                            (*bubbleCount).push_back(&bArray.bubbleMap[i][j]);
                            *curStack += 1;
                            GetGroupedCount(bArray, bubbleCount, i + k, j + l, curStack);
                        }
                    }
                }
            }
            else continue;
        }
    }
}

void BubbleGame::CheckPossibleDestroy(BubbleArray &bArray){ 
    for (size_t i = 0; i < bArray.bubbleMap.size(); i++) {
        for (size_t j = 0; j < bArray.bubbleMap[i].size(); j++) {
            if (bArray.bubbleMap[i][j].playerBubble == true) { // activator
                std::vector<Bubble*> bubbleCount;
                int groupedCount = 0;
                bubbleCount.push_back(&bArray.bubbleMap[i][j]);
                GetGroupedCount(bArray, &bubbleCount, i, j, &groupedCount);
                if (groupedCount >= 3) {
                    audMixer->PlaySFX("destroy_group");
                    for (Bubble *bubble : bubbleCount) {
                        if(!lowGfx) {
                            SingleBubble bubs = {bArray.playerAssigned, bArray.curLaunch, bubble->pos, {}, bArray.shooterSprite.angle, false, false, bArray.leftLimit, bArray.rightLimit, bArray.topLimit, lowGfx};
                            bubs.CopyBubbleProperties(bubble);
                            bubs.GenerateFreeFall(true);
                            singleBubbles.push_back(bubs);
                        }
                        bubble->bubbleId = -1;
                        bubble->playerBubble = false;
                    }
                }
                bubbleCount.clear();
                continue;
            }
        }
    }
    CheckAirBubbles(bArray);
}

bool isAttached(BubbleArray &bArray, int row, int col) {
    bool biggerThan = (bArray.bubbleMap[row].size() > bArray.bubbleMap[row - 1].size()) ? true : false;
    if(biggerThan) {
        if (col > 0) { if (bArray.bubbleMap[row-1][col-1].bubbleId != -1) return true; }
        if ((size_t)col < bArray.bubbleMap[row].size() - 1) { if (bArray.bubbleMap[row-1][col].bubbleId != -1) return true; }
    }
    else {
        if (bArray.bubbleMap[row-1][col].bubbleId != -1) return true;
        if (bArray.bubbleMap[row-1][col+1].bubbleId != -1) return true;
    }
    return false;
}

void CheckIfAttached(BubbleArray &bArray, int row, int col, int fc, bool *attached) {
    *attached = isAttached(bArray, row, col);
    if (*attached != true && bArray.bubbleMap[row][col].bubbleId != -1) { //if atp attached is still false, try the others!
        if (col > 0) {
            if (col-1 != fc && bArray.bubbleMap[row][col - 1].bubbleId != -1) CheckIfAttached(bArray, row, col - 1, col, attached);
        }
        if (*attached != true && (size_t)col < bArray.bubbleMap[row].size() - 1) {
            if (col+1 != fc && bArray.bubbleMap[row][col + 1].bubbleId != -1) CheckIfAttached(bArray, row, col + 1, col, attached);
        }
    }
}

void DoFalling(std::vector<SDL_Point> &map, std::vector<SingleBubble> &bubbles, bool &lowGfx) {
    if (map.size() < 1 || bubbles.size() < 1) return;
    int maxy = map[map.size() - 1].y;
    int shiftSameLine = 0, line = maxy;
    for (size_t i = map.size(); i > 0; i--) { //original FB does backwards sorting for the formula
        int y = map[i - 1].y;
        if(!lowGfx) {
            shiftSameLine = line != y ? 0 : shiftSameLine;
            line = y;
            bubbles[i - 1].GenerateFreeFall(false, (maxy - y) * 5 + shiftSameLine);
            singleBubbles.push_back(bubbles[i - 1]);
            shiftSameLine++;
        }
        
    }
    map.clear();
    bubbles.clear();
}

void BubbleGame::CheckAirBubbles(BubbleArray &bArray) {
    std::vector<SDL_Point> fallingLocs;
    std::vector<SingleBubble> singlesFalling; 
    for (size_t i = 0; i < bArray.bubbleMap.size(); i++) {
        for (size_t j = 0; j < bArray.bubbleMap[i].size(); j++) {
            if (bArray.bubbleMap[i][j].bubbleId == -1) continue; //just skip
            if (i > 0) { //not the top row
                bool attached = false;
                CheckIfAttached(bArray, i, j, 99, &attached);
                if (attached == false) {
                    SingleBubble bubbly = {bArray.playerAssigned, bArray.curLaunch, bArray.bubbleMap[i][j].pos, {}, bArray.shooterSprite.angle, false, false, bArray.leftLimit, bArray.rightLimit, bArray.topLimit, lowGfx};
                    bubbly.CopyBubbleProperties(&bArray.bubbleMap[i][j]);
                    singlesFalling.push_back(bubbly);
                    fallingLocs.push_back({(int)j, (int)i});
                    bArray.bubbleMap[i][j].bubbleId = -1;
                    bArray.bubbleMap[i][j].playerBubble = false;
                    continue;
                }
            }
            else continue;
        }
    }
    DoFalling(fallingLocs, singlesFalling, lowGfx);
}

void BubbleGame::DoFrozenAnimation(BubbleArray &bArray, int &waitTime){
    if (waitTime <= 0) {
        waitTime = FROZEN_FRAMEWAIT;
        for (int i = (int)bArray.bubbleMap.size() - 1; i >= 0; i--) {
            for (int j = (int)bArray.bubbleMap[i].size() - 1; j >= 0; j--) {
                if (bArray.bubbleMap[i][j].frozen != true && bArray.bubbleMap[i][j].bubbleId != -1) {
                    bArray.bubbleMap[i][j].frozen = true;
                    return;
                }
                else continue;
            }
        }
        audMixer->PlaySFX("noh");
        gameLost = true;
    }
    else waitTime--;
}

void BubbleGame::DoPrelightAnimation(BubbleArray &bArray, int &waitTime){
    if (waitTime <= 0) {
        if(bArray.framePrelight <= 0) {
            for (size_t i = 0; i < bArray.bubbleMap.size(); i++) {
                if (lowGfx) continue;
                for (size_t j = 0; j < bArray.bubbleMap[i].size(); j++) {
                    if (j == (size_t)alertColumn) {
                        bArray.bubbleMap[i][j].shining = true;
                    }
                    else {
                        bArray.bubbleMap[i][j].shining = false;
                        continue;
                    }
                }
            }
            alertColumn++;
            if (alertColumn > 8) {
                waitTime = bArray.waitPrelight;
                alertColumn = 0;
            }
            bArray.framePrelight = PRELIGHT_FRAMEWAIT;
        }
        else bArray.framePrelight--;
    }
    else waitTime--;
}

void ResetPrelight(BubbleArray &bArray) {
    for (size_t i = 0; i < bArray.bubbleMap.size(); i++) {
        for (size_t j = 0; j < bArray.bubbleMap[i].size(); j++) {
            bArray.bubbleMap[i][j].shining = false;
        }
    }
}

void BubbleGame::CheckGameState(BubbleArray &bArray) {
    bArray.turnsToCompress--;
    if (bArray.turnsToCompress == 1) bArray.waitPrelight = PRELIGHT_FAST;
    if (bArray.turnsToCompress == 0) {
        ResetPrelight(bArray);
        bArray.waitPrelight = PRELIGHT_SLOW;
        bArray.turnsToCompress = 9;
        bArray.dangerZone--;
        bArray.numSeparators++;
        bArray.ExpandOffset(0, 28);
        bArray.compressorRct.y += 28;
        audMixer->PlaySFX("newroot_solo");
    }
    if (bArray.allClear()) {
        gameFinish = true;
        gameWon = true;
        panelRct = {SCREEN_CENTER_X - 173, 480 - 289, 329, 159};
        bArray.penguinSprite.PlayAnimation(10);
    }
    if (bArray.bubbleOnDanger()) {
        gameFinish = true;
        audMixer->PlaySFX("lose");
        panelRct = {SCREEN_CENTER_X - 173, 480 - 248, 345, 124};
        bArray.curLaunchRct = {bArray.curLaunchRct.x - 1, bArray.curLaunchRct.y - 1, 34, 48};
        bArray.penguinSprite.PlayAnimation(11);
    }
}

void BubbleGame::Render() {
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);
    SDL_RenderCopy(rend, background, nullptr, nullptr);

    if(playedPause) {
        audMixer->PauseMusic(true);
        playedPause = false;
        FrozenBubble::Instance()->startTime += SDL_GetTicks() - timePaused;
    }
    
    if(currentSettings.playerCount == 1) {
        BubbleArray &curArray = bubbleArrays[0];

        SDL_Rect rct;
        for (int i = 1; i < 10; i++) {
            rct.x = curArray.rightLimit;
            rct.y = 104 - (7 * i) - i;
            rct.w = rct.h = 7;
            SDL_RenderCopy(rend, dotTexture[i == curArray.turnsToCompress ? 1 : 0], nullptr, &rct);
        }
        for (int i = 0; i < curArray.numSeparators; i++) {
            rct.x = SCREEN_CENTER_X - 95;
            rct.y = (28 * i);
            rct.w = 188;
            rct.h = 28;
            SDL_RenderCopy(rend, sepCompressorTexture, nullptr, &rct);
        }
        SDL_RenderCopy(rend, compressorTexture, nullptr, &curArray.compressorRct);

        if (curArray.turnsToCompress <= 2) {
            DoPrelightAnimation(curArray, curArray.prelightTime);
        }
        for (const std::vector<Bubble> &vecBubble : curArray.bubbleMap) for (Bubble bubble : vecBubble) bubble.Render(rend, imgBubbles, imgBubblePrelight, imgBubbleFrozen);

        if(gameFinish) {
            if (!gameWon && !gameLost) DoFrozenAnimation(curArray, curArray.frozenWait);

            if (gameLost) SDL_RenderCopy(rend, soloStatePanels[0], nullptr, &panelRct);
            else if (gameWon) SDL_RenderCopy(rend, soloStatePanels[1], nullptr, &panelRct);
        }

        if(singleBubbles.size() > 0) {
            UpdateSingleBubbles(0);
            for (SingleBubble &bubble : singleBubbles) bubble.Render(rend, imgBubbles);
        }

        SDL_RenderCopy(rend, gameFinish && !gameWon ? imgBubbleFrozen : imgBubbles[curArray.curLaunch], nullptr, &curArray.curLaunchRct);
        SDL_RenderCopy(rend, imgBubbles[curArray.nextBubble], nullptr, &curArray.nextBubbleRct);
        SDL_RenderCopy(rend, onTopTexture, nullptr, &curArray.onTopRct);
        if (gameFinish && !gameWon) SDL_RenderCopy(rend, imgBubbleFrozen, nullptr, &curArray.frozenBottomRct);

        UpdatePenguin(curArray);
        if(!lowGfx) {
            curArray.penguinSprite.Render();
            curArray.shooterSprite.Render();
        } else {
            curArray.lGfxShooterRct.x = (int)(SCREEN_CENTER_X + (LAUNCHER_DIAMETER * cosf(curArray.shooterSprite.angle)));
            curArray.lGfxShooterRct.y = (int)((480 - 69) - (LAUNCHER_DIAMETER * sinf(curArray.shooterSprite.angle)));
            SDL_RenderCopy(rend, lowShooterTexture, nullptr, &curArray.lGfxShooterRct);
        }
    }
    else { //iterate until all penguins are rendered
        for (int i = 0; i < currentSettings.playerCount; i++) {
            BubbleArray &curArray = bubbleArrays[i];

            UpdateSingleBubbles(i);
            for (const std::vector<Bubble> &vecBubble : curArray.bubbleMap) for (Bubble bubble : vecBubble) bubble.Render(rend, imgBubbles, imgBubblePrelight, imgBubbleFrozen);
            for (SingleBubble &bubble : singleBubbles) bubble.Render(rend, imgBubbles);

            SDL_RenderCopy(rend, imgBubbles[curArray.curLaunch], nullptr, new SDL_Rect{640/2 - 16, 480 - 89, 32, 32});
            SDL_RenderCopy(rend, imgBubbles[curArray.nextBubble], nullptr, new SDL_Rect{640/2 - 16, 480 - 40, 32, 32});
            SDL_RenderCopy(rend, onTopTexture, nullptr, new SDL_Rect{640/2 - 19, 480 - 43, 39, 39});

            UpdatePenguin(curArray);
            if(!lowGfx) {
                curArray.penguinSprite.Render();
                curArray.shooterSprite.Render();
            } else {
                curArray.lGfxShooterRct.x = (int)(SCREEN_CENTER_X + (LAUNCHER_DIAMETER * cosf(curArray.shooterSprite.angle)));
                curArray.lGfxShooterRct.y = (int)((480 - 69) - (LAUNCHER_DIAMETER * sinf(curArray.shooterSprite.angle)));
                SDL_RenderCopy(rend, lowShooterTexture, nullptr, &curArray.lGfxShooterRct);
            }
        }
    }

    SDL_RenderCopy(rend, inGameText.Texture(), nullptr, inGameText.Coords());

    if (!firstRenderDone) {
        TransitionManager::Instance()->TakeSnipOut(rend);
        firstRenderDone = true;
    }
}

void BubbleGame::RenderPaused() {
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);

    if(!playedPause) {
        audMixer->PauseMusic();
        audMixer->PlaySFX("pause");
        playedPause = true;
        pauseFrame = 0;

        if(prePauseBackground != nullptr) SDL_DestroyTexture(prePauseBackground);

        float w = 0, h = 0;
        SDL_RenderGetScale(rend, &w, &h);
        SDL_Surface *sfc = SDL_CreateRGBSurface(0, 640 * w, 480 * h, 32, 0, 0, 0, 0);
        SDL_RenderReadPixels(rend, NULL, SURF_FORMAT, sfc->pixels, sfc->pitch);
        prePauseBackground = SDL_CreateTextureFromSurface(rend, sfc);
        SDL_FreeSurface(sfc);
    }

    SDL_RenderClear(rend);
    SDL_RenderCopy(rend, prePauseBackground, nullptr, nullptr);
    SDL_RenderCopy(rend, pauseBackground, nullptr, nullptr);

    if (nextPauseUpd <= 0){
        pauseFrame++;
        nextPauseUpd = 2;
        if(pauseFrame >= 34) {
            pauseFrame = 12;
        }
    }
    else nextPauseUpd--;

    SDL_Rect pauseRct = {SCREEN_CENTER_X - 95, SCREEN_CENTER_Y - 72, 190, 143};
    SDL_RenderCopy(rend, pausePenguin[pauseFrame], nullptr, &pauseRct);

    timePaused = SDL_GetTicks();
}

void BubbleGame::HandleInput(SDL_Event *e) {
    switch(e->type) {
        case SDL_KEYDOWN:
            if(e->key.repeat) break;
            switch(e->key.keysym.sym) {
                case SDLK_UP:
                    if (gameFinish == false) bubbleArrays[0].shooterAction = true;
                    break;
                case SDLK_ESCAPE:
                    QuitToTitle();
                    break;
                case SDLK_F11: // mute / unpause audio
                    if(audMixer->IsHalted() == true) {
                        audMixer->MuteAll(true);
                        audMixer->PlayMusic("main1p");
                    }
                    else audMixer->MuteAll();
                    break;
                case SDLK_RETURN:
                    if (!gameFinish || (gameFinish && singleBubbles.size() > 0)) break;
                    if (gameWon) ReloadGame(++curLevel);
                    else if (gameLost) ReloadGame(curLevel);
                    break;
            }
            break;
    }
}

void BubbleGame::QuitToTitle() {
    RemoveArray(bubbleArrays[0]);
    FrozenBubble::Instance()->CallMenuReturn();
    firstRenderDone = false;
}