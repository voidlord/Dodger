#include "PowerUp.h"

//construct powerup using object constructor
PowerUp::PowerUp(int ID, float x, float y, int w, int h) : Object(x, y, w, h) {
    this->ID = ID;
}

PowerUp::~PowerUp() {
    
}

//return ID of the powerup
int PowerUp::getID() {
    return ID;
}