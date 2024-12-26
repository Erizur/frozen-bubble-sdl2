#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <SDL2/SDL.h>
#include <iostream>
#include <mutex>
#include <string>

class GameSettings final
{
public:
    void ReadSettings();
    void GetValue();

    const char *prefPath = SDL_GetPrefPath("frozen-bubble", "frozen-bubble");
    int gfxLevel() { return gfxQuality; }

    GameSettings(const GameSettings& obj) = delete;
    static GameSettings* instance(){
        return ptrInstance;
    };
private:
    void CreateDefaultSettings();

    int gfxQuality;

    GameSettings(){};
    static std::mutex mtx;
    static GameSettings* ptrInstance;
};

#endif //GAMESETTINGS_H