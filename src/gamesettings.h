#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <SDL2/SDL.h>
#include <iniparser.h>
#include <iostream>
#include <mutex>
#include <string>

class GameSettings final
{
public:
    void ReadSettings();
    void SaveSettings();
    void SetValue(const char *option, const char *value);
    void GetValue();

    const char *prefPath = SDL_GetPrefPath("frozen-bubble", "frozen-bubble");
    int gfxLevel() { return gfxQuality; }
    bool canPlayMusic() { return playMusic; }
    bool canPlaySFX() { return playSfx; }
    bool useClassicAudio() { return classicSound; } 

    GameSettings(const GameSettings& obj) = delete;
    void Dispose();
    static GameSettings* instance(){
        return ptrInstance;
    };
private:
    void CreateDefaultSettings();
    dictionary *optDict;

    int gfxQuality;
    bool playMusic, playSfx, classicSound;

    GameSettings(){};
    ~GameSettings();
    static std::mutex mtx;
    static GameSettings* ptrInstance;
};

#endif //GAMESETTINGS_H