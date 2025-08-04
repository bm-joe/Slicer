//Luke Fadel
//basic slicer 
// July 23, 2025

//importing libraries 
#include <wx/wxprec.h>

//test
#define GLM_ENABLE_EXPERIMENTAL

//defining macro and importing more libs 
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
 
#include "main.h"
#include <iostream>

//glm for mathematical objects
#include <glm/glm.hpp>      
#include <glm/gtc/type_ptr.hpp>                
#include <glm/gtc/matrix_transform.hpp>     
#include <glm/gtc/quaternion.hpp>           
#include <glm/gtx/quaternion.hpp>           


//enums
enum{
    viewTimerID = wxID_HIGHEST + 1,
};

//functions 

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


//overriding constructors 

MyFrame::MyFrame() : wxFrame(NULL, wxID_ANY, "SussySlicer v0 - BETA"/*, wxDefaultPosition, wxSize(500, 500)*/){
    auto* canvas = new MyGLCanvas(this);

};



MyGLCanvas::MyGLCanvas(wxWindow* parent) : wxGLCanvas( 
        parent,
        // Construct the attributes
        []{
            wxGLAttributes attrs;
            attrs.Defaults();
            attrs.PlatformDefaults(); // optional but recommended
            attrs.EndList();
            return attrs;
        }(),
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxFULL_REPAINT_ON_RESIZE
){
    auto* context = new MyGLContext(this);
    holdTimer = new wxTimer(this, viewTimerID);
   
    ComputeGrid();

    Bind(wxEVT_PAINT, &MyGLCanvas::OnPaint, this);
    Bind(wxEVT_MOUSEWHEEL, &MyGLCanvas::OnScroll, this);
    Bind(wxEVT_KEY_DOWN, &MyGLCanvas::OnKeyDown, this);
    Bind(wxEVT_RIGHT_DOWN, &MyGLCanvas::OnRightDown, this);
    Bind(wxEVT_RIGHT_UP, &MyGLCanvas::OnRightUp, this);
    Bind(wxEVT_TIMER, &MyGLCanvas::OnRightHolding, this, viewTimerID);
};

MyGLContext::MyGLContext(wxGLCanvas* canvas) : wxGLContext(canvas){
    SetCurrent(*canvas);
    // set up the parameters we want to use
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);
    // glEnable(GL_COLOR_MATERIAL);
    // add slightly more light, the default lighting is rather dark
    GLfloat ambient[] = { 0.5, 0.5, 0.5, 0.5 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

    // set viewing projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-0.5, 0.5, -0.5, 0.5, 1, 3);

};

//overriding methods

void MyGLCanvas::OnKeyDown(wxKeyEvent& event){
    switch (event.GetKeyCode())
    {
    case WXK_ESCAPE:
        pitch = 0.0f;
        yaw = 0.0f;
        
        //old rotation modes
        // OGXDelta = 0.0f;
        // TempYDelta = 0.0f;
        //  TempXDelta = 0.0f;
        //  OGYDelta = 0.0f;
        
        // cameraQuat = glm::quat();
        // yawQuat = glm::quat();
        // pitchQuat = glm::quat();
        // std::cout << "RESET: " << TempXDelta << ", " << TempYDelta << std::endl;
        Refresh(false);
        break;
    case WXK_UP:
        gridScale += 0.1;
        ComputeGrid();
        Refresh(false);
        break;
    case WXK_DOWN:
        ComputeGrid();
        gridScale -= 0.1;
        Refresh(false);
        break;
    default:
        event.Skip();
        break;
    }
};

void MyGLCanvas::OnScroll(wxMouseEvent& event){
    std::cout << event.GetWheelRotation() << std::endl;
    cameraDistance += event.GetWheelRotation() * scrollSensitivity;
    cameraDistance = glm::clamp(cameraDistance, 2.0f, 100.0f);
    Refresh(false);
};
void MyGLCanvas::OnPaint(wxPaintEvent& WXUNUSED(event)){

    wxPaintDC dc(this);
    //resizing opengl renderable space. this method is called when the window is resized or has other updates
    const wxSize ClientSize = GetClientSize() * GetContentScaleFactor();

    glMatrixMode(GL_PROJECTION);
    // aspect = ClientSize.x / ClientSize.y;
    aspect = static_cast<float>(ClientSize.x) / static_cast<float>(ClientSize.y);

    top = nearVar * tan(glm::radians(fovY)/2.0f);
    bottom = -top;
    right = top * aspect;
    left  = -right;

    glViewport(0,0,ClientSize.x, ClientSize.y);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        glFrustum(left, right, bottom, top, nearVar, farVar);
        // glTranslatef(0.0f,0.0f,-2.0f);

        //CUSTOM ROTATION METHOD USING MY OWN FUNCTIONS 
        // //x mouse motion should rotate around the y axis
        // glRotatef(TempXDelta, 0.0f, 1.0f, 0.0f);
        // //y mouse motion should rotate around the x axis
        // float X;
        // float Z;
        // if(TempXDelta >= 0){
        //     X = (std::abs(TempXDelta-180.0f))/(90.0f) - 1.0f;
        //     if (TempXDelta<= 90.0f){
        //         Z = TempXDelta/90.0f;
        //     }else if(TempXDelta <= 270.0f){
        //         Z = (TempXDelta-180.0f)/90.0f;
        //     }else{
        //         Z = (TempXDelta - 360.0f)/90.0f;
        //     }
        // }else{
        //     X = (std::abs(TempXDelta+180.0f))/(90.0f) - 1.0f;
        //     if(TempXDelta >= -90.0f){
        //         Z = TempXDelta/90.0f;
        //     }else if(TempXDelta >= -270.0f){
        //         Z = (TempXDelta + 180.0f )/90.0f;
        //     }else{
        //         Z = (TempXDelta +360.0f)/90.0f;
        //     }
        // }
        // std::cout << TempXDelta << "; " << X << ", " << Z << std::endl;
        // glRotatef(TempYDelta, X, 0.0f, Z);
        

        //ROTATION USING QUATERNIONS 
        // glm::mat4 cameraMatrix = glm::mat4_cast(cameraQuat);
        // glMultMatrixf(glm::value_ptr(cameraMatrix));

        cameraPos.x = target.x + cameraDistance * cos(glm::radians(pitch)) * sin(glm::radians(-yaw));
        cameraPos.z = target.z + cameraDistance * cos(glm::radians(pitch)) * cos(glm::radians(-yaw));
        cameraPos.y = target.y + cameraDistance * sin(glm::radians(pitch));

        glm::mat4 cameraMatrix = glm::lookAt(cameraPos, target, glm::vec3(0,1,0));
        glMultMatrixf(glm::value_ptr(cameraMatrix));
        
        glDisable(GL_CULL_FACE);
    //positive x indicator 
    glBegin(GL_QUADS);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, redColour);
        glNormal3f( 0.0f, 0.0f, 1.0f);
        //bottom left corner
        glTexCoord2f(0, 0); glVertex3f( 0.0f, 0.0f, 0.0f);
        //bottom right corner 
        glTexCoord2f(1, 0); glVertex3f(1.0f, 0.0f, 0.0f);
        //top right corner
        glTexCoord2f(1, 1); glVertex3f(1.0f,1.0f, 0.0f);
        //top left corner
        glTexCoord2f(0, 1); glVertex3f( 0.0f,1.0f, 0.0f);
    glEnd();

    
    //positive y indicator 
    glBegin(GL_QUADS);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, greenColour);
        glNormal3f( 0.0f, 1.0f, 0.0f);
        //bottom left corner
        glTexCoord2f(0, 0); glVertex3f( 0.0f, 0.0f, 0.0f);
        //bottom right corner 
        glTexCoord2f(1, 0); glVertex3f(1.0f, 0.0f, 0.0f);
        //top right corner
        glTexCoord2f(1, 1); glVertex3f(1.0f,0.0f, 1.0f);
        //top left corner
        glTexCoord2f(0, 1); glVertex3f( 0.0f, 0.0f, 1.0f);
    glEnd();

    //positive z indicator 
    glBegin(GL_QUADS);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, blueColour);
        glNormal3f( 0.0f, 0.0f, 1.0f);
        //bottom left corner
        glTexCoord2f(0, 0); glVertex3f( 0.0f, 0.0f, 0.0f);
        //bottom right corner 
        glTexCoord2f(1, 0); glVertex3f(0.0f, 0.0f, 1.0f);
        //top right corner
        glTexCoord2f(1, 1); glVertex3f(0.0f,1.0f, 1.0f);
        //top left corner
        glTexCoord2f(0, 1); glVertex3f( 0.0f,1.0f, 0.0f);
    glEnd();



    glEnable(GL_CULL_FACE);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, greyColour);

        glNormal3f(0.0f, 1.0f, 0.0f);
        for ( int i = 0 ; i < sizeof(gridPoints) / sizeof(gridPoints[0]); i++){
            glVertex3f(gridPoints[i][0], 0.0f, gridPoints[i][1]);
        } 

    glEnd();
    glFlush();
    SwapBuffers();

    // std::cout << "WINDOW REFRESHING: " << ClientSize.x << ", " << ClientSize.y << std::endl;
};


void MyGLCanvas::OnRightDown(wxMouseEvent& event){
    //starting the timer to call the onholding method every 8 ms
    holdTimer->Start(8);
    startingRPos = event.GetPosition();
    // std::cout << "RIGHT MOUSE PRESSED: " << startingRPos.x << ", " << startingRPos.y << std::endl;
};
void MyGLCanvas::OnRightUp(wxMouseEvent& event){
    holdTimer->Stop();
    holdingR = false;

    //old lookiung method
    //on release, the original delta variables become the old temporary values
    // OGXDelta = TempXDelta;
    // OGYDelta = TempYDelta;

    //resetting deltas
    lx = 0.0f;
    ly = 0.0f;

    // std::cout << "RIGHT MOUSE RELEASED: " << OGXDelta << ", " << OGYDelta << std::endl;
};
void MyGLCanvas::OnRightHolding(wxTimerEvent& WXUNUSED(event)){
    wxPoint mousePos = ScreenToClient(::wxGetMousePosition());

    //old looking method
    // //while holding, the temp delta variables are updated
    // TempXDelta = OGXDelta + (mousePos.x - startingRPos.x );
    // TempYDelta = OGYDelta + (mousePos.y - startingRPos.y );
    
    // TempXDelta = std::fmod(TempXDelta, 360.0f);
    // if(TempYDelta > 89.0f) TempYDelta = 89.0f;
    // else if (TempYDelta < -89.0f) TempYDelta = -89.0f;

    //new looking method
    if(holdingR){
        dx = mousePos.x - lx;
        dy = mousePos.y - ly;
    }else{
        lx = mousePos.x;
        ly = mousePos.y;
        dx, dy = 0.0f;
        holdingR = true;
    }

    pitch += dy*sensitivity;
    yaw += dx*sensitivity;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    //camera methhod 2
    
    // yawQuat = glm::angleAxis(glm::radians(yaw), glm::vec3(0,1,0));
    // glm::vec3 right = glm::rotate(yawQuat, glm::vec3(1,0,0));
    // pitchQuat = glm::angleAxis(glm::radians(pitch), right);

    // cameraQuat = yawQuat * pitchQuat;// * cameraQuat;
    // cameraQuat = glm::normalize(cameraQuat);

    // std::cout << "Right is holding down: " << cameraQuat.x << std::endl;
    lx = mousePos.x;
    ly = mousePos.y;
    Refresh(false);
};


//calling implementation macro for myapp clas s
wxIMPLEMENT_APP(MyApp);

// overriding oninit method to create a nwe frame and show that frame using myframe class 
bool MyApp::OnInit(){
    //making new frame pointer
    MyFrame *frame = new MyFrame();
    //showing frame  
    frame->Show(true);
    return true;
};