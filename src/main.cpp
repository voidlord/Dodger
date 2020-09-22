#include <memory>
#include <conio.h>

#include "Game.h"


int main(int argc, char* args[]) {

    
    std::unique_ptr<Game> game(new Game);
        
    unsigned int next_game_tick = SDL_GetTicks();
    int loops;
    
    while(game->isRunning()) {
        loops = 0;
        
        //this makes the game tick independent from the game fps
        while (SDL_GetTicks() > next_game_tick && loops < MAX_FRAMESKIP) {
            game->event();
            game->update();
            
            next_game_tick += SKIP_TICKS;
            loops++;
        }
        
        game->render();
    }
    
    game.reset();
        
    return 0;
}