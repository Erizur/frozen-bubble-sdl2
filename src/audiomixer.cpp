#include "audiomixer.h"

const struct MusicFile
{
    const char *id;
    const char *file;
} musicFiles[3] = {
    { "intro", "/snd/introzik.ogg" },
    { "main1p", "/snd/frozen-mainzik-1p.ogg" },
    { "main2p", "/snd/frozen-mainzik-2p.ogg" },
};

AudioMixer *AudioMixer::ptrInstance = NULL;

AudioMixer *AudioMixer::Instance()
{
    if(ptrInstance == NULL)
        ptrInstance = new AudioMixer();
    return ptrInstance;
}

AudioMixer::AudioMixer()
{
    gameSettings = GameSettings::Instance();

    int freq = gameSettings->useClassicAudio() ? 22050 : MIX_DEFAULT_FREQUENCY;
    int sampl = gameSettings->useClassicAudio() ? 1024 : 4096;

    if (Mix_OpenAudio(freq, MIX_DEFAULT_FORMAT, 2, sampl) < 0) {
        SDL_LogError(1, "Could not open audio mixer! Music will be disabled. (%s)", Mix_GetError());
        mixerEnabled = false;
    }
}

Mix_Chunk* AudioMixer::GetSFX(const char *sfx){
    if(sfxFiles[sfx] == nullptr) {
        char path[256];
        sprintf(path, DATA_DIR "/snd/%s.ogg", sfx);
        sfxFiles[sfx] = Mix_LoadWAV(path);
    }
    
    return sfxFiles[sfx];
}

AudioMixer::~AudioMixer(){
}

void AudioMixer::Dispose(){
    Mix_Quit();
    this->~AudioMixer();
}

void AudioMixer::PlayMusic(const char *track)
{
    if(mixerEnabled == false || !gameSettings->canPlayMusic() || haltedMixer == true) return;

    while (Mix_FadingMusic()) SDL_Delay(10);
    if (Mix_PlayingMusic()) Mix_FadeOutMusic(500);
    SDL_Delay(400);
    while (Mix_PlayingMusic()) SDL_Delay(10);
    
    std::string path(std::string(DATA_DIR));
    for (const MusicFile &musFile: musicFiles)
    {
        if (0 == strcmp(track, musFile.id))
        {
            path += musFile.file;
            break;
        }
    }

    //if(curMusic) curMusic = NULL;
    curMusic = Mix_LoadMUS(path.c_str());
    if(curMusic) Mix_PlayMusic(curMusic, -1);
}

void AudioMixer::PlaySFX(const char *sfx)
{
    if(mixerEnabled == false || gameSettings->canPlaySFX() == false || haltedMixer == true) return;
    if (Mix_PlayChannel(-1, GetSFX(sfx), 0) < 0) SDL_LogError(1, "Could not play sound because of: %s", SDL_GetError());
}

void AudioMixer::MuteAll(bool enable){
    if(enable == true) haltedMixer = false;
    else {
        Mix_HaltMusic();
        Mix_HaltChannel(-1);
        haltedMixer = true;
    }
}