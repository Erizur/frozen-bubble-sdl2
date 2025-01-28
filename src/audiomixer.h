#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <map>
#include <string>
#include <vector>

#include "gamesettings.h"

class AudioMixer final
{
public:
    void PlayMusic(const char *track);
    void PlaySFX(const char *sfx);
    void PauseMusic(bool enable = false);
    void MuteAll(bool enable = false);
    bool IsHalted() { return haltedMixer; };

    AudioMixer(const AudioMixer& obj) = delete;
    void Dispose();
    static AudioMixer* Instance();
private:
    std::map<const char *, Mix_Chunk *> sfxFiles;
    Mix_Chunk* GetSFX(const char *);

    AudioMixer();
    ~AudioMixer();
    static AudioMixer* ptrInstance;
    GameSettings* gameSettings;

    bool mixerEnabled = true, haltedMixer = false;
    Mix_Music* curMusic;
};

#endif // AUDIOMIXER_H