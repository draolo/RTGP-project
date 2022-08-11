#pragma once

#include <utils/gameObject.h>

class Bullet:public GameObject 
{
private:
    /* data */
public:
    glm::vec3 shoot_pos;
    Bullet(glm::vec3 pos, glm::vec3 s, glm::vec3 r, Model* m):GameObject(pos,s,r,m),shoot_pos(pos) {
        rb=nullptr;
    }
};


