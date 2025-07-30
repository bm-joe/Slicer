//luke fadel slicer header file>? 
#include "wx/glcanvas.h"

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
        float OGXDelta;
        float OGYDelta;
        float TempXDelta;
        float TempYDelta;

        wxTimer* holdTimer; 
        wxPoint startingRPos;
        void OnPaint(wxPaintEvent& event);
        void OnKeyDown(wxKeyEvent& event);
        void OnRightDown(wxMouseEvent& event);
        void OnRightUp(wxMouseEvent& event);
        void OnRightHolding(wxTimerEvent& WXUNUSED(event));

};

class MyGLContext : public wxGLContext {
    public:
        MyGLContext(wxGLCanvas* canvas);
    private:

};