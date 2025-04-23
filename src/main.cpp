#define SDL_MAIN_HANDLED
#include "frozenbubble.h"

int main(int, char **) {
    FrozenBubble *frozenBubble = FrozenBubble::Instance();
    return frozenBubble->RunForEver();
}
