#ifndef GAME_H
#define GAME_H

#define GLEW_STATIC

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL.h>
#include <SDL_opengl.h>

#include "Shader.h"

#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "Object.h"
#include "PowerUp.h"

const int TICKS_PER_SECOND = 60;
const int SKIP_TICKS = 1000/TICKS_PER_SECOND;
const int MAX_FRAMESKIP = 10;

enum class gameState {
    inMenu,
    gameRunning,
    gamePaused
};

class Game {
public:
    Game();
    ~Game();
    
    void init();
    
    void event();
    void update();
    void render();
    
    int getScore();
    
    void spawnEnemies();
    void spawnPowerUps();
    void clearEnemies();
    void clearPowerUps();
    
    void draw(Object* t, SDL_Color color);
    void drawMainScreen();
    void drawAllObjects();
    void drawPowerUps();
    //void drawText(std::string text, int x, int y, int size, SDL_Color color);
    
    void setCamera();
    bool checkCollision(Object* A, Object* B);
    
    bool isRunning();
    gameState getState();
    void setState(gameState state);
    void pause();
    void unPause();
    void restart();
    void clean();
    void logSDLError(const char* msg);
    
private:
    SDL_Window* sdl_window;
    SDL_GLContext opengl_context;

    Shader* shader;
    GLuint VAO;

    SDL_Event events;
    SDL_Rect camera;
    
    bool running;
    bool spawnObjects;
    gameState gState;
    bool restartFlag;
    unsigned int ticks;
    int mouseX, mouseY;
    unsigned int skipCollisionTicks;
    
    Object* player;
    unsigned int score;
    unsigned int highscore;
    
    int width, height;
    int rate;
    float speed;
    bool speedUp;
    
    unsigned int immune;
    bool scoreMultiply;
    unsigned int scoreMultiplyStart;
    
    //SDL_Texture* textTexture;

    std::shared_ptr<spdlog::logger> logger;
    
    std::vector<Object> objectsVec;
    std::vector<PowerUp> powerUpsVec;
};

#endif