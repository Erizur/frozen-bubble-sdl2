#ifndef SHADERSTUFF_H
#define SHADERSTUFF_H

#define _USE_MATH_DEFINES
#if WIN32
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#endif

#include <iconv.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>

struct TextureEx {
    SDL_Renderer *rend;
    SDL_Surface *sfc;
    Uint32 *pixels = nullptr;
    Uint32 format;
    SDL_PixelFormat pixelFormat;
    int w, h, pitch;

    void LoadTextureData(SDL_Renderer* renderer, const char* path){
        rend = renderer;
        sfc = IMG_Load(path); 
        if(!sfc) SDL_LogWarn(1, "Failed to init SDL_Surface");
    };

    void LoadFromSurface(SDL_Surface *img, SDL_Renderer* renderer){
        rend = renderer;
        sfc = SDL_CreateRGBSurface(0, img->w, img->h, 32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);
        SDL_SetSurfaceBlendMode(sfc, SDL_BLENDMODE_BLEND);
        if(!sfc || !img) SDL_LogWarn(1, "Failed to init SDL_Surface");
        SDL_BlitSurface(img, NULL, sfc, NULL);
    };

    void LoadEmptyAndApply(SDL_Rect *sz, SDL_Renderer* renderer, const char* path){
        rend = renderer;
        sfc = SDL_CreateRGBSurface(0, sz->w, sz->h, 32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);
        SDL_SetSurfaceBlendMode(sfc, SDL_BLENDMODE_BLEND);
        SDL_Surface *img = IMG_Load(path); 
        if(!sfc || !img) SDL_LogWarn(1, "Failed to init SDL_Surface");
        SDL_BlitSurface(img, new SDL_Rect{0, 0, img->w, img->h}, sfc, new SDL_Rect{sz->x, sz->y, img->w, img->h});
        SDL_FreeSurface(img);
    };

    SDL_Texture *OutputTexture() {
        return SDL_CreateTextureFromSurface(rend, sfc);
    };
};

void set_pixel(SDL_Surface *s, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void get_pixel(SDL_Surface *s, int x, int y, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a);

void myLockSurface(SDL_Surface *s);
void myUnlockSurface(SDL_Surface *s);
void synchro_before(SDL_Surface *s);
void synchro_after(SDL_Surface *s);

void fb__out_of_memory(void);

/************************** Graphical effects ****************************/

/*
 * Features:
 *
 *   - plasma-ordered fill (with top-bottom and/or left-right mirrored plasma's)
 *   - random points
 *   - horizontal blinds
 *   - vertical blinds
 *   - center=>edge circle
 *   - up=>down bars
 *   - top-left=>bottom-right squares
 *
 */

/* -------------- Double Store ------------------ */
void copy_line(int l, SDL_Surface *s, SDL_Surface *img);
void copy_column(int c, SDL_Surface *s, SDL_Surface *img);

void store_effect(SDL_Surface *s, SDL_Surface *img);

/* -------------- Bars ------------------ */

void bars_effect(SDL_Surface *s, SDL_Surface *img);

/* -------------- Squares ------------------ */
int fillrect(int i, int j, SDL_Surface *s, SDL_Surface *img, int bpp, const int squares_size);

void squares_effect(SDL_Surface *s, SDL_Surface *img);

/* -------------- Circle ------------------ */

void circle_init(void);

void circle_effect(SDL_Surface *s, SDL_Surface *img);

/* -------------- Plasma ------------------ */

void plasma_init(char *datapath);

void plasma_effect(SDL_Surface *s, SDL_Surface *img);

void shrink_(SDL_Surface *dest, SDL_Surface *orig, int xpos, int ypos, SDL_Rect *orig_rect, int factor);

void rotate_nearest_(SDL_Surface *dest, SDL_Surface *orig, double angle);

void rotate_bilinear_(SDL_Surface *dest, SDL_Surface *orig, double angle);

void rotate_bicubic_(SDL_Surface *dest, SDL_Surface *orig, double angle);

void flipflop_(SDL_Surface *dest, SDL_Surface *orig, int offset);

// moves an highlighted spot over the surface (shape of ellypse, depending on width and hight)
void enlighten_(SDL_Surface *dest, SDL_Surface *orig, int offset);

void stretch_(SDL_Surface *dest, SDL_Surface *orig, int offset);

void tilt_(SDL_Surface *dest, SDL_Surface *orig, int offset);

void points_(SDL_Surface *dest, SDL_Surface *orig, SDL_Surface *mask);

void waterize_(SDL_Surface *dest, SDL_Surface *orig, int offset);

void brokentv_(SDL_Surface *dest, SDL_Surface *orig, int offset);

void alphaize_(SDL_Surface *surf);

void pixelize_(SDL_Surface *dest, SDL_Surface *orig);

void blacken_(SDL_Surface *surf, int step);

void overlook_(SDL_Surface *dest, SDL_Surface *orig, int step, int pivot);

void snow_(SDL_Surface *dest, SDL_Surface *orig);

void draw_line_(SDL_Surface *surface, int x1, int y1, int x2, int y2, SDL_Color *color);

void init_effects(char* path);

#endif // SHADERSTUFF_H