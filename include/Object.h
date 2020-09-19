#ifndef OBJECT_H
#define OBJECT_H

class Object {
public:
    Object(float x, float y, int w, int h);
    ~Object();
    
    void setPos(float x, float y);
    
    float getPosX();
    float getPosY();
    int getWidth();
    int getHeight();
    
private:
    float x, y;
    int w, h;
};

#endif