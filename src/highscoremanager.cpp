#include "highscoremanager.h"
#include "shaderstuff.h"
#include "frozenbubble.h"
#include "ttftext.h"

#include <fstream>
#include <sstream>
#include <string>

struct HighscoreData {
    int level;
    float time;
    std::string name;
    int picId;
    TTFText layoutText;
    bool newHighscore = false;

    std::string formatTime(){
        int min = time / 60;
        int sec = time - min*60;
        char time[8];
        sprintf(time, "%d'%02d\"", min, sec);
        return std::string(time);
    }

    void RefreshTextStatus(SDL_Renderer *rend, TTF_Font *fnt){
        layoutText.LoadFont(DATA_DIR "/gfx/DroidSans.ttf", 16);
        layoutText.UpdateColor({255, 255, 255, 255},  {0, 0, 0, 255});
        layoutText.UpdateAlignment(TTF_WRAPPED_ALIGN_CENTER);
        if (newHighscore) layoutText.UpdateStyle(TTF_STYLE_BOLD);
        std::string data = (name.size() > 12 ? name.substr(0, 9) + "..." : name) + "\n" + (level > 100 ? "won!" : "level " + std::to_string(level)) + "\n" + formatTime(); 
        layoutText.UpdateText(rend, data.c_str(), 0);
    }
};
std::vector<HighscoreData> levelsetScores;

HighscoreManager *HighscoreManager::ptrInstance = NULL;

HighscoreManager *HighscoreManager::Instance(SDL_Renderer *rend)
{
    if(ptrInstance == NULL)
        ptrInstance = new HighscoreManager(rend);
    return ptrInstance;
}

void HighscoreManager::LoadLevelsetHighscores(const char *path) {
    std::ifstream scoreSet(path);
    std::string curLine;
    
    if(scoreSet.is_open())
    {
        std::string curChar;
        while(std::getline(scoreSet, curLine))
        {
            
            int task = 0;
            if (!curLine.empty())
            {
                std::stringstream ss(curLine);
                HighscoreData hs;
                while(std::getline(ss, curChar, ','))
                {
                    if (task == 0) hs.level = stoi(curChar);
                    else if (task == 1) hs.name = curChar;
                    else if (task == 2) hs.time = stof(curChar);
                    else if (task == 3) {
                        hs.picId = stoi(curChar);
                        levelsetScores.push_back(hs);
                    }
                    task++;
                }
                task = 0;
            }
        }
    }
    else {
        SDL_LogError(1, "Could not load highscore levels (%s).", path);
    }
}

void HighscoreManager::LoadHighscoreLevels(const char *path) {
    std::ifstream lvlSet(path);
    std::string curLine;

    highscoreLevels.clear();
    if(lvlSet.is_open())
    {
        int linecount = 0;
        int idx = 0;
        std::string curChar;
        std::array<std::vector<int>, 10> level;
        std::vector<int> line;
        while(std::getline(lvlSet, curLine))
        {
            linecount++;
            if (curLine.empty())
            {
                if (linecount < 10) return;
                idx = 0;
                highscoreLevels[highscoreLevels.size()] = level;
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
        SDL_LogError(1, "Could not load highscore levels (%s).", path);
    }
}

void HighscoreManager::AppendToLevels(std::array<std::vector<int>, 10> lvl, int id){

}

HighscoreManager::HighscoreManager(SDL_Renderer *renderer)
{
    rend = renderer;
    gameSettings = GameSettings::Instance();

    backgroundSfc = IMG_Load(DATA_DIR "/gfx/back_one_player.png");

    char path[256];
    for (int i = 1; i <= 8; i++)
    {
        if(gameSettings->colorBlind()) {
            sprintf(path, DATA_DIR "/gfx/balls/bubble-colourblind-%d.gif", i);
            useBubbles[i - 1] = IMG_Load(path);
        }
        else {
            sprintf(path, DATA_DIR "/gfx/balls/bubble-%d.gif", i);
            useBubbles[i - 1] = IMG_Load(path);
        }
    }

    highscoresBG = IMG_LoadTexture(rend, DATA_DIR "/gfx/back_hiscores.png");
    highscoreFrame = IMG_LoadTexture(rend, DATA_DIR "/gfx/hiscore_frame.png");
    headerLevelset = IMG_LoadTexture(rend, DATA_DIR "/gfx/hiscore-levelset.png");
    headerMptrain = IMG_LoadTexture(rend, DATA_DIR "/gfx/hiscore-mptraining.png");

    highscoreFont = TTF_OpenFont(DATA_DIR "/gfx/DroidSans.ttf", 18);

    voidPanelBG = IMG_LoadTexture(rend, DATA_DIR "/gfx/menu/void_panel.png");
    
    panelText.LoadFont(DATA_DIR "/gfx/DroidSans.ttf", 15);
    nameInput.LoadFont(DATA_DIR "/gfx/DroidSans.ttf", 15);
    panelText.UpdateAlignment(TTF_WRAPPED_ALIGN_CENTER);
    nameInput.UpdateAlignment(TTF_WRAPPED_ALIGN_CENTER);
    panelText.UpdateColor({255, 255, 255, 255}, {0, 0, 0, 255});
    nameInput.UpdateColor({255, 255, 255, 255}, {0, 0, 0, 255});

    panelText.UpdateText(rend, "Congratulations!\n\nYou got a high score!\n\nEnter name:            \n", 0);
    panelText.UpdatePosition({(640/2) - (panelText.Coords()->w / 2), (480/2) - 120});
    nameInput.UpdatePosition({(640/2) - 45 - (panelText.Coords()->w / 2), (480/2) - 25});

    std::string historypath = gameSettings->prefPath + std::string("highlevelshistory");
    std::string levelsetpath = gameSettings->prefPath + std::string("highscores");
    LoadHighscoreLevels(historypath.c_str());
    LoadLevelsetHighscores(levelsetpath.c_str());

    CreateLevelImages();
}

HighscoreManager::~HighscoreManager(){
    levelsetScores.clear();
    TTF_CloseFont(highscoreFont);
}

void HighscoreManager::Dispose(){
    SaveNewHighscores();
    this->~HighscoreManager();
}

std::string levelToData(std::array<std::vector<int>, 10> lvl) {
    std::string current;
    for (int i = 0; i < 10; i++) {
        if (lvl[i].size() != 8) current += "  ";
        for (size_t j = 0; j < lvl[i].size(); j++) {
            if (lvl[i][j] != -1) current += std::to_string(lvl[i][j]);
            else current += "-";
            if (j < lvl[i].size() - 1) current += "   ";
            else current += "\n";
        }
    }
    current += "\n";
    return current;
}

void HighscoreManager::SaveNewHighscores() {
    std::string historypath = gameSettings->prefPath + std::string("highlevelshistory");
    std::string levelsetpath = gameSettings->prefPath + std::string("highscores");

    std::ofstream historyFile(historypath);
    for (size_t i = 0; i < highscoreLevels.size(); i++)
    {
        historyFile << levelToData(highscoreLevels[i]);
    }
    historyFile.close();

    std::ofstream levelsetFile(levelsetpath);
    for (size_t i = 0; i < levelsetScores.size(); i++) {
        HighscoreData &a = levelsetScores[i];
        levelsetFile << a.level << "," << a.name << "," << a.time << "," << a.picId << "\n"; 
    }
    levelsetFile.close();
}

void HighscoreManager::CreateLevelImages() {
    SDL_Rect highRect = {(640/2)-128, 51, ((640/2)+128)-((640/2)-128), 340};

    for (int i = 0; i < 10; i++) {
        if (i >= (int)highscoreLevels.size()) continue;
        if (smallBG[i] != nullptr) SDL_DestroyTexture(smallBG[i]);
        SDL_Surface *bigOne = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SDL_PIXELFORMAT_ARGB8888);
        SDL_Surface *sfc = SDL_CreateRGBSurfaceWithFormat(0, highRect.w/4, highRect.h/4, 32, SDL_PIXELFORMAT_ARGB8888);
        SDL_BlitSurface(backgroundSfc, nullptr, bigOne, nullptr);
        std::array<std::vector<int>, 10> lvl = highscoreLevels[i];
        for (int j = 0; j < 10; j++){
            int smallerSep = lvl[j].size() % 2 == 0 ? 0 : 32 / 2;
            for (size_t k = 0; k < lvl[j].size(); k++) {
                if (lvl[j][k] == -1) continue;
                SDL_Rect dest = {(smallerSep + 32 * ((int)k)) + 190, (32 * j) + 51, 64, 64};
                SDL_BlitSurface(useBubbles[lvl[j][k]], nullptr, bigOne, &dest);
            }
        }
        shrink_(sfc, bigOne, 0, 0, &highRect, 4);
        smallBG[i] = SDL_CreateTextureFromSurface(rend, sfc);
    }

    for (size_t i = 0; i < levelsetScores.size(); i++) {
        levelsetScores[i].RefreshTextStatus(rend, highscoreFont);
        levelsetScores[i].layoutText.UpdatePosition({120 * ((int)i + 1) - levelsetScores[i].layoutText.Coords()->w/2, (115 * ((int)i + 1)) + (70 * (((int)i + 1) % 6))});
    }
}

void HighscoreManager::ShowScoreScreen(int ls) {
    lastState = ls;
    FrozenBubble::Instance()->currentState = Highscores;
}

void HighscoreManager::RenderScoreScreen() {
    SDL_RenderCopy(rend, highscoresBG, nullptr, nullptr);
    
    if (curMode == Levelset) {
        for (size_t i = 0; i < levelsetScores.size(); i++) {
            int sx, sy;
            SDL_QueryTexture(smallBG[i], nullptr, nullptr, &sx, &sy);
            SDL_Rect bgPos = {85 * ((int)i + 1), (20 * ((int)i + 1)) + (70 * (((int)i + 1) % 6)), sx, sy};
            SDL_Rect framePos = {bgPos.x - 7, bgPos.y - 7, 81, 100};
            SDL_RenderCopy(rend, highscoreFrame, nullptr, &framePos);
            SDL_RenderCopy(rend, smallBG[i], nullptr, &bgPos);
            SDL_RenderCopy(rend, levelsetScores[i].layoutText.Texture(), nullptr, levelsetScores[i].layoutText.Coords());
        }
    }
}

void HighscoreManager::ShowNewScorePanel(int mode) {
    SDL_StartTextInput();
}

void HighscoreManager::RenderPanel() {
    SDL_RenderCopy(rend, voidPanelBG, nullptr, &voidPanelRct);
    SDL_RenderCopy(rend, panelText.Texture(), nullptr, panelText.Coords());

    if (textTickWait <= 0) {
        if (awaitKeyType) {
            if (showTick) {
                nameInput.UpdateText(rend, newName.c_str(), 0);
                showTick = false;
            }
            else {
                std::string nam = newName + "|";
                nameInput.UpdateText(rend, nam.c_str(), 0);
                showTick = true;
            }
        }
    }
    else textTickWait--;

    SDL_RenderCopy(rend, nameInput.Texture(), nullptr, nameInput.Coords());
}

void HighscoreManager::HandleInput(SDL_Event *e){
    switch(e->type) {
        case SDL_KEYDOWN:
            if(e->key.repeat) break;
            switch(e->key.keysym.sym) {
                case SDLK_ESCAPE:
                    if (lastState != 1) FrozenBubble::Instance()->currentState = TitleScreen;
                    break;
                case SDLK_RETURN:
                    if (awaitKeyType) {
                        awaitKeyType = false;
                        panelText.UpdateText(rend, "Congratulations!\n\nYou got a high score!\n\nEnter name:            \n\n\nGood game!", 0);
                        SDL_StopTextInput();
                        break;
                    }
                    if (lastState != 1) FrozenBubble::Instance()->currentState = TitleScreen;
                    break;
                case SDLK_BACKSPACE:
                    if (awaitKeyType) {
                        if(newName.size() == 0) AudioMixer::Instance()->PlaySFX("stick");
                        else {
                            newName.pop_back();
                            AudioMixer::Instance()->PlaySFX("typewriter");
                        }
                    }
                    break;
                default:
                    if (!awaitKeyType) {
                        if (lastState != 1) FrozenBubble::Instance()->currentState = TitleScreen;
                    }
                    break;
            }
            break;
        case SDL_TEXTINPUT:
            if (newName.size() < 11){
                newName += e->text.text;
                std::string nam = newName + "|";
                nameInput.UpdateText(rend, nam.c_str(), 0);
                showTick = true;
                textTickWait = TEXTANIM_TICKSPEED + 10;
                AudioMixer::Instance()->PlaySFX("typewriter");
            }
            else {
                AudioMixer::Instance()->PlaySFX("stick");
            }
            break;       
    }
}