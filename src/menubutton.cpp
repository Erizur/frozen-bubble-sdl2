#include "menubutton.h"
#include <SDL2/SDL_image.h>
#include <utility>

MenuButton::MenuButton(uint32_t x, uint32_t y, const std::string &name, const SDL_Renderer *renderer, const std::string icontag, const int sheetlen)
    : isActive(false)
{
    std::string pathBase = std::string(DATA_DIR) + "/gfx/menu/anims/" + icontag + "_";
    if (name == "graphics") pathBase = std::string(DATA_DIR) + "/gfx/menu/anims/" + "gfx-l2" + "_";
    std::string pathExt = ".png";

    for (int i = 1; i <= sheetlen + 1; i++)
    {
        std::string fileName = pathBase;
        char fileIndex[16];

        sprintf(fileIndex, "%04d", i);
        fileName += fileIndex;
        fileName += pathExt;
        icons.push_back(IMG_LoadTexture(const_cast<SDL_Renderer*>(renderer), fileName.c_str()));
    }

    std::string backgroundActive_path = std::string(DATA_DIR) + "/gfx/menu/txt_" + name + "_over.png";
    backgroundActive= IMG_LoadTexture(const_cast<SDL_Renderer*>(renderer), backgroundActive_path.c_str());
    std::string background_path = std::string(DATA_DIR) + "/gfx/menu/txt_" + name + "_off.png";
    background= IMG_LoadTexture(const_cast<SDL_Renderer*>(renderer), background_path.c_str());
    rect.x = x;
    rect.y = y;
    rect.w = 202;
    rect.h = 46;
    icon_rect.x = 248;
    icon_rect.y = y + 8;
    icon_rect.w = 40;
    icon_rect.h = 30;

    curFrame = 0;
    sheetLen = sheetlen;
}

MenuButton::MenuButton(MenuButton &&src) noexcept
    : isActive(std::move(src.isActive)),
    sheetLen(std::move(src.sheetLen)),
    curFrame(std::move(src.curFrame)),
    icons(std::move(src.icons)),
    icon_rect(std::move(src.icon_rect)),
    backgroundActive(std::exchange(src.backgroundActive, nullptr)),
    background(std::exchange(src.background, nullptr)),
    rect(std::move(src.rect))
{
}

MenuButton::~MenuButton()
{
    for (int i = 0; i < icons.size(); i++) SDL_DestroyTexture(icons[i]);
    icons.clear();
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(backgroundActive);
}

void MenuButton::Render(const SDL_Renderer *renderer)
{
    if(isActive){
        curFrame++;
        fixedFrame = curFrame / 2;
        if(fixedFrame >= sheetLen) fixedFrame = curFrame = 0;
        SDL_SetTextureAlphaMod(icons[fixedFrame], 255);
    }
    else {
        SDL_SetTextureAlphaMod(icons[fixedFrame], 100);
    }
    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), isActive?backgroundActive:background, nullptr, &rect);
    SDL_RenderCopy(const_cast<SDL_Renderer*>(renderer), icons[fixedFrame], nullptr, &icon_rect);
}

void MenuButton::Activate()
{
    isActive = true;
}

void MenuButton::Deactivate()
{
    isActive = false;
}
