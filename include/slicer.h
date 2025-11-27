#pragma once
#include "Triangle.h"
#include "functions.h"
#include "solid.h"
#include <fstream>
#include <sstream>
class Slicer{
    public:
        void slice(std::vector<glm::dvec3> renderingTris, wxString name){
            //setting the slicing triangles and adjusting the model to the printbed 
            //clearing lists from previous slicing
            sTris.clear();
            segments.clear();

            maxHeight = std::numeric_limits<double>::min();
            minHeight = std::numeric_limits<double>::max();

            int fourthSize = (renderingTris.size()/4.0)-1;
            //remove every fourth vector, which is the normal vector which is useless
            for (int i = fourthSize; i >= 0; i--){
                // std::cout<<i<<std::endl;
                slicingProgress = abs(static_cast<double>(i)-static_cast<double>(fourthSize))/static_cast<double>(fourthSize) * 100.0;
                // std::cout<<slicingProgress<<std::endl;
                renderingTris.erase(renderingTris.begin() + (i*4) );
            }
            // std::abort();
            

            for (int i = 0; i < renderingTris.size()/3; i++){
                sTris.push_back( Triangle( { 
                    renderingTris.at(i*3), 
                    renderingTris.at((i*3)+1), 
                    renderingTris.at((i*3)+2)
                }));
            }
            std::cout<<"hellko"<<std::endl;

            std::cout<<"slicing started"<<std::endl;
            //finding min and max height
            for (int tri = 0; tri < sTris.size(); tri++){
                for (int v = 0; v < 3; v++){
                    maxHeight = glm::max(maxHeight, sTris.at(tri).verticies[v].y);
                    minHeight = glm::min(minHeight, sTris.at(tri).verticies[v].y);
                }
            }
            std::cout << "max: " << maxHeight << " min: " << minHeight << std::endl;

            //moving the model to z=0
            if (minHeight != 0.0){
                for (int i = 0; i < sTris.size(); i++){
                    for (int v = 0; v < 3; v++){
                        sTris.at(i).verticies[v].y -= minHeight;
                    }
                }
                maxHeight -= minHeight;
                minHeight =0.0f;
            }
            // std::cout << " pringint triagnles \n\n\n\n\n" << std::endl;

            //     for (int tri = 0; tri < sTris.size(); tri++){
            //         for ( int  v = 0; v  < 3 ; v++){
            //             std::cout << currentTri.verticies[v].x << ", " << currentTri.verticies[v].y << ", " << currentTri.verticies[v].z << std::endl;
            //         }
            //     }

            std::cout << " \n\n\n\n\n" << std::endl;

            std::cout << "max: " << maxHeight << " min: " << minHeight << std::endl;
            std::cout << sTris.size() << std::endl;
            double z;
            glm::dvec3 v1,v2;
            std::vector<std::array<glm::dvec2, 2>> currentSegments;
            std::array<glm::dvec2, 2> currentArray;
            //main slicing loop
            //iterating through all layers
            std::cout<< "before slicing loop" << std::endl;
            for ( int layer = 1; layer < (maxHeight / layerHeight) +1; layer++){
                //printing status update
                slicingProgress = layer/(maxHeight/layerHeight)*100.0;
                // std::cout<<slicingProgress<<std::endl; 
                //defining the z coordinate for convenience
                //shifting up by half a layer height to handle coplanar triangles 
                z = ((static_cast<double>(layer)) * layerHeight);

                //clearing current points
                currentSegments.clear();
                //iterating through all triangles

                // std::cout<<"slicing layer "<< layer << " at z " << z << std::endl;
                for (int tri = 0; tri < sTris.size(); tri++){
                    currentArray.at(0) = glm::dvec2();
                    currentArray.at(1) = glm::dvec2();
                    //defining a reference to the current triangle for memory optimization 
                    Triangle &currentTri = sTris.at(tri);
                    // std::cout <<  "slicing triangle " << tri << " at layer " << layer << " at z " << z << std::endl; 

                    //if the current layer is in between then min and max of the current triangle
                    if (currentTri.getMin() <= z && currentTri.getMax() >= z || floatingEquals(currentTri.getMin(), z) || floatingEquals(currentTri.getMax(), z)){
                        // std::cout<<"triangle " << tri << " is in range at y = " << currentTri.getMin() << " and y = " << currentTri.getMax() <<std::endl;

            
                        //if there is a point on the plane
                        if (currentTri.pointOnPlane(z)){
                            //one point on the plane
                            if (currentTri.numOfPointsOnPlane() == 1){
                                //if not on only on plane
                                if (!floatingEquals(currentTri.getMax(), z) && !floatingEquals(currentTri.getMin(), z)){
                                // std::cout<<"1"<<std::endl;
                                // getting first vertex
                                bool c = false;
                                // std::cout<<"starting"<<std::endl;
                                for (int i= 0; i < 3; i ++ ){
                                    //if the current vertex is on the plane
                                    if (currentTri.pointsOnPlane.at(i)){
                                        currentArray.at(0) = {currentTri.verticies[i].x, currentTri.verticies[i].z};
                                        // std::cout<<"p: "<<currentTri.verticies[i].x<<", "<< currentTri.verticies[i].y<< ", "<< currentTri.verticies[i].z<<std::endl;
                                    }
                                    //if the current vertex is not on the plane  (time 1)
                                    else if(!c){
                                        v1 = currentTri.verticies[i];
                                        // std::cout<<"v1: "<<v1.x<<", "<< v1.y<< ", "<< v1.z<<std::endl;
                                        c = true;
                                    }
                                    //if the current vertex is not on the plane (time 2)
                                    else{
                                        v2 = currentTri.verticies[i];

                                        // std::cout<<"v2: "<<v2.x<<", "<< v2.y<< ", "<< v2.z<<std::endl;
                                    }

                                }
                                // std::cout<<"ended"<<std::endl;
                                
                                //calculating intersecting point 
                                currentArray.at(1) = {
                                    //x intersecction
                                    v1.x + (v2.x - v1.x) * ( (z - v1.y)/(v2.y - v1.y) )
                                    ,
                                    //z intersection
                                    v1.z + (v2.z - v1.z) * ( (z - v1.y)/(v2.y - v1.y) )
                                };

                                // std::cout<<" other: "<<currentArray.at(0).x<<", "<<currentArray.at(0).y<<std::endl;
                                // std::cout<<z<<" interection: "<<currentArray.at(1).x<<", "<<currentArray.at(1).y<<std::endl;

                                currentSegments.push_back(currentArray);
                            }
                            }
                            //two points on the plane
                            else if (currentTri.numOfPointsOnPlane() == 2){

                                // std::cout<<"2 "<< layer<< std::endl;
                                //pushing edge
                                bool c = false;
                                for (int i = 0; i < 3; i++){
                                    if (currentTri.pointsOnPlane.at(i)){
                                        if (!c){
                                            currentArray.at(0) = {currentTri.verticies[i].x, currentTri.verticies[i].z};
                                            c = true;
                                        }else{

                                            currentArray.at(1) = {currentTri.verticies[i].x, currentTri.verticies[i].z};
                                            break;
                                        }
                                    }
                                    
                                } 

                                currentSegments.push_back(currentArray);
                            }
                            //three points on the plane. coplanar triangle, push all three edges ??!//1/1/11
                            //a better strategy is to ignore coplanar triangles 
                            else{
                                std::cout<<"3"<<std::endl;
                            }
                            //three points on the plane


                            // std::cout<< "point is on the plane" << std::endl;
                        }
                        //if no points are on the plane
                        //push two intersecting points
                        else{
                            // std::cout << "point is not on the plane " << std::endl;
                            //find odd point out
                            
                            //running total APPROACH:
                            std::vector<glm::dvec3> above;
                            std::vector<glm::dvec3> below;

                            for (int v = 0; v < 3; v++){
                                if (currentTri.verticies[v].y > z){
                                    above.push_back(currentTri.verticies[v]);
                                }else if (currentTri.verticies[v].y < z){
                                    below.push_back(currentTri.verticies[v]);
                                }else{
                                    std::cout << "\n\n\nSOMETHING IS REALLY WRONG " << std::endl;
                                    std::abort();
                                }
                            }
                            
                            //if look at size of above and below lists
                            //v1 = odd point out
                            //v2 = cycles through the common points 

                            for ( int p = 0; p < 2; p++){
                                //OPTIMIZE THIS
                                if (above.size() == 1){
                                    v1 = above.at(0);
                                    v2 = below.at(p);
                                }else{
                                    v1 = below.at(0);
                                    v2 = above.at(p);
                                }
                                currentArray.at(p) = {
                                    //x intersecction
                                    v1.x + (v2.x - v1.x) * ( (z - v1.y)/(v2.y - v1.y) )
                                    ,
                                    //z intersection
                                    v1.z + (v2.z - v1.z) * ( (z - v1.y)/(v2.y - v1.y) )
                                };
                                // std::cout << "intersecting point pushed!" << std::endl;
                            }

                        currentSegments.push_back(currentArray);

                        
                        }
                    }
                }
                //pushing current points to final list of points
                if (currentSegments.empty()){
                    std::cout<<"segments is emtpy at "<< layer<<std::endl;
                }
                segments.push_back(currentSegments);
                // std::cout << "printing out points for layer  " <<layer  << std::endl;
                // for ( int hi =0 ; hi < currentPoints.size(); hi++){
                //     std::cout << currentPoints.at(hi).x << ", " << currentPoints.at(hi).y << std::endl;
                // }
            }
            std::cout<< "done slicing" << std::endl;
            doneSlicing = true;

            
            // std::cout<< segmentsTouchingTips({glm::dvec2(0,1), glm::dvec2(100, 13)}, {glm::dvec2(-69, -69), glm::dvec2(100, 13)})<<std::endl;

            makeToolPath(name);
        };

        //call slice() before this
        void makeToolPath(wxString name){
            std::cout<<"starting tool path!"<<std::endl;
            //defining list of polygons for each layer
            std::vector<std::shared_ptr<polygon>> currentPolygons;
            //defining currentpolygon
            std::shared_ptr<polygon> currentPoly;


            //creating polygons

            //iterating through every layer
            //TODO FIX THE ERROR WITH THE TOP LAYER OF SEGMENTS
            for ( int layer = 0; layer < segments.size(); layer++){
    // std::cout<<"new layer: " << layer<< " " << segments.size()-1 <<std::endl;
                //reference to this layer 
                std::vector<std::array<glm::dvec2, 2>> &thisLayer = segments.at(layer);
                //remaining segments
                std::vector<std::array<glm::dvec2, 2>> rSegments = segments.at(layer);

                
                //resetting polygons
                currentPolygons.clear();

                //resetting polygon
                currentPoly = std::make_shared<polygon>();

    // std::cout<<"hi "<< segments.at(layer+1).size()<<std::endl;

                if (!segments.at(layer).empty()){
                currentPoly.get()->perimeter.push_back(rSegments.at(0));


                rSegments.erase(rSegments.begin());
        // std::cout<<"new layer: " << layer<< " " << segments.size()-1 <<std::endl;
                //iterating through every segment in that layer

                for (int segment = 0; segment < thisLayer.size() ; segment++){
                    bool found = false;
                    //iterate through all remaining segments to find one that connects to the current open segment of the polygon 
                    //if it can't find another matching segment, check the starting segment of the current polygon and if it matches then the polygon is complete
        // std::cout<<thisLayer.at(segment)[0].x<<", "<<thisLayer.at(segment)[0].y<<" | "<<thisLayer.at(segment)[1].x<<", "<<thisLayer.at(segment)[1].y<<std::endl;
        // std::cout<< rSegments.size() << std::endl;
                    for (int s = 0; s < rSegments.size() ; s++){
                        //if current remaining segment matches up to last segment of the polygon
                        if (segmentsTouchingTips(rSegments.at(s), currentPoly.get()->perimeter.at(currentPoly.get()->perimeter.size()-1))){
        // std::cout<<"touching tips!"<<std::endl;
                            //pushing new segment to polygon, removing it from remaining segments 
                            found = true;
                            currentPoly.get()->perimeter.push_back(rSegments.at(s));
                            rSegments.erase(rSegments.begin() + s);
                            break;
                        }
    
                    }
                    if (!found){
                        //check to see if the polygon is complete 
                        if (segmentsTouchingTips(currentPoly.get()->perimeter.at(0), currentPoly.get()->perimeter.at(currentPoly.get()->perimeter.size()-1))){
        // std::cout << "polygon is complete !"<< std::endl;
                            //erasing poly
                            currentPolygons.push_back(currentPoly);
                            currentPoly = std::make_shared<polygon>();
                            //start the next polygon if there are still segments remaining
                            if (!rSegments.empty()){
                            currentPoly.get()->perimeter.push_back(rSegments.at(0));
                            rSegments.erase(rSegments.begin());
                            }

                        }else{
                        //else print error
                            std::cout<<"error @layer " << layer <<std::endl;
                            
                            // std::abort();
                        }
                    }
                    
                } 
                }
                //pushing polygons
                polygons.push_back(currentPolygons);
                
            }
            std::cout<<"created polygons"<<std::endl;

            //processing polygons, finding parents and children, assigning to solids. 

            //1. assign every polygon to their own solid
            //2. compute polygon areas and centroids 
            for (std::vector<std::shared_ptr<polygon>> layer : polygons){
                // std::vector<std::shared_ptr<solid>> solidlist;
                // solidlist.clear();
                if (!layer.empty()){
                for (std::shared_ptr<polygon> &p : layer){ 
                    p.get()->area = calculatePolygonArea(p.get());
                    p.get()->centroid = calculatePolygonCentroid(p.get());
                    // std::cout<<"area = "<< p.area<<"mm2"<<std::endl;
                    // std::cout<<"centriod = "<< p->centroid.x<<", "<< p->centroid.y<<std::endl;

                    //making solids for each polygon
                    std::shared_ptr<solid> h = std::make_shared<solid>(p);
                    p.get()->thisSolid = h;

                    // solidlist.push_back(h);
                    
                     

                }
                }
            }
            std::cout<<"done area"<<std::endl;
            //3. computing heirachy 
            //every layer 
            int templayercount = 0;
            for ( std::vector<std::shared_ptr<polygon>> l : polygons){
                // std::cout<<"a = {{1,0}, {1,2}}, y = 1new layer"<<std::endl;
                //crossing the polygons with every other polygon on that layer
                if (!l.empty()){
                for (int p = 0; p < l.size(); p++){
                    for (int c = l.size() -1; c > -1; c--){
                        // std::cout<<"c: "<<c<<"p: "<< p <<std::endl;
                        //if the objects aren't the same

                        //each solid should have 1 parent, with a list of children. These children are can only be holes, so there should be no grandchildren in a solid
                        // the children of polygon should only be immediate children, not grandchildren 
                        if (p!=c){
                            //making references to the parent and child pointers
                            std::shared_ptr<polygon> &pp = l.at(p);
                            std::shared_ptr<polygon> &cp = l.at(c);
                            if(isParent(pp, cp)){
                                //adding the child as a child of the parent
                                pp.get()->children.push_back(cp);
                                //setting the child's parent to parent
                                cp.get()->parent = pp;
                                //assigning solid
                                cp.get()->thisSolid = pp.get()->thisSolid;
                                //adding on to the layers
                                cp.get()->layer ++;
                            }
                        }
                    }
                }
                
                    //after determining all parents and children, complete list of solids  
                // std::vector<std::shared_ptr<solid>> solidLayer;
                // solidLayer.clear();
                

                std::vector<fillPoint> tempPoints;
                std::vector<fillPoint> layerPoints;
                layerPoints.clear();
                //generating tool path
                for ( std::shared_ptr<polygon> p : l){
                    if (p.get()->parent == nullptr){
                        //if has no children, then infill between itself and itself.
                        // if (p.get()->children.empty()){
                            std::array<double, 2> bounds = calculatePolygonYBounds(p.get());
                            bool jCondition = true;
                            glm::dvec2 iPoint;
                            //pushing the polygon perimeter before the infill
                            for (std::array<glm::dvec2,2> segment : p.get()->perimeter){
                                    layerPoints.push_back(fillPoint(segment[0], jCondition));
                                    jCondition = false;
                            }
                            //pushing last point
                            layerPoints.push_back(fillPoint(p.get()->perimeter.at(p.get()->perimeter.size()-1)[1], jCondition));

                            //pushing the perimietre of children
                            
                            for (std::shared_ptr<polygon> c : p.get()->children){
                                jCondition = true;
                                for (std::array<glm::dvec2,2> segment : c.get()->perimeter){
                                    layerPoints.push_back(fillPoint(segment[0], jCondition));
                                    jCondition = false;
                                }
                                //pushing last point
                                layerPoints.push_back(fillPoint(c.get()->perimeter.at(c.get()->perimeter.size()-1)[1], jCondition));


                            }
                            //iterating from the bottom of the polygon to the top of the polygon 
                            for (double y = bounds[0] + infillWidth; y < bounds[1]; y+=infillWidth){
                                tempPoints.clear();
                                //iterating through the segments of the polygon
                                for (std::array<glm::dvec2,2> segment : p.get()->perimeter){
                                    if (intersectRay2( segment, y, iPoint )){
                                        tempPoints.push_back(fillPoint(iPoint, false));
                                    }
                                }
                                //iterating throuugh the segments of the polygon's children
                                for (std::shared_ptr<polygon> c : p.get()->children){
                                    for (std::array<glm::dvec2,2> segment : c.get()->perimeter){
                                        if (intersectRay2( segment, y, iPoint )){
                                            tempPoints.push_back(fillPoint(iPoint, false));
                                        }
                                    }
                                }
                                // std::cout<<"before sort:"<<std::endl;
                                // for (fillPoint f: tempPoints){
                                //     std::cout<<f.point.x<< ", "<<f.point.y<<std::endl;
                                // }
                                //sortingthem by xvalue 
                                insertionSort(tempPoints);
                                // std::cout<<"after sort"<<std::endl;
                                // for (fillPoint f: tempPoints){
                                for (int k = 0; k < tempPoints.size(); k++){
                                    if (k%2 == 0){
                                        tempPoints[k].jumpPoint = true;
                                    }
                                    // std::cout<<f.point.x<< ", "<<f.point.y<<std::endl;
                                    layerPoints.push_back(tempPoints[k]);
                                }

                            }
                        // }

                        //if has children, infill between this and layer 1, then layer 2 and layer 3 so on and so forth
                        // else{
                            
                        // }
                    }
                }
                toolpath.push_back(layerPoints);


                //debug printing
                // std::cout<<"new layer"<<std::endl;
                // for (std::shared_ptr<polygon> p : l){
                //     std::cout<<"me: "<<p<< " layer: "<< p.get()->layer<< " area: " << abs(p.get()->area)<<" parent: " << p.get()->parent << " children: ";
                //     for (std::shared_ptr<polygon> c : p.get()->children){
                //         std::cout<<c<<" ";
                //     }
                //     std::cout<< "|"<<std::endl;
                // }
                
                
                
                // solids.push_back(solidLayer);
                }

                std::cout<<templayercount<<std::endl;
                templayercount++;
            }
           


            std::cout<<"done computing toolpath"<<std::endl;

            doneToolpath = true;

            writeToolPath(name);
            
        };

        //call maketoolpath() before this
        void writeToolPath(wxString name){
            std::cout<<name<<std::endl;
            name+=".gcode";
            std::ofstream outputFile = std::ofstream("output/" + static_cast<std::string>(name));
            outputFile << ";generated by SUSSYSLICER by LUKE FADEL\n";
            //put things
            //in absolute mode, the e or extrusion value is summative, so it can never decrease between commands 
            outputFile << "G90 ; Use absolute coordinates \nM82 ; Use absolute extruder positioning\nM140 S60 ; Set bed temperature to 60C (do not wait)\nM104 S205 ; Set nozzle temperature to 205C (do not wait)\nG28 ; Home all axes\nM190 S60 ; Wait for bed temperature to reach 60C\nM109 S205 ; Wait for nozzle temperature to reach 205C\nG29 ; Perform bed leveling\nG92 E0 ; Reset extruder position\nG1 E10 F600 ; Prime nozzle\nG0 F6000 ; Rapid move to start position\nG1 Z"<<layerHeight<<" F300 ; Move to first layer height\n";


             std::cout<<"writing to file now"<<std::endl;
            //writing toolpath to file
            std::cout<<segments.size()<<", "<<polygons.size()<<", "<<toolpath.size()<<std::endl;

            for (int layer = 0; layer < toolpath.size(); layer++){

                double z = ((static_cast<double>(layer+1)) * layerHeight);
                std::cout<<"outputting on layer " <<z<<std::endl;

                outputFile << "G1 Z" << z << "\n";
                //go to point 0

                outputFile << "G1 X" << toolpath[layer][0].point.x << " Y" << toolpath[layer][0].point.y << "\n";
                for (int point = 1; point < toolpath.at(layer).size() ; point ++){
                    if (!toolpath[layer][point].jumpPoint){
                        //writing this point with e value
                        e += calculateEValue(toolpath[layer][point-1].point, toolpath[layer][point].point);
                        outputFile << "G1 X" << toolpath[layer][point].point.x << " Y" << toolpath[layer][point].point.y << " E" << e <<  "\n";

                    }else{
                        //writing this point with E0
                        outputFile << "G1 X" << toolpath[layer][point].point.x << " Y" << toolpath[layer][point].point.y << "\n";
                    }
                    
                }
                std::cout<<"finished layer " << layer<<std::endl;

            }

            outputFile.close();
        };

        double layerHeight = 0.2;
        double infillWidth = 0.5;
        double filamentDiameter = 1.75;

        bool doneSlicing = false;
        bool doneToolpath = false;
        float slicingProgress = 0.0f;
    private:
        double maxHeight;
        double minHeight;
        // std::vector<std::vector<std::shared_ptr<solid>>> solids;
        double e = 0.0;
        double calculateEValue(const glm::dvec2 v1, const glm::dvec2 v2){
            double d = sqrt( pow((v2.x - v1.x),2.0) + pow((v2.y - v1.y),2.0) );
            double Ae = layerHeight * infillWidth;
            double Af = M_PI * pow((filamentDiameter/2.0), 2.0);
            return (Ae * d)/Af;
        };
        
    public: 
        std::vector<Triangle> sTris;
        //making the list out of references to make sure the objects don't get copied and stuff when you do things 
        std::vector<std::vector<std::shared_ptr<polygon>>> polygons;
        std::vector<std::vector<std::array<glm::dvec2, 2>>> segments;
        std::vector<std::vector<fillPoint>> toolpath;
};
