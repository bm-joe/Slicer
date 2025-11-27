
#pragma once
#include <vector>
#include <glm/glm.hpp>                      
#include <memory>

struct solid;
struct polygon{
    public:
        std::vector<std::array<glm::dvec2,2>> perimeter;
        //each polygon should have one parent if it is an inline
        std::shared_ptr<polygon> parent = nullptr;
        //polygons can have multiple children
        std::vector<std::shared_ptr<polygon>> children;

        std::shared_ptr<solid> thisSolid;

        int layer = 0;
        
        double area;
        glm::dvec2 centroid;
};