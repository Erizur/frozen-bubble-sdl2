#include "frozenbubble.h"
#include "bubblegame.h"
#include "audiomixer.h"

#include <cmath>
#include <algorithm>
#include <iterator>

inline int ranrange(int a, int b) { return a + rand() % ((b - a ) + 1); }

struct SingleBubble {
    int assignedArray; // assigned board to use
    int bubbleId; // id to use bubble image
    SDL_Point pos; // current position, top left aligned
    SDL_Point oldpos; // old position, useful for collision
    float direction; // angle
    bool falling = false; // is falling from the top
    bool launching = false; // is launched from shooter
    int leftLimit, rightLimit, topLimit; // limit before bouncing
    int bubbleSize = 32; // bubble size (change when creating for small variants)

    void CopyBubbleProperties(Bubble *prop) {
        bubbleId = (*prop).bubbleId;
        pos = (*prop).pos;
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
            pos.x -= BUBBLE_SPEED * SDL_cos(direction);
            pos.y -= BUBBLE_SPEED * SDL_sin(direction);
            if (pos.x < leftLimit) {
                AudioMixer::Instance()->PlaySFX("rebound");
                pos.x = 2 * leftLimit - pos.x;
                direction -= 2 * (direction-M_PI/2);
            }
            if (pos.x > rightLimit - bubbleSize) {
                AudioMixer::Instance()->PlaySFX("rebound");
                pos.x = 2 * (rightLimit - bubbleSize) - pos.x;
                direction += 2 * (M_PI/2-direction);
            }
        }
        else if (falling) {
            // not implemented
        }
    }

    void Render(SDL_Renderer *rend, SDL_Texture *bubbles[]) {
        if (bubbleId == -1) return;
        SDL_RenderCopy(rend, bubbles[bubbleId], nullptr, new SDL_Rect{pos.x, pos.y, bubbleSize, bubbleSize});
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

    SDL_Point *offset = &bubbleArrays[0].bubbleOffset;
    std::array<std::vector<Bubble>, 13> *bubbleMap = &bubbleArrays[0].bubbleMap;

    for (size_t i = 0; i < level.size(); i++)
    {
        int smallerSep = level[i].size() % 2 == 0 ? 0 : bubbleSize / 2;
        for (size_t j = 0; j < level[i].size(); j++)
        {
            (*bubbleMap)[i].push_back(Bubble{level[i][j], {(smallerSep + bubbleSize * ((int)j)) + offset->x, (initBubbleY * ((int)i)) + offset->y}});
        }
    }
    if((*bubbleMap)[9].size() % 2 == 0) {
        for (int i = 0; i < 3; i++)
        {
            int smallerSep = i % 2 == 0 ? 0 : bubbleSize / 2;
            for (int j = 0; j < (i % 2 == 0 ? 7 : 8); j++) (*bubbleMap)[10 + i].push_back({-1, {(smallerSep + bubbleSize * j) + offset->x, (initBubbleY * (10 + i)) + offset->y}});
        }
    } else {
        for (int i = 1; i < 4; i++)
        {
            int smallerSep = i % 2 == 0 ? bubbleSize / 2 : 0;
            for (int j = 0; j < (i % 2 == 0 ? 7 : 8); j++) (*bubbleMap)[10 + (i - 1)].push_back({-1, {(smallerSep + bubbleSize * j) + offset->x, (initBubbleY * (9 + i)) + offset->y}});
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
        bubbleArrays[0].penguinSprite.LoadPenguin(rend, (char*)"p1");
        bubbleArrays[0].shooterSprite = {lowGfx ? lowShooterTexture : shooterTexture, rend};
        bubbleArrays[0].bubbleOffset = {190, 51};
        bubbleArrays[0].leftLimit = (640 / 2) - 128;
        bubbleArrays[0].rightLimit = (640 / 2) + 128;
        bubbleArrays[0].topLimit = 51;
        bubbleArrays[0].numSeparators = 0;
        audMixer->PlayMusic("main1p");
    }

    LoadLevelset(DATA_DIR "/data/levels");
    LoadLevel(10);

    FrozenBubble::Instance()->startTime = SDL_GetTicks();
    FrozenBubble::Instance()->currentState = MainGame;

    ChooseFirstBubble(bubbleArrays[0]);
}

void BubbleGame::LaunchBubble(BubbleArray &bArray) {
    audMixer->PlaySFX("launch");
    singleBubbles.push_back({0, bArray.curLaunch, {640/2 - 19, 480 - 89}, {}, bArray.shooterSprite.angle, false, true, bArray.leftLimit, bArray.rightLimit, bArray.topLimit});
    PickNextBubble(bArray);
}

void BubbleGame::UpdatePenguin(BubbleArray &bArray) {
    if (gameFinish) return;

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
        if (sBubble.assignedArray != id) continue; // if not from current array, ignore!
        sBubble.UpdatePosition();
        if (sBubble.launching != true) continue; // if not being launched, ignore collision!
        for (const std::vector<Bubble> &vecBubble : bArray->bubbleMap) for (Bubble bubble : vecBubble) {
            if (sBubble.IsCollision(&bubble)) {
                int row, col;
                GetClosestFreeCell(sBubble, *bArray, &row, &col);
                bArray->PlacePlayerBubble(sBubble.bubbleId, row, col);
                bArray->newShoot = true;
                audMixer->PlaySFX("stick");
                singleBubbles.erase(singleBubbles.begin() + i);
                CheckPossibleDestroy(*bArray);
                CheckGameState(*bArray);
                return;
            }
        };
    }
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
                        bubble->bubbleId = -1;
                        bubble->playerBubble = false;
                    }
                }
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

void BubbleGame::CheckAirBubbles(BubbleArray &bArray) {
    for (size_t i = 0; i < bArray.bubbleMap.size(); i++) {
        for (size_t j = 0; j < bArray.bubbleMap[i].size(); j++) {
            if (bArray.bubbleMap[i][j].bubbleId == -1) continue; //just skip
            if (i > 0) { //not the top row
                bool attached = false;
                CheckIfAttached(bArray, i, j, 99, &attached);
                if (attached == false) {
                    bArray.bubbleMap[i][j].bubbleId = -1;
                    bArray.bubbleMap[i][j].playerBubble = false;
                    continue;
                }
            }
            else continue;
        }
    }
}

void BubbleGame::CheckGameState(BubbleArray &bArray) {
    bArray.turnsToCompress--;
    if (bArray.turnsToCompress == 0) {
        bArray.turnsToCompress = 9;
        bArray.dangerZone--;
        bArray.numSeparators++;
        bArray.ExpandOffset(0, 28);
        audMixer->PlaySFX("newroot_solo");
    }
    if (bArray.allClear()) {
        gameFinish = true;
        gameWon = true;
        bArray.penguinSprite.PlayAnimation(10);
    }
    if (bArray.bubbleOnDanger()) {
        gameFinish = true;
        audMixer->PlaySFX("lose");
        bArray.penguinSprite.PlayAnimation(11);
    }
}

void BubbleGame::Render() {
    SDL_Renderer *rend = const_cast<SDL_Renderer*>(renderer);
    SDL_RenderCopy(rend, background, nullptr, nullptr);
    
    if(currentSettings.playerCount == 1) {
        BubbleArray &curArray = bubbleArrays[0];

        UpdateSingleBubbles(0);
        for (const std::vector<Bubble> &vecBubble : curArray.bubbleMap) for (Bubble bubble : vecBubble) bubble.Render(rend, imgBubbles, imgBubblePrelight, imgBubbleFrozen);
        for (SingleBubble &bubble : singleBubbles) bubble.Render(rend, imgBubbles);

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

        for (int i = 1; i < 10; i++) SDL_RenderCopy(rend, dotTexture[i == curArray.turnsToCompress ? 1 : 0], nullptr, new SDL_Rect{curArray.rightLimit, 104 - (7 * i) - i, 7, 7});
        for (int i = 0; i < curArray.numSeparators; i++) SDL_RenderCopy(rend, sepCompressorTexture, nullptr, new SDL_Rect{(640/2) - 95, (28 * i), 188, 28});
        SDL_RenderCopy(rend, compressorTexture, nullptr, new SDL_Rect{(640/2) - 128, -5 + (28 * curArray.numSeparators), 252, 56});
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
                    if (gameFinish == false) bubbleArrays[0].shooterAction = true;
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