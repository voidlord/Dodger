#ifndef POWERUP_H
#define POWERUP_H

#include "Object.h"

class PowerUp : public Object {
public:
    PowerUp(int ID, float x, float y, int w, int h);
    ~PowerUp();
    
    int getID();
    
private:
    int ID;
    
};

#endif