#include "Object.h"

//construct object
Object::Object(float x, float y, int w, int h) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
}

Object::~Object() {
    
}

//set position of the object
void Object::setPos(float x, float y) {
    this->x = x;
    this->y = y;
}

//return X position of the object
float Object::getPosX() {
    return x;
}

//return Y position of the object
float Object::getPosY() {
    return y;
}

//return width of the objext
int Object::getWidth() {
    return w;
}

//return height of the object
int Object::getHeight() {
    return h;
}