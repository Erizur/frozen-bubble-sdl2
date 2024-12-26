#include "gamesettings.h"
#include <iniparser.h>

GameSettings *GameSettings::ptrInstance = new GameSettings(); 

void GameSettings::CreateDefaultSettings()
{
    FILE *setFile;
    char setPath[256];
    int rval;
    strcpy(setPath, prefPath);
    strcat(setPath, "settings.ini");
    if((setFile = fopen(setPath, "w")) == NULL)
    {
        SDL_Log("FATAL: Could not create default save file. Exiting.");
        throw;
    }
    fclose(setFile);

    dictionary *dict;
    dict = iniparser_load(setPath);

    rval = iniparser_set(dict, "GFX", NULL);
    if(rval < 0) {
        SDL_Log("Could not write GFX header to ini file!");
        goto finish;
    }

    rval = iniparser_set(dict, "GFX:Quality", "1");
    if(rval < 0) {
        SDL_Log("Could not write GFX header to ini file!");
        goto finish;
    }

    if((setFile = fopen(setPath, "w+")) == NULL)
    {
        SDL_Log("FATAL: Could not create default save file. Exiting.");
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

    dictionary *dict;
    //int rval;

    dict = iniparser_load(setPath);

    while (dict == NULL)
    {
        SDL_Log("WARNING: Settings file failed to load (or doesn't exist). Creating default fallback...");
        CreateDefaultSettings();
        dict = iniparser_load(setPath);
    }

    gfxQuality = iniparser_getint(dict, "GFX:Quality", 1);
    if (gfxQuality > 3 || gfxQuality < 1) gfxQuality = 3;
    iniparser_freedict(dict);
}

void GameSettings::GetValue()
{
    
}