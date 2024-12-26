#ifndef MENUBUTTON_H
#define MENUBUTTON_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>

class MenuButton final
{
public:
    MenuButton(uint32_t x, uint32_t y, const std::string &name, const SDL_Renderer *renderer, const std::string icontag, const int sheetlen);
    MenuButton(const MenuButton&) = delete;
    MenuButton & operator= ( const MenuButton & ) = delete;
    MenuButton(MenuButton&& src) noexcept;
    MenuButton & operator= ( MenuButton && ) = delete;
    ~MenuButton();
    void Render(const SDL_Renderer *renderer);
    void Pressed();
    void Activate();
    void Deactivate();
private:
    bool isActive;
    int sheetLen;
    int curFrame;
    int fixedFrame;
    std::string buttonName;
    std::vector<SDL_Texture*> icons;
    SDL_Rect icon_rect;
    SDL_Texture *backgroundActive;
    SDL_Texture *background;
    SDL_Rect rect;
};

#endif // MENUBUTTON_H
