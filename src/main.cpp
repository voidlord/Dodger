#include <memory>
#include <conio.h>

#include <easylogging++.h>

#include "Game.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* args[]) {
    START_EASYLOGGINGPP(argc, args);
    //flag to make sure it always writes to new lines
    el::Loggers::addFlag(el::LoggingFlag::NewLineForContainer);
    
    std::unique_ptr<Game> game(new Game);
    //initialize game
    game->init();
        
    unsigned int next_game_tick = SDL_GetTicks();
    int loops;
    
    //gameloop
    while(game->isRunning()) {
        loops = 0;
        
        //this makes the game tick independent from the game fps
        while (SDL_GetTicks() > next_game_tick && loops < MAX_FRAMESKIP) {
            game->event();
            game->update();
            
            next_game_tick += SKIP_TICKS;
            loops++;
        }
        
        //render frame
        game->render();
    }
    
    //clean up the game
    game->clean();
    
    //delete unique_ptr
    game.reset();
        
    return 0;
}