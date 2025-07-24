//Luke Fadel
//basic slicer 
// July 23, 2025

//importing libraries 
#include <wx/wxprec.h>

//defining macro and importing more libs 
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
 

//enums
// enum{

// }

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
        //method s
        // void OnExit(wxCommandEvent& event);
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit(){
    //making new frame pointer
    MyFrame *frame = new MyFrame();
    //showing frame  
    frame->Show(true);
    return true;
}

MyFrame::MyFrame() : wxFrame(NULL, wxID_ANY, "balls"){

    // Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
};

// void MyFrame::OnExit(wxCommandEvent& event){
//     Close(true);
// };