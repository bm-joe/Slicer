//luke fadel slicer header file>? 
#include "wx/glcanvas.h"

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
//defining myapp and myframe classes to override later
class MyApp : public  wxApp {
    public:
        //startup method 
        virtual bool OnInit();
};

struct triangle {
    std::array<glm::dvec3, 3> verticies;
    double min,max;
};

class Slicer{
    public:


        void setSlicingTris(std::vector<std::array<double,3>> renderingTris){
            //clearing lists from previous slicing
            sTris.clear();
            points.clear();
            maxHeight = -100000000000000.0f;
            minHeight = 100000000000000.0f;

            //remove every fourth vector, which is the normal vector which is useless
            for (int i = (renderingTris.size()/4)-1; i >= 0; i--){
                renderingTris.erase(renderingTris.begin() + (i*4) );
            }

            //DEBUGGING PRINTING
            std::cout << "size after erase: " << renderingTris.size() << std::endl;
            for ( int i = 0; i < renderingTris.size(); i++){
                std::cout << renderingTris.at(i)[0] << ", " <<renderingTris.at(i)[1] << ", " << renderingTris.at(i)[2]<< std::endl;
            }

        
            //converting the list of points into triangle structs 
            for (int i = 0; i < renderingTris.size()/3; i++){
                sTris.push_back({glm::dvec3(renderingTris.at(i)[0], renderingTris.at(i)[1],renderingTris.at(i)[2]), glm::dvec3(renderingTris.at(i+1)[0], renderingTris.at(i+1)[1],renderingTris.at(i+1)[2]), glm::dvec3(renderingTris.at(i+2)[0], renderingTris.at(i+2)[1],renderingTris.at(i+2)[2])});
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
            if (minHeight != 0.0f){
                for (int i = 0; i < sTris.size(); i++){
                    for (int v = 0; v < 3; v++){
                        sTris.at(i).verticies[v].y -= minHeight;
                    }
                }
                maxHeight -= minHeight;
                minHeight =0.0f;
            }

            std::cout << "max: " << maxHeight << " min: " << minHeight << std::endl;
            std::cout << sTris.size() << std::endl;
            double z;
            std::vector<glm::dvec2> currentPoints;
            //main slicing loop
            //iterating through all layers
            for ( int layer = 0; layer < (maxHeight / layerHeight); layer++){
                //defining the z coordinate for convience 
                z = layer * layerHeight;
                
                //iterating through all triangles
                for (int tri = 0; tri < sTris.size(); tri++){
                    //use glm::min(z, y) maybe???
                    //iterating through the verticies of this current triangle
                    for (int v = 0; v < 3; v++){
                        //if the y vertex is below the current slicing level, then break to triangle loop 
                        if (sTris.at(tri).verticies[v].y < z ){
                            //breaking out of verticies loop
                            break;
                        }
                    }

                    currentPoints.push_back({ /*x value*/ 0,z});
                }
            }
        };

        void makeToolPath();

        void writeToolPath();

        double layerHeight = 0.2f;

    private:
        double maxHeight;
        double minHeight;

        std::vector<triangle> sTris;
        std::vector<std::vector<glm::dvec2>> points;
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

    private:
        const GLfloat greenColour[4] = {0.0f, 1.0f, 0.0f, 1.0f};
        const GLfloat redColour[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        const GLfloat blueColour[4] = {0.0f, 0.0f, 1.0f, 0.0f};
        const GLfloat purpleColour[4] = {0.5f, 0.0f, 1.0f, 1.0f};
        const GLfloat yellowColour[4] = {1.0f, 1.0f, 0.0f, 1.0f};
        const GLfloat whiteColour[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        const GLfloat pinkColour[4] = {1.0f, 0.0f, 0.96f, 1.0f};
        const GLfloat greyColour[4] = {0.9f, 0.9f, 0.9f, 1.0f};

        bool loadedModel = false;

        //making grid array
        static const int h = 16;
        static const int l = 16;
        float gridScale = 100;
        GLfloat gridPoints[(h+l)*2 + 4][2];


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
                slicer->slice();

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