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

    void MuteAll(bool enable = false);
    bool IsHalted() { return haltedMixer; };

    AudioMixer(const AudioMixer& obj) = delete;
    void Dispose();
    static AudioMixer* instance();
private:
    /*inline static const char * sfxTags[] = {
        "stick", "destroy_group", "newroot",
        "newroot_solo", "lose", "hurry", "pause",
        "menu_change", "menu_selected", "rebound",
        "launch", "malus", "noh", "snore", "cancel",
        "typewriter", "applause", "chatted"
    };*/
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