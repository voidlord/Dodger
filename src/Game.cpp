#include "Game.h"

Game::Game() {
    this->init();
}

Game::~Game() {
    this->clean();
}

void Game::init() {
    std::srand((unsigned)time(0));

    try {
        logger = spdlog::basic_logger_mt("logger", "logs/log.txt");
    }
    catch (const spdlog::spdlog_ex& ex) {

    }

    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::info);
    
    width = 600;
    height = 800;
    
    logger->info("Initializing Game object.");
    
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        window = SDL_CreateWindow("Dodger", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
        if (window != 0) {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (renderer != 0) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                if (TTF_Init() != -1) {
                    logger->info("Successfully initialized Game object");
                    running = true;
                } else {
                    logger->error("Failed to init TTF");
                    logSDLError("TTF_Init");
                    running = false;
                }
            } else {
                logger->error("Failed to create renderer.");
                logSDLError("SDL_CreateRenderer");
                running = false;
            }
        } else {
            logger->error("Failed to create window.");
            logSDLError("SDL_CreateWindow");
            running = false;
        }
    } else {
        logger->error("Failed to init SDL.");
        logSDLError("SDL_Init");
        running = false;
    }    
    
    camera.x = 0;
    camera.y = 0;
    camera.w = width;
    camera.h = height;
    ticks = 0;
    rate = 0;
    speed = 0.0f;
    speedUp = false;
    immune = 0;
    scoreMultiply = false;
    scoreMultiplyStart = 0;
    spawnObjects = false;
    restartFlag = false;
    skipCollisionTicks = 0;
    
    int playerSize = 40;
    player = new Object(0, 0, playerSize, playerSize);
    score = 0;
    this->highscore = 516;
    gState = gameState::gameRunning;
    SDL_ShowCursor(false);
}

void Game::event() {
    while(SDL_PollEvent(&events)) {
        if (events.type == SDL_QUIT) {
            running = false;
        }
        if (events.type == SDL_KEYDOWN) {
            //if ESC is pressed pause the game
            if (events.key.keysym.sym == SDLK_ESCAPE) {
                if ((gState == gameState::gamePaused) && (spawnObjects == true)) {
                    SDL_ShowCursor(false);
                    SDL_WarpMouseInWindow(window, player->getPosX() + player->getWidth()/2, player->getPosY() + player->getHeight()/2);
                    skipCollisionTicks = 1;
                    gState = gameState::gameRunning;
                } else if ((gState == gameState::gameRunning) && (spawnObjects == true)) { //otherwise unpause it
                    SDL_ShowCursor(true);
                    gState = gameState::gamePaused;
                }
            }
            if (events.key.keysym.sym == SDLK_SPACE) {
                speedUp = true;
            }
        }
        if (events.type == SDL_KEYUP) {
            if (events.key.keysym.sym == SDLK_SPACE) {
                speedUp = false;
            }
        }
        if (gState == gameState::inMenu) {
            if ((events.type == SDL_MOUSEBUTTONDOWN) && (events.button.button == SDL_BUTTON_LEFT)) {
                restartFlag = true;
            }
        }
    }
}

void Game::update() {
    if (restartFlag) {
        this->restart();
    }
    
    SDL_GetMouseState(&mouseX, &mouseY);
    
    if (gState == gameState::gameRunning) {
        //position the player on the mouse position
        player->setPos(mouseX - player->getWidth()/2, mouseY - player->getHeight()/2);
        
        //fix player position inside visible area
        if (player->getPosX() < 0) {
            player->setPos(0, player->getPosY());
        }
        if (player->getPosY() < 0) {
            player->setPos(player->getPosX(), 0);
        }
        if (player->getPosX() > width - player->getWidth()) {
            player->setPos(width - player->getWidth(), player->getPosY());
        }
        if (player->getPosY() > height - player->getHeight()) {
            player->setPos(player->getPosX(), height - player->getHeight());
        }
        
        //if moved on the start square position, then start the game
        if (mouseY > height - (height/4)) {
            spawnObjects = true;
        }
        
        //change speed based on points
        if (score < 50) {
            speed = 1.5f;
        } else if (score < 100) {
            speed = 2.0f;          
        } else if (score < 200) {
            speed = 2.5f;          
        } else if (score < 300) {
            speed = 3.5f;          
        } else if (score < 500) {
            speed = 4.5f;           
        } else if (score < 750) {
            speed = 5.5f;          
        } else if (score < 1000) {
            speed = 6.5f;          
        } else if (score < 1500) {
            speed = 8.5f;          
        } else if (score < 2000) {
            speed = 10.0f;         
        } else if (score < 5000) {
            speed = 11.5f;          
        } else if (score < 10000) {
            speed = 13.0f;         
        } else if (score < 25000) {
            speed = 15.0f;          
        } else if (score < 50000) {
            speed = 17.0f;        
        } else if (score < 100000) {
            speed = 20.0f;  
        }
        
        //increase speed while boost is true
        if (speedUp == true) {
            speed += 2.0f;
        }
        
        //change spawnrate based on speed
        rate = speed * 2.0;
        
        //spawn objects while the flag is true
        if (spawnObjects) {
            for (int i = 1; i <= rate; i++) {
                spawnEnemies();
                spawnPowerUps();
            }
        }
        
        std::vector<PowerUp>::iterator it1 = powerUpsVec.begin();
        std::vector<Object>::iterator it2 = objectsVec.begin();
        
        //iterate through all powerups, erasing ones off-screen, and moving the ones visible
        while (it1 != powerUpsVec.end()) {
            if (it1->getPosY() > height) {
                it1 = powerUpsVec.erase(it1);
            } else {
                it1->setPos(it1->getPosX(), it1->getPosY() + speed);
                
                it1++;
            }
        }
        
        //iterate through all green squares in the game, erasing ones off-screen, and moving the ones visible
        while (it2 != objectsVec.end()) {
            if (it2->getPosY() > height) {
                it2 = objectsVec.erase(it2);
            } else {
                it2->setPos(it2->getPosX(), it2->getPosY() + speed);
                
                it2++;
            }
        }
        
        if (skipCollisionTicks == 0) {
            //update game when player touches a green square, even though he was told to avoid them
            for (it2 = objectsVec.begin(); it2 != objectsVec.end(); it2++) {
                if (checkCollision(player, &(*it2))) {
                    //make the immune effect have an effect
                    if (immune > 0) {
                        immune--;
                        objectsVec.erase(it2);
                        break;
                    } else {
                        gState = gameState::inMenu;
                        spawnObjects = false;
                        SDL_ShowCursor(true);
                    }
                }
            }
            
            //update game when player touches a powerup
            for (it1 = powerUpsVec.begin(); it1 != powerUpsVec.end(); it1++) {
                if (checkCollision(player, &(*it1))) {
                    if (it1->getID() == 1 && immune < 3) {
                        immune++;
                        powerUpsVec.erase(it1);
                        break;
                    } else if (it1->getID() == 2) {
                        scoreMultiply = true;
                        scoreMultiplyStart = ticks;
                        powerUpsVec.erase(it1);
                        break;
                    }
                }
            }
        }
        if (skipCollisionTicks > 0) {
            skipCollisionTicks--;
        }
        
        //expire score multiply after 15 seconds
        if (ticks - scoreMultiplyStart >= TICKS_PER_SECOND * 15) {
            scoreMultiply = false;
        }
        
        ticks++;
    }
}

void Game::render() {
    setCamera();
    
    //clean current frame
    SDL_SetRenderDrawColor(renderer, 149, 148, 139, 255);
    SDL_RenderClear(renderer);
    
    drawPowerUps();
    drawAllObjects();
    
    if (immune > 0) {
        if (immune == 1) {
            draw(player, {176, 196, 222});
        } else if (immune == 2) {
            draw(player, {143, 177, 223});
        } else if (immune == 3) {
            draw(player, {109, 155, 223});
        }
    } else {
        draw(player, {255, 255, 255});
    }
    
    if (gState != gameState::inMenu) {
        if (!spawnObjects) {
            drawMainScreen();
        } else {
            drawText("Score: " + std::to_string(score), 0, 0, 36, {255, 255, 255});
        }
        if (immune > 0) {
            SDL_Rect rect;
            
            rect.x = width - 60;
            rect.y = 10;
            rect.w = 20;
            rect.h = 20;
            
            SDL_SetRenderDrawColor(renderer, 70, 130, 180, 255);
            
            SDL_RenderFillRect(renderer, &rect);
            
            drawText(std::to_string(immune), width - 54, 12, 20, {255, 255, 255});
        }
        if (scoreMultiply) {
            SDL_Rect rect;
            
            rect.x = width - 30;
            rect.y = 10;
            rect.w = 20;
            rect.h = 20;
            
            SDL_SetRenderDrawColor(renderer, 255, 255, 51, 255);
            
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    
    if (gState == gameState::inMenu) {
        if (score > highscore) {
            drawText("New Highscore!", width*0.33, height*0.4, 48, {255, 255, 255});
        } else if (score <= highscore) {
            drawText("Highscore: " + std::to_string(highscore), width*0.34, height*0.4, 40, {255, 255, 255});
        }
        
        drawText("Score: " + std::to_string(score), width*0.36, height*0.5, 64, {255, 255, 255});
        drawText("Click to play again", width*0.22, height*0.75, 58, {255, 255, 255});
    }
    
    if (gState == gameState::gamePaused) {
        drawText("Paused", width*0.41, height*0.75, 58, {255, 255, 255});
    }

    SDL_RenderPresent(renderer);
}

int Game::getScore() {
    return score;
}

void Game::spawnEnemies() {
    int size = 50;
    static unsigned int localTick = 1;
    static int randomNum = 10;
    
    if (localTick % randomNum == 0) {
        Object tmpObject(std::rand() % (width - size), -size, size, size);
        objectsVec.push_back(tmpObject);
        localTick = 1;
        //randomNum is the Y position of the next object
        randomNum = (std::rand() % 90) + 60;
        
        if (scoreMultiply) {
            score += 2;
        } else {
            score++;
        }
    }
    localTick++;
}

void Game::spawnPowerUps() {
    int size = 35;
    static unsigned int localTick = 1;
    static int randomNum = 1000;
    
    if (localTick % randomNum == 0) {
        PowerUp tmpPowerUp = PowerUp((std::rand() % 2) + 1, std::rand() % (width - size), -size, size, size);
        
        bool isCollided = false;
        std::vector<Object>::iterator it;
        
        while (true) {
            isCollided = false;
            for (it = objectsVec.begin(); it != objectsVec.end(); it++) {
                if (checkCollision(&tmpPowerUp, &(*it))) {
                    isCollided = true;
                }
            }
            if (isCollided == false) {
                break;
            } else {
                tmpPowerUp.setPos(std::rand() % (width - size), tmpPowerUp.getPosY());
            }
        }
        powerUpsVec.push_back(tmpPowerUp);
        localTick = 1;
        //randomNum is the Y position of the next powerup
        randomNum = (std::rand() % 2500) + 3500;
    }
    
    localTick++;
}

void Game::clearEnemies() {
    objectsVec.clear();
}

void Game::clearPowerUps() {
    powerUpsVec.clear();
}

void Game::draw(Object* e, SDL_Color color) {
    SDL_Rect rect;
    
    rect.x = e->getPosX();
    rect.y = e->getPosY();
    rect.w = e->getWidth();
    rect.h = e->getHeight();
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    
    SDL_RenderFillRect(renderer, &rect);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

void Game::drawMainScreen() {
    SDL_Rect rect;
    
    rect.x = 0;
    rect.y = height - (height/4);
    rect.w = width;
    rect.h = height - (height/4);
    
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);

    drawText("Move here to start!", width*0.18, height*0.8, 64, {255, 255, 255});
    drawText("Avoid green squares", width*0.30, height*0.1, 40, {255, 255, 255});
    drawText("Blue squares give one-use immunity", width*0.15, height*0.2, 40, {255, 255, 255});
    drawText("Yellow squares give 2x score for 15s", width*0.13, height*0.3, 40, {255, 255, 255});
    drawText("Use mouse to move around", width*0.24, height*0.5, 40, {255, 255, 255});
    drawText("Hold space to gain speed boost", width*0.20, height*0.6, 40, {255, 255, 255});
}

void Game::drawAllObjects() {
    std::vector<Object>::iterator it;
    for (it = objectsVec.begin(); it != objectsVec.end(); it++) {
        draw(&(*it), {65, 146, 75});
    }
}

void Game::drawPowerUps() {
    std::vector<PowerUp>::iterator it;
    for (it = powerUpsVec.begin(); it != powerUpsVec.end(); it++) {
        if (it->getID() == 1) {
            draw(&(*it), {70, 130, 180});
        } else if (it->getID() == 2) {
            draw(&(*it), {255, 255, 51});
        }
    }
}

void Game::drawText(std::string text, int x, int y, int size, SDL_Color color) {
    TTF_Font* font = TTF_OpenFont("assets/font.ttf", size);
    
    if (font == NULL) {
        logger->error("Failed to load font.");
        logSDLError("Failed to load font.");
    }
    
    SDL_Surface* surfaceMessage = TTF_RenderText_Blended(font, text.c_str(), color);
    
    textTexture = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    
    drawTexture(textTexture, x, y);
    
    SDL_FreeSurface(surfaceMessage);
    surfaceMessage = NULL;
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
}

void Game::drawTexture(SDL_Texture* tex, int x, int y) {
    SDL_Rect tRect;
    tRect.x = x;
    tRect.y = y;
    
    SDL_QueryTexture(tex, NULL, NULL, &tRect.w, &tRect.h);
    SDL_RenderCopy(renderer, tex, NULL, &tRect);
}

void Game::setCamera() {
    camera.x = 0;
    camera.y = 0;
}

bool Game::checkCollision(Object* A, Object* B) {
    if((A->getPosX() < (B->getPosX() + B->getWidth())) && ((A->getPosX() + A->getWidth()) > B->getPosX()) 
    && (A->getPosY() < (B->getPosY() + B->getHeight())) && ((A->getPosY() + A->getHeight()) > B->getPosY())) {
        return true;
    } else {
        return false;
    }
}

bool Game::isRunning() {
    return running;
}

gameState Game::getState() {
    return gState;
}

void Game::setState(gameState state) {
    gState = state;
}

void Game::pause() {
    this->setState(gameState::gamePaused);
}

void Game::unPause() {
    this->setState(gameState::gameRunning);
}

void Game::restart() {
    logger->info("Preparing to restart game.");
    restartFlag = false;
    if (score > highscore) {
        highscore = score;
    }
    ticks = 0;
    rate = 2.0;
    score = 0;
    speed = 1.0f;
    immune = 0;
    scoreMultiply = false;
    spawnObjects = false;
    
    this->clearEnemies();
    this->clearPowerUps();
    
    SDL_ShowCursor(false);
    gState = gameState::gameRunning;
    logger->info("Restart successful");
}

void Game::clean() {
    logger->info("Cleaning up Game object");
    delete player;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(textTexture);
    TTF_Quit();
    SDL_Quit();
    logger->info("Cleaning finished.");
}

void Game::logSDLError(const char* msg) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDLError", msg, window);
}