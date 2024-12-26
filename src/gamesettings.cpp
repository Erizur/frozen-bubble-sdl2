#include "gamesettings.h"

GameSettings *GameSettings::ptrInstance = new GameSettings(); 

GameSettings::~GameSettings() {
    iniparser_freedict(optDict);
}

void GameSettings::Dispose() {
    SaveSettings();
    this->~GameSettings();
}

void GameSettings::CreateDefaultSettings()
{
    FILE *setFile;
    char setPath[256];
    int rval;
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

    rval = iniparser_set(dict, "GFX", NULL);
    if(rval < 0) {
        SDL_LogWarn(1, "Could not write GFX header to ini file!");
        goto finish;
    }

    rval = iniparser_set(dict, "GFX:Quality", "1");
    if(rval < 0) {
        SDL_LogWarn(1, "Could not write GFX:Quality to ini file!");
        goto finish;
    }

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
    if (gfxQuality > 3 || gfxQuality < 1) gfxQuality = 3;
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

    //update ini file set
    iniparser_set(optDict, option, value);
}