#ifndef HIGHSCOREMANAGER_H
#define HIGHSCOREMANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <vector>
#include <array>
#include <map>

#include "gamesettings.h"
#include "ttftext.h"

#define TEXTANIM_TICKSPEED 2

class HighscoreManager final
{
public:
    void ShowScoreScreen(int ls);
    void ShowNewScorePanel(int mode);
    void RenderPanel(void);
    void RenderScoreScreen(void);
    void HandleInput(SDL_Event *e);
    int lastState;

    void AppendToLevels(std::array<std::vector<int>, 10> lvl, int id);

    HighscoreManager(const HighscoreManager& obj) = delete;
    void Dispose();
    static HighscoreManager* Instance(SDL_Renderer *rend = nullptr);
private:
    enum ScoreScreen {
        Levelset = 0,
        Multiplayer = 1
    } scoreScreen = Levelset;
    int curMode;

    GameSettings *gameSettings;
    SDL_Renderer *rend;

    std::map<int, std::array<std::vector<int>,10>> highscoreLevels;
    void SaveNewHighscores();
    void LoadLevelsetHighscores(const char *path);
    void LoadHighscoreLevels(const char *path);
    void CreateLevelImages();

    SDL_Surface *backgroundSfc, *useBubbles[8];
    SDL_Texture *smallBG[10], *highscoresBG, *highscoreFrame, *headerLevelset, *headerMptrain;

    TTF_Font *highscoreFont;
    TTFText panelText, nameInput;

    SDL_Rect voidPanelRct = {(640/2) - (341/2), (480/2) - (280/2), 341, 280};
    SDL_Texture *voidPanelBG;

    std::string newName;
    int textTickWait = TEXTANIM_TICKSPEED;

    bool levelsetOnly = true, awaitKeyType = false, showTick = true;

    HighscoreManager(SDL_Renderer *rend);
    ~HighscoreManager();
    static HighscoreManager* ptrInstance;
};

#endif //HIGHSCOREMANAGER_H