#pragma once
#include <array>
#include "functions.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>                      
#include <glm/gtc/matrix_transform.hpp>     
#include <glm/gtc/quaternion.hpp>           
#include <glm/gtx/quaternion.hpp>     

class Triangle {
    public:
        Triangle(std::array<glm::dvec3, 3> v){
            verticies = v;
        };
        std::array<glm::dvec3, 3> verticies;

        double getMin(){
            return glm::min(verticies[0].y, verticies[1].y, verticies[2].y);
        }

        double getMax(){
            return glm::max(verticies[0].y, verticies[1].y, verticies[2].y);
        }

        bool pointOnPlane(double plane){
            bool c = false;
            total = 0;
            for (int v = 0; v < 3; v++){
                pointsOnPlane.at(v) = floatingEquals(verticies[v].y, plane);
                if (pointsOnPlane.at(v)){
                    verticies[v].y = plane;
                    c = true;
                    total++;
                }
            }
            return c;
        }
        //make sure pointonplane() is called before using this method
        int numOfPointsOnPlane(){
            return total;
        }
        std::array<bool, 3> pointsOnPlane;
    private:
        int total = 0;
};
