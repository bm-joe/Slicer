#pragma once
#include <glm/glm.hpp>                      

struct fillPoint{
    public:
        glm::dvec2 point;
        bool jumpPoint = false;
        fillPoint(glm::dvec2 p, bool jp = false){
            point = p;
            jumpPoint = jp;
        }
};