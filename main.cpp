//Luke Fadel
//basic slicer 
// July 23, 2025

//importing libraries 
#include <wx/wxprec.h>

//defining macro and importing more libs 
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
 
#include "main.h"
#include <iostream>

//enums
enum{
    viewTimerID = wxID_HIGHEST + 1,
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

    Bind(wxEVT_PAINT, &MyGLCanvas::OnPaint, this);
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
        OGXDelta = 0.0f;
        TempYDelta = 0.0f;
         TempXDelta = 0.0f;
         OGYDelta = 0.0f;
        std::cout << "RESET: " << TempXDelta << ", " << TempYDelta << std::endl;
        Refresh(false);
        break;
    
    default:
        event.Skip();
        break;
    }
};
void MyGLCanvas::OnPaint(wxPaintEvent& WXUNUSED(event)){

    wxPaintDC dc(this);
    //resizing opengl renderable space. this method is called when the window is resized or has other updates
    const wxSize ClientSize = GetClientSize() * GetContentScaleFactor();
    glViewport(0,0,ClientSize.x, ClientSize.y);
        // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f,0.0f,-2.0f);

        //x mouse motion should rotate around the y axis
        glRotatef(TempXDelta, 0.0f, 1.0f, 0.0f);
        //y mouse motion should rotate around the x axis
        float X;
        float Z;
        if(TempXDelta >= 0){
            X = (std::abs(TempXDelta-180.0f))/(90.0f) - 1.0f;
            if (TempXDelta<= 90.0f){
                Z = TempXDelta/90.0f;
            }else if(TempXDelta <= 270.0f){
                Z = (TempXDelta-180.0f)/90.0f;
            }else{
                Z = (TempXDelta - 360.0f)/90.0f;
            }
        }else{
            X = (std::abs(TempXDelta+180.0f))/(90.0f) - 1.0f;
            if(TempXDelta >= -90.0f){
                Z = TempXDelta/90.0f;
            }else if(TempXDelta >= -270.0f){
                Z = (TempXDelta + 180.0f )/90.0f;
            }else{
                Z = (TempXDelta +360.0f)/90.0f;
            }
        }
        std::cout << TempXDelta << "; " << X << ", " << Z << std::endl;
        glRotatef(TempYDelta, X, 0.0f, Z);

    glBegin(GL_QUADS);
        glNormal3f( 0.0f, 0.0f, 1.0f);
        glTexCoord2f(0, 0); glVertex3f( 0.5f, 0.5f, 0.5f);
        glTexCoord2f(1, 0); glVertex3f(-0.5f, 0.5f, 0.5f);
        glTexCoord2f(1, 1); glVertex3f(-0.5f,-0.5f, 0.5f);
        glTexCoord2f(0, 1); glVertex3f( 0.5f,-0.5f, 0.5f);
    glEnd();

    glBegin(GL_QUADS);
        glNormal3f( 0.0f, 0.0f,-1.0f);
        glTexCoord2f(0, 0); glVertex3f(-0.5f,-0.5f,-0.5f);
        glTexCoord2f(1, 0); glVertex3f(-0.5f, 0.5f,-0.5f);
        glTexCoord2f(1, 1); glVertex3f( 0.5f, 0.5f,-0.5f);
        glTexCoord2f(0, 1); glVertex3f( 0.5f,-0.5f,-0.5f);
    glEnd();

    glBegin(GL_QUADS);
        glNormal3f( 0.0f, 1.0f, 0.0f);
        glTexCoord2f(0, 0); glVertex3f( 0.5f, 0.5f, 0.5f);
        glTexCoord2f(1, 0); glVertex3f( 0.5f, 0.5f,-0.5f);
        glTexCoord2f(1, 1); glVertex3f(-0.5f, 0.5f,-0.5f);
        glTexCoord2f(0, 1); glVertex3f(-0.5f, 0.5f, 0.5f);
    glEnd();

    glBegin(GL_QUADS);
        glNormal3f( 0.0f,-1.0f, 0.0f);
        glTexCoord2f(0, 0); glVertex3f(-0.5f,-0.5f,-0.5f);
        glTexCoord2f(1, 0); glVertex3f( 0.5f,-0.5f,-0.5f);
        glTexCoord2f(1, 1); glVertex3f( 0.5f,-0.5f, 0.5f);
        glTexCoord2f(0, 1); glVertex3f(-0.5f,-0.5f, 0.5f);
    glEnd();

    glBegin(GL_QUADS);
        glNormal3f( 1.0f, 0.0f, 0.0f);
        glTexCoord2f(0, 0); glVertex3f( 0.5f, 0.5f, 0.5f);
        glTexCoord2f(1, 0); glVertex3f( 0.5f,-0.5f, 0.5f);
        glTexCoord2f(1, 1); glVertex3f( 0.5f,-0.5f,-0.5f);
        glTexCoord2f(0, 1); glVertex3f( 0.5f, 0.5f,-0.5f);
    glEnd();

    glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0, 0); glVertex3f(-0.5f,-0.5f,-0.5f);
        glTexCoord2f(1, 0); glVertex3f(-0.5f,-0.5f, 0.5f);
        glTexCoord2f(1, 1); glVertex3f(-0.5f, 0.5f, 0.5f);
        glTexCoord2f(0, 1); glVertex3f(-0.5f, 0.5f,-0.5f);
    glEnd();
        glFlush();
        SwapBuffers();
    // std::cout << "WINDOW REFRESHING: " << ClientSize.x << ", " << ClientSize.y << std::endl;
};


void MyGLCanvas::OnRightDown(wxMouseEvent& event){
    holdTimer->Start(16);
    startingRPos = event.GetPosition();
    std::cout << "RIGHT MOUSE PRESSED: " << startingRPos.x << ", " << startingRPos.y << std::endl;
};
void MyGLCanvas::OnRightUp(wxMouseEvent& event){
    holdTimer->Stop();
    
    //on release, the original delta variables become the old temporary values
    OGXDelta = TempXDelta;


    OGYDelta = TempYDelta;


    std::cout << "RIGHT MOUSE RELEASED: " << OGXDelta << ", " << OGYDelta << std::endl;
};
void MyGLCanvas::OnRightHolding(wxTimerEvent& WXUNUSED(event)){
    wxPoint mousePos = ScreenToClient(::wxGetMousePosition());
    //while holding, the temp delta variables are updated
    TempXDelta = OGXDelta + (mousePos.x - startingRPos.x );
    TempYDelta = OGYDelta + (mousePos.y - startingRPos.y );
    
    TempXDelta = std::fmod(TempXDelta, 360.0f);
    if(TempYDelta > 89.0f) TempYDelta = 89.0f;
    else if (TempYDelta < -89.0f) TempYDelta = -89.0f;

    // std::cout << "Right is holding down: " << TempXDelta << ", " << TempYDelta << std::endl;
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