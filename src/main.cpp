#include <SDL.h>
#include "Game.h"

int main(int, char**) {
    Game& game = *Game::getInstance();  // ★ 修改
    game.init();
    game.run();
    return 0;
}