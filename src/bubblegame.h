#ifndef BUBBLEGAME_H
#define BUBBLEGAME_H

#define M_PI 3.1415926535897932384626433832795028841972

#include <SDL2/SDL.h>
#include "shaderstuff.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <algorithm>

#pragma region "BubbleGame Defines"
#define TIME_APPEARS_NEW_ROOT_MP 11
#define TIME_HURRY_WARN_MP 250
#define TIME_HURRY_MAX_MP 375

#define TIME_APPEARS_NEW_ROOT 8
#define TIME_HURRY_WARN 400
#define TIME_HURRY_MAX 525

// frame count for animations
#define PENGUIN_HANDLEFC 71
#define PENGUIN_WAITFC 97
#define PENGUIN_WINFC 68
#define PENGUIN_LOSEFC 158

#define PENGUIN_FRAMEWAIT 1
#define TIMEOUT_PENGUIN_SLEEP 200 * 2

#define BUBBLE_STYLES 8
#define BUBBLE_STICKFC 7
#define BUBBLE_SPEED 10 / 2
#define MALUS_BUBBLE_SPEED 30 / 2
#define LAUNCHER_SPEED 0.015 * 0.6

#define LAUNCHER_DIAMETER 50
#define LAUNCHER_DIAMETER_MINI 25
#define CANON_ROTATIONS 100

#define COMPRESSOR_OFFSET 28
#define FREEFALL_CONSTANT 0.5
#define FROZEN_FRAMEWAIT 1
#pragma endregion

//hardcoded framecount, theres like a ton of frames here
struct Penguin {
    SDL_Texture* handle[PENGUIN_HANDLEFC];
    SDL_Texture* wait[PENGUIN_WAITFC];
    SDL_Texture* win[PENGUIN_WINFC];
    SDL_Texture* lose[PENGUIN_LOSEFC];
    // curAnimation up to 13 values
    int curAnimation = 0, curFrame = 1, waitFrame = 0;
    int sleeping = 0;
    bool invertAnimation = false;
    SDL_Renderer *rend;

    void LoadPenguin(SDL_Renderer* renderer, char *whichOne) {
        rend = renderer;

        char path[256];
        for (int i = 0; i < PENGUIN_HANDLEFC; i++) {
            sprintf(path, DATA_DIR "/gfx/pinguins/anime-shooter_%s_%04d.png", whichOne, i + 1);
            handle[i] = IMG_LoadTexture(renderer, path);
        }
        for (int i = 0; i < PENGUIN_WAITFC; i++) {
            sprintf(path, DATA_DIR "/gfx/pinguins/wait_%s_%04d.png", whichOne, i + 1);
            wait[i] = IMG_LoadTexture(renderer, path);
        }
        for (int i = 0; i < PENGUIN_WINFC; i++) {
            sprintf(path, DATA_DIR "/gfx/pinguins/win_%s_%04d.png", whichOne, i + 1);
            win[i] = IMG_LoadTexture(renderer, path);
        }
        for (int i = 0; i < PENGUIN_LOSEFC; i++) {
            sprintf(path, DATA_DIR "/gfx/pinguins/loose_%s_%04d.png", whichOne, i + 1);
            lose[i] = IMG_LoadTexture(renderer, path);
        }
    }

    int PlayAnimation(int animId) {
        if (animId > 12) return 0;
        curAnimation = animId;
        curFrame = 0;

        //check if animation needs invertion or not
        if (curAnimation == 2 || curAnimation == 7) invertAnimation = true;
        else invertAnimation = false;

        if (curAnimation == 1) curFrame = 21;
        else if (curAnimation == 2) curFrame = 19;
        else if (curAnimation == 7) curFrame =71;

        switch(curAnimation) {
            case 0: curFrame = 20; break;
            case 1: curFrame = curFrame < 21 || curFrame > 50 ? (invertAnimation == true ? (curFrame < 21 ? PlayAnimation(0) : 50) : (curFrame > 50 ? PlayAnimation(0) : 21)) : curFrame; break;
            case 2: curFrame = curFrame < 2 || curFrame > 19 ? (invertAnimation == true ? (curFrame < 2 ? PlayAnimation(3) : 19) : (curFrame > 19 ? PlayAnimation(3) : 2)) : curFrame; break;
            case 3: curFrame = 1; break;
            case 4: curFrame = curFrame < 2 || curFrame > 19 ? (invertAnimation == true ? (curFrame < 2 ? PlayAnimation(0) : 19) : (curFrame > 19 ? PlayAnimation(0) : 2)) : curFrame; break;
            case 5: curFrame = curFrame < 51 || curFrame > 70 ? (invertAnimation == true ? (curFrame < 51 ? PlayAnimation(6) : 70) : (curFrame > 70 ? PlayAnimation(6) : 51)) : curFrame; break;
            case 6: curFrame = 71; break;
            case 7: curFrame = curFrame < 51 || curFrame > 71 ? (invertAnimation == true ? (curFrame < 51 ? PlayAnimation(0) : 71) : (curFrame > 71 ? PlayAnimation(0) : 51)) : curFrame; break;
            case 8: curFrame = curFrame < 1 || curFrame > 74 ? (invertAnimation == true ? (curFrame < 1 ? PlayAnimation(9) : 74) : (curFrame > 74 ? PlayAnimation(9) : 1)) : curFrame; break;
            case 9: curFrame = curFrame < 75 || curFrame > 97 ? (invertAnimation == true ? 97 : 75) : curFrame; break;
            case 10: curFrame = curFrame < 1 || curFrame > 68 ? (invertAnimation == true ? 68 : 1) : curFrame; break;
            case 11: curFrame = curFrame < 1 || curFrame > 64 ? (invertAnimation == true ? (curFrame < 1 ? PlayAnimation(12) : 64) : (curFrame > 64 ? PlayAnimation(12) : 1)) : curFrame; break;
            case 12: curFrame = curFrame < 65 || curFrame > 158 ? (invertAnimation == true ? 158 : 65) : curFrame; break;
        }

        return curFrame;
    }

    void UpdateFrame() {
        //check if animation needs invertion or not
        if (curAnimation == 2 || curAnimation == 7) invertAnimation = true;
        else invertAnimation = false;

        if(waitFrame >= PENGUIN_FRAMEWAIT) {
            waitFrame = 0;
            if (invertAnimation == false) curFrame++;
            else curFrame--;
        }
        else waitFrame++;

        switch(curAnimation) {
            case 0: curFrame = 20; break;
            case 1: curFrame = curFrame < 21 || curFrame > 50 ? (invertAnimation == true ? (curFrame < 21 ? PlayAnimation(0) : 50) : (curFrame > 50 ? PlayAnimation(0) : 21)) : curFrame; break;
            case 2: curFrame = curFrame < 2 || curFrame > 19 ? (invertAnimation == true ? (curFrame < 2 ? PlayAnimation(3) : 19) : (curFrame > 19 ? PlayAnimation(3) : 2)) : curFrame; break;
            case 3: curFrame = 1; break;
            case 4: curFrame = curFrame < 2 || curFrame > 19 ? (invertAnimation == true ? (curFrame < 2 ? PlayAnimation(0) : 19) : (curFrame > 19 ? PlayAnimation(0) : 2)) : curFrame; break;
            case 5: curFrame = curFrame < 51 || curFrame > 70 ? (invertAnimation == true ? (curFrame < 51 ? PlayAnimation(6) : 70) : (curFrame > 70 ? PlayAnimation(6) : 51)) : curFrame; break;
            case 6: curFrame = 71; break;
            case 7: curFrame = curFrame < 51 || curFrame > 71 ? (invertAnimation == true ? (curFrame < 51 ? PlayAnimation(0) : 71) : (curFrame > 71 ? PlayAnimation(0) : 51)) : curFrame; break;
            case 8: curFrame = curFrame < 1 || curFrame > 74 ? (invertAnimation == true ? (curFrame < 1 ? PlayAnimation(9) : 74) : (curFrame > 74 ? PlayAnimation(9) : 1)) : curFrame; break;
            case 9: curFrame = curFrame < 75 || curFrame > 97 ? (invertAnimation == true ? 97 : 75) : curFrame; break;
            case 10: curFrame = curFrame < 1 || curFrame > 68 ? (invertAnimation == true ? 68 : 1) : curFrame; break;
            case 11: curFrame = curFrame < 1 || curFrame > 64 ? (invertAnimation == true ? (curFrame < 1 ? PlayAnimation(12) : 64) : (curFrame > 64 ? PlayAnimation(12) : 1)) : curFrame; break;
            case 12: curFrame = curFrame < 65 || curFrame > 158 ? (invertAnimation == true ? 158 : 65) : curFrame; break;
        }
    }

    SDL_Texture *CurrentFrame() {
        UpdateFrame();
        
        if (curAnimation <= 7) return handle[curFrame - 1];
        else if (curAnimation == 8 || curAnimation == 9) return wait[curFrame - 1];
        else if (curAnimation == 10) return win[curFrame - 1];
        else return lose[curFrame - 1];
    }

    void Render(SDL_Rect *dstrct) {
        SDL_RenderCopy(rend, CurrentFrame(), nullptr, dstrct);
    }
};

struct Bubble {
    int bubbleId; // id to use bubble image
    SDL_Point pos; // current position, top left aligned
    bool playerBubble = false; // if bubble was launched by player
    bool shining = false; // doing that shiny animation
    bool frozen = false; // frozen (game over)

    void RenderFrozen(SDL_Renderer *rend, SDL_Texture *frozen) {
        if (bubbleId == -1) return;
        SDL_Point size;
        SDL_QueryTexture(frozen, NULL, NULL, &size.x, &size.y);
        SDL_RenderCopy(rend, frozen, nullptr, new SDL_Rect{pos.x - 2, pos.y - 2, size.x, size.y});
    }

    void Render(SDL_Renderer *rend, SDL_Texture *bubbles[], SDL_Texture *shinyTexture, SDL_Texture *frozenTexture) {
        if (bubbleId == -1) return;
        SDL_Point size;
        SDL_QueryTexture(bubbles[bubbleId], NULL, NULL, &size.x, &size.y);
        SDL_RenderCopy(rend, bubbles[bubbleId], nullptr, new SDL_Rect{pos.x, pos.y, size.x, size.y});
        if (frozen) RenderFrozen(rend, frozenTexture);
        if(shining) SDL_RenderCopy(rend, shinyTexture, nullptr, new SDL_Rect{pos.x, pos.y, size.x, size.y});
    };
};

struct Shooter {
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    float angle = M_PI/2.;

    void Render(SDL_Rect *dstrct){
        SDL_RenderCopyEx(renderer, texture, nullptr, dstrct, ((angle*CANON_ROTATIONS/(M_PI/2.) + 0.5) - CANON_ROTATIONS), NULL, SDL_FLIP_NONE);
    }
};

struct SetupSettings {
    bool chainReaction = false;
    int playerCount = 1;
    bool networkGame = false;
};

struct BubbleArray {
    std::array<std::vector<Bubble>, 13> bubbleMap;
    SDL_Point bubbleOffset;
    Penguin penguinSprite;
    Shooter shooterSprite;
    int playerAssigned, nextBubble, curLaunch, leftLimit, rightLimit, topLimit, numSeparators, turnsToCompress = 9, dangerZone = 12, frozenWait = FROZEN_FRAMEWAIT;
    bool shooterLeft = false, shooterRight = false, shooterCenter = false, shooterAction = false, newShoot = true;

    std::vector<int> remainingBubbles() {
        std::vector<int> a;
        for (int i = 0; i < 13; i++) {
            for (const Bubble &bubble : bubbleMap[i]) {
                if (bubble.bubbleId != -1 && std::count(a.begin(), a.end(), bubble.bubbleId) == 0) a.push_back(bubble.bubbleId); 
            }
        }

        return a;
    }

    bool allClear() {
        for (int i = 0; i < 13; i++) {
            for (const Bubble &bubble : bubbleMap[i]) {
                if (bubble.bubbleId != -1) return false; 
            }
        }

        return true;
    }
    
    bool bubbleOnDanger() {
        for (int i = 0; i < 13; i++) {
            for (const Bubble &bubble : bubbleMap[i]) {
                if (i >= dangerZone && bubble.bubbleId != -1) return true; 
            }
        }

        return false;
    }

    void ExpandOffset(int ex, int ey) {
        bubbleOffset.x += ex;
        bubbleOffset.y += ey;
        topLimit += ey;
        leftLimit += ex;
        rightLimit += ex;

        for (int i = 0; i < 13; i++) {
            for (Bubble &bubble : bubbleMap[i]) {
                bubble.pos.x += ex;
                bubble.pos.y += ey;
            }
        }
    }

    void PlacePlayerBubble(int bubbleId, int row, int col) {
        Bubble &bubble = bubbleMap[row][col];
        bubble.bubbleId = bubbleId;
        bubble.playerBubble = true;
    }
};

class BubbleGame final
{
public:
    BubbleGame(const SDL_Renderer *renderer);
    BubbleGame(const BubbleGame&) = delete;
    ~BubbleGame();

    void Render(void);
    void NewGame(SetupSettings setup);
    void HandleInput(SDL_Event *e);
    void UpdatePenguin(BubbleArray &bArray);

    void LoadLevelset(const char *path);
    void LoadLevel(int id);
private:
    const SDL_Renderer *renderer;
    SDL_Texture *background;

    SDL_Texture *imgColorblindBubbles[BUBBLE_STYLES];
    SDL_Texture *imgBubbles[BUBBLE_STYLES];

    SDL_Texture *imgMiniColorblindBubbles[BUBBLE_STYLES];
    SDL_Texture *imgMiniBubbles[BUBBLE_STYLES];

    SDL_Texture *imgBubbleStick[BUBBLE_STICKFC];
    SDL_Texture *imgMiniBubbleStick[BUBBLE_STICKFC];

    SDL_Texture *imgBubbleFrozen, *imgMiniBubbleFrozen;
    SDL_Texture *imgBubblePrelight, *imgMiniBubblePrelight;

    SDL_Texture *pausePenguin[35];
    SDL_Texture *dotTexture[2];

    SDL_Texture *soloStatePanels[2];
    SDL_Texture *multiStatePanels[2];

    SDL_Texture *shooterTexture, *miniShooterTexture, *lowShooterTexture, *compressorTexture, *sepCompressorTexture, *onTopTexture, *miniOnTopTexture;

    bool lowGfx = false, gameWon = false, gameLost = false, gameFinish = false;

    bool chainReaction = false;
    int timeLeft = 0, dangerZone = 12;

    SetupSettings currentSettings;
    AudioMixer *audMixer;

    std::vector<std::array<std::vector<int>, 10>> loadedLevels;
    BubbleArray bubbleArrays[5]; //5 custom arrays wtih different players

    void ChooseFirstBubble(BubbleArray &bArray);
    void PickNextBubble(BubbleArray &bArray);
    void LaunchBubble(BubbleArray &bArray);
    void UpdateSingleBubbles(int id);
    
    void CheckPossibleDestroy(BubbleArray &bArray);
    void CheckAirBubbles(BubbleArray &bArray);
    void CheckGameState(BubbleArray &bArray);

    void DoFrozenAnimation(BubbleArray &bArray, int &waitTime);
};

#endif // BUBBLEGAME_H