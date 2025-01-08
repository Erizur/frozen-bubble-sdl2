#ifndef BUBBLEGAME_H
#define BUBBLEGAME_H

#include <SDL2/SDL.h>
#include "shaderstuff.h"

#include <vector>

#pragma region "BubbleGame Defines"
#define TIME_APPEARS_NEW_ROOT_MP 11
#define TIME_HURRY_WARN_MP 250
#define TIME_HURRY_MAX_MP 375

#define TIME_APPEARS_NEW_ROOT 8
#define TIME_HURRY_WARN 400
#define TIME_HURRY_MAX 525

// fyi fc stands for frame count not full combo, this isn't a rhythm game!!!!!!
#define PENGUIN_HANDLEFC 71
#define PENGUIN_WAITFC 97
#define PENGUIN_WINFC 68
#define PENGUIN_LOSEFC 158

#define PENGUIN_FRAMEWAIT 3
#define TIMEOUT_PENGUIN_SLEEP 200

#define BUBBLE_STYLES 8
#define CANON_ROTATIONS_NB 100
#pragma endregion

//hardcoded framecount, theres like a ton of frames here
struct Penguin {
    SDL_Texture* handle[PENGUIN_HANDLEFC];
    SDL_Texture* wait[PENGUIN_WAITFC];
    SDL_Texture* win[PENGUIN_WINFC];
    SDL_Texture* lose[PENGUIN_LOSEFC];
    // curAnimation up to 13 values
    int curAnimation = 0, curFrame = 1, waitFrame = 0;
    bool invertAnimation = false;

    void LoadPenguin(SDL_Renderer* renderer, char *whichOne) {
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

        if (curAnimation == 2) curFrame = 19;
        else if (curAnimation == 7) curFrame =71;

        switch(curAnimation) {
            case 0: curFrame = 20; break;
            case 1: curFrame = curFrame < 21 || curFrame > 50 ? (invertAnimation == true ? 50 : 21) : curFrame; break;
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
            case 1: curFrame = curFrame < 21 || curFrame > 50 ? (invertAnimation == true ? 50 : 21) : curFrame; break;
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
};

struct Bubble {
    int bubbleId; // id to use bubble image
    SDL_Point pos; // current position
    bool falling = false; // check if the bubble is on falling state, else snap
    bool shining = false; // doing that shiny animation
    bool frozen = false; // frozen (game over)
};

struct Shooter {
    int bubbleToShow = 0;
    bool miniShooter = false;
    bool lowGfx = false;
    SDL_Surface *shooterSfc;
    SDL_Renderer *rend;
    float angle = 0;

    void Initialize(SDL_Renderer *renderer, const char *path) {
        //if you haven't setted up the miniShooter and lowGfx variables before this, you may be screwed
        shooterSfc = IMG_Load(path);
        rend = renderer;
    }

    SDL_Texture *OutputTexture() {
        return SDL_CreateTextureFromSurface(rend, shooterSfc);
    };
};

struct SetupSettings {
    bool chainReaction = false;
    int playerCount = 1;
    bool networkGame = false;
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
private:
    const SDL_Renderer *renderer;
    SDL_Texture *background;

    SDL_Surface *imgColorblindBubbles[BUBBLE_STYLES];
    SDL_Surface *imgBubbles[BUBBLE_STYLES];

    SDL_Surface *imgMiniColorblindBubbles[BUBBLE_STYLES];
    SDL_Surface *imgMiniBubbles[BUBBLE_STYLES];

    Penguin penguinSprites[5];
    SDL_Texture *pausePenguin[35];

    bool chainReaction;
    int timeLeft = 0, dangerZone = 99;

    std::vector<Bubble> bubbleArrays[5]; //5 vectors wtih different players
};

#endif // BUBBLEGAME_H