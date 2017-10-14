#include "game/Game.hpp"
#include "engine/Engine.hpp"

Game game = Game();

int main() {
    //Engine engine(1920, 1080);
    Engine engine(1280, 720);

    return engine.initialize(&game);
}
