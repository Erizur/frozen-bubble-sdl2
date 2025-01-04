#ifndef BUBBLEGAME_H
#define BUBBLEGAME_H

#include <SDL2/SDL.h>
#include "shaderstuff.h"

#include <vector>

class BubbleGame final
{
public:
    BubbleGame(const SDL_Renderer *renderer);
    BubbleGame(const BubbleGame&) = delete;
    ~BubbleGame();

    void Render(void);
    void NewGame();
private:
    const SDL_Renderer *renderer;
    SDL_Texture *background;
};

#endif // BUBBLEGAME_H