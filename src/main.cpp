#include <SDL.h>
#include "Game.h"

int main(int, char**) {
    Game game;
    game.init();
    game.run();
    return 0;
}