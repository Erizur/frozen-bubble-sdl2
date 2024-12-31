#include "textureshader.h"

bool ShadedTexture::LoadPixelsFromFile(const char *path)
{
    //Free preexisting assets
    free();

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        //Convert surface to display format
        mSurfacePixels = SDL_ConvertSurfaceFormat( loadedSurface, SDL_GetWindowPixelFormat( gWindow ), 0 );
        if( mSurfacePixels == NULL )
        {
            printf( "Unable to convert loaded surface to display format! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = mSurfacePixels->w;
            mHeight = mSurfacePixels->h;
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return mSurfacePixels != NULL;
}