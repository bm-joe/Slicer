#pragma once
#include <memory>
#include "polygon.h"
struct solid{
    public:
        std::shared_ptr<polygon> parentPolygon;
        solid(std::shared_ptr<polygon> p){
            parentPolygon = p;
        }

};