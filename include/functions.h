#pragma once
#include <glm/glm.hpp>                      
#include <array>
#include <vector>
#include <memory>
#include <iostream>
#include <glm/gtc/quaternion.hpp>           

#include "fillpoint.h"
#include "polygon.h"

bool floatingEquals(double a, double b, double epsilon = 1e-8   ){
    return fabs(a-b) < epsilon; 
}

static inline double sqdist(const glm::dvec2 &a, const glm::dvec2 &b){
    glm::dvec2 c = a - b;
    return c.x * c.x + c.y * c.y;
}

bool segmentsTouchingTips(const std::array<glm::dvec2, 2> &seg1, const std::array<glm::dvec2, 2> &seg2, double epsilon = 1e-6){
    double sqrEpsilon = epsilon * epsilon;

    for (int i = 0; i < 2; i++){
        for (int j = 0; j < 2; j++){
            if (sqdist(seg1[i], seg2[j])<=sqrEpsilon){
                return true;
            }
        }
    }
    return false;
    
    // glm::dvec2 p1, p2, p3, p4;
    // p1 = (seg1[0] - seg2[0]);
    // p2 = (seg1[0] - seg2[1]);
    // p3 = (seg1[1] - seg2[0]);
    // p4 = (seg1[1] - seg2[1]);

    // return(
    //     sqrt(p1.x + p1.y) < epsilon ||
    //     sqrt(p2.x + p2.y) < epsilon ||
    //     sqrt(p3.x + p3.y) < epsilon ||
    //     sqrt(p4.x + p4.y) < epsilon 
    // );
    
}

//returns a bool stating if the pitch of a quat is outside the desired clamp range 
bool clampQuat(const glm::quat& q, float minPitch, float maxPitch){
    glm::vec3 euler = glm::eulerAngles(q);
    if (euler.x > minPitch && euler.x < maxPitch){
        // std::cout << minPitch << euler.x << maxPitch << std::endl;
        return true;
    }else{
        return false;
    }
};




//sorting algorithm
void swap(std::vector<fillPoint> &v, int i1, int i2){
    fillPoint temp = v[i1];
    v[i1] = v[i2];
    v[i2] = temp;
}

void insertionSort(std::vector<fillPoint> &v){
    for (int i = 1; i < v.size(); i++){
        int j = i;
        while(j > 0 && v[j-1].point.x > v[j].point.x){
            swap(v, j, j-1);
            j--;
        }
    }
}

double calculatePolygonArea(const polygon *p){
    //shoelace formula
    double top = 0.0;
    double bottom = 0.0;
    for (int i =0 ; i < p->perimeter.size()-1; i++){
        top += (p->perimeter.at(i)[0].x * p->perimeter.at(i+1)[0].y);
        bottom += (p->perimeter.at(i)[0].y * p->perimeter.at(i+1)[0].x);
    }
    top += (p->perimeter.at(p->perimeter.size()-1)[0].x * p->perimeter.at(0)[0].y);
    bottom += (p->perimeter.at(p->perimeter.size()-1)[0].y * p->perimeter.at(0)[0].x);
    
    // std::cout<<"calculated area"<<std::endl;
    //signed area formula for centroid calculations
    return (top-bottom)/2.0;

}

glm::dvec2 calculatePolygonCentroid(const polygon *p){
    double x = 0.0;
    double y = 0.0;
    // Cx = (1/6A)Sigma_points-1((xi + xi+1)(xi*yi+1 - xi+1*yi))
    // Cy = (1/6A)Sigma_points((yi + yi+1)(xi*yi+1 - xi+1*yi))
    for (int i = 0; i < p->perimeter.size()-1; i++){
        const glm::dvec2 &p1 = p->perimeter.at(i)[0];
        const glm::dvec2 &p2 = p->perimeter.at(i+1)[0];

        x += (p1.x + p2.x) * (p1.x * p2.y - p2.x * p1.y);
        y += (p1.y + p2.y) * (p1.x * p2.y - p2.x * p1.y);
    }
    //wraparound
    const glm::dvec2 &p1 = p->perimeter.at(p->perimeter.size()-1)[0];
    const glm::dvec2 &p2 = p->perimeter.at(0)[0];

    x += (p1.x + p2.x) * (p1.x * p2.y - p2.x * p1.y);
    y += (p1.y + p2.y) * (p1.x * p2.y - p2.x * p1.y);

    x *= (1.0/(6.0*p->area));
    y *= (1.0/(6.0*p->area));
    return glm::dvec2(x,y);
}

std::array<double,2> calculatePolygonYBounds(const polygon *p){
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::min();
    for( std::array<glm::dvec2, 2> s : p->perimeter){
        if (s[0].y < min){
            min = s[0].y;
        }
        if (s[0].y > max){
            max = s[0].y;
        }
    }
    return {min,max};
}


bool isIntersectingSegments(const std::array<glm::dvec2, 2> a,const std::array<glm::dvec2, 2> b ){
    //defining slopes
    double ma = (a[1].y - a[0].y)/(a[1].x - a[0].x);
    double mb = (b[1].y - b[0].y)/(b[1].x - b[0].x);
    double x,y;
    //checking edge cases:
    //if lines are paralell 
    if (floatingEquals(ma, mb )){
        return false;
    }
    //if there is an undefined slope
    if (floatingEquals(a[1].x - a[0].x, 0.0)){
        x = a[1].x;
        y = mb * x + b[0].y - mb * b[0].x; 
 
    }
    else if (floatingEquals(b[1].x - b[0].x, 0.0)){
        x = b[1].x;
        y = ma * x + a[0].y - ma * a[0].x; 
    }else{
    //calculating poi 
    x = 
    (b[0].y - ( mb ) * (b[0].x) - (a[0].y - ( ma ) * (a[0].x)))
    /
    ( ma - mb );
    
    y = ma * x + a[0].y - ma * (a[0].x);
    }
    // if POI is in the range of the segments 
    if (std::min(a[0].x, a[1].x) <= x && std::max(a[0].x, a[1].x) >= x && std::min(a[0].y, a[1].y) <= y && std::max(a[0].y, a[1].y) >= y){
        if (std::min(b[0].x, b[1].x) <= x && std::max(b[0].x, b[1].x) >= x && std::min(b[0].y, b[1].y) <= y && std::max(b[0].y, b[1].y) >= y){
            return true;
        }
    }

    return false;
}

bool intersectRay2(const std::array<glm::dvec2, 2>& a,
                   double y,
                   glm::dvec2 &output)
{
    const double x0 = a[0].x, y0 = a[0].y;
    const double x1 = a[1].x, y1 = a[1].y;

    const double dx = x1 - x0;
    const double dy = y1 - y0;

    // If segment is horizontal (dy == 0)
    if (floatingEquals(dy, 0.0)) {
        // Segment is horizontal:
        // - if it's exactly on y (collinear), it's ambiguous (infinite points).
        //   Decide policy: here we treat it as NO intersection (return false).
        if (floatingEquals(y, y0)) return false; // collinear -> no single intersection
        return false; // parallel and different y -> no intersection
    }

    // Use half-open interval [minY, maxY) to avoid double-counting at vertices.
    const double minY = std::min(y0, y1);
    const double maxY = std::max(y0, y1);
    if (!(y >= minY && y < maxY)) {
        return false; // y outside segment's vertical span
    }

    // If segment is vertical, x is constant
    if (floatingEquals(dx, 0.0)) {
        output = glm::dvec2(x0, y);
        return true;
    }

    // Otherwise compute intersection X
    const double m = dy / dx;                 // slope (safe because dx != 0)
    const double b = y0 - m * x0;             // intercept
    const double ix = (y - b) / m;            // x at given y
    output = glm::dvec2(ix, y);
    return true;
}

// bool intersectRay2(const std::array<glm::dvec2, 2> a, const double y, glm::dvec2 &output){

//     //ray starts at point and shoots both ways at slope of 0
    
//     //defining slopes
//     double ma = (a[1].y - a[0].y)/(a[1].x - a[0].x);
//     double mb = 0.0;

//     double x;
//     //checking edge cases:

//     //if lines are paralell 
//     if (floatingEquals(ma, mb )){
//         return false;
//     }

//     //if there is an undefined slope
//     if (floatingEquals((a[1].x - a[0].x), 0.0)){
//         //x = x
//         x = a[1].x;
//     }
//     else{
//         //calculating poi 
//         x = (y - (a[0].y -( ma * a[0].x)))/ma;
//     }

//     //checking if segment is in range of y 
//     if ( std::min(a[0].y, a[1].y) <= y && std::max(a[1].y, a[0].y) >= y ){
//         // std::cout<<"! ("<<a[0].x<<", "<<a[1].x<<"|"<<point.x<<"), ("<<a[0].y<<", "<<a[1].y<<"|"<<point.y<<")"<<std::endl;
//         output = glm::dvec2(x, y);
//         return true;
//     }

//     return false;
// }
bool isIntersectingRay(const std::array<glm::dvec2, 2> a, const glm::dvec2 point ){

    //ray starts at point and shoots right at slope of 0
    
    //defining slopes
    double ma = (a[1].y - a[0].y)/(a[1].x - a[0].x);
    double mb = 0.0;

    double x;
    //checking edge cases:

    //if lines are paralell 
    if (floatingEquals(ma, mb )){
        return false;
    }

    //if there is an undefined slope
    if (floatingEquals((a[1].x - a[0].x), 0.0)){
        //x = x
        x = a[1].x;
    }
    else{
        //calculating poi 
        x = (point.y - (a[0].y -( ma * a[0].x)))/ma;
    }

    //if in range of the x
    if (x >= point.x){
        //checking if segment is in range of y 
        if ( std::min(a[0].y, a[1].y) <= point.y && std::max(a[1].y, a[0].y) >= point.y ){
            // std::cout<<"! ("<<a[0].x<<", "<<a[1].x<<"|"<<point.x<<"), ("<<a[0].y<<", "<<a[1].y<<"|"<<point.y<<")"<<std::endl;
            return true;
        }
    }

    return false;
}


bool isParent(std::shared_ptr<polygon> p, std::shared_ptr<polygon> c){
    //check collisions (error checking)
    for (std::array< glm::dvec2, 2> i: p->perimeter){
        for (std::array< glm::dvec2, 2> j: c->perimeter){
            if (isIntersectingSegments(i, j)){
                std::cout<<"intersecting polygons"<<std::endl;
                std::abort();
            }
        }
    }
    //checking if its a parent using raycasting

    //selecting point to shoot ray from (the centroid of child)
    glm::dvec2 point = c->centroid;
    
    int count = 0;

    // f(x) = point.y {x >= point.x}

    //iterate through all segments of the parent 
    for (std::array<glm::dvec2, 2> i : p->perimeter){
        //if the ray intersects with the parent, AND the parent has a bigger area 
        if (isIntersectingRay(i, point) && abs(p->area) > abs(c->area)){
            count++;
        }
    }

    if (count % 2 == 1){
        // std::cout<<"is parent"<<std::endl;
        return true;
    }
    return false;
}

