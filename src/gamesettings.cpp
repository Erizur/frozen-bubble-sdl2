#include "gamesettings.h"

GameSettings *GameSettings::ptrInstance = new GameSettings(); 

GameSettings::~GameSettings() {
    iniparser_freedict(optDict);
}

void GameSettings::Dispose() {
    SaveSettings();
    this->~GameSettings();
}

int WriteToIni(dictionary *ini, const char *key, const char *value){
    int a = iniparser_set(ini, key, value);
    if (a != 0) {
        SDL_LogWarn(1, "Could not write %s %s to ini file!", key, value == NULL ? " header" : "");
    }
    return a;
}

#define EvalIniResult(a,ini,k,v) a = WriteToIni(ini, k, v)

void GameSettings::CreateDefaultSettings()
{
    FILE *setFile;
    char setPath[256];
    int rval = 0;
    strcpy(setPath, prefPath);
    strcat(setPath, "settings.ini");
    if((setFile = fopen(setPath, "w")) == NULL)
    {
        SDL_LogError(1, "Could not create default save file. Exiting.");
        throw;
    }
    fclose(setFile);

    dictionary *dict;
    dict = iniparser_load(setPath);

    while (rval == 0)
    {
        EvalIniResult(rval, dict, "GFX", NULL);
        EvalIniResult(rval, dict, "GFX:Quality", "1");
        EvalIniResult(rval, dict, "GFX:LinearScaling", "false");
        EvalIniResult(rval, dict, "GFX:Fullscreen", "false");
        EvalIniResult(rval, dict, "GFX:WindowWidth", "640");
        EvalIniResult(rval, dict, "GFX:WindowHeight", "480");

        EvalIniResult(rval, dict, "Sound", NULL);
        EvalIniResult(rval, dict, "Sound:EnableMusic", "true");
        EvalIniResult(rval, dict, "Sound:EnableSFX", "true");
        EvalIniResult(rval, dict, "Sound:ClassicAF", "false");

        //break while
        rval = 1;
    }

    if (rval < 0) goto finish;

    if((setFile = fopen(setPath, "w+")) == NULL)
    {
        SDL_LogError(1, "Could not create default save file. Exiting.");
        throw;
    }
    iniparser_dump_ini(dict, setFile);
    fclose(setFile);
finish:
    iniparser_freedict(dict);
}

void GameSettings::ReadSettings()
{
    char setPath[256];
    strcpy(setPath, prefPath);
    strcat(setPath, "settings.ini");

    //int rval;

    optDict = iniparser_load(setPath);

    while (optDict == NULL)
    {
        SDL_LogWarn(1, "Settings file failed to load (or doesn't exist). Creating default fallback...");
        CreateDefaultSettings();
        optDict = iniparser_load(setPath);
    }

    gfxQuality = iniparser_getint(optDict, "GFX:Quality", 1);
    linearScaling = iniparser_getboolean(optDict, "GFX:LinearScaling", false);
    useFullscreen = iniparser_getboolean(optDict, "GFX:Fullscreen", false);
    windowWidth = iniparser_getint(optDict, "GFX:WindowWidth", 640);
    windowHeight = iniparser_getint(optDict, "GFX:WindowHeight", 480);
    if (gfxQuality > 3 || gfxQuality < 1) gfxQuality = 3;
    if (windowWidth < 640 || windowWidth > 9999) windowWidth = 640;
    if (windowHeight < 480 || windowWidth > 9999) windowHeight = 480;

    playMusic = iniparser_getboolean(optDict, "Sound:EnableMusic", true);
    playSfx = iniparser_getboolean(optDict, "Sound:EnableSFX", true);
    classicSound = iniparser_getboolean(optDict, "Sound:ClassicAF", false);
}

void GameSettings::SaveSettings()
{
    FILE *setFile;
    char setPath[256];
    strcpy(setPath, prefPath);
    strcat(setPath, "settings.ini");

    if((setFile = fopen(setPath, "w+")) == NULL)
    {
        SDL_LogWarn(1, "Could not save to the save file!");
        return;
    }
    iniparser_dump_ini(optDict, setFile);
    fclose(setFile);
}

void GameSettings::SetValue(const char* option, const char* value)
{
    //update runtime options
    if (strcmp(option, "GFX:Quality") == 0) {
        if (gfxQuality == 1) gfxQuality = 3;
        else gfxQuality--;

        // gfxQuality needs a hot reload
        iniparser_set(optDict, option, std::to_string(gfxQuality).c_str());
        return;
    }
    else if (strcmp(option, "GFX:Fullscreen") == 0) {
        useFullscreen = !useFullscreen;
        iniparser_set(optDict, option, useFullscreen ? "true" : "false");
        return;
    }

    //update ini file set
    iniparser_set(optDict, option, value);
}