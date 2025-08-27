//luke fadel slicer header file>? 
#include "wx/glcanvas.h"
#include <thread>
//test
#define GLM_ENABLE_EXPERIMENTAL

//glm for mathematical objects
#include <glm/glm.hpp>                      
#include <glm/gtc/matrix_transform.hpp>     
#include <glm/gtc/quaternion.hpp>           
#include <glm/gtx/quaternion.hpp>       
#include <fstream>
#include <sstream>
#include <iostream>
//defining double comparing methods

bool floatingEquals(double a, double b, double epsilon = 1e-5){
    return fabs(a-b) < epsilon; 
}
bool floatingNotEquals(double a, double b, double epsilon = 1e-5){
    return fabs(a-b) > epsilon; 
}

//defining myapp and myframe classes to override later
class MyApp : public  wxApp {
    public:
        //startup method 
        virtual bool OnInit();
};

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

class Slicer{
    public:


        void setSlicingTris(std::vector<std::array<double,3>> renderingTris){
            //clearing lists from previous slicing
            sTris.clear();
            segments.clear();
            maxHeight = -100000000000000.0f;
            minHeight = 100000000000000.0f;

            //remove every fourth vector, which is the normal vector which is useless
            for (int i = (renderingTris.size()/4)-1; i >= 0; i--){
                renderingTris.erase(renderingTris.begin() + (i*4) );
            }

            // //DEBUGGING PRINTING
            // std::cout << "size after erase: " << renderingTris.size() << std::endl;
            // for ( int i = 0; i < renderingTris.size(); i++){
            //     std::cout << renderingTris.at(i)[0] << ", " <<renderingTris.at(i)[1] << ", " << renderingTris.at(i)[2]<< std::endl;
            // }

        
            //converting the list of points into triangle structs 
            for (int i = 0; i < renderingTris.size()/3; i++){
                sTris.push_back( Triangle( { 
                    glm::dvec3(renderingTris.at(i*3)[0], renderingTris.at(i*3)[1],renderingTris.at(i*3)[2]), 
                    glm::dvec3(renderingTris.at((i*3)+1)[0], renderingTris.at((i*3)+1)[1],renderingTris.at((i*3)+1)[2]), 
                    glm::dvec3(renderingTris.at((i*3)+2)[0], renderingTris.at((i*3)+2)[1],renderingTris.at((i*3)+2)[2])
                }));
            }
        };

        

        void slice(){
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
            for ( int layer = 0; layer < (maxHeight / layerHeight) +1; layer++){
                //printing status update
                std::cout<<layer/(maxHeight/layerHeight)<<"%"<<std::endl;

                //defining the z coordinate for convenience
                //shifting up by half a layer height to handle coplanar triangles 
                z = ((static_cast<double>(layer)) * layerHeight) + (layerHeight/2.0);

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
                                if (floatingNotEquals(currentTri.getMax(), z) && floatingNotEquals(currentTri.getMin(), z)){
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

                                // std::cout<<"2"<<std::endl;
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
                        // currentSegments.push_back(currentArray);
                    }
                }
                //pushing current points to final list of points
                segments.push_back(currentSegments);
                // std::cout << "printing out points for layer  " <<layer  << std::endl;
                // for ( int hi =0 ; hi < currentPoints.size(); hi++){
                //     std::cout << currentPoints.at(hi).x << ", " << currentPoints.at(hi).y << std::endl;
                // }
            }
            std::cout<< "done slicing" << std::endl;
            doneSlicing = true;
        };

        void makeToolPath();

        void writeToolPath();

        double layerHeight = 1.0;

        bool doneSlicing = false;

    private:
        double maxHeight;
        double minHeight;

        public: 
        std::vector<Triangle> sTris;
        
        std::vector<std::vector<std::array<glm::dvec2, 2>>> segments;
};

class MyGLCanvas : public wxGLCanvas {
    public:
    //construcotr
        MyGLCanvas(wxWindow* parent);
        std::vector<std::array<double, 3>> renderingTriangles;        

        void loadModel(){
            loadedModel = true;
            Refresh(false);
        }
        void unLoadModel(){
            loadedModel = false;
        }
        bool isModelLoaded(){
            return loadedModel;
        }
        void setSlicer(Slicer *s){
            slicer = s;
        }
    private:
        const GLfloat greenColour[4] = {0.0f, 1.0f, 0.0f, 1.0f};
        const GLfloat redColour[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        const GLfloat blueColour[4] = {0.0f, 0.0f, 1.0f, 0.0f};
        const GLfloat purpleColour[4] = {0.5f, 0.0f, 1.0f, 1.0f};
        const GLfloat yellowColour[4] = {1.0f, 1.0f, 0.0f, 1.0f};
        const GLfloat whiteColour[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        const GLfloat pinkColour[4] = {1.0f, 0.0f, 0.96f, 1.0f};
        const GLfloat greyColour[4] = {0.9f, 0.9f, 0.9f, 1.0f};
        const GLfloat blackColour[4] = {1.0f, 1.0f, 1.0f, 1.0f};

        bool loadedModel = false;

        //making grid array
        static const int h = 16;
        static const int l = 16;
        float gridScale = 100;
        GLfloat gridPoints[(h+l)*2 + 4][2];

        //TEMP
        int ti;

        //resizing varibales
        const float fovY = 45.0f;
        float aspect;
        const float nearVar = 0.1f;
        const float farVar = 10000000000.0f;
        float top;
        float bottom;
        float right;
        float left;

        const float sensitivity = 0.1f;
        const float moveSensitivity = 1.0f;
        const float scrollSensitivity = 1.0f;

        float cameraDistance = 2000.0f;

        bool holdingR = false;
        bool holdingM = false;

        //old camera method
        // float OGXDelta;
        // float OGYDelta;
        // float TempXDelta;
        // float TempYDelta;

        //new camera method
        float lx = 0.0f;
        float ly = 0.0f;
        float dx;
        float dy;

        float mlx = 0.0f;
        float mly = 0.0f;
        float mdx;
        float mdy;

        
        const glm::vec3 axisY = glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::vec3 axisX = glm::vec3(1.0f, 0.0f, 0.0f);

        float pitch = 0.0f;
        float yaw = 0.0f;

        glm::vec3 cameraPos;
        glm::vec3 target = glm::vec3(0,0,0);

        //camear method 2
        // glm::quat yawQuat;
        // glm::quat pitchQuat;
        // glm::quat cameraQuat;

        wxTimer* holdTimer; 
        wxTimer* moveTimer; 

        Slicer *slicer;
        
        void ComputeGrid(){
            //making grid
            for ( int i = 0 ; i < l+1; i++){
                gridPoints[i*2][0] = (i - (l/2.0f)) * gridScale;
                gridPoints[i*2][1] = (h/2.0f) * gridScale;
                gridPoints[i*2 +1][0] = (i - (l/2.0f)) * gridScale;
                gridPoints[i*2 +1][1] = (-h/2.0f) * gridScale;
            }
            for ( int i = 0 ; i < h + 1 ;i++){
                gridPoints[i*2 + (l+1)*2][0] = (-l/2.0f) * gridScale;
                gridPoints[i*2+ (l+1)*2][1] = (i - (h/2.0f)) * gridScale;
                gridPoints[i*2 +1+ (l+1)*2][0] = (l/2.0f) * gridScale;
                gridPoints[i*2 +1+ (l+1)*2][1] = (i - (h/2.0f)) * gridScale;
            }
        };



        //event methods
        void OnPaint(wxPaintEvent& event);
        void OnMiddleDown(wxMouseEvent& event);
        void OnMiddleUp(wxMouseEvent& event);
        void OnMiddleHolding(wxTimerEvent& WXUNUSED(event));
        void OnKeyDown(wxKeyEvent& event);
        void OnRightDown(wxMouseEvent& event);
        void OnRightUp(wxMouseEvent& event);
        void OnRightHolding(wxTimerEvent& WXUNUSED(event));
        void OnScroll(wxMouseEvent& event);



};

class STLHandler {
    public:
        //using references (&) to parameters so there isn't a copy of the variable created  
        void parseASCII(std::vector<std::array<double, 3>>& outputArray, std::ifstream& inputFile){
            double vectorStorage[3];
            std::string thisLine;
            while(std::getline(inputFile, thisLine)) {
                std::istringstream iss(thisLine);
                std::string keyword;
                iss >> keyword;
                if(keyword == "facet"){
                    std::string normalStr;
                    iss >> normalStr;
                    double nx, ny, nz;
                    iss >> nx >> ny >> nz;
                    outputArray.push_back({nx, ny, nz});
                    // std::cout << "vertex: " << nx << ", " << ny << ", " << nz << std::endl;
                }
                else if(keyword == "vertex"){
                    double vx, vy, vz;
                    iss >> vx >> vy >> vz;
                    outputArray.push_back({vx, vy, vz});
                    // std::cout << "vertex: " << vx << ", " << vy << ", " << vz << std::endl;
                }
            }
        };
    private:
};

class MyFrame : public wxFrame{
    public: 
        //constuructor 
        MyFrame();
    private:
        MyGLCanvas *canvas;
        STLHandler *STLManager;
        Slicer *slicer;
        //methods
        void OnExit(wxCommandEvent& event){
            Close(true);
        };
        void OnOpenFile(wxCommandEvent& event){
            //if a model isn't loaded
            if(canvas->isModelLoaded()){
                canvas->renderingTriangles.clear();
                canvas->unLoadModel();
            }
            wxFileDialog openFileDialog(
                this,                        // parent window
                "Open STL File",                 // dialog title
                "",                          // default directory
                "",                          // default filename
                "STL files (*.STL;*.stl)|*.STL;*.stl",       // file filter
                wxFD_OPEN | wxFD_FILE_MUST_EXIST);

            if (openFileDialog.ShowModal() == wxID_OK) {
                wxString path = openFileDialog.GetPath();  // Full selected file path

                //parsing file
                std::ifstream inputFile(path.ToStdString());
                if(!inputFile.is_open()){
                    wxMessageBox("There was an error accessing the selected file", "File Error", wxCLOSE | wxICON_ERROR);
                    return;
                }
                //ascii stls 
                // new line character = 10
                STLManager->parseASCII(canvas->renderingTriangles, inputFile);
                // std::cout << canvas -> triangles.at(1)[0] << ", " << canvas -> triangles.at(1)[1]<< ", " << canvas -> triangles.at(1)[2] << std::endl;
                canvas->loadModel();
            }
        };
        void OnSlice(wxCommandEvent& event){
            if (canvas->isModelLoaded()){
                slicer->setSlicingTris(canvas->renderingTriangles);
                std::thread t(&Slicer::slice, slicer);
                t.detach();
                canvas->Refresh(false);

            }else{
                wxMessageBox("There is no model loaded", "Slice Error", wxCLOSE | wxICON_ERROR);
            }
        };
};

class MyGLContext : public wxGLContext {
    public:
        MyGLContext(wxGLCanvas* canvas);
    private:

};