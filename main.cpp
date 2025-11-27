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
 
#include "include/MyApp.h"
#include "include/MyFrame.h"

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