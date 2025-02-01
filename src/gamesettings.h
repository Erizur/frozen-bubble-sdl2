#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <SDL2/SDL.h>
#include <iniparser.h>
#include <iostream>
#include <mutex>
#include <string>

struct PlayerKeys {
    SDL_Scancode left, right, fire, center;
};

class GameSettings final
{
public:
    void ReadSettings();
    void SaveSettings();
    void SetValue(const char *option, const char *value);
    void GetValue();

    const char *prefPath = SDL_GetPrefPath("", "frozen-bubble"); //do not place org name if you don't want duplicated folders!
    int gfxLevel() { return gfxQuality; }
    SDL_Point curResolution() { return {windowWidth, windowHeight}; }
    bool fullscreenMode() { return useFullscreen; }
    bool linearScaling;
    bool canPlayMusic() { return playMusic; }
    bool canPlaySFX() { return playSfx; }
    bool useClassicAudio() { return classicSound; }
    bool colorBlind() { return colorblindBubbles; }

    GameSettings(const GameSettings& obj) = delete;
    void Dispose();
    static GameSettings* Instance(){
        return ptrInstance;
    };
private:
    void CreateDefaultSettings();
    dictionary *optDict;

    int gfxQuality, windowWidth, windowHeight;
    bool useFullscreen, colorblindBubbles, playMusic, playSfx, classicSound;

    GameSettings(){};
    ~GameSettings();
    static std::mutex mtx;
    static GameSettings* ptrInstance;
};

#endif //GAMESETTINGS_H