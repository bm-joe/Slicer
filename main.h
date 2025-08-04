//luke fadel slicer header file>? 
#include "wx/glcanvas.h"

//test
#define GLM_ENABLE_EXPERIMENTAL

//glm for mathematical objects
#include <glm/glm.hpp>                      
#include <glm/gtc/matrix_transform.hpp>     
#include <glm/gtc/quaternion.hpp>           
#include <glm/gtx/quaternion.hpp>       
//defining myapp and myframe classes to override later
class MyApp : public  wxApp {
    public:
        //startup method 
        virtual bool OnInit();
};

class MyFrame : public wxFrame{
    public: 
        //constuructor 
        MyFrame();
    // private:
        //methods
};

class MyGLCanvas : public wxGLCanvas {
    public:
    //construcotr
        MyGLCanvas(wxWindow* parent);
//destructor
    // MyGLCanvas::~MyGLCanvas() {
    //     if (holdTimer) {
    //         holdTimer->Stop();
    //         delete holdTimer;
    //     }
    // };
    private:
        const GLfloat greenColour[4] = {0.0f, 1.0f, 0.0f, 1.0f};
        const GLfloat redColour[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        const GLfloat blueColour[4] = {0.0f, 0.0f, 1.0f, 0.0f};
        const GLfloat purpleColour[4] = {0.5f, 0.0f, 1.0f, 1.0f};
        const GLfloat yellowColour[4] = {1.0f, 1.0f, 0.0f, 1.0f};
        const GLfloat whiteColour[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        const GLfloat pinkColour[4] = {1.0f, 0.0f, 0.96f, 1.0f};
        const GLfloat greyColour[4] = {0.9f, 0.9f, 0.9f, 1.0f};



        //making grid array
        static const int h = 16;
        static const int l = 16;
        float gridScale = 0.5;
        GLfloat gridPoints[(h+l)*2 + 4][2];



        //resizing varibales
        const float fovY = 45.0f;
        float aspect;
        const float nearVar = 0.1f;
        const float farVar = 1000.0f;
        float top;
        float bottom;
        float right;
        float left;

        const float sensitivity = 0.1f;
        const float scrollSensitivity = 0.005f;

        float cameraDistance = 2.0f;

        bool holdingR = false;

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
        
        const glm::vec3 axisY = glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::vec3 axisX = glm::vec3(1.0f, 0.0f, 0.0f);

        float pitch = 0.0f;
        float yaw = 0.0f;

        glm::vec3 cameraPos;
        const glm::vec3 target = glm::vec3(0,0,0);

        //camear method 2
        // glm::quat yawQuat;
        // glm::quat pitchQuat;
        // glm::quat cameraQuat;

        wxTimer* holdTimer; 
        wxPoint startingRPos;
        
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
        void OnKeyDown(wxKeyEvent& event);
        void OnRightDown(wxMouseEvent& event);
        void OnRightUp(wxMouseEvent& event);
        void OnRightHolding(wxTimerEvent& WXUNUSED(event));
        void OnScroll(wxMouseEvent& event);

};

class MyGLContext : public wxGLContext {
    public:
        MyGLContext(wxGLCanvas* canvas);
    private:

};