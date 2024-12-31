#ifndef TEXTURESHADER_H
#define TEXTURESHADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

class ShadedTexture
{
    public:
        //Initializes variables
        ShadedTexture();

        //Deallocates memory
        ~ShadedTexture();

        //Loads image at specified path
        bool LoadFromFile(const char *path );
        
        //Loads image into pixel buffer
        bool LoadPixelsFromFile(const char *path );

        //Creates image from preloaded pixels
        bool LoadFromPixels();

        #if defined(SDL_TTF_MAJOR_VERSION)
        //Creates image from font string
        bool LoadFromText( std::string textureText, SDL_Color textColor );
        #endif

        //Deallocates texture
        void Release();

        //Set color modulation
        void SetColor( Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha );

        //Set blending
        void SetBlendMode( SDL_BlendMode blending );

        //Set alpha modulation
        void SetAlpha( Uint8 alpha );
        
        //Renders texture at given point
        void Render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

        //Gets image dimensions
        SDL_Point GetSize();

        //Pixel accessors
        Uint32* GetPixels32();
        Uint32 GetPitch32();
        Uint32 MapRGBA( Uint8 r, Uint8 g, Uint8 b, Uint8 a );

    private:
        //The actual hardware texture
        SDL_Texture* mTexture;

        //Surface pixels
        SDL_Surface* mSurfacePixels;
        
        //Image dimensions
        int mWidth;
        int mHeight;
};

#endif // TEXTURESHADER_H