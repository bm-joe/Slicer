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
#include <fstream>
#include <sstream>

//glm for mathematical objects
#include <glm/glm.hpp>      
#include <glm/gtc/type_ptr.hpp>                
#include <glm/gtc/matrix_transform.hpp>     
#include <glm/gtc/quaternion.hpp>           
#include <glm/gtx/quaternion.hpp>           


//enums
enum{
    ID_VIEW_TIMER = wxID_HIGHEST + 1,
    ID_MOVE_VIEW_TIMER = wxID_HIGHEST + 2,
    ID_SLICING_PROGRESS_TIMER = wxID_HIGHEST + 3,
    ID_OPEN_FILE = 1,
    ID_SLICE = 2
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
    canvas = new MyGLCanvas(this);
    slicer = new Slicer();
    gaugeOverlay = new GaugeOverlay(this);
    gaugeOverlay->Move(this->GetScreenPosition());
    canvas->setSlicer(slicer);

    STLManager = new STLHandler();
    //making menu bar 
    wxMenuBar *menuBar = new wxMenuBar;

    wxMenu *menuFile = new wxMenu;
    menuFile -> Append(ID_OPEN_FILE, "&Open STL File\tCtrl-O");
    menuFile->Append(ID_SLICE, "&Slice Currently Loaded Model\tCtrl-S");
    menuFile->Append(wxID_EXIT, "&Exit the application\tCtrl-Q");

    menuBar->Append(menuFile, "&File");

    SetMenuBar(menuBar);


    slicingTimer = new wxTimer(this, ID_SLICING_PROGRESS_TIMER);
    //bindingn to event manager
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::OnOpenFile, this, ID_OPEN_FILE);

    Bind(wxEVT_TIMER, &MyFrame::OnSlicing, this, ID_SLICING_PROGRESS_TIMER);
    Bind(wxEVT_MENU, &MyFrame::OnSlice, this, ID_SLICE);
    Bind(wxEVT_MOVE, &MyFrame::OnMove, this );
};




MyGLCanvas::MyGLCanvas(wxWindow* parent) : wxGLCanvas( 
        parent,
        []{
            wxGLAttributes attrs;
            attrs.Defaults();
            attrs.PlatformDefaults(); 
            attrs.EndList();
            return attrs;
        }(),
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxFULL_REPAINT_ON_RESIZE
){
    auto* context = new MyGLContext(this);
    holdTimer = new wxTimer(this, ID_VIEW_TIMER);
    moveTimer = new wxTimer(this, ID_MOVE_VIEW_TIMER);
    ComputeGrid();
    Bind(wxEVT_PAINT, &MyGLCanvas::OnPaint, this);
    Bind(wxEVT_MOUSEWHEEL, &MyGLCanvas::OnScroll, this);
    Bind(wxEVT_KEY_DOWN, &MyGLCanvas::OnKeyDown, this);
    Bind(wxEVT_RIGHT_DOWN, &MyGLCanvas::OnRightDown, this);
    Bind(wxEVT_RIGHT_UP, &MyGLCanvas::OnRightUp, this);
    Bind(wxEVT_TIMER, &MyGLCanvas::OnRightHolding, this, ID_VIEW_TIMER);
    Bind(wxEVT_LEFT_DOWN, &MyGLCanvas::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &MyGLCanvas::OnLeftUp, this);
    Bind(wxEVT_TIMER, &MyGLCanvas::OnLeftHolding, this, ID_MOVE_VIEW_TIMER);
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

    glEnable(GL_NORMALIZE);

    // Smooth shading
    glShadeModel(GL_SMOOTH);

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
        target = glm::vec3(0,0,0);
        
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
        // gridScale += 0.1;
        // ComputeGrid();
        
        if (ti < slicer->sTris.size()-1){
            ti++;
        }
        

        Refresh(false);
        break;
    case WXK_DOWN:
        // ComputeGrid();
        // gridScale -= 0.1;
        
        if (ti>0){
            ti--;
        }

        Refresh(false);
        break;
    //p
    case 'P':
    case 'p':
        std::cout<<"\n\n\n"<<std::endl;
        for (int v = 0; v < 3; v++){
            std::cout<< slicer->sTris.at(ti).verticies[v].x<< ", "<<slicer->sTris.at(ti).verticies[v].y<<", "<< slicer->sTris.at(ti).verticies[v].z<<std::endl;
        }
        break;
    default:
        event.Skip();
        break;
    }
};

void MyGLCanvas::OnScroll(wxMouseEvent& event){
    cameraDistance += event.GetWheelRotation() * scrollSensitivity;
    cameraDistance = glm::clamp(cameraDistance, 20.0f, 100000.0f);
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

        cameraPos.x = target.x + cameraDistance * cos(glm::radians(pitch)) * sin(glm::radians(-yaw));
        cameraPos.z = target.z + cameraDistance * cos(glm::radians(pitch)) * cos(glm::radians(-yaw));
        cameraPos.y = target.y + cameraDistance * sin(glm::radians(pitch));
        // std::cout<<yaw<< ", " << pitch << std::endl;
        glm::mat4 cameraMatrix = glm::lookAt(cameraPos, target, glm::vec3(0,1,0));
        glMultMatrixf(glm::value_ptr(cameraMatrix));
        
        // glDisable(GL_CULL_FACE);
        
    // //positive x indicator 
    // glBegin(GL_QUADS);
    //     glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, redColour);
    //     glNormal3f( 0.0f, 0.0f, 1.0f);
    //     //bottom left corner
    //     glTexCoord2f(0, 0); glVertex3f( 0.0f, 0.0f, 0.0f);
    //     //bottom right corner 
    //     glTexCoord2f(1, 0); glVertex3f(1.0f, 0.0f, 0.0f);
    //     //top right corner
    //     glTexCoord2f(1, 1); glVertex3f(1.0f,1.0f, 0.0f);
    //     //top left corner
    //     glTexCoord2f(0, 1); glVertex3f( 0.0f,1.0f, 0.0f);
    // glEnd();

    
    // //positive y indicator 
    // glBegin(GL_QUADS);
    //     glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, greenColour);
    //     glNormal3f( 0.0f, 1.0f, 0.0f);
    //     //bottom left corner
    //     glTexCoord2f(0, 0); glVertex3f( 0.0f, 0.0f, 0.0f);
    //     //bottom right corner 
    //     glTexCoord2f(1, 0); glVertex3f(1.0f, 0.0f, 0.0f);
    //     //top right corner
    //     glTexCoord2f(1, 1); glVertex3f(1.0f,0.0f, 1.0f);
    //     //top left corner
    //     glTexCoord2f(0, 1); glVertex3f( 0.0f, 0.0f, 1.0f);
    // glEnd();

    // //positive z indicator 
    // glBegin(GL_QUADS);
    //     glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, blueColour);
    //     glNormal3f( 0.0f, 0.0f, 1.0f);
    //     //bottom left corner
    //     glTexCoord2f(0, 0); glVertex3f( 0.0f, 0.0f, 0.0f);
    //     //bottom right corner 
    //     glTexCoord2f(1, 0); glVertex3f(0.0f, 0.0f, 1.0f);
    //     //top right corner
    //     glTexCoord2f(1, 1); glVertex3f(0.0f,1.0f, 1.0f);
    //     //top left corner
    //     glTexCoord2f(0, 1); glVertex3f( 0.0f,1.0f, 0.0f);
    // glEnd();

    // glEnable(GL_CULL_FACE);


    //rendering modlels
    if(isModelLoaded()){
        glBegin(GL_TRIANGLES);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, purpleColour);
            for (int i = 0; i < renderingTriangles.size()/4; i++){
                glNormal3f( renderingTriangles.at(i*4).x, renderingTriangles.at(i*4).y, renderingTriangles.at(i*4).z);
                //bottom left corner
                glVertex3d(renderingTriangles.at(i*4 + 1).x, renderingTriangles.at(i*4 + 1).y, renderingTriangles.at(i*4 + 1).z);
                glVertex3d(renderingTriangles.at(i*4 + 2).x, renderingTriangles.at(i*4 + 2).y, renderingTriangles.at(i*4 + 2).z);
                glVertex3d(renderingTriangles.at(i*4 + 3).x, renderingTriangles.at(i*4 + 3).y, renderingTriangles.at(i*4 + 3).z);
 
            }
       glEnd();
    }

    glLineWidth(1.0f);
    glPointSize(5.0f);
    
    if (slicer->doneToolpath){
        //load points 
        double z = 0.0;
        glDisable(GL_DEPTH_TEST);
        glBegin(GL_LINES);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, blueColour);
            // for (int i = 0; i < slicer->segments.size(); i++){
            //     z = (static_cast<double>(i) * slicer-> layerHeight) + (slicer->layerHeight/2.0);
            //     for (int j = 0; j < slicer->segments.at(i).size(); j++){
            //         glVertex3d(slicer->segments.at(i).at(j).at(0).x, z, slicer->segments.at(i).at(j).at(0).y);
            //         glVertex3d(slicer->segments.at(i).at(j).at(1).x, z, slicer->segments.at(i).at(j).at(1).y);
            //     }
            // }
            for (int i = 0; i < slicer->unprocessedPolygons.size(); i++){
                z = (static_cast<double>(i) * slicer-> layerHeight) + (slicer->layerHeight/2.0);
                for (polygon p : slicer->unprocessedPolygons.at(i)){
                  
                    for (int j = 0; j < p.perimeter.size(); j++){
                        glVertex3d(p.perimeter.at(j)[0].x, z,p.perimeter.at(j)[0].y);
                        glVertex3d(p.perimeter.at(j)[1].x, z,p.perimeter.at(j)[1].y);
                    }
                }
            }

        glEnd();
            //debugging thing
        glLineWidth(2.0f);
        glBegin(GL_LINES);
            
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, redColour);
            glVertex3d(slicer->sTris.at(ti).verticies[0].x, slicer->sTris.at(ti).verticies[0].y, slicer->sTris.at(ti).verticies[0].z);
            glVertex3d(slicer->sTris.at(ti).verticies[1].x, slicer->sTris.at(ti).verticies[1].y, slicer->sTris.at(ti).verticies[1].z);
            glVertex3d(slicer->sTris.at(ti).verticies[1].x, slicer->sTris.at(ti).verticies[1].y, slicer->sTris.at(ti).verticies[1].z);
            glVertex3d(slicer->sTris.at(ti).verticies[2].x, slicer->sTris.at(ti).verticies[2].y, slicer->sTris.at(ti).verticies[2].z);
            glVertex3d(slicer->sTris.at(ti).verticies[2].x, slicer->sTris.at(ti).verticies[2].y, slicer->sTris.at(ti).verticies[2].z);
            glVertex3d(slicer->sTris.at(ti).verticies[0].x, slicer->sTris.at(ti).verticies[0].y, slicer->sTris.at(ti).verticies[0].z);
        glEnd();
        glEnable(GL_DEPTH_TEST);
    }

    glLineWidth(1.0f);
    glBegin(GL_LINES);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, greyColour);

        glNormal3f(0.0f, 1.0f, 0.0f);
        //OPTIMIZE THIS
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
    // std::cout << "RIGHT MOUSE PRESSED: " << startingRPos.x << ", " << startingRPos.y << std::endl;
};
void MyGLCanvas::OnRightUp(wxMouseEvent& event){
    holdTimer->Stop();
    holdingR = false;

    //resetting deltas
    lx = 0.0f;
    ly = 0.0f;

    // std::cout << "RIGHT MOUSE RELEASED: " << OGXDelta << ", " << OGYDelta << std::endl;
};
void MyGLCanvas::OnRightHolding(wxTimerEvent& WXUNUSED(event)){
    wxPoint mousePos = ScreenToClient(::wxGetMousePosition());

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

    lx = mousePos.x;
    ly = mousePos.y;
    Refresh(false);
};

void MyGLCanvas::OnLeftDown(wxMouseEvent& event){
    moveTimer->Start(8);
};
void MyGLCanvas::OnLeftUp(wxMouseEvent&event){
    moveTimer->Stop();
    holdingM = false;

    mlx, mdy = 0.0f;
};

void MyGLCanvas::OnLeftHolding(wxTimerEvent& WXUNUSED(event)){
    wxPoint mousePos = ScreenToClient(::wxGetMousePosition());
    if (holdingM){
        mdx = mousePos.x - mlx;
        mdy = mousePos.y - mly;
    }else{
        mlx = mousePos.x;
        mly = mousePos.y;
        mdx, mdy = 0.0f;

        holdingM = true;
    }

    target.y +=  mdy * moveSensitivity * cameraDistance * ((89.0f - abs(pitch))/89.0f);
    
    //looking down
    target.z += mdy * moveSensitivity * cameraDistance*-cos(glm::radians(yaw)) * (abs(pitch)/89.0f); 
    target.x += mdy * moveSensitivity * cameraDistance* sin(glm::radians(yaw)) * (abs(pitch)/89.0f); 

    //looking sideways 
    target.x += mdx * moveSensitivity *cameraDistance* -cos(glm::radians(yaw));
    target.z += mdx * moveSensitivity * cameraDistance* -sin(glm::radians(yaw));

    mlx = mousePos.x;
    mly = mousePos.y;
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